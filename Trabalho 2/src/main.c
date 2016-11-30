#include "url.h"
#include "ftp.h"

int main(int argc, char** argv)
{

    Parsed_URL* purl = parse_url("http://ipv4.download.thinkbroadband.com:8080/20MB.zip");

    printParsedUrl(purl);

    return 0;
}