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
	//int fd = 1;

	if(fd>0){
			if(strcmp("w", argv[2])==0){

				 int filesize ;
				unsigned char * filename = "pinguim.gif";
				unsigned char * start_packet;
				unsigned char * end_packet;
				FILE *fileToSend = fopen(filename,"rb");
				if(get_file_size(fileToSend,&filesize) == 1)
				{
					return 1;
				}
				/*int startpacket_size = create_STARTEND_packet(start_packet,filename,filesize,1);
				send_message(&fd,start_packet,startpacket_size);
				handle_readfile(fileToSend,fd,128);
				int endpacket_size = create_STARTEND_packet(end_packet,filename,filesize,0);
				send_message(&fd,end_packet,endpacket_size);*/

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
	}else 
	{
		printf("Error opening serial port\n");
		return 1;
	}

}


int get_file_size(FILE *ptr_myfile, int* filesize){
	struct stat buf;
	int fileDescriptor = fileno(ptr_myfile);

	if(fstat(fileDescriptor,&buf) < 0){

		printf("Error getting file info\n");
		return 1;

	}

	*filesize =  buf.st_size;
	return 0;

}

int create_STARTEND_packet(unsigned char* start_packet,unsigned char* filename,int filesize,int type)
{
	int length_filename = sizeof(filename)/sizeof(filename[0]);
	unsigned char * filesize_char;
	memcpy(filesize_char,(unsigned char *) filesize,sizeof(int));
	int length_filesize = sizeof(filesize_char)/sizeof(filesize_char[0]);

	start_packet = (unsigned char *) malloc(length_filename+length_filesize+5);
	if(type == 1)
	start_packet[0] = 0x02;
	else {
		start_packet[0] = 0x03;
	}
	start_packet[1] = 0x00;
	start_packet[2] = (unsigned char) length_filesize;
	int i = 0;
	int j = 3;
	for(i; i < length_filesize; i++,j++){
		start_packet[j] = filesize_char[i];
	}
	start_packet[j] = 0x01;
	j++;
	start_packet[j] = (unsigned char) length_filename;
	j++;
	i=0;
	for(;i < length_filename; i++,j++)
	{
		start_packet[j] = filename[i];
	}

	return sizeof(start_packet)/sizeof(start_packet[0]);

}



void handle_readfile(FILE*fp,int port,int sizetoread)
{
	unsigned char* data = malloc(sizetoread);
	//FILE * newfile = fopen("penguin.gif","wb");

	fseek(fp,0,SEEK_SET);
	while(!feof(fp))
	{
		int res = 0;
		res = fread(data,sizeof(unsigned char),sizetoread,fp);
		if(res > 0)
		{
			//handle_writefile(newfile,data,sizetoread);
			send_message(&port,data,sizeof(data)/sizeof(data[0]));
		}
		
	}
}

void handle_writefile(FILE * fp,unsigned char* data,int sizetowrite){
	
	fseek(fp,0,SEEK_END);
	fwrite(data,sizeof(unsigned char),sizetowrite,fp);
}


