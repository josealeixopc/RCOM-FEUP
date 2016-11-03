#include "application.h"

ApplicationLayer appL;
LinkLayer linkL;

Stats stats = {
  .framesSent = 0,
  .framesReceived = 0,
  .numRR = 0,
  .numREJ = 0,
  .numTimeouts = 0
};

int frame_size_default = 100;
struct termios oldtio;
char nome_ficheiro_enviar[MAX_SIZE] = "pinguim.gif";

void displayStats()
{
  printf("*********** FINAL STATS ************\n");

  if(appL.status == TRANSMITTER)
  {
    printf("Number of frames sent: %u\n", stats.framesSent);
    printf("Number of timeouts: %u\n", stats.numTimeouts);
    printf("Number of RRs: %u\n", stats.numRR);
    printf("Number of REJs: %u\n", stats.numREJ);
  }
  else
  {
    printf("Number of frames received: %u\n", stats.framesReceived);
    printf("Number of RRs: %u\n", stats.numRR);
    printf("Number of REJs: %u\n", stats.numREJ);
  }
}

void loadFile(char * filename);
void receiveFile();

void menu_cycle(){

	int status = -1;
	while((status = startmenu()) == -1);
	appL.status = (status == 1) ? TRANSMITTER : RECEIVER;

	while((status = selectPort()) == -1);
	(status == 1) ? strcpy(linkL.port,"/dev/ttyS0") : strcpy(linkL.port,"/dev/ttyS1");

	while((status = baudarecheck()) == -1);
	linkL.baudRate = status;


	if(appL.status == TRANSMITTER){
	while((status = selectMaxSize()) == -1);
	frame_size_default = status;

	while((status = selectTimeout()) == -1);
	linkL.timeout = status;
	while((status = selectAttempts()) == -1);
	linkL.numTransmissions = status;

	char * nome = malloc(sizeof(char *) *  MAX_SIZE);
	getfilename(nome);
	strcpy(nome_ficheiro_enviar, nome);
	free(nome);
	}
}

int main(/*int argc, char** argv*/)
{
/*
    if ((argc < 3) ||
      ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
       (strcmp("/dev/ttyS1", argv[1]) != 0))){
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1 <t or r>\n");
    exit(1);
 	}

 	if(strcmp(argv[2],"t") == 0) appL.status = TRANSMITTER; //mudar dp
 	else if(strcmp(argv[2],"r") == 0) appL.status = RECEIVER;
 	else{
 		printf("2nd argument must be t(transmitter) our r (receiver)");
 		exit(2);
 	}

	strcpy(linkL.port, argv[1]);

	if(DEBUG)
	{
		printf ("linkL.port in main: %s\n", linkL.port);
	}
*/ // funcoes historicas #laig falta apenas fazer o timeout e o cenas

	(void)signal(SIGALRM, alarmHandler);

	printf("Going to start menu .. \n");

	menu_cycle();

	printf("Started execution...\n");

	llopen(&appL, &linkL, &oldtio);

	printf("llopen() done with success!\n");


  //Pikachu EX FA 150hp

  if(ERROR_SIMULATION)
  {
    srand(time(NULL));
  }

  if(appL.status == TRANSMITTER){
    printf("Enviar ficheiro!\n");
    loadFile(nome_ficheiro_enviar);
  }
  else if(appL.status == RECEIVER){
    printf("Receber ficheiro!\n");
    receiveFile();
  }

	llclose(&appL, &oldtio);

	printf("llclose() done with success!\n");

  displayStats();

  return 0;

}



long int verifyControlData(unsigned char * data ,char * file_name, int value){
  if(data[0] != value)
  {
    //printf("Control Data corrupted!\n");
    return -1;
  }
  if(data[1] != FILE_SIZE)
  {
    printf("FILE_SIZE corrupted!\n");
    return -1;
  }
  int length_camp = (int) data[2];
  long int file_size_expected = *((uint32_t *) &data[3]);

  if(data[3 + length_camp] != NAME_SIZE){
    printf("NAME FLAG corrupted!\n");
    return -1;
  }

  int length_name = (int) data[4 + length_camp];
  //free(file_name);
  //file_name = malloc(length_name);
  memcpy(file_name,&data[5 + length_camp],length_name);
  printf("File name is %s, and has %ld bytes \n", file_name, file_size_expected);
  return file_size_expected;
}

int writeToFile(unsigned char * trama, int file_d, int * n_trama){
  if(trama[0] != DATA_CONTROL){
    printf("Not a Data package!\n");
    return 0;
  }
  if((*(n_trama)%255) != trama[1]){
    printf("Incorrect Trama, next try please\n");
    return 0;
  }
  *(n_trama) += 1;
  long int length_of_trama = 256 * trama[2] + trama[3]; // K = 256 * L2 + L1

  write(file_d, &trama[4], length_of_trama-4);

  return length_of_trama;

}

