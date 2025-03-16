
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>

#include "connsat_options.h"
#include "connsat_circular_buffer_algorithm.h"
#include "connsat_buffer_printf.h"

// #define DEBUG_CONNSAT_OPTIONS 1

static char *short_options = ":TtKkA::aDdL:lR:S:sNni:p:W:vVh?";

/*
** Getopt short options: optstring:
**
** You can change the default behavior by using a
** special first character in optstring, as follows:
**
**	optstring[0] == '+'
**
** GNU getopt() behaves like standard getopt(); it
** returns options in the order in which they are
** found, stopping at the first nonoption argument.
** This will also be true if POSIXLY_CORRECT exists
** in the environment.
**
**	optstring[0] == '-'
**
** GNU getopt() returns every command-line argument,
** whether or not it represents an argument. In this
** case, for each such argument, the function returns
** the integer 1 and sets optarg to point to the
** string.
**
** As for standard getopt(), if the first character
** of optstring is a ':', then GNU getopt()
** distinguishes between "invalid option" and
** "missing option argument" by returning '?' or ':',
** respectively. The ':' in optstring can be the
** second character if the first character is '+' or
** '-'.
**
** Finally, if an option letter in optstring is
** followed by two colon characters, then that option
** is allowed to have an optional option argument.
** (Say that three times fast!) Such an argument is
** deemed to be present if it's in the same argv
** element as the option, and absent otherwise. In
** the case that it's absent, GNU getopt() returns
** the option letter and sets optarg to NULL. For
** example, given—
**
**	while ((c = getopt(argc, argv, "ab::")) != 1)
**	   ...
**
** —for -bYANKEES, the return value is 'b', and
** optarg points to "YANKEES", while for -b or '-b
** YANKEES', the return value is still 'b' but optarg
** is set to NULL. In the latter case, "YANKEES" is a
** separate command-line argument.
*/

#if 0
	struct option {
		const char *name;
		int         has_arg;
		int        *flag;
		int         val;
	};
#endif

static struct option long_options[] = {
//		{"add", required_argument, 0, 0},
//		{"append", no_argument, 0, 0},
//		{"delete", required_argument, 0, 0},
//		{"verbose", no_argument, 0, 0},
//		{"create", required_argument, 0, 'c'},
//		{"file", required_argument, 0, 0},
		{"ascii", no_argument, 0, 'D'},
		{"alive", optional_argument, 0, 'A'},
		{"calc", no_argument, 0, 'K'},
		{"exec", required_argument, 0, 'S'},
		{"help", no_argument, 0, 'h'},
		{"ipaddr", required_argument, 0, 'i'},
		{"port", required_argument, 0, 'p'},
		{"session", required_argument, 0, 'L'},
		{"capture", required_argument, 0, 'R'},
		{"trans", no_argument, 0, 'T'},
		{"version", no_argument, 0, 'v'},
		{"wait", required_argument, 0, 'W'},
		{0, 0, 0, 0}
};

connsat_options_t connsat_options_all;

int
connsat_options_usage (
	char *cmd,
	int raw_mode_flag
	)
{
	char *newline_string_p;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	printf("%s: usage:%s",
		 cmd,
		newline_string_p);

	printf("\t-D\t\t\t\t\t# Debug Ascii%s",
		newline_string_p);
	printf("\t\t--ascii%s",
		newline_string_p);

	printf("%s",
		newline_string_p);

	printf("\t-A \"<command_string>\"\t\t\t# Keep Alive Feature%s",
		newline_string_p);
	printf("\t\t--alive=\"<command_string>\"%s",
		newline_string_p);

	printf("%s",
		newline_string_p);

	printf("\t-K\t\t\t\t\t# Show Buffer Distribution Calculations%s",
		newline_string_p);
	printf("\t\t--calc%s",
		newline_string_p);

	printf("%s",
		newline_string_p);

	printf("\t-S \"<spawn_command_string>\"\t\t# ForkPTY Spawn Command String%s",
		newline_string_p);
	printf("\t\t--exec=\"<spawn_command_string>\"%s",
		newline_string_p);

	printf("%s",
		newline_string_p);

	printf("\t-h\t\t\t\t\t# Show Help%s",
		newline_string_p);
	printf("\t\t--help%s",
		newline_string_p);

	printf("%s",
		newline_string_p);

	printf("\t-i \"127.0.0.1\"\t\t\t\t# IP_address (ip-address|INADDR_ANY)%s",
		newline_string_p);
	printf("\t\t--ipaddr=\"127.0.0.1\"%s",
		newline_string_p);

	printf("%s",
		newline_string_p);

	printf("\t-p \"20001\"\t\t\t\t# IP_port%s",
		newline_string_p);
	printf("\t\t--port=\"20001\"%s",
		newline_string_p);

	printf("%s",
		newline_string_p);

	printf("\t-L \"<path_string>\"\t\t\t# Session_Log_Prepend_Path%s",
		newline_string_p);
	printf("\t\t--session=\"<path_string>\"%s",
		newline_string_p);

	printf("%s",
		newline_string_p);

	printf("\t-R \"<path_string>\"\t\t\t# Capture_Log_Prepend_Path%s",
		newline_string_p);
	printf("\t\t--capture=\"<path_string>\"%s",
		newline_string_p);

	printf("%s",
		newline_string_p);

	printf("\t-T\t\t\t\t\t# Show Buffer Distribution Transfers%s",
		newline_string_p);
	printf("\t\t--trans%s",
		newline_string_p);

	printf("\t-W \"15\"\t\t\t\t# Alive feature wait time in seconds%s",
		newline_string_p);
	printf("\t\t--wait=\"15\"%s",
		newline_string_p);

	printf("%s",
		newline_string_p);

	printf("\t-v\t\t\t\t# display version%s",
		newline_string_p);
	printf("\t\t--version%s",
		newline_string_p);

	printf("%s",
		newline_string_p);

	return 0; /* ok */
}

