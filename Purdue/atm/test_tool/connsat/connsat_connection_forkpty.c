
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <stdlib.h>
	// free()

#if 0
#include <poll.h>
#endif

#include "connsat_main.h"

#include "connsat_connection_forkpty.h"
#include "connsat_connections.h"
#include "connsat_spawn_pty.h"
#include "connsat_child_args.h"
#include "connsat_options.h"
#include "connsat_timestamp.h"
#include "connsat_buffer_printf.h"

int connsat_connection_forkpty_amaster_fd = -1;
pid_t connsat_connection_forkpty_child_pid = 0;
char *connsat_connection_forkpty_spawn_command_string_p;
char **connsat_connection_forkpty_spawn_command_argv_pp;
int connsat_connection_forkpty_spawn_command_argv_size;
int connsat_connection_forkpty_spawn_command_argc;

time_t connsat_connection_forkpty_spawn_time_stamp = 0;	/* last keepalive time seconds */

int connsat_connection_forkpty_buffer_distribution_worker(
	struct connsat_connection_s *connsat_connection_forkpty_p,
	int distribution_mode_flag,
	int *calculation_mode_max_readable_chunk_p,
	int *calculation_mode_max_writeable_chunk_p,
	int raw_mode_flag);

int
connsat_connection_forkpty (
	struct connsat_connection_s **connsat_connection_forkpty_pp,
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

	if (connsat_options_all.spawnpty_shell_command_p != NULL) {

		connsat_connection_forkpty_spawn_command_string_p =
			connsat_options_all.spawnpty_shell_command_p;

	} else {
		/* pick a sensible default */
		connsat_connection_forkpty_spawn_command_string_p =
			"/bin/bash -i";
	}

	printf("Connsat-Initialize-ForkPTY: \"%s\"%s",
		connsat_connection_forkpty_spawn_command_string_p,
		newline_string_p);

	r = connsat_connection_new(
		"ForkPTY-Connnection",
		CONNSAT_CONNECTION_TYPE_FORKPTY, 	// CONNSAT_CONNECTION_TYPE_NONE,
		2048,					/* input buffer size */
		10240,					/* output buffer size */
		raw_mode_flag,

		connsat_connection_forkpty_input_open,	/* connection_input_open_p */
		connsat_connection_forkpty_input_close,	/* connection_input_close_p */

		connsat_connection_forkpty_output_open,	/* connection_output_open_p */
		connsat_connection_forkpty_output_close,	/* connection_output_close_p */

		connsat_connection_forkpty_input_read,	/* connection_input_read_p */
		connsat_connection_forkpty_output_write,	/* connection_output_write_p */

		connsat_connection_forkpty_buffer_distribution, /* buffer_distribution_p */

		connsat_connection_forkpty_pp		/* connsat_connection_new_p */
		);

	if (r < 0) {
		printf("FAILED: connsat_connection_forkpty_p connsat_connection_new(): %d%s", r, newline_string_p);

		// exit(1);
		return -1; /* error */
	}

#ifdef CONNSAT_DEBUG_STARTUP
	printf("\tCREATED: connsat_connection_forkpty_p: %p \"%s\"%s",
		*connsat_connection_forkpty_pp,
		(*connsat_connection_forkpty_pp)->name,
		newline_string_p);
#endif

#if 1
	if (connsat_options_all.spawnshell_enable_flag != 0) {

		r = connsat_connection_forkpty_input_open(
			*connsat_connection_forkpty_pp,
			menu_mode_flag,
			raw_mode_flag);

		r = connsat_connection_forkpty_output_open(
			*connsat_connection_forkpty_pp,
			menu_mode_flag,
			raw_mode_flag);
	}
#endif

	return 0; /* ok */
}

