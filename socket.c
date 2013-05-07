#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "socket.h"

int socket_create_tcp(void)
{
        int socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        return socket_fd;
}

void socket_set_nonblock(int socket_fd)
{
        int flags = fcntl(socket_fd, F_GETFL);
        flags |= O_NONBLOCK;
        fcntl(socket_fd, F_SETFL, flags);
}

void socket_set_block(int socket_fd)
{
        int flags = fcntl(socket_fd, F_GETFL);
        flags &= ~O_NONBLOCK;
        fcntl(socket_fd, F_SETFL, flags);
}

static int socket_check_writable(int socket_fd, int ms)
{
        fd_set fdw;
        FD_ZERO(&fdw);
        FD_SET(socket_fd, &fdw);

        struct timeval tv;
        tv.tv_sec  = ms / 1000;
        tv.tv_usec = (ms % 1000) * 1000;

        int ret = select(socket_fd + 1, NULL, &fdw, NULL, &tv);
        if (ret < 0) {
                return SOCKET_FAIL;
        } else if (ret == 0) {
                return SOCKET_TIMEOUT;
        }

        if (FD_ISSET(socket_fd, &fdw)) {
                return SOCKET_OK;
        }

        return SOCKET_FAIL;
}

static int socket_check_readable(int socket_fd, int ms)
{
        fd_set fdr;
        FD_ZERO(&fdr);
        FD_SET(socket_fd, &fdr);

        struct timeval tv;
        tv.tv_sec  = ms / 1000;
        tv.tv_usec = (ms % 1000) * 1000;

        int ret = select(socket_fd + 1, &fdr, NULL, NULL, &tv);
        if (ret < 0) {
                return SOCKET_FAIL;
        } else if (ret == 0) {
                return SOCKET_TIMEOUT;
        }

        if (FD_ISSET(socket_fd, &fdr)) {
                return SOCKET_OK;
        }

        return SOCKET_FAIL;
}

int socket_connect(int socket_fd, char *dest_addr, int dest_port)
{
        struct sockaddr_in sin;
        bzero(&sin, sizeof(sin));

        sin.sin_family          = AF_INET;
        sin.sin_port            = htons((u_short)dest_port);
        sin.sin_addr.s_addr     = inet_addr(dest_addr);

        if (connect(socket_fd, (struct sockaddr *)&sin, sizeof(sin)) == 0) {
                return SOCKET_OK;
        }

        switch (errno) {
        case EINPROGRESS:
        case EALREADY:
                usleep(100000);
                break;
        default:
                usleep(100000);
                return SOCKET_FAIL;
        }

        int connect_timeout = 2000;
        if (socket_check_writable(socket_fd, connect_timeout)) {
                return SOCKET_FAIL;
        }

        int error = 0;
        socklen_t len = sizeof(error);
        getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &error, &len);

        if (error == 0) {
                return SOCKET_OK;
        }

        return SOCKET_FAIL;
}

int socket_write(int socket_fd, char *buf, int length, int ms)
{
        int sent_len = 0;
        int n = 0;
        int ret = 0;

        while (sent_len < length) {
                ret = socket_check_writable(socket_fd, ms);
                if (ret != SOCKET_OK) {
                        return ret;
                }

                n = send(socket_fd, buf + sent_len,
                                length - sent_len, 0);
                if (n == -1) {
                        return SOCKET_FAIL;
                } else if (n == 0) {
                        return SOCKET_FAIL;     //copy to send buffer fail
                }

                sent_len += n;
        }

        return SOCKET_OK;
}

int socket_read(int socket_fd, char *buf, int max_length, int ms)
{
        int length  = 0;
        int ret = 0;

        ret = socket_check_readable(socket_fd, ms);
        if (ret == SOCKET_OK) {
                length = recv(socket_fd, buf, max_length, 0);
        } else {
                return ret;
        }

        return length;
}

void socket_clear_recv_buffer(int socket_fd)
{
        int ret = 0;
        int buf_size = 256;
        char buf[buf_size];

        while (1) {
                ret = socket_read(socket_fd, buf, buf_size, 0);
                if ((ret == SOCKET_TIMEOUT)
                        || (ret == SOCKET_FAIL)) {
                        break;
                }
        }
}

int socket_close(int socket_fd)
{
        if (socket_fd) {
                shutdown(socket_fd, SHUT_RDWR);
                close(socket_fd);
        }

        return SOCKET_OK;
}
