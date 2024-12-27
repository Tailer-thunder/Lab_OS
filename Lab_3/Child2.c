#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>

#define FILE_SIZE 1024
#define FILE_NAME "shared_memory"

void remove_extra_spaces(char *str) {
    char *dest = str;
    int space_flag = 0;

    while (*str) {
        if (*str == ' ') {
            if (!space_flag) {
                *dest++ = *str;
                space_flag = 1;
            }
        } else {
            *dest++ = *str;
            space_flag = 0;
        }
        str++;
    }
    *dest = '\0';
}

int main() {
    int fd;
    char *shared_memory;

    fd = open(FILE_NAME, O_RDWR);
    if (fd == -1) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    shared_memory = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("Failed to map file");
        exit(EXIT_FAILURE);
    }
    close(fd);


    remove_extra_spaces(shared_memory);


    if (munmap(shared_memory, FILE_SIZE) == -1) {
        perror("Failed to unmap memory");
        exit(EXIT_FAILURE);
    }

    return 0;
}
