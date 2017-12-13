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
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <strings.h>
#include <ctype.h>

#define SERVER_PORT 21
#define MAX_IP_LENGTH 16

/**
 * Send a message to the file descriptor sokfd 
 * with the following structure: message+param+"\r\n". 
 * The 2 final characters represent the end of 

*/
int sendMessage(int sockfd, char* message, char* param);
int getCodeResponse(int sockfd, char* response);
int communication(int sockfd,char* message,char* param);
int logInServer(int sockfd);
char* get_ip_addr();
int openConnection(int port,int isCommandOpen);
char* getFilename();
int getFile();
int main(int argc, char** argv);

#endif /* FTP_h */
