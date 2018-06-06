/*
 * sh360plus.c
 * CSC 360, Summer 2018
 * Andrea Williams 
 */

/*
 * Bonus features implemented:
 * (1) cd: To change directories, input "cd <arg1>" as in standard Unix terminal
 * (2) update: To update the command prompt and/or binary directories without having to restart the shell,
 *             input "update"
 * (3) ORAPP: To perform output redirection, but append to the end of the specified file, rather than overwriting it (as OR does),
 *             input "ORAPP <command> <args> -> <dest-file>"
 * (4) HELP: To read helpfile for sh360plus-specific commands, input "HELP"
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdbool.h>

/**************************** CONSTANTS ***********************************/
#define MAX_NUM_ARGS 7
#define MAX_INPUT_LINE 80
#define MAX_LEN_PROMPT 10
#define MAX_NUM_DIRS 10
#define CONFIG_FILENAME "./.sh360rc"
#define MAX_LEN_DIR_NAME 200 //chosen arbitrarily

/**************************** GLOBAL VARIABLES ******************************/
int _num_dirs = 0; // number of directories listed in config file; i.e. number of entries in _dirs[]
char _dirs[MAX_NUM_DIRS][MAX_LEN_DIR_NAME]; // array of directories listed in config file
char _prompt[MAX_LEN_PROMPT]; // string to be displayed for each command prompt
const char* _helpfile = "\n***********\nsh360plus help:\ncd: To change directories, input \'cd <arg1>\' as in standard Unix terminal\nHELP: To read helpfile for sh360plus-specific commands, input \'HELP\'\nORAPP: To perform output redirection, but append to the end of the specified file, rather than overwriting it (as OR does), input \'ORAPP <command> <args> -> <dest-file>\'\nupdate: To update the command prompt and/or binary directories without having to restart the shell, input \'update\'.\n***********";

/**************************** FUNCTION PROTOTYPES **************************/
void getCommandDirectories(FILE * fp);
int check_for_file(char* filename, int permis);
int findBinaryForCommand(char* binary_name, char* binary_fullpath, int buffersize);
int getCommandPrompt();
int tokenizeInput(char** tokens, char* input);
int exec_standard(char ** args, int num_tokens, char *binary_fullpath);
int exec_or(char ** args, int num_tokens, char* binary_fullpath, int cat);
int exec_pipe_2(char ** command_head, char* binary_head, char** command_tail, char* binary_tail);
int exec_pipe_3(char** command_head, char* binary_head, char** command_middle, char* binary_middle, char** command_tail, char* binary_tail);
int count_arrows(char* token[], int num_tokens);
int prep_cmd_for_piping(char* token[], int num_tokens, char* commands[][MAX_NUM_ARGS+1], int num_arrows);
void run_command(char* token[], int num_tokens);
int exec_cd(char ** args, int num_tokens);
int update_config();
void get_help();

/*
 * reads remaining lines (other than first line) of config file
 * to get list of directories in which binary commands are stored
 * @param: fp = pointer to FILE CONFIG_FILENAME
 * sets global variables _num_dirs and _dirs[]
 */
void getCommandDirectories(FILE * fp) {
    while( fgets(_dirs[_num_dirs], MAX_LEN_DIR_NAME, fp) && (_num_dirs < MAX_NUM_DIRS) ) {
        _num_dirs++;
    }
    int i;
    for (i = 0; i < _num_dirs; i++)
    {
        if (_dirs[i][strlen(_dirs[i])-1]) {
            _dirs[i][strlen(_dirs[i])-1] = 0; // remove newline character if it exists
        }
    }
}

int update_config() {
    if (getCommandPrompt() == 0) {
        return 1;
    }
    else return 0;
}

void get_help() {
    fprintf(stdout, "%s\n", _helpfile);
}

/*
 * int permis: if == 1, check for permission to execute
 * if permis == 2, check for permission to write
 * 
 * code adapted from appendix_f.c
 */
int check_for_file(char *filename, int permis) {
    struct stat file_stat;

    if (stat(filename, &file_stat) != 0) {
        fprintf(stderr, "Error: '%s' either does not exist or cannot access file/directory.\n", filename);
        return 0;
    } else {
        if (permis == 1) {
            if (!(file_stat.st_mode & S_IXOTH)) {
                fprintf(stderr, "Error: do not have execute permissions on file '%s'\n",  filename);
                return 0;
            }
            return 1;    
        }
        else if (permis == 2) {
            if (!file_stat.st_mode & S_IWOTH) {
                fprintf(stderr, "Error: do not have write permissions on file '%s'\n",  filename);
                return 0;
            }
            return 1;
        }
    }

}

