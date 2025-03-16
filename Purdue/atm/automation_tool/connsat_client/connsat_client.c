
/* gcc -g -o connsat_client connsat_client.c */

/* https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/ */

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <strings.h> // bzero()
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // inet_addr()
#include <unistd.h> // read(), write(), close()

#include <poll.h>

#include "connsat_main.h"
#include "connsat_options.h"
#include "connsat_termios.h"
#include "connsat_circular_buffer_algorithm.h"

// #define CONNSAT_CLIENT_TRACE

// #define BUFFER_MAX 80

#define CONNSAT_CLIENT_FDS_ARRAY_MALLOC_SIZE 10

#define CONNSAT_CLIENT_FDS_ARRAY_STDIN_CHANNEL_INDEX 0
#define CONNSAT_CLIENT_FDS_ARRAY_STDIN_CHANNEL_FD 0

#define CONNSAT_CLIENT_FDS_ARRAY_STDOUT_CHANNEL_INDEX 1
#define CONNSAT_CLIENT_FDS_ARRAY_STDOUT_CHANNEL_FD 1

#define CONNSAT_CLIENT_FDS_ARRAY_STREAM_CHANNEL_INDEX 2
#define CONNSAT_CLIENT_FDS_ARRAY_STREAM_CHANNEL_FD 2



// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_01 1
// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_05 1

// #define CONNSAT_CLIENT_CIRCULAR_BUFFER_DUMP_DEBUG_FLAG 1
// #define CONNSAT_CLIENT_CIRCULAR_BUFFER_VIEW_DEBUG_FLAG 1

// #define CONNSAT_CLIENT_MAX_LOOPS_FOR_DEBUG 1

// #define CONNSAT_CLIENT_LOOP_DEBUG_FLAG 1



#if 1
#define NEWLINE "\r\n"
#else
#define NEWLINE "\n"
#endif

int client_communication_connect(
	connsat_options_t *connsat_options_p,
	int *client_socket_connection_fd_p);

int client_communication_buffer_init(
	);

int client_communication_loop(
	int client_socket_connection_fd,
	int raw_mode_flag);

int client_buffer_view(
	circular_buffer_t *circular_buffer_p,
	int raw_mode_flag);

struct pollfd *poll_fds_array_p = NULL;
int poll_fds_array_malloc_size = 0;
int poll_fds_array_nfds = 0;
int poll_fds_array_index = -1;

circular_buffer_t *circular_buffer_01_p;	/* stdin [--> socket-out] */
circular_buffer_t *circular_buffer_02_p;	/* stdout */
circular_buffer_t *circular_buffer_03_p;	/* socket-in [--> stdout] */
circular_buffer_t *circular_buffer_04_p;	/* socket-out */

int
main (
	int argc,
	char **argv_p)
{
	int client_socket_connection_fd;
	struct sockaddr_in servaddr;
	int r;
	int r2;
	int r3;
	int r4;
	int raw_mode_flag;

	raw_mode_flag = 0;

	/* set up the default values */
	r = connsat_options_init(
		&connsat_options_all);

	/* get the user command line options */
	r = connsat_options_main(
		argc,
		argv_p,
		&connsat_options_all,
		raw_mode_flag);

	if (r != 0) {

		/* bad flags or usage/help was requested */

		exit(-1);
	}
#if 0
	if (argc != 0) {
		printf("argc==%d\n", argc);
	}
#endif

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

	/* connect to the network destination */
	r = client_communication_connect(
		&connsat_options_all,
		&client_socket_connection_fd);

	if (r != 0) {

		exit(-1);	/* error, can't connect */

	} else if (r == 0) {

		connsat_termios_init();
		raw_mode_flag = 1;

		/* initialize the buffers */
		r2 = client_communication_buffer_init();
		if (r2 != 0) {
			/* something is wrong? */
		}

		r3 = client_communication_loop(
			client_socket_connection_fd,
			raw_mode_flag);
		if (r3 != 0) {
			/* shuting down anyway */
		}

		r4 = connsat_termios_done();
		if (r4 != 0) {
			/* shuting down anyway */
		}
		raw_mode_flag = 0;

		// close the socket
		close(client_socket_connection_fd);
	}

	return 0;	/* ok */
}

