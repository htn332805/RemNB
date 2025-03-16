
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
extern char **environ;

#include "connsat_main.h"
#include "connsat_connections.h"
#include "connsat_buffer_printf.h"
#include "connsat_connection_init.h"
#include "connsat_terminal_control_menu.h"

#define structure_member_size(type, member) (sizeof( ((type *)0)->member ))

int connsat_connection_buffer_poll_loop_buffer_distribution(
	int menu_mode_flag,
	int raw_mode_flag);

int connsat_connections_pollable_stream_count(
	int menu_mode_flag,
	int raw_mode_flag,
	int *pollable_connection_count_p);

struct connsat_connection_s *connsat_connection_list_head_p = NULL;

int connsat_connection_global_change_detected_flag = 0;

int connsat_connection_global_menu_mode_state_flag = 0;
int connsat_connection_global_menu_mode_request_exit_flag = 0;

struct pollfd *poll_fds_array_p = NULL;
struct connsat_connection_fds_array_map_s *poll_fds_array_map_p = NULL;
int poll_fds_array_malloc_size = 0;
int poll_fds_array_nfds = 0;
int poll_fds_array_index = -1;
int poll_fds_array_map_malloc_size = 0;

/* allocate and add a new connection */
int
connsat_connection_new (
	char *new_connection_name_p,
	connsat_connection_type_t connsat_connection_type,
	int input_buffer_size,
	int output_buffer_size,
	int raw_mode_flag,

        int (*connection_input_open_p)(
		struct connsat_connection_s *connsat_connection_p,
		int menu_mode_flag,
		int raw_mode_flag
		),						// Pointer to function that opens this connection stream
        int (*connection_input_close_p)(
		struct connsat_connection_s *connsat_connection_p,
		int menu_mode_flag,
		int raw_mode_flag
		),						// Pointer to function that closes this connection stream
        int (*connection_output_open_p)(
		struct connsat_connection_s *connsat_connection_p,
		int menu_mode_flag,
		int raw_mode_flag
		),						// Pointer to function that opens this connection stream
        int (*connection_output_close_p)(
		struct connsat_connection_s *connsat_connection_p,
		int menu_mode_flag,
		int raw_mode_flag
		),						// Pointer to function that closes this connection stream
        int (*connection_input_read_p)(
		struct connsat_connection_s *connsat_connection_p,
		int menu_mode_flag,
		int raw_mode_flag
		),						// Pointer to function that reads this connection stream
        int (*connection_output_write_p)(
		struct connsat_connection_s *connsat_connection_p,
		int menu_mode_flag,
		int raw_mode_flag
		),						// Pointer to function that write this connection stream
        int (*connection_buffer_distribution_p)(
		struct connsat_connection_s *connsat_connection_p,
		int distribution_mode_flag,

		int *calculation_mode_max_readable_chunk_p,
		int *calculation_mode_max_writeable_chunk_p,

		int raw_mode_flag
		),						// Pointer to function that performs buffer distribution
	struct connsat_connection_s **connsat_connection_new_pp)
{
	int r;
	struct connsat_connection_s *connsat_connection_list_prev_p;
	struct connsat_connection_s *connsat_connection_list_new_p;
	struct connsat_connection_s *connsat_connection_list_p;

	/* allocate space for the new connection context */
	connsat_connection_list_new_p = (struct connsat_connection_s *) malloc(sizeof(struct connsat_connection_s));

	if (connsat_connection_list_new_p == NULL) {

		if (connsat_connection_new_pp != NULL) {
			*connsat_connection_new_pp = (struct connsat_connection_s *) NULL;
		}

		return -1; /* out of memory */
	}

	/* clear space new connection context memory space */
	memset(connsat_connection_list_new_p, 0, sizeof(struct connsat_connection_s));

	strncpy(&connsat_connection_list_new_p->name[0], new_connection_name_p, sizeof( ((struct connsat_connection_s *)0)->name) );
	connsat_connection_list_new_p->name[sizeof( ((struct connsat_connection_s *)0)->name) -1] = '\0';

	switch (connsat_connection_type) {
		case CONNSAT_CONNECTION_TYPE_TERMINAL:
		case CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_SERVER:
		case CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL:
		case CONNSAT_CONNECTION_TYPE_FORKPTY:
		case CONNSAT_CONNECTION_TYPE_CONTROL_MENU:
		case CONNSAT_CONNECTION_TYPE_SESSION_LOG:
		case CONNSAT_CONNECTION_TYPE_CAPTURE_LOG:
		case CONNSAT_CONNECTION_TYPE_NONE:
			connsat_connection_list_new_p->connection_type = connsat_connection_type;
			break;

		default:
			connsat_connection_list_new_p->connection_type = CONNSAT_CONNECTION_TYPE_NONE;
			break;
	}

	connsat_connection_list_new_p->connection_input_state = CONNSAT_CONNECTION_STATE_CLOSED;

	connsat_connection_list_new_p->connection_output_state = CONNSAT_CONNECTION_STATE_CLOSED;

//@@@@@@@
#if 0
	connsat_connection_list_new_p->connection_poll_input_fd_index = -1;
#endif
	connsat_connection_list_new_p->connection_input_fd = -1;
	//@@@connsat_connection_list_new_p->connection_input_poll_events = 0;
	connsat_connection_list_new_p->connection_input_fd_poll_flag = 0;

//@@@@@@@
#if 0
	connsat_connection_list_new_p->connection_poll_output_fd_index = -1;
#endif
	connsat_connection_list_new_p->connection_output_fd = -1;
	//@@@connsat_connection_list_new_p->connection_output_poll_events = 0;
	connsat_connection_list_new_p->connection_output_fd_poll_flag = 0;

	if (input_buffer_size > 0) {

		r = circular_buffer_init(
			"input_buffer",
			input_buffer_size,
			&connsat_connection_list_new_p->connection_input_buffer_p);

		if (r != 0) { 
// @@@@@@@
			printf("Error creating input_buffer\n");
			exit(-1);
		}

		connsat_connection_list_new_p->connection_allocated_input_buffer_size = input_buffer_size;

		r = circular_buffer_calculations(connsat_connection_list_new_p->connection_input_buffer_p);
	}

	if (output_buffer_size > 0) {

		r = circular_buffer_init(
			"output_buffer",
			output_buffer_size,
			&connsat_connection_list_new_p->connection_output_buffer_p);

		if (r != 0) { 
// @@@@@@@
			printf("Error creating output_buffer\n");
			exit(-1);
		}

		connsat_connection_list_new_p->connection_allocated_output_buffer_size = output_buffer_size;

		r = circular_buffer_calculations(connsat_connection_list_new_p->connection_output_buffer_p);

	}
#ifdef MASTER_DEBUG_FLAG_01_CB_ALLOCATIONS
	r = connsat_connection_dump_one(
		connsat_connection_list_new_p,
		CONNSAT_CONNECTION_DUMP_ONE_FLAGS_FULL,
		raw_mode_flag);
#endif

        if (connection_input_open_p != NULL) {
		connsat_connection_list_new_p->connection_input_open_p = connection_input_open_p;
	}

        if (connection_output_open_p != NULL) {
		connsat_connection_list_new_p->connection_output_open_p = connection_output_open_p;
	}

        if (connection_input_close_p != NULL) {
		connsat_connection_list_new_p->connection_input_close_p = connection_input_close_p;
	}

        if (connection_output_close_p != NULL) {
		connsat_connection_list_new_p->connection_output_close_p = connection_output_close_p;
	}

        if (connection_input_read_p != NULL) {
		connsat_connection_list_new_p->connection_input_read_p = connection_input_read_p;
	}

        if (connection_output_write_p != NULL) {
		connsat_connection_list_new_p->connection_output_write_p = connection_output_write_p;
	}

        if (connection_buffer_distribution_p != NULL) {
		connsat_connection_list_new_p->connection_buffer_distribution_p = connection_buffer_distribution_p;
	}

	/* link the new new connection context memory space into the main list */
	if (connsat_connection_list_head_p == NULL) {
		/* empty list, we're adding the first connection */

		// everything was zero'd already:
		// connsat_connection_list_new_p->connsat_connection_prev_p = NULL:
		// connsat_connection_list_new_p->connsat_connection_next_p = NULL:

		connsat_connection_list_head_p = connsat_connection_list_new_p;

	} else {

		/* find the last item in the list */
		for (connsat_connection_list_p = connsat_connection_list_head_p;
			connsat_connection_list_p->connsat_connection_next_p != NULL ;
			connsat_connection_list_p = connsat_connection_list_p->connsat_connection_next_p) {
		}

		/* now let's link the newly allocated connection context to the end of the list */
		connsat_connection_list_p->connsat_connection_next_p = connsat_connection_list_new_p;

		/* link the new connection context back to the last item */
		connsat_connection_list_new_p->connsat_connection_prev_p = connsat_connection_list_p;
	}

	/* return a pointer to the new connection context */
	if (connsat_connection_new_pp != NULL) {
		*connsat_connection_new_pp = connsat_connection_list_new_p;
	}

	return 0; /* ok */
}

