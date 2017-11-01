#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

int utils_response_value[2];


void start_counting_time();
double calculate_time_elapsed();
void progress_bar(int filesize, int file_sent_size, char* filename, char type, unsigned char n_trama);

#endif
