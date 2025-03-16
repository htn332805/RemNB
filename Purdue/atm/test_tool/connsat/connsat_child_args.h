
#ifndef __CONNSAT_CHILD_ARGS_H__
#define __CONNSAT_CHILD_ARGS_H__ 1

int child_args_parse_string(
	char *command_string_p,
	int *return_malloc_size_p,
	int *return_argc_p,
	char ***return_argv_ppp
);

int child_args_argv_dump(
	int *argc_p,
	char ***argv_ppp,
	int raw_mode_flag);

void hd_buff(
	unsigned char *bf, /* address of buffer */
	unsigned char *da, /* print address of buffer */
	int sz, /* size of buffer */
	int do_stars, /* duplicate lines indicated by single line with '*' */
	int show_eof
	);

#endif /* __CONNSAT_CHILD_ARGS_H__ */

