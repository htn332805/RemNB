
#include <stdio.h>

#include "connsat_connection_sideband_server.h"
#include "connsat_connection_sideband_channel.h"
#include "connsat_connections.h"
#include "connsat_sideband_server.h"
#include "connsat_connection_init.h"
#include "connsat_options.h"

int connsat_connection_sideband_server_buffer_distribution_worker(
	struct connsat_connection_s *connsat_connection_sideband_server_p,
	int distribution_mode_flag,
	int *calculation_mode_max_readable_chunk_p,
	int *calculation_mode_max_writeable_chunk_p,
	int raw_mode_flag);

int sideband_master_socket_fd = -1;

int
connsat_connection_sideband_server (
	struct connsat_connection_s **connsat_connection_sideband_server_pp,
	int menu_mode_flag,
	int raw_mode_flag
	)
{
	int r;
	char *newline_string_p;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	printf("Connsat-Initialize-Sideband-Server:%s", newline_string_p);

	r = connsat_connection_new(
		"SideBand-Server-Connnection",
		CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_SERVER, // CONNSAT_CONNECTION_TYPE_NONE,
		2048,					/* input buffer size */
		10240,					/* output buffer size */
		raw_mode_flag,

		connsat_connection_sideband_server_input_open,		/* connection_input_open_p */
		connsat_connection_sideband_server_input_close,		/* connection_input_close_p */

		connsat_connection_sideband_server_output_open,		/* connection_output_open_p */
		connsat_connection_sideband_server_output_close,	/* connection_output_close_p */

		connsat_connection_sideband_server_input_read,		/* connection_input_read_p */
		connsat_connection_sideband_server_output_write,	/* connection_output_write_p */

		connsat_connection_sideband_server_buffer_distribution, /* buffer_distribution_p */

		connsat_connection_sideband_server_pp			/* connsat_connection_new_p */
		);

	if (r < 0) {
		printf("FAILED: connsat_connection_sideband_server_p connsat_connection_new(): %d%s", r, newline_string_p);

		// exit(1);
		return -1; /* error */
	}

#ifdef CONNSAT_DEBUG_STARTUP
	printf("\tCREATED: connsat_connection_sideband_server_p: %p \"%s\"%s",
		*connsat_connection_sideband_server_pp,
		(*connsat_connection_sideband_server_pp)->name,
		newline_string_p);
#endif

#if 1
	r = connsat_connection_sideband_server_input_open(
		*connsat_connection_sideband_server_pp,
		menu_mode_flag,
		raw_mode_flag);

	if (r == 0) {

		r = connsat_connection_sideband_server_output_open(
			*connsat_connection_sideband_server_pp,
			menu_mode_flag,
			raw_mode_flag);
	}
#endif

	return 0; /* ok */
}

