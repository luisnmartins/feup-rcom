#include "utils.h"

struct timespec initial_time, final_time;
int n_number = -256;

void start_counting_time(){
    clock_gettime(CLOCK_REALTIME, &initial_time);
}

double calculate_time_elapsed(){
  clock_gettime(CLOCK_REALTIME, &final_time);
  double time_val = (final_time.tv_sec-initial_time.tv_sec)+
					(final_time.tv_nsec- initial_time.tv_nsec)/1E9;
  return time_val;
}

void progress_bar(int filesize, int file_sent_size, char* filename, char type){
  system("clear");
  int perc_real =  file_sent_size * 100 / filesize;
  int size_bar = 50;
  int perc = perc_real * size_bar / 100;
  int i;

  /* SHOW TYPE */
  if (type == 'r')
    printf("\n\t\t\t\t     RECEIVING\n");
  else
    printf("\n\t\t\t\t      SENDING\n");

  /*SHOW FILENAME */
  unsigned int filename_length = (unsigned int) strlen(filename);
  printf("\n\t| FILE: ");
  for (i = 0; i < filename_length; i++) {
    printf("%c", filename[i]);
  }
  printf("\n\t|");

  /*SHOW FILE SIZE */
  printf("\n\t| SIZE: %d bytes", filesize);
  printf("\n\t|");

  /*SHOW FILE SIZE */
  double timeSpent = calculate_time_elapsed();
  printf("\n\t| TIME ELAPSED: %.4lf s", timeSpent);
  printf("\n\t|");

  /* SHOW TRANSMISSION SPEED */
  float speed = (float) (file_sent_size / timeSpent);
  printf("\n\t| SPEED: %.2lf bytes/s", speed);
  printf("\n\t|");

  /* SHOW RESPONSE (ONLY READER) */
  if (type == 'r')
  {
	switch(utils_response_value[0]){

		case 0:
			printf("\n\t| RESPONSE: REJ%d", utils_response_value[1]);
  			printf("\n\t|");
			break;

		case 1:
			printf("\n\t| RESPONSE: RR%d", utils_response_value[1]);
			printf("\n\t|");
			break;

	}
  }

  /* SHOW PACKAGE NUMBER */
  printf("\n\t| PACKAGE NUMBER: %d", utils_n_package);
  printf("\n\t|");

  /* SHOW PROGRESS BAR */
  printf("\n\t| STATUS [");
  for (i = 0; i < perc; i++) {
      printf("*");
  }

  int rest = size_bar - perc;
  for (i = 0; i < rest ; i++) {
      printf(" ");
  }
  printf("]");

  /* convert % to char */
  int tmp = 37;
  unsigned char ch = tmp;

  printf(" %d%c\n\n", perc_real, ch);


}
