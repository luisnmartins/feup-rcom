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
#include <time.h>
#include "utils.h"

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
#define ERROR_PERCENTAGE_BCC1 0
#define ERROR_PERCENTAGE_BCC2 0
#define S0 0
#define S1 1
#define S2 2
#define S3 3
#define S4 4
#define SESC 5

#define READER 0
#define WRITER 1

#define RR 1
#define REJ 0
#define DISC 0x0B

#define C_START 2
#define C_END 3
#define C_I 0x01


#define TRAMA_S 0
#define TRAMA_I 1

#define ERROR_BCC1 1
#define ERROR_BCC2 2


void alarm_handler();
void state_machine(unsigned char c, int* state, unsigned char* trama, int* length, int trama_type);
int set_writer(int* fd);
int set_reader(int* fd);
void set_serial_port(char* port, int* fd);
int close_serial_port(int fd);
int LLOPEN(char* port, char* mode);
unsigned char* create_package(unsigned char* msg, int* length);
int get_result(int *fd);
unsigned char* verify_bcc2(unsigned char* control_message, int* length);
unsigned char* remove_head_msg_connection(unsigned char* msg, int* length);
unsigned char* add_control_message(unsigned char* stuffed_message_control, int* length);
unsigned char* byte_stuffing(unsigned char* msg, int* length);
unsigned char* byte_destuffing(unsigned char* msg, int* length);
int LLWRITE(int fd, unsigned char* msg, int* length);
int send_response(int fd, unsigned int type, unsigned char c);
unsigned char* mess_up_bcc1(unsigned char* packet, int size_packet);
unsigned char* mess_up_bcc2(unsigned char* packet, int size_packet);
unsigned char* LLREAD(int fd, int* length);
void LLCLOSE(int fd, int type);
unsigned char* reader_disc(int fd,unsigned char* disc);
//unsigned char* distortBCC(unsigned char * packet,int sizePacket,int type);


#endif
