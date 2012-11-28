#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ftp_types.h"
#include "ftp_client.h"
#include "socket.h"

static int kDefaultPort = 21;
static int kMaxResponseSize = 256;
static int kSocketTimeout = 2000;

static char *kDefaultHost = "127.0.0.1";

static int kInit = 0;
static struct _ftp_cmd ftp_cmd[MaxIndex];

static void ftp_cmd_init(void)
{
        ftp_cmd[0].cmd                  = "USER ";
        ftp_cmd[0].response_code        = RES_REQUIRE_PASSWORD;
        ftp_cmd[1].cmd                  = "PASS ";
        ftp_cmd[1].response_code        = RES_LOGIN_PASS;
        ftp_cmd[2].cmd                  = "QUIT";
        ftp_cmd[2].response_code        = RES_QUIT_SUCCESS;
        ftp_cmd[3].cmd                  = "PWD";
        ftp_cmd[3].response_code        = RES_PWD;
        ftp_cmd[4].cmd                  = "MKD ";
        ftp_cmd[4].response_code        = RES_MKDIR;
        ftp_cmd[5].cmd                  = "CWD ";
        ftp_cmd[5].response_code        = RES_CHANGE_DIR;
        ftp_cmd[6].cmd                  = "STOR";
        ftp_cmd[6].response_code        = RES_UPLOAD;
        ftp_cmd[7].cmd                  = "TYPE I";
        ftp_cmd[7].response_code        = RES_EXCUTE_SUCCESS;
        ftp_cmd[8].cmd                  = "SYST";
        ftp_cmd[8].response_code        = RES_SYST;
        ftp_cmd[9].cmd                  = "PASV";
        ftp_cmd[9].response_code        = RES_PASSIVE;
        ftp_cmd[10].cmd                 = "PORT";
        ftp_cmd[10].response_code       = RES_EXCUTE_SUCCESS;

        kInit = 1;
}

static int ftp_get_response(struct ftp_connect *ftp_client)
{
        char response[kMaxResponseSize];
        char response_code[4] = {0};

        int len = socket_read(ftp_client->control_fd, response, 
                                kMaxResponseSize, kSocketTimeout);
        if (len <= 0) {
                printf("response buffer is %d\n", len);
                return FTP_FAIL;
        }
        strncpy(response_code, response, 3);

        socket_clear_recv_buffer(ftp_client->control_fd);
        printf("response code is %s\n", response_code);

        return atoi(response_code);
}

static int ftp_connect(struct ftp_connect *ftp_client)
{
        int ret = -1;
        ftp_client->control_fd = socket_create_tcp();
        socket_set_nonblock(ftp_client->control_fd);
        ret = socket_connect(ftp_client->control_fd, ftp_client->hostname,
                                ftp_client->port);
        if (ret) {
                return FTP_FAIL;
        }

        socket_set_block(ftp_client->control_fd);
        if (ftp_get_response(ftp_client) == RES_CONNECT_SUCCESS) {
                printf("connect success\n");

                socket_clear_recv_buffer(ftp_client->control_fd);
                return FTP_OK;
        }

        return FTP_FAIL;
}

static int ftp_send_cmd(int index,
                        char *append_info,
                        struct ftp_connect *ftp_client)
{
        char cmd[kMaxResponseSize];
        sprintf(cmd, "%s%s\r\n", ftp_cmd[index].cmd, append_info);

        int ret = socket_write(ftp_client->control_fd, cmd,
                                        strlen(cmd), kSocketTimeout);
        if (ret != SOCKET_OK) {
                return FTP_FAIL;
        }

        int response_code = ftp_get_response(ftp_client);
        if (response_code == ftp_cmd[index].response_code) {
                return FTP_OK;
        }

        if (response_code == RES_SERVER_DOWN) {
                return FTP_SERVER_DOWN;
        }

        return FTP_FAIL;
}

static int ftp_login(struct ftp_connect *client)
{
        int ret = ftp_send_cmd(Username, client->username, client);
        if (ret == FTP_OK) {
                ret = ftp_send_cmd(Password, client->password, client);
        }

        return ret;
}

int ftp_init(struct ftp_connect *client)
{
        if (!kInit) {
                ftp_cmd_init();
        }

        int ret = ftp_connect(client);
        if (ret) {
                return FTP_FAIL;
        }

        ret = ftp_login(client);
        if (ret) {
                return FTP_FAIL;
        }

        return FTP_OK;
}

int ftp_release(struct ftp_connect *client)
{
        if (client == NULL) {
                return FTP_FAIL;
        }

        socket_close(client->control_fd);
        socket_close(client->data_fd);

        client->control_fd      = -1;
        client->data_fd         = -1;

        return FTP_OK;
}

static int ftp_cwd(struct ftp_connect *client, char *path)
{
        return ftp_send_cmd(ChangeDir, path, client);
}

static int ftp_mkdir(struct ftp_connect *client, char *path)
{
        return ftp_send_cmd(Mkdir, path, client);
}

static int ftp_change_dir(struct ftp_connect *client, char *absolute_path)
{
        int ret = 0;

        while (1) {
                ret = ftp_cwd(client, absolute_path);
                if (ret == FTP_OK ||
                        ret == FTP_SERVER_DOWN) {
                        break;
                }

                ret = ftp_mkdir(client, absolute_path);
                if (ret == FTP_FAIL ||
                        ret == FTP_SERVER_DOWN) {
                        break;
                }
        }

        return ret;
}

static int ftp_put_file(struct ftp_connect *client, char *filename)
{
        return 0;
}

int ftp_upload(struct ftp_connect *client, struct file_description *file)
{
        int ret = ftp_change_dir(client, file->remote_dir);
        if (ret) {
                return FTP_FAIL;
        }

        return FTP_OK;
}

int main()
{
        struct ftp_connect client;
        memcpy(client.hostname, "192.168.0.107", sizeof(client.hostname));
        client.port = kDefaultPort;
        memcpy(client.username, "odin", sizeof(client.username));
        memcpy(client.password, "odin", sizeof(client.password));

        struct file_description file;
        memcpy(file.remote_dir, "/test/X00/192.168.0.188/20121128/09", sizeof(file.remote_dir));
        memcpy(file.file_name, "a.txt", sizeof(file.file_name));
        int size = 1024 * 1024;
        char *ptr = (char *)malloc(size * sizeof(char));
        file.file_buffer = ptr;
        file.file_size = size;

        if (ftp_init(&client) == FTP_OK) {
                ftp_upload(&client, &file);
        }

        ftp_release(&client);
        free(ptr);
        ptr = NULL;

        return 0;
}
