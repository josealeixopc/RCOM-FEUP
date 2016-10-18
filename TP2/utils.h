#pragma once

#include <signal.h>
#include <stdlib.h>

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

#define MAX_SIZE 255
#define MAX_TRIES 3


#define ESCAPE 0x7d
#define XOR_BYTE 0x20

unsigned char SET[5] = {FLAG, A_SND, C_SET, A_SND^C_SET, FLAG};
unsigned char UA[5] = {FLAG, A_SND, C_UA, A_SND^C_UA, FLAG};

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

int badUA(unsigned char *UA)
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
	unsigned int sequenceNumber;   /*Número de sequência da trama: 0, 1*/
	unsigned int timeout;	/*Valor do temporizador: 1 s*/
	unsigned int numTransmissions; /*Número de tentativas em caso de falha*/
	char frame[MAX_SIZE];	/*Trama*/
};