int
connsat_options_init (connsat_options_t *connsat_options_p)
{
	/* initialize default option values */

	if (connsat_options_p != NULL) {

		connsat_options_p->trace_buffer_calculations_flag = 0;

		connsat_options_p->trace_buffer_transfer_flag = 0;

		connsat_options_p->alive_feature_enable_flag = 0;
		connsat_options_p->alive_feature_wait_time = 15;
		// connsat_options_p->alive_feature_command_string_p = "echo Waiting for command input...";
		connsat_options_p->alive_feature_command_string_p = "echo Waiting for command input...";

		connsat_options_p->debug_ascii_characters_flag = 0;

		connsat_options_p->session_buffer_enable_flag = 0;

		connsat_options_p->session_log_path_p = "session_log_";

		connsat_options_p->capture_buffer_enable_flag = 0;

		connsat_options_p->capture_log_path_p = "capture_log_";

		connsat_options_p->spawnshell_enable_flag = 1;

		connsat_options_p->spawnpty_shell_command_p = "/bin/bash -i";

		connsat_options_p->network_server_enable_flag = 1;

		connsat_options_p->network_ip_addr_string_p = "127.0.0.1";

		connsat_options_p->network_ip_port_uint16 = 20001;

		connsat_options_p->connsat_help_flag = 0;

		connsat_options_p->display_version = 0;

	} else {
		return -1; /* error */
	}

	return 0; /* ok */
}

