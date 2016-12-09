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

    // Obtain URL example ftp://ftp.up.pt/pub/robots.txt

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
    // Start FTP connection
    if(purl->username == NULL){
        purl->username = "anonymous";
        printf("Anon mode(4chin mod): Introduce email address pl0x:\n");
        char pass[69];
        fgets(pass, 69, stdin);
        purl->password = (char*) malloc(strlen(pass));
        strncpy(purl->password, pass, strlen(pass) - 1);
    }

    FTP_Socket ftp;
    ftpConnect(&ftp, purl->ip, purl->port);
    ftpLogin(&ftp, purl->username, purl->password);
	  ftpPassive(&ftp);
    ftpCWD(&ftp, purl->path);
    ftpRequest(&ftp, purl->file_name);
    ftpDownload(&ftp,purl->file_name);
    ftpDisconnect(&ftp);

    return 0;
}
