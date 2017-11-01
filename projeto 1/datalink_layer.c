/*Non-Canonical Input Processing*/
#include "datalink_layer.h"

volatile int STOP=FALSE;
volatile unsigned char flag_attempts=1;
volatile unsigned char flag_alarm=1;
volatile unsigned char flag_error=0;
const unsigned char control_values[] = { 0x00, 0x40, 0x05, 0x85, 0x01, 0x81 };
struct termios oldtio,newtio;
struct timespec clockStart,clockEnd;
volatile unsigned char control_value=0;
volatile unsigned char duplicate=FALSE;



void alarm_handler(){
	flag_attempts++;

  if(flag_attempts >= 4){
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
						*state = SESC;
					}
					else{
						*state=S2;
					}
				}
			}
			else
			{
				*length = 1;
				trama[*length-1] = c;
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
		case SESC:
			trama[*length-1] = c;
			if(c == FLAG){
				if(trama_type == TRAMA_I){
					flag_error = 1;
					STOP = TRUE;
				}
				else{
					*state = S0;
					*length = 0;
				}
			}
	}
}


int set_writer(int* fd){

  unsigned char SET[5] = {FLAG, ADDR, CW, BCCW, FLAG};
  unsigned char elem;
	int res;
  unsigned char trama[5];
	int trama_length = 0;
  int state=0;
	(void) signal(SIGALRM, alarm_handler);
  while(flag_attempts < 4 && flag_alarm == 1){
	    //printf("TRY: %x\n", flag_attempts);
		res = write(*fd,SET,5);
      //printf("%d bytes written\n", res);
      alarm(3);
      flag_alarm=0;

    // Wait for UA signal.

      while(STOP == FALSE && flag_alarm == 0){
			res = read(*fd,&elem,1);
       		if(res >0) {
				trama_length++;
          		state_machine(elem, &state, trama, &trama_length, TRAMA_S);

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
	int trama_length =0;
  int state=0;
  while (STOP==FALSE) {       /* loop for input */
      res = read(*fd,&elem,1);

      if(res>0){
		trama_length++;
        state_machine(elem, &state, trama, &trama_length, TRAMA_S);

      }
    }
  printf("%u%u%u\n",trama[1],trama[2],trama[3]);

	res = write(*fd,UA,5);


	//printf("%d bytes written\n", res);

	return TRUE;
}


/* SET Serial Port Initilizations */

void set_serial_port(char* port, int* fd){

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

	//start clock
	clock_gettime(CLOCK_REALTIME,&clockStart);

    if ( tcsetattr(*fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    //printf("New termios structure set\n");

}


int close_serial_port(int fd){

	if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
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
		LLCLOSE(fd, -1);
	return -1;
  }

}


unsigned char* create_package(unsigned char* msg, int* length){
	int i=0;
	unsigned char bcc2 = 0x00;
	unsigned char* new_message = (unsigned char*) malloc(*length+1);
	for(i=0; i<*length; i++){
		new_message[i] = msg[i];
		bcc2 ^=msg[i];
	}
	new_message[*length] = bcc2;
		*length = *length+1;
	i=0;
	unsigned char* stuffed_message = byte_stuffing(new_message, length);

	unsigned char* control_message = add_control_message(stuffed_message, length);

	return control_message;
}



unsigned char* verify_bcc2(unsigned char* control_message, int* length){
	unsigned char* destuffed_message = byte_destuffing(control_message, length);
	int i=0;
	unsigned char control_bcc2 = 0x00;
	for(; i<*length-1; i++){
		control_bcc2 ^= destuffed_message[i];
	}
	if(control_bcc2 != destuffed_message[*length-1]){
		*length = -1;
		return NULL;
	}
	*length = *length-1;
	unsigned char* data_message = (unsigned char*) malloc(*length);
	for(i=0; i<*length; i++){
			data_message[i] = destuffed_message[i];
	}
	free(destuffed_message);
	return data_message;
}



unsigned char* remove_head_msg_connection(unsigned char* msg, int* length){

	unsigned char* control_message = (unsigned char*) malloc(*length-5);
	int i=4;
	int j=0;
	for(; i<*length-1; i++, j++){

		control_message[j] = msg[i];
	}
	*length = *length-5;
	free(msg);
	return control_message;
}


unsigned char* add_control_message(unsigned char* stuffed_message_control, int* length){
	unsigned char* full_message = (unsigned char*) malloc(*length+5);
	int i=0;
	full_message[0] = FLAG;
	full_message[1] = ADDR;
	full_message[2] = control_values[control_value];
	full_message[3] = full_message[1]^full_message[2];
	for(; i<*length; i++){
		full_message[i+4] = stuffed_message_control[i];
	}
	full_message[*length+4] = FLAG;
	*length = *length+5;

	free(stuffed_message_control);

	return full_message;
}


unsigned char* byte_stuffing(unsigned char* msg, int* length){
	unsigned char* str;
	int i=0;
	int j=0;
	unsigned int array_length = *length;
	str = (unsigned char *) malloc(array_length);
	for(; i < *length; i++, j++){

		if(j >= array_length){
			array_length = array_length+(array_length/2);
			str = (unsigned char*) realloc(str, array_length);

		}
		if(msg[i] ==  0x7e){
			str[j] = 0x7d;
			str[j+1] = 0x5e;
			j++;
		}
		else if(msg[i] == 0x7d){
			str[j] = 0x7d;
			str[j+1]= 0x5d;
			j++;
		}
		else{
			str[j] = msg[i];
		}
	}
	*length = j;
	free(msg);

	/*
	i=0;
	for(; i<*length; i++){
		printf("STUFFED: %x\n", str[i]);
	}
	*/
	return str;
}

unsigned char* byte_destuffing(unsigned char* msg, int* length){
	unsigned int array_length = 133;
	unsigned char* str = (unsigned char*) malloc(array_length);
	int i=0;
	int new_length = 0;

	for(; i<*length; i++){
		new_length++;
		if(new_length >= array_length){
			array_length = array_length+ (array_length/2);
			str = (unsigned char *) realloc(str, array_length);
		}
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
	free(msg);
	return str;
}



int LLWRITE(int fd, unsigned char* msg, int* length){
	unsigned char* full_message= create_package(msg, length);
	if(*length<0)
		return FALSE;

	unsigned char elem;
	int res;
	unsigned char trama[5];
	int trama_length = 0;
	int state=S0;

	STOP=FALSE;
	flag_attempts=1;
	flag_alarm=1;
	flag_error=0;

	while(flag_attempts < 4 && flag_alarm == 1){
		res = write(fd, full_message, *length);
		//printf("%d bytes written\n", res);

		alarm(3);
		flag_alarm=0;
		// Wait for response signal.
		while(STOP == FALSE && flag_alarm == 0){
			res = read(fd,&elem,1);
			if(res >0) {
				trama_length++;
				state_machine(elem, &state, trama, &trama_length, TRAMA_S);

			}
		}



		if (STOP == TRUE) {
			if(trama[2] == control_values[control_value+4]){
				printf("ERROR CONTROL VALUE\n");
				flag_alarm=1;
				flag_attempts = 1;
				flag_error=0;
				STOP = FALSE;
				state = S0;
				trama_length = 0;
			}
		}
	}


	if (flag_error == 1){
		printf("LLWRITE FLAG ERROR\n");
		return FALSE;
	}

	control_value = control_value^1;
	//printf("PASS CONTROL VALUE\n");
	return TRUE;
}


unsigned char* LLREAD(int fd, int* length){
	unsigned char elem;
	int state = S0;
	int res;
	*length=0;
	flag_error = 0;
	STOP = FALSE;
	unsigned int msg_array_length = 138;
	unsigned char* msg= (unsigned char*) malloc(msg_array_length);
	unsigned char* finish = (unsigned char*) malloc(1);
	finish[0] = DISC;
	while (STOP==FALSE) {       /* loop for read info */
		res = read(fd,&elem,1);
		if(res>0){
			*length = *length+1;
			if(*length >= msg_array_length){
				msg_array_length = msg_array_length+(msg_array_length/2);
				msg = realloc(msg, msg_array_length);
			}
			state_machine(elem, &state, msg, length, TRAMA_I);
		}
	}
	
	if((msg[4] == C_I) && (flag_error != 1)){
		msg = mess_up_bcc1(msg, *length);
		msg = mess_up_bcc2(msg, *length);
	}

	if(flag_error == 1){
		//printf("REJ BCC1:\n");
		return NULL;
	}
	if(msg[2] == DISC){
		LLCLOSE(fd, READER);
		return finish;
	}

	duplicate = (control_values[control_value] == msg[2]) ? FALSE: TRUE;
	unsigned char char2_temp = msg[2];
	unsigned char* msg_no_head = remove_head_msg_connection(msg, length);
	unsigned char* msg_no_bcc2= verify_bcc2(msg_no_head, length);


	if(*length == -1){
		if(duplicate == TRUE){
			send_response(fd, RR, char2_temp);
			return NULL;
		}
		else{
			send_response(fd, REJ, char2_temp);
			return NULL;
		}
	}
	else{
		if(duplicate != TRUE){
			control_value = send_response(fd, RR, char2_temp);
			return msg_no_bcc2;
		}
		else{
			send_response(fd, RR, char2_temp);
			return NULL;
		}

	}

}

int send_response(int fd, unsigned int type, unsigned char c){
	unsigned char bool_val;
	unsigned char response[5];
	response[0] = FLAG;
	response[1] = ADDR;
	response[4] = FLAG;
	if(c == 0x00)
		bool_val = 0;
	else
		bool_val = 1;

	switch (type) {
		case RR:
			//printf("RR%d\n", bool_val^1);
			response[2] = control_values[(bool_val^1)+2];
			break;
		case REJ:
			//printf("REJ%d\n", bool_val);
			response[2] = control_values[bool_val+4];
			break;
	}
	response[3] = response[1]^response[2];

 	write(fd, response, 5);

	return bool_val^1;
}


void LLCLOSE(int fd, int type){
	unsigned char disc[5] = {FLAG, ADDR, DISC, ADDR^DISC, FLAG};
	unsigned char * received;
	if(type == READER){
		received = reader_disc(fd,disc);
		if(received[2] == CR){
			printf("Final UA received with success\n");
		}else {
			printf("Problem receiving final UA\n");
		}


	}else if(type == WRITER){

		received = reader_disc(fd,disc);

		if(received[2] == DISC){

			printf("Final DISC received with success\n");
		}else {
			printf("Problem receiving final DISC\n");
		}
		unsigned char UA[5] = {FLAG, ADDR, CR, BCCR, FLAG};
		write(fd,UA,5);
		sleep(1);
	}

	close_serial_port(fd);
}

unsigned char* reader_disc(int fd,unsigned char* disc){
	 flag_attempts=1;
	 flag_alarm=1;
	 flag_error=0;
	 STOP = FALSE;
	unsigned char elem;
	int res;
  unsigned char* trama = (unsigned char*) malloc(5);
	int trama_length = 0;
  int state=0;

	while(flag_attempts < 4 && flag_alarm == 1){
			//printf("TRY: %x\n", flag_attempts);
		res = write(fd,disc,5);

			alarm(3);
			flag_alarm=0;



			while(STOP == FALSE && flag_alarm == 0){
			res = read(fd,&elem,1);
					if(res >0) {
						trama_length++;
							state_machine(elem, &state, trama, &trama_length, TRAMA_S);

					}
			}

	}

	//printf("DISC0 %x\n",trama[0]);
	return trama;
}


/*unsigned char* distortBCC(unsigned char * packet,int sizePacket,int type){
	unsigned char * result = (unsigned char*) malloc(sizePacket);
	int i;
	memcpy(result,packet,sizePacket);
	int random = (rand() % 100) + 1;
	if(random <= ERRORPERCENTAGE){
		if(type == ERROR_BCC2)
		i = (rand() % (sizePacket-5))+4;
		else if(type == ERROR_BCC1)
		i = (rand() % 2) + 1;
		unsigned char letter = (unsigned char) ('A' + (rand() % 26));
		result[i] = letter;
	}
	return result;
}*/

unsigned char* mess_up_bcc1(unsigned char* packet, int size_packet){
	
	unsigned char* messed_up_msg = (unsigned char*) malloc(size_packet);
	unsigned char letter;
	
	memcpy(messed_up_msg, packet, size_packet);
	int perc = (rand()%100)+1;
	printf("PERCENTAGE BCC1: %d\n", perc);
	if(perc <= ERROR_PERCENTAGE_BCC1){
		do{
			letter = (unsigned char) ('A' + (rand()%256));
		}while(letter == messed_up_msg[3]);
		messed_up_msg[3] = letter;
		flag_error=1;
		printf("BBC1 messedUP\n");
	}
	free(packet);
	return messed_up_msg;		
}


unsigned char* mess_up_bcc2(unsigned char* packet, int size_packet){
	
	unsigned char* messed_up_msg = (unsigned char*) malloc(size_packet);
	unsigned char letter;
	
	memcpy(messed_up_msg, packet, size_packet);
	int perc = (rand() %100)+1;
	
	if(perc<= ERROR_PERCENTAGE_BCC2){
		//change data to have error in bcc2
		int i = (rand() % (size_packet-5))+4;
		do{
			letter = (unsigned char) ('A' + (rand()%256));
		}while(letter == messed_up_msg[i]);
		messed_up_msg[i] = letter;
	}
	free(packet);
	return messed_up_msg;
}