int
connsat_connection_open (
	struct connsat_connection_s *connsat_connection_p,
	int menu_mode_flag,
	int raw_mode_flag)
{
	int new_input_fd;
	int new_output_fd;
	int ret_value;

	if (connsat_connection_p == NULL) {
		return -1; /* error, null connection context pointer */
	}

	if (connsat_connection_p->connection_input_open_p == NULL) {
		return -2; /* error, null connection context open function pointer */
	}

	if (connsat_connection_p->connection_output_open_p == NULL) {
		return -3; /* error, null connection context open function pointer */
	}

	if (connsat_connection_p->connection_input_state == CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {
		return -4; /* error, stream already open */
	}

	if (connsat_connection_p->connection_input_state == CONNSAT_CONNECTION_STATE_OPEN_INACTIVE) {
		return -5; /* error, stream already open */
	}

	if (connsat_connection_p->connection_output_state == CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {
		return -4; /* error, stream already open */
	}

	if (connsat_connection_p->connection_output_state == CONNSAT_CONNECTION_STATE_OPEN_INACTIVE) {
		return -5; /* error, stream already open */
	}

	ret_value = connsat_connection_p->connection_input_open_p(
		connsat_connection_p,
		menu_mode_flag,
		raw_mode_flag);

	if (ret_value < 0) {
		return -6; /* error, can't open stream */
	} else {
//@@@@@@@ UNINITIALIZED FD
		connsat_connection_p->connection_input_fd = new_input_fd;
		connsat_connection_p->connection_input_state = CONNSAT_CONNECTION_STATE_OPEN_INACTIVE;
	}

	ret_value = connsat_connection_p->connection_output_open_p(
		connsat_connection_p,
		menu_mode_flag,
		raw_mode_flag);

	if (ret_value < 0) {
		return -7; /* error, can't open stream */
	} else {
//@@@@@@@ UNINITIALIZED FD
		connsat_connection_p->connection_output_fd = new_output_fd;
		connsat_connection_p->connection_output_state = CONNSAT_CONNECTION_STATE_OPEN_INACTIVE;
	}

	return 0; /* ok */
}

int
connsat_connection_close (
	struct connsat_connection_s *connsat_connection_p,
	int menu_mode_flag,
	int raw_mode_flag)
{
	int ret_value;

	if (connsat_connection_p == NULL) {
		return -1; /* error, null connection context pointer */
	}

	if (connsat_connection_p->connection_input_open_p == NULL) {
		return -2; /* error, null connection context open function pointer */
	}

	if (connsat_connection_p->connection_output_open_p == NULL) {
		return -3; /* error, null connection context open function pointer */
	}

	if (connsat_connection_p->connection_input_state != CONNSAT_CONNECTION_STATE_OPEN_ACTIVE
		&& connsat_connection_p->connection_input_state != CONNSAT_CONNECTION_STATE_OPEN_INACTIVE) {

		return -4; /* error, stream not open */
	}

	if (connsat_connection_p->connection_output_state != CONNSAT_CONNECTION_STATE_OPEN_ACTIVE
		&& connsat_connection_p->connection_output_state != CONNSAT_CONNECTION_STATE_OPEN_INACTIVE) {

		return -4; /* error, stream not open */
	}

	ret_value = connsat_connection_p->connection_input_close_p(
		connsat_connection_p,
		menu_mode_flag,
		raw_mode_flag);

	if (ret_value < 0) {
		return -6; /* error, can't close stream */
	} else {
		connsat_connection_p->connection_input_fd = -1;
		connsat_connection_p->connection_input_state = CONNSAT_CONNECTION_STATE_CLOSED;
	}

// @@@
	ret_value = connsat_connection_p->connection_output_close_p(
		connsat_connection_p,
		menu_mode_flag,
		raw_mode_flag);

	if (ret_value < 0) {
		return -6; /* error, can't close stream */
	} else {
		connsat_connection_p->connection_output_fd = -1;
		connsat_connection_p->connection_output_state = CONNSAT_CONNECTION_STATE_CLOSED;
	}
	return 0; /* ok */
}

// @@@ LOCK READING UNTIL READ BUFFER IS FLUSHED */
// Poll input stream, if room in all big buffers, fill small buffer with one read load.
// NOTE: Do not read anything extra into small buffer until it's distributed and cleared again.
//	small_buffer_busy_flag
// Distribute copies of small buffer to all big buffers in
// the channel distribution list.
// After distribution is done, clear small buffer.
// @@@@@@@ NOT USED YET!!!
int
connsat_connection_read (
	struct connsat_connection_s *connsat_connection_p,
	int menu_mode_flag,
	int raw_mode_flag)
{
	int new_fd;
	int ret_value;

	if (connsat_connection_p == NULL) {
		return -1; /* error, null connection context pointer */
	}

	if (connsat_connection_p->connection_input_read_p == NULL) {
		return -2; /* error, null connection context read function pointer */
	}

	if (connsat_connection_p->connection_input_state != CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {
		return -3; /* error, stream is not open */
	}

	if (connsat_connection_p->connection_input_state != CONNSAT_CONNECTION_STATE_OPEN_INACTIVE) {
		return -4; /* error, stream is not open */
	}

#if 0
	if (connsat_connection_p->connection_output_state != CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {
		return -3; /* error, stream is not open */
	}

	if (connsat_connection_p->connection_output_state != CONNSAT_CONNECTION_STATE_OPEN_INACTIVE) {
		return -4; /* error, stream is not open */
	}
#endif

// @@@@@@@
#if 0
	ret_value = connsat_connection_p->connection_input_read_p(
		connsat_connection_p->connection_input_fd,
		connsat_connection_p->connection_input_buffer_p,
		connsat_connection_p->connection_input_buffer_size
		);

	if (ret_value < 0) {
		return -5; /* error, stream read error */
	} else {
//		connsat_connection_p->connection_input_buffer_index_start = 0;
//		connsat_connection_p->connection_input_buffer_index_end = ret_value;
	}
#endif

	return 0; /* ok */
}

// @@@ WRITE BUFFER CAN WRAP */
// @@@ WRITE/FLUSH AS MUCH AS POSSIBLE, IF NON-BLOCKING, MOVE BUFFER POINTERS */
// Poll output stream, if room flush all or part of big buffer.
// Buffer full configuration option, block or drop.
// @@@@@@@ NOT USED YET!!!
int
connsat_connection_write (
	struct connsat_connection_s *connsat_connection_p,
	int menu_mode_flag,
	int raw_mode_flag)
{
	int ret_value;

	if (connsat_connection_p == NULL) {
		return -1; /* error, null connection context pointer */
	}

	if (connsat_connection_p->connection_output_write_p == NULL) {
		return -2; /* error, null connection context read function pointer */
	}

	if (connsat_connection_p->connection_output_state != CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {
		return -3; /* error, stream is not open */
	}

	if (connsat_connection_p->connection_output_state != CONNSAT_CONNECTION_STATE_OPEN_INACTIVE) {
		return -4; /* error, stream is not open */
	}

// @@@@@@@
#if 0
	ret_value = connsat_connection_p->connection_output_write_p(
		connsat_connection_p->connection_output_fd,
		connsat_connection_p->connection_output_buffer_p,
		connsat_connection_p->connection_output_buffer_size
		);

	if (ret_value < 0) {
		return -5; /* error, stream write error */
	} else {
//		connsat_connection_p->connection_output_buffer_index_start = 0;
//		connsat_connection_p->connection_output_buffer_index_end = ret_value;
	}
#endif

	return 0; /* ok */
}

int
connsat_connection_poll_loop (
	int menu_mode_flag,
	int raw_mode_flag)
{
	struct connsat_connection_s *connsat_connection_list_p;
	// struct connsat_connection_fds_array_map_s *connsat_connection_list_map_p;
	int connection_count;
	int pollable_connection_count;
	char *newline_string_p;
	int poll_status;
	int poll_timeout;
	int input_loop_count;
	int x;
	int menu_mode_request_exit_flag;
#ifdef MASTER_DEBUG_FLAG_01_LIMIT_LOOP
	int debug_max_loops;
#endif
	int r;
	int main_poll_loop_enable_flag;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}
 
#ifdef MASTER_DEBUG_FLAG_01_POLL_LOOP_START
	printf("CONNECTION_MAIN_POLL_LOOP:%s%s",
		newline_string_p,
		newline_string_p);
#endif

#ifdef MASTER_DEBUG_FLAG_01_LIMIT_LOOP
	// debug_max_loops = 10;
	debug_max_loops = 5;
#endif
	input_loop_count = 0;
	main_poll_loop_enable_flag = 1;

	while (main_poll_loop_enable_flag) { /* MAIN-POLL_LOOP-while */

		CONNSAT_TRACE;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// get open pollable stream count:

		/* start at the begining of the list */
		connsat_connection_list_p = connsat_connection_list_head_p;

		connection_count = 0;
		pollable_connection_count = 0;

		r = connsat_connections_pollable_stream_count(
			menu_mode_flag,
			raw_mode_flag,
			&pollable_connection_count);

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_01
		printf("\t%s-%d: ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^%s",
			__FILE__, __LINE__,
			newline_string_p);
		printf("%s", newline_string_p);

		printf("\tPOLLABLE_CONNECTIONS_COUNT: %s-%d, CONNECTIONS==%d%s",
			__FILE__, __LINE__,
			pollable_connection_count,
			newline_string_p);
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// allocate fds_array for open pollable stream count:

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

		if (poll_fds_array_map_p != NULL) {

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_01
			printf("\tFREE_POLLABLE_CONNECTIONS_MAP: %s-%d,%s",
				__FILE__, __LINE__,
				newline_string_p);
#endif

			free(poll_fds_array_map_p);

			poll_fds_array_map_p = NULL;
		}

		poll_fds_array_nfds = pollable_connection_count;
		poll_fds_array_malloc_size = sizeof(struct pollfd) * (pollable_connection_count + 1);
		poll_fds_array_index = -1;
		poll_fds_array_map_malloc_size = sizeof(struct connsat_connection_fds_array_map_s) * (pollable_connection_count + 1);

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_01
		printf("\t\tPOLLABLE_MALLOC: %s-%d, poll_fds_array_nfds==%d, poll_fds_array_malloc_size==%d, poll_fds_array_map_malloc_size==%d%s",
			__FILE__, __LINE__,
			poll_fds_array_nfds,
			poll_fds_array_malloc_size,
			poll_fds_array_map_malloc_size,
			newline_string_p);
#endif

		/* malloc pollfd_array */

		poll_fds_array_p = malloc( poll_fds_array_malloc_size );

		if (poll_fds_array_p == NULL) {

			printf("%3d: POLL-MALLOC-ERROR%s",
				input_loop_count,
				newline_string_p);

			return -1;
		}

		/* malloc poll_fds_array_map_p */

		poll_fds_array_map_p = malloc( poll_fds_array_map_malloc_size );

		if (poll_fds_array_map_p == NULL) {

			printf("%3d: POLL-MAP-MALLOC-ERROR%s",
				input_loop_count,
				newline_string_p);

			return -1;
		}

		/* clear newly allocated pollfd_array space */
		memset(poll_fds_array_p, 0, poll_fds_array_malloc_size);

		/* clear newly allocated poll_fds_array_map_p space */
		memset(poll_fds_array_map_p, 0, poll_fds_array_map_malloc_size);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

		/* build poll array of active connections */
		connsat_connection_list_p = connsat_connection_list_head_p;

		CONNSAT_TRACE;
		while (connsat_connection_list_p) {	/* build-poll-fds */

			CONNSAT_TRACE;

			if ( (connsat_connection_list_p->connection_input_state ==
				CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) &&
				(connsat_connection_list_p->connection_output_state ==
				CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) ) {

				if (connsat_connection_list_p->connection_input_fd >= 0) {

					/* INPUT_STREAM_FD */

					if (connsat_connection_list_p->connection_output_fd >= 0) {

						/* OUTPUT_STREAM_FD */

						if (connsat_connection_list_p->connection_input_fd ==
							connsat_connection_list_p->connection_output_fd) {

// MAJOR CONNECTION CASE 1:
#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_04
							printf("\t%s-%d: ####### POLLING_MAJOR_CONNECTION_CASE_1:%s",
								__FILE__, __LINE__,
								newline_string_p);
#endif

							/* INPUT_STREAM_FD */
							/* OUTPUT_STREAM_FD */

							/* ONE SHARED STREAM FD, POLLIN and POLLOUT */

							++poll_fds_array_index;	/* increment to next avabile slot index */

							/* this connection has a single pollable stream for input and output */

							poll_fds_array_p[poll_fds_array_index].fd =
								connsat_connection_list_p->connection_input_fd;

							/*
							** With this we can quickly map poll_fds_array_p array elements
							** to connsat_connection_list_p items.
							*/
							poll_fds_array_map_p[poll_fds_array_index].connsat_connection_p =
								connsat_connection_list_p;

							poll_fds_array_map_p[poll_fds_array_index].fds_array_map_flags = 0;

							if (connsat_connection_list_p->connection_input_fd_poll_flag) {
								poll_fds_array_map_p[poll_fds_array_index].fds_array_map_flags |=
									FDS_ARRAY_MAP_FLAGS_IS_STREAM_INPUT;
							}
							if (connsat_connection_list_p->connection_output_fd_poll_flag) {
								poll_fds_array_map_p[poll_fds_array_index].fds_array_map_flags |=
									FDS_ARRAY_MAP_FLAGS_IS_STREAM_OUTPUT;
							}


							/* does the connection output buffer have anything to write? */
							circular_buffer_calculations(connsat_connection_list_p->connection_output_buffer_p);

							if (connsat_connection_list_p->connection_output_buffer_p->calculated_buffer_max_readable_chunk > 0) {

								poll_fds_array_p[poll_fds_array_index].events = 0;

								if (connsat_connection_list_p->connection_input_fd_poll_flag != 0) {
									poll_fds_array_p[poll_fds_array_index].events |= POLLIN;
								}
								if (connsat_connection_list_p->connection_output_fd_poll_flag != 0) {
									poll_fds_array_p[poll_fds_array_index].events |= POLLOUT;
								}

							} else {

								poll_fds_array_p[poll_fds_array_index].events = 0;

								if (connsat_connection_list_p->connection_input_fd_poll_flag != 0) {
									poll_fds_array_p[poll_fds_array_index].events |= POLLIN;
								}
							}

							poll_fds_array_p[poll_fds_array_index].revents = 0;

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_01
							printf("\t\t\t\t%s-%d: poll_fds_array_p[%d]: %s-%d, poll_fds_array_nfds: %d%s",
								__FILE__, __LINE__,
								poll_fds_array_index,
								__FILE__, __LINE__,
								poll_fds_array_nfds,
								newline_string_p);
#endif
#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_01
							printf("\t\t\t\tpoll_fds_array_p[%d]: %s-%d, poll_fds_array_index: %d, fd==%d, connsat_connection_p==%p, map_flags==0x%04x, %s%s",
								poll_fds_array_index,
								__FILE__, __LINE__,
								poll_fds_array_index,
								poll_fds_array_p[poll_fds_array_index].fd,
								poll_fds_array_map_p[poll_fds_array_index].connsat_connection_p,
								poll_fds_array_map_p[poll_fds_array_index].fds_array_map_flags,
								poll_fds_array_map_p[poll_fds_array_index].connsat_connection_p->name,
								newline_string_p);
#endif


						} else {

// MAJOR CONNECTION CASE 2:

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_04
							printf("\t%s-%d: ####### POLLING_MAJOR_CONNECTION_CASE_2:%s",
								__FILE__, __LINE__,
								newline_string_p);
#endif

							/* INPUT_STREAM_FD */
							/* OUTPUT_STREAM_FD */

							/* TWO SEPARATE STREAM FDs, one POLLIN and one POLLOUT */

							++poll_fds_array_index;	/* increment to next avabile slot index */

							poll_fds_array_p[poll_fds_array_index].fd =
								connsat_connection_list_p->connection_input_fd;

							/*
							** With this we can quickly map poll_fds_array_p array elements
							** to connsat_connection_list_p items.
							*/
							poll_fds_array_map_p[poll_fds_array_index].connsat_connection_p =
								connsat_connection_list_p;

							if (connsat_connection_list_p->connection_input_fd_poll_flag) {
								poll_fds_array_map_p[poll_fds_array_index].fds_array_map_flags |=
									FDS_ARRAY_MAP_FLAGS_IS_STREAM_INPUT;
							}

							poll_fds_array_p[poll_fds_array_index].events = 0;

							if (connsat_connection_list_p->connection_input_fd_poll_flag != 0) {
								poll_fds_array_p[poll_fds_array_index].events |= POLLIN;
							}

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_01
							printf("\t\t\t\tpoll_fds_array_p[%d]: %s-%d, poll_fds_array_index: %d, fd==%d, connsat_connection_p==%p, map_flags==0x%04x, %s%s",
								poll_fds_array_index,
								__FILE__, __LINE__,
								poll_fds_array_index,
								poll_fds_array_p[poll_fds_array_index].fd,
								poll_fds_array_map_p[poll_fds_array_index].connsat_connection_p,
								poll_fds_array_map_p[poll_fds_array_index].fds_array_map_flags,
								poll_fds_array_map_p[poll_fds_array_index].connsat_connection_p->name,
								newline_string_p);
#endif



							++poll_fds_array_index;	/* increment to next avabile slot index */

							poll_fds_array_p[poll_fds_array_index].fd =
								connsat_connection_list_p->connection_output_fd;

							/*
							** With this we can quickly map poll_fds_array_p array elements
							** to connsat_connection_list_p items.
							*/
							poll_fds_array_map_p[poll_fds_array_index].connsat_connection_p =
								connsat_connection_list_p;

							if (connsat_connection_list_p->connection_output_fd_poll_flag) {
								poll_fds_array_map_p[poll_fds_array_index].fds_array_map_flags |=
									FDS_ARRAY_MAP_FLAGS_IS_STREAM_OUTPUT;
							}

							/* does the connection output buffer have anything to write? */
							circular_buffer_calculations(connsat_connection_list_p->connection_output_buffer_p);

							if (connsat_connection_list_p->connection_output_buffer_p->calculated_buffer_max_readable_chunk > 0) {

								poll_fds_array_p[poll_fds_array_index].events = 0;

								if (connsat_connection_list_p->connection_output_fd_poll_flag != 0) {
									poll_fds_array_p[poll_fds_array_index].events |= POLLOUT;
								}
							} else {
								poll_fds_array_p[poll_fds_array_index].events = 0;
							}

							poll_fds_array_p[poll_fds_array_index].revents = 0;

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_01
							printf("\t\t\t\tpoll_fds_array_p[%d]: %s-%d, poll_fds_array_index: %d, fd==%d, connsat_connection_p==%p, map_flags==0x%04x, %s%s",
								poll_fds_array_index,
								__FILE__, __LINE__,
								poll_fds_array_index,
								poll_fds_array_p[poll_fds_array_index].fd,
								poll_fds_array_map_p[poll_fds_array_index].connsat_connection_p,
								poll_fds_array_map_p[poll_fds_array_index].fds_array_map_flags,
								poll_fds_array_map_p[poll_fds_array_index].connsat_connection_p->name,
								newline_string_p);
#endif

						}

					} else {

// MAJOR CONNECTION CASE 3:

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_04
						printf("\t%s-%d: ####### POLLING_MAJOR_CONNECTION_CASE_3:%s",
							__FILE__, __LINE__,
							newline_string_p);
#endif

						/* OUTPUT_BUFFER_ONLY */
						/* INPUT_STREAM_FD */

						/* ONE INPUT ONLY STREAM FD, POLLIN */

						++poll_fds_array_index;	/* increment to next avabile slot index */

						/* this connection has a single pollable stream for input only */

						poll_fds_array_p[poll_fds_array_index].fd =
							connsat_connection_list_p->connection_input_fd;

						/*
						** With this we can quickly map poll_fds_array_p array elements
						** to connsat_connection_list_p items.
						*/
						poll_fds_array_map_p[poll_fds_array_index].connsat_connection_p =
							connsat_connection_list_p;

						poll_fds_array_map_p[poll_fds_array_index].fds_array_map_flags = 0;

						if (connsat_connection_list_p->connection_input_fd_poll_flag) {
							poll_fds_array_map_p[poll_fds_array_index].fds_array_map_flags |=
								FDS_ARRAY_MAP_FLAGS_IS_STREAM_INPUT;
						}

						poll_fds_array_p[poll_fds_array_index].events = 0;

						if (connsat_connection_list_p->connection_input_fd_poll_flag != 0) {

							poll_fds_array_p[poll_fds_array_index].events |= POLLIN;
						}

						poll_fds_array_p[poll_fds_array_index].revents = 0;

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_01
						printf("\t\t\t\tpoll_fds_array_p[%d]: %s-%d, poll_fds_array_nfds: %d%s",
							poll_fds_array_index,
							__FILE__, __LINE__,
							poll_fds_array_nfds,
							newline_string_p);
#endif
#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_01
						printf("\t\t\t\tpoll_fds_array_p[%d]: %s-%d, poll_fds_array_index: %d, fd==%d, connsat_connection_p==%p, map_flags==0x%04x, %s%s",
							poll_fds_array_index,
							__FILE__, __LINE__,
							poll_fds_array_index,
							poll_fds_array_p[poll_fds_array_index].fd,
							poll_fds_array_map_p[poll_fds_array_index].connsat_connection_p,
							poll_fds_array_map_p[poll_fds_array_index].fds_array_map_flags,
							poll_fds_array_map_p[poll_fds_array_index].connsat_connection_p->name,
							newline_string_p);
#endif

					}

				} else {
					/* INPUT_BUFFER_ONLY */
					/* OUTPUT_STREAM_FD */

					if (connsat_connection_list_p->connection_output_fd >= 0) {

// MAJOR CONNECTION CASE 4:

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_04
						printf("\t%s-%d: ####### POLLING_MAJOR_CONNECTION_CASE_4:%s",
							__FILE__, __LINE__,
							newline_string_p);
#endif

						/* OUTPUT_STREAM_FD */


						/* ONE OUTPUT ONLY STREAM FD, POLLOUT */

						++poll_fds_array_index;	/* increment to next avabile slot index */

						/* this connection has a single pollable stream for output only */

						poll_fds_array_p[poll_fds_array_index].fd =
							connsat_connection_list_p->connection_output_fd;

						/*
						** With this we can quickly map poll_fds_array_p array elements
						** to connsat_connection_list_p items.
						*/
						poll_fds_array_map_p[poll_fds_array_index].connsat_connection_p =
							connsat_connection_list_p;

						poll_fds_array_map_p[poll_fds_array_index].fds_array_map_flags = 0;

						if (connsat_connection_list_p->connection_output_fd_poll_flag) {
							poll_fds_array_map_p[poll_fds_array_index].fds_array_map_flags |=
								FDS_ARRAY_MAP_FLAGS_IS_STREAM_OUTPUT;
						}

						/* does the connection output buffer have anything to write? */
						circular_buffer_calculations(connsat_connection_list_p->connection_output_buffer_p);

						if (connsat_connection_list_p->connection_output_buffer_p->calculated_buffer_max_readable_chunk > 0) {

							poll_fds_array_p[poll_fds_array_index].events = 0;

							if (connsat_connection_list_p->connection_output_fd_poll_flag != 0) {
								poll_fds_array_p[poll_fds_array_index].events |= POLLOUT;
							}

						} else {

							poll_fds_array_p[poll_fds_array_index].events = 0;
						}

						poll_fds_array_p[poll_fds_array_index].revents = 0;

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_01
						printf("\t\t\t\tpoll_fds_array_p[%d]: %s-%d, poll_fds_array_index: %d, fd==%d, connsat_connection_p==%p, map_flags==0x%04x, %s%s",
							poll_fds_array_index,
							__FILE__, __LINE__,
							poll_fds_array_index,
							poll_fds_array_p[poll_fds_array_index].fd,
							poll_fds_array_map_p[poll_fds_array_index].connsat_connection_p,
							poll_fds_array_map_p[poll_fds_array_index].fds_array_map_flags,
							poll_fds_array_map_p[poll_fds_array_index].connsat_connection_p->name,
							newline_string_p);
#endif


					} else {

// MAJOR CONNECTION CASE 5:

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_04
						printf("\t%s-%d: ####### POLLING_MAJOR_CONNECTION_CASE_5:%s",
							__FILE__, __LINE__,
							newline_string_p);
#endif

						/* INPUT_BUFFER_ONLY */
						/* OUTPUT_BUFFER_ONLY */

// int connection_input_buffer_enable_flag;	/* turning this on enables writes into this buffer */
// int connection_output_buffer_enable_flag;	/* turning this on enables writes into this buffer */
//
// int connection_input_buffer_distribution_flag;	/* turning this on enables distribution from this input buffer to other output buffers */
// int connection_output_buffer_distribution_flag;	/* turning this on enables receiving buffer distributions from other input buffers */

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_01
						printf("\t\t\t\tpoll_fds_array_p[%d]: %s-%d, poll_fds_array_index: %d, fd==%d, connsat_connection_p==%p, map_flags==0x%04x, %s%s",
							poll_fds_array_index,
							__FILE__, __LINE__,
							poll_fds_array_index,
							poll_fds_array_p[poll_fds_array_index].fd,
							poll_fds_array_map_p[poll_fds_array_index].connsat_connection_p,
							poll_fds_array_map_p[poll_fds_array_index].fds_array_map_flags,
							poll_fds_array_map_p[poll_fds_array_index].connsat_connection_p->name,
							newline_string_p);
#endif


					}

				}

			}

			connsat_connection_list_p = connsat_connection_list_p->connsat_connection_next_p;

		} /* end-while, build-poll-fds */

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

		/* the poll_fds_array_p[] array has been set up for use with the poll() function call */

		CONNSAT_TRACE;

		while (1) {	/* poll-only-loop */

			CONNSAT_TRACE;

			++input_loop_count;

//@@@@@@@
			/* if the buffers are empty, make the poll_timeout large */
			/* if the buffers-to-buffers are not empty, make the poll_timeout non-blocking-zero */
			poll_timeout = 250;

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_01
			// @@@ printf("%3d: BLOCKING-POLL: nfds==%d, timeout==%d\r\n", input_loop_count, poll_fds_array_index, poll_timeout);
			printf("%3d: %s-%d: BLOCKING-POLL: nfds==%d, timeout==%d%s",
				input_loop_count,
				__FILE__, __LINE__,
				poll_fds_array_nfds,
				poll_timeout,
				newline_string_p);
#endif

#ifdef MASTER_DEBUG_FLAG_01_POLL_SLEEP /* debug, get time to type on the keyboard when testing */

			printf("%3d: SLEEP-POLL(type-input-here): %s-%d%s",
				input_loop_count,
				__FILE__, __LINE__,
				newline_string_p);

			sleep(3);
#endif

			/* actual poll */

//@@@@@@@
// check input + output poll streams count, if zero skip poll() call

			// @@@ poll_status = poll(poll_fds_array_p, poll_fds_array_index, poll_timeout);
			CONNSAT_TRACE;

#ifdef MASTER_DEBUG_FLAG_01_POLL_ACTIVE
			r = connsat_connection_dump_active_buffers(raw_mode_flag);
#endif

CONNSAT_TRACE_DUMP;

			poll_status = poll(poll_fds_array_p, poll_fds_array_nfds, poll_timeout);

			if ( poll_status < 0 ) {

				CONNSAT_TRACE;

				perror("POLL-ERROR: ");

			} else if ( poll_status == 0 ) {

				CONNSAT_TRACE;

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

				CONNSAT_TRACE;

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

					CONNSAT_TRACE;

					if (poll_fds_array_p[x].events & POLLIN) {

						/* Didn't check for POLLHUP yet, because there may be
						** buffered data still present in the stream.
						** Below, we won't write to the stream if a POLLHUP
						** gets detected */

						CONNSAT_TRACE;

						/* read connections input stream into its input buffer */
#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_01
						printf("\t%s-%d: fds[%d]: fd==%d: POLL-REQUESTED-EVENTS-POLLIN:\t",
							__FILE__, __LINE__,
							x,
							poll_fds_array_p[x].fd);
#endif

						if (poll_fds_array_p[x].revents & POLLIN) {

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_01
							printf("\t%s-%d: fds[%d]: fd==%d: POLL-RECEIVED-EVENTS-POLLIN: %s",
								__FILE__, __LINE__,
								x,
								poll_fds_array_p[x].fd,
								newline_string_p);
#endif

							//@@@@@@@
							//@@@@@@@ DO_THE_READ!!!  //CALL_CHANNEL_STREAM_READ
							//@@@@@@@

							if (poll_fds_array_map_p[x].connsat_connection_p != NULL) {

								if (poll_fds_array_map_p[x].connsat_connection_p->connection_input_read_p != NULL) {

									/*
									** This will look at the active distribution connections that this
									** channel may do distribution output to.  It will determine that
									** maximum transer size that could be done from the small input buffer
									** and to all large output buffers. It picks the largest common
									** transfer size that can fit everything. Then it will perform a
									** channel read up to that size possible.
									*/
									// poll_fds_array_map_p[poll_fds_array_index].fds_array_map_flags =
									// FDS_ARRAY_MAP_FLAGS_IS_STREAM_INPUT |
									// FDS_ARRAY_MAP_FLAGS_IS_STREAM_OUTPUT;
									(*poll_fds_array_map_p[x].connsat_connection_p->connection_input_read_p)(
										poll_fds_array_map_p[x].connsat_connection_p,
										menu_mode_flag,
										raw_mode_flag
										);
								} else {
									return -3;	/* error */
								}
							} else {
								return -4;		/* error */
							}

//@@@@@@@ DEBUG_READ
#if 0 /* debug */
int r;
char temp_buffer[1024];
r = read(poll_fds_array_p[x].fd, temp_buffer, sizeof temp_buffer -1 );
temp_buffer[r] = '\0';
printf("READ %d: \"%s\"%s",
r,
temp_buffer,
newline_string_p);
#endif

						} else {
#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_01
							printf("%s",
								newline_string_p);
#endif
						}
					}

					if (poll_fds_array_p[x].revents & POLLHUP) {

						/* HANGUP: POLLHUP Detected, let's close the connection */

						CONNSAT_TRACE;

						// poll_fds_array_map_p[poll_fds_array_index].connsat_connection_p

						if (poll_fds_array_map_p[x].connsat_connection_p != NULL) {

							if (poll_fds_array_map_p[x].connsat_connection_p->connection_input_close_p != NULL) {

								/* close the input side of the connection */
								(*poll_fds_array_map_p[x].connsat_connection_p->connection_input_close_p)(
									poll_fds_array_map_p[x].connsat_connection_p,
									menu_mode_flag,
									raw_mode_flag
									);

								printf("\t%s-%d: fds[%d]: fd==%d: POLL-POLLHUP-DETECTED-CONNECTION-CLOSED: %s\t",
									__FILE__, __LINE__,
									x,
									poll_fds_array_p[x].fd,
									poll_fds_array_map_p[x].connsat_connection_p->name);
								printf("%s",
									newline_string_p);
							} else {

								return -3;	/* error */
							}

							if (poll_fds_array_map_p[x].connsat_connection_p->connection_output_close_p != NULL) {

								/* close the output side of the connection */
								(*poll_fds_array_map_p[x].connsat_connection_p->connection_output_close_p)(
									poll_fds_array_map_p[x].connsat_connection_p,
									menu_mode_flag,
									raw_mode_flag
									);
							} else {
								return -4;		/* error */
							}

							/* clear future events */
							poll_fds_array_p[x].events = 0;

							/* clear received events */
							poll_fds_array_p[x].revents = 0;

							/* notify the poll logic to rebuild the poll list */
							connsat_connection_global_change_detected_flag = 1;

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_02
							printf("\t%s-%d: fds[%d]: fd==%d: POLL-POLLHUP-DETECTED-CONNECTION-DISABLED:\t",
								__FILE__, __LINE__,
								x,
								poll_fds_array_p[x].fd);
#endif
						}

					} else {

						CONNSAT_TRACE;

						if (poll_fds_array_p[x].events & POLLOUT) {

							CONNSAT_TRACE;

							/* write connections output stream from its output buffer */
#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_02
							printf("\t%s-%d: fds[%d]: fd==%d: POLL-REQUESTED-EVENTS-POLLOUT:\t",
								__FILE__, __LINE__,
								x,
								poll_fds_array_p[x].fd);
#endif

							if (poll_fds_array_p[x].revents & POLLOUT) {

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_02
								printf("\t%s-%d: fds[%d]: fd==%d: POLL-RECEIVED-EVENTS-POLLOUT: %s",
									__FILE__, __LINE__,
									x,
									poll_fds_array_p[x].fd,
									newline_string_p);
#endif

								//@@@@@@@
								//@@@@@@@ DO_THE_WRITE!!!  //CALL_CHANEL_STREAM_WRITE
								//@@@@@@@

								if (poll_fds_array_map_p[x].connsat_connection_p != NULL) {

									if (poll_fds_array_map_p[x].connsat_connection_p->connection_output_write_p != NULL) {

										/*
										** This will look at the channel output and perform the maximum
										** write supported by the steam fd.
										*/
										// poll_fds_array_map_p[poll_fds_array_index].fds_array_map_flags =
										// FDS_ARRAY_MAP_FLAGS_IS_STREAM_INPUT |
										// FDS_ARRAY_MAP_FLAGS_IS_STREAM_OUTPUT;
										CONNSAT_TRACE;

#if 0 /* CONNSAT_TEMP_DEBUG */
printf("NAME-%s-%d: %s%s",
__FILE__, __LINE__,
poll_fds_array_map_p[x].connsat_connection_p->name,
newline_string_p);
usleep(250000);
#endif
										(*poll_fds_array_map_p[x].connsat_connection_p->connection_output_write_p)(
											poll_fds_array_map_p[x].connsat_connection_p,
											menu_mode_flag,
											raw_mode_flag
											);

										CONNSAT_TRACE;

									} else {
										return -3;	/* error */
									}
								} else {
									return -4;		/* error */
								}

							} else {
#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_02
								printf("%s",
									newline_string_p);
#endif
							}
						}
					}

#ifdef MASTER_DEBUG_FLAG_02
					printf("\t= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = %s",
						newline_string_p);
#endif
				}
			}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
			/* Process pending buffer to buffer transfers for all buffers */

			CONNSAT_TRACE;


#ifdef MASTER_DEBUG_FLAG_01
			printf("ALL_BUFFER_DISTRIBUTION:%s",
				newline_string_p);
#endif

			/* menu_mode may have changed */
			r = connsat_connection_global_menu_mode_flag_state_get(
				&menu_mode_flag,
				raw_mode_flag);

#ifdef MASTER_DEBUG_FLAG_01_POLL_BUFFER_DISTRIBUTION_ENABLE

CONNSAT_TRACE_DUMP;

			/* run all buffer distribution */
			r = connsat_connection_buffer_poll_loop_buffer_distribution(
				menu_mode_flag,
				raw_mode_flag);
#endif

			r = connsat_connection_global_menu_mode_flag_state_get(
				&menu_mode_flag,
				raw_mode_flag);

			r = connsat_connection_global_menu_mode_flag_request_exit_get(
				&menu_mode_request_exit_flag,
				raw_mode_flag);

#ifdef MASTER_DEBUG_FLAG_01_CONTROL_MENU_EXIT
			printf("\t%s-%d: TOP/MENU_MODE_FLAG==%d, menu_mode_request_exit_flag==%d !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!%s",
				__FILE__, __LINE__,
				menu_mode_flag,
				menu_mode_request_exit_flag,
				newline_string_p);
#endif

#ifdef MASTER_DEBUG_FLAG_01_POLL_CONTROL_MENU_ENABLE
			/* continue this loop to keep polling if there are no connection changes */
			// return_value = -1;	/* EOF */
			// return_value = 0;	/* Double ^D, exit */
			// return_value = 1;	/* Done with control menu, continue */
			// return_value = 2;	/* Stay in control menu, continue */
			r = connsat_connection_buffer_poll_loop_control_menu (
				menu_mode_flag,
				raw_mode_flag);

			if (r == -1) {

				r = connsat_connection_global_menu_mode_flag_state_get(
					&menu_mode_flag,
					raw_mode_flag);

				if (menu_mode_flag) {

#ifdef MASTER_DEBUG_FLAG_01_CONTROL_MENU_EXIT
					printf("\tREAD-EOF, SWITCH_TO_NORMAL_MODE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!%s",
						newline_string_p);
#endif

					/* exit menu mode */
					menu_mode_flag = 0;

					r = connsat_connection_global_menu_mode_flag_state_set(
						menu_mode_flag,
						raw_mode_flag);

#if 0
					r = connsat_connection_global_menu_mode_flag_state_get(
						&menu_mode_flag,
						raw_mode_flag);
#endif
				}

			} else if (r == 0) {

				/* gracefully terminate the program */
				main_poll_loop_enable_flag = 0;

#ifdef MASTER_DEBUG_FLAG_01_CONTROL_MENU_EXIT
				printf("\t%s-%d: r==%d, READ-DOUBLE-CNTRL-D, GRACEFULLY-EXIT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!%s",
					__FILE__, __LINE__,
					0,
					newline_string_p);
#endif

			} else if (r == 1) {

				r = connsat_connection_global_menu_mode_flag_state_get(
					&menu_mode_flag,
					raw_mode_flag);

				if (menu_mode_flag) {

#ifdef MASTER_DEBUG_FLAG_01_CONTROL_MENU_EXIT
					printf("\t%s-%d: r==%d, READ-EXIT, SWITCH_TO_NORMAL_MODE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!%s",
						__FILE__, __LINE__,
						1,
						newline_string_p);
#endif

					/* exit menu mode */
					menu_mode_flag = 0;

					r = connsat_connection_global_menu_mode_flag_state_set(
						menu_mode_flag,
						raw_mode_flag);

#if 0
					r = connsat_connection_global_menu_mode_flag_state_get(
						&menu_mode_flag,
						raw_mode_flag);
#endif
				}

			} else if (r == 2) {

				r = connsat_connection_global_menu_mode_flag_state_get(
					&menu_mode_flag,
					raw_mode_flag);

#ifdef MASTER_DEBUG_FLAG_01_CONTROL_MENU_EXIT
				if (menu_mode_flag) {

					/* Stay in control menu, continue */
					printf("\t%s-%d: r==%d, STAY_IN_MENU_MODE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!%s",
						__FILE__, __LINE__,
						2,
						newline_string_p);
				}
#endif

			} else {

// #ifdef MASTER_DEBUG_FLAG_01
				printf("CONTROL_MENU_RETURNED UNEXPECTED VALUE: %d in %s-%d%s",
					r,
					__FILE__, __LINE__,
					newline_string_p);
// #endif

				/* gracefully terminate the program */
				main_poll_loop_enable_flag = 0;

			}
#endif

//@@@@@@@
			/*
			** If the connection list has changed, break out of this loop
			** to rebuild poll array.
			*/
			if (connsat_connection_global_change_detected_flag) {

				/* POLLHUP was detected, or other change */
				connsat_connection_global_change_detected_flag = 0;

				/* Dropping below to MAIN-POLL_LOOP-end-while, will rebuild the poll list */

				break;
			}

			break; /* break anyway for now */

		} /* end-while, poll-only-loop */

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//@@@@@@@
#ifdef MASTER_DEBUG_FLAG_01_LIMIT_LOOP
		/* break to end polling scan */
		if (debug_max_loops <= 0) {
			break;
		} else {

			printf("DEBUG-SCAN-LOOP: %d%s",
				debug_max_loops,
				newline_string_p);

			--debug_max_loops;
		}
#endif

	} /* MAIN-POLL_LOOP-end-while */

	CONNSAT_TRACE;

	return 0; /* ok */
}

int
connsat_connection_dump_one (
	struct connsat_connection_s *connsat_connection_list_p,
	int output_flags,
	int raw_mode_flag)
{
	// int connection_count;
	char string_type[128];
	char string_input_state[128];
	char string_output_state[128];
	// int total_used_input_buffer_size;
	// int total_used_output_buffer_size;
	char *newline_string_p;
	int circular_buffer_allocated_size;
	int circular_buffer_used_bytes;
	int r;
	int x;
	char *view_buffer_p;
	int view_buffer_size;
	int menu_mode_flag;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	if (1) {

		switch (connsat_connection_list_p->connection_type) {
			default:
			case CONNSAT_CONNECTION_TYPE_NONE:
				sprintf(string_type, "CONNECTION_TYPE_NONE");
				break;
			case CONNSAT_CONNECTION_TYPE_TERMINAL:
				sprintf(string_type, "CONNECTION_TYPE_TERMINAL");
				break;
			case CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_SERVER:
				sprintf(string_type, "CONNECTION_TYPE_SIDEBAND_SERVER");
				break;
			case CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL:
				sprintf(string_type, "CONNECTION_TYPE_SIDEBAND_CHANNEL");
				break;
			case CONNSAT_CONNECTION_TYPE_FORKPTY:
				sprintf(string_type, "CONNECTION_TYPE_FORKPTY");
				break;
			case CONNSAT_CONNECTION_TYPE_CONTROL_MENU:
				sprintf(string_type, "CONNECTION_TYPE_MENU");
				break;
			case CONNSAT_CONNECTION_TYPE_SESSION_LOG:
				sprintf(string_type, "CONNECTION_TYPE_SESSION_LOG");
				break;
			case CONNSAT_CONNECTION_TYPE_CAPTURE_LOG:
				sprintf(string_type, "CONNECTION_TYPE_CAPTURE_LOG");
				break;
		}

		switch (connsat_connection_list_p->connection_input_state) {
			default:
			case CONNSAT_CONNECTION_STATE_NONE:
				sprintf(string_input_state, "CONNECTION_STATE_NONE");
				break;
			case CONNSAT_CONNECTION_STATE_OPEN_ACTIVE:
				sprintf(string_input_state, "CONNECTION_STATE_OPEN_ACTIVE");
				break;
			case CONNSAT_CONNECTION_STATE_OPEN_INACTIVE:
				sprintf(string_input_state, "CONNECTION_STATE_OPEN_INACTIVE");
				break;
			case CONNSAT_CONNECTION_STATE_CLOSED:
				sprintf(string_input_state, "CONNECTION_STATE_CLOSED");
				break;
		}

		switch (connsat_connection_list_p->connection_output_state) {
			default:
			case CONNSAT_CONNECTION_STATE_NONE:
				sprintf(string_output_state, "CONNECTION_STATE_NONE");
				break;
			case CONNSAT_CONNECTION_STATE_OPEN_ACTIVE:
				sprintf(string_output_state, "CONNECTION_STATE_OPEN_ACTIVE");
				break;
			case CONNSAT_CONNECTION_STATE_OPEN_INACTIVE:
				sprintf(string_output_state, "CONNECTION_STATE_OPEN_INACTIVE");
				break;
			case CONNSAT_CONNECTION_STATE_CLOSED:
				sprintf(string_output_state, "CONNECTION_STATE_CLOSED");
				break;
		}

#if 0
		/* override menu_mode_flag */
		r = connsat_connection_global_menu_mode_flag_state_get(&menu_mode_flag, raw_mode_flag);

		printf("Connection-%02d: \"%s\", %s-%d, menu_mode_flag==%d%s",
			connection_count,
			connsat_connection_list_p->name,
			__FILE__, __LINE__,
			menu_mode_flag,
			newline_string_p);
#endif

		if (output_flags & CONNSAT_CONNECTION_DUMP_ONE_FLAGS_TYPE) {
			printf("\ttype: %d, \"%s\"%s",
				connsat_connection_list_p->connection_type,
				string_type,
				newline_string_p);
		}

		if (output_flags & CONNSAT_CONNECTION_DUMP_ONE_FLAGS_STATE) {
			printf("\tinput-state: %d, \"%s\"%s",
				connsat_connection_list_p->connection_input_state,
				string_input_state,
				newline_string_p);

			printf("\toutput-state: %d, \"%s\"%s",
				connsat_connection_list_p->connection_output_state,
				string_output_state,
				newline_string_p);
		}


#if 0 /* redundant */
		printf("\tinput_fd_poll_flag: %d %s",
			connsat_connection_list_p->connection_input_fd_poll_flag,
			newline_string_p);
#endif

		if (output_flags & CONNSAT_CONNECTION_DUMP_ONE_FLAGS_BUFFER_ENABLE) {
			printf("\tinput_buffer_enable_flag: %d %s",
				connsat_connection_list_p->connection_input_buffer_enable_flag,
				newline_string_p);
		}

		if (output_flags & CONNSAT_CONNECTION_DUMP_ONE_FLAGS_BUFFER_DIST) {
			printf("\tinput_buffer_distribution_flag: %d %s",
				connsat_connection_list_p->connection_input_buffer_distribution_flag,
				newline_string_p);
		}


#if 0 /* redundant */
		printf("\toutput_fd_poll_flag: %d %s",
			connsat_connection_list_p->connection_output_fd_poll_flag,
			newline_string_p);
#endif

		if (output_flags & CONNSAT_CONNECTION_DUMP_ONE_FLAGS_BUFFER_ENABLE) {
			printf("\toutput_buffer_enable_flag: %d %s",
				connsat_connection_list_p->connection_output_buffer_enable_flag,
				newline_string_p);
		}

		if (output_flags & CONNSAT_CONNECTION_DUMP_ONE_FLAGS_BUFFER_DIST) {
			printf("\toutput_buffer_distribution_flag: %d %s",
				connsat_connection_list_p->connection_output_buffer_distribution_flag,
				newline_string_p);
		}

		if (output_flags & CONNSAT_CONNECTION_DUMP_ONE_FLAGS_LOOPBACK) {
			printf("\tconnection_distribution_self_loopback_enable_flag: %d %s",
				connsat_connection_list_p->connection_distribution_self_loopback_enable_flag,
				newline_string_p);
		}




//@@@@@@@
#if 0
		printf("\tpoll_input_fd_index: %d, ",
			connsat_connection_list_p->connection_poll_input_fd_index);
#endif

		if (output_flags & CONNSAT_CONNECTION_DUMP_ONE_FLAGS_TRACE) {
			printf("\t%s-%d: ",
				__FILE__, __LINE__);
		}

		if (output_flags & CONNSAT_CONNECTION_DUMP_ONE_FLAGS_FD) {
			printf("input_fd: %d, ",
				connsat_connection_list_p->connection_input_fd);
		}

		if (output_flags & CONNSAT_CONNECTION_DUMP_ONE_FLAGS_POLL) {
			printf("\tinput_fd_poll_flag: %d, ",
				connsat_connection_list_p->connection_input_fd_poll_flag);
		}

		if (connsat_connection_list_p == NULL) {
			circular_buffer_allocated_size = 0;
		} else if (connsat_connection_list_p->connection_input_buffer_p == NULL) {
			circular_buffer_allocated_size = 0;
		} else {
			circular_buffer_allocated_size = connsat_connection_list_p->connection_input_buffer_p->circular_buffer_allocated_size;
		}

		if (output_flags & CONNSAT_CONNECTION_DUMP_ONE_FLAGS_ALLOCATED) {
			printf("\t\tallocated_input_buffer_size: %d/%d",
				circular_buffer_allocated_size,						/* actual allocated size */
				connsat_connection_list_p->connection_allocated_input_buffer_size	/* default size to allocate */
				);
		}

		if (connsat_connection_list_p == NULL) {
			circular_buffer_used_bytes = 0;
		} else if (connsat_connection_list_p->connection_input_buffer_p == NULL) {
			circular_buffer_used_bytes = 0;
		} else {
			circular_buffer_used_bytes = connsat_connection_list_p->connection_input_buffer_p->circular_buffer_used_bytes;
		}

		if (output_flags & CONNSAT_CONNECTION_DUMP_ONE_FLAGS_SIZE) {
			printf("\t\tused_input_buffer_size: %d, ",
				circular_buffer_used_bytes);
		}

		r = circular_buffer_view(
			connsat_connection_list_p->connection_input_buffer_p,
			&view_buffer_p,
			&view_buffer_size
			);

		if (r == 0) {

#ifdef MASTER_DEBUG_FLAG_01_CB_ALLOCATIONS
			printf("<%s>", connsat_connection_list_p->connection_input_buffer_p->buffer_name);
			printf("{%d/", connsat_connection_list_p->connection_input_buffer_p->circular_buffer_used_bytes);
			printf("%d}", connsat_connection_list_p->connection_input_buffer_p->circular_buffer_allocated_size);
			printf("[%d]", connsat_connection_list_p->connection_input_buffer_p->unwrapped_read_start_ptr);
			printf("r=%d:", connsat_connection_list_p->connection_input_buffer_p->calculated_buffer_max_readable_chunk);
			printf("w=%d:", connsat_connection_list_p->connection_input_buffer_p->calculated_buffer_wrapped_write_start_ptr);
			printf("f=%d:", connsat_connection_list_p->connection_input_buffer_p->calculated_buffer_max_writeable_chunk);
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
					printf("%c", view_buffer_p[x]);
				}
			}
			printf("\"");

			free(view_buffer_p);
		}

		printf("%s",
			newline_string_p);

#if 0
		total_used_input_buffer_size +=
			circular_buffer_used_bytes;
#endif


//@@@@@@@
#if 0
		printf("\tpoll_output_fd_index: %d, ",
			connsat_connection_list_p->connection_poll_output_fd_index);
#endif

		if (output_flags & CONNSAT_CONNECTION_DUMP_ONE_FLAGS_TRACE) {
			printf("\t%s-%d: ",
				__FILE__, __LINE__);
		}

		if (output_flags & CONNSAT_CONNECTION_DUMP_ONE_FLAGS_FD) {
			printf("\toutput_fd: %d, ",
				connsat_connection_list_p->connection_output_fd);
		}

		if (output_flags & CONNSAT_CONNECTION_DUMP_ONE_FLAGS_POLL) {
			printf("\toutput_fd_poll_flag: %d, ",
				connsat_connection_list_p->connection_output_fd_poll_flag);
		}

		if (connsat_connection_list_p == NULL) {
			circular_buffer_allocated_size = 0;
		} else if (connsat_connection_list_p->connection_output_buffer_p == NULL) {
			circular_buffer_allocated_size = 0;
		} else {
			circular_buffer_allocated_size = connsat_connection_list_p->connection_output_buffer_p->circular_buffer_allocated_size;
		}

		if (output_flags & CONNSAT_CONNECTION_DUMP_ONE_FLAGS_ALLOCATED) {
			printf("\tallocated_output_buffer_size: %d/%d",
				circular_buffer_allocated_size,						/* actual allocated size */
				connsat_connection_list_p->connection_allocated_output_buffer_size	/* default size to allocate */
				);
		}

		if (connsat_connection_list_p == NULL) {
			circular_buffer_used_bytes = 0;
		} else if (connsat_connection_list_p->connection_output_buffer_p == NULL) {
			circular_buffer_used_bytes = 0;
		} else {
			circular_buffer_used_bytes = connsat_connection_list_p->connection_output_buffer_p->circular_buffer_used_bytes;
		}

		if (output_flags & CONNSAT_CONNECTION_DUMP_ONE_FLAGS_SIZE) {
			printf("\tused_output_buffer_size: %d, ",
				circular_buffer_used_bytes);
		}

		r = circular_buffer_view(
			connsat_connection_list_p->connection_output_buffer_p,
			&view_buffer_p,
			&view_buffer_size
			);

		if (r == 0) {

#ifdef MASTER_DEBUG_FLAG_01_CB_ALLOCATIONS
			printf("<%s>", connsat_connection_list_p->connection_output_buffer_p->buffer_name);
			printf("{%d/", connsat_connection_list_p->connection_output_buffer_p->circular_buffer_used_bytes);
			printf("%d}", connsat_connection_list_p->connection_output_buffer_p->circular_buffer_allocated_size);
			printf("[%d]", connsat_connection_list_p->connection_output_buffer_p->unwrapped_read_start_ptr);
			printf("r=%d:", connsat_connection_list_p->connection_output_buffer_p->calculated_buffer_max_readable_chunk);
			printf("w=%d:", connsat_connection_list_p->connection_output_buffer_p->calculated_buffer_wrapped_write_start_ptr);
			printf("f=%d:", connsat_connection_list_p->connection_output_buffer_p->calculated_buffer_max_writeable_chunk);
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
					printf("%c", view_buffer_p[x]);
				}
			}
			printf("\"");

			free(view_buffer_p);
		}

		printf("%s",
			newline_string_p);

		printf("%s",
			newline_string_p);
	}

	return 0; /* ok */
}

