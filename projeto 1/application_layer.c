#include "application_layer.h"


int is_start = FALSE;
static file_info file;
static application_layer app_info;


int send_message(unsigned char* msg, int length){
	int res;
	if(is_start == FALSE){
		unsigned char* data_package = data_package_constructor(msg, &length);
		printf("DATA PACKAGE\n");
		res= LLWRITE(app_info.file_descriptor, data_package, &length);
		printf("RES LLWIRTE: %d\n", res);

	}
	else{
		is_start= FALSE;
		res = LLWRITE(app_info.file_descriptor, msg, &length);
	}

	if (res == FALSE){

		return FALSE;
	}

	return TRUE;

}


unsigned char* get_message(){
	int length;
	unsigned char* readed_msg;
	unsigned char* only_data;
		readed_msg = LLREAD(app_info.file_descriptor, &length);
		if(readed_msg == NULL || readed_msg[0] == DISC){
			return readed_msg;
		}
	switch(readed_msg[0]){
		case 0x02:
			start_message(readed_msg);
			break;
		case 0x01:
			only_data = get_only_data(readed_msg, &length);
			handle_writefile(only_data,length);
			break;
		case 0x03:
			verify_end(readed_msg);
			break;

	}


	return readed_msg;

}

unsigned char* get_only_data(unsigned char* readed_msg, int* length){
	int i=4;
	int j=0;
	unsigned char* only_data = (unsigned char*) malloc(*length-4);
	for(; i<*length; i++, j++){
			only_data[j] = readed_msg[i];
	}
	*length = *length-4;
	free(readed_msg);
	return only_data;
}


int verify_end(unsigned char* msg){
	int i=0;
	unsigned char file_size[4];
	int file_size_size = msg[2];
	int file_size_total;

	for(; i<file_size_size; i++){
		file_size[i] = msg[i+3];
	}
	file_size_total = (file_size[0] <<24) | (file_size[1] << 16) | (file_size[2] << 8) | (file_size[3]);

	if(file_size_total == file.filesize && file_size_total == get_file_size()){
				printf("Received file size is correct\n");
				return TRUE;
	}
	else{
				printf("Received file is probably corrupted\n");
				return FALSE;
	}
	return FALSE;
}


