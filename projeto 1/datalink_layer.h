#ifndef DATALINK_LAYER_H
#define DATALINK_LAYER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define FLAG 0x7E
#define ADDR 0x03
#define CW 0x03
#define CR 0x07
#define BCCW 0x00
#define BCCR 0x04
#define S0 0
#define S1 1
#define S2 2
#define S3 3
#define S4 4

#define C_START 2
#define C_END 3


#define TRAMA_S 0
#define TRAMA_I 1

void alarm_handler();
void state_machine(unsigned char c, int* state, unsigned char* trama, int* length, int trama_type);
int set_writer(int* fd);
int set_reader(int* fd);
void set_serial_port(char* port, int* fd);
int close_serial_port(int* fd);
int LLOPEN(char* port, char* mode);
int create_package(int* fd, unsigned char* msg, int length);
int get_package(int* fd, unsigned char* msg);
int get_result(int *fd);
int verify_bcc2(unsigned char* control_message, int length);
int verify_rmsg_connection(unsigned char* msg, int length);
int add_control_message(unsigned char* msg, int length);
int byte_stuffing(unsigned char* msg, int length);
int byte_destuffing(unsigned char* msg, int length);
int LLWRITE(int* fd, char* msg, int length);
int LLREAD(int* fd,unsigned char* msg);
void LLCLOSE(int* fd);


#endif
