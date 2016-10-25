#include "application.h"

ApplicationLayer appL;
LinkLayer linkL;
Stats stats;

struct termios oldtio;

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

	llopen(&appL, &linkL, &oldtio);

	//unsigned char test[9] = {FLAG, A_SND, C_SET, 0x1, FLAG, ESCAPE, FLAG};

    //llwrite(0, test, sizeof(test), &linkL);

    //printf ("Sizeof test: %lu", sizeof(test));

	llclose(&appL, &oldtio);

    return 0;

}