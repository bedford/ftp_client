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


/**
 * @brief       connect to the dest_addr host, dest_port 
 *
 * @param       socket_fd [I ] client socket file description
 * @param       dest_addr [I ] destination host address
 * @param       dest_port [I ] destination port
 *
 * @return      result of connecting
 * @retval      -1      fail
 * @retval      0       success
 */

int socket_connect(int socket_fd, char *dest_addr, int dest_port);


/**
 * @brief       socket_check_writable To check whether the socket is writable
 *
 * @param       socket_fd [I ] The socket file description to check
 * @param       ms      [I ]   timeout up bound (Unit: ms)
 *
 * @return      result of able to write or not
 * @retval      -2      timeout error
 * @retval      -1      unwritable
 * @retval      0       writable
 */

int socket_write(int socket_fd, char *buf, int length, int ms);


/**
 * @brief       socket_check_readable To check whether the socket is readable
 *
 * @param       socket_fd [I ] The socket file description to check
 * @param       ms      [I ]   timeout up bound (Unit: ms)
 *
 * @return      result of able to read or not
 * @retval      -2      timeout error
 * @retval      -1      unreadable
 * @retval      0       readable
 */

int socket_read(int socket_fd, char *buf, int max_length, int ms);

/**
 * @brief       socket_clear_recv_buffer To clear socket receive buffer
 *
 * @param       socket_fd [I ] The socket file description to clear
 */

void socket_clear_recv_buffer(int socket_fd);


int socket_close(int socket_fd);

#ifdef __cplusplus
}
#endif

#endif
