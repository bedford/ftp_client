#ifndef _FTP_CLIENT_H_
#define _FTP_CLIENT_H_

#define FTP_OK          0
#define FTP_FAIL        -1

struct ftp_connect {
        char hostname[64];
        char username[32];
        char password[32];
        char remote_dir[256];
        unsigned short port;
        int control_fd;
        int data_fd;
};

#endif
