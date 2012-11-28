#ifndef _FTP_CLIENT_H_
#define _FTP_CLIENT_H_

#define FTP_OK          0
#define FTP_FAIL        -1
#define FTP_SERVER_DOWN -2

struct ftp_connect {
        char hostname[64];
        char username[32];
        char password[32];
        unsigned short control_port;
        int control_fd;
        int data_fd;
        int mode;       // 0, pasv; 1, port
};

struct file_description {
        char remote_dir[256];
        char file_name[256];
        char *file_buffer;
        unsigned int file_size;
};


/**
 * @brief       ftp_upload  To upload file to destination host
 *
 * @param       client  [I ] Ftp client description information
 * @param       file    [I ] File to be uploaded
 *
 * @return      result of upload
 * @retval      -1      failed
 * @retval      -2      destination host off line
 * @retval      0       success
 */

int ftp_upload(struct ftp_connect *client, struct file_description *file);


/**
 * @brief       ftp_open Create a ftp client instance to interact with remote host
 *
 * @param       client  [I ] Ftp client description information
 *
 * @return      result of initial 
 * @retval      -1      failed
 * @retval      0       success
 */

int ftp_open(struct ftp_connect *client);


/**
 * @brief       ftp_close Close ftp client instance
 *
 * @param       client  [I ] Ftp client description information
 *
 * @return      result of release
 * @retval      -1      failed
 * @retval      0       success
 */

int ftp_close(struct ftp_connect *client);


/**
 * @brief       ftp_init Initialize ftp client resource
 * @remark      TO use it only in the first time initial
 */
void ftp_init(void);


/**
 * @brief       ftp_release Release ftp client resource
 * @remark      To use it at last to release all resource alloc in initial
 */
void ftp_release(void);


#endif
