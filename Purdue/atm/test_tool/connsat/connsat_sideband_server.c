
#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#include "connsat_sideband_server.h"

static int optvalue;

int
connsat_sideband_server_create (
	char *ip_address_string_p,
		// "127.0.0.1"
		// "0.0.0.0", same as INADDR_ANY
		// "INADDR_ANY"
	unsigned int ip_port,
	int max_socket_backlog,
	unsigned int flags,
		// flags = INADDR_ANY
	int *sideband_master_socket_fd_p)
{
	int sideband_master_socket_fd;
	int socket_flags;
	struct sockaddr_in addr;
	int sideband_server_status;
	int ret_value;

	// 0 == "ok", errno == "error"
	sideband_server_status = 0; /* ok */

	if (ip_address_string_p == NULL) {
		ip_address_string_p = "127.0.0.1";	/* let's default to a safe and sane value */
	}

	do {

		/* Create a new socket */
		sideband_master_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

		if (sideband_master_socket_fd < 0) {

			// errno is set */

			connsat_sideband_server_error("socket");

			// 0 == "ok", errno == "error"
			sideband_server_status = errno;
			if (sideband_master_socket_fd_p != NULL) {
				*sideband_master_socket_fd_p = -1;
			}

			break;
		}

		/* Set the new socket into Non-blocking mode */
// https://www.scottklement.com/rpg/socktut/nonblocking.html
// http://www.kegel.com/dkftpbench/nonblocking.html
		socket_flags = fcntl(sideband_master_socket_fd, F_GETFD, 0);

		ret_value = fcntl(sideband_master_socket_fd, F_SETFD, socket_flags | O_NONBLOCK);

		if (ret_value < 0) {

			// errno is set */

			connsat_sideband_server_error("fcntl");

			// 0 == "ok", errno == "error"
			sideband_server_status = errno;
			if (sideband_master_socket_fd_p != NULL) {
				*sideband_master_socket_fd_p = -1;
			}

			break;
		}

		optvalue = 1;

		/* Set the new socket into reusable server mode */
		ret_value = setsockopt(
			sideband_master_socket_fd,
			SOL_SOCKET,
			SO_REUSEADDR,
			&optvalue,
			sizeof(optvalue) );

		if (ret_value < 0) {

			// errno is set */
			connsat_sideband_server_error("setsockopt");

			// 0 == "ok", errno == "error"
			sideband_server_status = errno;
			if (sideband_master_socket_fd_p != NULL) {
				*sideband_master_socket_fd_p = -1;
			}

			break;
		}

		addr.sin_family = AF_INET;
		addr.sin_port = htons(ip_port);			// uint16_t
		if (strcmp(ip_address_string_p, "INADDR_ANY") == 0) {
			addr.sin_addr.s_addr = htonl(INADDR_ANY);	// uint32_t
		} else {
			addr.sin_addr.s_addr = inet_addr(ip_address_string_p);	// uint32_t
		}
		// ABOVE: char *ip_address_string_p,
		// /usr/include/netinet/in.h
		//	typedef uint32_t in_addr_t
		// INADDR_ANY;
		// inet_addr("127.0.0.1");
		// htonl(INADDR_ANY);

		/* Bind the new socket server to the IP address and PORT */
		if (bind(sideband_master_socket_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {

			// errno is set */

			connsat_sideband_server_error("bind");

			// 0 == "ok", errno == "error"
			sideband_server_status = errno;
			if (sideband_master_socket_fd_p != NULL) {
				*sideband_master_socket_fd_p = -1;
			}

			break;
		}

		/* Put the new socket server into the listening state */
		if (listen(sideband_master_socket_fd, max_socket_backlog) < 0) {

			// errno is set */

			connsat_sideband_server_error("listen");

			// 0 == "ok", errno == "error"
			sideband_server_status = errno;
			if (sideband_master_socket_fd_p != NULL) {
				*sideband_master_socket_fd_p = -1;
			}

			break;
		}

		if (sideband_master_socket_fd_p != NULL) {
			*sideband_master_socket_fd_p = sideband_master_socket_fd;
		}

	} while (0);

	// 0 == "ok", errno == "error"
	return sideband_server_status;
}

int
connsat_sideband_server_accept (
	int sideband_master_socket_fd,
	int *new_sideband_server_fd_p)
{

	int sideband_server_status;
	int new_sideband_server_fd;
	int socket_flags;

	sideband_server_status = 0; /* ok */

//@@@
	// if (pollfds[0].revents & POLLERR) {
	//	connsat_sideband_server_error("accept");
	//
	//	sideband_server_status = errno;
	//
	//	if (new_sideband_server_fd_p != NULL) {
	//		*new_sideband_server_fd_p = -1;
	//	}
	//
	// } else if (pollfds[0].revents & POLLIN) {
	// }

	do {

		/* Accept an incoming connection. */
		if ((new_sideband_server_fd = accept(sideband_master_socket_fd, NULL, NULL)) < 0) {

			// errno is set */

			connsat_sideband_server_error("accept");

			sideband_server_status = errno;

			if (new_sideband_server_fd_p != NULL) {
				*new_sideband_server_fd_p = -1;
			}

			break;
		}

		/* Set nonblocking mode. */
		socket_flags = fcntl(new_sideband_server_fd, F_GETFD, 0); 

		if (fcntl(new_sideband_server_fd, F_SETFD, socket_flags | O_NONBLOCK)) {

			// errno is set */

			connsat_sideband_server_error("fcntl");

			sideband_server_status = errno;

			if (new_sideband_server_fd_p != NULL) {
				*new_sideband_server_fd_p = -1;
			}

			break;
		}

	} while (0);

	if (new_sideband_server_fd_p != NULL) {
		*new_sideband_server_fd_p = new_sideband_server_fd;
	}

	return sideband_server_status;
}

int
connsat_sideband_server_error (
	char *s)
{
	perror(s);

	return 0;
}