int
connsat_connection_list (
	int raw_mode_flag)
{
	struct connsat_connection_s *connsat_connection_list_p;
	int connection_count;
	char string_type[64];
	char string_input_state[64];
	char string_output_state[64];
	int total_used_input_buffer_size;
	int total_used_output_buffer_size;
	char *newline_string_p;
	int circular_buffer_allocated_size;
	int circular_buffer_used_bytes;
	int r;
	int menu_mode_flag;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	connsat_connection_list_p = connsat_connection_list_head_p;

	total_used_input_buffer_size = 0;
	total_used_output_buffer_size = 0;

	connection_count = 0;
	while (connsat_connection_list_p) {

		++connection_count;

		if (connection_count == 1) {
			printf("CONNECTION_BUFFER_LIST:%s%s",
			newline_string_p,
			newline_string_p);
		}

		/* override menu_mode_flag */
		r = connsat_connection_global_menu_mode_flag_state_get(&menu_mode_flag, raw_mode_flag);

		printf("Connection-%02d: \"%s\", %s-%d, menu_mode_flag==%d%s",
			connection_count,
			connsat_connection_list_p->name,
			__FILE__, __LINE__,
			menu_mode_flag,
			newline_string_p);

		r = connsat_connection_dump_one(
			connsat_connection_list_p,
			CONNSAT_CONNECTION_DUMP_ONE_FLAGS_FULL,
			raw_mode_flag);


		if (connsat_connection_list_p == NULL) {
			circular_buffer_allocated_size = 0;
		} else if (connsat_connection_list_p->connection_input_buffer_p == NULL) {
			circular_buffer_allocated_size = 0;
		} else {
			circular_buffer_allocated_size = connsat_connection_list_p->connection_input_buffer_p->circular_buffer_allocated_size;
		}

		if (connsat_connection_list_p == NULL) {
			circular_buffer_used_bytes = 0;
		} else if (connsat_connection_list_p->connection_input_buffer_p == NULL) {
			circular_buffer_used_bytes = 0;
		} else {
			circular_buffer_used_bytes = connsat_connection_list_p->connection_input_buffer_p->circular_buffer_used_bytes;
		}

		total_used_input_buffer_size +=
			circular_buffer_used_bytes;

		if (connsat_connection_list_p == NULL) {
			circular_buffer_allocated_size = 0;
		} else if (connsat_connection_list_p->connection_output_buffer_p == NULL) {
			circular_buffer_allocated_size = 0;
		} else {
			circular_buffer_allocated_size = connsat_connection_list_p->connection_output_buffer_p->circular_buffer_allocated_size;
		}

		if (connsat_connection_list_p == NULL) {
			circular_buffer_used_bytes = 0;
		} else if (connsat_connection_list_p->connection_output_buffer_p == NULL) {
			circular_buffer_used_bytes = 0;
		} else {
			circular_buffer_used_bytes = connsat_connection_list_p->connection_output_buffer_p->circular_buffer_used_bytes;
		}

		total_used_output_buffer_size +=
			circular_buffer_used_bytes;

		connsat_connection_list_p = connsat_connection_list_p->connsat_connection_next_p;
	}

	printf("Connections-Totals:%s",
		newline_string_p);

	printf("\ttotal_used_input_buffer_size: %d, total_used_output_buffer_size: %d%s",
		total_used_input_buffer_size,
		total_used_output_buffer_size,
		newline_string_p);

	printf("%s",
		newline_string_p);

	return 0; /* ok */
}

