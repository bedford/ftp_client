#ifndef _FTP_CLIENT_H_
#define _FTP_CLIENT_H_

enum {
        FTP_NEW_CONNECT         = 0,
        FTP_WAIT_CONNECT        = 1,
        FTP_WAIT_LOGIN          = 2,
        FTP_IDLE                = 3,
        FTP_WAITING             = 4,
        FTP_TRANSFER            = 5,
        FTP_ERROR               = 100,
};

enum {
        FTP_OK   = 0,
        FTP_FAIL = 1,
};

struct ftp_connect {
        char hostname[64];
        char username[32];
        char password[32];
        char remote_dir[256];
        unsigned short port;
        int control_fd;
        int data_fd;
        int status;
};

#endif
