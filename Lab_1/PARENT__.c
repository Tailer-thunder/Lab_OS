
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "sys/wait.h"
#include <string.h>

int create_process();

int main() {
    char *str = NULL;
    size_t size = 0;
    ssize_t len;

    printf("Hello, I am a parent process, enter lines (Ctrl+D to end):\n");

    while ((len = getline(&str, &size, stdin)) != -1) {
        str[strcspn(str, "\n")] = '\0'; 

        int pipe_fd_1[2], pipe_fd_2[2], pipe_fd_ch[2];

        if (pipe(pipe_fd_1) == -1 || pipe(pipe_fd_2) == -1 || pipe(pipe_fd_ch) == -1) {
            perror("pipe");
            return -1;
        }

        pid_t pid1 = create_process();

        if (pid1 == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid1 == 0) { 
            close(pipe_fd_1[1]);
            dup2(pipe_fd_1[0], STDIN_FILENO);
            close(pipe_fd_1[0]);
            dup2(pipe_fd_ch[1], STDOUT_FILENO);
            close(pipe_fd_ch[1]);

            execl("./child_program", "child_program", NULL);
            perror("execl");
            exit(EXIT_FAILURE);
        } else { 
            close(pipe_fd_1[0]);
            write(pipe_fd_1[1], str, strlen(str) + 1);
            close(pipe_fd_1[1]);

            wait(NULL); 

            pid_t pid2 = create_process();

            if (pid2 == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (pid2 == 0) { 
                close(pipe_fd_ch[1]);
                dup2(pipe_fd_ch[0], STDIN_FILENO);
                close(pipe_fd_ch[0]);
                dup2(pipe_fd_2[1], STDOUT_FILENO);
                close(pipe_fd_2[1]);

                execl("./child2_program", "child2_program", NULL);
                perror("execl");
                exit(EXIT_FAILURE);
            } else { 
                close(pipe_fd_ch[0]);
                wait(NULL); 

                char res_line[144];
                close(pipe_fd_2[1]);
                int n = read(pipe_fd_2[0], res_line, 143);
                if (n > 0) {
                    res_line[n] = '\0';
                    printf("Child process has finished work. Result line: '%s'\n", res_line);
                } else {
                    printf("Failed to read from pipe.\n");
                }
                close(pipe_fd_2[0]);
                wait(NULL);
            }
        }
    }

    free(str); 
    return 0;
}

int create_process() {
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    return pid;
}
