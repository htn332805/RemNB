
/*
**
** Test connsat_buffer_printf.c functionality:
**
** gcc -g -D__STANDALONE__ connsat_buffer_printf_test.c connsat_circular_buffer_algorithm.c connsat_buffer_printf.c
**
*/

#include <stdio.h>
#include <stdlib.h>
	/* getenv() */
#include <poll.h>

#include "connsat_termios.h"
#include "connsat_options.h"
#include "connsat_spawn_pty.h"
#include "connsat_ascii.h"
#include "connsat_terminal_control_menu.h"

#include "connsat_buffer_printf.h"

// #define __STANDALONE__ 1

#ifdef __STANDALONE__

int
main (int argc, char **argv, char *(*env_p)[])
{
	int i;
	int r;
	int x;
	int byte_count;
	circular_buffer_t *circular_buffer_01_p;

	r = circular_buffer_init("Buffer-01", 5000, &circular_buffer_01_p);
        if (r != 0) {
		printf("Error creating Buffer-01\n");
		exit(-1);
	}
#if 0
	circular_buffer_01_p->unwrapped_read_start_ptr = 4985;
#endif

	for (i = 0; i < 10; ++i) {
		connsat_buffer_printf(circular_buffer_01_p, "%3d: %s-%d: Buffer-Printf-Test\n", i, __FILE__, __LINE__);

		byte_count = circular_buffer_01_p->circular_buffer_used_bytes;

		r = circular_buffer_calculations(circular_buffer_01_p);
		if (r < 0) {
			printf("%s-%d(): circular_buffer_calculations() Error!\n", __FILE__, __LINE__);
			break;
		}

#if 0
		r = circular_buffer_stream_write(
			circular_buffer_01_p,
			1	/* stdout */
			);
#endif
	}

#if 1
	while (circular_buffer_01_p->circular_buffer_used_bytes) {
		r = circular_buffer_stream_write(
			circular_buffer_01_p,
			1	/* stdout */
			);

#if 1
		if (circular_buffer_01_p->circular_buffer_used_bytes) {
			printf("\n%s-%d(): !!!WRAP!!!\n", __FILE__, __LINE__);
		}
#endif

		if (r < 0) {
			printf("%s-%d(): circular_buffer_stream_write() Error!\n", __FILE__, __LINE__);
			break;
		}
	}
#endif

	return 0;	/* ok */
}
#endif

