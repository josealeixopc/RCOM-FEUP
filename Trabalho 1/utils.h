#pragma once

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

//Group of flags for verifying connection
#define FLAG 0x7e

#define A_SND 0x03	// commands sent by sender and responses given by receiver
#define A_RCV 0x01	// commands sent by receiver and responses given by sender

#define C_SET 0x03
#define C_UA 0x07
#define C_DISC 0x0b

#define RR_0 0x05
#define RR_1 0x85

#define REJ_0 0x01
#define REJ_1 0x81

#define MAX_SIZE 255
#define MAX_TRIES 3


#define ESCAPE 0x7d
#define XOR_BYTE 0x20

char SET[5] = {FLAG, A_SND, C_SET, A_SND^C_SET, FLAG};
char UA[5] = {FLAG, A_SND, C_UA, A_SND^C_UA, FLAG};

int badSET(char* SET)
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

int badUA(char *UA)
{
  if (UA[0] != FLAG)
    return -1;

  if (UA[1] != A_SND)
    return -2;

  if (UA[2] != C_UA)
    return -3;

  if (UA[3] != (A_SND^C_UA))
    return -4;

  if (UA[4] != FLAG)
    return -5;

  return 0;
}

struct applicationLayer {
	int fileDescriptor; /*Descritor correspondente à porta série*/
	int status;	/*TRANSMITTER | RECEIVER*/
};
//ayy lmao :)
struct linkLayer {
	char port[20]; /*Dispositivo /dev/ttySx, x = 0, 1*/
	int baudRate;	/*Velocidade de transmissão (no clue) ??!?*/
	int sequenceNumber;   /*Número de sequência da trama: 0, 1*/
	int timeout;	/*Valor do temporizador: 1 s*/
	int numTransmissions; /*Número de tentativas em caso de falha*/
	char frame[MAX_SIZE];	/*Trama*/
};

// BEGIN OF ARRAY STRUCT

typedef struct {
  char* array;
  size_t used;
  size_t size;
} Array;

void initArray(Array *a, size_t initialSize) {
  a->array = (char* )malloc(initialSize * sizeof(char*));
  a->used = 0;
  a->size = initialSize;

  if(initialSize <= 0)
  {
      printf("ERROR: Array size cannot be 0 or less.\n");
  }
}

void insertArray(Array *a, char element) {
  if (a->used == a->size) {
    a->size *= 1;
    a->array = (char*)realloc(a->array, a->size * sizeof(char*));
  }
  a->array[a->used++] = element;
}

void copyArray(char* source, Array* destiny, size_t length)
{
	printf ("%d, ", 10);

	for(int i = 0; i < length; i++)
  {      
		printf ("%d, ", i);
  	insertArray(destiny, source[i]);
  }
}

void freeArray(Array *a) {
  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}

// END OF ARRAY STRUCT

int printHexArray(char* array, size_t length)
{
	for(unsigned int i = 0; i < length; i++)
	{
		printf ("0x%x ", array[i]);
	}
  printf ("\n");
}