int
connsat_connection_dump_active_buffers (
	int raw_mode_flag)
{
	struct connsat_connection_s *connsat_connection_list_p;
	int connection_count;
	char string_type[64];
	char string_input_state[64];
	char string_output_state[64];
	int total_used_input_buffer_size;
	int total_used_output_buffer_size;
	char *newline_string_p;
	int circular_buffer_allocated_size;
	int circular_buffer_used_bytes;
	int r;
	int menu_mode_flag;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	connsat_connection_list_p = connsat_connection_list_head_p;

	total_used_input_buffer_size = 0;
	total_used_output_buffer_size = 0;

	connection_count = 0;
	while (connsat_connection_list_p) {

		++connection_count;

		if (connection_count == 1) {
			printf("CONNECTION_DUMP_ACTIVE_BUFFERS:%s%s",
			newline_string_p,
			newline_string_p);
		}

		/* override menu_mode_flag */
		r = connsat_connection_global_menu_mode_flag_state_get(&menu_mode_flag, raw_mode_flag);

		printf("Connection-%02d: \"%s\", %s-%d, menu_mode_flag==%d%s",
			connection_count,
			connsat_connection_list_p->name,
			__FILE__, __LINE__,
			menu_mode_flag,
			newline_string_p);

		r = connsat_connection_dump_one(
			connsat_connection_list_p,
			CONNSAT_CONNECTION_DUMP_ONE_FLAGS_TYPE,
			raw_mode_flag);

#if 0
		if (connsat_connection_list_p == NULL) {
			circular_buffer_allocated_size = 0;
		} else if (connsat_connection_list_p->connection_input_buffer_p == NULL) {
			circular_buffer_allocated_size = 0;
		} else {
			circular_buffer_allocated_size = connsat_connection_list_p->connection_input_buffer_p->circular_buffer_allocated_size;
		}

		if (connsat_connection_list_p == NULL) {
			circular_buffer_used_bytes = 0;
		} else if (connsat_connection_list_p->connection_input_buffer_p == NULL) {
			circular_buffer_used_bytes = 0;
		} else {
			circular_buffer_used_bytes = connsat_connection_list_p->connection_input_buffer_p->circular_buffer_used_bytes;
		}

		total_used_input_buffer_size +=
			circular_buffer_used_bytes;

		if (connsat_connection_list_p == NULL) {
			circular_buffer_allocated_size = 0;
		} else if (connsat_connection_list_p->connection_output_buffer_p == NULL) {
			circular_buffer_allocated_size = 0;
		} else {
			circular_buffer_allocated_size = connsat_connection_list_p->connection_output_buffer_p->circular_buffer_allocated_size;
		}

		if (connsat_connection_list_p == NULL) {
			circular_buffer_used_bytes = 0;
		} else if (connsat_connection_list_p->connection_output_buffer_p == NULL) {
			circular_buffer_used_bytes = 0;
		} else {
			circular_buffer_used_bytes = connsat_connection_list_p->connection_output_buffer_p->circular_buffer_used_bytes;
		}

		total_used_output_buffer_size +=
			circular_buffer_used_bytes;
#endif

		connsat_connection_list_p = connsat_connection_list_p->connsat_connection_next_p;
	}

#if 0
	printf("Connections-Totals:%s",
		newline_string_p);

	printf("\ttotal_used_input_buffer_size: %d, total_used_output_buffer_size: %d%s",
		total_used_input_buffer_size,
		total_used_output_buffer_size,
		newline_string_p);
#endif

	printf("%s",
		newline_string_p);

	return 0; /* ok */
}

