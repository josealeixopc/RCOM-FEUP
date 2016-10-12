/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <math.h>

#include "utils.h"

#define DEBUG 1

int flag = 1; /* 1 if an alarm has not been set and a SET needs to be sent. 0 if it's waiting for a response*/
int numOfTries = 1; /* number tries made to send SET */

int verifyUA(unsigned char *SET)
{
  if (SET[0] != FLAG)
    return 0;

  if (SET[1] != A_SND)
    return 0;

  if (SET[2] != C_UA)
    return 0;

  if (SET[3] != A_SND ^ C_UA)
    return 0;

  if (SET[4] != FLAG)
    return 0;

  return 1;
}

int alarmHandler()
{
  if (DEBUG)
  {
    printf("handled alarm. Try number: %d\n", numOfTries);
  }
  flag = 1;
  numOfTries++;
}

volatile int STOP = FALSE;

int main(int argc, char **argv)
{
  int fd, c, res;
  struct termios oldtio, newtio;
  char buf[5];

  unsigned char SET[5] = {FLAG, A_SND, C_SET, A_SND ^ C_SET, FLAG};

  int i, sum = 0, speed = 0;

  if ((argc < 2) ||
      ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
       (strcmp("/dev/ttyS1", argv[1]) != 0)))
  {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

  fd = open(argv[1], O_RDWR | O_NOCTTY);
  if (fd < 0)
  {
    perror(argv[1]);
    exit(-1);
  }

  if (tcgetattr(fd, &oldtio) == -1)
  { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  /*VMIN = 0 and VTIME = 0

    This is a completely non-blocking read - the call is satisfied immediately directly from the driver's input queue. 
	  If data are available, it's transferred to the caller's buffer up to nbytes and returned. Otherwise zero is immediately returned to indicate "no data".
  	We'll note that this is "polling" of the serial port, and it's almost always a bad idea. If done repeatedly, it can consume enormous amounts of processor 
	  time and is highly inefficient. Don't use this mode unless you really, really know what you're doing.*/

  newtio.c_cc[VTIME] = 0;
  newtio.c_cc[VMIN] = 0;

  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) prÃ³ximo(s) caracter(es)
  */

  tcflush(fd, TCIOFLUSH);

  if (tcsetattr(fd, TCSANOW, &newtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");

  (void)signal(SIGALRM, alarmHandler); /* sets alarmHandler function as SIGALRM handler*/

  //Cycle that sends the SET bytes, while waiting for UA
  while (numOfTries != MAX_TRIES)
  {
    if(flag)
    {
      res = write(fd, SET, sizeof(SET)); // writes the flags

      if(DEBUG)
      {
        printf("%d bytes written.\n", res);
      }

      alarm(3); /* waits 3 seconds, then activates a SIGALRM */
      flag = 0; /* doesn't resend a signal until an alarm is handled */
    }

    res = read(fd, buf, sizeof(buf)); // read feedback

    if (res >= 1) // if it read something
      break;
  }

  if(numOfTries == MAX_TRIES)
  {
    printf("ERROR: No response from receiver.\n");
    exit(-1);
  }

  if (DEBUG)
  {
    printf("I'm outside the feedback loop!\n");
    printf("%d bytes received: ", res);
    printf("0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", buf[0], buf[1], buf[2], buf[3], buf[4]);
    printf("VerifyUA: %d", verifyUA(buf));
  }


  sleep(2); /* To let the controller send information before the new settings are applied */


  if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}