/* connection_input_open_p */
int
connsat_connection_forkpty_input_open (
	struct connsat_connection_s *connsat_connection_forkpty_p,
	int menu_mode_flag,
	int raw_mode_flag)
{

	int r;

	connsat_connection_forkpty_p->connection_input_fd = -1;	/* stdin-fd */

	connsat_connection_forkpty_p->connection_input_state = CONNSAT_CONNECTION_STATE_OPEN_ACTIVE;

	/* add the input_fd stream to the POLLIN list */
	connsat_connection_forkpty_p->connection_input_fd_poll_flag = 1;
//@@@@@@@
//POLLIN

	/* enable the input buffer to receive characters */
	connsat_connection_forkpty_p->connection_input_buffer_enable_flag = 1;

	/* enables receiving buffer distributions from other input buffers */
	connsat_connection_forkpty_p->connection_input_buffer_distribution_flag = 1;

	if (connsat_connection_forkpty_amaster_fd == -1) {

		connsat_connection_forkpty_spawn_command_argv_pp = NULL;
		connsat_connection_forkpty_spawn_command_argc = 0;

		if (connsat_connection_forkpty_spawn_command_string_p != NULL) {

			r = child_args_parse_string(
				connsat_connection_forkpty_spawn_command_string_p,
				&connsat_connection_forkpty_spawn_command_argv_size,
				&connsat_connection_forkpty_spawn_command_argc,
				&connsat_connection_forkpty_spawn_command_argv_pp);

			if (r != 0) {
				r = connsat_connection_forkpty_input_close(
					connsat_connection_forkpty_p,
					menu_mode_flag,
					raw_mode_flag);

				return -1; /* error */
			}

#if 0
			child_args_argv_dump(
				&connsat_connection_forkpty_spawn_command_argc,
				&connsat_connection_forkpty_spawn_command_argv_pp,
				raw_mode_flag);
#endif
		}

		r = connsat_spawn_pty(
			&connsat_connection_forkpty_child_pid,
			&connsat_connection_forkpty_amaster_fd,
			connsat_connection_forkpty_spawn_command_argc,
			connsat_connection_forkpty_spawn_command_argv_pp);

		connsat_connection_forkpty_p->connection_input_fd = connsat_connection_forkpty_amaster_fd;

	} else {

		/* stream must have been openned by the
		** connsat_connection_forkpty_output_open() already.
		*/
		connsat_connection_forkpty_p->connection_input_fd = connsat_connection_forkpty_amaster_fd;
	}

	return 0; /* ok */
}

/* connection_input_close_p */
int
connsat_connection_forkpty_input_close (
	struct connsat_connection_s *connsat_connection_forkpty_p,
	int menu_mode_flag,
	int raw_mode_flag)
{
	char *newline_string_p;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	connsat_connection_forkpty_p->connection_input_state = CONNSAT_CONNECTION_STATE_CLOSED;

	connsat_connection_forkpty_p->connection_input_fd = -1;	/* stdin-fd */

	connsat_connection_forkpty_p->connection_input_fd_poll_flag = 0;
//@@@@@@@
//POLLIN

	connsat_connection_forkpty_p->connection_input_buffer_enable_flag = 0;

	connsat_connection_forkpty_p->connection_input_buffer_distribution_flag = 0;

	if (connsat_connection_forkpty_amaster_fd != -1) {
		connsat_connection_forkpty_amaster_fd = -1;
		connsat_connection_forkpty_child_pid = 0;
		connsat_connection_forkpty_spawn_command_argc = 0;
		connsat_connection_forkpty_spawn_command_argv_size = 0;
		if (connsat_connection_forkpty_spawn_command_argv_pp != NULL) {
			free(connsat_connection_forkpty_spawn_command_argv_pp);
			connsat_connection_forkpty_spawn_command_argv_pp = NULL;
		}
	}

	return 0; /* ok */
}

