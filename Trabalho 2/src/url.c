#include "url.h"

/*
 * Prototype declarations
 */
static __inline__ int _is_scheme_char(int);

/*
 * Check whether the character is permitted in scheme string
 */
static __inline__ int _is_scheme_char(int c)
{
    return (!isalpha(c) && '+' != c && '-' != c && '.' != c) ? 0 : 1;
}

Parsed_URL * parse_url(const char *url)
{
    Parsed_URL *purl;
    const char *tmpstr;
    const char *curstr;
    int len;
    int i;
    int userpass_flag;
    int bracket_flag;

    /* Allocate the parsed url storage */
    purl = malloc(sizeof(Parsed_URL));
    if ( NULL == purl ) {
        return NULL;
    }
    purl->url = url;
    purl->scheme = NULL;
    purl->host = NULL;
    purl->port = 21;
    purl->path = NULL;
    purl->file_name = NULL;
    purl->username = NULL;
    purl->password = NULL;
    purl->ip = NULL;

    curstr = url;

    /*
     * <scheme>:<scheme-specific-part>
     * <scheme> := [a-z\+\-\.]+
     *             upper case = lower case for resiliency
     */

    /* Read scheme */
    tmpstr = strchr(curstr, ':');
    // strchr returns a POINTER to the first ocurrence of the given character in the string

    if ( NULL == tmpstr ) {
        printf("Scheme : delimiter not found!\n");
        freeUrlStruct(purl);
        return NULL;
    }

    /* Get the scheme length */
    len = tmpstr - curstr;
    // len -> gives the number of bytes between the two pointers a.k.a. number of letters

    /* Check restrictions */
    for ( i = 0; i < len; i++ ) {//todo pode-se optimizar isto aqui, passa-se para baixo e evita-se fazer 2 ciclos
        if ( !_is_scheme_char(curstr[i]) ) {
            /* Invalid format */
            freeUrlStruct(purl);
            return NULL;
        }
    }
    /* Copy the scheme to the storage */
    purl->scheme = malloc(sizeof(char) * (len + 1));
    if ( NULL == purl->scheme ) {//??????????? XDDDDD
        freeUrlStruct(purl);
        return NULL;
    }

    (void)strncpy(purl->scheme, curstr, len);

    purl->scheme[len] = '\0';
    /* Make the character to lower if it is upper case. */
    for ( i = 0; i < len; i++ ) {
        purl->scheme[i] = tolower(purl->scheme[i]);
    }
    /* Skip ':' */
    tmpstr++;

    curstr = tmpstr;    // work with the rest of the string

    /*
     * //<user>:<password>@<host>:<port>/<url-path>
     * Any ":", "@" and "/" must be encoded.
     */

    /* Eat "//" */
    for ( i = 0; i < 2; i++ ) {
        if ( '/' != *curstr ) {
            freeUrlStruct(purl);
            return NULL;
        }
        curstr++;
    }

    /* Check if the user (and password) are specified. */
    userpass_flag = 0;
    tmpstr = curstr;
    while ( '\0' != *tmpstr ) {
        if ( '@' == *tmpstr ) {
            /* Username and password are specified */
            userpass_flag = 1;
            break;
        } else if ( '/' == *tmpstr ) {
            /* End of <host>:<port> specification */
            userpass_flag = 0;
            break;
        }
        tmpstr++;
    }

    /* User and password specification */
    tmpstr = curstr;
    if ( userpass_flag ) {
        /* Read username */
        while ( '\0' != *tmpstr && ':' != *tmpstr && '@' != *tmpstr ) {
            tmpstr++;
        }
        len = tmpstr - curstr;
        purl->username = malloc(sizeof(char) * (len + 1));
        if ( NULL == purl->username ) {
            freeUrlStruct(purl);
            return NULL;
        }
        (void)strncpy(purl->username, curstr, len);
        purl->username[len] = '\0';
        /* Proceed current pointer */
        curstr = tmpstr;
        if ( ':' == *curstr ) {
            /* Skip ':' */
            curstr++;
            /* Read password */
            tmpstr = curstr;
            while ( '\0' != *tmpstr && '@' != *tmpstr ) {
                tmpstr++;
            }
            len = tmpstr - curstr;
            purl->password = malloc(sizeof(char) * (len + 1));
            if ( NULL == purl->password ) {
                freeUrlStruct(purl);
                return NULL;
            }
            (void)strncpy(purl->password, curstr, len);
            purl->password[len] = '\0';
            curstr = tmpstr;
        }
        /* Skip '@' */
        if ( '@' != *curstr ) {
            freeUrlStruct(purl);
            return NULL;
        }
        curstr++;
    }

    if ( '[' == *curstr ) {
        bracket_flag = 1;
    } else {
        bracket_flag = 0;
    }
    /* Proceed on by delimiters with reading host */
    tmpstr = curstr;
    while ( '\0' != *tmpstr ) {
        if ( bracket_flag && ']' == *tmpstr ) {
            /* End of IPv6 address. */
            tmpstr++;
            break;
        } else if ( !bracket_flag && (':' == *tmpstr || '/' == *tmpstr) ) {
            /* Port number is specified. */
            break;
        }
        tmpstr++;
    }
    len = tmpstr - curstr;
    purl->host = malloc(sizeof(char) * (len + 1));
    if ( NULL == purl->host || len <= 0 ) {
        freeUrlStruct(purl);
        return NULL;
    }
    (void)strncpy(purl->host, curstr, len);
    purl->host[len] = '\0';
    curstr = tmpstr;

    /* Is port number specified? */
    if ( ':' == *curstr ) {
        curstr++;
        /* Read port number */
        tmpstr = curstr;
        while ( '\0' != *tmpstr && '/' != *tmpstr ) {
            tmpstr++;
        }
        len = tmpstr - curstr;
        char * port = malloc(sizeof(char) * (len + 1));
        if ( NULL == port ) {
            freeUrlStruct(purl);
            return NULL;
        }
        (void)strncpy(port, curstr, len);
        port[len] = '\0';
      //  purl->port = atoi(port);
        curstr = tmpstr;
    }

    /* End of the string */
    if ( '\0' == *curstr ) {
        return purl;
    }

    /* Skip '/' */
    if ( '/' != *curstr ) {
        freeUrlStruct(purl);
        return NULL;
    }
    curstr++;

    /* Parse path */
    tmpstr = curstr;
    while ( '\0' != *tmpstr && '#' != *tmpstr  && '?' != *tmpstr ) {
        tmpstr++;
    }
    const char * temp_name = tmpstr;
    while('/' != *temp_name){
        temp_name--;
    }
    len = tmpstr - curstr;
    int name_l = tmpstr - temp_name - 1;

    len = tmpstr - curstr - name_l;
    purl->path = malloc(sizeof(char) * (len + 1));
    if ( NULL == purl->path ) {
        freeUrlStruct(purl);
        return NULL;
    }
    (void)strncpy(purl->path, curstr, len);
    purl->path[len] = '\0';
    curstr = temp_name + 1;

    purl->file_name = malloc(sizeof(char) * (name_l + 1));
    if ( NULL == purl->file_name ) {
        printf("ERROR: @ file_name");
        freeUrlStruct(purl);
        return NULL;
    }
    (void)strncpy(purl->file_name, curstr, name_l);
    purl->file_name[name_l] = '\0';
    curstr = tmpstr;

    if(hostToIP(purl) == -1){
        freeUrlStruct(purl);
        return NULL;
    }

    return purl;
}


