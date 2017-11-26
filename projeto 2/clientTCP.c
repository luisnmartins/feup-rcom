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

//TODO verify errors
int parseArgs(char* input, char* user, char* pass, char* host_name, char* file_path) {

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

void sendMessage(int sockfd, char* message, char* param){

  char* total_message = (char*) malloc(MAX_STRING_LENGTH);
  strcat(total_message,message);
  if(param != NULL)
    strcat(total_message, param);
  strcat(total_message, "\n");
  /*send a string to the server*/
  printf("SEND MESSAGE %s\n", total_message);
  write(sockfd, total_message, strlen(total_message));
}


void readResponse(int sockfd, char* response){
  read(sockfd, response, MAX_STRING_LENGTH);
}


//returns the more significant digit of the response code and finish connection if it is 5(Permanent Negative Completion reply)
int getCodeResponse(int sockfd,char* response){
  int responseCode;
  responseCode = (int) response[0]-'0';

  if(responseCode == 5){
        close(sockfd);
        exit(1);
  }
  return responseCode;
}




char* communication(int sockfd,char* message,char* param){
  char* response = (char*) malloc(MAX_STRING_LENGTH);
  int finalcode;

  do{

    sendMessage(sockfd, message, param);
    do{

      readResponse(sockfd,response);
      printf("RESPONSE: %s\n", response);
      finalcode = getCodeResponse(sockfd, response);

    }while(finalcode == 1);

  }while(finalcode == 4);

  return response;
}



int logInServer(int sockfd, char* user, char* pass){
  char* response = communication(sockfd, "user ", user);
  if(response[0] != '3')
    return 1;
  response =communication(sockfd, "pass ", pass);
  if(response[0] != '2')
    return 1;

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


int openConnection(char* host_name,int port,int isFirstOpen){

  int    sockfd;
  struct    sockaddr_in server_addr;


  char* ip_addr = get_ip_addr(host_name);
  printf("IP Addr %s\n", inet_ntoa(*((struct in_addr *)ip_addr)));

  /*server address handling*/
  bzero((char*)&server_addr,sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)ip_addr)));    /*32 bit Internet address network byte ordered*/
  server_addr.sin_port = htons(port);        /*server TCP port must be network byte ordered */

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



  char openResponse[MAX_STRING_LENGTH];
  int code;
  //TODO verify this loop
  if(isFirstOpen){
    do{
      readResponse(sockfd, openResponse);
      printf("RESP: %s\n", openResponse);
      code = getCodeResponse(sockfd, openResponse);
    }while(code != 2);
  }

  return sockfd;
}


int getPasvPort(char* msgToParse){

  // get only numbers
  char pasvCodes[24];
  unsigned int length = strlen(msgToParse)-30;
  int i=0;
  for(; i < length; i++){
    pasvCodes[i] = msgToParse[i+27];
  }

  // parse to get the last two numbers
  unsigned int countCommas=0;
  unsigned int state =0;
  unsigned int actualPos = 0;
  i=0;
  char num1[4];
  memset(num1, 0, 4);
  char num2[3];
  memset(num2, 0, 4);

  while(1){
    switch(state){
      case 0:
        if(pasvCodes[i] == ',') {
          countCommas++;
        }
        if(countCommas == 4) {
          state = 1;
        }
        break;
      case 1:
        if(pasvCodes[i] == ',') {
          actualPos = 0;
          state = 2;
        } else {
            num1[actualPos] = pasvCodes[i];
            actualPos++;
        }
        break;
      case 2:
        if(pasvCodes[i] != ')') {
          num2[actualPos] = pasvCodes[i];
          actualPos++;
        }
        break;
      }
      if(pasvCodes[i] == ')'){
        break;
      }
      i++;
  }

  printf("PARSE 2: %s\n", num1);
  printf("PARSE 1: %s\n", num2);
  int firstNumber = atoi(num1);
  int secondNumber = atoi(num2);
  return (firstNumber*256+secondNumber);
}


int main(int argc, char** argv){

    int    sockfd;

    char user[MAX_STRING_LENGTH];
    memset(user, 0, MAX_STRING_LENGTH);
    char pass[MAX_STRING_LENGTH];
    memset(pass, 0, MAX_STRING_LENGTH);
    char host_name[MAX_STRING_LENGTH];
    memset(host_name, 0, MAX_STRING_LENGTH);
    char file_path[MAX_STRING_LENGTH];
    memset(file_path, 0, MAX_STRING_LENGTH);


    parseArgs(argv[1], user, pass, host_name, file_path);
    sockfd = openConnection(host_name, SERVER_PORT, 1);

    printf("User: %s\n", user);
    printf("Pass: %s\n", pass);
    printf("Host: %s\n", host_name);
    printf("File: %s\n", file_path);
    //error logging In
    if(logInServer(sockfd, user, pass) != 0){
      close(sockfd);
      exit(1);
    }


    char* pasvResponse = communication(sockfd, "pasv", NULL);
    int newPort = getPasvPort(pasvResponse);

    int Newsockfd = openConnection(host_name, newPort, 0);
    char* receiveFileResponse = communication(sockfd, "retr ", file_path);
    getNameFile = readResponse(sockfd, )
    printf("FILE: %s\n", receiveFileResponse);
    exit(0);
}
