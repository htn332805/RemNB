
#include <stdio.h>
#include <stdlib.h>
	/* getenv() */

// #include <unistd.h>
// extern char **environ;

#include "connsat_connections.h"
#include "connsat_circular_buffer_algorithm.h"
#include "connsat_buffer_printf.h"
#include "connsat_terminal_control_menu.h"
#include "connsat_options.h"

// #define DEBUG_CONNSAT_TERMINAL_CONTROL_MENU 1

// return_value = -1;	/* EOF */
// return_value = 0;	/* Double ^D, exit */
// return_value = 1;	/* Done with control menu, continue */
// return_value = 2;	/* Stay in control menu mode, continue */
int
connsat_control_menu (
	circular_buffer_t *menu_output_buffer_p,
	circular_buffer_t *menu_input_buffer_p,
	char *(*env_p)[],
	int debug_output_flag)
{
	int return_value;
	int input_char;
	int r;
	char *p;
	char *env_name_p;
	// char string_buf[128];

	int index;

	return_value = 2;

#ifdef DEBUG_CONNSAT_TERMINAL_CONTROL_MENU
	if (menu_output_buffer_p->circular_buffer_used_bytes == 0) {
#if 0
		/* Let the user know we're in the control menu */
		connsat_buffer_printf(menu_input_buffer_p, "\r\n%s-%d!!> ", __FILE__, __LINE__);
#endif
#if 1
		printf("\r\n%s-%d!!> ", __FILE__, __LINE__);
#endif
	}
#endif


#ifdef DEBUG_CONNSAT_TERMINAL_CONTROL_MENU
	printf("\r\n%s-%d, circular_buffer_used_bytes==%d\r\n", __FILE__, __LINE__, menu_output_buffer_p->circular_buffer_used_bytes);
	fflush(stdout);
#endif

	while (menu_output_buffer_p->circular_buffer_used_bytes > 0) {

#ifdef DEBUG_CONNSAT_TERMINAL_CONTROL_MENU
		printf("\r\n%s-%d, circular_buffer_used_bytes==%d\r\n", __FILE__, __LINE__, menu_output_buffer_p->circular_buffer_used_bytes);
		fflush(stdout);
#endif

#ifdef DEBUG_CONNSAT_TERMINAL_CONTROL_MENU
		/* Let the user know we're in the control menu */
		connsat_buffer_printf(menu_input_buffer_p, "\r\n%s-%d@@> ", __FILE__, __LINE__);
#endif

		r = circular_buffer_getc(menu_output_buffer_p, &input_char);

		if (r < 0) {

			/* error */

		} else if (r == 0) {

			/* no keyboard characters to drive menu */
#ifdef DEBUG_CONNSAT_TERMINAL_CONTROL_MENU
			/* Let the user know we're in the control menu */
			connsat_buffer_printf(menu_input_buffer_p, "\r\n%s-%d%%> ", __FILE__, __LINE__);
#endif

		} else if (r > 0) {

			/* there are keyboard characters to drive menu */

			if (input_char == EOF) {

				connsat_buffer_printf(menu_input_buffer_p, "\r\n@");

				connsat_buffer_printf(menu_input_buffer_p, "MAIN_STDIN Received: EOF\n");

				return_value = -1;	/* EOF */

				break;

			} else if (input_char == 0x04) {

				connsat_buffer_printf(menu_input_buffer_p, "\r\n^D");

#if 0
				connsat_buffer_printf(menu_input_buffer_p, "MAIN_STDIN: Received: %3d 0x%2x, DONE", input_char, input_char);
				connsat_buffer_printf(menu_input_buffer_p, "\r");
				connsat_buffer_printf(menu_input_buffer_p, "\n");
				fflush(stdout);
#endif

				return_value = 0;	/* Double ^D, exit */

				break;

			} else if (input_char == 0x0a) {	/* LF */

				continue;
				// break;

			} else if (input_char == 0x0d) {	/* CR */

				continue;
				// break;

			} else if (input_char == 0x1b) {	/* ESC key */

				return_value = 1;	/* exit control menu */

			} else if (input_char == 0x2e) {	/* period key */

				connsat_buffer_printf(menu_input_buffer_p, "\r\n%s-%d>>> ", __FILE__, __LINE__);

			} else if (input_char == 'b') {	/* dump buffers mini */

				r = connsat_connection_dump_active_buffers(1);

				connsat_buffer_printf(menu_input_buffer_p, "\r\n%s-%d>>> ",
					__FILE__, __LINE__);

			} else if (input_char == 'B') {	/* dump buffers full */

				r = connsat_connection_list(1);

				// r = connsat_connection_buffer_distribution_list(0, 1);
				// r = connsat_connection_buffer_distribution_list(1, 1);

				connsat_buffer_printf(menu_input_buffer_p, "\r\n%s-%d>>> ",
					__FILE__, __LINE__);

			} else if (input_char == 'E') {	/* display environment */

				index = 0;

				connsat_buffer_printf(menu_input_buffer_p, "\r\n");

				while ( (*env_p)[index] != NULL) {

#if 1
					p = (*env_p)[index];

					connsat_buffer_printf(menu_input_buffer_p,
						"MAIN_STDIN: %d: *ENV_P[%d]: \"",
						index + 1,
						index);

					/* Our terminal is in a mode that doesn't add the \r with each \n. */
					/* We need to do it here. */

					while (*p) {
						connsat_buffer_printf(menu_input_buffer_p, "%c", *p);

						if ( *p == '\n') {
							connsat_buffer_printf(menu_input_buffer_p, "\r");
							connsat_buffer_printf(menu_input_buffer_p, "\t");	/* hack to try and make the output more readable */
						}

						++p;
					}

					connsat_buffer_printf(menu_input_buffer_p, "\"");

					connsat_buffer_printf(menu_input_buffer_p, "\r");
					connsat_buffer_printf(menu_input_buffer_p, "\n");
#else
					connsat_buffer_printf(menu_input_buffer_p, "MAIN_STDIN: %d: *ENV_P[%d]: \"%s\"", index + 1, index, (*env_p)[index]);
					connsat_buffer_printf(menu_input_buffer_p, "<\\r>\r");
					connsat_buffer_printf(menu_input_buffer_p, "<\\n>\n");
#endif

					fflush(stdout);

					++index;
				}

				return_value = 1;	/* Done with control menu, continue */

				break;

			} else if (input_char == 'd') {	/* debug character mode off */

				connsat_options_all.debug_ascii_characters_flag = 0;

				connsat_buffer_printf(menu_input_buffer_p, "debug_ascii_characters_flag==%d\r\n%s-%d>>> ",
					connsat_options_all.debug_ascii_characters_flag,
					__FILE__, __LINE__);

			} else if (input_char == 'D') {	/* debug character mode on */

				connsat_options_all.debug_ascii_characters_flag = 1;

				connsat_buffer_printf(menu_input_buffer_p, "debug_ascii_characters_flag==%d\r\n%s-%d>>> ",
					connsat_options_all.debug_ascii_characters_flag,
					__FILE__, __LINE__);

			} else if (input_char == 'a') {	/* alive feature flag */

				/* -A, -a */
				connsat_options_all.alive_feature_enable_flag = 0;

				connsat_buffer_printf(menu_input_buffer_p, "alive_feature_enable_flag==%d\r\n%s-%d>>> ",
					connsat_options_all.alive_feature_enable_flag,
					__FILE__, __LINE__);

			} else if (input_char == 'A') {	/* alive feature flag */

				/* -A, -a */
				connsat_options_all.alive_feature_enable_flag = 1;

				connsat_buffer_printf(menu_input_buffer_p, "alive_feature_enable_flag==%d\r\n%s-%d>>> ",
					connsat_options_all.alive_feature_enable_flag,
					__FILE__, __LINE__);


			} else if (input_char == 'k') {	/* clear buffer calculation trace flag */

				/* -K, -k */
				connsat_options_all.trace_buffer_calculations_flag = 0;

				connsat_buffer_printf(menu_input_buffer_p, "trace_buffer_calculations_flag==%d\r\n%s-%d>>> ",
					connsat_options_all.trace_buffer_calculations_flag,
					__FILE__, __LINE__);

			} else if (input_char == 'K') {	/* set buffer calculation trace flag */

				/* -K, -k */
				connsat_options_all.trace_buffer_calculations_flag = 1;

				connsat_buffer_printf(menu_input_buffer_p, "trace_buffer_calculations_flag==%d\r\n%s-%d>>> ",
					connsat_options_all.trace_buffer_calculations_flag,
					__FILE__, __LINE__);

			} else if (input_char == 't') {	/* clear buffer transfers trace flag */

				/* -T, -t */
				connsat_options_all.trace_buffer_transfer_flag = 0;

				connsat_buffer_printf(menu_input_buffer_p, "trace_buffer_transfer_flag==%d\r\n%s-%d>>> ",
					connsat_options_all.trace_buffer_transfer_flag,
					__FILE__, __LINE__);

			} else if (input_char == 'T') {	/* set buffer transfers trace flag */

				/* -T, -t */
				connsat_options_all.trace_buffer_transfer_flag = 1;

				connsat_buffer_printf(menu_input_buffer_p, "trace_buffer_transfer_flag==%d\r\n%s-%d>>> ",
					connsat_options_all.trace_buffer_transfer_flag,
					__FILE__, __LINE__);

			/* set spawnshell command placeholder */

			} else if (input_char == 's') {	/* disable spawnshell */

				/* -S, -s */
				connsat_options_all.spawnshell_enable_flag = 0;

				connsat_buffer_printf(menu_input_buffer_p, "spawnshell_enable_flag==%d\r\n%s-%d>>> ",
					connsat_options_all.spawnshell_enable_flag,
					__FILE__, __LINE__);

			} else if (input_char == 'S') {	/* enable spawnshell */

				/* -S, -s */
				connsat_options_all.spawnshell_enable_flag = 1;

				connsat_buffer_printf(menu_input_buffer_p, "spawnshell_enable_flag==%d\r\n%s-%d>>> ",
					connsat_options_all.spawnshell_enable_flag,
					__FILE__, __LINE__);

			/* set ip address placeholder */

			/* set ip port placeholder */

			} else if (input_char == 'n') {	/* disable network server */

				/* -N, -n */
				connsat_options_all.network_server_enable_flag = 0;

				connsat_buffer_printf(menu_input_buffer_p, "network_server_enable_flag==%d\r\n%s-%d>>> ",
					connsat_options_all.network_server_enable_flag,
					__FILE__, __LINE__);

			} else if (input_char == 'N') {	/* enable network server */

				/* -N, -n */
				connsat_options_all.network_server_enable_flag = 1;

				connsat_buffer_printf(menu_input_buffer_p, "network_server_enable_flag==%d\r\n%s-%d>>> ",
					connsat_options_all.network_server_enable_flag,
					__FILE__, __LINE__);

			/* session log path placeholder */


			} else if (input_char == 'l') {	/* disable sesstion log flag */

				/* -L, -l */
				connsat_options_all.session_buffer_enable_flag = 0;

				connsat_buffer_printf(menu_input_buffer_p, "session_buffer_enable_flag==%d\r\n%s-%d>>> ",
					connsat_options_all.session_buffer_enable_flag,
					__FILE__, __LINE__);

			} else if (input_char == 'L') {	/* enable sesstion log flag */

				/* -L, -l */
				connsat_options_all.session_buffer_enable_flag = 1;

				connsat_buffer_printf(menu_input_buffer_p, "session_buffer_enable_flag==%d\r\n%s-%d>>> ",
					connsat_options_all.session_buffer_enable_flag,
					__FILE__, __LINE__);

			/* capture log path placeholder */

			} else if (input_char == 'r') {	/* disable capture log flag */

				/* -R, -r */
				connsat_options_all.capture_buffer_enable_flag = 0;

				connsat_buffer_printf(menu_input_buffer_p, "capture_buffer_enable_flag==%d\r\n%s-%d>>> ",
					connsat_options_all.capture_buffer_enable_flag,
					__FILE__, __LINE__);

			} else if (input_char == 'R') {	/* enable capture log flag */

				/* -R, -r */
				connsat_options_all.capture_buffer_enable_flag = 1;

				connsat_buffer_printf(menu_input_buffer_p, "capture_buffer_enable_flag==%d\r\n%s-%d>>> ",
					connsat_options_all.capture_buffer_enable_flag,
					__FILE__, __LINE__);




			} else if (input_char == 'V') {	/* display options */

				connsat_options_buffer_print(
					menu_input_buffer_p,
					&connsat_options_all,
					1
					);

			} else if (input_char == 'H') {	/* display help menu items */

				connsat_buffer_printf(menu_input_buffer_p, "\r\n");

				r = connsat_control_menu_help(menu_input_buffer_p);

				connsat_buffer_printf(menu_input_buffer_p, "\r\n%s-%d>>> ",
					__FILE__, __LINE__);

			} else if (input_char == 'h') {	/* display help menu items */

				connsat_buffer_printf(menu_input_buffer_p, "\r\n");

				r = connsat_control_menu_help(menu_input_buffer_p);

				connsat_buffer_printf(menu_input_buffer_p, "\r\n%s-%d>>> ",
					__FILE__, __LINE__);

			} else if (input_char == '?') {	/* display help menu items */

				connsat_buffer_printf(menu_input_buffer_p, "\r\n");

				r = connsat_control_menu_help(menu_input_buffer_p);

				connsat_buffer_printf(menu_input_buffer_p, "\r\n%s-%d>>> ",
					__FILE__, __LINE__);

			} else if (input_char == 'e') {	/* display TMUX environment */

				connsat_buffer_printf(menu_input_buffer_p, "\r\n");

				env_name_p = "TMUX";
				p = getenv(env_name_p);

				if (p != NULL) {
					connsat_buffer_printf(menu_input_buffer_p, "MAIN_STDIN: ENV: %s==%s", env_name_p, p);
					connsat_buffer_printf(menu_input_buffer_p, "\r");
					connsat_buffer_printf(menu_input_buffer_p, "\n");
				}

				env_name_p = "TMUX_PANE";
				p = getenv(env_name_p);

				if (p != NULL) {
					connsat_buffer_printf(menu_input_buffer_p, "MAIN_STDIN: ENV: %s==%s", env_name_p, p);
					connsat_buffer_printf(menu_input_buffer_p, "\r");
					connsat_buffer_printf(menu_input_buffer_p, "\n");
				}

				env_name_p = "HOSTNAME";
				p = getenv(env_name_p);

				if (p != NULL) {
					connsat_buffer_printf(menu_input_buffer_p, "MAIN_STDIN: ENV: %s==%s", env_name_p, p);
					connsat_buffer_printf(menu_input_buffer_p, "\r");
					connsat_buffer_printf(menu_input_buffer_p, "\n");
				}

				env_name_p = "USER";
				p = getenv(env_name_p);

				if (p != NULL) {
					connsat_buffer_printf(menu_input_buffer_p, "MAIN_STDIN: ENV: %s==%s", env_name_p, p);
					connsat_buffer_printf(menu_input_buffer_p, "\r");
					connsat_buffer_printf(menu_input_buffer_p, "\n");
				}

				env_name_p = "PWD";
				p = getenv(env_name_p);

				if (p != NULL) {
					connsat_buffer_printf(menu_input_buffer_p, "MAIN_STDIN: ENV: %s==%s", env_name_p, p);
					connsat_buffer_printf(menu_input_buffer_p, "\r");
					connsat_buffer_printf(menu_input_buffer_p, "\n");
				}

				env_name_p = "HOME";
				p = getenv(env_name_p);

				if (p != NULL) {
					connsat_buffer_printf(menu_input_buffer_p, "MAIN_STDIN: ENV: %s==%s", env_name_p, p);
					connsat_buffer_printf(menu_input_buffer_p, "\r");
					connsat_buffer_printf(menu_input_buffer_p, "\n");
				}

				env_name_p = "SHELL";
				p = getenv(env_name_p);

				if (p != NULL) {
					connsat_buffer_printf(menu_input_buffer_p, "MAIN_STDIN: ENV: %s==%s", env_name_p, p);
					connsat_buffer_printf(menu_input_buffer_p, "\r");
					connsat_buffer_printf(menu_input_buffer_p, "\n");
				}

				env_name_p = "PATH";
				p = getenv(env_name_p);

				if (p != NULL) {
					connsat_buffer_printf(menu_input_buffer_p, "MAIN_STDIN: ENV: %s==%s", env_name_p, p);
					connsat_buffer_printf(menu_input_buffer_p, "\r");
					connsat_buffer_printf(menu_input_buffer_p, "\n");
				}

				env_name_p = "LOGNAME";
				p = getenv(env_name_p);

				if (p != NULL) {
					connsat_buffer_printf(menu_input_buffer_p, "MAIN_STDIN: ENV: %s==%s", env_name_p, p);
					connsat_buffer_printf(menu_input_buffer_p, "\r");
					connsat_buffer_printf(menu_input_buffer_p, "\n");
				}

				fflush(stdout);

				return_value = 1;	/* Done with control menu, continue */

				break;

			} else {

				/* unknown menu item, ignore and return */

				connsat_buffer_printf(menu_input_buffer_p, "UNKNOWN_MENU_COMMAND: %d %c", input_char, input_char);

				connsat_buffer_printf(menu_input_buffer_p, "\r\n%s-%d??? ", __FILE__, __LINE__);

				return_value = 2;	/* Done with control menu, continue */

				break;
			}
		}
	}

	return return_value;
}

int
connsat_control_menu_help (
	circular_buffer_t *menu_input_buffer_p)
{
	connsat_buffer_printf(menu_input_buffer_p, "\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "==================\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "Control Menu Help:\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "------------------\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t^D\tTerminate Program, Exit\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t b\tDump Buffers Mini\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t B\tDump Buffers Full\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t E\tDisplay Full Environment\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t e\tDisplay Short Environment\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t d\tDebug Chars Off\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t D\tDebug Chars On\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t k\tclear buffer calculation trace flag Off\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t K\tset buffer calculation trace flag On\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t t\tclear buffer transfer flag Off\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t T\tset buffer transfer flag On\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t s\tdisable spawnshell flag Off\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t S\tenable spawnshell flag On\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t n\tdisable network server flag Off\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t N\tenable network server flag On\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t l\tclear session log flag Off\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t L\tset session log flag On\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t r\tdisable capture log flag Off\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t R\tenable capture log flag On\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t V\tshow current option setting values\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t h\tDisplay This Control Menu Help\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t H\tDisplay This Control Menu Help\r\n");
	connsat_buffer_printf(menu_input_buffer_p, "\t ?\tDisplay This Control Menu Help\r\n");

	fflush(stdout);

	return 0;	/* ok */
}

