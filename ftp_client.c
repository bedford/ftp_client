#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "ftp_types.h"
#include "ftp_client.h"
#include "socket.h"

#define kMaxResponseSize        256

static const int kSocketTimeout = 2000;
static const int kMinLength = 32;

static int kInit = 0;
static struct _ftp_cmd *ftp_cmd = NULL;
static char response[kMaxResponseSize];

static void processSignal(int signo)
{
        printf("signal %d\n", signo);
        signal(signo, processSignal);
}

static void ftp_cmd_init(void)
{
        int size = sizeof(struct _ftp_cmd) * MaxIndex;
        ftp_cmd = (struct _ftp_cmd *)malloc(size);
        bzero(ftp_cmd, size);

        ftp_cmd[0].cmd                  = "USER";
        ftp_cmd[0].response_code        = RES_REQUIRE_PASSWORD;
        ftp_cmd[1].cmd                  = "PASS";
        ftp_cmd[1].response_code        = RES_LOGIN_PASS;
        ftp_cmd[2].cmd                  = "QUIT";
        ftp_cmd[2].response_code        = RES_QUIT_SUCCESS;
        ftp_cmd[3].cmd                  = "PWD";
        ftp_cmd[3].response_code        = RES_PWD;
        ftp_cmd[4].cmd                  = "MKD";
        ftp_cmd[4].response_code        = RES_MKDIR;
        ftp_cmd[5].cmd                  = "CWD";
        ftp_cmd[5].response_code        = RES_CHANGE_DIR;
        ftp_cmd[6].cmd                  = "STOR";
        ftp_cmd[6].response_code        = RES_UPLOAD;
        ftp_cmd[7].cmd                  = "TYPE I";
        ftp_cmd[7].response_code        = RES_EXCUTE_SUCCESS;
        ftp_cmd[8].cmd                  = "PASV";
        ftp_cmd[8].response_code        = RES_PASSIVE;
        ftp_cmd[9].cmd                  = "PORT";
        ftp_cmd[9].response_code        = RES_EXCUTE_SUCCESS;
        ftp_cmd[10].cmd                 = "SIZE";
        ftp_cmd[10].response_code       = RES_UPLOADED_SIZE;
        ftp_cmd[11].cmd                 = "REST";
        ftp_cmd[11].response_code       = RES_REST_POSITION;

        kInit = 1;
}

static int ftp_get_response(struct ftp_connect *ftp_client)
{
        char response_code[4] = {0};
        memset(response, 0, sizeof(response));

        int len = socket_readn(ftp_client->control_fd, response,
                                kMaxResponseSize, kSocketTimeout);
        if (len <= 0) {
                //printf("response buffer is %d\n", len);
                return FTP_FAIL;
        }
        strncpy(response_code, response, 3);

        socket_clear_recv_buffer(ftp_client->control_fd);
        //printf("response code is %s\n", response_code);

        return atoi(response_code);
}

