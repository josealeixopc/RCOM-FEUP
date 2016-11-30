#include "ftp.h"

/* Works for both control and data socket. */
/* @return The socket file descriptor that results from the given IP and Port. */
static int connectSocket(const char* server_ip, int server_port) 
{
    
    int socket_fd;

    struct sockaddr_in socket_address;
    // sockaddr_in will store the socket address in the internet, which is the combination of the IP and the Port

	bzero((char*)&socket_address, sizeof(socket_address));      // set to 0 every byte <=> clean memory
	socket_address.sin_family = AF_INET;                        // address family. Normaly AF_INET
	socket_address.sin_addr.s_addr = inet_addr(server_ip);	    /*32 bit Internet address network byte ordered*/
	socket_address.sin_port = htons(server_port);		        /*server TCP port must be network byte ordered */

    /*open the socket*/
	if ((socket_fd = socket(AF_INET,SOCK_STREAM,0)) < 0) 
    {
    	perror("socket()");
        exit(0);
    }
	/*connect to the server*/
    if(connect  (socket_fd, 
	            (struct sockaddr *)&socket_address, 
		        sizeof(socket_address)) 
                < 0)
    {
        perror("connect()");
		exit(0);
	}

	return socket_fd;
}

int ftpWrite(FTP_Socket* ftp, const char* str, size_t size) 
{
    int bytes_written = 0;

    bytes_written = write(ftp->control_fd, str, size);

    if(bytes_written <= 0)
    {
        printf("Error: nothing was sent to the FTP control socket.\n");
        return -1;
    }

    if(DEBUG)
    {
        printf("Info sent: %s\nBytes sent: %d\n", str, bytes_written);
    }

	return 0;
}

int ftpRead(FTP_Socket* ftp, char* str, size_t size) 
{
    FILE* control_socket = fdopen(ftp->control_fd, "r");    // read-only

    memset(str, 0, size); // "clean" str array

    /* The last line of a reply will begin with the a code, followed immediately by Space , optionally some text, and the Telnet end-of-line code. */
    /* Replies have codes between 1xx and 5xx according to RFC 959, so the first (str[0]) has to be between 1-5 AND/OR the fourth (str[4])*/
    while(! ('1' <= str[0] && str[0] <= '5') || str[3] || str[3] == ' ')
    {
        memset(str, 0, size);   // "clean" garbage hat may have been collected
        str = fgets(str, size, control_socket); // read socket until the reply is obtained

        if(DEBUG)
        {
            printf("%s", str);
        }
    }

	return 0;
}

/* Make initial connection  */
int ftpConnect(FTP_Socket* ftp, const char* ip, int port) {

	return 0;
}

int ftpLogin(FTP_Socket* ftp, const char* user, const char* password) {
	

	return 0;
}

int ftpCWD(FTP_Socket* ftp, const char* path) {

	return 0;
}

int ftpPasv(FTP_Socket* ftp) {

	return 0;
}

int ftpRetr(FTP_Socket* ftp, const char* filename) {

	return 0;
}

int ftpDownload(FTP_Socket* ftp, const char* filename) {

	return 0;
}

int ftpDisconnect(FTP_Socket* ftp) {

	return 0;
}
