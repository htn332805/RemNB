
#ifndef __CONNSAT_OPTIONS_H__
#define __CONNSAT_OPTIONS_H__ 1

#include "connsat_circular_buffer_algorithm.h"

int connsat_options_usage(
	char *cmd,
	int raw_mode_flag
	);

int connsat_options_main(
	int ac,
	char **av,
	int raw_mode_flag
	);

#if 0

Show Vars
	control_menu> V

MASTER_DEBUG_FLAG_01_TRANS
	control_menu> T == on
	control_menu> t == off

	-T
	-t
	--trans

MASTER_DEBUG_FLAG_01_CALC
	control_menu> K == on
	control_menu> k == off

	-K
	-k
	--calc

Alive Feature
	control_menu> A == on
	control_menu> a == off

	-A="echo Waiting for command"
	-a
	-W 15

	--alive="echo Waiting for command"
	--wait=15

Debug Ascii Characters
	control_menu> D == on
	control_menu> d == off

	-D
	-d

	--ascii

Session Log
	control_menu> L == on
	control_menu> l == off

	-L="<session_log_prepend_pathname>"
	# <session_log_prepend_pathname>_timestamp

Capture Log
	control_menu> R == on
	control_menu> r == off

	-R="<capture_log_prepend_pathname>"
	# <capture_log_prepend_pathname>_timestamp
	# create symbolic_link to latest capture_log

Spawnshell_Forkpty
	control_menu> S == on
	control_menu> s == off

	-S
	-s

	-S="/bin/bash -i"
	--exec="/bin/bash -i"

Network_Server
	control_menu> N == on
	control_menu> n == off

	-n
		disable Network Server

	-i=0.0.0.0
	--ipaddr==0.0.0.0

	-p=12345
	--port==12345

Help_flag:
	-h

Version_flag:
	-v

#endif

struct connsat_options_s {

	int trace_buffer_calculations_flag;
		/* debug output */
		/* -K, -k */

	int trace_buffer_transfer_flag;
		/* debug output */
		/* -T, -t */

	int alive_feature_enable_flag;
		/* -A, -a */

	int alive_feature_wait_time;
		/* -W=15 */
		/* --wait=15 */

	char *alive_feature_command_string_p;
		/* -A="echo Waiting for command" */

	int debug_ascii_characters_flag;
		/* -D, -d */

	int spawnshell_enable_flag;
		/* -S, -s */

	char *spawnpty_shell_command_p;
		// malloc'd
		/* -e="/bin/bash -i" */

	int network_server_enable_flag;
		/* -N, -n */

	char *network_ip_addr_string_p;
		// malloc'd
			// values: 0.0.0.0
			// values: 127.0.0.1
			// values: INADDR_ANY
		/* -i=<xxxx> */

	unsigned long network_ip_port_uint16;
		// values: 12345
		/* -p=<xxxx> */

	int session_buffer_enable_flag;
		/* -L, -l */

	char *session_log_path_p;
		/* -L="xxxxxxxx" */

	int capture_buffer_enable_flag;
		/* -R, -r */

	char *capture_log_path_p;
		/* -R="xxxxxxxx" */

	int connsat_help_flag;
		/* -h */

	int display_version;
		/* -v */
};

typedef struct connsat_options_s connsat_options_t;

extern connsat_options_t connsat_options_all;

int connsat_options_init(
	connsat_options_t *connsat_options_p
	);

int connsat_options_print(
	connsat_options_t *connsat_options_p,
	int raw_mode_flag
	);

int connsat_options_buffer_print(
	circular_buffer_t *circular_buffer_p,
	connsat_options_t *connsat_options_p,
	int raw_mode_flag
	);

#endif /* __CONNSAT_OPTIONS_H__ */