/* connection_output_open_p */
int
connsat_connection_forkpty_output_open (
	struct connsat_connection_s *connsat_connection_forkpty_p,
	int menu_mode_flag,
	int raw_mode_flag)
{
	int r;

//@@@@@@@ UPDATE from connsat_spawn_pty
	connsat_connection_forkpty_p->connection_output_fd = -1; /* stdout-fd */

	connsat_connection_forkpty_p->connection_output_state = CONNSAT_CONNECTION_STATE_OPEN_ACTIVE;

	/* add the output_fd stream to the POLLOUT list */
	connsat_connection_forkpty_p->connection_output_fd_poll_flag = 1;
//@@@@@@@
//POLLOUT

	/* enable the output buffer to receive characters */
	connsat_connection_forkpty_p->connection_output_buffer_enable_flag = 1;

	/* enables receiving buffer distributions from other input buffers */
	connsat_connection_forkpty_p->connection_output_buffer_distribution_flag = 1;

	/* SELF_LOOPBACK */
	connsat_connection_forkpty_p->connection_distribution_self_loopback_enable_flag = 1;

	if (connsat_connection_forkpty_amaster_fd == -1) {

		connsat_connection_forkpty_spawn_command_argv_pp = NULL;
		connsat_connection_forkpty_spawn_command_argc = 0;

		r = connsat_spawn_pty(
			&connsat_connection_forkpty_child_pid,
			&connsat_connection_forkpty_amaster_fd,
			connsat_connection_forkpty_spawn_command_argc,
			connsat_connection_forkpty_spawn_command_argv_pp);

		connsat_connection_forkpty_p->connection_output_fd = connsat_connection_forkpty_amaster_fd;

	} else {

		/* stream must have been openned by the
		** connsat_connection_forkpty_input_open() already.
		*/
		connsat_connection_forkpty_p->connection_output_fd = connsat_connection_forkpty_amaster_fd;
	}

	return 0; /* ok */
}

/* connection_output_close_p */
int
connsat_connection_forkpty_output_close (
	struct connsat_connection_s *connsat_connection_forkpty_p,
	int menu_mode_flag,
	int raw_mode_flag)
{

	connsat_connection_forkpty_p->connection_output_state = CONNSAT_CONNECTION_STATE_CLOSED;

	connsat_connection_forkpty_p->connection_output_fd = -1; /* stdout-fd */

	connsat_connection_forkpty_p->connection_output_fd_poll_flag = 0;
//@@@@@@@
//POLLOUT

	connsat_connection_forkpty_p->connection_output_buffer_enable_flag = 0;

	connsat_connection_forkpty_p->connection_output_buffer_distribution_flag = 0;

	/* SELF_LOOPBACK */
	connsat_connection_forkpty_p->connection_distribution_self_loopback_enable_flag = 0;

	if (connsat_connection_forkpty_amaster_fd != -1) {
		connsat_connection_forkpty_amaster_fd = -1;
		connsat_connection_forkpty_child_pid = 0;
		connsat_connection_forkpty_spawn_command_argc = 0;
		connsat_connection_forkpty_spawn_command_argv_size = 0;
		if (connsat_connection_forkpty_spawn_command_argv_pp != NULL) {
			free(connsat_connection_forkpty_spawn_command_argv_pp);
			connsat_connection_forkpty_spawn_command_argv_pp = NULL;
		}
	}

	return 0; /* ok */
}

