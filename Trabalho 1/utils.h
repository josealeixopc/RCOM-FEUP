#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define MAX_SIZE 255

#define DEBUG 1

typedef struct {
	int fileDescriptor; /*Descritor correspondente à porta série*/
	int status;	/*TRANSMITTER | RECEIVER*/
} ApplicationLayer;

typedef struct {
	char port[20]; /*Dispositivo /dev/ttySx, x = 0, 1*/
	int baudRate;	/*Velocidade de transmissão (no clue) ??!?*/
	int sequenceNumber;   /*Número de sequência da trama: 0, 1*/
	int timeout;	/*Valor do temporizador: 1 s*/
	int numTransmissions; /*Número de tentativas em caso de falha*/
	char frame[MAX_SIZE];	/*Trama*/
} LinkLayer;

typedef struct {
  unsigned char* array;
  size_t used;
  size_t size;
} Array;

typedef struct {
    unsigned int framesSent;
    unsigned int framesReceived;

    unsigned int numTimeouts;

    unsigned int numRR;

    unsigned int numREJ;

} Stats;


void initArray(Array *a, size_t initialSize);

void insertArray(Array *a, unsigned char element);

void copyArray(unsigned char* source, Array* destiny, size_t length);

void freeArray(Array *a);

/* Prints array elements in hexadecimal form*/
void printHexArray(Array* array);
void printHexBuffer(unsigned char* array, size_t length);

