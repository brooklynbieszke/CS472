#include <stdio.h>      
#include <string.h> 
#include <stdlib.h>      
#include <sys/types.h>
#include <unistd.h>      
#include <sys/wait.h>    
#include <regex.h>
#include <ctype.h>       
#define INTERACTIVE_MODE 1
#define BATCH_MODE 2
#define BUFF_SIZE 256


FILE *inputFile = NULL;
char *paths[BUFF_SIZE] = {"/bin", NULL};
char *line = NULL;

struct function_args{
    char *command;
};

void printError(){
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message)); 
}


char *trim(char *sArgs){
    while (isspace(*sArgs)){
        sArgs++;
    }
    if (*sArgs == '\0'){
        return sArgs;
    }
    char *end = sArgs + strlen(sArgs) - 1;
    while (end > sArgs && isspace(*end)){
        end--;
    }
    end[1] = '\0';
    return sArgs;
}

void clean(void){
    free(line);
    fclose(inputFile);
}

void redirect(FILE *fileOut){
    int outFileno;
    if ((outFileno = fileno(fileOut)) == -1){
        printError();
        return;
    }

    if (outFileno != STDOUT_FILENO){
        if (dup2(outFileno, STDOUT_FILENO) == -1){
            printError();
            return;
        }
        if (dup2(outFileno, STDERR_FILENO) == -1){
            printError();
            return;
        }
        fclose(fileOut);
    }
}

int searchPath(char path[], char *firstArg){
    int i = 0;
    while (paths[i] != NULL){
        snprintf(path, BUFF_SIZE, "%s/%s", paths[i], firstArg);
        if (access(path, X_OK) == 0){
            return 0;
        }
        i++;
    }
    return -1;
}

void executeCommands(char *args[], int args_num, FILE *fileOut){
    if (strcmp(args[0], "exit") == 0){
        if (args_num > 1){
            printError();
        }
        else{
            atexit(clean);
            exit(0);
        }
    }
    else if (strcmp(args[0], "cd") == 0){
        if (args_num == 1 || args_num > 2){
            printError();
        }
        else if (chdir(args[1]) == -1){
            printError();
        }
    }
    else if (strcmp(args[0], "path") == 0){
        size_t i = 0;
        paths[0] = NULL;
        for ( ; i < args_num - 1; i++){
            paths[i] = strdup(args[i+1]);
        }
        paths[i+1] = NULL;
    }
    else{
        char path[BUFF_SIZE];
        if (searchPath(path, args[0]) == 0){
            pid_t pid = fork();
            if (pid == -1){
                printError();
            }
            else if (pid == 0){
                redirect(fileOut);
                char *here = args[0];
                args[0] = strdup(path);
                if (execv(path, args) == -1){
                    printError();
                }
                free(args[0]);
                args[0] = here;
            }
            else{
                waitpid(pid, NULL, 0);
            }
        }
        else{
            printError();
        }
    }
}
void *parseInput(void *arg){
    char *args[BUFF_SIZE];
    int args_num = 0;
    FILE *output = stdout;
    struct function_args *fun_args = (struct function_args *) arg;
    char *commandLine = fun_args->command;
    char *command = strsep(&commandLine, ">");
    if (command == NULL || *command == '\0'){
        printError();
        return NULL;
    }
    command = trim(command);
    if(strncmp(command, "&", strlen(command)) == 0){
        return NULL;
    }
    if (commandLine != NULL){
        if ((output = fopen(trim(commandLine), "w")) == NULL){
            printError();
            return NULL;
        }
    }

    char **ap = args;
    while ((*ap = strsep(&command, " \t")) != NULL){
        if (**ap != '\0'){
            *ap = trim(*ap);
            ap++;
            if (++args_num >= BUFF_SIZE){
                break;
            }
        }
    }

    if (args_num > 0){
        executeCommands(args, args_num, output);
    }
    return NULL;
}
int main(int argc, char *argv[]){
    int mode = INTERACTIVE_MODE;
    inputFile = stdin;
    size_t linecap = 0;
    ssize_t nread;

    if (argc > 1){
        mode = BATCH_MODE;
        if (argc > 2 || (inputFile = fopen(argv[1], "r")) == NULL){
            printError();
            exit(1);
        }
    }

    while (1){
        if (mode == INTERACTIVE_MODE){
            printf("wish> ");
        }
        if ((nread = getline(&line, &linecap, inputFile)) > 0){
            char *command;
            int commands_num = 0;
            struct function_args args[BUFF_SIZE];

            if (line[nread - 1] == '\n'){
                line[nread - 1] = '\0';
            }
            char *here = line;
            while ((command = strsep(&here, "&")) != NULL){
                if (command[0] != '\0'){
                    args[commands_num++].command = strdup(command);
                    if (commands_num >= BUFF_SIZE){
                        break;
                    }
                }
            }
            for (int i = 0; i < commands_num -1; i++){
                int rc;
                rc = fork();
                if(rc == 0){
                    parseInput(&args[i]);
                    atexit(clean);
                    exit(0);
                }
                else if(rc < 0){
                    printError();
                    exit(1);
                }
            }
            if(commands_num != 0){
                parseInput(&args[commands_num-1]);
            }
            for(int i=0; i<commands_num-1; i++){
                wait(NULL);
            }
            for(int i=0; i<commands_num; i++){
                if(args[i].command != NULL){
                    free(args[i].command);
                }
            }
        }
        else if (feof(inputFile) != 0){
            atexit(clean);
            exit(0);
        }
    }
    return 0;
}