int
connsat_connection_buffer_distribution_list (
	int menu_mode_flag,
	int raw_mode_flag)
{
	char *newline_string_p;
	struct connsat_connection_s *connsat_connection_list_p;
	int total_used_input_buffer_size;
	int total_used_output_buffer_size;
	int connection_count;
	int distribution_mode_flag;

	int calculation_mode_max_readable_chunk;
	int calculation_mode_max_writeable_chunk;

	int r;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	connsat_connection_list_p = connsat_connection_list_head_p;

	total_used_input_buffer_size = 0;
	total_used_output_buffer_size = 0;

	connection_count = 0;
	while (connsat_connection_list_p) {

		++connection_count;

		if (connection_count > 1) {
			printf("%s",
			newline_string_p);
		} else {
			printf("CONNECTION_BUFFER_DISTRIBUTION_LIST:%s%s",
			newline_string_p,
			newline_string_p);
		}

		/* override menu_mode_flag */
		r = connsat_connection_global_menu_mode_flag_state_get(&menu_mode_flag, raw_mode_flag);

		printf("Connection-%02d: \"%s\", %s-%d, menu_mode_flag==%d%s",
			connection_count,
			connsat_connection_list_p->name,
			__FILE__, __LINE__,
			menu_mode_flag,
			newline_string_p);

		r = connsat_connection_dump_one(
			connsat_connection_list_p,
			CONNSAT_CONNECTION_DUMP_ONE_FLAGS_FULL,
			raw_mode_flag);

		// CONNSAT_TRACE;

#if 1
		printf("\t\tMENU_MODE_FLAG: %s-%d, menu_mode_flag: %d%s", __FILE__, __LINE__, menu_mode_flag, newline_string_p);
#endif

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		/* enables distribution from this input buffer to other output buffers */
		if (connsat_connection_list_p->connection_input_buffer_distribution_flag) {

			distribution_mode_flag = 0;
			if (menu_mode_flag) {
				distribution_mode_flag |= BUFFER_DISTRIBUTION_MODE_FLAG_MENU_MODE;
			}
			distribution_mode_flag |= BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE;
			distribution_mode_flag |= BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE;

			/*
			** This connection is a CONNSAT_CONNECTION_TYPE_TERMINAL,
			** call connsat_connection_main_terminal_buffer_distribution()
			*/

			calculation_mode_max_readable_chunk == -1;
			calculation_mode_max_writeable_chunk == -1;

			r = (*connsat_connection_list_p->connection_buffer_distribution_p)(
				connsat_connection_list_p,
				distribution_mode_flag,
				&calculation_mode_max_readable_chunk,
				&calculation_mode_max_writeable_chunk,
				raw_mode_flag
				);

#if 1
			printf("\tCONNECTION_BUFFERS_DISTRIBUTION_CALCULATIONS: %s-%d, %s: calculation_mode_max_readable_chunk==%d, calculation_mode_max_writeable_chunk==%d%s",
				__FILE__, __LINE__,
				connsat_connection_list_p->name,
				calculation_mode_max_readable_chunk,
				calculation_mode_max_writeable_chunk,
				newline_string_p);
#endif

		}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		// CONNSAT_TRACE;

		connsat_connection_list_p = connsat_connection_list_p->connsat_connection_next_p;
	}

	return 0; /* ok */
}