/* connection_input_open_p */
int
connsat_connection_sideband_server_input_open (
	struct connsat_connection_s *connsat_connection_sideband_server_p,
	int menu_mode_flag,
	int raw_mode_flag)
{
	char *ip_address_p;
	unsigned int ip_port;
	int max_socket_backlog;
	unsigned int flags;
	int r;
	char *newline_string_p;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	if (connsat_connection_sideband_server_p->connection_input_state != CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {

		if (sideband_master_socket_fd == -1) {

			ip_address_p = connsat_options_all.network_ip_addr_string_p;
				// "127.0.0.1"
				// "0.0.0.0", same as INADDR_ANY
				// "INADDR_ANY"

			ip_port = connsat_options_all.network_ip_port_uint16;

			max_socket_backlog = 5;

			flags = 0; /* not used yet */

			printf("\tSideband-Server: IP_ADDRESS: %s, IP_PORT: %d%s",
				ip_address_p,
				ip_port,
				newline_string_p);

			r = connsat_sideband_server_create(
				ip_address_p,
					// "127.0.0.1"
					// "0.0.0.0", same as INADDR_ANY
					// "INADDR_ANY"
				ip_port,
				max_socket_backlog,
				flags,
				&sideband_master_socket_fd);

			printf("\tSideband-Server: Returned: %d, SERVER_SOCKET_FD: %d%s",
				r,
				sideband_master_socket_fd,
				newline_string_p);

			if (r != 0) {
				return -1;
			}

		} else {

			/* The open_output must have been called first, let's
			** pick up the sideband_master_socket_fd.
			*/

			/* Drop through and mark the input stream connection as "open" */
		}

		connsat_connection_sideband_server_p->connection_input_fd = sideband_master_socket_fd;

		connsat_connection_sideband_server_p->connection_input_state = CONNSAT_CONNECTION_STATE_OPEN_ACTIVE;

		/* add the input_fd stream to the POLLIN list */
		connsat_connection_sideband_server_p->connection_input_fd_poll_flag = 1;

		/* enable the input buffer to receive characters */
		connsat_connection_sideband_server_p->connection_input_buffer_enable_flag = 1;

		/* enables receiving buffer distributions from other input buffers */
		connsat_connection_sideband_server_p->connection_input_buffer_distribution_flag = 1;

		connsat_connection_sideband_server_p->connection_input_fd = sideband_master_socket_fd;

	} else {

		return -2;
	}

	return 0; /* ok */
}

/* connection_input_close_p */
int
connsat_connection_sideband_server_input_close (
	struct connsat_connection_s *connsat_connection_sideband_server_p,
	int menu_mode_flag,
	int raw_mode_flag)
{
	char *newline_string_p;

	raw_mode_flag = 0;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	printf("MAIN_STDIN: Terminal Line Discipline Restored.%s",
		newline_string_p);
	fflush(stdout);

	connsat_connection_sideband_server_p->connection_input_state = CONNSAT_CONNECTION_STATE_CLOSED;

	connsat_connection_sideband_server_p->connection_input_fd = -1;

	connsat_connection_sideband_server_p->connection_input_fd_poll_flag = 0;
//@@@@@@@
//POLLIN

	connsat_connection_sideband_server_p->connection_input_buffer_enable_flag = 0;

	connsat_connection_sideband_server_p->connection_input_buffer_distribution_flag = 0;

	sideband_master_socket_fd = -1;

	return 0; /* ok */
}

/* connection_output_open_p */
int
connsat_connection_sideband_server_output_open (
	struct connsat_connection_s *connsat_connection_sideband_server_p,
	int menu_mode_flag,
	int raw_mode_flag)
{
	char *ip_address_p;
	unsigned int ip_port;
	int max_socket_backlog;
	unsigned int flags;
	int r;

	char *newline_string_p;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	if (connsat_connection_sideband_server_p->connection_output_state != CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {

		if (sideband_master_socket_fd == -1) {

			ip_address_p = connsat_options_all.network_ip_addr_string_p;
				// "127.0.0.1"
				// "0.0.0.0", same as INADDR_ANY
				// "INADDR_ANY"

			ip_port = connsat_options_all.network_ip_port_uint16;

			max_socket_backlog = 5;

			flags = 0; /* not used yet */

			printf("\tSideband-Server: IP_ADDRESS: %s, IP_PORT: %d%s",
				ip_address_p,
				ip_port,
				newline_string_p);

			r = connsat_sideband_server_create(
				ip_address_p,
					// "127.0.0.1"
					// "0.0.0.0", same as INADDR_ANY
					// "INADDR_ANY"
				ip_port,
				max_socket_backlog,
				flags,
				&sideband_master_socket_fd);

			printf("\tSideband-Server: Returned: %d, SERVER_SOCKET_FD: %d%s",
				r,
				sideband_master_socket_fd,
				newline_string_p);

			if (r != 0) {
				return -1;
			}
		} else {

			/* The open_input must have been called first, let's
			** pick up the sideband_master_socket_fd.
			*/

			/* Drop through and mark the output stream connection as "open" */
		}

		connsat_connection_sideband_server_p->connection_output_fd = sideband_master_socket_fd;

		connsat_connection_sideband_server_p->connection_output_state = CONNSAT_CONNECTION_STATE_OPEN_ACTIVE;

		/* add the output_fd stream to the POLLOUT list */
		connsat_connection_sideband_server_p->connection_output_fd_poll_flag = 1;

		/* enables receiving buffer distributions from other input buffers */
		connsat_connection_sideband_server_p->connection_output_buffer_enable_flag = 1;

		/* enables receiving buffer distributions from other output buffers */
		connsat_connection_sideband_server_p->connection_output_buffer_distribution_flag = 1;

		/* SELF_LOOPBACK */
		connsat_connection_sideband_server_p->connection_distribution_self_loopback_enable_flag = 1;

		connsat_connection_sideband_server_p->connection_output_fd = sideband_master_socket_fd;

	} else {

		return -2;
	}

	return 0; /* ok */
}

/* connection_output_close_p */
int
connsat_connection_sideband_server_output_close (
	struct connsat_connection_s *connsat_connection_sideband_server_p,
	int menu_mode_flag,
	int raw_mode_flag)
{
	connsat_connection_sideband_server_p->connection_output_state = CONNSAT_CONNECTION_STATE_CLOSED;

	connsat_connection_sideband_server_p->connection_output_fd = -1; /* stdout-fd */

	connsat_connection_sideband_server_p->connection_output_fd_poll_flag = 0;
//@@@@@@@
//POLLOUT

	connsat_connection_sideband_server_p->connection_output_buffer_enable_flag = 0;

	connsat_connection_sideband_server_p->connection_output_buffer_distribution_flag = 0;

	/* SELF_LOOPBACK */
	connsat_connection_sideband_server_p->connection_distribution_self_loopback_enable_flag = 0;

	sideband_master_socket_fd = -1;

	return 0; /* ok */
}

/* connection_input_read_p */
/* read terminal stdin into small distribution buffer */
/* Use POLLIN, to be sure read will not block */
int
connsat_connection_sideband_server_input_read (
	struct connsat_connection_s *connsat_connection_sideband_server_p,
	int menu_mode_flag,
	int raw_mode_flag)
{
	// int distribution_mode_flag;
	// int calculation_mode_max_readable_chunk;
	// int calculation_mode_max_writeable_chunk;
	// int small_input_buffer_max_writeable_chunk;
	// int max_input_size;
	char *newline_string_p;
	int r;
        int new_sideband_channel_fd;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	/* This connection is the sideband-server.
	** Note that this input function is only called
	** after a POLLIN event has been detected in a
	** non-blocking poll() loop.
	** The job here is to accept new network connections
	** and open a new connsat sideband-terminal connection
	** with the new socket_fd that is created.
	*/
	r = connsat_sideband_server_accept(
        	sideband_master_socket_fd,
        	&new_sideband_channel_fd);

	printf("%s\tSideband-Server-Connection-ACCEPTED: Returned: %d, SIDEBAND_CHANNEL_SOCKET_FD: %d%s",
		newline_string_p,
		r,
		new_sideband_channel_fd,
		newline_string_p);

	r = connsat_connection_sideband_channel_new_connection(
        	new_sideband_channel_fd,
		menu_mode_flag,
		raw_mode_flag);

	return 0; /* ok */
}

/* connection_output_write_p */
/* write terminal stdout from large output distribution buffer */
/* Use POLLOUT, to be sure write will not block */
int
connsat_connection_sideband_server_output_write (
	struct connsat_connection_s *connsat_connection_sideband_server_p,
	int menu_mode_flag,
	int raw_mode_flag)
{
	char *newline_string_p;
	int max_readable_chunk; /* source_read_buffer */
//	int max_writeable_chunk; /* destination_write_buffer */
	int r;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	if (connsat_connection_sideband_server_p->connection_type != CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_SERVER) {

		printf("Error: %s-%d, connection is not CONNECTION_TYPE_NETWORK_SIDEBAND_SERVER<%s", __FILE__, __LINE__,
			newline_string_p);

		return -1;
	}

	// circular_buffer_calculations(connsat_connection_sideband_server_p->connection_input_buffer_p);
	circular_buffer_calculations(connsat_connection_sideband_server_p->connection_output_buffer_p);

	/* read_the_output_buffer source_read_buffer */
	max_readable_chunk = connsat_connection_sideband_server_p->connection_input_buffer_p->calculated_buffer_max_readable_chunk;
	/* destination_write_buffer */
	// max_writeable_chunk = connsat_connection_distribution_list_p->connection_output_buffer_p->max_writeable_chunk;

	/* check POLLOUT to avoid blocking on write() */

#ifdef NETWORK_SIDEBAND_SERVER_STREAM_WRITE_BUFFER_DUMP
	printf("\t\tOUTPUT_BUFFER_MAX_READABLE_CHUNK: %s-%d, max_readable_chunk==%d%s",
		__FILE__, __LINE__,
		max_readable_chunk,
		newline_string_p);
#endif

	/* write output_fd from connsat_connection_distribution_list_p->connection_output_buffer_p */
	r = circular_buffer_stream_write(
		connsat_connection_sideband_server_p->connection_output_buffer_p,
		connsat_connection_sideband_server_p->connection_output_fd
		);

	return 0; /* ok */
}

int
connsat_connection_sideband_server_buffer_distribution (
	struct connsat_connection_s *connsat_connection_sideband_server_p,
		/* FROM CONNECTION */
		/* is a CONNSAT_CONNECTION_TYPE_TERMINAL */
	int distribution_mode_flag,
			/* BUFFER_DISTRIBUTION_MODE_FLAG_MENU_MODE */
			/* BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE */
			/* BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE */
				// find max-transfer-byte-count of all combined input and distribution buffers
					// check all buffers available
			/* BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE */
			/* BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE */

	int *calculation_mode_max_readable_chunk_p,
		/* for buffer transfer, what's the biggest chunk we can read from the source buffer */
	int *calculation_mode_max_writeable_chunk_p,
		/* for buffer transfer, what's the biggest chunk we can write to all destination buffers */

	int raw_mode_flag)
{

	char *newline_string_p;

	int buffer_distribution_max_readable_chunk; /* source_read_buffer */
	int buffer_distribution_max_writeable_chunk; /* destination_write_buffer */
	int from_count;
	int worker_distribution_mode_flag;
	int r;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	if (connsat_connection_sideband_server_p->connection_type != CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_SERVER) {

		printf("Error: %s-%d, connection is not CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_SERVER<%s", __FILE__, __LINE__,
			newline_string_p);

		return -1;
	}

	if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {

		worker_distribution_mode_flag = 0;

		if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_MENU_MODE) {
			worker_distribution_mode_flag |= BUFFER_DISTRIBUTION_MODE_FLAG_MENU_MODE;
		}

		worker_distribution_mode_flag |= BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE;

		if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) {

// #ifdef MASTER_DEBUG_FLAG_01_CALC
			if (connsat_options_all.trace_buffer_calculations_flag) {
				worker_distribution_mode_flag |= BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE;
			}
// #endif
		}

		buffer_distribution_max_readable_chunk = 0; /* source_read_buffer */
		buffer_distribution_max_writeable_chunk = 0; /* destination_write_buffer */

#ifdef MASTER_DEBUG_FLAG_07
printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ %s-%d%s", __FILE__, __LINE__, newline_string_p);
#endif

		/* first do the calculations */

		/* This sets buffer_distribution_max_readable_chunk and
		** buffer_distribution_max_writeable_chunk
		*/
		r = connsat_connection_sideband_server_buffer_distribution_worker (
			connsat_connection_sideband_server_p,
			worker_distribution_mode_flag,
/* SET */		&buffer_distribution_max_readable_chunk, /* calculated input, source_read_buffer */
/* SET */		&buffer_distribution_max_writeable_chunk, /* calculated output, destination_write_buffer */
			raw_mode_flag);

		if (r != 0) {

			printf("Error: %s-%d, buffer transfer calculation error for CONNECTION_TYPE_TERMINAL<%s",
				__FILE__, __LINE__,
				newline_string_p);

			return -2;
		}

#ifdef MASTER_DEBUG_FLAG_07
printf("%s-%d: max_readable_chunk==%d, max_writeable_chunk==%d%s",
__FILE__, __LINE__,
buffer_distribution_max_readable_chunk, buffer_distribution_max_writeable_chunk,
newline_string_p);

printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ %s-%d%s", __FILE__, __LINE__, newline_string_p);
#endif

		if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) {

// #ifdef MASTER_DEBUG_FLAG_01_CALC
			if (connsat_options_all.trace_buffer_calculations_flag) {

				printf("\t\t\t%s: %s-%d, CONNECTION_TYPE_TERMINAL-ALL_BUFFERS >>%d:%d<<%s",
					"TRANSFER_CALCULATIONS",
					__FILE__, __LINE__,
					buffer_distribution_max_readable_chunk,
					buffer_distribution_max_writeable_chunk,
					newline_string_p);
			}
// #endif
		}

		if (calculation_mode_max_readable_chunk_p != NULL) {

			/* calculation mode, save this */
/* SAVE */		*calculation_mode_max_readable_chunk_p = buffer_distribution_max_readable_chunk;

#ifdef MASTER_DEBUG_FLAG_01_SAVED
			printf("\t\t\t\t\t\tSAVED_DISTRIBUTION_CALCULATION_MODE_MAX_READABLE_CHUNK__DISTRIBUTION_INPUT: %s-%d, max_readable_chunk==%d%s",
				__FILE__, __LINE__,
				buffer_distribution_max_readable_chunk,
				newline_string_p);
#endif
		}

		if (calculation_mode_max_writeable_chunk_p != NULL) {

			/* calculation mode, save this */
/* SAVE */		*calculation_mode_max_writeable_chunk_p = buffer_distribution_max_writeable_chunk;

#ifdef MASTER_DEBUG_FLAG_01_SAVED
			printf("\t\t\t\t\t\tSAVED_DISTRIBUTION_CALCULATION_MODE_MAX_WRITEABLE_CHUNK__DISTRIBUTION_OUTPUT: %s-%d, max_writable_chunk==%d%s",
				__FILE__, __LINE__,
				buffer_distribution_max_writeable_chunk,
				newline_string_p);
#endif
		}

		if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE) {

			/* Is there something to transfer? */
			if (buffer_distribution_max_readable_chunk > 0 && buffer_distribution_max_writeable_chunk > 0) {

				worker_distribution_mode_flag = 0;

				if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_MENU_MODE) {
					worker_distribution_mode_flag |= BUFFER_DISTRIBUTION_MODE_FLAG_MENU_MODE;
				}

				worker_distribution_mode_flag |= BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE;

				if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) {
// #ifdef MASTER_DEBUG_FLAG_01_TRANS
					if (connsat_options_all.trace_buffer_transfer_flag) {
						worker_distribution_mode_flag |= BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE;
					}
// #endif
				}

