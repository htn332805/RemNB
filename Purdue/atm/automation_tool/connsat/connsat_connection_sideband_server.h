
#ifndef __connsat_connection_sideband_server_h__
#define __connsat_connection_sideband_server_h__ 1

#include "connsat_connections.h"

/* connection_input_open_p */
int connsat_connection_sideband_server_input_open(
	struct connsat_connection_s *connsat_connection_p,
	int menu_mode_flag,
	int raw_mode_flag);

/* connection_input_close_p */
int connsat_connection_sideband_server_input_close(
	struct connsat_connection_s *connsat_connection_p,
	int menu_mode_flag,
	int raw_mode_flag);

/* connection_output_open_p */
int connsat_connection_sideband_server_output_open(
	struct connsat_connection_s *connsat_connection_p,
	int menu_mode_flag,
	int raw_mode_flag);

/* connection_output_close_p */
int connsat_connection_sideband_server_output_close(
	struct connsat_connection_s *connsat_connection_p,
	int menu_mode_flag,
	int raw_mode_flag);

/* connection_input_read_p */
int connsat_connection_sideband_server_input_read(
	struct connsat_connection_s *connsat_connection_p,
	int menu_mode_flag,
	int raw_mode_flag);

/* connection_output_write_p */
int connsat_connection_sideband_server_output_write(
	struct connsat_connection_s *connsat_connection_p,
	int menu_mode_flag,
	int raw_mode_flag);

int connsat_connection_sideband_server_buffer_distribution(
	struct connsat_connection_s *connsat_connection_terminal_p,

	// int menu_mode_flag,
	int distribution_mode_flag,

	int *calculation_mode_max_readable_chunk_p,
		/* for buffer transfer, what's the biggest chunk we can read from the source buffer */
	int *calculation_mode_max_writeable_chunk_p,
		/* for buffer transfer, what's the biggest chunk we can write to all destination buffers */

	int raw_mode_flag);

/* install sideband_server connection */
int connsat_connection_sideband_server(
	struct connsat_connection_s **connsat_connection_sideband_server_pp,
	int menu_mode_flag,
	int raw_mode_flag
	);

#endif /* __connsat_connection_sideband_server_h__ */

