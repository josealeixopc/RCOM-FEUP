/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <math.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define FLAG 0x7e
#define A 0x03
#define C_SET 0x03

#define DEBUG 1


volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[5];
	unsigned char SET[5];
	SET[0] = FLAG;
	SET[1] = A;
	SET[2] = C_SET;
	SET[3] = A^C_SET;
	SET[4] = FLAG;

    int i, sum = 0, speed = 0;
    
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



  //	gets(buf); 
	int tries = 3; //number of tries to receive feedback

	while(tries)
	{
		res = write(fd,SET,5);	// writes the flags

		if(DEBUG)
			printf ("%d bytes written.\n", res);


		sleep(3);	// waits 3 seconds (use SIGALARM?)

		res = read(fd,buf,5);	// read feedback
		
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
     
    	printf("0x%x\n", buf[0]);
 

  /* 
    O ciclo FOR e as instruções seguintes devem ser alterados de modo a respeitar 
    o indicado no guiÃ£o 
  */

	sleep(2);
/*
   char letter[255];
int counter =0;
    while (STOP==FALSE) {   
      res = read(fd,buf,1);  
      buf[res]=0;
      letter[counter] = buf[0];	             
      printf("buf value:%s\n", buf);
      if (buf[0]=='\0') STOP=TRUE;
      counter+=1;
    }
letter[counter] = 0;
printf("%s \n" ,letter);
*/

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }




    close(fd);
    return 0;
}
