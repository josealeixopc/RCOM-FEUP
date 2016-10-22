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
int byteStuff(Array* inArray, Array* outArray)
{
    int count = 0;

    for(int i = 0; i < inArray->used; i++)
    {
        if (inArray->array[i] == FLAG)
        {
            insertArray(outArray, ESCAPE);
            insertArray(outArray, (FLAG^XOR_BYTE));
            count++;
        }

        else if (inArray->array[i] == ESCAPE)
        {
            insertArray(outArray, ESCAPE);
            insertArray(outArray, (ESCAPE^XOR_BYTE));
            count++;
        }

        else
        {
            insertArray(outArray, inArray->array[i]);
        }
    }

    return count;
}

/*
    Returns the number of bytes that was converted to either a FLAG or a ESCAPE.
*/
int byteUnstuff(Array* inArray, Array* outArray)
{
    int count = 0;

    for(int i = 0; i < inArray->used; i++)
    {
        if ((inArray->array[i] == ESCAPE) && (inArray->array[i+1] == (FLAG^XOR_BYTE)))
        {
            insertArray(outArray, FLAG);
           
            i++; // to skip next byte evaluation
            count++;
        }

        else if ((inArray->array[i] == ESCAPE) && (inArray->array[i+1] == (ESCAPE^XOR_BYTE)))
        {
            insertArray(outArray, ESCAPE);
            
            i++;
            count++;
        }

        else
        {
            insertArray(outArray, inArray->array[i]);
        }
    }

    return count;
}

// END OF STUFFING FUNCTIONS

