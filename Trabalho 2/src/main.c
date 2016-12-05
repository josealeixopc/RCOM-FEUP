#include "url.h"
#include "ftp.h"

int main(int argc, char** argv)
{
    if (argc != 2) {
		printf("WARNING: Wrong number of arguments.\n");
		
        printf("Use with user and password: ftp://[<user>:<password>@]<host>/<url-path>\n");

        printf("Use anonymously: ftp://<host>/<url-path>\n\n");

		return -1;
    }   

    // Obtain URL

    Parsed_URL* purl = parse_url(argv[1]);

    if(DEBUG)
    {
        printParsedUrl(purl);
    }

    // If host is unavailable/does not exist
    if(hostToIP(purl) < 0)
    {
        printf("ERROR: cannot associate IP to host '%s'.\n", purl->host);
        return -1;
    }


    printf("Host: '%s'\nIP: %s\n", purl->host, purl->ip);

    // Start FTP connection

    FTP_Socket ftp;
    ftpConnect(&ftp, purl->ip, purl->port);


    return 0;
}

