
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "connsat_buffer_printf.h"
#include "connsat_circular_buffer_algorithm.h"

char stagging_buffer[20480];

int
connsat_buffer_printf (circular_buffer_t *circular_buffer_p, char *fmt, ...)
{
	va_list ap;
	int n;
	int buffer_size;
	int r;
	int max_buffer_transfer;
	char *p;

	va_start(ap, fmt);
	n = vsnprintf(stagging_buffer, sizeof stagging_buffer, fmt, ap);
	va_end(ap);

	/* n < 0 means error */
	/* n > 0 means character count */
	/* n > buffer_size means output truncated */

	if (n < 0) {

		printf("%s-%d(): vsnprintf() Error!\r\n", __FILE__, __LINE__);

	} else if (n > sizeof stagging_buffer) {

		printf("%s-%d(): Truncation Error!\r\n", __FILE__, __LINE__);

	} else if (n > (circular_buffer_p->circular_buffer_allocated_size - circular_buffer_p->circular_buffer_used_bytes)) {

		printf("%s-%d(): Circular Buffer Truncation Error! (%d, %d, %d)\r\n", __FILE__, __LINE__,
			n,
			circular_buffer_p->circular_buffer_allocated_size,
			circular_buffer_p->circular_buffer_used_bytes);

		/* max room left in circular buffer */
		n = (circular_buffer_p->circular_buffer_allocated_size - circular_buffer_p->circular_buffer_used_bytes);
	}

	p = stagging_buffer;

	while (n > 0) {

		r = circular_buffer_calculations(circular_buffer_p);
		if (r < 0) {
			printf("%s-%d(): circular_buffer_calculations() Error!\r\n", __FILE__, __LINE__);
			break;
		}

		/* biggest chunk we can write this pass */
		max_buffer_transfer = circular_buffer_p->calculated_buffer_max_writeable_chunk;

		if (max_buffer_transfer > n) {
			max_buffer_transfer = n;
		}

		if (max_buffer_transfer >= 1) {
			memcpy(
				&circular_buffer_p->circular_buffer_data_p[circular_buffer_p->calculated_buffer_wrapped_write_start_ptr],
				p,
				max_buffer_transfer);

			circular_buffer_p->circular_buffer_used_bytes += max_buffer_transfer;
			p += max_buffer_transfer;
			n -= max_buffer_transfer;

		} else {
			break;
		}
	}

#if 0
	r = circular_buffer_dump(circular_buffer_p);
#endif

	return 0;	/* ok */
}