// [TRANSMITTER] Function to send a message with timeout and receive response from receiver
// @return Result of read call
int send_cycle(int fd, char* feedback){
	
	(void)signal(SIGALRM, alarmHandler); /* sets alarmHandler function as SIGALRM handler*/

	int writtenChars = 0;
	int res = 0;

	flag = 1;
	numOfTries = 0; // tries to send 3 times each message

	//Cycle that sends the SET bytes, while waiting for UA
	while (numOfTries <= MAX_TRIES)
	{
		if(flag)
		{
			writtenChars = write(fd, linkL.frame, sizeof(linkL.frame)); // writes the flags

			if(DEBUG)
			{
				printf("Wrote linkL.frame: ");
				printHexArray(linkL.frame, 20);
				printf("\n");
			}

			alarm(3); /* waits 3 seconds, then activates a SIGALRM */
			flag = 0; /* doesn't resend a signal until an alarm is handled */
		}

		res = read(fd, feedback, sizeof(feedback)); // read feedback every clock tick

		if (res >= 1) // if it read something
			break;
	}

	if(res < 1)
	{
		printf("ERROR: No response from receiver.\n");
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
			printf ("SET: 0x%x , 0x%x, 0x%x, 0x%x, 0x%x\n", receivedSET[0], receivedSET[1], receivedSET[2], receivedSET[3], receivedSET[4]);
		}	

    	if (res >= 1)
		  {
        if (DEBUG)
          printf ("badSET = %d\n", badSET(receivedSET));

        

        if(!badSET(receivedSET))
        {
          res = write(fd, linkL.frame, sizeof(linkL.frame)); // send response
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
	char msg[5] = {};
  
	res = send_cycle(fd, msg);
    if(res == -1) {
    	printf("deu erro a enviar!");
    	return;
    }

    
	if (res < 1)
	{
		printf("ERROR: no message received.\n");
	}  

	else if(badUA(msg)) {
		printf("ERROR: bad UA received.\n");
		exit(-1);
	}

	else
	{
		if(DEBUG)
		{
		printf("Received valid UA.\n");
		}
	}

}

//[TRANSMITTER AND RECEIVER] Sets the connection up between devices

int llopen(){

	int fd;
	char trama_su[5];  // open frame 
	
	trama_su[0] = FLAG;
	trama_su[1] = A_SND;
	trama_su[2] = C_UA;
	trama_su[3] = (A_SND^C_UA);
	trama_su[4] = FLAG;
	if(appL.status == TRANSMITTER){
		trama_su[2] = C_SET;
		trama_su[3] = (A_SND^C_SET);
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

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
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

    if(DEBUG)
    {
      printf("New termios structure set\n");
    }
	
	memcpy(linkL.frame, trama_su, sizeof(trama_su));

	if(appL.status == TRANSMITTER)
		send_set(fd);
	else 
		receive_set(fd);
	
	return fd;
}

void close_set(int fd){
	int tries = 3; //number of tries to receive feedback
	int res;
	char buf[MAX_SIZE];
	linkL.frame[2] = C_DISC;

	res = send_cycle(fd, buf);

	if(buf[2] == C_DISC){
			printf("read disconnect success");
			linkL.frame[2] = C_UA;
			res = write(fd, linkL.frame, 5);
		}

	if(DEBUG)
		printf("I'm outside the feedback loop!\n");
}

void initializeInformationFrame(Array* frameArray)
{
	insertArray(frameArray, FLAG);
	insertArray(frameArray, A_SND);
	insertArray(frameArray, C_SET);
	insertArray(frameArray, (frameArray->array[1] ^ frameArray->array[2]));
}

void endInformationFrame(Array* frameArray)
{
	char BCC2 = 0x0;

	for(unsigned int i = 4; i < frameArray->used; i++)	// i == 4 so it skips first bytes
	{
		BCC2 = BCC2 ^ (frameArray->array[i]); // verify parity of data bytes
	}

	insertArray(frameArray, BCC2);
	insertArray(frameArray, FLAG);
}

// [TRANSMITTER] Writes a frame of information to send_cycle
// @param fd		File descriptor of the port
// @param buffer	Message to be written
// @param length	Length of the message
// @return 			Number of chars (bytes) written

int llwrite(int fd, char* buffer, int length)
{

	Array msgBuffer; // strcut to store message buffer
	initArray(&msgBuffer, 1);

	copyArray(buffer, &msgBuffer, length); // copy buffer to Array struct
	
	Array byteStuffed;  // struct to store converted array
	initArray(&byteStuffed, MAX_SIZE);

	initializeInformationFrame(&byteStuffed);

	int alteredBytes = byteStuff(&msgBuffer, &byteStuffed);

	endInformationFrame(&byteStuffed);

	if(DEBUG)
	{
		printf("\n%d bytes altered.\n", alteredBytes);
		printf("ByteStuffed: ");
		printHexArray(byteStuffed.array, byteStuffed.used);
	}

	memcpy(linkL.frame, byteStuffed.array, MAX_SIZE);

	char response[MAX_SIZE];

	int writtenChars = send_cycle(fd, response);
		
	if(DEBUG)
	{
		printf ("Wrote %lu chars for original: ", byteStuffed.used);
		printHexArray(buffer, length);
		printf ("Received %d chars: ", writtenChars);
		printHexArray(response, 20);
	}

	freeArray(&msgBuffer);
	freeArray(&byteStuffed);

	/* TCIOFLUSH flushes both data received but not read and adata written but not transmitted*/
	//tcflush(fd, TCIOFLUSH); // discards data stuck in fd

	return writtenChars;
}

int llread(int fd, char* buffer)
{
	/* TCIOFLUSH flushes both data received but not read and adata written but not transmitted*/
	tcflush(fd, TCIOFLUSH); // REMOVING THIS CAUSES TROUBLE READING!!!

	STOP=FALSE;

	//CYCLE
	while (STOP==FALSE)  // loop for input
		{      
			int res = read(fd, buffer, MAX_SIZE);

			if (res >= 1)
			{
				if(DEBUG)
				{
					printf ("Buffer inside llread: ");
					printHexArray(buffer, 15);
					printf ("\n");
				}

				break;
			}
		}

	//tcflush(fd, TCIOFLUSH);

	return 0;
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

	sleep(2);

	if(appL.status == TRANSMITTER)
	{
		char msg[4]= {FLAG, FLAG, ESCAPE, 1};
		printf ("Message to send: ");
		printHexArray(msg, 4);
		llwrite(fd, msg, 4);
	}

	else
	{
		char msg[MAX_SIZE];
		llread(fd, msg);
		printf ("Received message: ");
		printHexArray(msg, 15);
	}
		
	llclose(fd);

		return 0;
}
