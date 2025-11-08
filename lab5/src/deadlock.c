#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void* thread1_function(void* arg) {
    printf("Thread 1: Trying to lock mutex1...\n");
    pthread_mutex_lock(&mutex1);
    printf("Thread 1: Locked mutex1\n");
    
    // Имитация работы
    sleep(1);
    
    printf("Thread 1: Trying to lock mutex2 (non-blocking)...\n");
    
    // Пытаемся захватить второй мьютекс без блокировки
    int result = pthread_mutex_trylock(&mutex2);
    if (result == EBUSY) {
        printf("Thread 1: mutex2 is busy! Releasing mutex1 to avoid deadlock.\n");
        pthread_mutex_unlock(&mutex1);
        return NULL;
    } else if (result == 0) {
        printf("Thread 1: Locked mutex2\n");
        
        // Критическая секция
        printf("Thread 1: Entering critical section\n");
        sleep(1);
        printf("Thread 1: Leaving critical section\n");
        
        pthread_mutex_unlock(&mutex2);
    }
    
    pthread_mutex_unlock(&mutex1);
    printf("Thread 1: Completed successfully\n");
    
    return NULL;
}

void* thread2_function(void* arg) {
    printf("Thread 2: Trying to lock mutex2...\n");
    pthread_mutex_lock(&mutex2);
    printf("Thread 2: Locked mutex2\n");
    
    // Имитация работы
    sleep(2);
    
    printf("Thread 2: Trying to lock mutex1...\n");
    pthread_mutex_lock(&mutex1);
    printf("Thread 2: Locked mutex1\n");
    
    // Критическая секция
    printf("Thread 2: Entering critical section\n");
    sleep(1);
    printf("Thread 2: Leaving critical section\n");
    
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&mutex2);
    printf("Thread 2: Completed successfully\n");
    
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
    
    printf("=== Deadlock Demonstration with Recovery ===\n");
    printf("Thread 1 uses trylock to avoid deadlock\n");
    printf("Thread 2 uses normal blocking calls\n\n");
    
    // Создаем потоки
    if (pthread_create(&thread1, NULL, thread1_function, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }
    
    if (pthread_create(&thread2, NULL, thread2_function, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }
    
    // Ожидаем завершения потоков
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    printf("\nMain thread: All threads completed\n");
    
    // Очистка ресурсов
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);
    
    return 0;
}