/*
** This function is needed BEFORE the channels and buffer
** are initialized.
** The control menu code needs to do the same display
** functionality, but print into the distribution
** buffers for the correct functionality.
** NOTE: Keep this function exactly in sync with
** connsat_options_buffer_print().
*/
int
connsat_options_print (
	connsat_options_t *connsat_options_p,
	int raw_mode_flag
	)
{
	char *newline_string_p;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	if (connsat_options_p != NULL) {

		printf("%s%sCURRENT SETTINGS:%s",
			newline_string_p,
			newline_string_p,
			newline_string_p
			);

		printf("============================%s",
			newline_string_p
			);

		printf("trace_buffer_transfer_flag == %d%s",
			connsat_options_p->trace_buffer_transfer_flag,
			newline_string_p
			);
			/* -T, -t */

		printf("trace_buffer_calculations_flag == %d%s",
			connsat_options_p->trace_buffer_calculations_flag,
			newline_string_p
			);
			/* -K, -k */

		printf("alive_feature_enable_flag == %d%s",
			connsat_options_p->alive_feature_enable_flag,
			newline_string_p
			);
			/* -A, -a */

		printf("alive_feature_wait_time == %d%s",
			connsat_options_p->alive_feature_wait_time,
			newline_string_p
			);
			/* -W 15 */
			/* --wait=15 */

		if (connsat_options_p->alive_feature_command_string_p != NULL) {
			printf("alive_feature_command_string_p == \"%s\"%s",
				connsat_options_p->alive_feature_command_string_p,
				newline_string_p
				);
			/* -A="xxxxxxxxxxxx" */
		} else {
			printf("alive_feature_command_string_p == \"%s\"%s",
				"(NULL_PTR)",
				newline_string_p
				);
		}

		printf("debug_ascii_characters_flag == %d%s",
			connsat_options_p->debug_ascii_characters_flag,
			newline_string_p
			);
			/* -D, -d */

		printf("session_buffer_enable_flag == %d%s",
			connsat_options_p->session_buffer_enable_flag,
			newline_string_p
			);
			/* -L, -l="path" */

		if (connsat_options_p->session_log_path_p != NULL) {
			printf("session_log_path_p == \"%s\"%s",
				connsat_options_p->session_log_path_p,
				newline_string_p
				);
			/* -S="xxxxxxxxxxxx" */
			/* --session="xxxxxxxx" */
		} else {
			printf("session_log_path_p == \"%s\"%s",
				"(NULL_PTR)",
				newline_string_p
				);
		}

		printf("capture_buffer_enable_flag == %d%s",
			connsat_options_p->capture_buffer_enable_flag,
			newline_string_p
			);
			/* -R, -r="path" */

		if (connsat_options_p->capture_log_path_p != NULL) {
			printf("capture_log_path_p == \"%s\"%s",
				connsat_options_p->capture_log_path_p,
				newline_string_p
				);
			/* -R="xxxxxxxxxxxx" */
			/* --capture="xxxxxxxx" */
		} else {
			printf("capture_log_path_p == \"%s\"%s",
				"(NULL_PTR)",
				newline_string_p
				);
		}

		printf("spawnshell_enable_flag == %d%s",
			connsat_options_p->spawnshell_enable_flag,
			newline_string_p
			);
			/* -S, -s */

		if (connsat_options_p->spawnpty_shell_command_p != NULL) {
			printf("spawnpty_shell_command == \"%s\"%s",
				connsat_options_p->spawnpty_shell_command_p,
				newline_string_p
				);
			/* -S="/bin/bash -i" */
			/* --exec="xxxxxxxx" */

		} else {
			printf("spawnpty_shell_command == \"%s\"%s",
				"(NULL_PTR)",
				newline_string_p
				);
		}

		printf("network_server_enable_flag == %d%s",
			connsat_options_p->network_server_enable_flag,
			newline_string_p
			);
			/* -N, -n */

		if (connsat_options_p->network_ip_addr_string_p != NULL) {
			printf("spawnpty_shell_command == \"%s\"%s",
				connsat_options_p->network_ip_addr_string_p,
				newline_string_p
				);
			/* -i="0.0.0.0" */
			/* --ipaddr="0.0.0.0" */
		} else {
			printf("network_ip_addr_string_p == \"%s\"%s",
				"(NULL_PTR)",
				newline_string_p
				);
		}

		printf("network_ip_port_uint16 == %ld%s",
			connsat_options_p->network_ip_port_uint16,
			newline_string_p
			);
			/* -p="12345" */
			/* --port="12345" */

		printf("connsat_help_flag == %d%s",
			connsat_options_p->connsat_help_flag,
			newline_string_p
			);
			/* -H, -h */

		printf("display_version_flag == %d%s",
			connsat_options_p->display_version,
			newline_string_p
			);
			/* -v */


	} else {
		return -1; /* error */
	}

	return 0; /* ok */
}

