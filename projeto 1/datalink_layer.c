/*Non-Canonical Input Processing*/
#include "datalink_layer.h"

volatile int STOP=FALSE;
unsigned char flag_attempts=1;
unsigned char flag_alarm=1;
unsigned char flag_error=0;

void alarm_handler(){
	flag_attempts++;
	if(flag_attempts <4){
		printf("RETRY: %u\n", flag_attempts);
	}
  if(flag_attempts == 3){
    flag_error = 1;
  }
	flag_alarm=1;
}



void stateMachine(unsigned char c, int* state, unsigned char* trama){

	switch(*state){
		case S0:
			if(c == FLAG){
				*state = S1;
			}
			break;
		case S1:
			if(c != FLAG){
				*state = S2;
				trama[0] = c;
			}
			break;
		case S2:
			if(c != FLAG){
				*state = S3;
				trama[1] = c;
			}
			break;
		case S3:
			if(c != FLAG){
				trama[2] = c;
				if((trama[0]^trama[1]) != trama[2]){

					*state = S0;
				}
				else{
					*state=S4;
				}

			}
			break;
		case S4:
			if(c == FLAG){
				STOP = TRUE;
        alarm(0);
				flag_alarm=0;
			}
			else{
				*state = S0;
			}
			break;


	}
}


int set_writer(int* fd){

  unsigned char SET[5] = {FLAG, ADDR, CW, BCCW, FLAG};
  unsigned char elem;
	int res;
  unsigned char trama[3];
  int state=0;
	(void) signal(SIGALRM, alarm_handler);
  while(flag_attempts < 4 && flag_alarm == 1){
      res = write(*fd,SET,5);
      printf("%d bytes written\n", res);

      alarm(3);
      flag_alarm=0;

    // Wait for UA signal.

      while(STOP == FALSE && flag_alarm == 0){
					res = read(*fd,&elem,1);
       		if(res >0) {
          		stateMachine(elem, &state, trama);
       		}
      }
  }

  if(flag_error == 1){
     printf("Can't connect to the reader\n");
     return FALSE;
  }
  else{
    printf("%u%u%u\n",trama[0],trama[1],trama[2]);
    return TRUE;
  }

}

int set_reader(int* fd){

  unsigned char UA[5] = {FLAG, ADDR, CR, BCCR, FLAG};
  char elem;
	int res;
  unsigned char trama[3];
  int state=0;
  while (STOP==FALSE) {       /* loop for input */
      res = read(*fd,&elem,1);

      if(res>0){
        stateMachine(elem, &state, trama);
      }
    }
  printf("%u%u%u\n",trama[0],trama[1],trama[2]);

	res = write(*fd,UA,5);
	sleep(1);
	printf("%d bytes written\n", res);
}



int LLOPEN(int* fd, char* mode){

  if(strcmp(mode,"r") == 0){
    return set_reader(fd);

  }
  else if(strcmp(mode,"w") == 0){
    return set_writer(fd);
  }
}

int LLWRITE(char* msg){
	//TODO
}

int LLREAD(char* msg){
	//TODO
}
