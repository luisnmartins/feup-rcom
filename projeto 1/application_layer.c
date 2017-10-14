#include "application_layer.h"

int start_sending_msg(int *fd){

	char t_msg = 0x00;
	char l_msg = 0x01;
	char msg_size = 0x06;
	char i_start[] = {C_START, t_msg, l_msg, msg_size};




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
			if(strcmp(argv[2], "w") == 0){
				start_sending_msg(&fd);
			}
			else{
				read_start_msg(&fd);
			}
	}

	LLCLOSE(&fd);

}
