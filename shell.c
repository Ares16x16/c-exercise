//wsl2
//process creation and execution-foreground, use of signals, buildt-in command timeX and exit, , no bonus part


#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/resource.h>

//declare functions
int exitF(char **args);
int timeX(char **args);

//builtin command
char *builtin_string[] = {
  "exit",
  "timeX"
};
int (*builtin_function[]) (char **) = {
  &exitF,
  &timeX
};

int builtIns_no() {
  return sizeof(builtin_string) / sizeof(char *);
}


int exitF(char **args)  //exit the program
{
    if  (strcmp(*args,"exit") == 0 && args[1] == NULL){ //compare the string
        printf("%s\n", "3230shell: Terminated");
        return 0;
  }else{
        printf("3230shell: ""exit"" with other arguments!!!\n");
        return 1;
  }
}

//launch the program by creating process and wait for it to terminate
int launch(char **args, char *check)
{
  pid_t pid1;
  int status, fd[2];

  if (check != "from_timeX"){

      pid1 = fork();

      if (pid1 == 0) {  //child
          if (execvp(args[0], args) == -1) {
            printf("3230shell: '%s' : No such file or directory\n", (char*)args[0]);
          }
          exit(EXIT_FAILURE);
      } else if (pid1 < 0) {  //check error
        printf("3230shell: '%s' : No such file or directory\n", (char*)args[0]);
      } else {
        do { //parent
            waitpid(pid1, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
      }
      return 1;
  } else {
    //From timeX, same as above but passed from timeX
    struct rusage usageBefore, usageAfter;
    //printf("%s",args[1]);
    getrusage(RUSAGE_SELF,&usageBefore);

    pid1 = fork();

    if (pid1 == 0) {
        if (execvp(args[0], args) == -1) {
          printf("3230shell: '%s' : No such file or directory\n", (char*)args[0]);
        }
        exit(EXIT_FAILURE);
    } else if (pid1 < 0) {
      printf("3230shell: '%s' : No such file or directory\n", (char*)args[0]);
    } else {
      do {
           waitpid(pid1, &status, WUNTRACED);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));
     }

    getrusage(RUSAGE_SELF,&usageAfter);

    printf("\n(PID)%d  ", pid1);
    printf("(CMD)%s  ", args[0]);
    printf("(user)%.3f s  ", (usageAfter.ru_utime.tv_sec - usageBefore.ru_utime.tv_sec) + 1e-6*(usageAfter.ru_utime.tv_usec - usageBefore.ru_utime.tv_usec));
    printf("(sys)%.3f s  \n", (usageAfter.ru_stime.tv_sec - usageBefore.ru_stime.tv_sec) + 1e-6*(usageAfter.ru_stime.tv_usec - usageBefore.ru_stime.tv_usec));
    return 1;
  }
  return 1;
}


int execute(char **args, char *check)
{
  int i;

  if (args[0] == NULL) {  //return if command is empty
    return 1;
  }

  for (i = 0; i < builtIns_no(); i++) {
    if (strcmp(args[0], builtin_string[i]) == 0) {
      return (*builtin_function[i])(args);
    }
  }

  return launch(args, check); //go to launch
}

int timeX(char **args)
{
  //check if the command is correct
  if(strcmp(*args,"timeX") == 0 && args[1] == NULL){
    printf("3230shell: \"timeX\" cannot be a standalone command\n"); 
  }else{
    //move the command pointer to pointer char one step forward
    char *temp;
    if (args[2] == NULL){
       args[0] = args[1];
       args[1] = NULL;
    }else if(args[2] != NULL && args[3] == NULL){
       args[0] = args[1];
       args[1] = args[2]; 
       args[2] = NULL;
    }else if(args[2] != NULL && args[3] != NULL && args[4] == NULL){
       args[0] = args[1];
       args[1] = args[2];
       args[2] = args[3];  
       args[3] = NULL;
    }else if(args[2] != NULL && args[3] != NULL && args[4] != NULL && args[5] == NULL){
       args[0] = args[1];
       args[1] = args[2];
       args[2] = args[3];
       args[3] = args[4];   
       args[4] = NULL;
    }
    
    execute(args, "from_timeX");
  }
  return 1;
}

char *readLine(void)
{
#ifdef GETLINE
  char *l = NULL;
  //allocate buffer
  ssize_t bufsize = 0; 
  if (getline(&l, &bufsize, stdin) == -1) {
    if (feof(stdin)) {  //end of file
      exit(EXIT_SUCCESS); 
    } else  {
      perror("3230shell: getline\n");
      exit(EXIT_FAILURE);
    }
  }
  return l;

#else
#define RBUFSIZE 1024
  int bufsize = RBUFSIZE;
  int pos = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    exit(EXIT_FAILURE);
  }

  while (1) {
    //read char
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[pos] = '\0';
      return buffer;
    } else {
      buffer[pos] = c;
    }
    pos++;

    if (pos >= bufsize) {     //if the buffer is over, reallocate it again
      bufsize += RBUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        exit(EXIT_FAILURE);
      }
    }
  }
#endif
}

#define TOKEN_BUF 1024
#define TOKEN_DELIM " \t\r\n\a"

char **splitLine(char *line) //split the commands
{
  int bufsize = TOKEN_BUF, pos = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokenTemp;

  if (!tokens) {
    exit(EXIT_FAILURE);
  }

  token = strtok(line, TOKEN_DELIM);
  while (token != NULL) {
    tokens[pos] = token;
    pos++;

    if (pos >= bufsize) {
      bufsize += TOKEN_BUF;
      tokenTemp = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokenTemp);
        exit(EXIT_FAILURE);
      }
    }
    token = strtok(NULL, TOKEN_DELIM);
  }
  tokens[pos] = NULL;
  return tokens;
}

void sigint_handler(int signum) { //handle sigint
   signal(SIGINT, sigint_handler);
   fflush(stdout);
}

void mainLoop(void)  //loop of the whole program
{
  char *line;
  char **args;
  int status;

  do {
    printf("$$ 3230shell ## ");
    signal(SIGINT, sigint_handler);
    line = readLine();
    args = splitLine(line);
    status = execute(args,"");

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv)
{
  mainLoop();
  return EXIT_SUCCESS;
}