/*
 * searches through directores in _dirs[] looking for binary matching
 * @param binary_name
 * returns 1 if successful, 0 if can't be found
 *
 * string-passing code adapted from accepted answer to SO question
 * https://stackoverflow.com/questions/1496313/returning-c-string-from-a-function
 */
int findBinaryForCommand(char* binary_name, char* binary_fullpath, int buffersize) {
    FILE * fp = fopen(binary_name, "r");
    if (fp)
    {
        strncpy(binary_fullpath, binary_name, buffersize-1);
        binary_fullpath[buffersize-1] = '\0';
        fclose(fp);
        return 1;
    }

    // if binary_name hasn't supplied full path
    int i;
    char test_path[MAX_LEN_DIR_NAME];
    for (i = 0; i < _num_dirs; i++)
    {
        test_path[0] = '\0';
        strcat(test_path, _dirs[i]);
        strcat(test_path, "/");
        strcat(test_path, binary_name);
        strcat(test_path, "\0");

        fp = fopen(test_path, "r");
        if (fp) {
            strncpy(binary_fullpath, test_path, buffersize-1);
            binary_fullpath[buffersize-1] = '\0';
            fclose(fp);
            return 1;
        }  
    }
    return 0;
}

/*
 * reads first line of .sh360rc file to get string 
 * to be used as command prompt
 *
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
        fprintf(stderr, "Error: cannot open file %s. Exiting...\n", CONFIG_FILENAME);
        exit(EXIT_FAILURE);
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
 *
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
 *
 * code for this function adapted from example given in appendix_b.c
 */
