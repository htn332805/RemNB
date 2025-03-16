
#ifndef __CONNSAT_CONNECTIONS_H__
#define __CONNSAT_CONNECTIONS_H__ 1

#include <poll.h>

#include "connsat_circular_buffer_algorithm.h"

typedef enum {
	CONNSAT_CONNECTION_TYPE_NONE,

	CONNSAT_CONNECTION_TYPE_TERMINAL,
		// stdin + stdout

	CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_SERVER,
		// network listening socket, stream, bidirectional
		// listens for new sideband client connections, no other stream
		// can HUP?, restart server

	CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL,
		// network accepted connection socket, stream, bidirectional
		// if CONNSAT_CONNECTION_STATE_OPEN_ACTIVE poll_rw
		// can HUP

	CONNSAT_CONNECTION_TYPE_FORKPTY,
		// target stream, bidirectional
		// if CONNSAT_CONNECTION_STATE_OPEN_ACTIVE poll_rw
		// can HUP?, restart server

	CONNSAT_CONNECTION_TYPE_CONTROL_MENU,
		// virtual pipe, hard-coded to small/big buffers

	CONNSAT_CONNECTION_TYPE_SESSION_LOG,
		// full session log

	CONNSAT_CONNECTION_TYPE_CAPTURE_LOG
		// per command capture log

} connsat_connection_type_t;

#if 0
CHANNEL_FORWARDING_MAP_MODE:

	CONNSAT_CONNECTION_TYPE_TERMINAL
		Forwards-to: CONNSAT_CONNECTION_TYPE_TERMINAL (self-if-channel-local-echo-mode)
		Forwards-to: CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL (all active)
		Normal-mode:
			Forwards-to: CONNSAT_CONNECTION_TYPE_FORKPTY
		Menu-mode:
			Forwards-to: CONNSAT_CONNECTION_TYPE_CONTROL_MENU

	CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_SERVER
		Forwards-to: None, Log Only

	CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL
		Forwards-to: CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL (self-if-channel-local-echo-mode)
		Forwards-to: CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL (all active, except self)
		Forwards-to: CONNSAT_CONNECTION_TYPE_TERMINAL
		Normal-mode:
			Forwards-to: CONNSAT_CONNECTION_TYPE_FORKPTY
		Menu-mode:
			Forwards-to: CONNSAT_CONNECTION_TYPE_CONTROL_MENU

	CONNSAT_CONNECTION_TYPE_FORKPTY
		Normal-mode:
			Forwards-to: CONNSAT_CONNECTION_TYPE_FORKPTY (NEVER: self-if-channel-local-echo-mode)
			Forwards-to: CONNSAT_CONNECTION_TYPE_TERMINAL
			Forwards-to: CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL (all active)
		Menu-mode:
			Forwarding small-buffer-to-big-buffer-distribution  activity pauses
			FLAG: keep-block or discard data

	CONNSAT_CONNECTION_TYPE_CONTROL_MENU
		Normal-mode:
			Inactive
		Menu-mode:
			Forwards-to: CONNSAT_CONNECTION_TYPE_FORKPTY (NEVER: self-if-channel-local-echo-mode)
			Forwards-to: CONNSAT_CONNECTION_TYPE_TERMINAL
			Forwards-to: CONNSAT_CONNECTION_TYPE_NETWORK_SIDEBAND_CHANNEL (all active)

#endif

typedef enum {
	CONNSAT_CONNECTION_STATE_NONE,

	CONNSAT_CONNECTION_STATE_OPEN_ACTIVE,

	CONNSAT_CONNECTION_STATE_OPEN_INACTIVE,
		// PAUSE ACTIVITY__SKIP

	CONNSAT_CONNECTION_STATE_CLOSED
		// HUP
} connsat_connection_state_t;

struct connsat_connection_s {
	char name[64];						// Null terminated string
	connsat_connection_type_t connection_type;

	connsat_connection_state_t connection_input_state;
	connsat_connection_state_t connection_output_state;

	int connection_input_fd;
		/* -1 disables feature */
		/* If true, use POLLIN to read to input buffer */

	int connection_input_fd_poll_flag;

	int connection_output_fd;
		/* -1 disables feature */
		/* If true, use POLLOUT to write from output buffer */

	int connection_output_fd_poll_flag;

	int connection_allocated_input_buffer_size;		// Allocated input buffer size
	int connection_allocated_output_buffer_size;		// Allocated output buffer size

	circular_buffer_t *connection_input_buffer_p;		// Allocated circular input buffer object
	circular_buffer_t *connection_output_buffer_p;		// Allocated circular output buffer object

