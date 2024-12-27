#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define FILE_SIZE 1024
#define FILE_NAME "shared_memory"

int main() {
    int fd;
    char *shared_memory;
    pid_t child1, child2;

    // Создаем файл и выделяем память
    fd = open(FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, FILE_SIZE) == -1) {
        perror("Failed to set file size");
        exit(EXIT_FAILURE);
    }

    shared_memory = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("Failed to map file");
        exit(EXIT_FAILURE);
    }
    close(fd);

    // Ввод данных пользователем
    printf("Enter a string: ");
    fgets(shared_memory, FILE_SIZE, stdin);
    shared_memory[strcspn(shared_memory, "\n")] = '\0'; // Убираем символ новой строки

    // Запуск первого дочернего процесса
    if ((child1 = fork()) == 0) {
        execl("./child1", "./child1", NULL); // Запускаем программу child1
        perror("Failed to exec child1");
        exit(EXIT_FAILURE);
    } else if (child1 < 0) {
        perror("Failed to fork child1");
        exit(EXIT_FAILURE);
    }

    waitpid(child1, NULL, 0); // Ожидаем завершения child1

    // Запуск второго дочернего процесса
    if ((child2 = fork()) == 0) {
        execl("./child2", "./child2", NULL); // Запускаем программу child2
        perror("Failed to exec child2");
        exit(EXIT_FAILURE);
    } else if (child2 < 0) {
        perror("Failed to fork child2");
        exit(EXIT_FAILURE);
    }

    waitpid(child2, NULL, 0); // Ожидаем завершения child2

    // Вывод результата
    printf("Final result: %s\n", shared_memory);

    // Освобождение ресурсов
    if (munmap(shared_memory, FILE_SIZE) == -1) {
        perror("Failed to unmap memory");
        exit(EXIT_FAILURE);
    }
    if (unlink(FILE_NAME) == -1) {
        perror("Failed to delete file");
        exit(EXIT_FAILURE);
    }

    return 0;
}
