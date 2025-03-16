
#ifndef __CONNSAT_CLIENT_OPTIONS_H__
#define __CONNSAT_CLIENT_OPTIONS_H__ 1

struct connsat_options_s {

	char *network_ip_addr_string_p;
		// malloc'd
			// values: 0.0.0.0
			// values: 127.0.0.1
			// values: INADDR_ANY
		/* -i=<xxxx> */

	unsigned long network_ip_port_uint16;
		// values: 12345
		/* -p=<xxxx> */

	int display_version;
		/* -v */

	int connsat_help_flag;
		/* -h */
};

typedef struct connsat_options_s connsat_options_t;

extern connsat_options_t connsat_options_all;

int connsat_options_usage(
	char *cmd,
	int raw_mode_flag
	);

int connsat_options_main(
	int ac,
	char **av,
	connsat_options_t *connsat_options_p,
	int raw_mode_flag
	);

int connsat_options_init(
	connsat_options_t *connsat_options_p
	);

int connsat_options_print(
	connsat_options_t *connsat_options_p,
	int raw_mode_flag
	);

#endif /* __CONNSAT_CLIENT_OPTIONS_H__ */