int exec_standard(char ** args, int num_tokens, char *binary_fullpath) {

    char* envp[] = {0};
    int pid;
    int status;
    if ( (pid = fork()) == 0) {
        args[0] = binary_fullpath;
        args[num_tokens] = 0;

        if (check_for_file(args[0], 1) == 0) {
            exit(EXIT_FAILURE);
        }

        if( execve(args[0], args, envp) == -1) {
            fprintf(stderr, "Failed to execute %s\n", args[0]);
            //return 0;
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    else {
        while (wait(&status) > 0) {
            return 1;
        }
    }
}

int exec_cd(char ** args, int num_tokens) {
    if (num_tokens == 2) {
        if (chdir(args[1]) == 0) {
            return 1;
        }
        else {
            fprintf(stderr, "Error: directory '%s' does not exist.\n", args[1]);
            return 0;
        }
    }
    else {
        fprintf(stderr, "Error: could not parse args to 'cd'.\n" );
        return 0;
    }
}

/*
 * first do input validation
 * then manipulate tokens as needed to find filename & command (with args)
 * then spawn child process
 * then execute command
 *
 * code adapted from appendix_c.c
 */

int exec_or(char ** args, int num_tokens, char* binary_fullpath, int cat) {
    int i;
    int status;
    for (i=0; i<num_tokens; i++) {
        if (strcmp(args[i], "->") == 0) {
            char* filename = args[i+1];
            if (filename == NULL) {
                fprintf(stderr, "Error: must specify target file for output redirection.\n" );
                return 0;
            }
            int fd;
            if (cat == 0) {
                fd = open(filename, O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR);
            }
            else {
                fd = open(filename, O_CREAT|O_RDWR|O_APPEND, S_IRUSR|S_IWUSR);
            }
            if (fd == -1) {
                fprintf(stderr, "Error: could not open file %s\n", filename);
                return 0;
            }
            if (check_for_file(filename, 2) == 0) {
                return 0;
            }

            // manipulate args as necessary to pass to execve() 
            int j;
            for (j=0; j<i-1; j++) {
                args[j] = args[j+1];
            }
            for (j=j; j<num_tokens; j++) {
                args[j] = 0;                
            }


            short pid;
            if ( (pid = fork()) == 0) {
                args[0] = binary_fullpath;
                args[num_tokens] = 0;
                char* envp[] = {0};

                dup2(fd, 1);
                dup2(fd, 2);

                if (check_for_file(args[0], 1) == 0) {
                    exit(EXIT_FAILURE);
                }
                if( execve(args[0], args, envp) == -1) {
                    fprintf(stderr, "Error: failed to execute command %s\n", args[1]);
                    exit(EXIT_FAILURE);
                }
                exit(EXIT_SUCCESS);
            }
            else {
                while (wait(&status) > 0) {
                    return 1;
                }
            }    
        }
    }
    fprintf(stderr, "Invalid command. Must include '->'\n" );
    return 0;
}


/*
 * performs 2-command (one pipe) piping 
 *
 * first do input validation
 * then manipulate tokens as needed to break down commands
 * then spawn child process
 * then execute command(s)
 *
 * code adapted from appendix_d.c
 */
int exec_pipe_2(char ** command_head, char* binary_head, char** command_tail, char* binary_tail) {
    // spawn child process(es) - one child for each command
    int status;
    int fd1[2];
    int pid_1, pid_2;
    char* envp[] = {0};
    command_head[0] = binary_head;
    command_tail[0] = binary_tail;

    if ((check_for_file(command_head[0], 1) == 0) || (check_for_file(command_tail[0], 1) == 0)) {
        return 0;
    }

    pipe(fd1);

    if((pid_1 = fork()) == 0) {
        dup2(fd1[1], 1);
        close(fd1[0]);
        if (execve(command_head[0], command_head, envp) == -1) {
            fprintf(stderr, "Error: failed to execute %s\n", *command_head);
            close(fd1[1]);
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS); 
    }
    if ((pid_2 = fork()) == 0) {
        dup2(fd1[0], 0);
        close(fd1[1]);
        if (execve(command_tail[0], command_tail, envp) == -1) {
            fprintf(stderr, "Error: failed to execute %s\n", *command_tail);
            close(fd1[0]);
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }

    close(fd1[0]);
    close(fd1[1]);

    waitpid(pid_1, &status, 0);
    waitpid(pid_2, &status, 0);

    return 1;
} 

/*
 * performs 3-command (2 pipes) piping
 *
 * code adapted from https://gist.github.com/mplewis/5279108
 */
int exec_pipe_3(char** command_head, char* binary_head, char** command_middle, char* binary_middle, char** command_tail, char* binary_tail) {
    int pid_1, pid_2, pid_3;
    int status;
    int pipe1[2], pipe2[2];
    char* envp[] = {0};

    if ( (check_for_file(binary_head, 1) == 0) || (check_for_file(binary_middle, 1) == 0) || (check_for_file(binary_tail, 1) == 0) ) {
        exit(EXIT_FAILURE);
    }

    pipe(pipe1);

    if ((pid_1 = fork()) == 0) {
        dup2(pipe1[1], 1);

        close(pipe1[0]);

        command_head[0] = binary_head;

        if (execve(command_head[0], command_head, envp) == -1) {
            fprintf(stderr, "Error: failed to execute first command\n" );
            close(pipe1[1]);
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }

    pipe(pipe2);

    if ((pid_2 = fork()) == 0) {
        dup2(pipe1[0], 0);
        dup2(pipe2[1], 1);
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);

        command_middle[0] = binary_middle;

        if (execve(command_middle[0], command_middle, envp) == -1) {
            fprintf(stderr, "Error: failed to execute second command\n" );
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }

    close(pipe1[0]);
    close(pipe1[1]);

    if ((pid_3 = fork()) == 0) {
        dup2(pipe2[0], 0);
        close(pipe2[0]);
        close(pipe2[1]);

        command_tail[0] = binary_tail;

        if (execve(command_tail[0], command_tail, envp) == -1) {
            fprintf(stderr, "Error: failed to execute third command.\n" );
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }

    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);

    waitpid(pid_1, &status, 0);
    waitpid(pid_2, &status, 0);
    waitpid(pid_3, &status, 0);

    return 1;
}

int count_arrows(char* token[], int num_tokens) {
    int num_arrows = 0;
    int i;
    for (i=0; i < num_tokens; i++) {
        if (strcmp(token[i], "->") == 0) {
            num_arrows++;
        }
    }
    return num_arrows;
}  

/*
 * helper function for piping:
 * organize token[] array into 2D matrix of command strings, where each row of matrix is command and args
 * representing one step of the piping operation
 */
int prep_cmd_for_piping(char* token[], int num_tokens, char* commands[][MAX_NUM_ARGS+1], int num_arrows) {
    int arrows_index[num_arrows+1];
    int i;
    int a;

    // remove 'PP' - shift all tokens 1 left
    for (i=0; i < num_tokens; i++) {
        token[i] = token[i+1];
    }
    token[num_tokens-1] = 0;
    --num_tokens;

    // collect indices of endpoint for each command
    a = 0;
    for (i = 0; i < num_tokens; i++) {
        if (strcmp(token[i], "->") == 0) {
            arrows_index[a] = i;
            a++;
        }
    }
    arrows_index[num_arrows] = num_tokens;

    int j;
    int k;
    i = 0;
    for (j=0; j < num_arrows+1; j++) {
        k = 0;
        while (i < arrows_index[j]) {
            commands[j][k] = token[i];
            i++;
            k++;
        }
        commands[j][k] = '\0';
        i++;
    }

    return 1;
}

/*
 * parses first token in input to determine which function should be called
 */
void run_command(char* token[], int num_tokens) {
    if (strcmp(token[0], "cd") == 0) {
        exec_cd(token, num_tokens);
    }
    else if (strcmp(token[0], "update") == 0) {
        if (update_config() == 0) {
            fprintf(stdout, "Configurations from '%s' updated successfully.\n", CONFIG_FILENAME);
        }
        else {
            fprintf(stderr, "Error: failed to update configuration from '%s'.\n", CONFIG_FILENAME);
        }
    }
    else if (strcmp(token[0], "HELP") == 0) {
        get_help();
    }
    else if (strcmp(token[0], "OR") == 0) {
        char binary_fullpath[MAX_LEN_DIR_NAME] = {0};
        if (findBinaryForCommand(token[1], binary_fullpath, MAX_LEN_DIR_NAME) != 0) {
            exec_or(token, num_tokens, binary_fullpath, 0);
        }
        else {
            fprintf(stderr, "Error: command '%s' not found.\n", token[1]);
            return;
        }
    }
    else if (strcmp(token[0], "ORAPP") == 0) {
        char binary_fullpath[MAX_LEN_DIR_NAME] = {0};
        if (findBinaryForCommand(token[1], binary_fullpath, MAX_LEN_DIR_NAME) != 0) {
            exec_or(token, num_tokens, binary_fullpath, 1);
        }
        else {
            fprintf(stderr, "Error: command '%s' not found.\n", token[1]);
            return;
        }        
    }
    else if (strcmp(token[0], "PP") == 0) {
        int num_arrows = count_arrows(token, num_tokens);
        if (num_arrows == 0) {
            fprintf(stderr, "Error: missing '->' symbol.\n");
            return;
        }

        char* commands[num_arrows+1][MAX_NUM_ARGS+1];
        prep_cmd_for_piping(token, num_tokens, commands, num_arrows);
        
        int i;
        char binary_fullpaths[3][MAX_LEN_DIR_NAME];
        for (i=0; i<num_arrows+1; i++) {
            if (findBinaryForCommand(commands[i][0], binary_fullpaths[i], MAX_LEN_DIR_NAME) == 0) {
                    fprintf(stderr, "Error: command '%s' not found.\n", commands[i][0]);
                    return;
            }
        }
        if (num_arrows == 1) {
            exec_pipe_2(commands[0], binary_fullpaths[0], commands[1], binary_fullpaths[1]);
        }
        else {  //num_arrows must == 2
            exec_pipe_3(commands[0], binary_fullpaths[0], commands[1], binary_fullpaths[1], commands[2], binary_fullpaths[2]);
        }
    }
    else {
        char binary_fullpath[MAX_LEN_DIR_NAME] = {0};
        if (findBinaryForCommand(token[0], binary_fullpath, MAX_LEN_DIR_NAME) != 0) {
            exec_standard(token, num_tokens, binary_fullpath);
        }
        else {
            fprintf(stderr, "Error: command '%s' not found.\n", token[0]);
            return;
        }
    } 
} 

int main(int argc, char *argv[]) {
    char input[MAX_INPUT_LINE];
    int  line_len;

    getCommandPrompt();
    get_help();

    // infinite loop to accept user input
    for(;;) {
        fprintf(stdout, "%s ", _prompt);
        fflush(stdout);
        fgets(input, MAX_INPUT_LINE, stdin);
        if (strlen(input) < 2) {
		    continue;
	    }
	    if (input[strlen(input) - 1] == '\n') {
            input[strlen(input) - 1] = '\0';
        }
        if (strcmp(input, "exit") == 0) {
            printf("Exiting...\n");
            exit(EXIT_SUCCESS);
        }

        char *token[30]; // 30 is approx. max number of tokens that could be input at once
        int num_tokens = tokenizeInput(token, input);
	    if (num_tokens > 0) {        
            run_command(token, num_tokens);   
        }
    }
}