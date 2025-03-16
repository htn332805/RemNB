
#include <stdio.h>
#include <stdlib.h>

#include "connsat_connection_main_terminal.h"
#include "connsat_connection_control_menu.h"
#include "connsat_connection_forkpty.h"
#include "connsat_connection_sideband_server.h"
#include "connsat_connection_sideband_channel.h"

struct connsat_connection_s *connsat_connection_terminal_p;
struct connsat_connection_s *connsat_connection_control_menu_p;
struct connsat_connection_s *connsat_connection_forkpty_p;
struct connsat_connection_s *connsat_connection_sideband_server_p;
struct connsat_connection_s *connsat_connection_sideband_channel_p;

int
connsat_connection_init (int raw_mode_flag)
{
	int r;
	char *newline_string_p;
	int menu_mode_flag;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

//@@@@@@@
	menu_mode_flag = 0;

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

	r = connsat_connection_main_terminal(
		&connsat_connection_terminal_p,
		menu_mode_flag,
		raw_mode_flag);

	if (r < 0) {
		printf("FAILED: connsat_connection_terminal_p connsat_connection_new(): %d%s", r, newline_string_p);
		exit(1);
	}

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

	r = connsat_connection_control_menu(
		&connsat_connection_control_menu_p,
		menu_mode_flag,
		raw_mode_flag);

	if (r < 0) {
		printf("FAILED: connsat_connection_control_menu_p connsat_connection_new(): %d%s", r, newline_string_p);
		exit(1);
	}

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

	r = connsat_connection_forkpty(
		&connsat_connection_forkpty_p,
		menu_mode_flag,
		raw_mode_flag);

	if (r < 0) {
		printf("FAILED: connsat_connection_forkpty_p connsat_connection_new(): %d%s", r, newline_string_p);
		exit(1);
	}

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

	r = connsat_connection_sideband_server(
		&connsat_connection_sideband_server_p,
		menu_mode_flag,
		raw_mode_flag);

	if (r < 0) {
		printf("FAILED: connsat_connection_sideband_server_p connsat_connection_new(): %d%s", r, newline_string_p);
		exit(1);
	}

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

	r = connsat_connection_sideband_channel(
		&connsat_connection_sideband_channel_p,
		menu_mode_flag,
		raw_mode_flag);

	if (r < 0) {
		printf("FAILED: connsat_connection_sideband_channel_p connsat_connection_new(): %d%s", r, newline_string_p);
		exit(1);
	}

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if 0	/* DEBUG */
	/* menu_mode_flag == 0 */
	r = connsat_connection_buffer_distribution_list(0, raw_mode_flag);

	/* menu_mode_flag == 1 */
	r = connsat_connection_buffer_distribution_list(1, raw_mode_flag);
#endif

	return 0;	/* ok */
}

