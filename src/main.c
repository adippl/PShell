#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int BUFFER = 512;
const char *CLEAR_SCREEN = " \e[1;1H\e[2J";
const char *BASE_PATH = "/usr/bin/";

char* readLine(){
  char c;
  char *line = calloc(BUFFER,sizeof(char));
  int i = 0;

  while((c = getchar()) != '\n'){
    line[i] = c;
    i++;
  }
  return line;
}

int main() {
  char *line;
  char **splitted_line;
  int child_id;
  int status;
  char cd[512];

  write(STDOUT_FILENO,CLEAR_SCREEN,strlen(CLEAR_SCREEN));
  while (1){
    char* current_dir = getcwd(cd,sizeof(cd));
    printf("%s > ",getLastTwoDirs(current_dir));

    line = readLine();
    splitted_line = splitString(line,' ');
    if(strcmp(splitted_line[0],"q") == 0){
      break;
    }

    pid_t pid = fork();
    if (pid == 0){
      int error = execlp(splitted_line[0],splitted_line[1],NULL);
      printf("%d\n",error);
    } else {
      waitpid(pid,NULL,0);
    }
  free(splitted_line);
  free(line);
  }
}

