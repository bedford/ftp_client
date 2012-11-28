#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ftp_client.h"

int main()
{
        ftp_init();

        struct ftp_connect client;
        memcpy(client.hostname, "192.168.0.107", sizeof(client.hostname));
        client.control_port = 21;
        memcpy(client.username, "odin", sizeof(client.username));
        memcpy(client.password, "odin", sizeof(client.password));
        client.mode = 0;

        struct file_description file;
        memcpy(file.remote_dir, "/test/X00/192.168.0.188/20121128/09", sizeof(file.remote_dir));
        memcpy(file.file_name, "a.txt", sizeof(file.file_name));
        int size = 1024 * 1024;
        char *ptr = (char *)malloc(size * sizeof(char));
        file.file_buffer = ptr;
        file.file_size = size;

        if (ftp_open(&client) == FTP_OK) {
                ftp_upload(&client, &file);
        }

        ftp_close(&client);
        free(ptr);
        ptr = NULL;

        ftp_release();

        return 0;
}