#ifdef MASTER_DEBUG_FLAG_07
printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ %s-%d%s", __FILE__, __LINE__, newline_string_p);
printf("START USE ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ %s-%d%s", __FILE__, __LINE__, newline_string_p);
printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ %s-%d%s", __FILE__, __LINE__, newline_string_p);
printf("%s-%d: USE_NOW buffer_distribution_max_readable_chunk==%d, buffer_distribution_max_writeable_chunk==%d%s",
__FILE__, __LINE__,
buffer_distribution_max_readable_chunk, buffer_distribution_max_writeable_chunk,
newline_string_p);
#endif
				/* after the calculations we can do the multi-buffer transfer */
				r = connsat_connection_sideband_server_buffer_distribution_worker (
					connsat_connection_sideband_server_p,
					worker_distribution_mode_flag,
/* USE */				&buffer_distribution_max_readable_chunk, /* calculated input, source_read_buffer */
/* USE */				&buffer_distribution_max_writeable_chunk, /* calculated input, destination_write_buffer */
					raw_mode_flag);

				if (r != 0) {

					printf("Error: %s-%d, buffer transfer copy error for CONNECTION_TYPE_TERMINAL<%s",
						__FILE__, __LINE__,
						newline_string_p);

					return -3;
				}
#ifdef MASTER_DEBUG_FLAG_07
printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ %s-%d%s", __FILE__, __LINE__, newline_string_p);
printf("END USE ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ %s-%d%s", __FILE__, __LINE__, newline_string_p);
printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ %s-%d%s", __FILE__, __LINE__, newline_string_p);
#endif
			}
		}

	} else {

		if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE) {
			printf("Error: %s-%d, can't perform buffer transfer without perform calculations first for CONNECTION_TYPE_TERMINAL<%s",
				__FILE__, __LINE__,
				newline_string_p);

			return -4;
		}
	}

	return 0; /* ok */
}

