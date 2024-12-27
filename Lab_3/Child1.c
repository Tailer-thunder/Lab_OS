#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define FILE_SIZE 1024
#define FILE_NAME "shared_memory"

void to_uppercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper(str[i]);
    }
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

    to_uppercase(shared_memory);

    if (munmap(shared_memory, FILE_SIZE) == -1) {
        perror("Failed to unmap memory");
        exit(EXIT_FAILURE);
    }

    return 0;
}
