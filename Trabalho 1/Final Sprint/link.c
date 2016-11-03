#include "link.h"

LinkLayer* linkL;

int flag = -1;           /* 1 if an alarm has not been set and a SET needs to be sent. 0 if it's waiting for a response*/
int numOfTries = -1;    /* number tries made to send messages */

int max_num_tries = 3;
int time_out = 3;

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

/******************** STATE MACHINES *******************/

int supervisionSM(int fd, unsigned char* frame)
{
	int pass = FALSE;

	int state = 0;

	int res = 0; // result of read function

	while(!pass)
	{
		char readByte;

		if(state < 5) // while frame is not complete
		{
			res = read(fd, &readByte, 1); // reads one byte at a time

			if(flag == 1 && res < 0)
			{
				return -1; // interrupt function to resend command
			}

			if(res < 0)
			{
				continue; // continue reading, until a char appears
			}
			else
			{
				if(DEBUG)
				{
					printf("supervisionSM received %d bytes: 0x%x\n", res, readByte);
				}
			}
		}

		switch (state)
		{
			case 0:
				if(readByte == FLAG)
				{
					frame[state] = readByte;
					state++;
				}

				break;

			case 1:
				if(readByte == A_RCV || readByte == A_SND)
				{
					frame[state] = readByte;
					state++;
				}

				else if (readByte != FLAG)
				{
					state = 0;	// resets state machine. If it was a flag, the state would continue to be 1.
				}

				break;

			case 2:
				if(readByte == C_SET || readByte == C_UA
				|| readByte == RR_0 || readByte == RR_1 ||
				readByte == REJ_0 || readByte == REJ_1 ||
				readByte == C_DISC)
				{
					frame[state] = readByte;
					state++;
				}

				else if (readByte == FLAG)
				{
					state = 1;
				}
				else
				{
					state = 0;
				}

				break;

			case 3:
				if(readByte == (frame[1]^frame[2]))
				{
					frame[state] = readByte;
					state++;
				}

				else if (readByte == FLAG)
				{
					state = 1;
				}

				else
				{
					state = 0;
				}

				break;

			case 4:
				if(readByte == FLAG)
				{
					frame[state] = readByte;
					state ++;
				}
				else
				{
					state = 0;
				}

				break;

			default: // if state is bigger than 4
				pass = TRUE;
				state = SM_SUCCESS;
				break;
		}
	}

	return state;
}

int informationSM(int fd, Array* frameArray)
{
	int i = 0;	// index of the char being written
	int state = 0;

	int complete = FALSE; // whether the frame is complete or not

	int r = 0;

	unsigned char readByte = 20;

	while(!complete)
	{
		r = read(fd, &readByte, 1);

		if(r <= 0)
		{
			continue; 	// if no byte is received, try to read again
		}

		switch(state)
		{
			case 0:
				if (readByte == FLAG)
				{
					insertArray(frameArray, readByte);

					i++;
					state++;
				}

				break;

			case 1:
				if(readByte == A_SND || readByte == A_RCV)
				{
					insertArray(frameArray, readByte);

					i++;
					state++;
				}
				else if (readByte != FLAG)
				{
					state = 0;
					i = 0;
				}

				break;

			case 2:
				if(readByte == C_FRAME_0 || readByte == C_FRAME_1)
				{
					insertArray(frameArray, readByte);

					i++;
					state++;
				}

				else if (readByte == FLAG)
				{
					state = 1;
					i = 1;
				}

				else
				{
					state = 0;
					i = 0;
				}

				break;

			case 3:
				if(readByte == (frameArray->array[1] ^ frameArray->array[2]))
				{
					insertArray(frameArray, readByte);

					i++;
					state++;
				}

				else if (readByte == FLAG)
				{
					state = 1;
					i = 1;
				}

				else
				{
					state = 0;
					i = 0;
				}

				break;

			case 4:
				if(readByte == FLAG)
				{
					insertArray(frameArray, readByte);

					i++;
					complete = TRUE;
				}
				else
				{
					insertArray(frameArray, readByte); // read information while FLAG does not appear
					i++;
				}

				break;

			default:
				break;
		}

	}

	return i;
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

	if (rr[2] != RR_0 && rr[2] != RR_1)
		return -3;


	if (rr[4] != FLAG)
		return -4;

  	if(rr[2] == RR_0 && rr[3] == (A_SND^RR_0))
		return 0;

	if(rr[2] == RR_1 && rr[3] == (A_SND^RR_1))
		return 1;

	return -5;
}

