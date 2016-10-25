#include "link.h"

LinkLayer* linkL;

int flag = -1;           /* 1 if an alarm has not been set and a SET needs to be sent. 0 if it's waiting for a response*/
int numOfTries = -1;    /* number tries made to send messages */

volatile int STOP=FALSE;

unsigned char SET[5] = {FLAG, A_SND, C_SET, A_SND^C_SET, FLAG};
unsigned char UA[5] = {FLAG, A_SND, C_UA, A_SND^C_UA, FLAG};

/******************** ALARM HANDLER *******************/

void alarmHandler()
{
	if (DEBUG)
	{
		printf("handled alarm. Try number: %d\n", numOfTries);
	}
	flag = 1;
	numOfTries++;
}

/******************** VERIFYING COMMAND FUNCTIONS *******************/

int badSET(unsigned char* set)
{
	if (set[0] != FLAG)
		return -1;

	if (set[1] != A_SND)
		return -2;

	if (set[2] != C_SET)
		return -3;

	if (set[3] != (A_SND ^ C_SET))
		return -4;

	if (set[4] != FLAG)
		return -5;

	return 0;
}

int badUA(unsigned char *ua)
{
  if (ua[0] != FLAG)
    return -1;

  if (ua[1] != A_SND)
    return -2;

  if (ua[2] != C_UA)
    return -3;

  if (ua[3] != (A_SND^C_UA))
    return -4;

  if (ua[4] != FLAG)
    return -5;

  return 0;
}

/******************** STUFFING FUNCTIONS *******************/

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

/******************** COMMUNICATION FUNCTIONS *******************/

// [TRANSMITTER] Function to send a message with timeout and receive response from receiver
// @return Result of read call

int send_cycle(int fd, unsigned char * sendMsg, size_t size){

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
				printHexBuffer(sendMsg, size);
			}

			if(writtenChars > 1 ) 
                break;

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
			printHexBuffer(receivedSET, sizeof(receivedSET));
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
		printHexBuffer(msg, sizeof(msg));
		}
	}

}

/******************** CONFIGURE CONNECTION FUNCTIONS *******************/

//[TRANSMITTER AND RECEIVER] Sets the connection up between devices

void initSetFrame(ApplicationLayer* appL, LinkLayer* linkL)
{
	unsigned char trama_su[5];  // open frame
	trama_su[0] = FLAG;
	trama_su[1] = A_SND;
	trama_su[2] = C_UA;
	trama_su[3] = (A_SND^C_UA);
	trama_su[4] = FLAG;
	if(appL->status == TRANSMITTER){
		trama_su[2] = C_SET;
		trama_su[3] = (A_SND^C_SET);
	}

	memcpy(linkL->frame, trama_su, 5);
}

int llopen(ApplicationLayer* appL, LinkLayer* linkL, struct termios* oldtio){

	int fd;
    struct termios newtio;

    /*
        Open serial port device for reading and writing and not as controlling tty
        because we don't want to get killed if linenoise sends CTRL-C.
    */

	initSetFrame(appL, linkL);

    fd = open(linkL->port, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(linkL->port); exit(-1); }

    appL->fileDescriptor = fd;

    if ( tcgetattr(fd,oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 1;   /* if for 0.1 seconds no char is received, read is unblocked */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 char received */

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    if(DEBUG)
    {
      printf("New termios structure set\n");
    }

	if(appL->status == TRANSMITTER)
		send_set(fd);
	else
		receive_set(fd);

	return fd;
}

/******************** FRAME PREPARATION FUNCTIONS *******************/

void initializeInformationFrame(Array* frameArray, LinkLayer* linkL)
{
	insertArray(frameArray, FLAG);
	insertArray(frameArray, A_SND);

    if(linkL->sequenceNumber == 0)
    {
        insertArray(frameArray, C_FRAME_0);
    }
    else
    {
        insertArray(frameArray, C_FRAME_1);
    }

	insertArray(frameArray, C_SET);
	insertArray(frameArray, (frameArray->array[1] ^ frameArray->array[2])); // header BCC
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

/******************** SEND & RECEIVE FUNCTIONS *******************/

int llwrite(int fd, unsigned char* packet, size_t length, LinkLayer* linkL)
{
    int returnValue = 0;

    Array packetArray;
    initArray(&packetArray, 1);

    copyArray(packet, &packetArray, length);

    if(DEBUG)
    {
        printf("Copied array after copyArray(): ");
        printHexArray(&packetArray);
    }

    Array stuffedArray;
    initArray(&stuffedArray, 1);

    initializeInformationFrame(&stuffedArray, linkL);

    int bytesAltered = byteStuff(&packetArray, &stuffedArray);

    endInformationFrame(&stuffedArray);

    if(DEBUG)
    {
        printf("Altered %d bytes in stuffing: ", bytesAltered);
        printHexArray(&stuffedArray);
    }


    freeArray(&packetArray);

    return returnValue;
}

