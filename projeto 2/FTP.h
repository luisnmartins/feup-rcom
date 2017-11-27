//
//  FTP.h
//

#ifndef FTP_h
#define FTP_h

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>

#define SERVER_PORT 21
#define MAX_STRING_LENGTH 256
#define MAX_IP_LENGTH 16

typedef struct{
    char* user;
    char* password;
    char* hostname;
    char* file_path;
    char* ip;
    
} connection_info;



int parseArgs(char* input);
int sendMessage(int sockfd, char* message, char* param);
int getCodeResponse(int sockfd, char* response);
char* communication(int sockfd,char* message,char* param);
int logInServer(int sockfd);
char* get_ip_addr();
int openConnection(int port,int isCommandOpen);
int getPasvPort(char* msgToParse);
char* getFilename();
int getFile(int sockfd);
int main(int argc, char** argv);

#endif /* FTP_h */
