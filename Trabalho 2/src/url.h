#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <arpa/inet.h>

/*
 * URL storage
 */
typedef struct parsed_url {
    const char *url;
    char *scheme;               /* Url Scheme -> https, ftp, mailto, etc. It is mandatory */
    char *host;                 /* mandatory */
    int port;                   /* optional */
    char *path;                 /* optional */
    char *file_name;            /* optional */
    char *username;             /* optional */
    char *password;             /* optional */
    char *ip;
} Parsed_URL;

Parsed_URL* parse_url(const char *);
int hostToIP(Parsed_URL*);
void printParsedUrl(Parsed_URL *);
void freeUrlStruct(Parsed_URL *);
