
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

// send()
#include <sys/types.h>
#include <sys/socket.h>

#if 0
#include <poll.h>
#endif

#include "connsat_main.h"

#include "connsat_connection_sideband_channel.h"
#include "connsat_connections.h"
#include "connsat_options.h"
#include "connsat_ascii.h"

#ifdef EXPERIMENTAL_CONTROL_MENU_UPDATES_001

#define SIDEBAND_TERMINAL_STREAM_READ_BUFFER_CRLF
// #define SIDEBAND_TERMINAL_STREAM_READ_BUFFER_CRLF_CONSOLIDATE

// #define TELNET_HACK_LINEMODE 1
// #define TELNET_HACK_CHARACTER_MODE 1
#endif

int connsat_connection_sideband_channel_buffer_distribution_worker(
	struct connsat_connection_s *connsat_connection_sideband_channel_p,
	int distribution_mode_flag,
	int *calculation_mode_max_readable_chunk_p,
	int *calculation_mode_max_writeable_chunk_p,
	int raw_mode_flag);

int
connsat_connection_sideband_channel (
	struct connsat_connection_s **connsat_connection_sideband_channel_pp,
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

	printf("Connsat-Initialize-Server-Channel:%s", newline_string_p);

	r = connsat_connection_new(
		"Server-Channel-Connnection",
		CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL, 	// CONNSAT_CONNECTION_TYPE_NONE,
		2048,					/* input buffer size */
		10240,					/* output buffer size */
		raw_mode_flag,

		connsat_connection_sideband_channel_input_open,		/* connection_input_open_p */
		connsat_connection_sideband_channel_input_close,	/* connection_input_close_p */

		connsat_connection_sideband_channel_output_open,	/* connection_output_open_p */
		connsat_connection_sideband_channel_output_close,	/* connection_output_close_p */

		connsat_connection_sideband_channel_input_read,		/* connection_input_read_p */
		connsat_connection_sideband_channel_output_write,	/* connection_output_write_p */

		connsat_connection_sideband_channel_buffer_distribution, /* buffer_distribution_p */

		connsat_connection_sideband_channel_pp			/* connsat_connection_new_p */
		);

	if (r < 0) {
		printf("FAILED: connsat_connection_sideband_channel_p connsat_connection_new(): %d%s", r, newline_string_p);

		// exit(1);
		return -1; /* error */
	}

#ifdef CONNSAT_DEBUG_STARTUP
	printf("\tCREATED: connsat_connection_sideband_channel_p: %p \"%s\"%s",
		*connsat_connection_sideband_channel_pp,
		(*connsat_connection_sideband_channel_pp)->name,
		newline_string_p);
#endif

#if 0
	r = connsat_connection_sideband_channel_input_open(
		*connsat_connection_sideband_channel_pp,
		menu_mode_flag,
		raw_mode_flag);

	r = connsat_connection_sideband_channel_output_open(
		*connsat_connection_sideband_channel_pp,
		menu_mode_flag,
		raw_mode_flag);
#endif

	return 0; /* ok */
}

