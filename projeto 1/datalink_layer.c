/*Non-Canonical Input Processing*/
#include "datalink_layer.h"

volatile int STOP=FALSE;
unsigned char flag_attempts=1;
unsigned char flag_alarm=1;
unsigned char flag_error=0;

struct termios oldtio,newtio;



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

	return TRUE;
}


/* SET Serial Port Initilizations */

void set_serial_port(char* port, int* fd){

	int c;
    int i, sum = 0, speed = 0;

    /*
      Open serial port device for reading and writing and not as controlling tty
      because we don't want to get killed if linenoise sends CTRL-C.
    */

    *fd = open(port, O_RDWR | O_NOCTTY );

	if (*fd <0) {perror(port); exit(-1); }

    if (tcgetattr(*fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */


    /*
      VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
      leitura do(s) próximo(s) caracter(es)
    */

    tcflush(*fd, TCIOFLUSH);

    if ( tcsetattr(*fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

}


int close_serial_port(int* fd){

	if ( tcsetattr(*fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(*fd);
    return 0;
}


int LLOPEN(char* port, char* mode){

  int fd;
  int result;
  set_serial_port(port, &fd);

  if(strcmp(mode,"r") == 0){
    result = set_reader(&fd);

  }
  else if(strcmp(mode,"w") == 0){
    result = set_writer(&fd);
  }

  if(result  == TRUE){
	return fd;
  }
  else{
	return -1;
  }

}

int send_package(int* fd, unsigned char* msg, int* length){
	int i=0;
	unsigned char bcc2;
	msg = (unsigned char *) realloc(str, *length+1);
	for(i; i<*length; i++){
		bcc2 ^=msg[i];
	}
	msg[*length] = bbc2;
	

}

/*int send_message(int *fd, char* msg, int length){
		int i=0;
		unsigned char* bcc2 = (unsigned char*);
		char* final_msg;
		for(i; i<length; i++){
			bcc2 ^=msg[i];
		}
		msg = (char *) realloc(msg, strlen(msg)+sizeof(int));
		strcat(msg, bcc2);
		byte_stuffing(msg);
		final_msg = (char *) malloc(strlen(msg)+4);
		/*strcat(final_msg, 0x)
		LLWRITE(fd, msg)

}*/

unsigned char* byte_stuffing(unsigned char* msg, int* length){
	unsigned char* str;
	int i=0;
	int j=0;
	int new_length = *length;
	str = (unsigned char *) malloc(*length);

	for(i; i < *length; i++, j++){
		if(msg[i] ==  0x7e){
			str = (unsigned char *) realloc(str, new_length+1);
			str[j] = 0x7d;
			str[j+1] = 0x5e;
			new_length++;
			j++;
		}
		else if(msg[i] == 0x7d){
			str = (unsigned char *) realloc(str, new_length+1);
			str[j] = 0x7d;
			str[j+1]= 0x5d;
			new_length++;
			j++;
		}
		else{
			str[j] = msg[i];
		}
	}

	*length = new_length;
	return str;
}

unsigned char* byte_destuffing(unsigned char* msg, int* length){
	unsigned char* str;
	int i=0;
	int new_length = 0;

	for(i; i<*length; i++){
		new_length++;
		str = (unsigned char *) realloc(str, new_length);
		if(msg[i] == 0x7d){
			if(msg[i+1] == 0x5e){
				str[new_length-1] = 0x7e;
				i++;
			}
	 		else if(msg[i+1] == 0x5d){
				str[new_length -1] = 0x7d;
				i++;
			}
		}
		else{
			str[new_length-1] = msg[i];
		}

	}
	*length = new_length;
	return str;
}


int LLWRITE(int* fd, char* msg, int length){

	int res = write(*fd, msg, length);
	if(res>0 && res == length){
		return TRUE;
	}
	else{
		return FALSE;
	}
}

int LLREAD(int* fd, char* msg){
	//TODO
}

void LLCLOSE(int* fd){
	close_serial_port(fd);
}
