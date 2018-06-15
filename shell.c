#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)
#define HISTORY_DEPTH 10
char history[HISTORY_DEPTH][COMMAND_LENGTH];
char playBuffer[COMMAND_LENGTH];//so that I can mess with a global buffer
int curr = 0;
int commands = 0;
_Bool skip = false; // boolean to skip sections
//stuff for the history
void killsignal(_Bool bg){
  if(bg){
    strcat(history[curr]," ");
    strcat(history[curr],"&");
  }
  commands++;
  curr = (curr+1)%HISTORY_DEPTH;
}
void putInHistory(const char* str, int cat){
    if(cat == 0){
      strcpy(history[curr],str);

      //printf("hello hi\n" );
      //printf("%s\n", history[curr]);
      //printf("%d\n",curr );
    }
    else {
      //printf("got here\n" );
      //printf("%s\n",history[curr] );

      strcat(history[curr]," ");
      strcat(history[curr],str);
      //printf("%s\n",history[curr] );
    }
  }
void printHistory(){
  char str[12];
  int num = 1;
  int i = (curr);//+1)%HISTORY_DEPTH;
//  printf("curr is now at %d\n",curr );
//  printf("commands is now at %d\n",commands );
  if (commands > 10)
    num = num + commands-10;
  do{
    if(strcmp(history[i],"")!=0){
      //printf(" i is now at %d\n",i );
      sprintf(str,"%d",num);
      write(STDOUT_FILENO, str, strlen(str));
      write(STDOUT_FILENO, "\t", strlen("\t"));
      write(STDOUT_FILENO, history[i], strlen(history[i]));
      write(STDOUT_FILENO,"\n", strlen("\n"));
      num++;
    }
    i = (i+1)%HISTORY_DEPTH;
  }while(i!= curr);//(curr+1)%HISTORY_DEPTH);
}
void handle_SIGINT(){
  write(STDOUT_FILENO,"\n", strlen("\n"));
  printHistory();
  skip = true;

}
/**
 * Command Input and Processing
 */

/*
 * Tokenize the string in 'buff' into 'tokens'.
 * buff: Character array containing string to tokenize.
 *       Will be modified: all whitespace replaced with '\0'
 * tokens: array of pointers of size at least COMMAND_LENGTH/2 + 1.
 *       Will be modified so tokens[i] points to the i'th token
 *       in the string buff. All returned tokens will be non-empty.
 *       NOTE: pointers in tokens[] will all point into buff!
 *       Ends with a null pointer.
 * returns: number of tokens.
 */
int tokenize_command(char *buff, char *tokens[])
{
	int token_count = 0;
	_Bool in_token = false;
	int num_chars = strnlen(buff, COMMAND_LENGTH);
  if(num_chars == 0){
    //printf("got here \n" );
    return 0;
  }
  for (int i = 0; i < num_chars; i++) {
		switch (buff[i]) {
		// Handle token delimiters (ends):
		case ' ':
		case '\t':
		case '\n':
			buff[i] = '\0';
			in_token = false;
			break;

		// Handle other characters (may be start)
		default:
			if (!in_token) {
				tokens[token_count] = &buff[i];
				token_count++;
				in_token = true;
			}
		}
	}
	tokens[token_count] = NULL;
	return token_count;
}

/**
 * Read a command from the keyboard into the buffer 'buff' and tokenize it
 * such that 'tokens[i]' points into 'buff' to the i'th token in the command.
 * buff: Buffer allocated by the calling code. Must be at least
 *       COMMAND_LENGTH bytes long.
 * tokens[]: Array of character pointers which point into 'buff'. Must be at
 *       least NUM_TOKENS long. Will strip out up to one final '&' token.
 *       tokens will be NULL terminated (a NULL pointer indicates end of tokens).
 * in_background: pointer to a boolean variable. Set to true if user entered
 *       an & as their last token; otherwise set to false.
 */
int read_command(char *buff, char *tokens[], _Bool *in_background)
{
	*in_background = false;

	// Read input
	int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);

	if ((length < 0) && (errno !=EINTR)) {
		perror("Unable to read command from keyboard. Terminating.\n");
		exit(-1);
	}

	// Null terminate and strip \n.
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}
	// Tokenize (saving original command string)
	int token_count = tokenize_command(buff, tokens);
  if (token_count == 0) {
    //printf("got hero\n" );
		return 2;
	}

	// Extract if running in background:
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
		*in_background = true;
		tokens[token_count - 1] = 0;
	}
  char c = 33;
  if(tokens[0][0] == c){
    return 1;
  }
  return 0;
}
/* yall uhh fw INTERNAL COMMANDS????? */

int mapo(const char *str, int nothing){
  if(nothing==2){
    //write(STDOUT_FILENO, "line identified", strlen("line identified"));
    return 3;
  }
  else if(strcmp(str,"exit")==0){
    //write(STDOUT_FILENO, "exit identified", strlen("exit identified"));
    return 0;
  }
  else if(strcmp(str,"cd")==0){
    //write(STDOUT_FILENO, "cd identified", strlen("cd identified"));
    return 1;
  }
  else if(strcmp(str,"pwd")==0){
    //write(STDOUT_FILENO, "pwd identified", strlen("pwd identified"));
    return 2;
  }
  else if(strcmp(str, "history")==0){
    return 4;
  }
  return -1;
}

