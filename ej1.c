#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "ej1.h"

static int pipeCtoP [2] = {-1,-1};
static int pipePtoC [2] = {-1,-1};
static int stdinFD = STDIN_FILENO;
static int stdoutFD = STDOUT_FILENO;

int bitParity(char * number) {

  int b = 1;

  b = b ^ *(number);
  b = b ^ *(number + 1);
  b = b ^ *(number + 2);
  b = b ^ *(number + 3);
  b = b ^ *(number + 4);
  b = b ^ *(number + 5);
  b = b ^ *(number + 6);
  b = b ^ *(number + 7);

  return ((~b) & 1);

}

int main (int argc, char* argv[])
{

  pid_t pid;

  if (argc < 2)
  {
    fprintf(stderr, "Too few arguments\n");
    return EXIT_FAILURE;
  }

  initializePipes();

  pid = fork();

  if (pid == -1)
  {
    perror("Fork error\n");
    exit(1);
  }
  else if (pid == 0)
  {
    childProcess(argv[1]);
  }
  else
  {
    parentProcess(pipeCtoP, pipePtoC, BUFFER_SIZE);
    wait(NULL);
  }
  return 0;

}

static void childProcess(char * command)
{
  char *args[4];

  close(STDIN_FILENO);
  close(STDOUT_FILENO);

  close(pipeCtoP[READ]);
  close(pipePtoC[WRITE]);

// redirects standard input to child's end pipe
  dup2(pipePtoC[READ], STDIN_FILENO);

// redirects parent's to standard output  pipeCtoP[1], fileno(stdout)
  dup2(pipeCtoP[WRITE], STDOUT_FILENO);

  args[0] = "bash";
  args[1] = "-c";
  args[2] = command;
  args[3] = NULL;

  execvp(args[0], args);
}

void parentProcess(int pipeCtoP[], int pipePtoC[], int bufferSize)
{
  int inputParity = 0;
  int outputParity = 0;
  char buffer[bufferSize];
  int bytesRead = 0;

  close(pipeCtoP[WRITE]);
  close(pipePtoC[READ]);

  while((bytesRead = read(STDIN_FILENO, buffer, bufferSize - 1)) > 0){
    buffer[bytesRead] = 0;
    inputParity ^= bitParity(buffer);
    write(pipePtoC[WRITE], buffer, bytesRead);
  }

  close(pipePtoC[WRITE]);

  while((bytesRead = read(pipeCtoP[READ], buffer, bufferSize - 1)) > 0){
    buffer[bytesRead] = 0;
    outputParity ^= bitParity(buffer);
    printf("%s ",buffer);
  }

  outputParity = outputParity ^ '\n';

  /* %02 imprime al menos 2 digitos, si hay menos de 2 digitos pone un 0 adelante */
  fprintf(stderr, "in parity: 0x%02X\n", inputParity);
  fprintf(stderr, "out parity: 0x%02X\n", outputParity);

}

void initializePipes()
{
  if(pipe(pipeCtoP) == -1 || pipe(pipePtoC) == -1)
  {
    fprintf(stderr, "Error building pipes\n");
    exit(1);
  }
}
