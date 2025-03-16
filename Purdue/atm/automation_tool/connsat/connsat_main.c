
#include <stdio.h>
#include <stdlib.h>
	/* getenv() */
#include <poll.h>

#include "connsat_main.h"
#include "connsat_termios.h"
#include "connsat_options.h"
#include "connsat_spawn_pty.h"
#include "connsat_ascii.h"
#include "connsat_terminal_control_menu.h"
#include "connsat_connections.h"

#include "connsat_buffer_printf.h"

#include "connsat_connection_main_terminal.h"
#include "connsat_connection_control_menu.h"
#include "connsat_connection_forkpty.h"
#include "connsat_connection_sideband_server.h"
#include "connsat_connection_sideband_channel.h"

#include "connsat_connection_init.h"
#include "connsat_options.h"

int
main (int argc, char **argv, char *(*env_p)[])
{
	int r;
	int menu_mode_flag;
	int raw_mode_flag;
	char *newline_string_p;

	raw_mode_flag = 1;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	/* set up the default values */
	r = connsat_options_init(
		&connsat_options_all);

	/* get the user command line options */
	r = connsat_options_main(
		argc,
		argv,
		raw_mode_flag);

	if (r != 0) {

		/* bad flags or usage/help was requested */

		exit(-1);
	}

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

	r = connsat_connection_init(raw_mode_flag);

	CONNSAT_TRACE;

#if 0
	r = connsat_connection_list(raw_mode_flag);
#endif

#ifdef MASTER_DEBUG_FLAG_01
	printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%s", newline_string_p);
	printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%s", newline_string_p);
#endif

	menu_mode_flag = 0;
// @@@@@@@@@@@@@@@@@@@@
// menu_mode_flag = 1;
	r = connsat_connection_global_menu_mode_flag_state_set(menu_mode_flag, raw_mode_flag);

#if 0
#ifdef MASTER_DEBUG_FLAG_03

	CONNSAT_TRACE;

	/* menu_mode_flag == 0 */
	r = connsat_connection_buffer_distribution_injection_test(0, raw_mode_flag);

	CONNSAT_TRACE;

	// printf("%s", newline_string_p);
	// printf("%s", newline_string_p);
	// printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%s", newline_string_p);
	// printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%s", newline_string_p);
#endif
#endif

#if 1
#ifdef MASTER_DEBUG_FLAG_01

	CONNSAT_TRACE;

	/* menu_mode_flag == 0 */
	r = connsat_connection_buffer_distribution_list(0, raw_mode_flag);

	CONNSAT_TRACE;

	printf("%s", newline_string_p);
	printf("%s", newline_string_p);
	printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%s", newline_string_p);
	printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%s", newline_string_p);
#endif
#endif

// @@@@@@@@@@@@@@@@@@@@
#if 0
	/* menu_mode_flag == 1 */
	r = connsat_connection_buffer_distribution_list(1, raw_mode_flag);

	printf("\r\n");
	printf("\r\n");
	printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%s", newline_string_p);
	printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -%s", newline_string_p);
#endif

#if 0
	r = connsat_connection_list(raw_mode_flag);
#endif

#ifdef MASTER_DEBUG_FLAG_01
	printf("%s", newline_string_p);
	printf("%s", newline_string_p);
#endif

#if 1
	// MAIN_POLL_LOOP:
	CONNSAT_TRACE;

	r = connsat_connection_poll_loop(menu_mode_flag, raw_mode_flag);

	/* menu_mode may have changed */
	r = connsat_connection_global_menu_mode_flag_state_get(&menu_mode_flag, raw_mode_flag);

	CONNSAT_TRACE;

#if 1	//debug
#ifdef MASTER_DEBUG_FLAG_02
	printf("%s", newline_string_p);
	printf("%s", newline_string_p);

	CONNSAT_TRACE;

	r = connsat_connection_list(raw_mode_flag);

	CONNSAT_TRACE;
#endif
#endif
#endif

	// r = connsat_connection_main_terminal_input_close(1);
	r = connsat_connection_close(
		connsat_connection_terminal_p,
		menu_mode_flag,
		1);

	raw_mode_flag = 0;
	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	printf("connsat_connection_close returned: %d%s", r, newline_string_p);

	return 0;	/* ok */
}