int getNumberFromInput(char *num){
  //first, we strip the exclamation mark
  char *numbs = num+1;
  if(strcmp(num,"!!")==0){
    return -1;
  }
  int number = atoi(numbs);
  if(number > commands || number < commands-10){
    number = 0;
  }
  //printf("%d\n",number );
  return number;
}

void getCommandFromNumber(int numbo, char* tokens[], _Bool *in_background){

  int num = 1;
  int i = (curr);//+1)%HISTORY_DEPTH;
  int token_count = 0;
  if (commands > 10){
    num = num + commands-10;
    }
  if(numbo == -1){
    numbo = commands;
  }
  do{
    if(strcmp(history[i],"")!=0){
        if(numbo == num){
          write(STDOUT_FILENO, history[i], strlen(history[i]));
          write(STDOUT_FILENO,"\n", strlen("\n"));
          strcpy(playBuffer, history[i]);
          token_count = tokenize_command(playBuffer, tokens);
          //killsignal(false);
        }//printf(" i is now at %d\n",i );
      num++;
    }
    i = (i+1)%HISTORY_DEPTH;
  }while(i!= curr);
  if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
    *in_background = true;
    tokens[token_count - 1] = 0;
  }
}

/**
 * Main and Execute Commands
 */

int main(int argc, char* argv[])
{
  struct sigaction handler;
  handler.sa_handler = handle_SIGINT;
  handler.sa_flags = 0;
  sigemptyset(&handler.sa_mask);
  sigaction(SIGINT, &handler, NULL);

	char input_buffer[COMMAND_LENGTH];
	char *tokens[NUM_TOKENS];
	while (true) {

		// Get command
		// Use write because we need to use read() to work with
		// signals, and read() is incompatible with printf().
    //making this for the line the guys on
    skip = false;
    int gotPath =1;
    char *str = (char *)malloc(100);
    if(str){
      if (getcwd(str,99) == str){
        gotPath = 0;
      }
    }
    if(gotPath == 0)
      write(STDOUT_FILENO, str, strlen(str));

		write(STDOUT_FILENO, "> ", strlen("> "));
		_Bool in_background = false;
    _Bool is_internal = true;
  //  write(STDOUT_FILENO, input_buffer, strlen(input_buffer));
    /*if(input_buffer[strlen(input_buffer)] == '\n'){
      write(STDOUT_FILENO, "Run in background.", strlen("Run in background."));

    }*/
    int nothing = read_command(input_buffer, tokens, &in_background);
    if(nothing == 1){
      int num = getNumberFromInput(tokens[0]);
      if(((num == 0) || (num == -1 && commands == 0)) && !skip){
        write(STDOUT_FILENO, "Error invalid history access (did you use the right number?)\n", strlen("Error invalid history access (did you use the right number?)\n"));
        skip = true;
  //    is_internal = true;
      }
      else if(!skip){
        getCommandFromNumber(num, tokens, &in_background);
        //if(!in_background){
          //printf("In bg is now false\n");
        //}
      }
    }
    if(!skip && nothing != 2){
      for(int q = 0; tokens[q] != NULL; q++){
        putInHistory(tokens[q],q);
        //printf("So does it exist?\n" );
      }
      killsignal(in_background);
    }
    // DEBUG: Dump out arguments:
		/*for (int i = 0; tokens[i] != NULL; i++) {
      write(STDOUT_FILENO, "   Token: ", strlen("   Token: "));
			write(STDOUT_FILENO, tokens[i], strlen(tokens[i]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
		}*/
		/*if (in_background) {
			write(STDOUT_FILENO, "Run in background.\n", strlen("Run in background.\n"));
		}*/
    if(!skip){
      switch (mapo(tokens[0],nothing)) {
        case 0://'exit':
          free(str);
          exit(0);
          /*exit function*/
          break;
        case 1://'cd':
          if(chdir(tokens[1]) != 0)
            write(STDOUT_FILENO, "Error - does that directory exist?\n", strlen("Error - does that directory exist?\n"));
          break;
        case 2://'pwd'
          write(STDOUT_FILENO, str, strlen(str));
          write(STDOUT_FILENO, "\n", strlen("\n"));
          //pwd function
          break;
        case 3://'\n':
          //write(STDOUT_FILENO, str, strlen(str));
          //write(STDOUT_FILENO, "> ", strlen("> "));
          break;
        case 4:
          printHistory();
          break;
        default:
          is_internal = false;

      }
    }
    if(!is_internal && !skip){
      pid_t pride = fork();
      int childReturn;
      if (pride < 0) {
            write(STDOUT_FILENO,"Fork This! (your fork failed)\n", strlen("Fork This! (your fork failed)\n"));
            exit(1);
       }
       else if (pride == 0) {
            if (execvp(tokens[0], tokens) < 0) {
                 write(STDOUT_FILENO,"Execution failure - is that real program?\n", strlen("Execution failure - is that real program?\n"));
                 exit(1);
            }
       }
       else if (!in_background){
        //  printf("waiting for my child to return\n" );
            while (waitpid(pride, &childReturn, 0) < 0){}
       }
     }
     //Now to free our forked brethren fellow communists
     free(str);
     while (waitpid(-1, NULL, WNOHANG) > 0)
	     ; // BE FREED FROM YOUR SHACKLES

		/**
		 * Steps For Basic Shell:
		 * 1. Fork a child process
		 * 2. Child process invokes execvp() using results in token array.
		 * 3. If in_background is false, parent waits for
		 *    child to finish. Otherwise, parent loops back to
		 *    read_command() again immediately.
		 */

	}
	return 0;
}