/* connection_input_read_p */
/* read forkpty stdin into small distribution buffer */
/* Use POLLIN, to be sure read will not block */
/* This is not to be called without using POLLIN if that is enabled */
int
connsat_connection_forkpty_input_read (
	struct connsat_connection_s *connsat_connection_forkpty_p,
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
	int r2;
	struct pollfd pollfd_array[1];
	int poll_timeout;
	int retry_count;
	int max_retries;
	char input_buffer[16];
	int i;
	time_t current_time_stamp;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	/* check the small input buffer max_writeable_chunk size */
	circular_buffer_calculations(connsat_connection_forkpty_p->connection_input_buffer_p);

	small_input_buffer_max_writeable_chunk = connsat_connection_forkpty_p->connection_input_buffer_p->calculated_buffer_max_writeable_chunk;

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
	r = connsat_connection_forkpty_buffer_distribution_worker(
		connsat_connection_forkpty_p,
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

	pollfd_array[0].fd = connsat_connection_forkpty_p->connection_input_fd;	/* forkpty fd */

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

			r = circular_buffer_calculations(connsat_connection_forkpty_p->connection_input_buffer_p);

			if (connsat_connection_forkpty_p->connection_input_buffer_p->calculated_buffer_max_writeable_chunk >= 1) {

				CONNSAT_TRACE;
#if 1
				// read fd into buffer */
				r = read(connsat_connection_forkpty_p->connection_input_fd,
					input_buffer,
					1);

#else
				// read fd into buffer */
				r = read(connsat_connection_forkpty_p->connection_input_fd,
					&connsat_connection_forkpty_p->connection_input_buffer_p->circular_buffer_data_p[
						connsat_connection_forkpty_p->connection_input_buffer_p->wrapped_write_start_ptr],
					// connsat_connection_forkpty_p->connection_input_buffer_p->max_writeable_chunk);
					1);
#endif

				if (r == -1) {
					// errno
#ifdef FORKPTY_STREAM_READ_BUFFER_DUMP
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
#ifdef FORKPTY_STREAM_READ_BUFFER_DUMP
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

#if 0 /* TEMP_DEBUG */
printf("READ-BYTES-%s-%d: %d: %d %c%s",
__FILE__, __LINE__,
r,
input_buffer[0], input_buffer[0],
newline_string_p);
usleep(200000);
#endif


#ifdef KEEP_ALIVE_TIME_FEATURE	/* timestamp/watchdog update: channel read */

					r2 = get_timestamp_seconds(&current_time_stamp);

					if (connsat_connection_forkpty_spawn_time_stamp == 0) {

						/* last keepalive time seconds */
						connsat_connection_forkpty_spawn_time_stamp = current_time_stamp;

					} else {

#ifdef KEEP_ALIVE_TIME_TRACE_RW
						/*
						** Data read into the small input buffer from remote connection and distributed.
						** Reset timestamp like a watchdog timer...
						*/
						printf("%sREFRESH-KEEP-ALIVE-TIME-FOR-READ-DATA: %s-%d, spawn_time_stamp==%ld, current_time_stamp==%ld%s",
							newline_string_p,
							__FILE__, __LINE__,
							connsat_connection_forkpty_spawn_time_stamp,
							current_time_stamp,
							newline_string_p);
#endif

						/* update the wait timer */
						connsat_connection_forkpty_spawn_time_stamp = current_time_stamp;
					}
#endif /* KEEP_ALIVE_TIME_FEATURE */



#if 1
					CONNSAT_TRACE;

					/* save the character into the input buffer */
					connsat_connection_forkpty_p->connection_input_buffer_p->circular_buffer_data_p[
						connsat_connection_forkpty_p->connection_input_buffer_p->calculated_buffer_wrapped_write_start_ptr] = input_buffer[0];

					CONNSAT_TRACE;
#else
					/* LOOK FOR ^D character by character */
					if (input_buffer[0] == 0x04) {

// @@@@@@@
// flush all buffer distribution first
// freeze buffers if something is stuck

#ifdef FORKPTY_STREAM_READ_BUFFER_DUMP
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
#ifdef FORKPTY_STREAM_READ_BUFFER_DUMP
						printf("\tREAD-CNTRL-D, SWITCHED_TO_MENU_MODE_STOP_PROCESSING_INPUT_STDIN!!!!!!!!!!!!!%s",
							newline_string_p);
#endif

						break;

					} else {

						CONNSAT_TRACE;

						connsat_connection_forkpty_p->connection_input_buffer_p->circular_buffer_data_p[
							connsat_connection_forkpty_p->connection_input_buffer_p->calculated_buffer_wrapped_write_start_ptr] = input_buffer[0];

						CONNSAT_TRACE;

					}
#endif

#if 0
#ifdef MASTER_DEBUG_FLAG_01_LOCAL_ECHO
#ifdef FORKPTY_STREAM_READ_BUFFER_DUMP
// @@@@@@@
// DEBUG_ECHO_CHARS
					printf("\tREAD-BYTES-LOCAL-DEBUG-ECHO: %d, \"", r);
					for (i = 0; i < r; ++i) {

						if (
							connsat_connection_forkpty_p->connection_input_buffer_p->circular_buffer_data_p[
							connsat_connection_forkpty_p->connection_input_buffer_p->calculated_buffer_wrapped_write_start_ptr + i] == 0) {

							putc('\\', stdout);
							putc('0', stdout);

						} else if (
							connsat_connection_forkpty_p->connection_input_buffer_p->circular_buffer_data_p[
							connsat_connection_forkpty_p->connection_input_buffer_p->calculated_buffer_wrapped_write_start_ptr + i] == 10) {

							putc('\\', stdout);
							putc('n', stdout);

						} else if (
							connsat_connection_forkpty_p->connection_input_buffer_p->circular_buffer_data_p[
							connsat_connection_forkpty_p->connection_input_buffer_p->calculated_buffer_wrapped_write_start_ptr + i] == 13) {

							putc('\\', stdout);
							putc('r', stdout);

						} else {
							putc(
								connsat_connection_forkpty_p->connection_input_buffer_p->circular_buffer_data_p[
								connsat_connection_forkpty_p->connection_input_buffer_p->calculated_buffer_wrapped_write_start_ptr + i],
							stdout);
						}
					}
					printf("\"%s",
						newline_string_p);
#endif
#endif
#endif
					CONNSAT_TRACE;

					connsat_connection_forkpty_p->connection_input_buffer_p->circular_buffer_used_bytes += r;

					/* recalc buffer */
					r = circular_buffer_calculations(connsat_connection_forkpty_p->connection_input_buffer_p);
						/* this recalculates: connsat_connection_forkpty_p->connection_input_buffer_p->wrapped_write_start_ptr */

					if (r != 0) {
#ifdef FORKPTY_STREAM_READ_BUFFER_DUMP
						printf("\tcircular_buffer_calculations() returned: %d, \"", r);
#endif
						return -1;	/* error */
					}
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
connsat_connection_forkpty_output_write (
	struct connsat_connection_s *connsat_connection_forkpty_p,
	int menu_mode_flag,
	int raw_mode_flag)
{

	char *newline_string_p;
	int max_readable_chunk; /* source_read_buffer */
//	int max_writeable_chunk; /* destination_write_buffer */
	int r;
	int r2;
	time_t current_time_stamp;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	if (connsat_connection_forkpty_p->connection_type != CONNSAT_CONNECTION_TYPE_FORKPTY) {

		printf("Error: %s-%d, connection is not CONNECTION_TYPE_FORKPTY<%s", __FILE__, __LINE__,
			newline_string_p);

		return -1;
	}

	// circular_buffer_calculations(connsat_connection_forkpty_p->connection_input_buffer_p);
	circular_buffer_calculations(connsat_connection_forkpty_p->connection_output_buffer_p);

	/* read_the_output_buffer source_read_buffer */
	max_readable_chunk = connsat_connection_forkpty_p->connection_input_buffer_p->calculated_buffer_max_readable_chunk;
	/* destination_write_buffer */
	// max_writeable_chunk = connsat_connection_distribution_list_p->connection_output_buffer_p->max_writeable_chunk;

	/* check POLLOUT to avoid blocking on write() */

#ifdef FORKPTY_STREAM_WRITE_BUFFER_DUMP
	printf("\t\tOUTPUT_BUFFER_MAX_READABLE_CHUNK: %s-%d, max_readable_chunk==%d%s",
		__FILE__, __LINE__,
		max_readable_chunk,
		newline_string_p);
#endif

	/* write output_fd from connsat_connection_distribution_list_p->connection_output_buffer_p */
	r = circular_buffer_stream_write(
		connsat_connection_forkpty_p->connection_output_buffer_p,
		connsat_connection_forkpty_p->connection_output_fd
		);

#ifdef KEEP_ALIVE_TIME_FEATURE	/* timestamp/watchdog update: channel write */

	r2 = get_timestamp_seconds(&current_time_stamp);

	if (connsat_connection_forkpty_spawn_time_stamp == 0) {

		/* last keepalive time seconds */
		connsat_connection_forkpty_spawn_time_stamp = current_time_stamp;

	} else {

#ifdef KEEP_ALIVE_TIME_TRACE_RW
		/*
		** Data read into the small input buffer from remote connection and distributed.
		** Reset timestamp like a watchdog timer...
		*/
		printf("%sREFRESH-KEEP-ALIVE-TIME-FOR-WRITE-DATA: %s-%d, spawn_time_stamp==%ld, current_time_stamp==%ld%s",
			newline_string_p,
			__FILE__, __LINE__,
			connsat_connection_forkpty_spawn_time_stamp,
			current_time_stamp,
			newline_string_p);
#endif

		/* update the wait timer */
		connsat_connection_forkpty_spawn_time_stamp = current_time_stamp;
	}

#endif /* KEEP_ALIVE_TIME_FEATURE */

	return 0; /* ok */
}

int
connsat_connection_forkpty_buffer_distribution (
	struct connsat_connection_s *connsat_connection_forkpty_p,
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
	time_t current_time_stamp;
	int alive_feature_wait_time;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	r = get_timestamp_seconds(&current_time_stamp);

#ifdef KEEP_ALIVE_TIME_FEATURE	/* timestamp/watchdog update: channel buffer distribution */

	if (connsat_options_all.alive_feature_enable_flag != 0) {

		/* pick up user configured value */
		alive_feature_wait_time = connsat_options_all.alive_feature_wait_time;

		if (alive_feature_wait_time <= 0) {

			/* set it to a sane value if it got corrupted */
			alive_feature_wait_time = 15;
		}
	}

	if (connsat_connection_forkpty_spawn_time_stamp == 0) {

		/* last keepalive time seconds */
		connsat_connection_forkpty_spawn_time_stamp = current_time_stamp;

	} else if ( current_time_stamp >= (connsat_connection_forkpty_spawn_time_stamp + alive_feature_wait_time) ) {

#ifdef KEEP_ALIVE_TIME_TRACE
		printf("%sKEEP-ALIVE-TIME: %s-%d, spawn_time_stamp==%ld, current_time_stamp==%ld%s",
			newline_string_p,
			__FILE__, __LINE__,
			connsat_connection_forkpty_spawn_time_stamp,
			current_time_stamp,
			newline_string_p);
#endif

		if (connsat_options_all.alive_feature_enable_flag != 0) {

			if (connsat_options_all.alive_feature_command_string_p != NULL) {
				// connsat_connection_forkpty_p
				connsat_buffer_printf(connsat_connection_forkpty_p->connection_output_buffer_p,
					"%s\n",
					connsat_options_all.alive_feature_command_string_p
					);
			}

		}

		/* update the wait timer */
		connsat_connection_forkpty_spawn_time_stamp = current_time_stamp;
	}
#endif /* KEEP_ALIVE_TIME_FEATURE */

	if (connsat_connection_forkpty_p->connection_type != CONNSAT_CONNECTION_TYPE_FORKPTY) {

		printf("Error: %s-%d, connection is not CONNSAT_CONNECTION_TYPE_FORKPTY<%s", __FILE__, __LINE__,
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
		r = connsat_connection_forkpty_buffer_distribution_worker (
			connsat_connection_forkpty_p,
			worker_distribution_mode_flag,
/* SET */		&buffer_distribution_max_readable_chunk, /* calculated input, source_read_buffer */
/* SET */		&buffer_distribution_max_writeable_chunk, /* calculated output, destination_write_buffer */
			raw_mode_flag);

		if (r != 0) {

			printf("Error: %s-%d, buffer transfer calculation error for CONNECTION_TYPE_FORKPTY<%s",
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

#ifdef MASTER_DEBUG_FLAG_01
			printf("\t\t\t%s: %s-%d, CONNECTION_TYPE_FORKPTY-ALL_BUFFERS >>%d:%d<<%s",
				"TRANSFER_CALCULATIONS",
				__FILE__, __LINE__,
				buffer_distribution_max_readable_chunk,
				buffer_distribution_max_writeable_chunk,
				newline_string_p);
#endif
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
				r = connsat_connection_forkpty_buffer_distribution_worker (
					connsat_connection_forkpty_p,
					worker_distribution_mode_flag,
/* USE */				&buffer_distribution_max_readable_chunk, /* calculated input, source_read_buffer */
/* USE */				&buffer_distribution_max_writeable_chunk, /* calculated input, destination_write_buffer */
					raw_mode_flag);

				if (r != 0) {

					printf("Error: %s-%d, buffer transfer copy error for CONNECTION_TYPE_FORKPTY<%s",
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
			printf("Error: %s-%d, can't perform buffer transfer without perform calculations first for CONNECTION_TYPE_FORKPTY<%s",
				__FILE__, __LINE__,
				newline_string_p);

			return -4;
		}
	}

	return 0; /* ok */
}

int
connsat_connection_forkpty_buffer_distribution_worker (
	struct connsat_connection_s *connsat_connection_forkpty_p,
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

					break;

				case CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_SERVER:

					break;

				case CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL:

					break;

				case CONNSAT_CONNECTION_TYPE_FORKPTY:

					/* SELF_LOOPBACK */

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
					connsat_connection_forkpty_p->connection_input_buffer_p,
					max_buffer_transfer);

				if (r != 0) {

					printf("Error: %s-%d, circular_buffer_multi_block_transfer() returned %d%s",
						__FILE__, __LINE__,
						r,
						newline_string_p);

					return -11; /* error */
				}

				if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) {

					printf("%s\t\t\t\t\t%s: %s-%d, CONNECTION_TYPE_FORKPTY-smallbuffer[%d/%d] <<<%d>>> TRANSFER-COMPLETE%s",
						newline_string_p,
						to_string_p,
						__FILE__, __LINE__,
						connsat_connection_forkpty_p->connection_input_buffer_p->circular_buffer_used_bytes,
						connsat_connection_forkpty_p->connection_input_buffer_p->circular_buffer_allocated_size,
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
					circular_buffer_calculations(connsat_connection_forkpty_p->connection_input_buffer_p);

					/* destination of copy buffer */
					circular_buffer_calculations(connsat_connection_distribution_list_p->connection_output_buffer_p);

					/* source_read_buffer */
					max_readable_chunk = connsat_connection_forkpty_p->connection_input_buffer_p->calculated_buffer_max_readable_chunk;
					/* destination_write_buffer */
					max_writeable_chunk = connsat_connection_distribution_list_p->connection_output_buffer_p->calculated_buffer_max_writeable_chunk;

#ifdef MASTER_DEBUG_FLAG_01_MULTI_TRANSFER
					printf("%s-%d, MULTI_TRANSFER: FORKPTY->TERMINAL  *calculation_mode_max_readable_chunk_p==%d, *calculation_mode_max_writeable_chunk_p==%d, name==%s, dest==%s%s",
						__FILE__, __LINE__,
						*calculation_mode_max_readable_chunk_p,
						*calculation_mode_max_writeable_chunk_p,
						connsat_connection_forkpty_p->name,
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
						printf("%s-%d, MULTI_TRANSFER: FORKPTY->TERMINAL  *calculation_mode_max_readable_chunk_p==%d, *calculation_mode_max_writeable_chunk_p==%d, name==%s, dest==%s%s",
							__FILE__, __LINE__,
							*calculation_mode_max_readable_chunk_p,
							*calculation_mode_max_writeable_chunk_p,
							connsat_connection_forkpty_p->name,
							connsat_connection_distribution_list_p->name,
							newline_string_p);
#endif
						if ( (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE) ||
							(distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) ) {

							printf("%s\t\t\t%s: %s-%d, CONNECTION_TYPE_FORKPTY-smallbuffer[%d/%d] >>%d<<%s",
								newline_string_p,
								from_string_p,
								__FILE__, __LINE__,
								connsat_connection_forkpty_p->connection_input_buffer_p->circular_buffer_used_bytes,
								connsat_connection_forkpty_p->connection_input_buffer_p->circular_buffer_allocated_size,
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
					printf("%s-%d, MULTI_TRANSFER: FORKPTY->TERMINAL  *calculation_mode_max_readable_chunk_p==%d, *calculation_mode_max_writeable_chunk_p==%d, name==%s, dest==%s%s",
						__FILE__, __LINE__,
						*calculation_mode_max_readable_chunk_p,
						*calculation_mode_max_writeable_chunk_p,
						connsat_connection_forkpty_p->name,
						connsat_connection_distribution_list_p->name,
						newline_string_p);
#endif

					if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE) {
						int max_buffer_transfer = *calculation_mode_max_readable_chunk_p;
						if (max_buffer_transfer > *calculation_mode_max_writeable_chunk_p) {
							max_buffer_transfer = *calculation_mode_max_writeable_chunk_p;
						}

#ifdef MASTER_DEBUG_FLAG_01_MULTI_TRANSFER
						printf("%s-%d, MULTI_TRANSFER: FORKPTY->TERMINAL  max_buffer_transfer==%d, *calculation_mode_max_readable_chunk_p==%d, *calculation_mode_max_writeable_chunk_p==%d%s",
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
								connsat_connection_forkpty_p->connection_input_buffer_p,
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

					circular_buffer_calculations(connsat_connection_forkpty_p->connection_input_buffer_p);

					circular_buffer_calculations(connsat_connection_distribution_list_p->connection_output_buffer_p);

					/* source_read_buffer */
					max_readable_chunk = connsat_connection_forkpty_p->connection_input_buffer_p->calculated_buffer_max_readable_chunk;
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

							printf("%s\t\t\t%s: %s-%d, CONNECTION_TYPE_FORKPTY-smallbuffer[%d/%d] >>%d<<%s",
								newline_string_p,
								from_string_p,
								__FILE__, __LINE__,
								connsat_connection_forkpty_p->connection_input_buffer_p->circular_buffer_used_bytes,
								connsat_connection_forkpty_p->connection_input_buffer_p->circular_buffer_allocated_size,
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
						printf("%s-%d, MULTI_TRANSFER: FORKPTY->SIDEBAND_CHANNEL max_buffer_transfer==%d%s",
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
								connsat_connection_forkpty_p->connection_input_buffer_p,
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

					/* SELF_LOOPBACK */

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
					connsat_connection_forkpty_p->connection_input_buffer_p,
					max_buffer_transfer);

				if (r != 0) {

					printf("Error: %s-%d, circular_buffer_multi_block_transfer() returned %d%s",
						__FILE__, __LINE__,
						r,
						newline_string_p);

					return -11; /* error */
				}

				if (distribution_mode_flag & BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE) {

					printf("%s\t\t\t\t\t%s: %s-%d, CONNECTION_TYPE_FORKPTY-smallbuffer[%d/%d] <<<%d>>> TRANSFER-COMPLETE%s",
						newline_string_p,
						to_string_p,
						__FILE__, __LINE__,
						connsat_connection_forkpty_p->connection_input_buffer_p->circular_buffer_used_bytes,
						connsat_connection_forkpty_p->connection_input_buffer_p->circular_buffer_allocated_size,
						max_buffer_transfer,
						newline_string_p);
				}
			}
		}
	}

	return 0; /* ok */
}

