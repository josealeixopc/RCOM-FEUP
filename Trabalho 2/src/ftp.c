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
    while(! ('1' <= str[0] && str[0] <= '5')  || str[3] != ' ')
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

    int socket_fd;
    char response[1024];
    if((socket_fd = connectSocket(ip, port)) < 0)
    {
        printf("ERROR: Cannot establish connection to socket @ ftpConnect().\n");
        return -1;
    }
    ftp->control_fd = socket_fd;    // the connected socket is the control socket
    ftp->data_fd = 0;               // still to be defined
    if(ftpRead(ftp, response, sizeof(response)))
    {
        printf("ERROR: ftpRead failed @ ftpConnect().\n");
        return -2;
    }
	return 0;
}

/* Login with a given username and password. The server may then grant or deny access. */
int ftpLogin(FTP_Socket* ftp, const char* user, const char* password) {

    char command[1024];

    // Send user name
    sprintf(command, "USER %s\r\n", user);  // FTP commands are terminated with CRLF -> CR == \r, LF == \n
    if(ftpWrite(ftp, command, strlen(command)))
    {
        printf("ERROR: ftpWrite failed @ ftpLogin().\n");
        return -1;
    }


    if(ftpRead(ftp, command, sizeof(command)))
    {
        printf("ERROR: Access to server denied when sending user name @ ftpLogin().\n");
        return -2;
    }

    // Set command to 0s again
    memset(command, 0, sizeof(command));

    // Send passwords
    sprintf(command, "PASS %s\r\n", password);  // FTP commands are terminated with CRLF -> CR == \r, LF == \n
    if(ftpWrite(ftp, command, strlen(command)))
    {
        printf("ERROR: ftpWrite failed @ ftpLogin().\n");
        return -3;
    }


    if(ftpRead(ftp, command, sizeof(command)))
    {
        printf("ERROR: Access to server denied when sending user name @ ftpLogin().\n");
        return -4;
    }

	return 0;
}

int ftpCWD(FTP_Socket* ftp, const char* path) {

    char cwdCommand[1024];

    sprintf(cwdCommand, "CWD %s\r\n", path); // change directory in server to correspond to the path o the file

    if(ftpWrite(ftp, cwdCommand, strlen(cwdCommand)))
    {
        printf("ERROR: ftpWrite failed @ ftpCWD().\n");
        return -1;
    }

    if(ftpRead(ftp, cwdCommand, sizeof(cwdCommand)))
    {
        printf("ERROR: Access to server denied when sending path @ ftpCWD().\n");
        return -2;
    }

	return 0;
}

int ftpPassive(FTP_Socket* ftp) {

  char pasv[1024] = "pasv\r\n";
	if(ftpWrite(ftp, pasv, strlen(pasv))){
      printf("ERROR: ftpWrite failed @ ftpPassive().\n");
      return -1;
	}

  if(ftpRead(ftp, pasv, sizeof(pasv))){
      printf("ERROR: No answer from server @ ftpPassive\n");
      return -2;
  }

  int rec_ip[5]; //first four octets are the IP address while the last two octets comprise the port that will be used for the data connection.
  if(sscanf(pasv,"227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)",
      &rec_ip[0],&rec_ip[1],&rec_ip[2],&rec_ip[3],&rec_ip[4],&rec_ip[5]) < 0){
      printf("ERROR: invalid received information @ ftpPassive");
      return -3;
  }

  //memset(pasv,0,sizeof(pasv));
  char ip[1024];
  memset(ip,0,sizeof(ip));//buggy crap :/
  sprintf(ip,"%d.%d.%d.%d",rec_ip[0],rec_ip[1],rec_ip[2],rec_ip[3]);
  int port = rec_ip[4] * 256 + rec_ip[5];

  if((ftp->data_fd = connectSocket(ip, port)) < 0)
  {
      printf("ERROR: Cannot establish connection to socket @ ftpPassive().\n");
      return -4;
  }

	return 0;
}

int ftpRequest(FTP_Socket* ftp, const char* file_name) {
  char retrieve_cmd[1024];
  sprintf(retrieve_cmd, "RETR %s\r\n", file_name);
  if (ftpWrite(ftp, retrieve_cmd, strlen(retrieve_cmd))) {
      printf("ERROR: ftpWrite failed @ ftpRequest().\n");
      return -1;
  }
  if (ftpRead(ftp, retrieve_cmd, sizeof(retrieve_cmd))) {
      printf("ERROR: No answer from server @ ftpRequest()\n");
      return -2;
  }
	return 0;
}

int ftpDownload(FTP_Socket* ftp, const char* file_name) {
  int rec_bytes = 0;
  int file_d = open(file_name, O_WRONLY | O_TRUNC | O_CREAT, 0660);
  char file_content[1024];

  while((rec_bytes = read(ftp->data_fd, file_content, sizeof(file_content))) > 0){
      if(write(file_d, file_content, rec_bytes) < 0){
          printf("ERROR: failed to write to file %s",file_name);
          return -1;
      }
  }
  close(file_d);
  close(ftp->data_fd);

  memset(file_content,0,sizeof(file_content));
  if (ftpRead(ftp, file_content, sizeof(file_content))) {
      printf("ERROR: No answer from server @ ftpDisconnect()\n");
      return -2;
  }

	return 0;
}

int ftpDisconnect(FTP_Socket* ftp) {

  char disc_cmd[1024] = "QUIT\r\n";
  char lixo[1024];

  if(ftpWrite(ftp, disc_cmd, strlen(disc_cmd))){
      printf("ERROR: ftpWrite failed @ ftpDisconnect().\n");
      return -1;
  }
  memset(lixo,0,sizeof(lixo));
  if (ftpRead(ftp, lixo, sizeof(lixo))) {
      printf("ERROR: No answer from server @ ftpDisconnect()\n");
      return -2;
  }
  close(ftp->control_fd);
	return 0;
}
