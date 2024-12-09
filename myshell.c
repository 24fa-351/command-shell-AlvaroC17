#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>

#include "command_utils.h"

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 100

// Forward declarations
void process_command(char *input_line);
void execute_command(char *command, int input_fd, int output_fd);
void handle_piping(char *command_line);
void handle_redirection(char *command, int *input_fd, int *output_fd);

int main(void) {
    char input_line[MAX_INPUT_SIZE];

    while (true) {
        printf("myshell> ");
        if (fgets(input_line, sizeof(input_line), stdin) == NULL) {
            printf("Error reading input. Please try again.\n");
            continue;
        }

        input_line[strcspn(input_line, "\n")] = '\0'; // Remove newline character

        if (strcmp(input_line, "exit") == 0 || strcmp(input_line, "quit") == 0) {
            printf("Exiting myshell.\n");
            break;
        }

        if (strlen(input_line) > 0) {
            process_command(input_line);
        }
    }

    return 0;
}

void process_command(char *input_line) {
    if (strchr(input_line, '|') != NULL) {
        handle_piping(input_line);
    } else {
        execute_command(input_line, STDIN_FILENO, STDOUT_FILENO);
    }
}

void execute_command(char *command, int input_fd, int output_fd) {
    char *arguments[MAX_ARGS];
    int arg_count = split_command(command, arguments, ' ');

    if (arguments[0] == NULL) {
        printf("No command specified.\n");
        return;
    }

    // Handle built-in commands
    if (strcmp(arguments[0], "cd") == 0) {
        if (arguments[1] == NULL) {
            printf("Usage: cd <directory>\n");
        } else if (chdir(arguments[1]) != 0) {
            perror("cd failed");
        }
        return;
    }

    if (strcmp(arguments[0], "pwd") == 0) {
        char current_directory[MAX_INPUT_SIZE];
        if (getcwd(current_directory, sizeof(current_directory)) != NULL) {
            printf("%s\n", current_directory);
        } else {
            perror("pwd failed");
        }
        return;
    }

    // Handle redirection
    handle_redirection(command, &input_fd, &output_fd);

    // Fork and execute the command
    pid_t pid = fork();
    if (pid == 0) { // Child process
        if (input_fd != STDIN_FILENO) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }
        if (output_fd != STDOUT_FILENO) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        if (execvp(arguments[0], arguments) == -1) {
            perror("Command execution failed");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("Fork failed");
    } else { // Parent process
        wait(NULL);
    }
}

void handle_redirection(char *command, int *input_fd, int *output_fd) {
    char *input_file = NULL;
    char *output_file = NULL;
    char *input_redir = strchr(command, '<');
    char *output_redir = strchr(command, '>');

    if (input_redir != NULL) {
        *input_redir = '\0';
        input_file = strtok(input_redir + 1, " ");
        *input_fd = open(input_file, O_RDONLY);
        if (*input_fd == -1) {
            perror("Input file open failed");
        }
    }

    if (output_redir != NULL) {
        *output_redir = '\0';
        output_file = strtok(output_redir + 1, " ");
        *output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (*output_fd == -1) {
            perror("Output file open failed");
        }
    }
}

void handle_piping(char *command_line) {
    char *commands[MAX_ARGS];
    int command_count = split_command(command_line, commands, '|');

    int pipe_fd[2];
    int input_fd = STDIN_FILENO;
    pid_t pid;

    for (int i = 0; i < command_count; i++) {
        pipe(pipe_fd);

        if ((pid = fork()) == 0) { // Child process
            dup2(input_fd, STDIN_FILENO);
            if (i < command_count - 1) {
                dup2(pipe_fd[1], STDOUT_FILENO);
            }
            close(pipe_fd[0]);
            execute_command(commands[i], input_fd, STDOUT_FILENO);
            exit(EXIT_SUCCESS);
        } else if (pid < 0) {
            perror("Fork failed");
        }

        close(pipe_fd[1]);
        input_fd = pipe_fd[0];
        wait(NULL);
    }
}