void start_message(unsigned char* msg){

	int i=0;
	int j=0;
	unsigned char filesize[4];
	int filename_size;
	if(msg[1] == 0x00){
		int filesize_size = msg[2];
		for(; i<filesize_size; i++){
			filesize[i] = msg[i+3];
		}
		file.filesize = (filesize[0] <<24) | (filesize[1] << 16) | (filesize[2] << 8) | (filesize[3]);
	}
	i += 3;
	if(msg[i] == 0x01){
		i++;
		filename_size = msg[i];
		i++;
		file.filename = (char*) malloc (filename_size+1);
		for(; j<filename_size; j++,i++){
			file.filename[j] = msg[i];
		}
		file.filename[filename_size] = '\0';
	}
	i=0;
	for(; i<filename_size; i++){
		printf("FILENAME: %c\n", file.filename[i]);
	}

	file.fp = fopen((char*)file.filename,"wb");

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
		for(; i<*length; i++){
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
	srand(time(NULL));

	app_info.status = argv[2];
	app_info.file_descriptor = LLOPEN(argv[1], app_info.status);

	if(app_info.file_descriptor>0){
			if(strcmp("w", app_info.status)==0){
				if(argv[3] == NULL || argv[4] == NULL){
					printf("You need to specify the file to send and the size to read\n");
					exit(-1);
				}

				file.filename = (char*) argv[3];
				app_info.size_to_read = atoi(argv[4]);
				int start_end_max_size;
				unsigned char* start_packet;
				unsigned char* end_packet;

				file.fp = fopen((char*)file.filename,"rb");
				if(file.fp == NULL)
				{
					printf("invalid file!\n");
					exit(-1);
				}
				if((file.filesize = get_file_size()) == -1)
				{
					return FALSE;
				}
				start_end_max_size = 2*(strlen(file.filename) + 9 ) + MAXSIZELINK; //max size for start/end package;
				start_packet = (unsigned char*) malloc(start_end_max_size);

				int start_created_size = create_STARTEND_packet(start_packet, START_PACKET_TYPE);

				if(start_created_size == -1){
					printf("Error creating start packet\n");
					exit(-1);
				}

				int i=0;
				for(;i < start_created_size;i++){
					printf("Trama Start: %x\n",start_packet[i]);
				}

				is_start = TRUE;
				if (send_message(start_packet,start_created_size) == FALSE){
					LLCLOSE(app_info.file_descriptor, -1);
				}

				printf("IS START: %d\n", is_start);
				handle_readfile();
				printf("FINISH FILE IS GOING TO LAST PACKET\n");

				is_start = TRUE;
				end_packet = (unsigned char*) malloc(start_end_max_size);
				int endpacket_size = create_STARTEND_packet(end_packet,END_PACKET_TYPE);
				i=0;
				for(;i < endpacket_size;i++){
					printf("Trama END: %x\n",end_packet[i]);
				}
				is_start=TRUE;
				if(send_message(end_packet,endpacket_size) == FALSE){
					LLCLOSE(app_info.file_descriptor, -1);
				}

				LLCLOSE(app_info.file_descriptor,WRITER);


			}
			else if(strcmp("r", app_info.status)==0){
				unsigned char* msg;
				unsigned char null_val[] = {0xAA};
				do{
					msg = get_message();

					if(msg == NULL)
					{
						msg = null_val;
					}
				}while(msg[0] != DISC);

			}
	}else
	{
		printf("Error opening serial port\n");
		return 1;
	}
	return 0;

}


int get_file_size(){

	fseek(file.fp, 0L, SEEK_END);
	int filesize = (int) ftell(file.fp);
	if(filesize == -1)
		return -1;
	fseek(file.fp, 0L, SEEK_SET);
	return filesize;

}

int create_STARTEND_packet(unsigned char* packet, int type){
	int i = 0;
	int j = 3;
	unsigned char filesize_char[4];
	unsigned int filename_length = (unsigned int) strlen(file.filename);

	//convert filesize to and unsigned char array
	filesize_char[0] = (file.filesize >> 24) & 0xFF;
	filesize_char[1] = (file.filesize >> 16) & 0xFF;
	filesize_char[2] = (file.filesize >> 8) & 0xFF;
	filesize_char[3] = file.filesize & 0xFF;

	//size of filesize unsigned char array, normally is 4
	int length_filesize = sizeof(filesize_char)/sizeof(filesize_char[0]);


	if(type == START_PACKET_TYPE)
		packet[0] = 0x02;
	else if(type == END_PACKET_TYPE) {
		packet[0] = 0x03;
	}
	else{
		return -1;
	}
	packet[1] = 0x00;
	packet[2] = length_filesize;


	//put filesize unsigned char array in packet array
	for(; i < length_filesize; i++,j++){
		packet[j] = filesize_char[i];
	}

	packet[j] = 0x01;
	j++;
	packet[j] =  filename_length;

	j++;
	i=0;
	for(;i < filename_length; i++,j++)
	{
		packet[j] = file.filename[i];
	}


	return j;

}



void handle_readfile()
{
	unsigned char* data = malloc(app_info.size_to_read);
	//FILE * newfile = fopen("penguin.gif","wb");

	fseek(file.fp,0,SEEK_SET);
	while(TRUE)
	{

		int res = 0;
		res = fread(data,sizeof(unsigned char),app_info.size_to_read,file.fp);
		if(res > 0)
		{
			printf("RES: %d\n", res);
			//handle_writefile(newfile,data,sizetoread);
			if(send_message(data,res) == FALSE){
				LLCLOSE(app_info.file_descriptor, -1);
				exit(-1);
			}
			printf("GO FOR NEXT PACKAGE\n");
		}
		if(feof(file.fp))
			break;

	}
	printf("END FILE\n");

}

void handle_writefile(unsigned char* data,int sizetowrite){

	fseek(file.fp,0,SEEK_END);
	fwrite(data,sizeof(unsigned char),sizetowrite,file.fp);
}