	int connection_input_buffer_enable_flag;	/* turning this on enables writes into this buffer */
	int connection_output_buffer_enable_flag;	/* turning this on enables writes into this buffer */

	int connection_input_buffer_distribution_flag;	/* turning this on enables distribution from this input buffer to other output buffers */
	int connection_output_buffer_distribution_flag;	/* turning this on enables receiving buffer distributions from other input buffers */

	int connection_distribution_self_loopback_enable_flag;

	int (*connection_input_open_p)(
		struct connsat_connection_s *connsat_connection_p,
		int menu_mode_flag,
		int raw_mode_flag
		);			// Pointer to function that opens this connection stream

		/* terminal-stdin */
			/* terminal-keyboard-input */
		/* sideband-stdin */
			/* terminal-keyboard-input */
		/* forkpty-BMC-stdout */

	int (*connection_input_close_p)(
		struct connsat_connection_s *connsat_connection_p,
		int menu_mode_flag,
		int raw_mode_flag
		);			// Pointer to function that closes this connection stream

		/* terminal-stdin */
			/* terminal-keyboard-input */
		/* sideband-stdin */
			/* terminal-keyboard-input */
		/* forkpty-BMC-stdout */

	int (*connection_output_open_p)(
		struct connsat_connection_s *connsat_connection_p,
		int menu_mode_flag,
		int raw_mode_flag
		);			// Pointer to function that opens this connection stream

		/* terminal-stdout */
			/* terminal-display-output */
		/* sideband-stdout */
			/* terminal-display-output */
		/* forkpty-BMC-stdin */

	int (*connection_output_close_p)(
		struct connsat_connection_s *connsat_connection_p,
		int menu_mode_flag,
		int raw_mode_flag
		);			// Pointer to function that closes this connection stream

		/* terminal-stdout */
			/* terminal-display-output */
		/* sideband-stdout */
			/* terminal-display-output */
		/* forkpty-BMC-stdin */

	int (*connection_input_read_p)(
		struct connsat_connection_s *connsat_connection_p,
		int menu_mode_flag,
		int raw_mode_flag
		);			// Pointer to function that reads this connection stream

		/* terminal-stdin */
			/* terminal-keyboard-input */
		/* sideband-stdin */
			/* terminal-keyboard-input */
		/* forkpty-BMC-stdout */

	int (*connection_output_write_p)(
		struct connsat_connection_s *connsat_connection_p,
		int menu_mode_flag,
		int raw_mode_flag
		);			// Pointer to function that write this connection stream

		/* terminal-stdout */
			/* terminal-display-output */
		/* sideband-stdout */
			/* terminal-display-output */
		/* forkpty-BMC-stdin */

	int (*connection_buffer_distribution_p)(
		struct connsat_connection_s *connsat_connection_p,
		int menu_mode_flag,
		int *calculation_mode_max_readable_chunk_p,
		int *calculation_mode_max_writeable_chunk_p,
		int raw_mode_flag
		);			// Pointer to function that performs buffer distribution

	struct connsat_connection_s *connsat_connection_prev_p;
	struct connsat_connection_s *connsat_connection_next_p;
};

#define BUFFER_DISTRIBUTION_MODE_FLAG_MENU_MODE				0x8000
#define BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_DISPLAY_MODE		0x0010
#define BUFFER_DISTRIBUTION_MODE_FLAG_CALCULATION_MODE			0x0020
#define BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_DISPLAY_MODE		0x0100
#define BUFFER_DISTRIBUTION_MODE_FLAG_TRANSFER_MODE			0x0200

/*
** This is a parallel array to poll_fds_array_p.
*/
struct connsat_connection_fds_array_map_s {
	struct connsat_connection_s *connsat_connection_p;
	int fds_array_map_flags;
};
#define FDS_ARRAY_MAP_FLAGS_IS_STREAM_INPUT	0x00000001
#define FDS_ARRAY_MAP_FLAGS_IS_STREAM_OUTPUT	0x00000002

