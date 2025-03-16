
#ifndef __CONNSAT_SIDEBAND_SERVER_H__
#define __CONNSAT_SIDEBAND_SERVER_H__ 1

int connsat_sideband_server_create(
	char *ip_address,
	unsigned int ip_port,
	int max_socket_backlog,
	unsigned int flags,
		// flags = INADDR_ANY
	int *sideband_server_fd_p);

int connsat_sideband_server_accept(
	int sideband_master_socket_fd,
	int *new_sideband_server_fd_p);

int connsat_sideband_server_error(
	char *s);

#endif /* __CONNSAT_SIDEBAND_SERVER_H__ */

