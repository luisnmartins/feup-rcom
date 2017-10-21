#include "application_layer.h"

/*int start_sending_msg(int *fd){

	char t_msg = 0x00;
	char l_msg = 0x01;
	char msg_size = 0x06;
	char i_start[] = {C_START, t_msg, l_msg, msg_size};

}*/

int is_start = FALSE;
unsigned char* filename;
int filesize_total;
FILE * created_file;

int send_message(int* fd, unsigned char* msg, int length){
	printf("SEND MESSAGE RRES\n");
	int res;
	if(is_start == FALSE){
		unsigned char* data_package = data_package_constructor(msg, &length);
		res= LLWRITE(fd, data_package, &length);
		printf("RES LLWIRTE: %d\n", res);

	}
	else{
		is_start= FALSE;
		res = LLWRITE(fd, msg, &length);

	}

	if (res == FALSE){

		return FALSE;
	}

	return TRUE;

}


unsigned char* get_message(int* fd){
	int length;
	unsigned char* readed_msg;
	unsigned char* only_data;
		readed_msg = LLREAD(fd, &length);
		if(readed_msg == NULL || strcmp("finish",readed_msg) == 0){
			return readed_msg;
		}
	switch(readed_msg[0]){
		case 0x02:
			start_message(readed_msg);
			break;
		case 0x01:
			only_data = get_only_data(readed_msg, &length);
			handle_writefile(created_file,only_data,length);
			break;
		case 0x03:
			verify_end(readed_msg, created_file);
			break;

	}


	return readed_msg;

}

unsigned char* get_only_data(unsigned char* readed_msg, int* length){
	int i=4;
	int j=0;
	unsigned char* only_data = (unsigned char*) malloc(*length-4);
	for(i; i<*length; i++, j++){
			only_data[j] = readed_msg[i];
	}
	*length = *length-4;
	free(readed_msg);
	return only_data;
}


int verify_end(unsigned char* msg, FILE* fp){
	int i=0;
	int j=0;
	unsigned char file_size[4];
	int file_size_size = msg[2];
	int file_size_total;
	int real_filesize;

	for(i; i<file_size_size; i++){
		file_size[i] = msg[i+3];
	}
	file_size_total = (file_size[0] <<24) | (file_size[1] << 16) | (file_size[2] << 8) | (file_size[3]);

	if(file_size_total == filesize_total){
		if(get_file_size(fp,&real_filesize) != 1){
			if(file_size_total == real_filesize){
				printf("Received file size is correct\n");
				return TRUE;
			}
			else{
				printf("Received file is probably corrupted\n");
				return FALSE;
			}
		}
	}

}


void start_message(unsigned char* msg){

	int i=0;
	int j=0;
	unsigned char filesize[4];
	int filename_size;
	if(msg[1] == 0x00){
		int filesize_size = msg[2];
		for(i; i<filesize_size; i++){
			filesize[i] = msg[i+3];
		}
		filesize_total = (filesize[0] <<24) | (filesize[1] << 16) | (filesize[2] << 8) | (filesize[3]);
	}
	i += 3;
	if(msg[i] == 0x01){
		i++;
		filename_size = msg[i];
		i++;
		filename = (unsigned char*) malloc (filename_size+1);
		for(j; j<filename_size; j++,i++){
			filename[j] = msg[i];
		}
		filename[filename_size] = '\0';
	}
	i=0;
	for(i; i<filename_size; i++){
		printf("FILENAME: %c\n", filename[i]);
	}

	created_file = fopen(filename,"wb");

}