int connsat_connection_new(
        char *new_connection_name_p,
        connsat_connection_type_t connsat_connection_type,
        int input_allocated_buffer_size,
        int output_allocated_buffer_size,
	int raw_mode_flag,

        int (*connection_input_open_p)(
		struct connsat_connection_s *connsat_connection_p,
		int menu_mode_flag,
		int raw_mode_flag
		),			// Pointer to function that opens this connection stream
        int (*connection_input_close_p)(
		struct connsat_connection_s *connsat_connection_p,
		int menu_mode_flag,
		int raw_mode_flag
		),			// Pointer to function that closes this connection stream
        int (*connection_output_open_p)(
		struct connsat_connection_s *connsat_connection_p,
		int menu_mode_flag,
		int raw_mode_flag
		),			// Pointer to function that opens this connection stream
        int (*connection_output_close_p)(
		struct connsat_connection_s *connsat_connection_p,
		int menu_mode_flag,
		int raw_mode_flag
		),			// Pointer to function that closes this connection stream
        int (*connection_input_read_p)(
		struct connsat_connection_s *connsat_connection_p,
		int menu_mode_flag,
		int raw_mode_flag
		),			// Pointer to function that reads this connection stream
        int (*connection_output_write_p)(
		struct connsat_connection_s *connsat_connection_p,
		int menu_mode_flag,
		int raw_mode_flag
		),			// Pointer to function that write this connection stream
	int connsat_connection_buffer_distribution_p(
		struct connsat_connection_s *connsat_connection_p,
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
		int *calculation_mode_max_writeable_chunk_p,

        	int raw_mode_flag),

        struct connsat_connection_s **connsat_connection_new_pp);

int connsat_connection_open(
	struct connsat_connection_s *connsat_connection_p,
	int menu_mode_flag,
	int raw_mode_flag);

int connsat_connection_close(
	struct connsat_connection_s *connsat_connection_p,
	int menu_mode_flag,
	int raw_mode_flag);

int connsat_connection_read(
	struct connsat_connection_s *connsat_connection_p,
	int menu_mode_flag,
	int raw_mode_flag);

int connsat_connection_write(
	struct connsat_connection_s *connsat_connection_p,
	int menu_mode_flag,
	int raw_mode_flag);

int connsat_connection_poll_loop(
	int menu_mode_flag,
	int raw_mode_flag);

int connsat_connection_dump_one(
	struct connsat_connection_s *connsat_connection_list_p,
	int output_flags,
	int raw_mode_flag);

#define CONNSAT_CONNECTION_DUMP_ONE_FLAGS_FULL		0x00000fff

#define CONNSAT_CONNECTION_DUMP_ONE_FLAGS_TYPE		0x00000001
#define CONNSAT_CONNECTION_DUMP_ONE_FLAGS_STATE		0x00000002
#define CONNSAT_CONNECTION_DUMP_ONE_FLAGS_BUFFER_ENABLE	0x00000004
#define CONNSAT_CONNECTION_DUMP_ONE_FLAGS_BUFFER_DIST	0x00000008

#define CONNSAT_CONNECTION_DUMP_ONE_FLAGS_FD		0x00000010
#define CONNSAT_CONNECTION_DUMP_ONE_FLAGS_POLL		0x00000020
#define CONNSAT_CONNECTION_DUMP_ONE_FLAGS_ALLOCATED	0x00000040
#define CONNSAT_CONNECTION_DUMP_ONE_FLAGS_SIZE		0x00000080

#define CONNSAT_CONNECTION_DUMP_ONE_FLAGS_LOOPBACK	0x00000100
#define CONNSAT_CONNECTION_DUMP_ONE_FLAGS_TRACE		0x00000200

int connsat_connection_list(
	int raw_mode_flag);

int connsat_connection_dump_active_buffers(
	int raw_mode_flag);

int connsat_connection_buffer_distribution_list(
	int menu_mode_flag,
	int raw_mode_flag);

int connsat_connection_buffer_distribution_injection_test(
	int menu_mode_flag,
	int raw_mode_flag);

int connsat_connection_buffer_poll_loop_buffer_distribution(
	int menu_mode_flag,
	int raw_mode_flag);

int connsat_connection_buffer_poll_loop_control_menu(
	int menu_mode_flag,
	int raw_mode_flag);

int connsat_connections_pollable_stream_count(
	int menu_mode_flag,
	int raw_mode_flag,
	int *pollable_connection_count_p);

int connsat_connection_global_menu_mode_flag_state_get(
	int *menu_mode_flag_p,
	int raw_mode_flag);

int connsat_connection_global_menu_mode_flag_state_set(
	int menu_mode_flag,
	int raw_mode_flag);

int connsat_connection_global_menu_mode_flag_request_exit_get(
	int *menu_mode_exit_flag_p,
	int raw_mode_flag);

int connsat_connection_global_menu_mode_flag_request_exit_set(
	int menu_mode_exit_flag,
	int raw_mode_flag);

extern struct connsat_connection_s *connsat_connection_list_head_p;

extern int connsat_connection_global_change_detected_flag;

#endif /* __CONNSAT_CONNECTIONS_H__ */

