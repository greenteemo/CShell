/*
	to do list:
	1. how to paste outside content to vim
	2. how to use ctrl+D to select same block
	3.* figur out how can execpv execute system command
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define READ_LINE_BUFSIZE 1024
#define TOKEN_BUFSIZE 64
#define CURPATH_MAXSIZE 80		// ref: 

/*
	Function declarations for builtin shell commands
*/
int cd(char **args);
int help(char **args);
int _exit_(char **args);	// because name confict, use _exit_ instead of exit
int pwd(char **args);


/*
	Builtin commands
*/
char *builtin_str[] = {
 	"cd",
	"help",
	"exit",
	"pwd"
};

int (*builtin_func[]) (char **) = {
	&cd,
	&help,
	&_exit_,
	&pwd
};

int num_builtin(){
	return sizeof(builtin_str) / sizeof(char *);
}

/*
	Builtin function implementations
*/
int cd(char **args){
	if(args[1] == NULL){
		fprintf(stderr, "CShell: expected argument to \"cd\"\n");
	}else{
		if(chdir(args[1]) != 0){
			perror("CShell");
		}
	}
	return 1;
}

int help(char **args){
	printf("*CShell\n");
	printf("*Type program names and argusments, then hit enter.\n");
	printf("*The following are built in:\n");

	for(int i = 0; i < num_builtin(); i++){
		printf(" %s\n", builtin_str[i]);
	}

	return 1;
}

int _exit_(char **args){
	return 0;
}

int pwd(char **args){
	char *cur_path = (char *)malloc(sizeof(char*) * CURPATH_MAXSIZE);
	getcwd(cur_path, CURPATH_MAXSIZE);
	printf("%s\n", cur_path);
	free(cur_path);
	return 1;
}


int launch(char **args){
    pid_t pid, wpid;
	int status;
	
	pid = fork();
	if(pid == 0){
		// child process
		if(execvp(args[0], args) == -1){
			perror("CShell");
		}
		exit(EXIT_FAILURE);
	}else if(pid < 0){
		// error
		perror("CShell");
	}else{
		// parent process
		do{
			wpid = waitpid(pid, &status, WUNTRACED);
		}while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}

int execute(char **args){
	if(args[0] == NULL){
		// empty command
		return 1;
	}

	for(int i = 0; i < num_builtin(); i++){
		if(strcmp(args[0], builtin_str[i]) == 0){
			return (*builtin_func[i])(args);
		}
	}

	printf("CShell: Not exist command \"%s\"\n", args[0]);

	return 1;
}

char **split_line(char *line){
    int bufsize = TOKEN_BUFSIZE, position = 0;
    char **tokens = (char**)malloc(sizeof(char*) * bufsize);
    char *token;

    if(!tokens){
        fprintf(stderr, "CShell: allocation error --happend in char **split_line(char *line)\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, " ");
    while(token != NULL){
        tokens[position++] = token;

        if(position >= bufsize){
            bufsize += TOKEN_BUFSIZE;
            tokens = realloc(tokens, sizeof(char*) * bufsize);
            if(!tokens){
                fprintf(stderr, "CShell: allocation error --happend in char **split_line(char *line)\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, " ");
    }
    tokens[position] = NULL;

    return tokens;
}

char *read_line(){
    int bufsize = READ_LINE_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if(!buffer){
        fprintf(stderr, "CShell: allocation error --happend in char *read_line()\n");      // ??? why use fprintf
        exit(EXIT_FAILURE);
    }

    while(1){
        // read a char
        c = getchar();

        // hit end
        if(c == EOF || c == '\n'){
            buffer[position] = '\0';
            return buffer;
        }else{
            buffer[position++] = c;
        }

        if(position >= bufsize){
            bufsize += READ_LINE_BUFSIZE;
            buffer = realloc(buffer, sizeof(char) * bufsize);

            if(!buffer){
                fprintf(stderr, "CShell: allocation error --happend in char *read_line()\n");      // ??? why use fprintf
                exit(EXIT_FAILURE);
            }
        }
    }
}

void shell_loop(){
    char *line;
    char **args;
	char *cur_path;
    int status = 1;

    do{
		cur_path = (char *)malloc(sizeof(char*) * CURPATH_MAXSIZE);
		getcwd(cur_path, CURPATH_MAXSIZE);
        printf("%s@CShell:%s> ", "user", cur_path);
        line = read_line();
        // printf("line: ");
        // printf(line);
        // putchar('\n');
        
        args = split_line(line);
        // for(int i = 0; args && args[i]; i++){
        //     printf("arg%d: ", i);
        //     printf(args[i]);
        //     putchar('\n');
        // }

        status = execute(args);

        free(line);
        free(args);
		free(cur_path);
    }while(status);    
}

int main(){
    // Load config file, if any

    // Run command loop
    shell_loop();

    // Perform any shutdown/cleanup

    return EXIT_SUCCESS;
}