int
connsat_options_buffer_print (
	circular_buffer_t *circular_buffer_p,
	connsat_options_t *connsat_options_p,
	int raw_mode_flag
	)
{
	char *newline_string_p;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	if (connsat_options_p != NULL) {

		connsat_buffer_printf(circular_buffer_p,
			"%s%sCURRENT SETTINGS:%s",
			newline_string_p,
			newline_string_p,
			newline_string_p
			);

		connsat_buffer_printf(circular_buffer_p,
			"============================%s",
			newline_string_p
			);

		connsat_buffer_printf(circular_buffer_p,
			"trace_buffer_transfer_flag == %d%s",
			connsat_options_p->trace_buffer_transfer_flag,
			newline_string_p
			);
			/* -T, -t */

		connsat_buffer_printf(circular_buffer_p,
			"trace_buffer_calculations_flag == %d%s",
			connsat_options_p->trace_buffer_calculations_flag,
			newline_string_p
			);
			/* -K, -k */

		connsat_buffer_printf(circular_buffer_p,
			"alive_feature_enable_flag == %d%s",
			connsat_options_p->alive_feature_enable_flag,
			newline_string_p
			);
			/* -A, -a */

		connsat_buffer_printf(circular_buffer_p,
			"alive_feature_wait_time == %d%s",
			connsat_options_p->alive_feature_wait_time,
			newline_string_p
			);
			/* -W 15 */
			/* --wait=15 */

		if (connsat_options_p->alive_feature_command_string_p != NULL) {
			connsat_buffer_printf(circular_buffer_p,
				"alive_feature_command_string_p == \"%s\"%s",
				connsat_options_p->alive_feature_command_string_p,
				newline_string_p
				);
			/* -A="xxxxxxxxxxxx" */
		} else {
			connsat_buffer_printf(circular_buffer_p,
				"alive_feature_command_string_p == \"%s\"%s",
				"(NULL_PTR)",
				newline_string_p
				);
		}

		connsat_buffer_printf(circular_buffer_p,
			"debug_ascii_characters_flag == %d%s",
			connsat_options_p->debug_ascii_characters_flag,
			newline_string_p
			);
			/* -D, -d */

		connsat_buffer_printf(circular_buffer_p,
			"session_buffer_enable_flag == %d%s",
			connsat_options_p->session_buffer_enable_flag,
			newline_string_p
			);
			/* -L, -l="path" */

		if (connsat_options_p->session_log_path_p != NULL) {
			connsat_buffer_printf(circular_buffer_p,
				"session_log_path_p == \"%s\"%s",
				connsat_options_p->session_log_path_p,
				newline_string_p
				);
			/* -S="xxxxxxxxxxxx" */
			/* --session="xxxxxxxx" */
		} else {
			connsat_buffer_printf(circular_buffer_p,
				"session_log_path_p == \"%s\"%s",
				"(NULL_PTR)",
				newline_string_p
				);
		}

		connsat_buffer_printf(circular_buffer_p,
			"capture_buffer_enable_flag == %d%s",
			connsat_options_p->capture_buffer_enable_flag,
			newline_string_p
			);
			/* -R, -r="path" */

		if (connsat_options_p->capture_log_path_p != NULL) {
			connsat_buffer_printf(circular_buffer_p,
				"capture_log_path_p == \"%s\"%s",
				connsat_options_p->capture_log_path_p,
				newline_string_p
				);
			/* -R="xxxxxxxxxxxx" */
			/* --capture="xxxxxxxx" */
		} else {
			connsat_buffer_printf(circular_buffer_p,
				"capture_log_path_p == \"%s\"%s",
				"(NULL_PTR)",
				newline_string_p
				);
		}

		connsat_buffer_printf(circular_buffer_p,
			"spawnshell_enable_flag == %d%s",
			connsat_options_p->spawnshell_enable_flag,
			newline_string_p
			);
			/* -S, -s */

		if (connsat_options_p->spawnpty_shell_command_p != NULL) {
			connsat_buffer_printf(circular_buffer_p,
				"spawnpty_shell_command == \"%s\"%s",
				connsat_options_p->spawnpty_shell_command_p,
				newline_string_p
				);
			/* -S="/bin/bash -i" */
			/* --exec="xxxxxxxx" */

		} else {
			connsat_buffer_printf(circular_buffer_p,
				"spawnpty_shell_command == \"%s\"%s",
				"(NULL_PTR)",
				newline_string_p
				);
		}

		connsat_buffer_printf(circular_buffer_p,
			"network_server_enable_flag == %d%s",
			connsat_options_p->network_server_enable_flag,
			newline_string_p
			);
			/* -N, -n */

		if (connsat_options_p->network_ip_addr_string_p != NULL) {
			connsat_buffer_printf(circular_buffer_p,
				"spawnpty_shell_command == \"%s\"%s",
				connsat_options_p->network_ip_addr_string_p,
				newline_string_p
				);
			/* -i="0.0.0.0" */
			/* --ipaddr="0.0.0.0" */
		} else {
			connsat_buffer_printf(circular_buffer_p,
				"network_ip_addr_string_p == \"%s\"%s",
				"(NULL_PTR)",
				newline_string_p
				);
		}

		connsat_buffer_printf(circular_buffer_p,
			"network_ip_port_uint16 == %ld%s",
			connsat_options_p->network_ip_port_uint16,
			newline_string_p
			);
			/* -p="12345" */
			/* --port="12345" */

		connsat_buffer_printf(circular_buffer_p,
			"connsat_help_flag == %d%s",
			connsat_options_p->connsat_help_flag,
			newline_string_p
			);
			/* -T, -t */

		connsat_buffer_printf(circular_buffer_p,
			"display_version_flag == %d%s",
			connsat_options_p->display_version,
			newline_string_p
			);
			/* -v */


	} else {
		return -1; /* error */
	}

	return 0; /* ok */
}

