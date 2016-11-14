#include "interface.h"

int clearfunction(){
	int count = 15;

while (count > 0){
	printf("\n");
	count--;
	}
return 0;
}


int startmenu(){


  printf("\n\n\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
    "+                   RCOM ASSIGNMENT                    +\n"
    "+                                                      +\n"
    "+                                                      +\n"
    "+          Press for program functionality:            +\n"
    "+                                                      +\n"
    "+          1 - Transmitter                             +\n"
    "+          2 - Receiver                                +\n"
    "+                                                      +\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
);

	char programfunc;
	scanf("%c", &programfunc);
	
	if(programfunc != '1' && programfunc != '2')
		return -1;
	else
		return programfunc - '0';




}


int baudarecheck(){


  printf("\n\n\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
    "+                                                      +\n"
    "+                                                      +\n"
    "+                                                      +\n"
    "+                 Insert valid Baudrate:               +\n"
    "+     300, 600, 1200, 1800, 2400, 4800, 9600, 19200,   +\n"
    "+                  38400, 57600, 115200                +\n"
    "+                                                      +\n"
    "+                                                      +\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
);

	char bananas[MAX_SIZE];

	scanf("%s", bananas);
		
	
	int baudrate;
	sscanf(bananas, "%d", &baudrate);

	while(baudrate != 300&&baudrate != 600 && baudrate != 1200&& baudrate != 1800&& baudrate != 2400&& baudrate != 4800
&& baudrate != 9600&& baudrate != 19200&& baudrate != 38400&& baudrate != 57600&& baudrate != 115200){
		printf("\n Wrong Baudrate value please insert a valid one: \n");
		return -1;

	}
	switch(baudrate){
		case 300:
			baudrate = B300;
			break;
		case 600:
			baudrate = B600;
			break;
		case 1200:
			baudrate = B1200;
			break;
		case 1800:
			baudrate = B1800;
			break;
		case 2400:
			baudrate = B2400;
			break;
		case 4800:
			baudrate = B4800;
			break;
		case 9600:
			baudrate = B9600;
			break;
		case 19200:
			baudrate = B19200;
			break;
		case 38400:
			baudrate = B38400;
			break;
		case 57600:
			baudrate = B57600;
			break;
		case 115200:
			baudrate = B115200;
			break;
		
	}
		return baudrate;

}



int selectPort(){


  printf("\n\n\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
    "+                                                      +\n"
    "+                                                      +\n"
		"+              Select which port to use:               +\n"
    "+                                                      +\n"
		"+              1 - /dev/ttyS0                          +\n"
    "+              2 - /dev/ttyS1                          +\n"
    "+                                                      +\n"
    "+                                                      +\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
);

char portNum;
scanf("%c", &portNum);


if(portNum != '1' && portNum != '2'){
	return -1;

}
else
	return portNum - '0';

}



int selectMaxSize(){


  printf("\n\n\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
    "+                                                      +\n"
    "+                                                      +\n"
		"+        Choose the maximum size of the packet         +\n"
    "+                  must be between:                    +\n"
		"+                     10  -- 200                       +\n"
    "+                                                      +\n"
    "+                                                      +\n"
    "+                                                      +\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
);

char bananas[MAX_SIZE];
scanf("%s", bananas);

int packsize;
sscanf(bananas, "%d", &packsize);

if(packsize <= 9 || packsize >= 199){
	return -1;

}
else

	return packsize;

}



int selectTimeout(){

  printf("\n\n\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
    "+                                                      +\n"
    "+                                                      +\n"
		"+             Insert timeout value (seconds)           +\n"
    "+                   must be between                    +\n"
		"+                      1  --  60                       +\n"
    "+                                                      +\n"
    "+                                                      +\n"
    "+                                                      +\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
);

char bananas[MAX_SIZE];
scanf("%s", bananas);

int timeouttime;
sscanf(bananas, "%d", &timeouttime);

if(timeouttime < 1 || timeouttime > 60){
	return -1;

}
else

	return timeouttime;

}

int selectAttempts(){
  printf("\n\n\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
    "+                                                      +\n"
    "+                                                      +\n"
		"+                Insert attempts value                 +\n"
    "+                   must be between                    +\n"
		"+                      1  --  60                       +\n"
    "+                                                      +\n"
    "+                                                      +\n"
    "+                                                      +\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
);

char bananas[MAX_SIZE];
scanf("%s", bananas);

int attemptnum;
sscanf(bananas, "%d", &attemptnum);


if(attemptnum < 1 || attemptnum > 60){
	return -1;

}
else

	return attemptnum ;

}


void getfilename(char *filenamevar){
	printf("\n\n\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
    "+                                                      +\n"
    "+                                                      +\n"
    "+                                                      +\n"
		"+                   Insert filename                    +\n"
		"+                                                      +\n"
    "+                                                      +\n"
    "+                                                      +\n"
    "+                                                      +\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
);
	char dir[1024]; // but why?
	char fname[256];
	getcwd(dir, sizeof(dir));
	printf("%s\n", dir);
	int stop_reading = 1;
	int i = 0;
	while(stop_reading){
		char letra;
		read(STDIN_FILENO,&letra,1);
		if(letra == '\n') {
			fname[i] = '\0';
			stop_reading = 0;
		}
		else fname[i] = letra;
		i++;
	}
	

	char * bananas = strcat(dir,"/");
	bananas = strcat(bananas,fname);
	strcpy(filenamevar, fname);

}
