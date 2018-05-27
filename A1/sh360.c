/*
 * sh360.c
 * CSC 360, Summer 2018
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

/*
 * set global variables
 */
#define MAX_NUM_ARGS 7
#define MAX_INPUT_LINE 80
#define MAX_LEN_PROMPT 10
#define MAX_NUM_DIRS 10
#define CONFIG_FILENAME "./.uvshrc"

/*
 * reads first line of .sh360rc file to get string 
 * to be used as command prompt
 */
/*
 * this function has been adapted from code given as example in
 * https://www.tutorialspoint.com/c_standard_library/c_function_fopen.htm
 * and from example in 
 * http://man7.org/linux/man-pages/man3/getline.3.html
 */
char* getCommandPrompt() {
    FILE * fp;
    char * line = NULL;
    char * prompt = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(CONFIG_FILENAME, "r");

    if (fp == NULL)
    {
        fprintf(stderr, "Error: cannot open file %s\n", CONFIG_FILENAME);
        exit(1);
    }

    while( (read = getline(&line, &len, fp)) != -1 ) {
        prompt = line;
        prompt[strlen(prompt)-1] = 0;  // remove newline character
        break;   
    }

    fclose(fp);
    free(line);
    return prompt;
}

/*
 * code adapted from appendix_e.c
 */
int tokenizeInput(char** tokens, char* input) {
    int num_tokens = 0;
    char *t;

    t = strtok(input, " ");
    while (t != NULL && num_tokens < MAX_NUM_ARGS) {
        tokens[num_tokens] = t;
        num_tokens++;
        t = strtok(NULL, " ");
    }
    tokens[num_tokens] = NULL;

    for (int i = 0; i < num_tokens; i++)
    {
        printf("%d: %s\n", i, tokens[i]); //print tokens for now
    }

    return num_tokens;
}

int main(int argc, char *argv[]) {
    char input[MAX_INPUT_LINE];
    int  line_len;

    char* commandPrompt = getCommandPrompt();

    // infinite loop to accept user input
    for(;;) {
        fprintf(stdout, "%s ", commandPrompt);
        fflush(stdout);
        fgets(input, MAX_INPUT_LINE, stdin);
        if (input[strlen(input) - 1] == '\n') {
            input[strlen(input) - 1] = '\0';
        }

        line_len = strlen(input); 
        fprintf(stdout, "echo: line was %d chars long\n", line_len);

        char *token[MAX_NUM_ARGS];
        int num_tokens = tokenizeInput(token, input);

        if (strcmp(input, "exit") == 0) {
            exit(0);
        } 
    }
}