static int ftp_connect(struct ftp_connect *ftp_client)
{
        int ret = -1;
        ftp_client->control_fd = socket_create_tcp();
        socket_set_nonblock(ftp_client->control_fd);
        ret = socket_connect(ftp_client->control_fd, ftp_client->hostname,
                                ftp_client->control_port);
        if (ret) {
                return FTP_FAIL;
        }

        socket_set_block(ftp_client->control_fd);
        if (ftp_get_response(ftp_client) == RES_CONNECT_SUCCESS) {
                //printf("connect success\n");
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
        if (append_info) {
                sprintf(cmd, "%s %s\r\n", ftp_cmd[index].cmd, append_info);
        } else {
                sprintf(cmd, "%s\r\n", ftp_cmd[index].cmd);
        }

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

static int ftp_cwd(struct ftp_connect *client, char *path)
{
        return ftp_send_cmd(ChangeDir, path, client);
}

static int ftp_mkdir(struct ftp_connect *client, char *path)
{
        return ftp_send_cmd(Mkdir, path, client);
}

static int ftp_enter_dir(struct ftp_connect *client, char *path)
{
        int ret = 0;
        do {
                ret = ftp_cwd(client, path);
                if (ret == FTP_OK ||
                        ret == FTP_SERVER_DOWN) {
                        break;
                }

                ret = ftp_mkdir(client, path);
                if (ret == FTP_FAIL ||
                        ret == FTP_SERVER_DOWN) {
                        break;
                }

                ret = ftp_cwd(client, path);
                if (ret == FTP_OK ||
                        ret == FTP_SERVER_DOWN) {
                        break;
                }

        } while(0);

        return ret;
}

static int ftp_change_dir(struct ftp_connect *client, char *absolute_path)
{
        int ret = 0;
        char path[32];
        char *p = absolute_path;
        if (*p == '/') {
                ++p;
        }

        ret = ftp_enter_dir(client, "/");
        if (ret != FTP_OK) {
                return ret;
        }

        int index = 0;
        while (1) {
                index = 0;
                memset(path, 0, sizeof(path));
                while ((*p != '/') && (*p != '\0')) {
                        path[index] = *p;
                        ++index;
                        ++p;
                }

                path[index] = '\0';
                if (path[0] != '\0') {
                        ret = ftp_enter_dir(client, path);
                        if (ret != FTP_OK) {
                                break;
                        }
                }

                if (*p == '\0') {
                        break;
                }
                ++p;
        }

        return ret;
}

static int ftp_put_file_port()
{
        return FTP_OK;
}

static int ftp_get_passive(struct ftp_connect *client,
                                struct _data_connect *data_connect)
{
        int ret = ftp_send_cmd(PassiveMode, NULL, client);
        if (ret == FTP_FAIL ||
                ret == FTP_SERVER_DOWN) {
                return ret;
        }

        char *tmp1, *tmp2;
        tmp1 = strstr(response, "(");
        tmp2 = strstr(response, ")");
        if (tmp1 == NULL ||
                tmp2 == NULL) {
                return FTP_FAIL;
        }
        tmp1++;
        *tmp2 = '\0';

        int h1, h2, h3, h4, p1, p2;
        sscanf(tmp1, "%i,%i,%i,%i,%i,%i", &h1, &h2, &h3, &h4, &p1, &p2);
        data_connect->port = (p1 << 8) + p2;
        sprintf(data_connect->addr, "%d.%d.%d.%d", h1, h2, h3, h4);
        /* printf("pasv return addr %s and port is %d\n",
                      data_connect->addr, data_connect->port);*/

        return FTP_OK;
}

static int ftp_get_file_size(struct ftp_connect *client,
                             struct file_description *file,
                             int *exist_size)
{
        int ret = ftp_send_cmd(UploadedSize, file->file_name, client);
        if (ret == FTP_SERVER_DOWN) {
                return FTP_FAIL;
        }

        if (ret == FTP_FAIL) {
                *exist_size = 0;
                return FTP_OK;
        }

        char *tmp = strstr(response, " ");
        tmp++;
        if (tmp == NULL) {
                return FTP_FAIL;
        }
        *exist_size = atoi(tmp);

        ret = ftp_send_cmd(RestPosition, tmp, client);
        if (ret == FTP_SERVER_DOWN ||
                ret == FTP_FAIL) {
                return FTP_FAIL;
        }

        return FTP_OK;
}

static int ftp_put_file_pasv(struct ftp_connect *client,
                                struct file_description *file)
{
        int exist_size = 0;
        int ret = ftp_get_file_size(client, file, &exist_size);
        if (ret == FTP_FAIL) {
                return FTP_FAIL;
        }

        if (exist_size >= file->file_size) {
                return FTP_OK;
        }

        struct _data_connect data_connect;
        ftp_get_passive(client, &data_connect);

        client->data_fd = socket_create_tcp();
        socket_set_nonblock(client->data_fd);
        ret = socket_connect(client->data_fd, data_connect.addr,
                                        data_connect.port);
        socket_set_block(client->data_fd);
        if (ret) {
                socket_close(client->data_fd);
                printf("connect failed\n");
                return FTP_FAIL;
        }

        ret = ftp_send_cmd(Upload, file->file_name, client);
        if (ret) {
                printf("send command failed\n");
                return FTP_FAIL;
        }

        ret = socket_write(client->data_fd, file->file_buffer + exist_size,
                                file->file_size - exist_size, kSocketTimeout);
        socket_close(client->data_fd);

        int response_code = ftp_get_response(client);
        if (response_code == RES_TRANSFER_OK) {
                return FTP_OK;
        } else {
                return FTP_FAIL;
        }
}

static int ftp_put_file(struct ftp_connect *client,
                                struct file_description *file)
{
        int ret = ftp_send_cmd(FileType, NULL, client);
        if (ret == FTP_FAIL ||
                ret == FTP_SERVER_DOWN) {
                return ret;
        }

        if (client->mode) {
                ret = ftp_put_file_port(client, file);
        } else {
                ret = ftp_put_file_pasv(client, file);
        }

        return ret;
}

int ftp_upload(struct ftp_connect *client, struct file_description *file)
{
        int ret = ftp_change_dir(client, file->remote_dir);
        if (ret) {
                return FTP_FAIL;
        }

        return ftp_put_file(client, file);
}

int ftp_open(struct ftp_connect *client)
{
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

int ftp_close(struct ftp_connect *client)
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

void ftp_init(void)
{
        ftp_cmd_init();
        signal(SIGPIPE, processSignal);
}

void ftp_release(void)
{
        free(ftp_cmd);
        ftp_cmd = NULL;
}

