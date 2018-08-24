#ifndef EJ2_H
#define EJ2_H

#define READ 0
#define WRITE 1
/*1024 - 512 - 2048 */
#define BUFFER_SIZE 1024

int bitParity(char * s);

static void childProcess(char * command);

void parentProcess(int pipeCtoP[], int pipePtoC[], int bufferSize);

void initializePipes();

int copy_rw_parity(const int fromfd, const int tofd, unsigned int buffsize, int priority); 

#endif