int
client_communication_connect (
	connsat_options_t *connsat_options_p,
	int *client_socket_connection_fd_p)
{
	int client_socket_connection_fd;
	struct sockaddr_in servaddr;

	// socket create and verification
	client_socket_connection_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket_connection_fd == -1) {

		printf("Socket creation failed...\n");

		return -1; /* error */
	} else {

#if 0
		printf("Socket successfully created..\n");
#endif
	}

	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;

	servaddr.sin_addr.s_addr = inet_addr(connsat_options_p->network_ip_addr_string_p);
	servaddr.sin_port = htons(connsat_options_p->network_ip_port_uint16);

	// connect the client socket to server socket
	if (connect(client_socket_connection_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {

		printf("Error: Unable to connect to the server IP address: %s:%d\n",
			connsat_options_p->network_ip_addr_string_p,
			(int) connsat_options_p->network_ip_port_uint16);

		return -1; /* error */

	} else {

		printf("Connected to the server IP address: %s:%d\n",
			connsat_options_p->network_ip_addr_string_p,
			(int) connsat_options_p->network_ip_port_uint16);
	}

	if (client_socket_connection_fd_p != NULL) {
		*client_socket_connection_fd_p = client_socket_connection_fd;
	}

	return 0; /* ok */
}

int
client_communication_buffer_init ()
{
	int r;

	/* stdin */
	r = circular_buffer_init("Buffer-01", 2048, &circular_buffer_01_p);
	if (r != 0) {
		printf("Error creating Buffer-01"NEWLINE);
		exit(-1);
	}

	r = circular_buffer_calculations(circular_buffer_01_p);
	if (r != 0) {
		printf("Error calculating Buffer-01"NEWLINE);
		exit(-1);
	}

	/* stdout */
	r = circular_buffer_init("Buffer-02", 2048, &circular_buffer_02_p);
	if (r != 0) {
		printf("Error creating Buffer-02"NEWLINE);
		exit(-1);
	}

	r = circular_buffer_calculations(circular_buffer_02_p);
	if (r != 0) {
		printf("Error calculating Buffer-02"NEWLINE);
		exit(-1);
	}

	/* socket-in */
	r = circular_buffer_init("Buffer-03", 2048, &circular_buffer_03_p);
	if (r != 0) {
		printf("Error creating Buffer-03"NEWLINE);
		exit(-1);
	}

	r = circular_buffer_calculations(circular_buffer_03_p);
	if (r != 0) {
		printf("Error calculating Buffer-03"NEWLINE);
		exit(-1);
	}

	/* socket-out */
	r = circular_buffer_init("Buffer-04", 2048, &circular_buffer_04_p);
	if (r != 0) {
		printf("Error creating Buffer-04"NEWLINE);
		exit(-1);
	}

	r = circular_buffer_calculations(circular_buffer_04_p);
	if (r != 0) {
		printf("Error calculating Buffer-04"NEWLINE);
		exit(-1);
	}


	return 0; /* ok */
}

int
client_communication_loop (
	int client_socket_connection_fd,
	int raw_mode_flag)
{
	int poll_status;
	int poll_timeout;
	int input_loop_count;
	char *newline_string_p;
	int x;
	int something_to_poll_flag;
// #ifdef CONNSAT_CLIENT_MAX_LOOPS_FOR_DEBUG
	int max_loops_for_debug;	// @@@ remove code
// #endif
	int read_flags;
	int r;
	int max_buffer_transer;

	input_loop_count = 0;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}
 
#if 1
	printf("client_communication_loop(), %s-%d, client_socket_connection_fd==%d%s",
		__FILE__, __LINE__,
		client_socket_connection_fd,
		newline_string_p);
#endif

	/* build poll array */

	/* free old pollfd_array if allocated */
	if (poll_fds_array_p != NULL) { 
#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_01
		printf("\tFREE_POLLABLE_CONNECTIONS: %s-%d,%s",
			__FILE__, __LINE__,
			newline_string_p);
#endif

		free(poll_fds_array_p);

		poll_fds_array_p = NULL;
		poll_fds_array_nfds = 0;
		poll_fds_array_malloc_size = 0;
		poll_fds_array_index = -1;
	}

	/* malloc pollfd_array */
	poll_fds_array_malloc_size = sizeof (struct pollfd) * CONNSAT_CLIENT_FDS_ARRAY_MALLOC_SIZE;

	poll_fds_array_p = malloc( poll_fds_array_malloc_size );

#if 0
	printf("malloc(%d): %s-%d, units==%d, returned==%p%s",
		poll_fds_array_malloc_size,
		__FILE__, __LINE__,
		CONNSAT_CLIENT_FDS_ARRAY_MALLOC_SIZE,
		poll_fds_array_p,
		newline_string_p);
#endif

	if (poll_fds_array_p == NULL) {

		printf("%3d: POLL-MALLOC-ERROR%s",
			input_loop_count,
			newline_string_p);

		return -1;
	}

#ifdef CONNSAT_CLIENT_MAX_LOOPS_FOR_DEBUG
	max_loops_for_debug = 20;	// @@@ remove code
#else
	max_loops_for_debug = 1;
#endif

	/* poll loop */
	while ( max_loops_for_debug ) {	// @@@ remove code "max_loops_for_debug"

#ifdef CONNSAT_CLIENT_LOOP_DEBUG_FLAG		/* debug only delay, remove for normal use */
		printf("%3d: POLL-LOOP-START =========================================%s",
			input_loop_count,
			newline_string_p);
#endif

		CONNSAT_CLIENT_TRACE;

		/* clear allocated pollfd_array space */
		memset(poll_fds_array_p, 0, poll_fds_array_malloc_size);

		something_to_poll_flag = 0;


		poll_fds_array_nfds = 0;

		/* set up poll channel fd's */
		poll_fds_array_p[ CONNSAT_CLIENT_FDS_ARRAY_STDIN_CHANNEL_INDEX ].fd =
			CONNSAT_CLIENT_FDS_ARRAY_STDIN_CHANNEL_FD;
		++poll_fds_array_nfds;

		poll_fds_array_p[ CONNSAT_CLIENT_FDS_ARRAY_STDOUT_CHANNEL_INDEX ].fd =
			CONNSAT_CLIENT_FDS_ARRAY_STDOUT_CHANNEL_FD;
		++poll_fds_array_nfds;

		poll_fds_array_p[ CONNSAT_CLIENT_FDS_ARRAY_STREAM_CHANNEL_INDEX ].fd =
			client_socket_connection_fd;	// CONNSAT_CLIENT_FDS_ARRAY_STREAM_CHANNEL_FD
		++poll_fds_array_nfds;


		/* clear received events */

		/* stdin, input only */
		poll_fds_array_p[ CONNSAT_CLIENT_FDS_ARRAY_STDIN_CHANNEL_INDEX ].revents =
			0;

		/* stdout, output only */
		poll_fds_array_p[ CONNSAT_CLIENT_FDS_ARRAY_STDOUT_CHANNEL_INDEX ].revents =
			0;

		/* network stream, both input and output */
		poll_fds_array_p[ CONNSAT_CLIENT_FDS_ARRAY_STREAM_CHANNEL_INDEX ].revents =
			0;


		/* setup requested events */

		/* POLLIN, POLLOUT - read, write channels */
		poll_fds_array_p[ CONNSAT_CLIENT_FDS_ARRAY_STDIN_CHANNEL_INDEX ].events =
			POLLIN;
		++something_to_poll_flag;

#if 1
		/* write buffer has data to send */
		/* circular_buffer_02_p stdout */
		if (circular_buffer_02_p->calculated_buffer_max_readable_chunk >= 1) {
			poll_fds_array_p[ CONNSAT_CLIENT_FDS_ARRAY_STDOUT_CHANNEL_INDEX ].events =
				POLLOUT;

			++something_to_poll_flag;
		}
#endif

		poll_fds_array_p[ CONNSAT_CLIENT_FDS_ARRAY_STREAM_CHANNEL_INDEX ].events =
			POLLIN;
		++something_to_poll_flag;

#if 1
		/* write buffer has data to send */
		/* circular_buffer_04_p network-socket-out */
		if (circular_buffer_04_p->calculated_buffer_max_readable_chunk >= 1) {
			poll_fds_array_p[ CONNSAT_CLIENT_FDS_ARRAY_STREAM_CHANNEL_INDEX ].events |=
				POLLOUT;
			++something_to_poll_flag;
		}
#endif

		if (something_to_poll_flag) {

			CONNSAT_CLIENT_TRACE;

			poll_timeout = 250;

			poll_status = poll(poll_fds_array_p, poll_fds_array_nfds, poll_timeout);

#if 0
			printf("%3d: %s-%d: POLL-STATUS==%d%s",
				input_loop_count,
				__FILE__, __LINE__,
				poll_status,
				newline_string_p);
#endif

#ifdef CONNSAT_CLIENT_MAX_LOOPS_FOR_DEBUG
			if ( poll_status != 0 ) {
				--max_loops_for_debug;	// @@@ remove code
			}
#endif

			if ( poll_status < 0 ) {

				CONNSAT_CLIENT_TRACE;

#ifdef CONNSAT_CLIENT_LOOP_DEBUG_FLAG		/* debug only delay, remove for normal use */
				printf("%3d: POLL-LOOP-ERROR !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!%s",
					input_loop_count,
					newline_string_p);

				usleep(1000000);
#endif

				perror("POLL-ERROR: ");

				max_loops_for_debug = 0;	/* exit the communication loop */

			} else if ( poll_status == 0 ) {

				CONNSAT_CLIENT_TRACE;

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_01
				printf("%3d: %s-%d: POLL-TIMEOUT-RETURN%s",
					input_loop_count,
					__FILE__, __LINE__,
					newline_string_p);
#endif

				/* No pollable stream events across all open/active connections */

				/* Drop down below to process pending buffer to buffer transfers
				** for all buffers
				*/

			} else if ( poll_status > 0 ) {

				CONNSAT_CLIENT_TRACE;

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_05
				printf("%3d: %s-%d: POLL-HAS-EVENTS: poll_status_structure_array_event_count==%d%s",
					input_loop_count,
					__FILE__, __LINE__,
					poll_status,
					newline_string_p);
#endif

				for (x = 0; x < poll_fds_array_nfds; ++x) {

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_05
					if (1) { /* debug */

						printf("\t================================================================================================%s",
							newline_string_p);
						printf("\tfds_array[%d].revents==0x%08x, %s-%d%s",
							x, poll_fds_array_p[x].revents,
							__FILE__, __LINE__,
							newline_string_p);

						if (poll_fds_array_p[x].revents & POLLIN) {
							printf("\t\tfds_array[%d].revents POLLIN, %s-%d%s",
								x,
								__FILE__, __LINE__,
								newline_string_p);
						}
						if (poll_fds_array_p[x].revents & POLLOUT) {
							printf("\t\tfds_array[%d].revents POLLOUT, %s-%d%s",
								x,
								__FILE__, __LINE__,
								newline_string_p);
						}
						// if (poll_fds_array_p[x].revents & POLLRDHUP) {
						// 	printf("\t\t\tfds_array[%d].revents POLLRDHUP%s, %s-%d",
						//		x,
						//		__FILE__, __LINE__,
						//		newline_string_p);
						// }
						if (poll_fds_array_p[x].revents & POLLERR) {
							printf("\t\tfds_array[%d].revents POLLERR, %s-%d%s",
								x,
								__FILE__, __LINE__,
								newline_string_p);
						}
						if (poll_fds_array_p[x].revents & POLLHUP) {
							printf("\t\tfds_array[%d].revents POLLHUP, %s-%d%s",
								x,
								__FILE__, __LINE__,
								newline_string_p);
						}
						if (poll_fds_array_p[x].revents & POLLNVAL) {
							printf("\t\tfds_array[%d].revents POLLNVAL, %s-%d%s",
								x,
								__FILE__, __LINE__,
								newline_string_p);
						}
						printf("\t================================================================================================%s",
							newline_string_p);
					}
#endif

					CONNSAT_CLIENT_TRACE;

					if (poll_fds_array_p[x].revents & POLLHUP) {

						/* HANGUP: POLLHUP Detected, let's close the connection */

						CONNSAT_CLIENT_TRACE;

						printf("\t%s-%d: fds[%d]: fd==%d: POLL-POLLHUP-DETECTED-CONNECTION-CLOSED:",
							__FILE__, __LINE__,
							x,
							poll_fds_array_p[x].fd);

						printf("%s",
							newline_string_p);

						return -1;	/* POLLHUP detected, gracefully exit */

					}
				}




				CONNSAT_CLIENT_TRACE;

				if (1) {

					/* any POLLHUP event on STDIN? */
					if (poll_fds_array_p[CONNSAT_CLIENT_FDS_ARRAY_STDIN_CHANNEL_INDEX].events & POLLIN) {
						if (poll_fds_array_p[CONNSAT_CLIENT_FDS_ARRAY_STDIN_CHANNEL_INDEX].revents & POLLHUP) {
							return -1;	/* POLLHUP detected, gracefully exit */
						}
					}

					/* any POLLHUP event on NETWORK_SOCKET_STREAM_CHANNEL? */
					if (poll_fds_array_p[CONNSAT_CLIENT_FDS_ARRAY_STREAM_CHANNEL_INDEX].events & POLLIN) {
						if (poll_fds_array_p[CONNSAT_CLIENT_FDS_ARRAY_STREAM_CHANNEL_INDEX].revents & POLLHUP) {
							return -1;	/* POLLHUP detected, gracefully exit */
						}
					}




					/* did we request a POLLOUT event for stdout? */
					if (poll_fds_array_p[CONNSAT_CLIENT_FDS_ARRAY_STDOUT_CHANNEL_INDEX].events & POLLOUT) {

						/* did we receive a POLLOUT event for stdout? */
						if (poll_fds_array_p[CONNSAT_CLIENT_FDS_ARRAY_STDOUT_CHANNEL_INDEX].revents & POLLOUT) {

							/* write stdout from output buffer */
							r = circular_buffer_stream_write(
								circular_buffer_02_p,
								CONNSAT_CLIENT_FDS_ARRAY_STDOUT_CHANNEL_FD
								);

#if 0
							r = circular_buffer_dump(circular_buffer_02_p);
#endif
						}
					}

					/* did we request a POLLOUT event for network-socket? */
					if (poll_fds_array_p[CONNSAT_CLIENT_FDS_ARRAY_STREAM_CHANNEL_INDEX].events & POLLOUT) {

						/* did we receive a POLLOUT event for network-socket? */
						if (poll_fds_array_p[CONNSAT_CLIENT_FDS_ARRAY_STREAM_CHANNEL_INDEX].revents & POLLOUT) {

							/* write socket-out send buffer */
							r = circular_buffer_stream_write(
								circular_buffer_04_p,
								client_socket_connection_fd	// CONNSAT_CLIENT_FDS_ARRAY_STREAM_CHANNEL_FD
								);

#if 0
							r = circular_buffer_dump(circular_buffer_04_p);
#endif
						}
					}









					CONNSAT_CLIENT_TRACE;

					/* did we request a POLLIN event for stdin? */
					if (poll_fds_array_p[CONNSAT_CLIENT_FDS_ARRAY_STDIN_CHANNEL_INDEX].events & POLLIN) {

						CONNSAT_CLIENT_TRACE;

						/* did we receive a POLLIN event for stdin? */
						if (poll_fds_array_p[CONNSAT_CLIENT_FDS_ARRAY_STDIN_CHANNEL_INDEX].revents & POLLIN) {

							CONNSAT_CLIENT_TRACE;

							/* read stdin into stdin receive buffer */
							read_flags = 0;

							r = circular_buffer_stream_read(
								circular_buffer_01_p,
								CONNSAT_CLIENT_FDS_ARRAY_STDIN_CHANNEL_FD,
								&read_flags
								);

							if (read_flags & 0x0001) {      /* EOF */
								/* detected input EOF */
							}
#ifdef CONNSAT_CLIENT_CIRCULAR_BUFFER_DUMP_DEBUG_FLAG
							r = circular_buffer_dump(circular_buffer_01_p);
#endif
#ifdef CONNSAT_CLIENT_CIRCULAR_BUFFER_VIEW_DEBUG_FLAG
							r = client_buffer_view(
								circular_buffer_01_p,
								raw_mode_flag);
#endif
						}
					}

					CONNSAT_CLIENT_TRACE;

					/* did we request a POLLIN event for network-socket? */
					if (poll_fds_array_p[CONNSAT_CLIENT_FDS_ARRAY_STREAM_CHANNEL_INDEX].events & POLLIN) {

						CONNSAT_CLIENT_TRACE;

						/* did we receive a POLLIN event for network-socket? */
						if (poll_fds_array_p[CONNSAT_CLIENT_FDS_ARRAY_STREAM_CHANNEL_INDEX].revents & POLLIN) {

							CONNSAT_CLIENT_TRACE;

							/* read netowrk-socket-input into netowrk-socket receive buffer */
							read_flags = 0;

							/* read socket-in to receive buffer */
							r = circular_buffer_stream_read(
								circular_buffer_03_p,
								client_socket_connection_fd,	// CONNSAT_CLIENT_FDS_ARRAY_STREAM_CHANNEL_FD
								&read_flags
								);

							if (read_flags & 0x0001) {      /* EOF */

								/* detected input EOF */

								CONNSAT_CLIENT_TRACE;

								max_loops_for_debug = 0;	/* exit the communication loop */
							}
#ifdef CONNSAT_CLIENT_CIRCULAR_BUFFER_DUMP_DEBUG_FLAG
							r = circular_buffer_dump(circular_buffer_03_p);
#endif
#ifdef CONNSAT_CLIENT_CIRCULAR_BUFFER_VIEW_DEBUG_FLAG
							r = client_buffer_view(
								circular_buffer_03_p,
								raw_mode_flag);
#endif
						}
					}
				}




			}

			CONNSAT_CLIENT_TRACE;

		} else {

			CONNSAT_CLIENT_TRACE;

			/* nothing to poll */

			/* wait a bit, and try again */
			usleep(10000);
		}

		/* if the stdin receive buffer has data, send to network-socket-output-buffer */
		if (circular_buffer_01_p->calculated_buffer_max_readable_chunk >= 1) {

			if (circular_buffer_04_p->calculated_buffer_max_writeable_chunk >= 1) {

				int transfer_char;
				int transfer_count;

				CONNSAT_CLIENT_TRACE;

				max_buffer_transer = circular_buffer_01_p->calculated_buffer_max_readable_chunk;

				if (circular_buffer_04_p->calculated_buffer_max_writeable_chunk < max_buffer_transer) {

					max_buffer_transer = circular_buffer_04_p->calculated_buffer_max_writeable_chunk;

				}

				for (transfer_count = 0; transfer_count < max_buffer_transer; ++transfer_count) {

					r =  circular_buffer_getc(
						circular_buffer_01_p,
						&transfer_char
						);

					if (transfer_char == 0x1d) {		/* ^] sent */
						max_loops_for_debug = 0;	/* exit the communication loop */
					}

					r = circular_buffer_putc(
						circular_buffer_04_p,
						&transfer_char
						);

					r = circular_buffer_calculations(circular_buffer_01_p);

					if (r != 0) {
						printf("Error calculating Buffer-01"NEWLINE);
						exit(-1);
					}

					r = circular_buffer_calculations(circular_buffer_04_p);

					if (r != 0) {
						printf("Error calculating Buffer-04"NEWLINE);
						exit(-1);
					}
				}
			}
		}

		/* if the network-socket-receive-buffer has data, send to stdout-send-buffer */
		if (circular_buffer_03_p->calculated_buffer_max_readable_chunk >= 1) {

			if (circular_buffer_02_p->calculated_buffer_max_writeable_chunk >= 1) {

				int transfer_char;
				int transfer_count;

				CONNSAT_CLIENT_TRACE;

				max_buffer_transer = circular_buffer_03_p->calculated_buffer_max_readable_chunk;

				if (circular_buffer_02_p->calculated_buffer_max_writeable_chunk < max_buffer_transer) {

					max_buffer_transer = circular_buffer_02_p->calculated_buffer_max_writeable_chunk;

				}

				for (transfer_count = 0; transfer_count < max_buffer_transer; ++transfer_count) {

					r =  circular_buffer_getc(
						circular_buffer_03_p,
						&transfer_char
						);

					r = circular_buffer_putc(
						circular_buffer_02_p,
						&transfer_char
						);

					r = circular_buffer_calculations(circular_buffer_03_p);

					if (r != 0) {
						printf("Error calculating Buffer-03"NEWLINE);
						exit(-1);
					}

					r = circular_buffer_calculations(circular_buffer_02_p);

					if (r != 0) {
						printf("Error calculating Buffer-02"NEWLINE);
						exit(-1);
					}
				}

			}
		}

		CONNSAT_CLIENT_TRACE;

		++input_loop_count;

#ifdef CONNSAT_CLIENT_LOOP_DEBUG_FLAG		/* debug only delay, remove for normal use */
		// usleep(500000);
		usleep(2000000);
#endif
	}

	CONNSAT_CLIENT_TRACE;

	return 0; /* ok */
}

