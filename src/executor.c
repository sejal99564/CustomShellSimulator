#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "executor.h"

void execute_command(char **args) {
    if (args[0] == NULL) return;

    char command[1024] = "";
    int i = 0;
    int redirect_out = 0, redirect_append = 0, redirect_in = 0;
    char *outfile = NULL, *infile = NULL;

    // Scan for redirection operators
    while (args[i] != NULL) {
        if (strcmp(args[i], ">") == 0) {
            redirect_out = 1;
            outfile = args[i + 1];
            args[i] = NULL;
            break;
        } else if (strcmp(args[i], ">>") == 0) {
            redirect_append = 1;
            outfile = args[i + 1];
            args[i] = NULL;
            break;
        } else if (strcmp(args[i], "<") == 0) {
            redirect_in = 1;
            infile = args[i + 1];
            args[i] = NULL;
            break;
        }
        i++;
    }

    // Build command string
    for (int j = 0; args[j] != NULL; j++) {
        strcat(command, args[j]);
        strcat(command, " ");
    }

    // Handle redirections
    FILE *out = NULL, *in = NULL;
    if (redirect_out && outfile) {
        out = freopen(outfile, "w", stdout);
        if (!out) { perror("Failed to open output file"); return; }
    } else if (redirect_append && outfile) {
        out = freopen(outfile, "a", stdout);
        if (!out) { perror("Failed to open output file"); return; }
    }

    if (redirect_in && infile) {
        in = freopen(infile, "r", stdin);
        if (!in) { perror("Failed to open input file"); return; }
    }

    // Execute the command
    int ret = system(command);

    // Restore stdout and stdin
    if (out) { fflush(stdout); freopen("CON", "w", stdout); }
    if (in) { freopen("CON", "r", stdin); }

    if (ret == -1) {
        perror("Execution failed");
    }
}
