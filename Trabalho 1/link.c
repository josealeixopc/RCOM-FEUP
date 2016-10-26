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

int badDisc(unsigned char *disc)
{
  if (disc[0] != FLAG)
    return -1;

  if (disc[1] != A_SND)
    return -2;

  if (disc[2] != C_DISC)
    return -3;

  if (disc[3] != (A_SND^C_DISC))
    return -4;

  if (disc[4] != FLAG)
    return -5;

  return 0;
}

int receiverReady(unsigned char* rr)
{
	if (rr[0] != FLAG)
		return -1;

	if (rr[1] != A_SND)
		return -2;

	if (rr[2] != RR_0 || rr[2] != RR_1)
		return -3;


	if (rr[4] != FLAG)
		return -5;

  	if(rr[2] == RR_0 && rr[3] == (A_SND^RR_0))
		return 0;
	else
		return -4;

	if(rr[2] == RR_1 && rr[3] == (A_SND^RR_1))
		return 1;
	else
		return -4;
}

int reject(unsigned char* rej)
{
	if (rej[0] != FLAG)
		return -1;

	if (rej[1] != A_SND)
		return -2;

	if (rej[2] != REJ_1 || rej[2] != REJ_0)
		return -3;


	if (rej[4] != FLAG)
		return -5;

  	if (rej[2] == REJ_0 && rej[3] == (A_SND^REJ_0))
		return 0;
	else
		return -4;

	if( rej[2] == REJ_1 && rej[3] == (A_SND^REJ_1))
		return 1;
	else
		return -4;
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
// @return Control flag of sent response
int send_cycle(int fd, unsigned char * sendMsg, int size, unsigned char * received){

	flag = 1;
	numOfTries = 0;
	
	(void)signal(SIGALRM, alarmHandler); /* sets alarmHandler function as SIGALRM handler*/

	int writtenChars = 0;

	if(DEBUG)
		printf("flag: %d, numOfTries: %d\n", flag, numOfTries);

	//Cycle that sends the SET bytes, while waiting for UA
	while(numOfTries < MAX_TRIES){
		if(flag)
		{
			alarm(3); /* waits 3 seconds, then activates a SIGALRM */
			flag = 0; /* doesn't resend a signal until an alarm is handled */
			
			tcflush(fd, TCIOFLUSH);
			
			writtenChars = write(fd, sendMsg, size); // writes the flags

			if(DEBUG)
			{
				printf("Wrote msg: ");
				printHexBuffer(sendMsg, writtenChars);
			}
		}
		
		int res = read(fd, received, size);
		

		if(res >= 1) 
		{
			// if the messages received is a valid form of message, return, else, continue
			if(badUA(received) == 0 || badDisc(received) == 0 || receiverReady(received) >= 0 || reject(received) >= 0)
				return writtenChars;
			else
				continue;
		}

	}

	return -1;

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
			printHexBuffer(receivedSET,5);
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

	res = send_cycle(fd, SET, 5, msg);

    if(res == -1) {
    	printf("Couldn't receive response from SET!\n");
    	exit(-1);
    }

	else
	{
		if(DEBUG)
		{
			printf("Received valid UA.\n");
			printHexBuffer(msg, 5);
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

    newtio.c_cc[VTIME]    = 0.1;   /* if for 0.1 seconds no char is received, read is unblocked */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 1 char received */

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

int close_ua(int fd){
	int res;
	tcflush(fd, TCIOFLUSH);
	
	unsigned char * received = malloc(5*sizeof(unsigned char * ));	// array to store SET bytes
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
		printHexBuffer(received, 5);
	}

		if (res >= 1 && (!badDisc(received)))
		{
			res = write(fd, DISC, 5); // send response

			if(res > 1){
				unsigned char last[5];

				while(read(fd, last, 5) < 1);

				printHexBuffer(last, 5);

				if(!badUA(last) && DEBUG) 
					printf("Worked!\n");
			}
          		STOP=TRUE;
			free(received);
			return 0;
		}
		if(res >= 1 && badDisc(received))
		{
			printf("bad disc received!\n");
			free(received);
			return -1;
		}
	}
	free(received);
	return -1;
}

void close_set(int fd){
	int res;
	tcflush(fd, TCIOFLUSH);
	unsigned char * DISC = SET;
	DISC[2] = C_DISC;
	DISC[3] = A_SND^C_DISC;
	unsigned char buf[5];

	res = send_cycle(fd, DISC,5, buf);
	if(res < 1){
		printf("Error receiving disc!\n");
		return;
	}
	if(!badDisc(buf)){

			if(DEBUG)
				printf("read disconnect success");

			res = write(fd, UA, 5);

			if(DEBUG)
				if(res > 1) printf("\nsent final handshake\n");

			return;

	}

	if(DEBUG)
		printf("I'm outside the feedback loop!\n");
}

int llclose(ApplicationLayer* appL, struct termios* oldtio)
{
    int fd = appL->fileDescriptor;

	if(appL->status == TRANSMITTER) 
        close_set(fd);
	else 
        close_ua(fd);


    if (tcsetattr(fd,TCSANOW,oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);

	return 0;
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
    int returnValue = -1;

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

	unsigned char feedback[5] = {};

	send_cycle(fd, stuffedArray.array, stuffedArray.used, feedback);

	/*if(receiverReady(feedback) == 0)
	{
		if(linkL->sequenceNumber == 1)
		{
			linkL->sequenceNumber = 0;
			returnValue = 0;
		}
		else
		{
			send_cycle(fd, stuffedArray.array, stuffedArray.used, feedback);	// try again
		}
	}

	if(receiverReady(feedback) == 1)
	{
		if(linkL->sequenceNumber == 0)
		{
			linkL->sequenceNumber = 1;
			returnValue = 0;
		}
		else
		{
			send_cycle(fd, stuffedArray.array, stuffedArray.used, feedback);	// try again
		}
	}

	if(reject(feedback) == 1)
	{
		send_cycle(fd, stuffedArray.array, stuffedArray.used, feedback);
	}*/


    freeArray(&packetArray);
	freeArray(&stuffedArray);

    return returnValue;
}

/* Removes the header and trailer of the frame*/
// Returns the length of the dataOut array
int getDataFromFrame(unsigned char* frameIn, unsigned  char* dataOut)
{
	if(DEBUG)
	{
		printf("getDataFromFrame() begin:\n");
		printf("frameIn: ");
		printHexBuffer(frameIn, 15);
	}
	int beginFlag = 0, endFlag = 0;

	unsigned int beginFlagPosition = 0;
	unsigned int endFlagPosition = 0;

	unsigned int beginDataPosition = 0;
	unsigned int endDataPosition = 0;

	for(int i = 0; i < MAX_SIZE; i++)
	{
		if(frameIn[i] == FLAG)
		{
			beginFlagPosition = i;
			beginFlag = 1;

			beginDataPosition = beginFlagPosition + 4;
			break;
		}
	}

	for(int i = beginDataPosition ; i < MAX_SIZE - beginDataPosition ; i++)
	{
		if(frameIn[i] == FLAG)
		{
			endFlagPosition = i;
			endFlag = 1;

			endDataPosition = endFlagPosition - 1; // this is the byte after the last byte of data
			break;
		}
	}

	if(beginFlag == 0 || endFlag == 0)	// if one of the flags wasn't found
		return -1;

	unsigned int length = endDataPosition - beginDataPosition;

	memcpy(dataOut, frameIn + beginDataPosition, length);
	
	if(DEBUG)
	{
		printf("getDataFromFrame() end:\n");
		printf("dataOut: ");
		printHexBuffer(dataOut, length);
		printf("\n");
	}

	return length;
	
}

int llread(int fd, unsigned char* packet, LinkLayer* linkL)
{
	/* TCIOFLUSH flushes both data received but not read and adata written but not transmitted*/

	int res;

	int beginFlag = 0;

	STOP=FALSE;

	unsigned char readByte;

	Array receivedFrame;
	initArray(&receivedFrame, 1);

	Array dataArray;
	initArray(&dataArray, MAX_SIZE);

	Array packetArray;
	initArray(&packetArray, MAX_SIZE);

	tcflush(fd, TCIOFLUSH); // REMOVING THIS CAUSES TROUBLE READING!!!

	//CYCLE
	while (STOP==FALSE)  // loop for input
	{      
		res = read(fd, &readByte, 1);

		if(res < 0)
			continue;

		insertArray(&receivedFrame, readByte);

		if(readByte == FLAG && beginFlag == 0)
		{
			beginFlag = 1;
		}
		if(readByte == FLAG && beginFlag == 1)
		{
			STOP = TRUE;
		}
	}

	printHexArray(&receivedFrame);

	/*unsigned char data[MAX_SIZE];

	size_t dataLength = getDataFromFrame(receivedFrame.array, data);

	copyArray(data, &dataArray, dataLength);

	byteUnstuff(&dataArray, &packetArray);

	memcpy(packet, packetArray.array, packetArray.used);

	printHexBuffer(packet, packetArray.used);*/

	freeArray(&receivedFrame);
	freeArray(&dataArray);
	freeArray(&packetArray);

	return 0;
}

