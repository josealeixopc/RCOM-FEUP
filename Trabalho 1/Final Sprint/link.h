#pragma once

#include "utils.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define TRANSMITTER 1
#define RECEIVER 0

//Group of flags for verifying connection
#define FLAG 0x7e

#define A_SND 0x03	// commands sent by sender and responses given by receiver
#define A_RCV 0x01	// commands sent by receiver and responses given by sender

#define C_SET 0x03
#define C_UA 0x07
#define C_DISC 0x0b
#define C_FRAME_0 0x00  // sequence number 0
#define C_FRAME_1 0x40  // sequence number 1

#define RR_0 0x05   // ready for frame 0
#define RR_1 0x85   // ready for frame 1

#define REJ_0 0x01  // error receiving frame 0, send 0 again
#define REJ_1 0x81  // error receiving frame 1, send 1 again

#define MAX_TRIES 3
#define TIMEOUT 3

#define ESCAPE 0x7d
#define XOR_BYTE 0x20

// State machine outputs

#define SM_FAILED   -1
#define SM_SET      0
#define SM_UA       1
#define SM_I0       2
#define SM_I1       3
#define SM_RR0      4
#define SM_RR1      5
#define SM_REJ0     6
#define SM_REJ1     7
#define SM_DISC     8
#define SM_SUCCESS  9

//Alarm handler 

void alarmHandler();

// Verifying commands

/* The return is 0 if the command is OK, otherwise it returns a negative number */
int badSET(unsigned char* set);
int badUA(unsigned char *ua);
int badDisc(unsigned char *disc);

// Byte stuffing & unstuffing

/* The return is the number of bytes altered (number of ESCAPE and FLAG bytes) in frame */
int byteStuff(Array* inArray, Array* outArray);
int byteUnstuff(Array* inArray, Array* outArray);

/* Frame preparation functions */


/* Comunication functions */

// Open & close
int llopen(ApplicationLayer* appL, LinkLayer* linkL, struct termios* oldtio);
int llclose(ApplicationLayer* appL, struct termios* oldtio);

// [TRANSMITTER]
/* Returns 0 on success. Otherwise, returns a negative value */
int llwrite(int fd, unsigned char* packet, size_t packetLength, LinkLayer* linkL, Stats* stats);

// [RECEIVER]
/* Returns 0 if current frame is correct. Returns -1 if it is to be discarded. */
int llread(int fd, unsigned char* packet, size_t* packetLength, LinkLayer* linkL, Stats* stats);