int
connsat_connection_sideband_server_buffer_distribution_worker (
	struct connsat_connection_s *connsat_connection_sideband_server_p,
		/* FROM CONNECTION */
		/* is a CONNSAT_CONNECTION_TYPE_TERMINAL */
	int distribution_mode_flag,
			/* BUFFER_DISTRIBUTION_MODE_FLAG_MENU_MODE */
			/* BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE */
			/* BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE */
				// find max-transfer-byte-count of all combined input and distribution buffers
					// check all buffers available
			/* BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE */
			/* BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE */

	/* These are calculated in calculation_mode across all buffers
	** and are used in transfer mode.
	*/
	int *calculation_mode_max_readable_chunk_p,
		/* for buffer transfer, what's the biggest chunk we can read from the source buffer */
	int *calculation_mode_max_writeable_chunk_p,
		/* for buffer transfer, what's the biggest chunk we can write to the destination buffer */

	int raw_mode_flag)
{
	struct connsat_connection_s *connsat_connection_distribution_list_p;

	char *newline_string_p;

	int max_readable_chunk; /* source_read_buffer */
	int max_writeable_chunk; /* destination_write_buffer */
	int from_count;
	char *from_string_p;
	char *to_string_p;
	int r;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

#ifdef MASTER_DEBUG_FLAG_07
printf("%s-%d%s", __FILE__, __LINE__, newline_string_p);

printf("%s-%d, distribution_mode_flag==0x%04x%s",
__FILE__, __LINE__,
distribution_mode_flag,
newline_string_p);

if (calculation_mode_max_readable_chunk_p == NULL) {
printf("%s-%d, calculation_mode_max_readable_chunk_p==NULL_PTR!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!%s",
__FILE__, __LINE__,
newline_string_p);
} else {
printf("%s-%d, calculation_mode_max_readable_chunk_p==%d%s",
__FILE__, __LINE__,
*calculation_mode_max_readable_chunk_p,
newline_string_p);
}

if (calculation_mode_max_writeable_chunk_p == NULL) {
printf("%s-%d, calculation_mode_max_writeable_chunk_p==NULL_PTR!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!%s",
__FILE__, __LINE__,
newline_string_p);
} else {
printf("%s-%d, calculation_mode_max_writeable_chunk_p==%d%s",
__FILE__, __LINE__,
*calculation_mode_max_writeable_chunk_p,
newline_string_p);
}

printf("%s-%d%s", __FILE__, __LINE__, newline_string_p);
#endif

	/* NOTE: ONLY ONCE OF THESE CAN BE SELECTED; */
		/* BUFFER_DISTRIBUTION_CALCULATION_MODE */
		/* BUFFER_DISTRIBUTION_TRANSFER_MODE */
	if ( (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) &&
		(distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE )) {

		printf("Error: %s-%d, Function can only do calculation or transfer at one time%s", __FILE__, __LINE__, newline_string_p);

		return -1; /* error */
	}
	if ( (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) &&
		(distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE )) {

		printf("Error: %s-%d, Function can only print calculation or transfer at one time%s", __FILE__, __LINE__, newline_string_p);

		return -2; /* error */
	}

	if (calculation_mode_max_readable_chunk_p == NULL) {

		printf("Error: %s-%d, NULL pointer for max_readable_chunk%s", __FILE__, __LINE__, newline_string_p);

		return -3; /* error */
	}

	if (calculation_mode_max_writeable_chunk_p == NULL) {

		printf("Error: %s-%d, NULL pointer for max_writable_chunk%s", __FILE__, __LINE__, newline_string_p);

		return -4; /* error */
	}

	if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) {
		from_string_p = "From";
		to_string_p = "Forward";
	} else if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) {
		from_string_p = "Transfer";
		to_string_p = "Copy";
	}


	/* FORWARD CONNECTION */

	// printf("\tTRACE: %s-%d%s", __FILE__, __LINE__, newline_string_p);

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

	from_count = 0;

	if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_MENU_MODE) {

		connsat_connection_distribution_list_p = connsat_connection_list_head_p;

		while (connsat_connection_distribution_list_p) {

			/* MENU_MODE: */

			// printf("\tTRACE: %s-%d%s", __FILE__, __LINE__, newline_string_p);

			switch (connsat_connection_distribution_list_p->connection_type) {

				case CONNSAT_CONNECTION_TYPE_TERMINAL:

					/* input side of destination connection not is open */
					if (connsat_connection_distribution_list_p->connection_input_state != CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {
						break;
					}

					/* output side of destination connection not is open */
					if (connsat_connection_distribution_list_p->connection_output_state != CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {
						break;
					}

#if 0
					if (connsat_connection_distribution_list_p->connection_distribution_self_loopback_enable_flag == 0) {
						/* no self_loopback */
						break;
					}
#endif

					if (connsat_connection_distribution_list_p->connection_output_buffer_enable_flag == 0) {
						/* buffer is not enabled */
						break;
					}

					if (connsat_connection_distribution_list_p->connection_output_buffer_distribution_flag == 0) {
						/* distribution is not enabled */
						break;
					}

					/* source of copy buffer */
					circular_buffer_calculations(connsat_connection_sideband_server_p->connection_input_buffer_p);

					/* destination of copy buffer */
					circular_buffer_calculations(connsat_connection_distribution_list_p->connection_output_buffer_p);

					/* source_read_buffer */
					max_readable_chunk = connsat_connection_sideband_server_p->connection_input_buffer_p->calculated_buffer_max_readable_chunk;

					/* destination_write_buffer */
					max_writeable_chunk = connsat_connection_distribution_list_p->connection_output_buffer_p->calculated_buffer_max_writeable_chunk;

					/* forward to all CONNECTION_TYPE_TERMINAL */
						/* local-loopback mode */
					if (from_count++ == 0) {

						if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {
							*calculation_mode_max_readable_chunk_p = max_readable_chunk;
							*calculation_mode_max_writeable_chunk_p = max_writeable_chunk;
						}

						if ( (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) ||
							(distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) ) {

							printf("%s\t\t\t%s: %s-%d, CONNECTION_TYPE_NETWORK_SIDEBAND_SERVER-smallbuffer[%d/%d] >>%d<<%s",
								newline_string_p,
								from_string_p,
								__FILE__, __LINE__,
								connsat_connection_sideband_server_p->connection_input_buffer_p->circular_buffer_used_bytes,
								connsat_connection_sideband_server_p->connection_input_buffer_p->circular_buffer_allocated_size,
								max_readable_chunk,
								newline_string_p);
						}

					} else {

						if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {

#if 1 /* redundant code */
							/* source of distribution */
							if (*calculation_mode_max_readable_chunk_p >= max_readable_chunk) {
								*calculation_mode_max_readable_chunk_p = max_readable_chunk;
							}
#endif

							/* multi-destinations of distribution */
							if (*calculation_mode_max_writeable_chunk_p >= max_writeable_chunk) {
								*calculation_mode_max_writeable_chunk_p = max_writeable_chunk;
							}
						}
					}

					if ( (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) ||
						(distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) ) {

						printf("\t\t\t\t%s: %s-%d, CONNECTION_TYPE_TERMINAL-bigbuffer[%d/%d] <<<%d>>>%s",
							to_string_p,
							__FILE__, __LINE__,
							connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_used_bytes,
							connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_allocated_size,
							max_writeable_chunk,
							newline_string_p);
					}

					if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE) {
						int max_buffer_transfer = *calculation_mode_max_readable_chunk_p;
						if (max_buffer_transfer > *calculation_mode_max_writeable_chunk_p) {
							max_buffer_transfer = *calculation_mode_max_writeable_chunk_p;
						}

#ifdef MASTER_DEBUG_FLAG_01_MULTI_TRANSFER
						printf("%s-%d, MULTI_TRANSFER: NETWORK_SIDEBAND_SERVER->TERMINAL max_buffer_transfer==%d%s",
							__FILE__, __LINE__,
							max_buffer_transfer,
							newline_string_p);
#endif

						if (max_buffer_transfer > 0) {
							if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) {
								printf("\t\t\t\t\t%s: %s-%d, CONNECTION_TYPE_TERMINAL-bigbuffer[%d/%d] <<<%d>>> TRANSFER%s",
									to_string_p,
									__FILE__, __LINE__,
									connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_used_bytes,
									connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_allocated_size,
									max_buffer_transfer,
									newline_string_p);
							}

							/* do the buffer copy */
							r = circular_buffer_multi_block_transfer(
								connsat_connection_sideband_server_p->connection_input_buffer_p,
								connsat_connection_distribution_list_p->connection_output_buffer_p,
								max_buffer_transfer);

							if (r != 0) {

								printf("Error: %s-%d, circular_buffer_multi_block_transfer() returned %d%s",
									__FILE__, __LINE__,
									r,
									newline_string_p);

								return -5; /* error */
							}
						}
					}
					break;

				case CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_SERVER:

					/* SELF_LOOPBACK */

					break;

				case CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL:


					/* input side of destination connection not is open */
					if (connsat_connection_distribution_list_p->connection_input_state != CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {
						break;
					}

					/* output side of destination connection not is open */
					if (connsat_connection_distribution_list_p->connection_output_state != CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {
						break;
					}

					if (connsat_connection_distribution_list_p->connection_output_buffer_enable_flag == 0) {
						/* buffer is not enabled */
						break;
					}

					if (connsat_connection_distribution_list_p->connection_output_buffer_distribution_flag == 0) {
						/* distribution is not enabled */
						break;
					}

					/* source of copy buffer */
					circular_buffer_calculations(connsat_connection_sideband_server_p->connection_input_buffer_p);

					/* destination of copy buffer */
					circular_buffer_calculations(connsat_connection_distribution_list_p->connection_output_buffer_p);

					/* source_read_buffer */
					max_readable_chunk = connsat_connection_sideband_server_p->connection_input_buffer_p->calculated_buffer_max_readable_chunk;

					/* destination_write_buffer */
					max_writeable_chunk = connsat_connection_distribution_list_p->connection_output_buffer_p->calculated_buffer_max_writeable_chunk;

					/* forward to all CONNECTION_TYPE_SIDEBAND_CHANNEL */
					if (from_count++ == 0) {

						if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {
							*calculation_mode_max_readable_chunk_p = max_readable_chunk;
							*calculation_mode_max_writeable_chunk_p = max_writeable_chunk;
						}

						if ( (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) ||
							(distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) ) {

							printf("%s\t\t\t%s: %s-%d, CONNECTION_TYPE_NETWORK_SIDEBAND_SERVER-smallbuffer[%d/%d] >>%d<<%s",
								newline_string_p,
								from_string_p,
								__FILE__, __LINE__,
								connsat_connection_sideband_server_p->connection_input_buffer_p->circular_buffer_used_bytes,
								connsat_connection_sideband_server_p->connection_input_buffer_p->circular_buffer_allocated_size,
								max_readable_chunk,
								newline_string_p);
						}
					} else {

						if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {

							if (*calculation_mode_max_readable_chunk_p >= max_readable_chunk) {
								*calculation_mode_max_readable_chunk_p = max_readable_chunk;
							}

							if (*calculation_mode_max_writeable_chunk_p >= max_writeable_chunk) {
								*calculation_mode_max_writeable_chunk_p = max_writeable_chunk;
							}
						}
					}

					if ( (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) ||
						(distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) ) {

						printf("\t\t\t\t%s: %s-%d, CONNECTION_TYPE_SIDEBAND_CHANNEL-bigbuffer[%d/%d] <<<%d>>>%s",
							to_string_p,
							__FILE__, __LINE__,
							connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_used_bytes,
							connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_allocated_size,
							max_writeable_chunk,
							newline_string_p);
					}

					if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE) {
						int max_buffer_transfer = *calculation_mode_max_readable_chunk_p;
						if (max_buffer_transfer > *calculation_mode_max_writeable_chunk_p) {
							max_buffer_transfer = *calculation_mode_max_writeable_chunk_p;
						}

#ifdef MASTER_DEBUG_FLAG_01_MULTI_TRANSFER
						printf("%s-%d, MULTI_TRANSFER: NETWORK_SIDEBAND_SERVER->SIDEBAND_CHANNEL  max_buffer_transfer==%d%s",
							__FILE__, __LINE__,
							max_buffer_transfer,
							newline_string_p);
#endif

						if (max_buffer_transfer > 0) {

							if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) {

								printf("\t\t\t\t\t%s: %s-%d, CONNECTION_TYPE_SIDEBAND_CHANNEL-bigbuffer[%d/%d] <<<%d>>> TRANSFER%s",
									to_string_p,
									__FILE__, __LINE__,
									connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_used_bytes,
									connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_allocated_size,
									max_buffer_transfer,
									newline_string_p);
							}

							/* do the buffer copy */
							r = circular_buffer_multi_block_transfer(
								connsat_connection_sideband_server_p->connection_input_buffer_p,
								connsat_connection_distribution_list_p->connection_output_buffer_p,
								max_buffer_transfer);

							if (r != 0) {

								printf("Error: %s-%d, circular_buffer_multi_block_transfer() returned %d%s",
									__FILE__, __LINE__,
									r,
									newline_string_p);

								return -6; /* error */
							}
						}
					}
					break;

				case CONNSAT_CONNECTION_TYPE_FORKPTY:

					break;

				case CONNSAT_CONNECTION_TYPE_CONTROL_MENU:

					break;

				case CONNSAT_CONNECTION_TYPE_SESSION_LOG:

					break;

				case CONNSAT_CONNECTION_TYPE_CAPTURE_LOG:

					break;

				case CONNSAT_CONNECTION_TYPE_NONE:

					break;

				default:
					connsat_connection_distribution_list_p->connection_type = CONNSAT_CONNECTION_TYPE_NONE;
					break;
			}

			connsat_connection_distribution_list_p = connsat_connection_distribution_list_p->connsat_connection_next_p;
		}

		if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE) {

			int max_buffer_transfer = *calculation_mode_max_readable_chunk_p;
			if (max_buffer_transfer > *calculation_mode_max_writeable_chunk_p) {
				max_buffer_transfer = *calculation_mode_max_writeable_chunk_p;
			}

			if (max_buffer_transfer > 0) {

				/* complete the multi-buffer-transfer by consuming the input buffer */
				r = circular_buffer_multi_block_transfer_complete(
					connsat_connection_sideband_server_p->connection_input_buffer_p,
					max_buffer_transfer);

				if (r != 0) {

					printf("Error: %s-%d, circular_buffer_multi_block_transfer() returned %d%s",
						__FILE__, __LINE__,
						r,
						newline_string_p);

					return -11; /* error */
				}

				if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) {

					printf("%s\t\t\t\t\t%s: %s-%d, CONNECTION_TYPE_NETWORK_SIDEBAND_SERVER-smallbuffer[%d/%d] <<<%d>>> TRANSFER-COMPLETE%s",
						newline_string_p,
						to_string_p,
						__FILE__, __LINE__,
						connsat_connection_sideband_server_p->connection_input_buffer_p->circular_buffer_used_bytes,
						connsat_connection_sideband_server_p->connection_input_buffer_p->circular_buffer_allocated_size,
						max_buffer_transfer,
						newline_string_p);
				}
			}
		}

	} else {

		/* NON_MENU_MODE: */

		connsat_connection_distribution_list_p = connsat_connection_list_head_p;

		while (connsat_connection_distribution_list_p) {

			// printf("\tTRACE: %s-%d%s", __FILE__, __LINE__, newline_string_p);

			switch (connsat_connection_distribution_list_p->connection_type) {

				case CONNSAT_CONNECTION_TYPE_TERMINAL:

					/* input side of destination connection not is open */
					if (connsat_connection_distribution_list_p->connection_input_state != CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {
						break;
					}

					/* output side of destination connection not is open */
					if (connsat_connection_distribution_list_p->connection_output_state != CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {
						break;
					}

#if 0
					if (connsat_connection_distribution_list_p->connection_distribution_self_loopback_enable_flag == 0) {
						/* no self_loopback */
						break;
					}
#endif

					if (connsat_connection_distribution_list_p->connection_output_buffer_enable_flag == 0) {
						/* buffer is not enabled */
						break;
					}

					if (connsat_connection_distribution_list_p->connection_output_buffer_distribution_flag == 0) {
						/* distribution is not enabled */
						break;
					}

					/* source of copy buffer */
					circular_buffer_calculations(connsat_connection_sideband_server_p->connection_input_buffer_p);

					/* destination of copy buffer */
					circular_buffer_calculations(connsat_connection_distribution_list_p->connection_output_buffer_p);

					/* source_read_buffer */
					max_readable_chunk = connsat_connection_sideband_server_p->connection_input_buffer_p->calculated_buffer_max_readable_chunk;

					/* destination_write_buffer */
					max_writeable_chunk = connsat_connection_distribution_list_p->connection_output_buffer_p->calculated_buffer_max_writeable_chunk;

#ifdef MASTER_DEBUG_FLAG_01_MULTI_TRANSFER
					printf("%s-%d, MULTI_TRANSFER: NETWORK_SIDEBAND_SERVER->TERMINAL  *calculation_mode_max_readable_chunk_p==%d, *calculation_mode_max_writeable_chunk_p==%d, name==%s, dest==%s%s",
						__FILE__, __LINE__,
						*calculation_mode_max_readable_chunk_p,
						*calculation_mode_max_writeable_chunk_p,
						connsat_connection_terminal_p->name,
						connsat_connection_distribution_list_p->name,
						newline_string_p);
#endif
					/* forward to all CONNECTION_TYPE_TERMINAL */
						/* local-loopback mode */
					if (from_count++ == 0) {

						if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {
							*calculation_mode_max_readable_chunk_p = max_readable_chunk;
							*calculation_mode_max_writeable_chunk_p = max_writeable_chunk;
						}

#ifdef MASTER_DEBUG_FLAG_01_MULTI_TRANSFER
						printf("%s-%d, MULTI_TRANSFER: NETWORK_SIDEBAND_SERVER->TERMINAL  *calculation_mode_max_readable_chunk_p==%d, *calculation_mode_max_writeable_chunk_p==%d, name==%s, dest==%s%s",
							__FILE__, __LINE__,
							*calculation_mode_max_readable_chunk_p,
							*calculation_mode_max_writeable_chunk_p,
							connsat_connection_terminal_p->name,
							connsat_connection_distribution_list_p->name,
							newline_string_p);
#endif
						if ( (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) ||
							(distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) ) {

							printf("%s\t\t\t%s: %s-%d, CONNECTION_TYPE_TERMINAL-smallbuffer[%d/%d] >>%d<<%s",
								newline_string_p,
								from_string_p,
								__FILE__, __LINE__,
								connsat_connection_sideband_server_p->connection_input_buffer_p->circular_buffer_used_bytes,
								connsat_connection_sideband_server_p->connection_input_buffer_p->circular_buffer_allocated_size,
								max_readable_chunk,
								newline_string_p);
						}
					} else {

						if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {

#if 1 /* redundant code */
							/* source of distribution */
							if (*calculation_mode_max_readable_chunk_p >= max_readable_chunk) {
								*calculation_mode_max_readable_chunk_p = max_readable_chunk;
							}
#endif

							/* multi-destinations of distribution */
							if (*calculation_mode_max_writeable_chunk_p >= max_writeable_chunk) {
								*calculation_mode_max_writeable_chunk_p = max_writeable_chunk;
							}
						}
					}

					if ( (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) ||
						(distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) ) {

						printf("\t\t\t\t%s: %s-%d, CONNECTION_TYPE_TERMINAL-bigbuffer[%d/%d] <<<%d>>>%s",
							to_string_p,
							__FILE__, __LINE__,
							connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_used_bytes,
							connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_allocated_size,
							max_writeable_chunk,
							newline_string_p);
					}

#ifdef MASTER_DEBUG_FLAG_01_MULTI_TRANSFER
					printf("%s-%d, MULTI_TRANSFER: NETWORK_SIDEBAND_SERVER->TERMINAL  *calculation_mode_max_readable_chunk_p==%d, *calculation_mode_max_writeable_chunk_p==%d, name==%s, dest==%s%s",
						__FILE__, __LINE__,
						*calculation_mode_max_readable_chunk_p,
						*calculation_mode_max_writeable_chunk_p,
						connsat_connection_terminal_p->name,
						connsat_connection_distribution_list_p->name,
						newline_string_p);
#endif

					if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE) {
						int max_buffer_transfer = *calculation_mode_max_readable_chunk_p;
						if (max_buffer_transfer > *calculation_mode_max_writeable_chunk_p) {
							max_buffer_transfer = *calculation_mode_max_writeable_chunk_p;
						}

#ifdef MASTER_DEBUG_FLAG_01_MULTI_TRANSFER
						printf("%s-%d, MULTI_TRANSFER: NETWORK_SIDEBAND_SERVER->TERMINAL  max_buffer_transfer==%d, *calculation_mode_max_readable_chunk_p==%d, *calculation_mode_max_writeable_chunk_p==%d%s",
							__FILE__, __LINE__,
							max_buffer_transfer,
							*calculation_mode_max_readable_chunk_p,
							*calculation_mode_max_writeable_chunk_p,
							newline_string_p);
#endif

						if (max_buffer_transfer > 0) {
							if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) {
								printf("\t\t\t\t\t%s: %s-%d, CONNECTION_TYPE_TERMINAL-bigbuffer[%d/%d] <<<%d>>> TRANSFER%s",
									to_string_p,
									__FILE__, __LINE__,
									connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_used_bytes,
									connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_allocated_size,
									max_buffer_transfer,
									newline_string_p);
							}

							/* do the buffer copy */
							r = circular_buffer_multi_block_transfer(
								connsat_connection_sideband_server_p->connection_input_buffer_p,
								connsat_connection_distribution_list_p->connection_output_buffer_p,
								max_buffer_transfer);

							if (r != 0) {

								printf("Error: %s-%d, circular_buffer_multi_block_transfer() returned %d%s",
									__FILE__, __LINE__,
									r,
									newline_string_p);

								return -8; /* error */
							}
						}
					}
					break;

				case CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_SERVER:

					/* SELF_LOOPBACK */

					break;

				case CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL:

					/* input side of destination connection not is open */
					if (connsat_connection_distribution_list_p->connection_input_state != CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {
						break;
					}

					/* output side of destination connection not is open */
					if (connsat_connection_distribution_list_p->connection_output_state != CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {
						break;
					}

					if (connsat_connection_distribution_list_p->connection_output_buffer_enable_flag == 0) {
						/* buffer is not enabled */
						break;
					}

					if (connsat_connection_distribution_list_p->connection_output_buffer_distribution_flag == 0) {
						/* distribution is not enabled */
						break;
					}

					circular_buffer_calculations(connsat_connection_sideband_server_p->connection_input_buffer_p);

					circular_buffer_calculations(connsat_connection_distribution_list_p->connection_output_buffer_p);

					/* source_read_buffer */
					max_readable_chunk = connsat_connection_sideband_server_p->connection_input_buffer_p->calculated_buffer_max_readable_chunk;

					/* destination_write_buffer */
					max_writeable_chunk = connsat_connection_distribution_list_p->connection_output_buffer_p->calculated_buffer_max_writeable_chunk;

					/* forward to all CONNECTION_TYPE_SIDEBAND_CHANNEL */
					if (from_count++ == 0) {

						if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {
							*calculation_mode_max_readable_chunk_p = max_readable_chunk;
							*calculation_mode_max_writeable_chunk_p = max_writeable_chunk;
						}

						if ( (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) ||
							(distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) ) {

							printf("%s\t\t\t%s: %s-%d, CONNECTION_TYPE_NETWORK_SIDEBAND_SERVER-smallbuffer[%d/%d] >>%d<<%s",
								newline_string_p,
								from_string_p,
								__FILE__, __LINE__,
								connsat_connection_sideband_server_p->connection_input_buffer_p->circular_buffer_used_bytes,
								connsat_connection_sideband_server_p->connection_input_buffer_p->circular_buffer_allocated_size,
								max_readable_chunk,
								newline_string_p);
						}
					} else {

						if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {

							if (*calculation_mode_max_readable_chunk_p >= max_readable_chunk) {
								*calculation_mode_max_readable_chunk_p = max_readable_chunk;
							}

							if (*calculation_mode_max_writeable_chunk_p >= max_writeable_chunk) {
								*calculation_mode_max_writeable_chunk_p = max_writeable_chunk;
							}
						}
					}

					if ( (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) ||
						(distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) ) {

						printf("\t\t\t\t%s: %s-%d, CONNECTION_TYPE_SIDEBAND_CHANNEL-bigbuffer[%d/%d] <<<%d>>>%s",
							to_string_p,
							__FILE__, __LINE__,
							connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_used_bytes,
							connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_allocated_size,
							max_writeable_chunk,
							newline_string_p);
					}

					if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE) {
						int max_buffer_transfer = *calculation_mode_max_readable_chunk_p;
						if (max_buffer_transfer > *calculation_mode_max_writeable_chunk_p) {
							max_buffer_transfer = *calculation_mode_max_writeable_chunk_p;
						}

#ifdef MASTER_DEBUG_FLAG_01_MULTI_TRANSFER
						printf("%s-%d, MULTI_TRANSFER: NETWORK_SIDEBAND_SERVER->SIDEBAND_CHANNEL max_buffer_transfer==%d%s",
							__FILE__, __LINE__,
							max_buffer_transfer,
							newline_string_p);
#endif

						if (max_buffer_transfer > 0) {
							if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) {
								printf("\t\t\t\t\t%s: %s-%d, CONNECTION_TYPE_SIDEBAND_CHANNEL-bigbuffer[%d/%d] <<<%d>>> TRANSFER%s",
									to_string_p,
									__FILE__, __LINE__,
									connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_used_bytes,
									connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_allocated_size,
									max_buffer_transfer,
									newline_string_p);
							}

							/* do the buffer copy */
							r = circular_buffer_multi_block_transfer(
								connsat_connection_sideband_server_p->connection_input_buffer_p,
								connsat_connection_distribution_list_p->connection_output_buffer_p,
								max_buffer_transfer);

							if (r != 0) {

								printf("Error: %s-%d, circular_buffer_multi_block_transfer() returned %d%s",
									__FILE__, __LINE__,
									r,
									newline_string_p);

								return -9; /* error */
							}
						}
					}
					break;

				case CONNSAT_CONNECTION_TYPE_FORKPTY:

					break;

				case CONNSAT_CONNECTION_TYPE_CONTROL_MENU:

					break;

				case CONNSAT_CONNECTION_TYPE_SESSION_LOG:

					break;

				case CONNSAT_CONNECTION_TYPE_CAPTURE_LOG:

					break;

				case CONNSAT_CONNECTION_TYPE_NONE:

					break;

				default:
					connsat_connection_distribution_list_p->connection_type = CONNSAT_CONNECTION_TYPE_NONE;
					break;
			}

			// printf("\tTRACE: %s-%d%s", __FILE__, __LINE__, newline_string_p);

			connsat_connection_distribution_list_p = connsat_connection_distribution_list_p->connsat_connection_next_p;
		}

		if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE) {

			int max_buffer_transfer = *calculation_mode_max_readable_chunk_p;
			if (max_buffer_transfer > *calculation_mode_max_writeable_chunk_p) {
				max_buffer_transfer = *calculation_mode_max_writeable_chunk_p;
			}

			if (max_buffer_transfer > 0) {

				/* complete the multi-buffer-transfer by consuming the input buffer */
				r = circular_buffer_multi_block_transfer_complete(
					connsat_connection_sideband_server_p->connection_input_buffer_p,
					max_buffer_transfer);

				if (r != 0) {

					printf("Error: %s-%d, circular_buffer_multi_block_transfer() returned %d%s",
						__FILE__, __LINE__,
						r,
						newline_string_p);

					return -11; /* error */
				}

				if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) {

					printf("%s\t\t\t\t\t%s: %s-%d, CONNECTION_TYPE_NETWORK_SIDEBAND_SERVER-smallbuffer[%d/%d] <<<%d>>> TRANSFER-COMPLETE%s",
						newline_string_p,
						to_string_p,
						__FILE__, __LINE__,
						connsat_connection_sideband_server_p->connection_input_buffer_p->circular_buffer_used_bytes,
						connsat_connection_sideband_server_p->connection_input_buffer_p->circular_buffer_allocated_size,
						max_buffer_transfer,
						newline_string_p);
				}
			}
		}
	}

	return 0; /* ok */
}

