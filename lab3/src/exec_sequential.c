#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <seed> <array_size>\n", argv[0]);
        printf("Example: %s 123 100\n", argv[0]);
        return 1;
    }

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    pid_t pid = fork();
    
    if (pid == -1) {
        // Ошибка при создании процесса
        perror("fork failed");
        return 1;
    }
    else if (pid == 0) {
        // Дочерний процесс
        printf("Child process (PID: %d) is starting sequential_min_max...\n", getpid());
        
        // Используем execv для запуска sequential_min_max
        char *args[] = {"./sequential_min_max", argv[1], argv[2], NULL};
        
        if (execv("./sequential_min_max", args) == -1) {
            perror("execv failed");
            exit(1);
        }
        // Этот код не выполнится, если execv успешен
    }
    else {
        // Родительский процесс
        printf("Parent process (PID: %d) created child process (PID: %d)\n", 
               getpid(), pid);
        printf("Waiting for child process to complete...\n");
        
        int status;
        waitpid(pid, &status, 0);  // Ожидаем завершения дочернего процесса
        
        gettimeofday(&end_time, NULL);
        
        double elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000.0;
        elapsed_time += (end_time.tv_usec - start_time.tv_usec) / 1000.0;
        
        if (WIFEXITED(status)) {
            printf("Child process exited with status: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child process terminated by signal: %d\n", WTERMSIG(status));
        }
        
        printf("Total execution time: %.2f ms\n", elapsed_time);
    }
    
    return 0;
}