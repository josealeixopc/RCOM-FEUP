/*Non-Canonical Input Processing*/
#include "utils.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define TRANSMITTER 1
#define RECEIVER 0

#define RR 0x01
#define REJ 0x05 //todo mudar isto

#define MAX_SIZE 255

#define DEBUG 1


volatile int STOP=FALSE;

struct termios oldtio,newtio;

struct applicationLayer {
	int fileDescriptor; /*Descritor correspondente à porta série*/
	int status;	/*TRANSMITTER | RECEIVER*/
};

struct linkLayer {
	char port[20]; /*Dispositivo /dev/ttySx, x = 0, 1*/
	int baudRate;	/*Velocidade de transmissão (no clue) ??!?*/
	unsigned int sequenceNumber;   /*Número de sequência da trama: 0, 1*/
	unsigned int timeout;	/*Valor do temporizador: 1 s*/
	unsigned int numTransmissions; /*Número de tentativas em caso de falha*/
	char frame[MAX_SIZE];	/*Trama*/
};

struct applicationLayer appL;
struct linkLayer linkL;

void receive_set(int fd){
	printf("do later");
}

int send_cicle(int fd, char * msg){
	int tries = 3; //number of tries to receive feedback
	int res;
	while(tries)
	{
		res = write(fd,linkL.frame,5);	// writes the flags
		if(DEBUG)
			printf ("%d bytes written.\n", res);
		sleep(3);	// waits 3 seconds (use SIGALARM?)
		res = read(fd,msg,5);	// read feedback
		if(res >= 1)	
			break;
		else 
			printf("No feedback!\n");

		
		if(DEBUG)		
			printf ("I'm inside the loop!\n");

		tries--;

	}

	if(DEBUG)
		printf("I'm outside the feedback loop!\n");

	printf ("%d bytes received.\n", res);
	return res;
}


void send_set(int fd){
	int tries = 3; //number of tries to receive feedback
	int res;
	char * buf = (char *) malloc(5*sizeof(char));
	res = send_cicle(fd, buf);
    
    //todo ver isto dp
    
	if(buf[2] == C_UA && res >=1){
		printf("success!\n");
		linkL.frame[2] = RR;
	}
	else {
		printf("error\n");
	}

	printf("0x%x\n", buf[2]);

	free(buf);
}


int llopen(){
	int fd;
	unsigned char trama_su[5];
	
	trama_su[0] = FLAG;
	trama_su[1] = A_SND;
	trama_su[2] = C_UA;
	trama_su[3] = A_SND^C_UA;
	trama_su[4] = FLAG;
	if(appL.status == TRANSMITTER){
		trama_su[2] = C_SET;
		trama_su[3] = A_SND^C_SET;
	}

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
	
	
    fd = open(linkL.port, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(linkL.port); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 3;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */


  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) prÃ³ximo(s) caracter(es)
  */

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");
	
	strcpy(linkL.frame, trama_su);
	
	if(appL.status == TRANSMITTER)send_set(fd);
	else receive_set(fd);
	
	return fd;
}

void close_set(int fd){
	int tries = 3; //number of tries to receive feedback
	int res;
	char buf[MAX_SIZE];
	linkL.frame[2] = C_DISC;

	res = send_cicle(fd, buf);

	if(buf[2] == C_DISC){
			printf("read disconnect success");
			linkL.frame[2] = C_UA;
			res = write(fd, linkL.frame, 5);
		}

	if(DEBUG)
		printf("I'm outside the feedback loop!\n");
}

int llclose(int fd){
	/*
	if(appL.status == TRANSMITTER) 
	else do do chica chica boo boo
	*/
	sleep(2);
	
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
}

int main(int argc, char** argv)
{
    
	//todo mudar isto para ter 3 argumentos (nao me apetece por agr lol :D)
    if ((argc < 3) ||
      ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
       (strcmp("/dev/ttyS1", argv[1]) != 0))){
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
 	}

 	if(argv[2] == 't') appL.status = TRANSMITTER; //mudar dp
 	else if(arv[2] == 'r') appL.status = RECEIVER;
 	else{
 		printf("2nd argument must be t(transmitter) our r (receiver)");
 		exit(2);
 	}

	strcpy(linkL.port, argv[1]);
	
	
	int fd = llopen();
	
	llclose(fd);


    return 0;
}
