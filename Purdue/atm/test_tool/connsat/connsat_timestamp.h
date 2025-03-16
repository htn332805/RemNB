
#ifndef __CONNSAT_TIMESTAMP_H__
#define __CONNSAT_TIMESTAMP_H__ 1

int get_timestamp_string(
	char **malloc_timestamp_string_pp
	);

int get_timestamp_seconds(
	time_t *get_seconds_p
	);

#endif /* __CONNSAT_TIMESTAMP_H__ */

