#include <string.h>
#include <stdlib.h>
#include "command_utils.h"

int split_command(const char *input, char *tokens[], char delimiter) {
    int count = 0;
    char *temp = strdup(input);
    char *token = strtok(temp, &delimiter);

    while (token != NULL) {
        tokens[count++] = token;
        token = strtok(NULL, &delimiter);
    }

    tokens[count] = NULL;
    free(temp);
    return count;
}
