/*Non-Canonical Input Processing*/
#include "datalink_layer.h"

volatile int STOP=FALSE;
volatile unsigned char flag_attempts=1;
volatile unsigned char flag_alarm=1;
volatile unsigned char flag_error=0;

struct termios oldtio,newtio;
volatile unsigned char control_value=0;



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



void state_machine(unsigned char c, int* state, unsigned char* trama, int* length, int trama_type){

	switch(*state){
		case S0:
			if(c == FLAG){
				*state = S1;
				trama[*length-1] = c;
			}
			break;
		case S1:
			if(c != FLAG){
				trama[*length-1] = c;
				if(*length==4){
					if((trama[1]^trama[2]) != trama[3]){
						*state = S0;
						*length =0;
					}
					else{
						if(trama_type == TRAMA_I){
							control_value = c;
						}
						*state=S2;
					}
				}
			}
			else
			{
				*state = S0;
				*length = 0;
			}
			break;
		case S2:
			trama[*length-1] = c;
			if(c == FLAG){
				STOP = TRUE;
				alarm(0);
				flag_alarm=0;
			}
			else{
				if(trama_type == TRAMA_S){
					*state = S0;
					*length = 0;
				}
			}
			break;
	}
}


int set_writer(int* fd){

  unsigned char SET[5] = {FLAG, ADDR, CW, BCCW, FLAG};
  unsigned char elem;
	int res;
  unsigned char trama[5];
	int trama_length = 1;
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
          		state_machine(elem, &state, trama, &trama_length, TRAMA_S);
							trama_length++;
       		}
      }
  }

  if(flag_error == 1){
     printf("Can't connect to the reader\n");
     return FALSE;
  }
  else{
		printf("%u%u%u\n",trama[1],trama[2],trama[3]);
    return TRUE;
  }

}

int set_reader(int* fd){

  unsigned char UA[5] = {FLAG, ADDR, CR, BCCR, FLAG};
  char elem;
	int res;
  unsigned char trama[5];
	int trama_length =1;
  int state=0;
  while (STOP==FALSE) {       /* loop for input */
      res = read(*fd,&elem,1);

      if(res>0){
        state_machine(elem, &state, trama, &trama_length, TRAMA_S);
				trama_length++;
      }
    }
  printf("%u%u%u\n",trama[1],trama[2],trama[3]);

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
		LLCLOSE(&fd);
	return -1;
  }

}


int create_package(int* fd, unsigned char* msg, int length){
	int i=0;
	unsigned char bcc2 = 0x00;
	msg = (unsigned char *) realloc(msg, length+1);
	for(i; i<length; i++){
		bcc2 ^=msg[i];
	}
	msg[length] = bcc2;
	length = length+1;
	int stuffed_length = byte_stuffing(msg, length);
	int control_message_length = add_control_message(msg, stuffed_length);
	return control_message_length;
}


/*int send_rr(int* fd){
	unsigned char* rr[] = {FLAG, ADDR, control_value^1, ADDR^(control_value^1), FLAG};
	LLWRITE(fd, rr, 5);
}*/


int get_package(int* fd, unsigned char* msg){

	/*int length = LLREAD(fd, msg);
	if(length >0){
		send_rr();
	}
	unsigned char* control_message = verify_rmsg_connection(msg, &length);
	if(control_message == NULL)
		return -1;
	unsigned char* data_message = verify_bcc2(control_message, &length);
	msg = (unsigned char*) malloc(length);
	memcpy(msg, data_message, length);
	return length;*/
}

int verify_bcc2(unsigned char* control_message, int length){
	int des_length = byte_destuffing(control_message, length);
	int i=0;
	unsigned char control_bcc2 = 0x00;
	for(i; i<des_length-1; i++){
		control_bcc2 ^= control_message[i];
	}
	if(control_bcc2 != control_message[des_length-1])
		return -1;

	i=0;
	unsigned char* data_message = (unsigned char*) malloc(des_length-1);
	for(i; i<des_length-1; i++){
			data_message[i] = control_message[i];
	}
	des_length--;
	control_message = (unsigned char*) realloc(control_message, des_length);
	free(data_message);
	return des_length;
}



int verify_rmsg_connection(unsigned char* msg, int length){

	if((msg[1]^msg[2]) != msg[3]){
		return -1;
	}

	unsigned char* control_message = (unsigned char*) malloc(length-5);
	int i=4;
	int j=0;
	for(i; i<length-1; i++, j++){

		control_message[j] = msg[i];
	}
	msg = (unsigned char*) realloc(msg, length-5);
	memcpy(msg, control_message, length-5);
	free(control_message);
	return length-5;
}


int add_control_message(unsigned char* msg, int length){
	length = length+5;
	unsigned char* full_message = (unsigned char*) malloc(length);
	int i=0;
	full_message[0] = FLAG;
	full_message[1] = ADDR;
	full_message[2] = control_value;
	full_message[3] = full_message[1]^full_message[2];
	for(i; i<length; i++){
		full_message[i+4] = msg[i];
	}
	full_message[length-1] = FLAG;
	msg = (unsigned char*) realloc(msg, length);
	memcpy(msg, full_message, length);
	free(full_message);
	return length;
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

int byte_stuffing(unsigned char* msg, int length){
	unsigned char* str;
	int i=0;
	int j=0;
	int new_length = length;
	str = (unsigned char *) malloc(length);

	for(i; i < length; i++, j++){
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

	msg = (unsigned char*) realloc(msg, new_length);
	memcpy(msg, str, new_length);
	free(str);
	return new_length;
}

int byte_destuffing(unsigned char* msg, int length){
	unsigned char* str;
	str = (unsigned char*) malloc(1);
	int i=0;
	int new_length = 0;

	for(i; i<length; i++){
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
	msg = (unsigned char*) realloc(msg, new_length);
	memcpy(msg, str, new_length);
	free(str);
	return new_length;
}



int LLWRITE(int* fd, char* msg, int length){
	int final_length = create_package(fd, msg, length);
	if(final_length<0)
		return FALSE;
	int i=0;
	for(i=0;i<final_length; i++){
		printf("Valor: %x\n", msg[i]);
	}
	int res = write(*fd, msg, final_length);
	return get_result(fd);

	if(res>0 && res == length){
		return TRUE;
	}
	else{
		return FALSE;
	}
}

int get_result(int* fd){

}



int LLREAD(int* fd,unsigned char* msg){

	unsigned char elem;
	int state = S0;
	int res;
	int msg_length=0;
	STOP = FALSE;
	while (STOP==FALSE) {       /* loop for input */
			res = read(*fd,&elem,1);
			if(res>0){
				msg_length++;
				msg = realloc(msg, msg_length);
				state_machine(elem, &state, msg, &msg_length, TRAMA_I);
			}
	}

		/*int i=0;
		for(i; i<msg_length; i++){
			printf("READ: %x\n",msg[i]);
		}*/
		return msg_length;
}

void LLCLOSE(int* fd){
	close_serial_port(fd);
}