void receiveFile(){

  //ll_read(cenas);
  size_t size_trama = 1;
  unsigned char * initialData = malloc(sizeof(unsigned char *) * size_trama);
  char * file_name = malloc(sizeof( char *) * MAX_SIZE);
  llread(appL.fileDescriptor, initialData, &size_trama, &linkL, &stats);


  long int file_size_expected = verifyControlData(initialData, file_name, CRUISE_CONTROL);
  if(file_size_expected == -1){
    printf("Error receiving control pack\n");
    free(initialData);
    return;
  }
  free(initialData);

  int file_d = open(file_name, O_WRONLY | O_TRUNC | O_CREAT, 0660);
	long int size_received = 0;
  int * n_trama = malloc(sizeof(int *)); //possivelmente a coisa mais estupida que ja fiz na vida
  * n_trama = 0;
  int received = FALSE;

  while(received == FALSE){
    initialData = malloc(sizeof(unsigned char *) *  21474836 );
    size_trama = 1;


    int i = llread(appL.fileDescriptor, initialData, &size_trama, &linkL, &stats);

    if(i == 0){
      if(verifyControlData(initialData, file_name, END_CONTROL) > 1)
      {
        printf("done receiving file! wasnt expecting this ;)\n");
        received = TRUE;
      }
      else{
        size_received += writeToFile(initialData,file_d, n_trama);
      }

    }

    free(initialData);

  }
  printf("Final result =%ld, expecetd = %ld\n",size_received,file_size_expected);
  free(n_trama);
  free(file_name);
	close(file_d);
}


void createControlPackage(unsigned char * pack, long int size,  char * filename, int valor){
    pack[0] = valor; //C
  	pack[1] = FILE_SIZE; //TYPE
  	pack[2] = 4;//ficar em 32 bits filhao ;) LENGTH
  	int i;
  	for(i = 3;i < 7; i++){ //value
  		pack[i] = 0xFF & (size >> (8 * (i-3)));
  	} //todo isto vai copiado da net #wikipedia
  	pack[7] = NAME_SIZE; //TYPE
  	pack[8] = strlen(filename) + 1; //LENGTH
  	memcpy(&pack[9], filename,pack[8]); //VALUE
}


void createDataPackage(unsigned char * pack, unsigned char * info_trama, int n_trama, long int size_trama){
  pack[0] = DATA_CONTROL;
  pack[1] = n_trama;
  pack[2] = size_trama / 256; //l2
  pack[3] = size_trama % 256; //l1

  memcpy(&pack[4], info_trama, size_trama);
}

void loadFile(char * filename){
	int file_d;
	struct  stat * file_stat = malloc(sizeof(struct stat));
	file_d = open(filename, O_RDONLY);
  if(open < 0){
    perror("file not found!");
    exit(-1);
  }
	fstat(file_d, file_stat);
	off_t size = file_stat->st_size;
	printf("Size of file %ld", size);
	unsigned char * file_inside = malloc(size);
	read(file_d, file_inside, size);

  unsigned char * packageInit = malloc(10 + strlen(filename) + 1);
  createControlPackage(packageInit,size,filename,CRUISE_CONTROL);
  //write(appL.fileDescriptor, packageInit, 11 + strlen(filename));
  while(1)
  {
    int value;

    value =llwrite(appL.fileDescriptor, packageInit, 11 + strlen(filename), &linkL, &stats);

    printf("%d\n", value);

    if(value == 0)
    {
      break;
    }
    else
    {
      continue;
    }
  }


	//cenas para enviar copotes de controlo
	//definir de quanto espaço enviar por enquanto esta o valor hardcoded
  int frame_size = frame_size_default;//4 tags
	int bytes_sent = 0;
	int n_trama = 0;
  free(packageInit);

  while(bytes_sent < size-4){
		int send_size = (size - bytes_sent) > frame_size ? frame_size : size-bytes_sent;
    unsigned char * trama = malloc(sizeof(unsigned char *) * (send_size+16));
    createDataPackage(trama, &file_inside[bytes_sent], (n_trama % 255), send_size);
    llwrite(appL.fileDescriptor, trama, send_size, &linkL, &stats);
    //sleep(1);
    free(trama);
    bytes_sent += send_size-4;
    printf("trama %d\n", n_trama);
    n_trama++;
	}
  packageInit = malloc(10 * strlen(filename) + 1);
  createControlPackage(packageInit,size,filename,END_CONTROL);
  llwrite(appL.fileDescriptor, packageInit, 11 + strlen(filename), &linkL, &stats);
  free(packageInit);
	//fechar copotes de controlo THE END :D

	close(file_d);
	free(file_inside);
  free(file_stat);
}