int reject(unsigned char* rej)
{
	if (rej[0] != FLAG)
		return -1;

	if (rej[1] != A_SND)
		return -2;

	if (rej[2] != REJ_1 && rej[2] != REJ_0)
		return -3;


	if (rej[4] != FLAG)
		return -4;

  	if (rej[2] == REJ_0 && rej[3] == (A_SND^REJ_0))
		return 0;

	if( rej[2] == REJ_1 && rej[3] == (A_SND^REJ_1))
		return 1;

	return -5;
}

/******************** STUFFING FUNCTIONS *******************/

int byteStuff(Array* inArray, Array* outArray)
{
    int count = 0;

	int i;

    for(i = 0; i < inArray->used; i++)
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
	int i;

    for(i = 0; i < inArray->used; i++)
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
int send_cycle(int fd, unsigned char * sendMsg, int size, unsigned char * received, Stats* stats){

	flag = 1;
	numOfTries = 0;
	int res;

	(void)signal(SIGALRM, alarmHandler); /* sets alarmHandler function as SIGALRM handler*/

	int writtenChars = 0;

	//Cycle that sends the SET bytes, while waiting for UA
	while(numOfTries < max_num_tries){
		if(flag)
		{
			printf("Try #%d\n", numOfTries+1);

			alarm(time_out); /* waits x seconds, then activates a SIGALRM */
			flag = 0; /* doesn't resend a signal until an alarm is handled */

			tcflush(fd, TCIOFLUSH);

			writtenChars = write(fd, sendMsg, size); // writes the flags
		}

		res = read(fd, received, size);


		if(res >= 1)
		{
			return writtenChars;
		}

		if(numOfTries == max_num_tries)
		{
			stats->numTimeouts++;
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

    	if (res >= 1)
		  {
			if (DEBUG)
			printf ("badSET = %d\n", badSET(receivedSET));


			if(!badSET(receivedSET))
			{
				tcflush(fd, TCIOFLUSH);
				res = write(fd, UA, 5); // send response
				STOP=TRUE;	//end cycle

				break;
			}
    	}
	}
}

// [TRANSMITTER] Try to send set flag 3 times and receive UA response

void send_set(int fd, Stats* stats){

	int res;
	unsigned char msg[5];

	printf("Begin connection.\n");

	res = send_cycle(fd, SET, 5, msg, stats);

    if(res == -1) {
    	printf("ERROR: Couldn't receive response after sending SET.\n");
    	exit(-1);
    }

	else
	{
		if(!badUA(msg))
		{
			printf("Connection successful.\n");
		}
		else
		{
			printf("Bad UA.\n");
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

int llopen(ApplicationLayer* appL, LinkLayer* linkL, struct termios* oldtio, Stats* stats){

	int fd;
    struct termios newtio;

    /*
        Open serial port device for reading and writing and not as controlling tty
        because we don't want to get killed if linenoise sends CTRL-C.
    */

	initSetFrame(appL, linkL);

    fd = open(linkL->port, O_RDWR | O_NOCTTY);
    if (fd <0) {perror(linkL->port); exit(-1); }

    appL->fileDescriptor = fd;

    if ( tcgetattr(fd,oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = linkL->baudRate | CS8 | CLOCAL | CREAD;
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
	max_num_tries = linkL->numTransmissions;
	time_out = linkL->timeout;
	if(appL->status == TRANSMITTER)
		send_set(fd, stats);
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

		if (res >= 1 && (!badDisc(received)))
		{
			tcflush(fd, TCIOFLUSH);

			res = write(fd, DISC, 5); // send response

			if(res > 1){

				unsigned char last[5];

				while(read(fd, last, 5) < 1);

				if(!badUA(last) && DEBUG)
					printf("Worked!\n");
			}

			STOP=TRUE;

			free(received);

			return 0;
		}

		if(res >= 1 && badDisc(received))
		{
			printf("Bad DISC received!\n");
			free(received);
			return -1;
		}
	}

	free(received);
	return -1;
}

void close_set(int fd, Stats* stats){
	int res;
	tcflush(fd, TCIOFLUSH);
	unsigned char * DISC = SET;
	DISC[2] = C_DISC;
	DISC[3] = A_SND^C_DISC;
	unsigned char buf[5];

	res = send_cycle(fd, DISC,5, buf, stats);
	if(res < 1){
		printf("Error receiving disc!\n");
		return;
	}
	if(!badDisc(buf)){

			tcflush(fd, TCIOFLUSH);

			res = write(fd, UA, 5);


			if(res > 1)
				return;
	}

}

int llclose(ApplicationLayer* appL, struct termios* oldtio, Stats* stats)
{
    int fd = appL->fileDescriptor;

	printf("Begin closing attempt...\n");

	if(appL->status == TRANSMITTER)
        close_set(fd, stats);
	else
        close_ua(fd);


    if (tcsetattr(fd,TCSANOW,oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    close(fd);

	printf("Closed connection.\n");

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

	insertArray(frameArray, (frameArray->array[1] ^ frameArray->array[2])); // header BCC
}

void addBodyBCC(Array* packetArray)
{
	char BCC2 = 0x0;
	unsigned int i;

	for(i = 0; i < packetArray->used; i++)	// i == 4 so it skips first bytes
	{
		BCC2 = BCC2 ^ (packetArray->array[i]); // verify parity of data bytes
	}

	insertArray(packetArray, BCC2);

	if(DEBUG)
	{
		printf("BCC2: %x\n", BCC2);
	}


}

void endInformationFrame(Array* frameArray)
{
	insertArray(frameArray, FLAG);
}

/******************** SEND & RECEIVE FUNCTIONS *******************/

int llwrite(int fd, unsigned char* packet, size_t packetLength, LinkLayer* linkL, Stats* stats)
{
    int returnValue;

    Array packetArray;
    initArray(&packetArray, 1);

    copyArray(packet, &packetArray, packetLength);

    Array stuffedArray;
    initArray(&stuffedArray, 1);

    initializeInformationFrame(&stuffedArray, linkL);

	addBodyBCC(&packetArray);

    byteStuff(&packetArray, &stuffedArray);

    endInformationFrame(&stuffedArray);

	unsigned char feedback[5] = {};

	printf("Begin to send frame #%d: ", linkL->sequenceNumber);
	printHexArray(&stuffedArray);

	int numOfWrittenChars = send_cycle(fd, stuffedArray.array, stuffedArray.used, feedback, stats);

	while(numOfWrittenChars == -1)
	{
		char line[256];
		char input;

		while ((input = getchar()) != '\n' && input != EOF); // flush lost new lines

		input = 0;

		while (1)
		{
			printf("Connection seems to have been lost. Do you want to (r)esend or (e)xit the program? ");

			if (fgets(line, sizeof(line), stdin))
			{
  				if (1 != sscanf(line, "%c", &input))
				{
					printf("Invalid input.\n");
					continue;
   			 	}
				else
				{
					if(DEBUG)
					{
						printf("User input: %c\n", input);
					}
				}
			}

			printf("\n\n");

			if(input ==  'e' || input == 'E')
			{
				printf("Exiting...\n");
				sleep(1);
				exit(-1);
				break;
			}

			else if(input == 'r' || input == 'R')
			{
				printf("Sending again...\n");
				sleep(1);
				numOfWrittenChars = send_cycle(fd, stuffedArray.array, stuffedArray.used, feedback, stats);	
				break;
			}

			else
			{
				printf("Please write 'e' to exit or 'r' to resend.\n");
			}
		}
	}

	if(DEBUG)
	{
		printf("Feedback: ");
		printHexBuffer(feedback, 5);
	}

	if(receiverReady(feedback) == 0)
	{
		if(linkL->sequenceNumber == 1)
		{
			printf("Frame #%d sent with success!\n\n", linkL->sequenceNumber);
			linkL->sequenceNumber = 0;
			returnValue = 0;
			stats->framesSent++;
			stats->numRR++;
		}
		else
		{
			returnValue = -1;
		}
	}

	if(receiverReady(feedback) == 1)
	{
		if(linkL->sequenceNumber == 0)
		{
			printf("Frame #%d sent with success!\n\n", linkL->sequenceNumber);
			linkL->sequenceNumber = 1;
			returnValue = 0;
			stats->framesSent++;
			stats->numRR++;
		}
		else
		{
			returnValue = -2;
		}
	}

	int i = 0;

	while(reject(feedback) == (linkL->sequenceNumber) && i < 10)
	{
		stats->numREJ++;
		printf("Frame %d rejected. Sending again.\n\n", linkL->sequenceNumber);
		send_cycle(fd, stuffedArray.array, stuffedArray.used, feedback, stats);

		if(receiverReady(feedback) == 1 && linkL->sequenceNumber == 0)
		{
			stats->numRR++;
			break;
		}

		if(receiverReady(feedback) == 0 && linkL->sequenceNumber == 1)
		{
			stats->numRR++;
			break;
		}

		i++;

		if(i == 10)
		{
			printf("ERROR: Frame keeps getting rejected.\n\n");
			returnValue = -10;
		}
	}

    freeArray(&packetArray);
	freeArray(&stuffedArray);

    return returnValue;
}

/* Removes the header and trailer of the frame*/
// Returns the length of the dataOut array
int getDataAndBCCFromFrame(unsigned char* frameIn, unsigned  char* dataOut)
{
	int beginFlag = 0, endFlag = 0;

	unsigned int beginFlagPosition = 0;
	unsigned int endFlagPosition = 0;

	unsigned int beginDataPosition = 0;
	unsigned int endDataPosition = 0;

	int i;

	for(i = 0; i < MAX_SIZE; i++)
	{
		if(frameIn[i] == FLAG)
		{
			beginFlagPosition = i;
			beginFlag = 1;

			beginDataPosition = beginFlagPosition + 4;
			break;
		}
	}

	for(i = beginDataPosition ; i < MAX_SIZE - beginDataPosition ; i++)
	{
		if(frameIn[i] == FLAG)
		{
			endFlagPosition = i;
			endFlag = 1;

			endDataPosition = endFlagPosition; // this is the byte after the last byte of data
			break;
		}
	}

	if(beginFlag == 0 || endFlag == 0)	// if one of the flags wasn't found
		return -1;

	unsigned int length = endDataPosition - beginDataPosition;

	memcpy(dataOut, frameIn + beginDataPosition, length);

	return length;

}

// [RECEIVER]
/* Verifies the body BCC of the frame received*/
int verifyBodyBCC(Array* frameArray)
{
	unsigned char BCC2 = 0x0;
	unsigned char frameBCC2 = frameArray->array[frameArray->used - 1];

	unsigned int lastDataByte = frameArray->used - 2;

	unsigned int i;

	for(i = 0; i <= lastDataByte; i++)	// i == 4 so it skips first bytes
	{
		BCC2 = BCC2 ^ (frameArray->array[i]); // verify parity of data bytes
	}

	if(BCC2 == frameBCC2)
		return 1;
	else
		return 0;

}

// [RECEIVER]
/* Get frame sequence number*/
int getFrameSequenceNumber(Array* frameArray)
{
	if(frameArray->array[2] == C_FRAME_0)
	{
		return 0;
	}

	if(frameArray->array[2] == C_FRAME_1)
	{
		return 1;
	}

	return -1;

}

void removeBodyBCC(Array* dataAndBCCArray)
{
	dataAndBCCArray->array[dataAndBCCArray->used -1] = 0;
	dataAndBCCArray->used--;
}

// [RECEIVER]
/* Decides what array to send as a response */
/* Returns negative value if frame is to be rejected */
int generateResponse(Array* frameArray, int validBodyBCC, unsigned char* response, Stats* stats)
{
	int ret = 0;

	response[0] = FLAG;
	response[1] = A_SND;

	if(!validBodyBCC)
	{
		if(getFrameSequenceNumber(frameArray) == 1)
		{
			response[2] = REJ_1;
			ret = -1;

			if(DEBUG)
			{
				printf("Frame #1 received with body errors.\n");
			}

			stats->numREJ++;
		}

		else if(getFrameSequenceNumber(frameArray) == 0)
		{
			response[2] = REJ_0;
			ret = -2;

			if(DEBUG)
			{
				printf("Frame #0 received with body errors.\n");
			}

			stats->numREJ++;
		}

		else
		{
			if(DEBUG)
			{
				printf("Invalid frame sequence number received.\n");
			}
		}

	}
	else
	{
		if(getFrameSequenceNumber(frameArray) == 1)
		{
			response[2] = RR_0;
			ret = 1;

			stats->numRR++;
		}

		else if(getFrameSequenceNumber(frameArray) == 0)
		{
			response[2] = RR_1;
			ret = 2;

			stats->numRR++;
		}

		else
		{
			if(DEBUG)
			{
				printf("Invalid frame sequence number received.\n");
			}
		}
	}

	response[3] = (response[1] ^ response[2]);

	response[4] = FLAG;

	return ret;
}

int llread(int fd, unsigned char* packet, size_t* packetLength, LinkLayer* linkL, Stats* stats)
{
	/* TCIOFLUSH flushes both data received but not read and adata written but not transmitted*/

	Array receivedFrame;
	initArray(&receivedFrame, 1);

	Array dataAndBCCArray;
	initArray(&dataAndBCCArray, MAX_SIZE);

	Array packetArray;
	initArray(&packetArray, MAX_SIZE);

	printf("Begin receiving...\n");

	size_t frameSize = informationSM(fd, &receivedFrame);

	// ERROR SIMULATION
	if(rand() % 100 <= 10 && ERROR_SIMULATION){ 	// 10 in 100 probability
		printf("ERROR Simulation\n");

		//rand() % (max_number + 1 - minimum_number) + minimum_number
		int e = rand() % (frameSize - 1 - 4) + 4;

		printf("Random number: %d \n",e);
		receivedFrame.array[e] = C_UA;
	}

	// DISCONNECTION SIMULATION
	if(rand() % 1000 <= 5 && DISCONNECTION_SIMULATION){ 	// 5 in 1000 probability
		printf("DISCONNECTION SIMULATION\nSleeping...\n");
		sleep(10);
	}

	printf("Frame received.\n");
	printf("Size of received frame: %lu\n", receivedFrame.used);
	printHexArray(&receivedFrame);

	unsigned char dataAndBCC[MAX_SIZE];

	size_t dataLength = getDataAndBCCFromFrame(receivedFrame.array, dataAndBCC);

	copyArray(dataAndBCC, &dataAndBCCArray, dataLength);

	byteUnstuff(&dataAndBCCArray, &packetArray);

	int validBodyBCC = verifyBodyBCC(&packetArray);

	removeBodyBCC(&packetArray);

	unsigned char feedback[5];

	int gr = generateResponse(&receivedFrame, validBodyBCC, feedback, stats);

	tcflush(fd, TCIOFLUSH);

	write(fd, feedback, 5);

	if(gr <= 0)	// if frame has been rejected
	{
		freeArray(&receivedFrame);
		freeArray(&dataAndBCCArray);
		freeArray(&packetArray);

		if(DEBUG)
		{
			printf("Frame discarded. generateResponse() = %d\n\n", gr);
		}

		return -1;	// discard current frame
	}

	(*packetLength) = packetArray.used;

	packet = (unsigned char*) realloc (packet, packetArray.used * sizeof(unsigned char));

	memcpy(packet, packetArray.array, packetArray.used);

	printf("Packet inside frame: ");
	printHexArray(&packetArray);

	freeArray(&receivedFrame);
	freeArray(&dataAndBCCArray);
	freeArray(&packetArray);

	printf("Frame accepted.\n\n");

	stats->framesReceived++;

	return 0;
}
