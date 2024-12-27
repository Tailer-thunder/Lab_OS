
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include <ctype.h> 

int main() {
    char *str_ch = malloc(144 * sizeof(char));
    char *res_str_ch = malloc(144 * sizeof(char));

    if (!str_ch || !res_str_ch) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    int n = read(STDIN_FILENO, str_ch, 143);
    if (n > 0) {
        str_ch[n] = '\0';
        int j = 0, i = 0;

        while (i < n) {
            if (str_ch[i] != ' ') {
                res_str_ch[j++] = str_ch[i++]; 
            } else {
                res_str_ch[j++] = ' ';
                while (i < n && str_ch[i] == ' ') {
                    i++;
                }
            }
        }

        if (j > 0 && res_str_ch[j - 1] == ' ') {
            j--;
        }
        res_str_ch[j] = '\0';
        write(STDOUT_FILENO, res_str_ch, j);
    } else {
        printf("Failed to read from pipe.\n");
    }

    free(str_ch);
    free(res_str_ch);
    return 0;
}
