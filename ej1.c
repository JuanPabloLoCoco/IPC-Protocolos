#include <stdio.h>
#include <unistd.h>

static int pipeCtoP [2] = {-1,-1};
static int pipePtoC [2] = {-1,-1};
static int stdinFD = STDIN_FILENO;
static int stdoutFD = STDOUT_FILENO;


int bitParity(int number) {
  int b = 1;

  b = b ^ (number);
  b = b ^ (number >> 1);
  b = b ^ (number >> 2);
  b = b ^ (number >> 4);
  b = b ^ (number >> 5);
  b = b ^ (number >> 6);
  b = b ^ (number >> 7);

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
    childProcess();
  }
  else
  {
    parentProcess();
  }
  return 0;

}

/*
*
*
*/

static void childProcess(char * command)
{
  char *args[4];
  close(stdinFD);
  close(stdoutFD);

  close(pipeCtoP[0]);
  close(pipePtoC[1]);

// redirects standard input to child's end pipe
  dup2(pipePtoC[0], fileno(stdin));

// redirects parent's to standard output  pipeCtoP[1], fileno(stdout)
  dup2(pipeCtoP[1], fileno(stdout));

  args[0] = "bash";
  args[1] = "-c";
  args[2] = command;
  args[3] = NULL;

  execvp(args[0], args);
}

int parentProcess()
{
  close (pipePtoC[0]);
  close (pipeCtoP[1]);



}

void initializePipes()
{
  if(pipe(pipeCtoP) == -1 || pipe(pipePtoC) == -1)
  {
    fprintf(stderr, "Error building pipes\n");
    exit(1);
  }
}
