#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include "parser.h"
#include "executor.h"
#include "builtins.h"

#ifdef _WIN32
#include <windows.h>
#endif

/* =========================================================
   COLOR DEFINITIONS FOR PROMPT AND BANNER
   ========================================================= */
#define COL_RESET   "\x1b[0m"
#define COL_BOLD    "\x1b[1m"
#define COL_USER    "\x1b[32m" /* Green */
#define COL_PATH    "\x1b[34m" /* Blue  */
#define COL_DOLLAR  "\x1b[33m" /* Yellow */

/* Enable ANSI escape processing on Windows consoles */
static void enable_ansi_on_windows(void) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}

/* =========================================================
   CUSTOM GETLINE IMPLEMENTATION (for Windows)
   ========================================================= */
ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    if (*lineptr == NULL || *n == 0) {
        *n = 128;
        *lineptr = malloc(*n);
        if (*lineptr == NULL) return -1;
    }

    size_t i = 0;
    int c;
    while ((c = fgetc(stream)) != EOF) {
        if (i + 1 >= *n) {
            *n *= 2;
            *lineptr = realloc(*lineptr, *n);
            if (*lineptr == NULL) return -1;
        }
        (*lineptr)[i++] = c;
        if (c == '\n') break;
    }

    if (i == 0 && c == EOF) return -1;
    (*lineptr)[i] = '\0';
    return (ssize_t)i;
}

/* =========================================================
   STRING TRIM FUNCTION
   ========================================================= */
static char* trim(char* str) {
    if (!str) return NULL;
    while (*str == ' ' || *str == '\t' || *str == '\n') str++;
    if (*str == '\0') return str;
    char* end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n'))
        *end-- = '\0';
    return str;
}

/* =========================================================
   COLORFUL PROMPT FUNCTION
   ========================================================= */
static void print_prompt(const char *user, const char *cwd) {
    enable_ansi_on_windows();  // ensure color support

    printf("%s%s%s:%s%s%s %s$%s ",
           COL_USER, user, COL_RESET,
           COL_PATH, cwd, COL_RESET,
           COL_DOLLAR, COL_RESET);

    fflush(stdout);
}

/* =========================================================
   MAIN SHELL LOOP
   ========================================================= */
int main() {
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    int command_count = 0;   // 🔹 Track executed commands

    const char *user = getenv("USERNAME");
    if (!user) user = "user";

    /* =========================================================
       LOAD COMMAND HISTORY FROM FILE
       ========================================================= */
    load_history(); // Load persistent history

    /* =========================================================
       WELCOME BANNER
       ========================================================= */
    enable_ansi_on_windows(); // enable color for banner too
    printf("\n");
    printf("%s============================================\n", COL_BOLD);
    printf("   %sWelcome to Custom Shell Simulator%s\n", COL_USER, COL_RESET);
    printf("   Developed by: %sSejal Saquib%s\n", COL_PATH, COL_RESET);
    printf("============================================%s\n", COL_RESET);
    printf("Type '%shelp%s' to list built-in commands.\n\n", COL_DOLLAR, COL_RESET);

    while (1) {
        char cwd[PATH_MAX];
        if (!getcwd(cwd, sizeof(cwd))) {
            perror("getcwd");
            strcpy(cwd, "?");
        }

        print_prompt(user, cwd);

        nread = getline(&line, &len, stdin);
        if (nread == -1) {
            printf("\nYou executed %d commands this session.\n", command_count);
            printf("Goodbye, %s!\n", user);
            break;
        }

        char *cmd = trim(line);
        if (strlen(cmd) == 0) continue;

        command_count++;  // 🔹 Increment for each valid command

        char **args = parse_input(cmd);

        if (!handle_builtin(args)) {
            execute_command(args);
        }

        free(args);
    }

    /* =========================================================
       SAVE COMMAND HISTORY TO FILE BEFORE EXIT
       ========================================================= */
    save_history(); // Persist session history

    free(line);
    return 0;
}