int
client_buffer_view (
	circular_buffer_t *circular_buffer_p,
	int raw_mode_flag)
{
	int r;
	int x;
	char *newline_string_p;
	char *view_buffer_p;
	int view_buffer_size;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	view_buffer_p = NULL;
	view_buffer_size = 0;

	r = circular_buffer_view(
		circular_buffer_p,
		&view_buffer_p,
		&view_buffer_size
		);

	if (r == 0) {

#ifdef MASTER_DEBUG_FLAG_01_CB_ALLOCATIONS
		printf("<%s>", circular_buffer_p->buffer_name);
		printf("{%d/", circular_buffer_p->circular_buffer_used_bytes);
		printf("%d}", circular_buffer_p->circular_buffer_allocated_size);
		printf("[%d]", circular_buffer_p->unwrapped_read_start_ptr);
		printf("r=%d:", circular_buffer_p->calculated_buffer_max_readable_chunk);
		printf("w=%d:", circular_buffer_p->calculated_buffer_wrapped_write_start_ptr);
		printf("f=%d:", circular_buffer_p->calculated_buffer_max_writeable_chunk);
#endif

		printf("\"");
		for (x = 0; x < view_buffer_size; ++x) {

			if (view_buffer_p[x] == 0) {
				printf("\\0");
			} else if (view_buffer_p[x] == 10) {
				printf("\\n");
			} else if (view_buffer_p[x] == 13) {
				printf("\\r");
			} else {
				printf("0x%0x, %d: %c", view_buffer_p[x], view_buffer_p[x], view_buffer_p[x]);
			}
		}
		printf("\"");

		free(view_buffer_p);
	}

	printf("%s",
		newline_string_p);

}

