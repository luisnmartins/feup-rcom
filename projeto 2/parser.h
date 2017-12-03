//
//  parser.h
//

#ifndef parser_h
#define parser_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define MAX_STRING_LENGTH 256

typedef struct{
    char* user;
    char* password;
    char* hostname;
    char* file_path;
    char* ip;
    
} connection_info;

connection_info* parseArgs(char* input);
int parsePasvPort(char* msgToParse);

#endif /* parser_h */