int
connsat_options_main (
	int ac,
	char **av,
	int raw_mode_flag
	)
{
	int c;
	int digit_optind = 0;
	char *newline_string_p;
	int show_all_options_after_parsing_flag;
	int r;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

	show_all_options_after_parsing_flag = 0;

	while (1) {
		int this_option_optind = optind ? optind : 1;
		int long_option_index = 0;

		//opterr = 0; /* suppress getopt_long error messages */

		c = getopt_long(ac, av, short_options, long_options, &long_option_index);
		if (c == -1) {
			break;
		}

#if 0
printf("Option 0x%02x: optind==%d, opterr==%d, optopt==%d, optarg==0x%p%s",
	c, optind, opterr, optopt, optarg,
	newline_string_p);
#endif

		// static char *short_options = ":TtKkA::aDdL:lR:S:sNni:p:W:Vh?";
		switch (c) {
			case 0:
				printf("option[%d] \"%s\"", long_option_index, long_options[long_option_index].name);
				if (optarg)
					printf(" with arg \"%s\"", optarg);
					printf("%s", newline_string_p);
				break;

#if 0
			/* old sample code: */
			case '0':
			case '1':
			case '2':
				if (digit_optind != 0 && digit_optind != this_option_optind) {
					printf ("digits occur in two different argv-elements.%s");
				}
				digit_optind = this_option_optind;
				printf("option %c%s", c, newline_string_p);
				break;

			case 'a':
				printf("option a%s", newline_string_p);
				break;

			case 'b':
				printf("option b%s", newline_string_p);
				break;

			case 'c':
				printf("option c with value `%s'%s", optarg, newline_string_p);
				break;

			case 'd':
				printf("option d with value `%s'%s", optarg, newline_string_p);
				break;
#endif
			case 'T':
#ifdef DEBUG_CONNSAT_OPTIONS
				printf("option T%s", newline_string_p);
#endif
				connsat_options_all.trace_buffer_transfer_flag = 1;
				break;
			case 't':
#ifdef DEBUG_CONNSAT_OPTIONS
				printf("option t%s", newline_string_p);
#endif
				connsat_options_all.trace_buffer_transfer_flag = 0;
				break;
			case 'K':
#ifdef DEBUG_CONNSAT_OPTIONS
				printf("option K%s", newline_string_p);
#endif
				connsat_options_all.trace_buffer_calculations_flag = 1;
				break;
			case 'k':
#ifdef DEBUG_CONNSAT_OPTIONS
				printf("option k%s", newline_string_p);
#endif
				connsat_options_all.trace_buffer_calculations_flag = 0;
				break;
			case 'A':
				connsat_options_all.alive_feature_enable_flag = 1;

				if (optarg != NULL) { /* optional argument */

					connsat_options_all.alive_feature_command_string_p = optarg;
#ifdef DEBUG_CONNSAT_OPTIONS
					printf("option A with value `%s'%s", optarg, newline_string_p);
#endif

				} else {

#ifdef DEBUG_CONNSAT_OPTIONS
					if (connsat_options_all.alive_feature_command_string_p != NULL) {
						printf("option A with previous value `%s'%s",
							connsat_options_all.alive_feature_command_string_p,
							newline_string_p);
					} else {
						printf("option A with previous value `%s'%s",
							"<NULL>",
							newline_string_p);
					}
#endif
				}
				break;
			case 'a':
				connsat_options_all.alive_feature_enable_flag = 0;
#ifdef DEBUG_CONNSAT_OPTIONS
				printf("option a%s", newline_string_p);
#endif
				break;
			case 'D':
#ifdef DEBUG_CONNSAT_OPTIONS
				printf("option D%s", newline_string_p);
#endif
				connsat_options_all.debug_ascii_characters_flag = 1;
				break;
			case 'd':
#ifdef DEBUG_CONNSAT_OPTIONS
				printf("option d%s", newline_string_p);
#endif
				connsat_options_all.debug_ascii_characters_flag = 0;
				break;
			case 'L':
#ifdef DEBUG_CONNSAT_OPTIONS
				printf("option L with value `%s'%s", optarg, newline_string_p);
#endif
				connsat_options_all.session_buffer_enable_flag = 1;
				connsat_options_all.session_log_path_p = optarg;
				break;
			case 'l':
#ifdef DEBUG_CONNSAT_OPTIONS
				printf("option l%s", newline_string_p);
#endif
				connsat_options_all.session_buffer_enable_flag = 0;
				break;
			case 'R':
#ifdef DEBUG_CONNSAT_OPTIONS
				printf("option R with value `%s'%s", optarg, newline_string_p);
#endif
				connsat_options_all.capture_buffer_enable_flag = 1;
				connsat_options_all.capture_log_path_p = optarg;
				break;
			case 'S':
#ifdef DEBUG_CONNSAT_OPTIONS
				printf("option S with value `%s'%s", optarg, newline_string_p);
#endif
				connsat_options_all.spawnshell_enable_flag = 1;
				connsat_options_all.spawnpty_shell_command_p = optarg;
				break;
			case 's':
#ifdef DEBUG_CONNSAT_OPTIONS
				printf("option s%s", newline_string_p);
#endif
				connsat_options_all.spawnshell_enable_flag = 0;
				break;
			case 'N':
#ifdef DEBUG_CONNSAT_OPTIONS
				printf("option N%s", newline_string_p);
#endif
				connsat_options_all.network_server_enable_flag = 1;
				break;
			case 'n':
#ifdef DEBUG_CONNSAT_OPTIONS
				printf("option n%s", newline_string_p);
#endif
				connsat_options_all.network_server_enable_flag = 0;
				break;
			case 'i':
#ifdef DEBUG_CONNSAT_OPTIONS
				printf("option i with value `%s'%s", optarg, newline_string_p);
#endif
				connsat_options_all.network_ip_addr_string_p = optarg;
				break;
			case 'p':
#ifdef DEBUG_CONNSAT_OPTIONS
				printf("option p with value `%s'%s", optarg, newline_string_p);
#endif
				connsat_options_all.network_ip_port_uint16 =
					(unsigned short) strtoul(optarg, NULL, 0);
				break;

			case 'W':
#ifdef DEBUG_CONNSAT_OPTIONS
				printf("option W with value `%s'%s", optarg, newline_string_p);
#endif
				connsat_options_all.alive_feature_wait_time =
					(unsigned short) strtoul(optarg, NULL, 0);
				break;

			case 'v':
#ifdef DEBUG_CONNSAT_OPTIONS
				printf("option v%s", newline_string_p);
#endif
				// connsat_options_all.xxxxxxxx = 1;

				connsat_options_all.display_version = 1;

				break;

			case 'V':
#ifdef DEBUG_CONNSAT_OPTIONS
				printf("option V%s", newline_string_p);
#endif
				// connsat_options_all.xxxxxxxx = 1;

				show_all_options_after_parsing_flag = 1;

				break;

			case ':':
				/*
				** We will come here if any optional arguments are
				** missing from either short_options or long_options.
				*/
				// connsat_options_usage(av[0], raw_mode_flag);

				connsat_options_all.connsat_help_flag = 1;

				// exit(-1);
				break;

			case 'h':

#ifdef DEBUG_CONNSAT_OPTIONS
				// printf("option n%s", connsat_help_flag);
#endif

				// connsat_options_usage(av[0], raw_mode_flag);
				// exit(-1);

				connsat_options_all.connsat_help_flag = 1;

				break;

			case '?':
				// connsat_options_usage(av[0], raw_mode_flag);
				// exit(-1);

				connsat_options_all.connsat_help_flag = 1;

				break;

			default:
#ifdef DEBUG_CONNSAT_OPTIONS
				printf ("?? getopt returned character code 0x%02x ??%s", c, newline_string_p);
#endif
				break;
		}
	}

	if (optind < ac) {

		printf ("non-option ARGV-elements: ");

		while (optind < ac) {
			printf ("%s ", av[optind++]);
		}
		printf ("%s", newline_string_p);
	}

	if (show_all_options_after_parsing_flag != 0) {

		r = connsat_options_print(
			&connsat_options_all,
			raw_mode_flag
		);
	}

	if (connsat_options_all.display_version != 0) {

		// connsat_options_usage(av[0], raw_mode_flag);
		printf("%s:\n", CONNSAT_TARGET);
		printf("\tVersion: %s\n", CONNSAT_VERSION);
		printf("\tToolchain: %s\n", CONNSAT_TOOLCHAIN_VERSION);

		return -1; /* tell the program to exit */
	}

	if (connsat_options_all.connsat_help_flag != 0) {

		connsat_options_usage(av[0], raw_mode_flag);

		return -1; /* tell the program to exit */
	}

	return 0; /* ok */
}

