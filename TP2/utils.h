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

#define MAX_TRIES 3

#define ESCAPE 0x7d
#define XOR_BYTE 0x20

