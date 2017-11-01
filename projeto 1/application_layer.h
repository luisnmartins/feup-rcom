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


#define MAXSIZELINK 7 //2*1(bcc2) + 5 other element trama datalink

#define START_PACKET_TYPE 1
#define END_PACKET_TYPE 0

typedef struct {
  int filesize;
  char* filename;
  FILE* fp;
}file_info;

typedef struct{
  int file_descriptor;
  char* status;
  int size_to_read;
}application_layer;

int send_message(unsigned char* msg, int length);
unsigned char* get_only_data(unsigned char* readed_msg, int* length);
unsigned char* get_message();
unsigned char* data_package_constructor(unsigned char* msg, int* length);
int get_file_size();
int create_STARTEND_packet(unsigned char* packet, int type);
void start_message(unsigned char* msg);
void handle_readfile();
void handle_writefile(unsigned char * data,int sizetowrite);
int verify_end(unsigned char* msg);
int main(int argc, char** argv);

#endif
