
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include <ctype.h> 

int main() {
    char *str_ch = malloc(144 * sizeof(char));
    if (!str_ch) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    int n = read(STDIN_FILENO, str_ch, 143);
    if (n > 0) {
        str_ch[n] = '\0';
        for (int i = 0; i < n; i++) {
            str_ch[i] = toupper((unsigned char)str_ch[i]);
        }
        write(STDOUT_FILENO, str_ch, n);
    } else {
        printf("Failed to read from pipe.\n");
    }

    free(str_ch);
    return 0;
}
