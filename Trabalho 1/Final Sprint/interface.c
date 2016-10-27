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

	int programfunc;
	scanf("%d", &programfunc);


	if(programfunc != 1 || programfunc != 2){
		return -1;

	}
	else
		return programfunc;




}


int baudarecheck(){


  printf("\n\n\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
    "+                                                      +\n"
    "+                                                      +\n"
    "+                                                      +\n"
    "+                 Insert valid Baudrate:               +\n"
    "+                                                      +\n"
    "+                                                      +\n"
    "+                                                      +\n"
    "+                                                      +\n"
    "++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
);

	int baudrate = 0;

	scanf("%d", &baudrate);

	while(baudrate != 300&&baudrate != 600 && baudrate != 1200&& baudrate != 1800&& baudrate != 2400&& baudrate != 4800
&& baudrate != 9600&& baudrate != 19200&& baudrate != 38400&& baudrate != 57600&& baudrate != 115200){
		printf("\n Wrong Baudrate value please insert a valid one: \n");
		return -1;

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

int portNum;
scanf("%d", &portNum);


if(portNum != 1 || portNum != 2){
	return -1;

}
else
	return portNum;

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

int packsize;
scanf("%d", &packsize);


if(packsize <= 11 || packsize >= 2000){
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

int timeouttime;
scanf("%d", &timeouttime);


if(timeouttime <= 1 || timeouttime >= 60){
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

int attemptnum;
scanf("%d", &attemptnum);


if(attemptnum <= 1 || attemptnum >= 60){
	return -1;

}
else

	return attemptnum;

}


void getfilename(char **filenamevar){
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
	char dir[1024];
	char fname[256];
	getcwd(dir, sizeof(dir));
	printf("%s\n", dir);
	scanf("%s", fname);
	

	*filenamevar = strcat(dir,fname);


}
