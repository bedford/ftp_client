#ifndef _FTP_CLIENT_H_
#define _FTP_CLIENT_H_

#define FTP_OK          0
#define FTP_FAIL        -1
#define FTP_SERVER_DOWN -2

struct ftp_connect {
        char hostname[64];
        char username[32];
        char password[32];
        unsigned short port;
        int control_fd;
        int data_fd;
};

struct file_description {
        char remote_dir[256];
        char file_name[256];
        char *file_buffer;
        unsigned int file_size;
};

#endif
