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

char SET[5]; 
char UA[5];

int badSET(char* set);

int badUA(char *ua);

struct applicationLayer {
	int fileDescriptor; /*Descritor correspondente à porta série*/
	int status;	/*TRANSMITTER | RECEIVER*/
};

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

void initArray(Array *a, size_t initialSize);

void insertArray(Array *a, char element);

void copyArray(char* source, Array* destiny, size_t length);

void freeArray(Array *a);

// END OF ARRAY STRUCT


int printHexArray(char* array, size_t length);