int
connsat_connection_buffer_distribution_injection_test (
	int menu_mode_flag,
	int raw_mode_flag)
{
	char *newline_string_p;
	struct connsat_connection_s *connsat_connection_list_p;
	int total_used_input_buffer_size;
	int total_used_output_buffer_size;
	int connection_count;
	int distribution_mode_flag;
	int calculation_mode_max_readable_chunk;
	int calculation_mode_max_writeable_chunk;
	int r;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	connsat_connection_list_p = connsat_connection_list_head_p;

	total_used_input_buffer_size = 0;
	total_used_output_buffer_size = 0;

	connection_count = 0;
	while (connsat_connection_list_p) {

		++connection_count;

		if (connection_count > 1) {
			printf("%s",
				newline_string_p);
		} else {
			printf("%s",
				newline_string_p);

			printf("CONNECTION_BUFFER_DISTRIBUTION_INJECTION:%s%s",
				newline_string_p,
				newline_string_p);
		}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

		if (connsat_connection_list_p->connection_type == CONNSAT_CONNECTION_TYPE_CONTROL_MENU
			|| connsat_connection_list_p->connection_type == CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_SERVER
			) {

			/* skip these channels */

		} else {
#if 1

#ifdef MASTER_DEBUG_FLAG_03
			printf("\tConnection_Injection-%02d: \"%s\", %s-%d%s",
				connection_count,
				connsat_connection_list_p->name,
				__FILE__, __LINE__,
				newline_string_p);
#endif

			//@@@@@@@ BUFFER_FILL_TESTS

			/* enables distribution from this input buffer to other output buffers */
			if (connsat_connection_list_p->connection_input_buffer_distribution_flag) {

				if (connsat_connection_list_p->connection_input_state == CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {

#if 1
					/* for debug stick something in the buffers to see output below */
					connsat_buffer_printf(
						connsat_connection_list_p->connection_input_buffer_p,
						"INPUT_BUFFER_TEST_%s_001%s",
						connsat_connection_list_p->name,
						newline_string_p);
#endif

#if 0 /* debug */
					printf(
						"%s-%d: INPUT_BUFFER_TEST_%s_001%s",
						__FILE__, __LINE__,
						connsat_connection_list_p->name,
						newline_string_p);
#endif
				}
			}

			if (connsat_connection_list_p->connection_output_buffer_distribution_flag) {

				if (connsat_connection_list_p->connection_output_state == CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {

#if 1
					/* for debug stick something in the buffers to see output below */
					connsat_buffer_printf(
						connsat_connection_list_p->connection_output_buffer_p,
						"OUTPUT_BUFFER_TEST_%s_001%s",
						connsat_connection_list_p->name,
						newline_string_p);
#endif

#if 0 /* debug */
					printf(
						"%s-%d: OUTPUT_BUFFER_TEST_%s_001%s",
						__FILE__, __LINE__,
						connsat_connection_list_p->name,
						newline_string_p);
#endif
				}

			}
		}
#endif

		CONNSAT_TRACE;

		connsat_connection_list_p = connsat_connection_list_p->connsat_connection_next_p;
	}

	return 0; /* ok */
}

int
connsat_connection_buffer_poll_loop_buffer_distribution (
	int menu_mode_flag,
	int raw_mode_flag)
{
	char *newline_string_p;
	struct connsat_connection_s *connsat_connection_list_p;
	int total_used_input_buffer_size;
	int total_used_output_buffer_size;
	int connection_count;
	int distribution_mode_flag;

	int calculation_mode_max_readable_chunk;	/* top level */
	int calculation_mode_max_writeable_chunk;	/* top level */

	int r;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	connsat_connection_list_p = connsat_connection_list_head_p;

	total_used_input_buffer_size = 0;
	total_used_output_buffer_size = 0;

	connection_count = 0;
	while (connsat_connection_list_p) {

		// CONNSAT_TRACE;

		/* enables distribution from this input buffer to other output buffers */
		if (connsat_connection_list_p->connection_input_buffer_distribution_flag) {

			distribution_mode_flag = 0;
			if (menu_mode_flag) {
				distribution_mode_flag |= BUFFER_DISTRIBUTION_MODE_FLAG_MENU_MODE;
			}
//			distribution_mode_flag |= BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE;
			distribution_mode_flag |= BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE;
			distribution_mode_flag |= BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE;
			distribution_mode_flag |= BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE;

			/*
			** This connection is a CONNSAT_CONNECTION_TYPE_TERMINAL,
			** call connsat_connection_main_terminal_buffer_distribution()
			*/

			calculation_mode_max_readable_chunk == -1;
			calculation_mode_max_writeable_chunk == -1;

#ifdef MASTER_DEBUG_FLAG_07
printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++%s",
newline_string_p);
printf("START connsat_connection_buffer_poll_loop_buffer_distribution() +++++++++++++++++++++++++%s",
newline_string_p);
printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++%s",
newline_string_p);
printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++%s",
newline_string_p);
#endif

			r = (*connsat_connection_list_p->connection_buffer_distribution_p)(
				connsat_connection_list_p,
				distribution_mode_flag,
				&calculation_mode_max_readable_chunk,
				&calculation_mode_max_writeable_chunk,
				raw_mode_flag
				);

#ifdef MASTER_DEBUG_FLAG_07
printf("%s-%d: TOP_LEVEL_RETURN: calculation_mode_max_readable_chunk==%d, calculation_mode_max_writeable_chunk==%d%s",
__FILE__, __LINE__,
calculation_mode_max_readable_chunk, calculation_mode_max_writeable_chunk,
newline_string_p);

printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++%s",
newline_string_p);
printf("DONE connsat_connection_buffer_poll_loop_buffer_distribution() ++++++++++++++++++++++++++%s",
newline_string_p);
printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++%s",
newline_string_p);
printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++%s",
newline_string_p);
#endif

