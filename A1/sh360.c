/*
 * sh360.c
 * CSC 360, Summer 2018
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * set global variables
 */
#define MAX_NUM_ARGS 7
#define MAX_INPUT_LINE 80
#define MAX_LEN_PROMPT 10
#define MAX_NUM_DIRS 10


int main(int argc, char *argv[]) {
    char input[MAX_INPUT_LINE];
    int  line_len;

    for(;;) {
        fprintf(stdout, "> ");
        fflush(stdout);
        fgets(input, MAX_INPUT_LINE, stdin);
        if (input[strlen(input) - 1] == '\n') {
            input[strlen(input) - 1] = '\0';
        }

        line_len = strlen(input); 
        fprintf(stdout, "echo: line was %d chars long\n", line_len);

        if (strcmp(input, "exit") == 0) {
            exit(0);
        } 
    }
}