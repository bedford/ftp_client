#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


static int kConnect = 0;

static int ftp_get_response()
{

}

static int ftp_connect(struct ftp_connect *ftp_client)
{
        struct hostent *hp;

        if (ftp_client->status < FTP_WAIT_CONNECT) {
                return FTP_FAIL;
        }

        if ((!ftp_client->hostname) ||
                (!*ftp_client->hostname)) {
                return FTP_FAIL;
        }

        hp = (struct hostent *)gethostbyname(ftp_client->hostname);
        if (!hp) {
                return FTP_FAIL;
        }
}

static int ftp_login()
{

}

int ftp_process(struct ftp_connect *ftp_client)
{
        
}