int
connsat_connection_sideband_channel_new_connection (
	int new_sideband_channel_fd,
	int menu_mode_flag,
	int raw_mode_flag
	)
{
	int r;
	char *newline_string_p;
	struct connsat_connection_s *connsat_connection_list_p;
	int found_exiting_connection_flag;
	int send_ret;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	printf("Connsat-New-Server-Channel-Connection:%s", newline_string_p);

	CONNSAT_TRACE;

	/* Look through the existing connection list, and
	** see if we can find a closed sideband_channel.
	*/
	connsat_connection_list_p = connsat_connection_list_head_p;

	found_exiting_connection_flag = 0;

	while (connsat_connection_list_p) {

		CONNSAT_TRACE;

		switch (connsat_connection_list_p->connection_type) {

			case CONNSAT_CONNECTION_TYPE_TERMINAL:
				CONNSAT_TRACE;
				break;

			case CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_SERVER:
				CONNSAT_TRACE;
				break;

			case CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL:
				CONNSAT_TRACE;

				/* is it fully closed? */

				if (connsat_connection_list_p->connection_input_state == CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {
					/* in-use, skip */
					CONNSAT_TRACE;
					break;
				}
				if (connsat_connection_list_p->connection_output_state == CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {
					/* in-use, skip */
					CONNSAT_TRACE;
					break;
				}
				CONNSAT_TRACE;

				/* ok, we found an exiting connection that is not being used, we'll take it */

				printf("\tConnsat-Sideband-Channel-Connection: Use existing connection%s", newline_string_p);
#if 1
				r = connsat_connection_sideband_channel_input_open(
					connsat_connection_list_p,
					menu_mode_flag,
					raw_mode_flag);

				if (r == 0) {
					CONNSAT_TRACE;
					connsat_connection_list_p->connection_input_fd = new_sideband_channel_fd;

					CONNSAT_TRACE;
					r = connsat_connection_sideband_channel_output_open(
						connsat_connection_list_p,
						menu_mode_flag,
						raw_mode_flag);

					CONNSAT_TRACE;
					if (r == 0) {
						CONNSAT_TRACE;
						connsat_connection_list_p->connection_output_fd = new_sideband_channel_fd;

						found_exiting_connection_flag = 1;

						/* notify the poll logic to rebuild the poll list */
						connsat_connection_global_change_detected_flag = 1;
					}
					CONNSAT_TRACE;
				}

#ifdef TELNET_HACK_LINEMODE
				// Put the telnet client into LINEMODE:
				// IAC DO LINEMODE IAC WILL ECHO
				// write(new_socket, "\377\375\042\377\373\001", 6);

				//      Operating in single character mode
				//      Catching signals locally
				//      Remote character echo

				send_ret = send(new_sideband_channel_fd, "\377\375\042\377\373\001", 6, 0); 
				if (send_ret != 6) {
					perror("send");
				}
#endif

#ifdef TELNET_HACK_CHARACTER_MODE
				// telnet client enable character mode, 
				// which the server can do by sending 
				// IAC WILL GA IAC WILL ECHO
				// write(new_socket, "\377\373\371\377\373\001", 6);
				send_ret = send(new_sideband_channel_fd, "\377\373\371\377\373\001", 6, 0); 
				if (send_ret != 6) {
					perror("send");
				}
#endif

#endif
				CONNSAT_TRACE;

				break;

			case CONNSAT_CONNECTION_TYPE_FORKPTY:
				CONNSAT_TRACE;
				break;

			case CONNSAT_CONNECTION_TYPE_CONTROL_MENU:
				CONNSAT_TRACE;
				break;

			case CONNSAT_CONNECTION_TYPE_SESSION_LOG:
				CONNSAT_TRACE;
				break;

			case CONNSAT_CONNECTION_TYPE_CAPTURE_LOG:
				CONNSAT_TRACE;
				break;

			case CONNSAT_CONNECTION_TYPE_NONE:
				CONNSAT_TRACE;
				break;
			default:
				CONNSAT_TRACE;
				break;
		}

		CONNSAT_TRACE;
		if (found_exiting_connection_flag != 0) {
			CONNSAT_TRACE;
			return 0; /* ok */
		}

		connsat_connection_list_p = connsat_connection_list_p->connsat_connection_next_p;
	}

	CONNSAT_TRACE;

	if (found_exiting_connection_flag == 0) {

		struct connsat_connection_s *connsat_connection_sideband_channel_p;

		printf("\tConnsat-Sideband-Channel-Connection: Create new sideband-channel connection%s", newline_string_p);

		CONNSAT_TRACE;
		/* We went through the exiting list and no closed
		** sideband channel connection is available.
		** Let's create a new one.
		*/
		r = connsat_connection_sideband_channel (
			&connsat_connection_sideband_channel_p,
			menu_mode_flag,
			raw_mode_flag
			);

		if (r == 0) {
			CONNSAT_TRACE;

			/* ok, we created a new connection, let's use that now */
#if 1
			r = connsat_connection_sideband_channel_input_open(
				connsat_connection_sideband_channel_p,
				menu_mode_flag,
				raw_mode_flag);

			if (r == 0) {
				CONNSAT_TRACE;

				connsat_connection_sideband_channel_p->connection_input_fd = new_sideband_channel_fd;

				r = connsat_connection_sideband_channel_output_open(
					connsat_connection_sideband_channel_p,
					menu_mode_flag,
					raw_mode_flag);

				if (r == 0) {
					CONNSAT_TRACE;
					connsat_connection_sideband_channel_p->connection_output_fd = new_sideband_channel_fd;

					found_exiting_connection_flag = 1;

					/* notify the poll logic to rebuild the poll list */
					connsat_connection_global_change_detected_flag = 1;
				}
			}

#endif
		}
		CONNSAT_TRACE;

	}
	CONNSAT_TRACE;

	return 0; /* ok */
}

/* connection_input_open_p */
int
connsat_connection_sideband_channel_input_open (
	struct connsat_connection_s *connsat_connection_sideband_channel_p,
	int menu_mode_flag,
	int raw_mode_flag)
{
	char *newline_string_p;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	CONNSAT_TRACE;

// get sideband accept socket
	connsat_connection_sideband_channel_p->connection_input_fd = -1;

	connsat_connection_sideband_channel_p->connection_input_state = CONNSAT_CONNECTION_STATE_OPEN_ACTIVE;

	/* add the input_fd stream to the POLLIN list */
	connsat_connection_sideband_channel_p->connection_input_fd_poll_flag = 1;
//@@@@@@@
//POLLIN

	/* enable the input buffer to receive characters */
	connsat_connection_sideband_channel_p->connection_input_buffer_enable_flag = 1;

	/* enables receiving buffer distributions from other input buffers */
	connsat_connection_sideband_channel_p->connection_input_buffer_distribution_flag = 1;

	return 0; /* ok */
}

/* connection_input_close_p */
int
connsat_connection_sideband_channel_input_close (
	struct connsat_connection_s *connsat_connection_sideband_channel_p,
	int menu_mode_flag,
	int raw_mode_flag)
{
	char *newline_string_p;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	CONNSAT_TRACE;

	connsat_connection_sideband_channel_p->connection_input_state = CONNSAT_CONNECTION_STATE_CLOSED;

	connsat_connection_sideband_channel_p->connection_input_fd = -1;	/* stdin-fd */

	connsat_connection_sideband_channel_p->connection_input_fd_poll_flag = 0;
//@@@@@@@
//POLLIN

	connsat_connection_sideband_channel_p->connection_input_buffer_enable_flag = 0;

	connsat_connection_sideband_channel_p->connection_input_buffer_distribution_flag = 0;

	return 0; /* ok */
}

/* connection_output_open_p */
int
connsat_connection_sideband_channel_output_open (
	struct connsat_connection_s *connsat_connection_sideband_channel_p,
	int menu_mode_flag,
	int raw_mode_flag)
{
	char *newline_string_p;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	CONNSAT_TRACE;

// get sideband accept socket
	connsat_connection_sideband_channel_p->connection_output_fd = -1;

	connsat_connection_sideband_channel_p->connection_output_state = CONNSAT_CONNECTION_STATE_OPEN_ACTIVE;

	/* add the output_fd stream to the POLLOUT list */
	connsat_connection_sideband_channel_p->connection_output_fd_poll_flag = 1;
//@@@@@@@
//POLLOUT

	/* enable the output buffer to receive characters */
	connsat_connection_sideband_channel_p->connection_output_buffer_enable_flag = 1;

	/* enables receiving buffer distributions from other input buffers */
	connsat_connection_sideband_channel_p->connection_output_buffer_distribution_flag = 1;

#ifdef EXPERIMENTAL_CONTROL_MENU_UPDATES_001
	/* SELF_LOOPBACK */
	connsat_connection_sideband_channel_p->connection_distribution_self_loopback_enable_flag = 0;
#else
	/* SELF_LOOPBACK */
	connsat_connection_sideband_channel_p->connection_distribution_self_loopback_enable_flag = 1;
#endif

	return 0; /* ok */
}

/* connection_output_close_p */
int
connsat_connection_sideband_channel_output_close (
	struct connsat_connection_s *connsat_connection_sideband_channel_p,
	int menu_mode_flag,
	int raw_mode_flag)
{
	char *newline_string_p;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	CONNSAT_TRACE;

	connsat_connection_sideband_channel_p->connection_output_state = CONNSAT_CONNECTION_STATE_CLOSED;

	connsat_connection_sideband_channel_p->connection_output_fd = -1; /* stdout-fd */

	connsat_connection_sideband_channel_p->connection_output_fd_poll_flag = 0;
//@@@@@@@
//POLLOUT

	connsat_connection_sideband_channel_p->connection_output_buffer_enable_flag = 0;

	connsat_connection_sideband_channel_p->connection_output_buffer_distribution_flag = 0;

	/* SELF_LOOPBACK */
	connsat_connection_sideband_channel_p->connection_distribution_self_loopback_enable_flag = 0;

	return 0; /* ok */
}

/* connection_input_read_p */
/* read terminal stdin into small distribution buffer */
/* Use POLLIN, to be sure read will not block */
/* This is not to be called without using POLLIN if that is enabled */
int
connsat_connection_sideband_channel_input_read (
	struct connsat_connection_s *connsat_connection_sideband_channel_p,
	int menu_mode_flag,
	int raw_mode_flag)
{
	int distribution_mode_flag;
	int calculation_mode_max_readable_chunk;
	int calculation_mode_max_writeable_chunk;
	int small_input_buffer_max_writeable_chunk;
	int max_input_size;
	char *newline_string_p;
	int r;
	struct pollfd pollfd_array[1];
	int poll_timeout;
	int retry_count;
	int max_retries;
	char input_buffer[16];
	int i;
	static int last_char = 0;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	CONNSAT_TRACE;

	/* check the small input buffer buffer_distribution_max_writeable_chunk size */
	circular_buffer_calculations(connsat_connection_sideband_channel_p->connection_input_buffer_p);

	small_input_buffer_max_writeable_chunk = connsat_connection_sideband_channel_p->connection_input_buffer_p->calculated_buffer_max_writeable_chunk;

	max_input_size = small_input_buffer_max_writeable_chunk;

	/* override menu_mode_flag */
	r = connsat_connection_global_menu_mode_flag_state_get(&menu_mode_flag, raw_mode_flag);

	/*
	** Check that all distribution list large
	** output buffers have room before we read
	** anything.
	*/
	distribution_mode_flag = BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE;
	if (menu_mode_flag) {
		distribution_mode_flag |= BUFFER_DISTRIBUTION_MODE_FLAG_MENU_MODE;
	}

//@@@@@@@ DEBUG
// #ifdef MASTER_DEBUG_FLAG_01_CALC
	if (connsat_options_all.trace_buffer_calculations_flag) {
		distribution_mode_flag |= BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE;
	}
// #endif

	calculation_mode_max_readable_chunk = 0;
	calculation_mode_max_writeable_chunk = 0;

	/* determine the lowest max common distribution output buffer writable chunk */
	r = connsat_connection_sideband_channel_buffer_distribution_worker(
		connsat_connection_sideband_channel_p,
		distribution_mode_flag,
		&calculation_mode_max_readable_chunk,
		&calculation_mode_max_writeable_chunk,	/* this is the main thing we need here */
		raw_mode_flag
		);

	/* determine the smallest common size */
	if (max_input_size > calculation_mode_max_writeable_chunk) {
		max_input_size = calculation_mode_max_writeable_chunk;
	}

	/* buffers can read up to max_input_size from input */
//@@@@@@@

	CONNSAT_TRACE;
#if 1
#ifdef MASTER_DEBUG_FLAG_01
	printf("TERMINAL_READ: %s-%d, READ up to max_input_size==%d%s",
		__FILE__, __LINE__,
		max_input_size,
		newline_string_p);
#endif
#endif

#if 0 /* temp poll */
	struct pollfd pollfd_array[1];
	int poll_timeout = 0;
	nfds_t poll_nfds = 1;

	pollfd_array[0].fd = 0;	/* stdin fd */
	pollfd_array[0].events = POLLIN;	/* POLLIN */
	pollfd_array[0].revents = 0;

	r =  poll(&pollfd_array[0], poll_nfds, poll_timeout);

	if (pollfd_array[0].revents & POLLIN) {
		/* there is data to read */
	}
#endif

#if 1
	max_retries = 256;
	poll_timeout = 0;	/* no wait, non-blocking */
	nfds_t poll_nfds = 1;

	// pollfd_array[0].fd = 0;	/* main_terminal_stdin_fd */
	pollfd_array[0].fd = connsat_connection_sideband_channel_p->connection_input_fd; /* sideband-channel fd */
	pollfd_array[0].events = POLLIN;	/* POLLIN */
	pollfd_array[0].revents = 0;

	CONNSAT_TRACE;

	for (retry_count = 0; retry_count < max_retries; ++retry_count) {

		/* check POLLIN to avoid blocking on read() */

		CONNSAT_TRACE;

		r =  poll(&pollfd_array[0], poll_nfds, poll_timeout);

// @@@@@@@
// check pollfd_array[0].revents for HUP?

		if (pollfd_array[0].revents & POLLIN) {
			/* there is data to read */

			CONNSAT_TRACE;

			r = circular_buffer_calculations(connsat_connection_sideband_channel_p->connection_input_buffer_p);

			if (connsat_connection_sideband_channel_p->connection_input_buffer_p->calculated_buffer_max_writeable_chunk >= 1) {

				CONNSAT_TRACE;
#if 1
				// read fd into buffer */
				r = read(connsat_connection_sideband_channel_p->connection_input_fd,
					input_buffer,
					1);

#else
				// read fd into buffer */
				r = read(connsat_connection_sideband_channel_p->connection_input_fd,
					&connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_data_p[
						connsat_connection_sideband_channel_p->connection_input_buffer_p->wrapped_write_start_ptr],
					// connsat_connection_sideband_channel_p->connection_input_buffer_p->buffer_distribution_max_writeable_chunk);
					1);
#endif

				if (r == -1) {
					// errno
#ifdef SIDEBAND_TERMINAL_STREAM_READ_BUFFER_DUMP
					printf("\tREAD-ERROR: %d%s",
						errno,
						newline_string_p);

					perror("READ-ERROR: ");
#endif
					return -1;
				} else if (r == 0) {

					// EOF
					// *flags_p = 0x0001;	/* EOF */

// @@@@@@@
// check pollfd_array[0].revents for HUP?

// @@@
#ifdef SIDEBAND_TERMINAL_STREAM_READ_BUFFER_DUMP
					printf("\tREAD-EOF%s",
						newline_string_p);
#endif

				} else {
// @@@
					CONNSAT_TRACE;

#ifdef MASTER_DEBUG_FLAG_07
printf("READ-BYTES-%s-%d: %d%s",
__FILE__, __LINE__,
r,
newline_string_p);
#endif

					/* LOOK FOR ^D character by character */
					if (input_buffer[0] == 0x04) {

// @@@@@@@
// flush all buffer distribution first
// freeze buffers if something is stuck

#ifdef SIDEBAND_TERMINAL_STREAM_READ_BUFFER_DUMP
						printf("\tREAD-CNTRL-D, SWITCH_TO_MENU_MODE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!%s",
							newline_string_p);
#endif

						/* switch everything to menu mode now */
						r = connsat_connection_global_menu_mode_flag_state_set(
							1,
							raw_mode_flag);

						r = connsat_connection_global_menu_mode_flag_state_get(
							&menu_mode_flag,
							raw_mode_flag);
// @@@@@@@
						/*
						** We just switched to menu_mode now,
						** Stop processing normal input.
						** Return to global polling and distribution mode
						** so menu keystrokes will distribute through
						** the global menu subsystem.
						*/
#ifdef SIDEBAND_TERMINAL_STREAM_READ_BUFFER_DUMP
						printf("\tREAD-CNTRL-D, SWITCHED_TO_MENU_MODE_STOP_PROCESSING_INPUT_STDIN!!!!!!!!!!!!!%s",
							newline_string_p);
#endif

						break;

					} else {

#if 0
#ifdef MASTER_DEBUG_FLAG_01_LOCAL_ECHO
#ifdef SIDEBAND_TERMINAL_STREAM_READ_BUFFER_DUMP
// @@@@@@@
// DEBUG_ECHO_CHARS
						printf("\tREAD-BYTES-LOCAL-DEBUG-ECHO: %d, \"", r);
						for (i = 0; i < r; ++i) {

							if (
								connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_data_p[
								connsat_connection_sideband_channel_p->connection_input_buffer_p->calculated_buffer_wrapped_write_start_ptr + i] == 0) {

								putc('\\', stdout);
								putc('0', stdout);

							} else if (
								connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_data_p[
								connsat_connection_sideband_channel_p->connection_input_buffer_p->calculated_buffer_wrapped_write_start_ptr + i] == 10) {

								putc('\\', stdout);
								putc('n', stdout);

							} else if (
								connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_data_p[
								connsat_connection_sideband_channel_p->connection_input_buffer_p->calculated_buffer_wrapped_write_start_ptr + i] == 13) {

								putc('\\', stdout);
								putc('r', stdout);

							} else {
								putc(
									connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_data_p[
									connsat_connection_sideband_channel_p->connection_input_buffer_p->calculated_buffer_wrapped_write_start_ptr + i],
								stdout);
							}
						}
						printf("\"%s",
							newline_string_p);
#endif
#endif
#endif
						CONNSAT_TRACE;

#ifndef SIDEBAND_TERMINAL_STREAM_READ_BUFFER_CRLF_CONSOLIDATE
						last_char = 0;	/* don't consolidate <CR><LF> or <LF><CR> */
#endif

						if (input_buffer[0] == 0x0a) {		/* LF line feed */

							if (last_char != 0x0d) {
								connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_data_p[
									connsat_connection_sideband_channel_p->connection_input_buffer_p->calculated_buffer_wrapped_write_start_ptr] = input_buffer[0];

								/* recalc buffer */
								r = circular_buffer_calculations(connsat_connection_sideband_channel_p->connection_input_buffer_p);
									/* this recalculates: connsat_connection_sideband_channel_p->connection_input_buffer_p->wrapped_write_start_ptr */

								if (r != 0) {
#ifdef SIDEBAND_TERMINAL_STREAM_READ_BUFFER_DUMP
									printf("\tcircular_buffer_calculations() returned: %d, \"", r);
#endif
									return -1;	/* error */
								}

								last_char = input_buffer[0];

							} else {
								/* don't save the current character */
#ifdef SIDEBAND_TERMINAL_STREAM_READ_BUFFER_CRLF
								printf("SIDEBAND-EAT-<LF>%s",
									newline_string_p);
#endif

								/* don't detect <CR><LF> or <LF><CR> pair when the next character comes in */
								last_char = 0;
							}

						} else if (input_buffer[0] == 0x0d) {	/* CR carriage return */

							if (last_char != 0x0a) {
								connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_data_p[
									connsat_connection_sideband_channel_p->connection_input_buffer_p->calculated_buffer_wrapped_write_start_ptr] = input_buffer[0];

								connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_used_bytes += r;

								/* recalc buffer */
								r = circular_buffer_calculations(connsat_connection_sideband_channel_p->connection_input_buffer_p);
									/* this recalculates: connsat_connection_sideband_channel_p->connection_input_buffer_p->wrapped_write_start_ptr */

								if (r != 0) {
#ifdef SIDEBAND_TERMINAL_STREAM_READ_BUFFER_DUMP
									printf("\tcircular_buffer_calculations() returned: %d, \"", r);
#endif
									return -1;	/* error */
								}

								last_char = input_buffer[0];

							} else {
								/* don't save the current character */
#ifdef SIDEBAND_TERMINAL_STREAM_READ_BUFFER_CRLF
								printf("SIDEBAND-EAT-<CR>%s",
									newline_string_p);
#endif

								/* don't detect <CR><LF> or <LF><CR> pair when the next character comes in */
								last_char = 0;
							}

						} else {

							connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_data_p[
								connsat_connection_sideband_channel_p->connection_input_buffer_p->calculated_buffer_wrapped_write_start_ptr] = input_buffer[0];

							connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_used_bytes += r;

							/* recalc buffer */
							r = circular_buffer_calculations(connsat_connection_sideband_channel_p->connection_input_buffer_p);
								/* this recalculates: connsat_connection_sideband_channel_p->connection_input_buffer_p->wrapped_write_start_ptr */

							if (r != 0) {
#ifdef SIDEBAND_TERMINAL_STREAM_READ_BUFFER_DUMP
								printf("\tcircular_buffer_calculations() returned: %d, \"", r);
#endif
								return -1;	/* error */
							}

							last_char = input_buffer[0];

						}

						CONNSAT_TRACE;

					}

					CONNSAT_TRACE;

				}
			}

		} else {

			/* no more characters to read for now */

			CONNSAT_TRACE;

			break;
		}

		CONNSAT_TRACE;
	}
#endif

	CONNSAT_TRACE;

	return 0; /* ok */
}

/* connection_output_write_p */
/* write terminal stdout from large output distribution buffer */
/* Use POLLOUT, to be sure write will not block */
int
connsat_connection_sideband_channel_output_write (
	struct connsat_connection_s *connsat_connection_sideband_channel_p,
	int menu_mode_flag,
	int raw_mode_flag)
{
	char *newline_string_p;
	int buffer_distribution_max_readable_chunk; /* source_read_buffer */
//	int buffer_distribution_max_writeable_chunk; /* destination_write_buffer */
	int r;
#ifdef CIRCULAR_BUFFER_DUMP_DEBUG_SIDEBAND_OUTPUT
	int r2;
	int i;
	char printable_buff[32];
	char *printable_char_abrev;
	unsigned char current_char;
	int max_buffer_transfer;
#endif

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	CONNSAT_TRACE;

	if (connsat_connection_sideband_channel_p->connection_type != CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL) {

		printf("Error: %s-%d, connection is not CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL<%s", __FILE__, __LINE__,
			newline_string_p);

		return -1;
	}

	// circular_buffer_calculations(connsat_connection_sideband_channel_p->connection_input_buffer_p);
	circular_buffer_calculations(connsat_connection_sideband_channel_p->connection_output_buffer_p);

	/* read_the_output_buffer source_read_buffer */
	buffer_distribution_max_readable_chunk = connsat_connection_sideband_channel_p->connection_input_buffer_p->calculated_buffer_max_readable_chunk;
	/* destination_write_buffer */
	// buffer_distribution_max_writeable_chunk = connsat_connection_distribution_list_p->connection_output_buffer_p->buffer_distribution_max_writeable_chunk;

	/* check POLLOUT to avoid blocking on write() */

#ifdef SIDEBAND_TERMINAL_STREAM_WRITE_BUFFER_DUMP
	printf("\t\tOUTPUT_BUFFER_MAX_READABLE_CHUNK: %s-%d, buffer_distribution_max_readable_chunk==%d%s",
		__FILE__, __LINE__,
		buffer_distribution_max_readable_chunk,
		newline_string_p);
#endif

#ifdef CIRCULAR_BUFFER_DUMP_DEBUG_SIDEBAND_OUTPUT

		max_buffer_transfer = connsat_connection_sideband_channel_p->connection_output_buffer_p->calculated_buffer_max_readable_chunk;

		for (i = 0; i < max_buffer_transfer; ++i) {

			current_char =
				connsat_connection_sideband_channel_p->connection_output_buffer_p->circular_buffer_data_p[connsat_connection_sideband_channel_p->connection_output_buffer_p->unwrapped_read_start_ptr + i];

			printable_buff[0] = '\0';

			r2 = get_ascii_control_code_info(current_char, &printable_char_abrev, NULL);

			if (r2 == 0) {

				/* is a special control char */

				sprintf(printable_buff,
					"<%s>",
					printable_char_abrev
					);

			} else if (r2 != 0) {

				/* not a special control char */

				if (current_char >= 0x20 && current_char <= 0x7e) {
					sprintf(printable_buff,
						"\"%c\"",
						current_char
						);
				} else {
					sprintf(printable_buff,
						"\"???\""
						);
				}

			}

			printf("DUMP-SB: %s-%d: index==%d/size==%d, ptr==%d, char==0x%02x-%s",
				__FILE__, __LINE__,
				i,
				max_buffer_transfer,
				connsat_connection_sideband_channel_p->connection_output_buffer_p->unwrapped_read_start_ptr + i,
				current_char,
				printable_buff);

			printf("%s", newline_string_p);
		}
#endif


	/* write output_fd from connsat_connection_distribution_list_p->connection_output_buffer_p */
	r = circular_buffer_stream_write(
		connsat_connection_sideband_channel_p->connection_output_buffer_p,
		connsat_connection_sideband_channel_p->connection_output_fd
		);

	return 0; /* ok */
}

int
connsat_connection_sideband_channel_buffer_distribution (
	struct connsat_connection_s *connsat_connection_sideband_channel_p,
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

	CONNSAT_TRACE;

	if (connsat_connection_sideband_channel_p->connection_type != CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL) {

		printf("Error: %s-%d, connection is not CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL<%s", __FILE__, __LINE__,
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
		r = connsat_connection_sideband_channel_buffer_distribution_worker (
			connsat_connection_sideband_channel_p,
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
				r = connsat_connection_sideband_channel_buffer_distribution_worker (
					connsat_connection_sideband_channel_p,
					worker_distribution_mode_flag,
/* USE */				&buffer_distribution_max_readable_chunk, /* calculated input, source_read_buffer */
/* USE */				&buffer_distribution_max_writeable_chunk, /* calculated input, destination_write_buffer */
					raw_mode_flag);

				if (r != 0) {

					printf("Error: %s-%d, buffer transfer copy error for CONNECTION_TYPE_SIDEBAND_CHANNEL<%s",
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
			printf("Error: %s-%d, can't perform buffer transfer without perform calculations first for CONNECTION_TYPE_SIDEBAND_CHANNEL<%s",
				__FILE__, __LINE__,
				newline_string_p);

			return -4;
		}
	}

	return 0; /* ok */
}

int
connsat_connection_sideband_channel_buffer_distribution_worker (
	struct connsat_connection_s *connsat_connection_sideband_channel_p,
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

	int buffer_distribution_max_readable_chunk; /* source_read_buffer */
	int buffer_distribution_max_writeable_chunk; /* destination_write_buffer */
	int from_count;
	char *from_string_p;
	char *to_string_p;
	int r;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	CONNSAT_TRACE;

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

		printf("Error: %s-%d, NULL pointer for buffer_distribution_max_readable_chunk%s", __FILE__, __LINE__, newline_string_p);

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

#ifdef EXPERIMENTAL_CONTROL_MENU_UPDATES_001
					if (connsat_connection_sideband_channel_p->connection_distribution_self_loopback_enable_flag == 0) {
						/* no self_loopback */
						break;
					}
#endif

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
					circular_buffer_calculations(connsat_connection_sideband_channel_p->connection_input_buffer_p);

					/* destination of copy buffer */
					circular_buffer_calculations(connsat_connection_distribution_list_p->connection_output_buffer_p);

					/* source_read_buffer */
					buffer_distribution_max_readable_chunk = connsat_connection_sideband_channel_p->connection_input_buffer_p->calculated_buffer_max_readable_chunk;
					/* destination_write_buffer */
					buffer_distribution_max_writeable_chunk = connsat_connection_distribution_list_p->connection_output_buffer_p->calculated_buffer_max_writeable_chunk;

					/* forward to all CONNECTION_TYPE_TERMINAL */
						/* local-loopback mode */
					if (from_count++ == 0) {

						if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {
							*calculation_mode_max_readable_chunk_p = buffer_distribution_max_readable_chunk;
							*calculation_mode_max_writeable_chunk_p = buffer_distribution_max_writeable_chunk;
						}

						if ( (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) ||
							(distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) ) {

							printf("%s\t\t\t%s: %s-%d, CONNECTION_TYPE_SIDEBANE_TERMINAL-smallbuffer[%d/%d] >>%d<<%s",
								newline_string_p,
								from_string_p,
								__FILE__, __LINE__,
								connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_used_bytes,
								connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_allocated_size,
								buffer_distribution_max_readable_chunk,
								newline_string_p);
						}

					} else {

						if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {

#if 1 /* redundant code */
							/* source of distribution */
							if (*calculation_mode_max_readable_chunk_p >= buffer_distribution_max_readable_chunk) {
								*calculation_mode_max_readable_chunk_p = buffer_distribution_max_readable_chunk;
							}
#endif

							/* multi-destinations of distribution */
							if (*calculation_mode_max_writeable_chunk_p >= buffer_distribution_max_writeable_chunk) {
								*calculation_mode_max_writeable_chunk_p = buffer_distribution_max_writeable_chunk;
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
							buffer_distribution_max_writeable_chunk,
							newline_string_p);
					}

					if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE) {
						int max_buffer_transfer = *calculation_mode_max_readable_chunk_p;
						if (max_buffer_transfer > *calculation_mode_max_writeable_chunk_p) {
							max_buffer_transfer = *calculation_mode_max_writeable_chunk_p;
						}

#ifdef MASTER_DEBUG_FLAG_01_MULTI_TRANSFER
						printf("%s-%d, MULTI_TRANSFER: TERMINAL->TERMINAL max_buffer_transfer==%d%s",
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
								connsat_connection_sideband_channel_p->connection_input_buffer_p,
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
					break;

				case CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL:

					/* SELF_LOOPBACK */

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
					circular_buffer_calculations(connsat_connection_sideband_channel_p->connection_input_buffer_p);

					/* destination of copy buffer */
					circular_buffer_calculations(connsat_connection_distribution_list_p->connection_output_buffer_p);

					/* source_read_buffer */
					buffer_distribution_max_readable_chunk = connsat_connection_sideband_channel_p->connection_input_buffer_p->calculated_buffer_max_readable_chunk;
					/* destination_write_buffer */
					buffer_distribution_max_writeable_chunk = connsat_connection_distribution_list_p->connection_output_buffer_p->calculated_buffer_max_writeable_chunk;

					/* forward to all CONNECTION_TYPE_SIDEBAND_CHANNEL */
					if (from_count++ == 0) {

						if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {
							*calculation_mode_max_readable_chunk_p = buffer_distribution_max_readable_chunk;
							*calculation_mode_max_writeable_chunk_p = buffer_distribution_max_writeable_chunk;
						}

						if ( (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) ||
							(distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) ) {

							printf("%s\t\t\t%s: %s-%d, CONNECTION_TYPE_SIDEBAND_CHANNEL-smallbuffer[%d/%d] >>%d<<%s",
								newline_string_p,
								from_string_p,
								__FILE__, __LINE__,
								connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_used_bytes,
								connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_allocated_size,
								buffer_distribution_max_readable_chunk,
								newline_string_p);
						}
					} else {

						if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {

							if (*calculation_mode_max_readable_chunk_p >= buffer_distribution_max_readable_chunk) {
								*calculation_mode_max_readable_chunk_p = buffer_distribution_max_readable_chunk;
							}

							if (*calculation_mode_max_writeable_chunk_p >= buffer_distribution_max_writeable_chunk) {
								*calculation_mode_max_writeable_chunk_p = buffer_distribution_max_writeable_chunk;
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
							buffer_distribution_max_writeable_chunk,
							newline_string_p);
					}

					if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE) {
						int max_buffer_transfer = *calculation_mode_max_readable_chunk_p;
						if (max_buffer_transfer > *calculation_mode_max_writeable_chunk_p) {
							max_buffer_transfer = *calculation_mode_max_writeable_chunk_p;
						}

#ifdef MASTER_DEBUG_FLAG_01_MULTI_TRANSFER
						printf("%s-%d, MULTI_TRANSFER: SIDEBAND_CHANNEL->SIDEBAND_CHANNEL  max_buffer_transfer==%d%s",
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
								connsat_connection_sideband_channel_p->connection_input_buffer_p,
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

					circular_buffer_calculations(connsat_connection_sideband_channel_p->connection_input_buffer_p);

					circular_buffer_calculations(connsat_connection_distribution_list_p->connection_output_buffer_p);

					/* source_read_buffer */
					buffer_distribution_max_readable_chunk = connsat_connection_sideband_channel_p->connection_input_buffer_p->calculated_buffer_max_readable_chunk;
					/* destination_write_buffer */
					buffer_distribution_max_writeable_chunk = connsat_connection_distribution_list_p->connection_output_buffer_p->calculated_buffer_max_writeable_chunk;

					/* forward to all CONNECTION_TYPE_MENU */
					if (from_count++ == 0) {

						if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {
							*calculation_mode_max_readable_chunk_p = buffer_distribution_max_readable_chunk;
							*calculation_mode_max_writeable_chunk_p = buffer_distribution_max_writeable_chunk;
						}

						if ( (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) ||
							(distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) ) {

							printf("%s\t\t\t%s: %s-%d, CONNECTION_TYPE_SIDEBAND_CHANNEL-smallbuffer[%d/%d] >>%d<<%s",
								newline_string_p,
								from_string_p,
								__FILE__, __LINE__,
								connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_used_bytes,
								connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_allocated_size,
								buffer_distribution_max_readable_chunk,
								newline_string_p);
						}
					} else {

						if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {

#if 1 /* redundant code */
							/* source of distribution */
							if (*calculation_mode_max_readable_chunk_p >= buffer_distribution_max_readable_chunk) {
								*calculation_mode_max_readable_chunk_p = buffer_distribution_max_readable_chunk;
							}
#endif

							/* multi-destinations of distribution */
							if (*calculation_mode_max_writeable_chunk_p >= buffer_distribution_max_writeable_chunk) {
								*calculation_mode_max_writeable_chunk_p = buffer_distribution_max_writeable_chunk;
							}
						}
					}

					if ( (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) ||
						(distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) ) {

						printf("\t\t\t\t%s: %s-%d, CONNECTION_TYPE_CONTROL_MENU-bigbuffer[%d/%d] <<<%d>>>%s",
							to_string_p,
							__FILE__, __LINE__,
							connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_used_bytes,
							connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_allocated_size,
							buffer_distribution_max_writeable_chunk,
							newline_string_p);
					}

					if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE) {
						int max_buffer_transfer = *calculation_mode_max_readable_chunk_p;
						if (max_buffer_transfer > *calculation_mode_max_writeable_chunk_p) {
							max_buffer_transfer = *calculation_mode_max_writeable_chunk_p;
						}

#ifdef MASTER_DEBUG_FLAG_01_MULTI_TRANSFER
						printf("%s-%d, MULTI_TRANSFER: TERMINAL->CONTROL_MENU max_buffer_transfer==%d%s",
							__FILE__, __LINE__,
							max_buffer_transfer,
							newline_string_p);
#endif

						if (max_buffer_transfer > 0) {

							if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) {

								printf("\t\t\t\t\t%s: %s-%d, CONNECTION_TYPE_CONTROL_MENU-bigbuffer[%d/%d] <<<%d>>> TRANSFER%s",
									to_string_p,
									__FILE__, __LINE__,
									connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_used_bytes,
									connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_allocated_size,
									max_buffer_transfer,
									newline_string_p);
							}

							/* do the buffer copy */
							r = circular_buffer_multi_block_transfer(
								connsat_connection_sideband_channel_p->connection_input_buffer_p,
								connsat_connection_distribution_list_p->connection_output_buffer_p,
								max_buffer_transfer);

							if (r != 0) {

								printf("Error: %s-%d, circular_buffer_multi_block_transfer() returned %d%s",
									__FILE__, __LINE__,
									r,
									newline_string_p);

								return -7; /* error */
							}
						}
					}

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
					connsat_connection_sideband_channel_p->connection_input_buffer_p,
					max_buffer_transfer);

				if (r != 0) {

					printf("Error: %s-%d, circular_buffer_multi_block_transfer() returned %d%s",
						__FILE__, __LINE__,
						r,
						newline_string_p);

					return -11; /* error */
				}

				if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) {

					printf("%s\t\t\t\t\t%s: %s-%d, CONNECTION_TYPE_SIDEBAND_CHANNEL-smallbuffer[%d/%d] <<<%d>>> TRANSFER-COMPLETE%s",
						newline_string_p,
						to_string_p,
						__FILE__, __LINE__,
						connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_used_bytes,
						connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_allocated_size,
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

#ifdef EXPERIMENTAL_CONTROL_MENU_UPDATES_001
					if (connsat_connection_sideband_channel_p->connection_distribution_self_loopback_enable_flag == 0) {
						/* no self_loopback */
						break;
					}
#endif

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
					circular_buffer_calculations(connsat_connection_sideband_channel_p->connection_input_buffer_p);

					/* destination of copy buffer */
					circular_buffer_calculations(connsat_connection_distribution_list_p->connection_output_buffer_p);

					/* source_read_buffer */
					buffer_distribution_max_readable_chunk = connsat_connection_sideband_channel_p->connection_input_buffer_p->calculated_buffer_max_readable_chunk;
					/* destination_write_buffer */
					buffer_distribution_max_writeable_chunk = connsat_connection_distribution_list_p->connection_output_buffer_p->calculated_buffer_max_writeable_chunk;

#ifdef MASTER_DEBUG_FLAG_01_MULTI_TRANSFER
					printf("%s-%d, MULTI_TRANSFER: SIDEBAND_CHANNEL->TERMINAL  *calculation_mode_max_readable_chunk_p==%d, *calculation_mode_max_writeable_chunk_p==%d, name==%s, dest==%s%s",
						__FILE__, __LINE__,
						*calculation_mode_max_readable_chunk_p,
						*calculation_mode_max_writeable_chunk_p,
						connsat_connection_sideband_channel_p->name,
						connsat_connection_distribution_list_p->name,
						newline_string_p);
#endif

					/* forward to all CONNECTION_TYPE_TERMINAL */
						/* local-loopback mode */
					if (from_count++ == 0) {

						if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {
							*calculation_mode_max_readable_chunk_p = buffer_distribution_max_readable_chunk;
							*calculation_mode_max_writeable_chunk_p = buffer_distribution_max_writeable_chunk;
						}

#ifdef MASTER_DEBUG_FLAG_01_MULTI_TRANSFER
						printf("%s-%d, MULTI_TRANSFER: SIDEBAND_CHANNEL->TERMINAL  *calculation_mode_max_readable_chunk_p==%d, *calculation_mode_max_writeable_chunk_p==%d, name==%s, dest==%s%s",
							__FILE__, __LINE__,
							*calculation_mode_max_readable_chunk_p,
							*calculation_mode_max_writeable_chunk_p,
							connsat_connection_sideband_channel_p->name,
							connsat_connection_distribution_list_p->name,
							newline_string_p);
#endif
						if ( (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) ||
							(distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) ) {

							printf("%s\t\t\t%s: %s-%d, CONNECTION_TYPE_SIDEBAND_CHANNEL-smallbuffer[%d/%d] >>%d<<%s",
								newline_string_p,
								from_string_p,
								__FILE__, __LINE__,
								connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_used_bytes,
								connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_allocated_size,
								buffer_distribution_max_readable_chunk,
								newline_string_p);
						}
					} else {

						if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {

#if 1 /* redundant code */
							/* source of distribution */
							if (*calculation_mode_max_readable_chunk_p >= buffer_distribution_max_readable_chunk) {
								*calculation_mode_max_readable_chunk_p = buffer_distribution_max_readable_chunk;
							}
#endif

							/* multi-destinations of distribution */
							if (*calculation_mode_max_writeable_chunk_p >= buffer_distribution_max_writeable_chunk) {
								*calculation_mode_max_writeable_chunk_p = buffer_distribution_max_writeable_chunk;
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
							buffer_distribution_max_writeable_chunk,
							newline_string_p);
					}

#ifdef MASTER_DEBUG_FLAG_01_MULTI_TRANSFER
					printf("%s-%d, MULTI_TRANSFER: SIDEBAND_CHANNEL->TERMINAL  *calculation_mode_max_readable_chunk_p==%d, *calculation_mode_max_writeable_chunk_p==%d, name==%s, dest==%s%s",
						__FILE__, __LINE__,
						*calculation_mode_max_readable_chunk_p,
						*calculation_mode_max_writeable_chunk_p,
						connsat_connection_sideband_channel_p->name,
						connsat_connection_distribution_list_p->name,
						newline_string_p);
#endif

					if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE) {
						int max_buffer_transfer = *calculation_mode_max_readable_chunk_p;
						if (max_buffer_transfer > *calculation_mode_max_writeable_chunk_p) {
							max_buffer_transfer = *calculation_mode_max_writeable_chunk_p;
						}

#ifdef MASTER_DEBUG_FLAG_01_MULTI_TRANSFER
						printf("%s-%d, MULTI_TRANSFER: SIDEBAND_CHANNEL->TERMINAL  max_buffer_transfer==%d, *calculation_mode_max_readable_chunk_p==%d, *calculation_mode_max_writeable_chunk_p==%d%s",
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
								connsat_connection_sideband_channel_p->connection_input_buffer_p,
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
					break;

				case CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL:

					/* SELF_LOOPBACK */

					/* input side of destination connection not is open */
					if (connsat_connection_distribution_list_p->connection_input_state != CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {
						break;
					}

					/* output side of destination connection not is open */
					if (connsat_connection_distribution_list_p->connection_output_state != CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {
						break;
					}

#ifdef EXPERIMENTAL_CONTROL_MENU_UPDATES_001

					/* SELF_LOOPBACK */
					if (connsat_connection_sideband_channel_p->connection_distribution_self_loopback_enable_flag == 0) {
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

					circular_buffer_calculations(connsat_connection_sideband_channel_p->connection_input_buffer_p);

					circular_buffer_calculations(connsat_connection_distribution_list_p->connection_output_buffer_p);

					/* source_read_buffer */
					buffer_distribution_max_readable_chunk = connsat_connection_sideband_channel_p->connection_input_buffer_p->calculated_buffer_max_readable_chunk;
					/* destination_write_buffer */
					buffer_distribution_max_writeable_chunk = connsat_connection_distribution_list_p->connection_output_buffer_p->calculated_buffer_max_writeable_chunk;

					/* forward to all CONNECTION_TYPE_SIDEBAND_CHANNEL */
					if (from_count++ == 0) {

						if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {
							*calculation_mode_max_readable_chunk_p = buffer_distribution_max_readable_chunk;
							*calculation_mode_max_writeable_chunk_p = buffer_distribution_max_writeable_chunk;
						}

						if ( (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) ||
							(distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) ) {

							printf("%s\t\t\t%s: %s-%d, CONNECTION_TYPE_SIDEBAND_CHANNEL-smallbuffer[%d/%d] >>%d<<%s",
								newline_string_p,
								from_string_p,
								__FILE__, __LINE__,
								connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_used_bytes,
								connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_allocated_size,
								buffer_distribution_max_readable_chunk,
								newline_string_p);
						}
					} else {

						if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {

							if (*calculation_mode_max_readable_chunk_p >= buffer_distribution_max_readable_chunk) {
								*calculation_mode_max_readable_chunk_p = buffer_distribution_max_readable_chunk;
							}

							if (*calculation_mode_max_writeable_chunk_p >= buffer_distribution_max_writeable_chunk) {
								*calculation_mode_max_writeable_chunk_p = buffer_distribution_max_writeable_chunk;
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
							buffer_distribution_max_writeable_chunk,
							newline_string_p);
					}

					if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE) {
						int max_buffer_transfer = *calculation_mode_max_readable_chunk_p;
						if (max_buffer_transfer > *calculation_mode_max_writeable_chunk_p) {
							max_buffer_transfer = *calculation_mode_max_writeable_chunk_p;
						}

#ifdef MASTER_DEBUG_FLAG_01_MULTI_TRANSFER
						printf("%s-%d, MULTI_TRANSFER: SIDEBAND_CHANNEL->SIDEBAND_CHANNEL max_buffer_transfer==%d%s",
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
								connsat_connection_sideband_channel_p->connection_input_buffer_p,
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
					circular_buffer_calculations(connsat_connection_sideband_channel_p->connection_input_buffer_p);

					/* destination of copy buffer */
					circular_buffer_calculations(connsat_connection_distribution_list_p->connection_output_buffer_p);

					/* source_read_buffer */
					buffer_distribution_max_readable_chunk = connsat_connection_sideband_channel_p->connection_input_buffer_p->calculated_buffer_max_readable_chunk;
					/* destination_write_buffer */
					buffer_distribution_max_writeable_chunk = connsat_connection_distribution_list_p->connection_output_buffer_p->calculated_buffer_max_writeable_chunk;

					/* forward to all CONNECTION_TYPE_FORKPTY */
					if (from_count++ == 0) {

						if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {
							*calculation_mode_max_readable_chunk_p = buffer_distribution_max_readable_chunk;
							*calculation_mode_max_writeable_chunk_p = buffer_distribution_max_writeable_chunk;
						}

						if ( (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) ||
							(distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) ) {

							printf("%s\t\t\t%s: %s-%d, CONNECTION_TYPE_SIDEBAND_CHANNEL-smallbuffer[%d/%d] >>%d<<%s",
								newline_string_p,
								from_string_p,
								__FILE__, __LINE__,
								connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_used_bytes,
								connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_allocated_size,
								buffer_distribution_max_readable_chunk,
								newline_string_p);
						}

					} else {

						if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE) {

							if (*calculation_mode_max_readable_chunk_p >= buffer_distribution_max_readable_chunk) {
								*calculation_mode_max_readable_chunk_p = buffer_distribution_max_readable_chunk;
							}

							if (*calculation_mode_max_writeable_chunk_p >= buffer_distribution_max_writeable_chunk) {
								*calculation_mode_max_writeable_chunk_p = buffer_distribution_max_writeable_chunk;
							}
						}
					}

					if ( (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) ||
						(distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) ) {

						printf("\t\t\t\t%s: %s-%d, CONNECTION_TYPE_FORKPTY-bigbuffer[%d/%d] <<<%d>>>%s",
							to_string_p,
							__FILE__, __LINE__,
							connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_used_bytes,
							connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_allocated_size,
							buffer_distribution_max_writeable_chunk,
							newline_string_p);
					}

					if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE) {
						int max_buffer_transfer = *calculation_mode_max_readable_chunk_p;
						if (max_buffer_transfer > *calculation_mode_max_writeable_chunk_p) {
							max_buffer_transfer = *calculation_mode_max_writeable_chunk_p;
						}

#ifdef MASTER_DEBUG_FLAG_01_MULTI_TRANSFER
						printf("%s-%d, MULTI_TRANSFER: SIDEBAND_CHANNEL->FORKPTY max_buffer_transfer==%d%s",
							__FILE__, __LINE__,
							max_buffer_transfer,
							newline_string_p);
#endif

						if (max_buffer_transfer > 0) {

							if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) {

								printf("\t\t\t\t\t%s: %s-%d, CONNECTION_TYPE_FORKPTY-bigbuffer[%d/%d] <<<%d>>> TRANSFER%s",
									to_string_p,
									__FILE__, __LINE__,
									connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_used_bytes,
									connsat_connection_distribution_list_p->connection_output_buffer_p->circular_buffer_allocated_size,
									max_buffer_transfer,
									newline_string_p);
							}

							/* do the buffer copy */
							r = circular_buffer_multi_block_transfer(
								connsat_connection_sideband_channel_p->connection_input_buffer_p,
								connsat_connection_distribution_list_p->connection_output_buffer_p,
								max_buffer_transfer);

							if (r != 0) {

								printf("Error: %s-%d, circular_buffer_multi_block_transfer() returned %d%s",
									__FILE__, __LINE__,
									r,
									newline_string_p);

								return -10; /* error */
							}
						}
					}
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
					connsat_connection_sideband_channel_p->connection_input_buffer_p,
					max_buffer_transfer);

				if (r != 0) {

					printf("Error: %s-%d, circular_buffer_multi_block_transfer() returned %d%s",
						__FILE__, __LINE__,
						r,
						newline_string_p);

					return -11; /* error */
				}

				if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) {

					printf("%s\t\t\t\t\t%s: %s-%d, CONNECTION_TYPE_SIDEBAND_CHANNEL-smallbuffer[%d/%d] <<<%d>>> TRANSFER-COMPLETE%s",
						newline_string_p,
						to_string_p,
						__FILE__, __LINE__,
						connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_used_bytes,
						connsat_connection_sideband_channel_p->connection_input_buffer_p->circular_buffer_allocated_size,
						max_buffer_transfer,
						newline_string_p);
				}
			}
		}
	}

	return 0; /* ok */
}

