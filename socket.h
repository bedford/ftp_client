#ifndef _SOCKET_H_
#define _SOCKET_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SOCKET_OK       0
#define SOCKET_FAIL     -1
#define SOCKET_TIMEOUT  -2

int socket_create_tcp(void);
void socket_set_nonblock(int socket_fd);
void socket_set_block(int socket_fd);
int socket_connect(int socket_fd, char *dest_addr, int dest_port);
int socket_write(int socket_fd, char *buf, int length, int ms);
int socket_read(int socket_fd, char *buf, int max_length, int ms);
int socket_clear_recv_buffer(int socket_fd);
int socket_close(int socket_fd);

#ifdef __cplusplus
}
#endif

#endif