int hostToIP(Parsed_URL *purl){
    	struct hostent *h;

        if ((h=gethostbyname(purl->host)) == NULL) {
            herror("gethostbyname");
            return -1;
        }

        // Takes a network address (in a in_addr struct) and converts it into a Dots-And-Numbrs format (NTOA == Network To ASCII)
        purl->ip = inet_ntoa(*((struct in_addr *)h->h_addr));

        return 0;
}

void printParsedUrl(Parsed_URL *purl){
    if ( NULL != purl ) {
        if ( NULL != purl->scheme ) {
            printf("Scheme: %s\n", purl->scheme);
        }
        if ( NULL != purl->host ) {
            printf("Host: %s\n", purl->host);
        }
        if ( 0 != purl->port ) {
            printf("Port: %d\n", purl->port);
        }
        if ( NULL != purl->path ) {
            printf("Path: %s\n", purl->path);
        }
        if ( NULL != purl->file_name ) {
            printf("File name: %s\n", purl->file_name);
        }
        if ( NULL != purl->username ) {
            printf("Username: %s\n", purl->username);
        }
        if ( NULL != purl->password ) {
            printf("Password: %s\n", purl->password);
        }
        if ( NULL != purl->ip ) {
            printf("IP: %s\n", purl->ip);
        }
    }
}

/*
 * Free memory of parsed url
 */
void freeUrlStruct(Parsed_URL *purl)
{
    if ( NULL != purl ) {
        if ( NULL != purl->scheme ) {
            free(purl->scheme);
        }
        if ( NULL != purl->host ) {
            free(purl->host);
        }
        if ( NULL != purl->path ) {
            free(purl->path);
        }
        if ( NULL != purl->file_name ){
            free(purl->file_name);
        }
        if ( NULL != purl->username ) {
            free(purl->username);
        }
        if ( NULL != purl->password ) {
            free(purl->password);
        }
        free(purl);
    }
}
