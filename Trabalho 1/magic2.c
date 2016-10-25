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
#include <signal.h>
#include <unistd.h>

#define TRANSMITTER 1
#define RECEIVER 0

#define RR 0x01
#define REJ 0x05 //todo mudar isto

#define DEBUG 1

volatile int STOP=FALSE;

int flag = 1; /* 1 if an alarm has not been set and a SET needs to be sent. 0 if it's waiting for a response*/
int numOfTries = 1; /* number tries made to send SET */


struct termios oldtio,newtio;
struct applicationLayer appL;
struct linkLayer linkL;


// ALARM HANDLER

void alarmHandler()
{
	if (DEBUG)
	{
		printf("handled alarm. Try number: %d\n", numOfTries);
	}
	flag = 1;
	numOfTries++;
}

// BEGIN OF STUFFING FUNCTIONS

/*
    Returns the number of bytes that was either a FLAG or a ESCAPE and have been altered.
*/

// END OF STUFFING FUNCTIONS

// [TRANSMITTER] Function to send a message with timeout and receive response from receiver
// @return Result of read call
int send_cycle(int fd, unsigned char * sendMsg,int size){

	(void)signal(SIGALRM, alarmHandler); /* sets alarmHandler function as SIGALRM handler*/

	int writtenChars = 0;

	flag = 1;
	numOfTries = 0; // tries to send 3 times each message

	//Cycle that sends the SET bytes, while waiting for UA
	while(numOfTries < MAX_TRIES){
		if(flag)
		{
			writtenChars = write(fd, sendMsg, size); // writes the flags

			if(DEBUG)
			{
				printf("Wrote msg: ");
				printHexArray(sendMsg, 5);
			}
			if(writtenChars > 1 ) break;
			alarm(3); /* waits 3 seconds, then activates a SIGALRM */
			flag = 0; /* doesn't resend a signal until an alarm is handled */
		}
	}

	return writtenChars;
}

// [RECEIVER] Receive a set frame and verify it by sending UA response

void receive_set(int fd){

  	unsigned char receivedSET[5];	// array to store SET bytes

    STOP = FALSE;

	//CYCLE
    while (STOP==FALSE)  // loop for input
	  {

		int res = read (fd,receivedSET, sizeof(receivedSET)); // reads the flag value

		if(DEBUG)
		{
			printf ("%d bytes received\n", res);
			printHexArray(receivedSET,5);
		}

    	if (res >= 1)
		  {
        if (DEBUG)
          printf ("badSET = %d\n", badSET(receivedSET));



        if(!badSET(receivedSET))
        {
          res = write(fd, UA, 5); // send response
          STOP=TRUE;	//end cycle

          if(DEBUG)
          {
            printf("Sent response UA.\n");
            printf("%d bytes written\n", res);
          }
        }

        break;
    	}
	}
}

// [TRANSMITTER] Try to send set flag 3 times and receive UA response

void send_set(int fd){

	int res;
	unsigned char msg[5];

	res = send_cycle(fd, SET, 5);
    if(res == -1) {
    	printf("deu erro a enviar!");
    	return;
    }

	res = read(fd, msg, 5);
	if (res < 1)
	{
		printf("ERROR: no message received.\n");
	}
	if(badUA(msg)) {
		printf("ERROR: bad UA received.\n");
		exit(-1);
	}

	else
	{
		if(DEBUG)
		{
		printf("Received valid UA.\n");
		printHexArray(msg, 5);
		}
	}

}

//[TRANSMITTER AND RECEIVER] Sets the connection up between devices

void initTrama(){
	unsigned char trama_su[5];  // open frame
	trama_su[0] = FLAG;
	trama_su[1] = A_SND;
	trama_su[2] = C_UA;
	trama_su[3] = (A_SND^C_UA);
	trama_su[4] = FLAG;
	if(appL.status == TRANSMITTER){
		trama_su[2] = C_SET;
		trama_su[3] = (A_SND^C_SET);
	}

	memcpy(linkL.frame, trama_su, 5);
}

int llopen(){
	int fd;
  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
	initTrama();
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

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */


  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) prÃ³ximo(s) caracter(es)
  */

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    if(DEBUG)
    {
      printf("New termios structure set\n");
    }

	if(appL.status == TRANSMITTER)
		send_set(fd);
	else
		receive_set(fd);

	return fd;
}

int close_ua(int fd){
	int res;
	tcflush(fd, TCIOFLUSH);
	
	unsigned char received[5];	// array to store SET bytes
	unsigned char * DISC = UA;
	DISC[2] = C_DISC;
	DISC[3] = A_SND^C_DISC;
	STOP = FALSE;

//CYCLE
	while (STOP==FALSE)  // loop for input
	{

	res = read (fd,received, 5); // reads the flag value

	if(DEBUG)
	{
		printf ("%d bytes received\n", res);
		printHexArray(received, 5);
	}

		if (res >= 1 && !badDisc(received))
		{
			printf("bruno bates mal dos cornos");
			res = write(fd, DISC, 5); // send response
			if(res > 1){
				unsigned char last[5];
				res = read(fd, last, 5);
				printHexArray(last, 5);
				if(!badUA(last)) printf("carago deu!\n");
			}
          		STOP=TRUE;
			return 0;
		}
	}
	return -1;
}

void close_set(int fd){
	int res;
	tcflush(fd, TCIOFLUSH);
	unsigned char * DISC = SET;
	DISC[2] = C_DISC;
	DISC[3] = A_SND^C_DISC;
	unsigned char buf[5];

	res = send_cycle(fd, DISC,5);
	if(res < 1)printf("error sending disc \n");
	res = read(fd, buf, 5);
	if(!badDisc(DISC)){
			printf("read disconnect success");
			res = send_cycle(fd, UA, 5);
			if(res > 1) printf("\nsent final handshake\n");
			return;

	}

	if(DEBUG)
		printf("I'm outside the feedback loop!\n");
}


int llclose(int fd){

	if(appL.status == TRANSMITTER) close_set(fd);
	else close_ua(fd);


    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);
		return 0;
}

int main(int argc, char** argv)
{

	//todo mudar isto para ter 3 argumentos (nao me apetece por agr lol :D)
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

	int fd = llopen();

	//sleep(2);
/*
	if(appL.status == TRANSMITTER)
	{
		char msg[10]= {FLAG, FLAG, ESCAPE, 1,69,'f','u','c','k','k'};
		printf ("Message to send: ");
		printHexArray(msg, strlen(msg));
		llwrite(fd, msg, strlen(msg));
	}

	else
	{
		char msg[MAX_SIZE];
		llread(fd, msg);
		printf ("Received message: ");
		printHexArray(msg, 15);
	}
*/

	return llclose(fd);
}
