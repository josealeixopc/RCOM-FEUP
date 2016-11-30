#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <netinet/in.h>

#define DEBUG 1

typedef struct ftp_socket
{
    int control_fd;     // file descriptor to control socket
    int data_fd;        // file descriptor to data socket
} FTP_Socket;

/* Writes string to the FTP control socket, given its size.*/
int ftpWrite(FTP_Socket* ftp, const char* str, size_t size);
int ftpRead(FTP_Socket* ftp, char* str, size_t size);

int ftpConnect(FTP_Socket* ftp, const char* ip, int port);
int ftpLogin(FTP_Socket* ftp, const char* user, const char* password);
int ftpCWD(FTP_Socket* ftp, const char* path);
int ftpPasv(FTP_Socket* ftp);
int ftpRetr(FTP_Socket* ftp, const char* filename);
int ftpDownload(FTP_Socket* ftp, const char* filename);
int ftpDisconnect(FTP_Socket* ftp);
