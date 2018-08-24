#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "ej1.h"

static int pipeCtoP [2] = {-1,-1};
static int pipePtoC [2] = {-1,-1};
static int stdinFD = STDIN_FILENO;
static int stdoutFD = STDOUT_FILENO;

/*code taken from stackoverflow*/
int bitParity(char * s)
{
  int ret = 0;
  while(*s)
  {
    ret = ret ^ *s;
    s++;
  }
  return ret;
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

  inputParity = copy_rw_parity(STDIN_FILENO, pipePtoC[WRITE], bufferSize, inputParity);


  close(pipePtoC[WRITE]);
  int pid = fork();

  if(pid == 0){
    outputParity = copy_rw_parity(pipeCtoP[READ], stdoutFD, bufferSize, outputParity);
    // while((bytesRead = read(pipeCtoP[READ], buffer, bufferSize)) > 0){
    //   outputParity ^= bitParity(buffer);
    //   printf("%s ",buffer);
    // }
  }else{
    /* %02 imprime al menos 2 digitos, si hay menos de 2 digitos pone un 0 adelante */
    fprintf(stderr, "in parity: 0x%02X\n", inputParity);
    fprintf(stderr, "out parity: 0x%02X\n", outputParity);
  }







}

void initializePipes()
{
  if(pipe(pipeCtoP) == -1 || pipe(pipePtoC) == -1)
  {
    fprintf(stderr, "Error building pipes\n");
    exit(1);
  }
}

int
copy_rw_parity(const int fromfd, const int tofd, unsigned int buffsize, int parity) {
    char buf[buffsize];
    ssize_t nread;

    if(buffsize > sizeof(buf)) {
        fprintf(stderr, "buffsize should be <= : %lu\n", sizeof(buf));
        return 0;
    }
    while (nread = read(fromfd, buf, buffsize), nread > 0) {
        char *out_ptr = buf;
        ssize_t nwritten;
        parity = bitParity(buf);
        do {
            nwritten = write(tofd, out_ptr, nread);

            if (nwritten >= 0) {
                nread -= nwritten;
                out_ptr += nwritten;
            } else if (errno != EINTR) {
                goto error;
            }
        } while (nread > 0);
    }
    error:
        if(errno != 0) {
           fprintf(stderr, "Failed to copy: %s\n", strerror(errno));
        }
    return parity;
}
