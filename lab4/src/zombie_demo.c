#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

// Функция для создания зомби-процесса
void create_zombie() {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }
    else if (pid == 0) {
        // Дочерний процесс
        printf("Дочерний процесс (PID: %d) запущен\n", getpid());
        printf("Дочерний процесс (PID: %d) завершается\n", getpid());
        exit(0); // Дочерний процесс завершается, но родитель не вызывает wait()
    }
    else {
        // Родительский процесс
        printf("Родительский процесс (PID: %d) создал дочерний процесс (PID: %d)\n", 
               getpid(), pid);
        printf("Родительский процесс продолжает работу 30 секунд...\n");
        printf("В это время дочерний процесс станет зомби!\n");
        
        // Родитель спит 30 секунд, не вызывая wait()
        sleep(30);
        
        printf("Родительский процесс завершает работу\n");
    }
}

// Функция для правильного завершения дочернего процесса
void proper_cleanup() {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }
    else if (pid == 0) {
        // Дочерний процесс
        printf("Дочерний процесс (PID: %d) запущен\n", getpid());
        printf("Дочерний процесс (PID: %d) завершается\n", getpid());
        exit(0);
    }
    else {
        // Родительский процесс
        printf("Родительский процесс (PID: %d) создал дочерний процесс (PID: %d)\n", 
               getpid(), pid);
        
        int status;
        waitpid(pid, &status, 0); // Правильное ожидание завершения дочернего процесса
        
        printf("Родительский процесс дождался завершения дочернего процесса\n");
        printf("Дочерний процесс завершился со статусом: %d\n", WEXITSTATUS(status));
    }
}

int main() {
    printf("=== Демонстрация зомби-процессов ===\n\n");
    
    int choice;
    printf("Выберите режим:\n");
    printf("1 - Создать зомби-процесс\n");
    printf("2 - Правильное завершение дочернего процесса\n");
    printf("3 - Игнорирование SIGCHLD для автоматической очистки\n");
    printf("Ваш выбор: ");
    scanf("%d", &choice);
    
    switch(choice) {
        case 1:
            printf("\n--- Создание зомби-процесса ---\n");
            create_zombie();
            break;
            
        case 2:
            printf("\n--- Правильное завершение ---\n");
            proper_cleanup();
            break;
            
        case 3:
            printf("\n--- Автоматическая очистка через игнорирование SIGCHLD ---\n");
            // Игнорируем SIGCHLD - система автоматически убирает зомби
            signal(SIGCHLD, SIG_IGN);
            
            pid_t pid = fork();
            if (pid == 0) {
                printf("Дочерний процесс (PID: %d) запущен\n", getpid());
                printf("Дочерний процесс (PID: %d) завершается\n", getpid());
                exit(0);
            }
            else {
                printf("Родительский процесс создал дочерний процесс (PID: %d)\n", pid);
                printf("Родительский процесс спит 10 секунд...\n");
                sleep(10);
                printf("Родительский процесс завершает работу\n");
            }
            break;
            
        default:
            printf("Неверный выбор!\n");
            return 1;
    }
    
    return 0;
}