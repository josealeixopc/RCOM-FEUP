	/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#include "utils.h"

#define DEBUG 1

volatile int STOP=FALSE;

int badSET(unsigned char* SET)
{
	if (SET[0] != FLAG)
		return -1;

	if (SET[1] != A_SND)
		return -2;

	if (SET[2] != C_SET)
		return -3;

	if (SET[3] != (A_SND ^ C_SET))
		return -4;

	if (SET[4] != FLAG)
		return -5;

	return 0;
}

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];

	unsigned char UA[5] = {FLAG, A_SND, C_UA, A_SND^C_UA, FLAG};

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

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

	
	/* VTIME IS IN TENTHS OF A SECOND*/
    newtio.c_cc[VTIME]    = 0;
    newtio.c_cc[VMIN]     = 0;

	/* 

    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)

  	*/

    tcflush(fd, TCIOFLUSH);	// flush do buffer

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

	if(DEBUG)
    	printf("New termios structure set\n");

	/*Receive command to start sync*/

	unsigned char receivedSET[5];	// array to store SET bytes

	//TIMED READ
	/*
	res = read(fd,receivedSET, sizeof(receivedSET));

	if (res == 0) // no read for VTIME seconds
	{
		printf("ERROR: No signal was sent.\n");
		exit(-1);
	}

	else
	{
		if(!badSET(receivedSET))
		{
			res = write(fd, UA, 5); // send response

			if(DEBUG)
			{
				printf("Sent response UA.\n");
				printf("%d bytes written\n", res);
			}
		}
		else
		{
			printf ("ERROR: SET received doesn't match standard SET.\n'");
			exit(-1);
		}
	}
	*/
	//END OF TIMED READ

	//CYCLE
    while (STOP==FALSE)  // loop for input
	{      
     	
		res = read(fd,receivedSET, sizeof(receivedSET)); // reads the flag value

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
				res = write(fd, UA, 5); // send response
				STOP=TRUE;	//end cycle

				if(DEBUG)
				{
					printf("Sent response UA.\n");
					printf("%d bytes written\n", res);
				}
			}
    	}
	}
	//END OF CYCLE

	sleep(2); 

    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
  	{
    perror("tcsetattr");
    exit(-1);
    }

    close(fd);
    return 0;
}
