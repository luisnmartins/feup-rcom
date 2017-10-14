#include "application_layer.h"



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
		printf("CONNECTED\n");
	}

	LLCLOSE(&fd);

}
