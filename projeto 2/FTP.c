#include "parser.h"
#include "FTP.h"

static connection_info* connection;

int sendMessage(int sockfd, char* message, char* param){
  int bytes;
  char* total_message = (char*) malloc(MAX_STRING_LENGTH);
  strcat(total_message,message);
  if(param != NULL)
    strcat(total_message, param);
  strcat(total_message, "\n");
  /*send a string to the server*/
  bytes = write(sockfd, total_message, strlen(total_message));
  return bytes;
}


int readResponse(int sockfd, char* response){
  int bytes;
  memset(response, 0, MAX_STRING_LENGTH);
  bytes = read(sockfd, response, MAX_STRING_LENGTH);
  return bytes;
}


//returns the more significant digit of the response code and finish connection if it is 5(Permanent Negative Completion reply)
int getCodeResponse(int sockfd,char* response){
  int responseCode;
  responseCode = (int) response[0]-'0';
  if(responseCode == 5){
      fprintf(stderr,"%s\n", response);
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
      finalcode = getCodeResponse(sockfd, response);

    }while(finalcode == 1);

  }while(finalcode == 4);

  return response;
}



int logInServer(int sockfd){
  char* response = communication(sockfd, "user ", connection->user);
    if(response[0] != '3'){
        printf("%s\n", response);
        return 1;
    }
  response = communication(sockfd, "pass ", connection->password);
    if(response[0] != '2'){
        fprintf(stderr, "%s", "User or password incorrect\n");
        return 1;
    }
  printf("User logged in\n");
  return 0;
}


char* get_ip_addr(){
    struct hostent *h;
    char* ip = (char*) malloc(MAX_IP_LENGTH);
    printf("HOST: %s\n", connection->hostname);
    if ((h=gethostbyname(connection->hostname)) == NULL) {
        herror("gethostbyname");
        exit(1);
    }
    ip = inet_ntoa(*((struct in_addr *)h->h_addr));
    return ip;
}


int openConnection(int port,int isCommandConnection){

  int    sockfd;
  struct    sockaddr_in server_addr;


  /*server address handling*/
  bzero((char*)&server_addr,sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(connection->ip);     /*32 bit Internet address network byte ordered*/
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
  if(isCommandConnection){
    do{
      readResponse(sockfd, openResponse);
      code = getCodeResponse(sockfd, openResponse);
    }while(code != 2);
  }

  return sockfd;
}

char* getFilename(){
    char* filename = (char*) malloc(MAX_STRING_LENGTH);
    memset(filename, 0, MAX_STRING_LENGTH);
    unsigned int i=0, j=0, state=0;
    int length = strlen(connection->file_path);
    while(i<length){
        switch (state) {
            case 0:
                if(connection->file_path[i] != '/'){
                    filename[j] = connection->file_path[i];
                    j++;
                } else {
                    state = 1;
                }
                i++;
                break;
            case 1:
                memset(filename, 0, MAX_STRING_LENGTH);
                state = 0;
                j=0;
                break;
        }
    }
    return filename;
}


int getFile(int sockfd){
    char* filename = getFilename();
    
    char message[MAX_STRING_LENGTH];
    unsigned int bytesReaded;
    unsigned int totalBytes=0;
    
    FILE* filefd = fopen(filename, "w");
    if (filefd == NULL)
    {
        fprintf(stderr, "%s", "Error opening file to write!\n");
        exit(1);
    }
    
    while((bytesReaded = readResponse(sockfd, message)) > 0){
        totalBytes += bytesReaded;
        fseek(filefd, 0, SEEK_END);
        fwrite(message, sizeof(unsigned char), bytesReaded, filefd);
    }
    fclose(filefd);
    return totalBytes;
}


int main(int argc, char** argv){

    int commandSocket, dataSocket;

    if((connection = parseArgs(argv[1])) == NULL){
       printf("Input values are not valid! Please try again\n");
       exit(1);
    }
    connection->ip = get_ip_addr();
    printf("IP address: %s\n", connection->ip);
       
    //open connection to the server to send commands
    commandSocket = openConnection(SERVER_PORT, 1);
    
    //error logging In
    if(logInServer(commandSocket) != 0){
      fprintf(stderr, "%s", "Error logging in. Please try again!\n");
      close(commandSocket);
      exit(1);
    }
    
    //send pasv and get port to receive the file
    char* pasvResponse = communication(commandSocket, "pasv", NULL);
    int dataPort = parsePasvPort(pasvResponse);
    
    //open the new socket to receive the file
    dataSocket = openConnection(dataPort, 0);
    
    //send retrieve command to receive the file
    sendMessage(commandSocket, "retr ", connection->file_path);
    
    //get data and create file
    int fileBytes = getFile(dataSocket);
    if(fileBytes <= 0){
        fprintf(stderr, "%s", "Error reading the file\n");
        close(commandSocket);
        close(dataSocket);
        exit(1);
    }
    close(commandSocket);
    close(dataSocket);
    exit(0);
}
