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
  int size_to_read;
}file_info;

typedef struct{
  int file_descriptor;
  char* status;
}application_layer;


/**
 * @brief Create package to send and send it to the datalink layer using LLWRITE
 *
 * If is not a strat/end frame the package header
 * is added and after that the message is sent
 *
 * @param msg array with the data that is to send
 * @param length length of the message that is to send
 * @return returns TRUE if message was sent and false otherwise
 */
int send_message(unsigned char* msg, int length);

/**
 * @brief Removes the package header from the received trama
 *
 *
 * @param readed_msg array with the trama received
 * @param length Pointer to the length of the trama received. This param is updated to the length of the returned array
 * @return returns an array with the real data from the readed_msg
 */
unsigned char* get_only_data(unsigned char* readed_msg, int* length);

/**
 * @brief Create package to send and send it to the datalink layer using LLWRITE
 *
 * If is not a strat/end frame the package header is added and after that the message is sent
 *
 * @param msg array with the data that is to send
 * @param length length of the message that is to send
 * @return returns TRUE if message was sent and false otherwise
 */
unsigned char* get_message();

/**
 * @brief Create package to send and send it to the datalink layer using LLWRITE
 *
 * If is not a strat/end frame the package header is added and after that the message is sent
 *
 * @param msg array with the data that is to send
 * @param length length of the message that is to send
 * @return returns TRUE if message was sent and false otherwise
 */
unsigned char* data_package_constructor(unsigned char* msg, int* length);
int get_file_size();
int create_STARTEND_packet(unsigned char* packet, int type);
void start_message(unsigned char* msg);
void handle_readfile();
void handle_writefile(unsigned char * data,int sizetowrite);
int verify_end(unsigned char* msg);
int main(int argc, char** argv);

#endif
