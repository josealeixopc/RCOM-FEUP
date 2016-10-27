#include "application.h"

ApplicationLayer appL;
LinkLayer linkL;
Stats stats;

struct termios oldtio;

/******************** ALARM HANDLER *******************/

int main(int argc, char** argv)
{
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

	(void)signal(SIGALRM, alarmHandler);

	printf("Started execution...\n");

	llopen(&appL, &linkL, &oldtio);

	printf("llopen() done with success!\n");

    if(appL.status == TRANSMITTER)
	{
		unsigned char test[5] = {FLAG, ESCAPE, FLAG, FLAG, ESCAPE};
		llwrite(appL.fileDescriptor, test, sizeof(test), &linkL);
	}
	else
	{
		size_t packetLength = 1;
		unsigned char* packet = (unsigned char*) malloc (packetLength * sizeof(unsigned char));

		llread(appL.fileDescriptor, packet, &packetLength, &linkL);

		printf("Packet received begin: ");
		printHexBuffer(packet, packetLength);
		printf("Packet received end.\n");

		free(packet); // importanteee!
	}


	llclose(&appL, &oldtio);

	printf("llclose() done with success!\n");

    return 0;

}