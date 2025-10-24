#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <signal.h>
#include <errno.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

// Глобальные переменные для хранения PID дочерних процессов и их количества
static int *child_pids_array = NULL;
static int child_processes_count = 0;

// Обработчик сигнала SIGALRM
void alarm_handler(int sig) {
    for (int i = 0; i < child_processes_count; i++) {
        if (child_pids_array[i] > 0) {
            kill(child_pids_array[i], SIGKILL);
        }
    }
}

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    int pnum = -1;
    int timeout = -1; // Новый параметр таймаута
    bool with_files = false;

    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {
            {"seed", required_argument, 0, 0},
            {"array_size", required_argument, 0, 0},
            {"pnum", required_argument, 0, 0},
            {"timeout", required_argument, 0, 0}, // Добавляем новую опцию
            {"by_files", no_argument, 0, 'f'},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        seed = atoi(optarg);
                        if (seed <= 0) {
                            printf("seed must be a positive number\n");
                            return 1;
                        }
                        break;
                    case 1:
                        array_size = atoi(optarg);
                        if (array_size <= 0) {
                            printf("array_size must be a positive number\n");
                            return 1;
                        }
                        break;
                    case 2:
                        pnum = atoi(optarg);
                        if (pnum <= 0) {
                            printf("pnum must be a positive number\n");
                            return 1;
                        }
                        break;
                    case 3: // Обработка таймаута
                        timeout = atoi(optarg);
                        if (timeout <= 0) {
                            printf("timeout must be a positive number\n");
                            return 1;
                        }
                        break;
                    case 4:
                        with_files = true;
                        break;
                    default:
                        printf("Index %d is out of options\n", option_index);
                }
                break;
            case 'f':
                with_files = true;
                break;
            case '?':
                break;
            default:
                printf("getopt returned character code 0%o?\n", c);
        }
    }

    if (optind < argc) {
        printf("Has at least one no option argument\n");
        return 1;
    }

    if (seed == -1 || array_size == -1 || pnum == -1) {
        printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--timeout \"num\"]\n",
               argv[0]);
        return 1;
    }

    // Инициализация глобальных переменных для обработчика сигнала
    child_pids_array = malloc(sizeof(int) * pnum);
    child_processes_count = pnum;

    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);
    int active_child_processes = 0;

    int pipes[pnum][2];
    if (!with_files) {
        for (int i = 0; i < pnum; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("pipe failed");
                return 1;
            }
        }
    }

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    int chunk_size = array_size / pnum;

    for (int i = 0; i < pnum; i++) {
        pid_t child_pid = fork();
        if (child_pid >= 0) {
            active_child_processes += 1;
            child_pids_array[i] = child_pid; // Сохраняем PID для обработчика

            if (child_pid == 0) {
                // Дочерний процесс
                int start = i * chunk_size;
                int end = (i == pnum - 1) ? array_size : (i + 1) * chunk_size;
                
                struct MinMax local_min_max = GetMinMax(array, start, end);

                if (with_files) {
                    char filename[32];
                    sprintf(filename, "min_max_%d.txt", getpid());
                    FILE *file = fopen(filename, "w");
                    if (file == NULL) {
                        perror("fopen failed");
                        exit(1);
                    }
                    fprintf(file, "%d %d", local_min_max.min, local_min_max.max);
                    fclose(file);
                } else {
                    close(pipes[i][0]);
                    write(pipes[i][1], &local_min_max, sizeof(struct MinMax));
                    close(pipes[i][1]);
                }
                free(array);
                exit(0);
            }
        } else {
            printf("Fork failed!\n");
            return 1;
        }
    }

    // Установка обработчика сигнала SIGALRM
    if (timeout > 0) {
        signal(SIGALRM, alarm_handler);
        alarm(timeout); // Установка будильника
    }

    // Неблокирующее ожидание завершения дочерних процессов
    while (active_child_processes > 0) {
        int status;
        pid_t finished_pid = waitpid(-1, &status, WNOHANG);
        
        if (finished_pid > 0) {
            active_child_processes -= 1;
        } else if (finished_pid == 0) {
            // Дочерние процессы еще работают
            usleep(10000); // Небольшая задержка чтобы не нагружать CPU
        } else {
            if (errno != ECHILD) {
                perror("waitpid failed");
            }
            break;
        }
    }

    // Отключаем таймаут если он был установлен
    if (timeout > 0) {
        alarm(0);
    }

    struct MinMax min_max;
    min_max.min = INT_MAX;
    min_max.max = INT_MIN;

    for (int i = 0; i < pnum; i++) {
        int min = INT_MAX;
        int max = INT_MIN;

        if (with_files) {
            char filename[32];
            sprintf(filename, "min_max_%d.txt", child_pids_array[i]);
            FILE *file = fopen(filename, "r");
            if (file != NULL) {
                fscanf(file, "%d %d", &min, &max);
                fclose(file);
                remove(filename);
            }
        } else {
            close(pipes[i][1]);
            struct MinMax local_min_max;
            if (read(pipes[i][0], &local_min_max, sizeof(struct MinMax)) > 0) {
                min = local_min_max.min;
                max = local_min_max.max;
            }
            close(pipes[i][0]);
        }

        if (min < min_max.min) min_max.min = min;
        if (max > min_max.max) min_max.max = max;
    }

    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0; // Исправлено: finit_time -> finish_time

    free(array);
    free(child_pids_array);

    printf("Min: %d\n", min_max.min);
    printf("Max: %d\n", min_max.max);
    printf("Elapsed time: %fms\n", elapsed_time);
    fflush(NULL);
    return 0;
}


/**
 * 
 * Без таймаута (обычная работа)
./parallel_min_max --seed 123 --array_size 100 --pnum 4

С таймаутом 5 секунд
./parallel_min_max --seed 123 --array_size 100 --pnum 4 --timeout 5

С таймаутом и использованием файлов
./parallel_min_max --seed 123 --array_size 100 --pnum 4 --timeout 5 --by_files
 * 
 * 
 */