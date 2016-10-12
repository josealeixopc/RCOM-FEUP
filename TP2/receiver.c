	/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#include "utils.h"

#define DEBUG 1

volatile int STOP=FALSE;

int verifySET(unsigned char* SET)
{
	if (SET[0] != FLAG)
		return 0;

	if (SET[1] != A_SND)
		return 0;

	if (SET[2] != C_SET)
		return 0;

	if (SET[3] != A_SND ^ C_SET)
		return 0;

	if (SET[4] != FLAG)
		return 0;

	return 1;
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

    printf("New termios structure set\n");

 	char letter[255];
	int counter =0;


	/*Receive command to start sync*/

	char SET[255];

    while (STOP==FALSE) {       /* loop for input */
     	
		res = read(fd,SET,5); // reads the flag value

		if(DEBUG)
		{
			printf ("%d bytes received\n", res);
			printf ("SET: 0x%x , 0x%x, 0x%x, 0x%x, 0x%x\n", SET[0], SET[1], SET[2], SET[3], SET[4]);
		}	

    	if (verifySET(SET))
			STOP=TRUE;	//end cycle
	
		if (DEBUG)
			printf ("verifySET = %d\n", verifySET(SET));
    }
		

	sleep(2);


	res = write(fd, UA, 5);

	sleep(2);

	printf("%d bytes written\n", res);

	
    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
