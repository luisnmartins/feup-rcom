#ifndef APPLICATION_LAYER_H
#define APPLICATION_LAYER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "datalink_layer.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define DATA_CONTROL 1

/*struct data_info{
  int n;
  int length;
  unsigned char* data;
}*/

int send_message(int* fd, unsigned char* msg, int length);
unsigned char* get_only_data(unsigned char* readed_msg, int* length);
unsigned char* get_message(int* fd);
unsigned char* data_package_constructor(unsigned char* msg, int* length);
int get_file_size(FILE *ptr_myfile, int* filesize);
int create_STARTEND_packet(unsigned char* start_packet,unsigned char* filename,int filesize,int type);
void start_message(unsigned char* msg);
void handle_readfile(FILE*fp,int port,int sizetoread);
void handle_writefile(FILE * fp,unsigned char * data,int sizetowrite);
int verify_end(unsigned char* msg, FILE* fp);
int main(int argc, char** argv);

#endif
