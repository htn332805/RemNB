
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "connsat_timestamp.h"

static char timestamp_buffer[80];

// #define __STANDALONE__ 1

#ifdef __STANDALONE__
int
main ()
{
	int r;
	char *malloc_timestamp_string_p;

	r = get_timestamp_string(
		&malloc_timestamp_string_p
		);

	printf("timestamp_string==\"%s\"\n", malloc_timestamp_string_p);

	free(malloc_timestamp_string_p);

	return 0; /* ok */
}
#endif

int
get_timestamp_string (
	char **malloc_timestamp_string_pp
	)
{
	time_t what_time;
	time_t tloc;
	struct tm *tm_p;
	struct timeval tv;
	struct timezone tz;
	int len;
	char *timestamp_malloc_p;

	what_time = time(&tloc);

	tm_p = localtime(&what_time);

	gettimeofday(&tv, &tz);

#if 0
	printf("what_time==%ld\n", what_time);

	printf("tloc==%ld\n", tloc);

	printf("tm_year==%d\n", tm_p->tm_year + 1900);
	printf("tm_mon==%d\n", tm_p->tm_mon + 1);
	printf("tm_mday==%d\n", tm_p->tm_mday);
	printf("tm_hour==%d\n", tm_p->tm_hour);
	printf("tm_min==%d\n", tm_p->tm_min);
	printf("tm_sec==%d\n", tm_p->tm_sec);
#endif

	sprintf(timestamp_buffer,
		"%04d_%02d_%02d_%02d_%02d_%02d_%ld",
		tm_p->tm_year + 1900,
		tm_p->tm_mon + 1,
		tm_p->tm_mday,
		tm_p->tm_hour,
		tm_p->tm_min,
		tm_p->tm_sec,
		tv.tv_usec
		);

	len = strlen(timestamp_buffer);

#if 0
	printf("len==%d\n", len);
	printf("timestamp_string==\"%s\"\n", timestamp_buffer);
#endif

	timestamp_malloc_p = malloc(len + 1);

	strcpy(timestamp_malloc_p, timestamp_buffer);

#if 0
	printf("timestamp_malloc_string==\"%s\"\n", timestamp_malloc_p);

	free(timestamp_malloc_p);
#endif

	if (malloc_timestamp_string_pp != NULL) {
		*malloc_timestamp_string_pp = timestamp_malloc_p;
	}

	return 0; /* ok */
}

int
get_timestamp_seconds (
	time_t *get_seconds_p
	)
{
	time_t what_time;
	time_t tloc;

	what_time = time(&tloc);

#if 0
	printf("what_time==%ld\r\n", what_time);
	printf("tloc==%ld\r\n", tloc);
#endif

	if (get_seconds_p != NULL) {
		*get_seconds_p = what_time;
	}

	return 0; /* ok */
}

