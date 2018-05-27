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
 * define global variables
 */
#define MAX_NUM_ARGS 7
#define MAX_INPUT_LINE 80
#define MAX_LEN_PROMPT 10
#define MAX_NUM_DIRS 10
#define CONFIG_FILENAME "./.uvshrc"

/*
 * initialize other global variables
 */
int _num_dirs = 0; // number of directories listed in config file; i.e. number of entries in _dirs[]
char _dirs[MAX_NUM_DIRS][MAX_INPUT_LINE]; // array of directories listed in config file
char _binary_fullpath[MAX_INPUT_LINE]; // string with full filepath of binary command to be executed

/*
 * reads remaining lines (other than first line) of config file
 * to get list of directories in which binary commands are stored
 * @param: fp = pointer to FILE CONFIG_FILENAME
 * sets global variables _num_dirs and _dirs[]
 */
void getCommandDirectories(FILE * fp) {
    while( fgets(_dirs[_num_dirs], MAX_INPUT_LINE, fp) && (_num_dirs < MAX_NUM_DIRS) ) {
        _num_dirs++;
    }
    for (int i = 0; i < _num_dirs; i++)
    {
        if (_dirs[i][strlen(_dirs[i])-1]) {
            _dirs[i][strlen(_dirs[i])-1] = 0; // remove newline character if it exists
        }
        printf("%d %s\n", i, _dirs[i]);
    }
}

/*
 * searches through directores in _dirs[] looking for binary matching
 * @param binary_name
 * returns 1 if successful, -1 if can't be found
 */
int findBinaryForCommand(char binary_name[]) {
    // first check that binary_name is full path already
    FILE * fp = fopen(binary_name, "r");
    if (fp)
    {
        memcpy(_binary_fullpath, binary_name, strlen(binary_name) + 1);
        fclose(fp);
        return 1;
    }

    // if binary_name hasn't supplied full path
    for (int i = 0; i < _num_dirs; i++)
    {
        char test_path[MAX_INPUT_LINE];
        strcpy(test_path, _dirs[i]);
        strcat(test_path, "/");
        strcat(test_path, binary_name);
        printf("Searching test_path %s\n", test_path);
        fp = fopen(test_path, "r");
        if (fp) {
            memcpy(_binary_fullpath, test_path, strlen(test_path) + 1);
            fclose(fp);
            return 1;
        }  
    }
    return -1;
}

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
        if (prompt[strlen(prompt) - 1] == '\n')
        {
            prompt[strlen(prompt)-1] = 0;  // remove newline character
        }
        getCommandDirectories(fp);
        break;   
    }

    fclose(fp);
    free(line);
    return prompt;
}

/*
 * break user inputted string up into tokens, delimited by space
 */
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

    /*
    for (int i = 0; i < num_tokens; i++)
    {
        printf("%d: %s\n", i, tokens[i]); //print tokens for now
    }
    */

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

        char *token[MAX_NUM_ARGS];
        int num_tokens = tokenizeInput(token, input);

        if (strcmp(input, "exit") == 0) {
            exit(0);
        }
        else {
            if (findBinaryForCommand(token[0]) > 0) { /************** NEED BETTER WAY TO PASS BINARY NAME ************/
                printf("Binary path: %s\n", _binary_fullpath);
            } 
            else {
                fprintf(stderr, "Error: could not find location of binary %s\n", input);
            }
        }
    }
}