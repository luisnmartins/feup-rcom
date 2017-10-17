#include "application_layer.h"

/*int start_sending_msg(int *fd){

	char t_msg = 0x00;
	char l_msg = 0x01;
	char msg_size = 0x06;
	char i_start[] = {C_START, t_msg, l_msg, msg_size};

}*/
int send_message(int* fd, unsigned char* msg, int length){
	int data_length = data_package_constructor(msg, length);
	LLWRITE(fd, msg, data_length);
}

int get_message(int* fd, unsigned char* msg){
	
	msg = (unsigned char*) malloc(1);	
	int length;
	do{
		length = LLREAD(fd, msg);
	}while(length == -1);

	/*if(length > 0){
	
	int i=0;
	for(i; i<length; i++){
		printf("MSG: %x\n",msg[i]);
	}
	}*/
}



int data_package_constructor(unsigned char* msg, int length){

		unsigned char* data_package = (unsigned char*) malloc(length+4);

		unsigned char c = 0x01;
		unsigned char n = 0x00;
		int l2 = length/255;
		int l1 = length%255;

		data_package[0] = c;
		data_package[1] = n;
		data_package[2] = l2;
		data_package[3] = l1;

		int i=0;
		for(i; i<length; i++){
			data_package[i+4] = msg[i];
		}

		length = length+4;
		msg = (unsigned char*) realloc(msg, length);
		memcpy(msg, data_package, length);
		free(data_package);
		return length;
}


int main(int argc, char** argv){


	if ( (argc < 3) ||
  	   ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	    (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

    if(strcmp("r",argv[2]) != 0 && strcmp("w",argv[2]) != 0){
    	printf("Usage:\tinvalid read/write mode. a correct mode (r / w).\n\tex: nserial /dev/ttyS1 r\n");
      exit(1);
    }


  int fd = LLOPEN(argv[1], argv[2]);


	if(fd>0){
			if(strcmp("w", argv[2])==0){
				unsigned char* message = (unsigned char*) malloc(9*sizeof(unsigned char));
				message[0] = 0x48;
				message[1] = 0x45;
				message[2] = 0x4c;
				message[3] = 0x4c;
				message[4] = 0x4F;
				message[5] = 0x00;
				message[6] = 0x48;
				message[7] = 0x7e;
				message[8] = 0x7d;
				int length = 9;
				send_message(&fd, message, length);
			}
			else if(strcmp("r", argv[2])==0){
				unsigned char* read_msg;
				int msg_length = get_message(&fd, read_msg);

			}
			/*
			unsigned char* stuffed_message = byte_stuffing(message, &length);
			int i=0;
			for(i; i<length; i++){
				printf("STUFFED: %x ",stuffed_message[i]);
			}
			unsigned char* destuffed_message = byte_destuffing(stuffed_message, &length);
			i=0;
			printf("\n\n\n");
			for(i; i<length; i++){
				printf("DES: %x ",destuffed_message[i]);*/





			/*printf("MENSAGEM C STUFFING: %s\n", message);*/
			/*
			byte_destuffing(message);
			printf("%s\n", message);
			/*if(strcmp(argv[2], "w") == 0){
				start_sending_msg(&fd);
			}
			else if(strcmp(argv[2], "r") == 0){
				read_start_msg(&fd);
			}
			else{
				exit(-1);
			}*/
			LLCLOSE(&fd);
	}

}
