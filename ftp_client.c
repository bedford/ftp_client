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
        ftp_cmd[4].cmd                  = "MKD";
        ftp_cmd[4].response_code        = RES_MKDIR;
        ftp_cmd[5].cmd                  = "CWD";
        ftp_cmd[5].response_code        = RES_CDIR;
        ftp_cmd[6].cmd                  = "STOR";
        ftp_cmd[6].response_code        = RES_UPLOAD;

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

        if (ftp_get_response(ftp_client) != ftp_cmd[index].response_code) {
                return FTP_FAIL;
        }

        return FTP_OK;
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

static int ftp_cwd(struct ftp_connect *client)
{

}

int ftp_upload(struct ftp_connect *client)
{
        int ret = ftp_upload(client);
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

        if (ftp_init(&client) == FTP_OK) {
                ftp_upload(&client);
        }

        ftp_release(&client);

        return 0;
}