#if 0
			printf("\tCONNECTION_BUFFERS_DISTRIBUTION_CALCULATIONS: %s-%d, %s: calculation_mode_max_readable_chunk==%d, calculation_mode_max_writeable_chunk==%d%s",
				__FILE__, __LINE__,
				connsat_connection_list_p->name,
				calculation_mode_max_readable_chunk,
				calculation_mode_max_writeable_chunk,
				newline_string_p);
#endif

		}

		// CONNSAT_TRACE;

		connsat_connection_list_p = connsat_connection_list_p->connsat_connection_next_p;
	}

	return 0; /* ok */
}

// return_value = -1;	/* EOF */
// return_value = 0;	/* Double ^D, exit */
// return_value = 1;	/* Done with control menu, continue */
// return_value = 2;	/* Stay in control menu, continue */
int
connsat_connection_buffer_poll_loop_control_menu (
	int menu_mode_flag,
	int raw_mode_flag)
{
	char *newline_string_p;
	struct connsat_connection_s *connsat_connection_list_p;
	int total_used_input_buffer_size;
	int total_used_output_buffer_size;
	int connection_count;
	int distribution_mode_flag;
	int return_value;
	static int buffer_poll_loop_control_menu_entry_count = 0;
	int menu_mode_request_exit_flag;

	int r;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	return_value = 2;	/* DEFAULT: Stay in control menu mode, continue */

	connsat_connection_list_p = connsat_connection_list_head_p;

	while (connsat_connection_list_p) {

		switch (connsat_connection_list_p->connection_type) {

			case CONNSAT_CONNECTION_TYPE_CONTROL_MENU:
				if (connsat_connection_list_p->connection_input_state != CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {
					break;
				}
				if (connsat_connection_list_p->connection_output_state != CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) {
					break;
				}

				if (connsat_connection_list_p->connection_output_buffer_p == NULL) {
					break;
				}

				/* override menu_mode_flag */
				r = connsat_connection_global_menu_mode_flag_state_get(&menu_mode_flag, raw_mode_flag);

				/* find out if the control_menu requested to exit */
				r = connsat_connection_global_menu_mode_flag_request_exit_get(
					&menu_mode_request_exit_flag,
					raw_mode_flag);

#ifdef MASTER_DEBUG_FLAG_01_CONTROL_MENU_EXIT

				printf("\t%s-%d: ####### CONTROL_MENU_USED_BYTES: output_bytes_remaining==%d, input_bytes_remaining==%d, menu_mode_flag==%d, menu_mode_request_exit_flag==%d%s",
					__FILE__, __LINE__,
					connsat_connection_list_p->connection_output_buffer_p->circular_buffer_used_bytes,
					connsat_connection_list_p->connection_input_buffer_p->circular_buffer_used_bytes,
					menu_mode_flag,
					menu_mode_request_exit_flag,
					newline_string_p);
#endif
				if (menu_mode_flag) {

					if (menu_mode_request_exit_flag) {

						if (connsat_connection_list_p->connection_output_buffer_p->circular_buffer_used_bytes == 0) {

							if (connsat_connection_list_p->connection_input_buffer_p->circular_buffer_used_bytes == 0) {

#ifdef MASTER_DEBUG_FLAG_01_CONTROL_MENU_EXIT
								printf("\t%s-%d: ####### EXIT:::: CONTROL_MENU_USED_BYTES: output_bytes_remaining==%d, input_bytes_remaining==%d, menu_mode_flag==%d, menu_mode_request_exit_flag==%d%s",
									__FILE__, __LINE__,
									connsat_connection_list_p->connection_output_buffer_p->circular_buffer_used_bytes,
									connsat_connection_list_p->connection_input_buffer_p->circular_buffer_used_bytes,
									menu_mode_flag,
									menu_mode_request_exit_flag,
									newline_string_p);
#endif
								return_value = 1;	/* Done with control menu, continue */
							}
						}
					}
				}

				if (connsat_connection_list_p->connection_output_buffer_p->circular_buffer_used_bytes > 0) {
					/* control_menu output buffer (input) has menu input data to process */

					/* override menu_mode_flag */
					r = connsat_connection_global_menu_mode_flag_state_get(&menu_mode_flag, raw_mode_flag);

#ifdef MASTER_DEBUG_FLAG_01_CONTROL_MENU_INPUT
					printf("\t####### CONTROL_MENU_HAS_INPUT_AVAILABLE: %d, menu_mode_flag==%d%s",
						connsat_connection_list_p->connection_output_buffer_p->circular_buffer_used_bytes,
						menu_mode_flag,
						newline_string_p);
#endif

#if 1
					/* find out if the control_menu requested to exit */
					r = connsat_connection_global_menu_mode_flag_request_exit_get(
						&menu_mode_request_exit_flag,
						raw_mode_flag);

#ifdef MASTER_DEBUG_FLAG_01_CONTROL_MENU_EXIT
					printf("\t%s-%d: ####### CONTROL_MENU_USED_BYTES: output_bytes_remaining==%d, input_bytes_remaining==%d, menu_mode_flag==%d, menu_mode_request_exit_flag==%d%s",
						__FILE__, __LINE__,
						connsat_connection_list_p->connection_output_buffer_p->circular_buffer_used_bytes,
						connsat_connection_list_p->connection_input_buffer_p->circular_buffer_used_bytes,
						menu_mode_flag,
						menu_mode_request_exit_flag,
						newline_string_p);
#endif

					if (menu_mode_flag) {

						/* find out if the control_menu requested to exit */
						r = connsat_connection_global_menu_mode_flag_request_exit_get(
							&menu_mode_request_exit_flag,
							raw_mode_flag);

#ifdef MASTER_DEBUG_FLAG_01_CONTROL_MENU_EXIT
						printf("\t%s-%d: ####### CONTROL_MENU_EXIT_REQUEST_IS==%d%s",
							__FILE__, __LINE__,
							menu_mode_request_exit_flag,
							newline_string_p);
#endif

						if (1 || menu_mode_request_exit_flag == 0) {

							// return_value = -1;	/* EOF */
							// return_value = 0;	/* Double ^D, exit */
							// return_value = 1;	/* Done with control menu, continue */
							// return_value = 2;	/* Stay in control menu mode, continue */
							r = connsat_control_menu(
								connsat_connection_list_p->connection_output_buffer_p,
								connsat_connection_list_p->connection_input_buffer_p,
								(char * (*)[]) environ,		//env_p
								0);				// debug_flag

#ifdef MASTER_DEBUG_FLAG_01_CONTROL_MENU_INPUT
							printf("\t####### CONTROL_MENU_HAS_RETURNED: %d%s",
								r,
								newline_string_p);
#endif

#if 1
							if (r == -1) {
								return_value = -1;
							} else if (r == 0) {
								return_value = 0;
							} else if (r == 1) {

								// return_value = 1;
								return_value = 2;	/* stay in menu_control_flag==1 mode until the buffers flushed */

								menu_mode_request_exit_flag = 1;

								r = connsat_connection_global_menu_mode_flag_request_exit_set(
									menu_mode_request_exit_flag,
									raw_mode_flag);

							} else if (r == 2) {
								return_value = 2;
							} else {
								return_value = 1;
							}
#endif
						}

						if (menu_mode_request_exit_flag == 1) {
							/* exit was requested, grant once buffers are empty */

							if (
								(connsat_connection_list_p->connection_output_buffer_p->circular_buffer_used_bytes == 0) &&
								(connsat_connection_list_p->connection_input_buffer_p->circular_buffer_used_bytes == 0)
								) {

#ifdef MASTER_DEBUG_FLAG_01_CONTROL_MENU_INPUT
								printf("\t####### CONTROL_MENU_EXITING:%s",
									newline_string_p);
#endif
								menu_mode_request_exit_flag = 0;

								r = connsat_connection_global_menu_mode_flag_request_exit_set(
									menu_mode_request_exit_flag,
									raw_mode_flag);

								return_value = 1;

							} else {

#ifdef MASTER_DEBUG_FLAG_01_CONTROL_MENU_EXIT
								printf("\t####### CONTROL_MENU_HAS_REQUESTED_EXIT: output_bytes_remaining==%d, input_bytes_remaining==%d%s",
									connsat_connection_list_p->connection_output_buffer_p->circular_buffer_used_bytes,
									connsat_connection_list_p->connection_input_buffer_p->circular_buffer_used_bytes,
									newline_string_p);
#endif
							}
						}
					}

#else
					if (++buffer_poll_loop_control_menu_entry_count <= 1) {
						connsat_buffer_printf(connsat_connection_list_p->connection_input_buffer_p, "CONTROL_MENU_BUFFER_PRINT_TEST:\r\n");
					}
#endif
				}

				break;

			case CONNSAT_CONNECTION_TYPE_TERMINAL:
			case CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_SERVER:
			case CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL:
			case CONNSAT_CONNECTION_TYPE_FORKPTY:
			case CONNSAT_CONNECTION_TYPE_SESSION_LOG:
			case CONNSAT_CONNECTION_TYPE_CAPTURE_LOG:
			case CONNSAT_CONNECTION_TYPE_NONE:
			default:
				break;
		}

		connsat_connection_list_p = connsat_connection_list_p->connsat_connection_next_p;
	}

	return return_value;
}

int
connsat_connections_pollable_stream_count (
	int menu_mode_flag,
	int raw_mode_flag,
	int *pollable_connection_count_p)
{
	struct connsat_connection_s *connsat_connection_list_p;
	int pollable_connection_count;
	char *newline_string_p;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	/* build poll array of active connections */
	connsat_connection_list_p = connsat_connection_list_head_p;

	pollable_connection_count = 0;

	while (connsat_connection_list_p) {	/* build-poll-fds */

		if ( (connsat_connection_list_p->connection_input_state ==
			CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) &&
			(connsat_connection_list_p->connection_output_state ==
			CONNSAT_CONNECTION_STATE_OPEN_ACTIVE) ) {

			if (connsat_connection_list_p->connection_input_fd >= 0) {

				/* INPUT_STREAM_FD */

				if (connsat_connection_list_p->connection_output_fd >= 0) {

					/* OUTPUT_STREAM_FD */

					if (connsat_connection_list_p->connection_input_fd ==
						connsat_connection_list_p->connection_output_fd) {

// MAJOR CONNECTION CASE 1:
#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_03
						printf("\t%s-%d: ####### POLLING_MAJOR_CONNECTION_CASE_1:%s",
							__FILE__, __LINE__,
							newline_string_p);
#endif

						/* INPUT_STREAM_FD */
						/* OUTPUT_STREAM_FD */

						/* ONE SHARED STREAM FD, POLLIN and POLLOUT */

						++pollable_connection_count;	/* increment to next avabile slot index */

					} else {

// MAJOR CONNECTION CASE 2:

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_03
						printf("\t%s-%d: ####### POLLING_MAJOR_CONNECTION_CASE_2:%s",
							__FILE__, __LINE__,
							newline_string_p);
#endif

						/* INPUT_STREAM_FD */
						/* OUTPUT_STREAM_FD */

						/* TWO SEPARATE STREAM FDs, one POLLIN and one POLLOUT */

						++pollable_connection_count;	/* increment to next avabile slot index */

						++pollable_connection_count;	/* increment to next avabile slot index */

					}

				} else {

// MAJOR CONNECTION CASE 3:

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_03
					printf("\t%s-%d: ####### POLLING_MAJOR_CONNECTION_CASE_3:%s",
						__FILE__, __LINE__,
						newline_string_p);
#endif

					/* OUTPUT_BUFFER_ONLY */
					/* INPUT_STREAM_FD */

					/* ONE INPUT ONLY STREAM FD, POLLIN */

					++pollable_connection_count;	/* increment to next avabile slot index */
				}

			} else {
				/* INPUT_BUFFER_ONLY */
				/* OUTPUT_STREAM_FD */

				if (connsat_connection_list_p->connection_output_fd >= 0) {

// MAJOR CONNECTION CASE 4:

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_03
					printf("\t%s-%d: ####### POLLING_MAJOR_CONNECTION_CASE_4:%s",
						__FILE__, __LINE__,
						newline_string_p);
#endif

					/* OUTPUT_STREAM_FD */


					/* ONE OUTPUT ONLY STREAM FD, POLLOUT */

					++pollable_connection_count;	/* increment to next avabile slot index */

				} else {

// MAJOR CONNECTION CASE 5:

#ifdef MASTER_DEBUG_FLAG_01_POLL_DEBUG_03
					printf("\t%s-%d: ####### POLLING_MAJOR_CONNECTION_CASE_5:%s",
						__FILE__, __LINE__,
						newline_string_p);
#endif

					/* INPUT_BUFFER_ONLY */
					/* OUTPUT_BUFFER_ONLY */

				}

			}

		}

		connsat_connection_list_p = connsat_connection_list_p->connsat_connection_next_p;

	} /* end-while, build-poll-fds */

	if (pollable_connection_count_p != NULL) {
		*pollable_connection_count_p = pollable_connection_count;
	}

	return 0;	/* ok */
}

int
connsat_connection_global_menu_mode_flag_state_get (
	int *menu_mode_flag_p,
	int raw_mode_flag) {

	if (menu_mode_flag_p != NULL) {
		*menu_mode_flag_p = connsat_connection_global_menu_mode_state_flag;
	} else {
		return -1; /* error */
	}

	return 0; /* ok */
}

int
connsat_connection_global_menu_mode_flag_state_set (
	int menu_mode_flag,
	int raw_mode_flag) {

	if (menu_mode_flag == 0) {
		connsat_connection_global_menu_mode_state_flag = 0;
	} else {
		connsat_connection_global_menu_mode_state_flag = 1;

		/* when menu_mode_flag is enabled, clear the exit request flag */
		connsat_connection_global_menu_mode_request_exit_flag = 0;
	}

	return 0; /* ok */
}

int
connsat_connection_global_menu_mode_flag_request_exit_get (
	int *menu_mode_exit_flag_p,
	int raw_mode_flag) {

	if (menu_mode_exit_flag_p != NULL) {
		*menu_mode_exit_flag_p = connsat_connection_global_menu_mode_request_exit_flag;
	} else {
		return -1; /* error */
	}

	return 0; /* ok */
}

int
connsat_connection_global_menu_mode_flag_request_exit_set (
	int menu_mode_exit_flag,
	int raw_mode_flag) {

	if (menu_mode_exit_flag == 0) {
		connsat_connection_global_menu_mode_request_exit_flag = 0;
	} else {
		connsat_connection_global_menu_mode_request_exit_flag = 1;
	}

	return 0; /* ok */
}

