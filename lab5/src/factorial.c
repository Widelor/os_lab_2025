#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <getopt.h>

// Глобальные переменные
long long result = 1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Структура для передачи аргументов в поток
typedef struct {
    int start;
    int end;
    int mod;
} thread_args;

// Функция для вычисления частичного произведения
void* calculate_partial(void* args) {
    thread_args* t_args = (thread_args*)args;
    long long partial_result = 1;
    
    for (int i = t_args->start; i <= t_args->end; i++) {
        partial_result = (partial_result * i) % t_args->mod;
    }
    
    // Захватываем мьютекс для обновления глобального результата
    pthread_mutex_lock(&mutex);
    result = (result * partial_result) % t_args->mod;
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

// Функция для разбора аргументов командной строки
void parse_arguments(int argc, char* argv[], int* k, int* pnum, int* mod) {
    static struct option long_options[] = {
        {"k", required_argument, 0, 'k'},
        {"pnum", required_argument, 0, 'p'},
        {"mod", required_argument, 0, 'm'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "k:p:m:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'k':
                *k = atoi(optarg);
                break;
            case 'p':
                *pnum = atoi(optarg);
                break;
            case 'm':
                *mod = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -k <number> --pnum=<threads> --mod=<modulus>\n", argv[0]);
                exit(1);
        }
    }
    
    // Проверка обязательных аргументов
    if (*k <= 0 || *pnum <= 0 || *mod <= 0) {
        fprintf(stderr, "All parameters must be positive integers\n");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    int k = 0, pnum = 0, mod = 0;
    
    // Парсинг аргументов
    parse_arguments(argc, argv, &k, &pnum, &mod);
    
    printf("Calculating %d! mod %d using %d threads\n", k, mod, pnum);
    
    // Если k = 0, факториал равен 1
    if (k == 0) {
        printf("Result: 1\n");
        return 0;
    }
    
    pthread_t threads[pnum];
    thread_args args[pnum];
    
    // Распределение работы между потоками
    int numbers_per_thread = k / pnum;
    int remainder = k % pnum;
    int current_start = 1;
    
    for (int i = 0; i < pnum; i++) {
        int current_end = current_start + numbers_per_thread - 1;
        
        // Распределяем остаток по первым потокам
        if (i < remainder) {
            current_end++;
        }
        
        // Если текущий диапазон корректен
        if (current_start <= current_end) {
            args[i].start = current_start;
            args[i].end = current_end;
            args[i].mod = mod;
            
            if (pthread_create(&threads[i], NULL, calculate_partial, &args[i]) != 0) {
                perror("pthread_create");
                exit(1);
            }
        } else {
            // Если диапазон пустой, создаем поток который ничего не делает
            args[i].start = 1;
            args[i].end = 0; // пустой диапазон
            args[i].mod = mod;
            
            if (pthread_create(&threads[i], NULL, calculate_partial, &args[i]) != 0) {
                perror("pthread_create");
                exit(1);
            }
        }
        
        current_start = current_end + 1;
    }
    
    // Ожидание завершения всех потоков
    for (int i = 0; i < pnum; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            exit(1);
        }
    }
    
    // Уничтожение мьютекса
    pthread_mutex_destroy(&mutex);
    
    printf("Result: %lld\n", result);
    
    return 0;
}


// ./factorial -k 10 --pnum=4 --mod=1000000