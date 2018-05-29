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
char _prompt[MAX_LEN_PROMPT]; // string to be displayed for each command prompt

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
    int i;
    for (i = 0; i < _num_dirs; i++)
    {
        if (_dirs[i][strlen(_dirs[i])-1]) {
            _dirs[i][strlen(_dirs[i])-1] = 0; // remove newline character if it exists
        }
        //printf("%d %s\n", i, _dirs[i]);
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
    int i;
    for (i = 0; i < _num_dirs; i++)
    {
        char test_path[MAX_INPUT_LINE];
        strcpy(test_path, _dirs[i]);
        strcat(test_path, "/");
        strcat(test_path, binary_name);
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
int getCommandPrompt() {
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
        break;   
    }
    memcpy(_prompt, prompt, strlen(prompt) + 1);
    getCommandDirectories(fp);

    fclose(fp);
    free(line);
    return 1;
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
    while (t != NULL) {
        tokens[num_tokens] = t;
        num_tokens++;
        t = strtok(NULL, " ");
    }
    tokens[num_tokens] = NULL;   

    return num_tokens;
}

/*
 * spawn a child process and execute a "standard" binary command
 * (not OR or PP)
 */
/*
 * code for this function adapted from example given in Ch.1 of Doeppner
 */
int exec_standard(char ** args, int num_tokens) {

    char* envp[] = {0};
    short pid;
    if ( (pid = fork()) == 0) {
        args[0] = _binary_fullpath;
        args[num_tokens] = 0;

        if( execve(args[0], args, envp) == -1) {
            fprintf(stderr, "Failed to execute %s\n", args[0]);
            exit(0);
        }
    }
    else {
        int returnCode;
        while (pid != wait(&returnCode));
    }

    return 1;
}

/*
 * first do input validation
 * then manipulate tokens as needed to find filename & command (with args)
 * then spawn child process
 * then execute command
 */
/*
 * code adapted from appendix_c.c
 */
int exec_or(char ** args, int num_tokens) {
    int i;
    for (i=0; i<num_tokens; i++) {
        if (strcmp(args[i], "->") == 0) {
            char* filename = args[i+1];
            if (filename == NULL) {
                fprintf(stderr, "Error: must specify target file for output redirection.\n" );
                return 0;
            }
            int fd;
            fd = open(filename, O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR);
            if (fd == -1) {
                fprintf(stderr, "Error: could not open file %s\n", filename);
                return 0;
            }

            /* manipulate args as necessary to pass to execve() */
            int j;
            for (j=0; j<i-1; j++) {
                args[j] = args[j+1];
            }
            for (j=j; j<num_tokens; j++) {
                args[j] = 0;                
            }


            short pid;
            if ( (pid = fork()) == 0) {
                args[0] = _binary_fullpath;
                args[num_tokens] = 0;
                char* envp[] = {0};

                dup2(fd, 1);
                dup2(fd, 2);

                if( execve(args[0], args, envp) == -1) {
                    fprintf(stderr, "Error: failed to execute command %s\n", args[1]);
                    return 0;
                }
            }
            else {
                int returnCode;
                while (pid != wait(&returnCode));
            }
            return 1;    
        }
    }
    fprintf(stderr, "Invalid command. Must include '->'\n" );
    return 0;
}

/*
 * first do input validation
 * then manipulate tokens as needed to break down commands
 * then spawn child process
 * then execute command(s)
 */
/*
 * code adapted from appendix_d.c
 */
int exec_pipe(char ** args, int num_tokens) {
    char* commands[3][MAX_NUM_ARGS];
    int i;
    int arrows_index[3];
    int num_arrows = 0;

    // do second check (probably unnecessary) that first token is "PP"
    // if yes, remove it (no longer needed)
    // if not, error message
    if (strcmp(args[0], "PP") == 0)
    {
        for (i=0; i < num_tokens; i++) {
            args[i] = args[i+1];
        }
        args[num_tokens-1] = 0;
        --num_tokens;
    }
    else {
        fprintf(stderr, "Error: input does not start with 'PP'. This is not a pipe operation.\n");
        return 0;
    }

    // now count how many piping operations there will be
    for (i = 0; i < num_tokens; i++) {
        if (strcmp(args[i], "->") == 0) {
            arrows_index[num_arrows] = i;
            ++num_arrows;
        }
    }
    if (num_arrows == 0) {
        printf("Error: missing '->' symbol.\n");
        return 0;
    }
    // now separate each command to be executed into 2D array commands[][]
    int j = 0;
    int k = 0;
    int m = 0;
    for (i=0; i < num_tokens; i++) {
        if (i != arrows_index[j]) {
            commands[k][m] = args[i];
            m++;
        }
        else if(i == arrows_index[j]) {
            commands[k][m] = 0;
            m = 0;
            j++;
            k++;
        }
    }
    commands[k][m] = 0; // null-terminate final command[]

    if (k - 1 < num_arrows) {
        fprintf(stderr, "Error: missing command.\n" );    
    }

    for(i=0; i<3; i++) {
        printf("commands[%d]: ", i);
        for(k=0; k<3; k++) {
            printf(" %s ", commands[i][k]);
        }
        printf("\n");
    }

    // spawn child process(es) - one child for each command

    return 1;
}

int main(int argc, char *argv[]) {
    char input[MAX_INPUT_LINE];
    int  line_len;

    getCommandPrompt();

    // infinite loop to accept user input
    for(;;) {
        fprintf(stdout, "%s ", _prompt);
        fflush(stdout);
        fgets(input, MAX_INPUT_LINE, stdin);
        if (input[strlen(input) - 1] == '\n') {
            input[strlen(input) - 1] = '\0';
        }

        char *token[MAX_NUM_ARGS+2]; // +2 is to account for max possible number of '->'s included in input
        int num_tokens = tokenizeInput(token, input);

        if (strcmp(input, "exit") == 0) {
            exit(0);
        }
        else {
            if (strcmp(token[0], "OR") == 0) {
                if (findBinaryForCommand(token[1]) > 0) {
                    exec_or(token, num_tokens);
                }
                else {
                    fprintf(stderr, "Error: command '%s' not found.\n", token[1]);
                }
            }
            else if (strcmp(token[0], "PP") == 0) {
                if (findBinaryForCommand(token[1]) > 0) {
                    exec_pipe(token, num_tokens);
                }
                else {
                    fprintf(stderr, "Error: command '%s' not found.\n", token[1]);
                }
            }
            else if (findBinaryForCommand(token[0]) > 0) { /************** NEED BETTER WAY TO PASS BINARY NAME ************/
                //printf("Binary path: %s\n", _binary_fullpath);
                //else if (!strcmp(token[0], "PP")) {
                   // exec_pipe(token, num_tokens);
                //}
                exec_standard(token, num_tokens);
            } 
            else {
                fprintf(stderr, "Error: command '%s' not found.\n", input);
            }
        }
    }
}