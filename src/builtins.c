#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include "builtins.h"

#define MAX_HISTORY 100
static char *history[MAX_HISTORY];
static int history_count = 0;

void load_history() {
    FILE *file = fopen("history.txt", "r");
    if (!file) return; // no previous history yet

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // remove newline
        history[history_count++] = _strdup(line);
        if (history_count >= MAX_HISTORY) break;
    }

    fclose(file);
}

void save_history() {
    FILE *file = fopen("history.txt", "w");
    if (!file) return;

    for (int i = 0; i < history_count; i++) {
        fprintf(file, "%s\n", history[i]);
    }

    fclose(file);
}

int handle_builtin(char **args) {
    if (args[0] == NULL) return 0;

    // Record command in history
    if (history_count < MAX_HISTORY) {
        history[history_count++] = _strdup(args[0]);
    }

    if (strcmp(args[0], "exit") == 0) {
        save_history(); // persist on exit
        printf("Goodbye, Sejal!\n");
        exit(0);
    }

    else if (strcmp(args[0], "clear") == 0) {
        system("cls");
        return 1;
    }

    else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            printf("Usage: cd <directory>\n");
        } else if (_chdir(args[1]) != 0) {
            perror("cd failed");
        }
        return 1;
    }

    else if (strcmp(args[0], "history") == 0) {
        for (int i = 0; i < history_count; i++) {
            printf("%d: %s\n", i + 1, history[i]);
        }
        return 1;
    }

    return 0;
}
