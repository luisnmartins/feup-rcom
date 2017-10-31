#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>


void start_counting_time();
double calculate_time_elapsed();
void progress_bar(int filesize, int file_sent_size, char* filename, char type);

#endif
