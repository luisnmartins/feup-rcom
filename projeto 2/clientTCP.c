/*      (C)2000 FEUP  */

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
#define SERVER_ADDR "192.168.28.96"
#define MAX_STRING_LENGTH 256

int parseArgs(char* input, char* user, char* pass, char* host_name, char* file_path) {

    printf("start parse\n");
    unsigned int i=6;
    unsigned int word_index = 0;
    unsigned int state=0;
    unsigned int input_length = strlen(input);
    char elem;


    while(i < input_length){

        elem = input[i];
        switch(state){
            case 0:
                if(elem == ':') {
                    word_index = 0;
                    state = 1;

                } else {
                    user[word_index] = elem;
                    word_index++;
                }
                break;
            case 1:
                if(elem == '@'){
                    word_index = 0;
                    state = 2;

                } else {
                    pass[word_index] = elem;
                    word_index++;
                }
                break;
            case 2:
                if(elem == '/'){
                    word_index = 0;
                    state = 3;
                } else {
                    host_name[word_index] = elem;
                    word_index++;
                }
                break;
            case 3:
                file_path[word_index] = elem;
                word_index++;
                break;
        }
        i++;
    }
    return 0;
}

char* get_ip_addr(char* host_name){
    struct hostent *h;

    if ((h=gethostbyname(host_name)) == NULL) {
        herror("gethostbyname");
        exit(1);
    }
    return h->h_addr;
}

void responseCode(char* response, char* type){
  *type = response[0];
}


void sendMessage(int sockfd, char* message, char* param){
  int bytes;
  char* total_message = (char*) malloc(MAX_STRING_LENGTH);
  strcat(total_message,message);
  strcat(total_message, param);
  /*send a string to the server*/
  bytes = write(sockfd, total_message, strlen(total_message));
  printf("Bytes escritos %d\n", bytes);
}

void readResponse(int sockfd, char* response){
  char type;
  read(sockfd, response, MAX_STRING_LENGTH);
  responseCode(response, &type);
}

char* logInServer(int sockfd, char* user, char* pass){
  char* userResponse = (char*) malloc(MAX_STRING_LENGTH);
  sendMessage(sockfd, "user", user);
  readResponse(sockfd, userResponse);
  return userResponse;
}


int main(int argc, char** argv){

    int    sockfd;
    struct    sockaddr_in server_addr;

    char user[MAX_STRING_LENGTH];
    memset(user, 0, MAX_STRING_LENGTH);
    char pass[MAX_STRING_LENGTH];
    memset(pass, 0, MAX_STRING_LENGTH);
    char host_name[MAX_STRING_LENGTH];
    memset(host_name, 0, MAX_STRING_LENGTH);
    char file_path[MAX_STRING_LENGTH];
    memset(file_path, 0, MAX_STRING_LENGTH);


    parseArgs(argv[1], user, pass, host_name, file_path);
    printf("User: %s\n", user);
    printf("Pass: %s\n", pass);
    printf("Host: %s\n", host_name);
    printf("File: %s\n", file_path);
    char* ip_addr = get_ip_addr(host_name);

    /*server address handling*/
    bzero((char*)&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)ip_addr)));    /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(SERVER_PORT);        /*server TCP port must be network byte ordered */

    /*open an TCP socket*/
    if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        perror("socket()");
        exit(0);
    }

    /*connect to the server*/
    if(connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0){
        perror("connect()");
        exit(0);
    }

    logInServer(sockfd, user, pass);


    close(sockfd);
    exit(0);
}
