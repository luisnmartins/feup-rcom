#include "utils.h"

time_t initialTime, finalTime;

void start_counting_time(){
    time(&initialTime);
}

double calculate_time_elapsed(){
  time(&finalTime);
  return (double)(difftime(finalTime, initialTime));
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
  printf("\n\t| TIME ELAPSED: %.0lf s", timeSpent);
  printf("\n\t|");

  /* SHOW TRANSMISSION SPEED */
  float speed = (float) (file_sent_size / timeSpent);
  printf("\n\t| SPEED: %.2lf bytes/s", speed);
  printf("\n\t|");
  printf("\n\t|");
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