unsigned char* data_package_constructor(unsigned char* msg, int* length){

		unsigned char* data_package = (unsigned char*) malloc(*length+4);

		unsigned char c = 0x01;
		unsigned char n = 0x00;
		int l2 = *length/255;
		int l1 = *length%255;

		data_package[0] = c;
		data_package[1] = n;
		data_package[2] = l2;
		data_package[3] = l1;

		int i=0;
		for(i; i<*length; i++){
			data_package[i+4] = msg[i];
		}

		*length = *length+4;
		/*free(msg);*/
		return data_package;
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
				unsigned char * start_packet = malloc(5);
				unsigned char * end_packet = malloc(1);
				FILE *fileToSend = fopen(filename,"rb");
				if(get_file_size(fileToSend,&filesize) == 1)
				{
					return 1;
				}
				printf("TAMANHO DO ENVIAR%d\n",filesize);
				int i=0;
				int startpacket_size = create_STARTEND_packet(start_packet,filename,filesize,1);
				for(;i < startpacket_size;i++){
					printf("Trama Start: %x\n",start_packet[i]);
				}
				is_start = TRUE;
				if (send_message(&fd,start_packet,startpacket_size) == FALSE){
					LLCLOSE(&fd, -1);
				}

				printf("IS START: %d\n", is_start);
				handle_readfile(fileToSend,fd,128);
				printf("FINISH FILE IS GOING TO LAST PACKET\n");
				is_start = TRUE;
				int endpacket_size = create_STARTEND_packet(end_packet,filename,filesize,0);
				i=0;
				for(;i < endpacket_size;i++){
					printf("Trama END: %x\n",end_packet[i]);
				}
				is_start=TRUE;
				if(send_message(&fd,end_packet,endpacket_size) == FALSE){
					LLCLOSE(&fd, -1);
				}

				LLCLOSE(&fd,WRITER);


			}
			else if(strcmp("r", argv[2])==0){
				unsigned char* msg;
				do{
					msg = get_message(&fd);

					if(msg == NULL)
					{

						msg = "null";
					}
				}while(strcmp("finish",msg) != 0);

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
	}else
	{
		printf("Error opening serial port\n");
		return 1;
	}

}


int get_file_size(FILE *ptr_myfile, int* filesize){
	/*struct stat buf;
	int fileDescriptor = fileno(ptr_myfile);

	if(fstat(fileDescriptor,&buf) < 0){

		printf("Error getting file info\n");
		return 1;

	}

	*filesize =  buf.st_size;
	return 0;*/

	fseek(ptr_myfile, 0L, SEEK_END);
	*filesize = ftell(ptr_myfile);
	fseek(ptr_myfile, 0L, SEEK_SET);
	return 0;

}

int create_STARTEND_packet(unsigned char* start_packet,unsigned char* filename,int filesize,int type)
{
	int length_filename = 11;

	unsigned char filesize_char[4];
	filesize_char[0] = (filesize >> 24) & 0xFF;
	filesize_char[1] = (filesize >> 16) & 0xFF;
	filesize_char[2] = (filesize >> 8) & 0xFF;
	filesize_char[3] = filesize & 0xFF;
	int length_filesize = sizeof(filesize_char)/sizeof(filesize_char[0]);

	start_packet = (unsigned char *) realloc(start_packet,length_filename+length_filesize+5);
	if(type == 1)
	start_packet[0] = 0x02;
	else {
		start_packet[0] = 0x03;
	}
	start_packet[1] = 0x00;
	start_packet[2] = length_filesize;
	int i = 0;
	int j = 3;
	for(i; i < length_filesize; i++,j++){
		start_packet[j] = filesize_char[i];
	}

	start_packet[j] = 0x01;
	j++;
	start_packet[j] =  length_filename;

	//printf("ashdsa%x\n",start_packet[j]);
	j++;
	i=0;
	for(;i < length_filename; i++,j++)
	{
		start_packet[j] = filename[i];
	}
	/* i=0;
	 for(; i< length_filename;i++){
		 printf("%c\n",filename[i]);
	 }*/
	return length_filename+length_filesize+5;

}



void handle_readfile(FILE*fp,int port,int sizetoread)
{
	unsigned char* data = malloc(sizetoread);
	//FILE * newfile = fopen("penguin.gif","wb");

	fseek(fp,0,SEEK_SET);
	while(TRUE)
	{

		int res = 0;
		res = fread(data,sizeof(unsigned char),sizetoread,fp);
		if(res > 0)
		{
			printf("RES: %d\n", res);
			//handle_writefile(newfile,data,sizetoread);
			if(send_message(&port,data,res) == FALSE){
				LLCLOSE(&port, -1);
				exit(-1);
			}
			printf("GO FOR NEXT PACKAGE\n");
		}
		if(feof(fp))
			break;

	}
	printf("END FILE\n");

}

void handle_writefile(FILE * fp,unsigned char* data,int sizetowrite){

	fseek(fp,0,SEEK_END);
	fwrite(data,sizeof(unsigned char),sizetowrite,fp);
}
