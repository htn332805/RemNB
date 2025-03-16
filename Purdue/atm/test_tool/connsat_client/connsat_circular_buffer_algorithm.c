
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <errno.h>

#include "connsat_main.h"
#include "connsat_circular_buffer_algorithm.h"
#include "connsat_ascii.h"

// #define CIRCULAR_BUFFER_DUMP_DEBUG 1

#if 1
#define NEWLINE "\r\n"
#else
#define NEWLINE "\n"
#endif



// #define CIRCULAR_BUFFER_WRAP_DEBUG 1




// #define STANDALONE_CIRCULAR_BUFFER 1
	// STANDALONE MODE FOR DEVELOPMENT TESTING AND DEBUGGING


// #define STANDALONE_CIRCULAR_BUFFER_MODE_1 1
	// MODE_1 is CIRCULAR_BUFFER_MATH_TEST


// #define STANDALONE_CIRCULAR_BUFFER_MODE_2 1
	// MODE_2 is CIRCULAR_BUFFER_THROUGHPUT_TEST
		// read file "aaaa", create-truncate and write output file "bbbb1"


// #define STANDALONE_CIRCULAR_BUFFER_MODE_3 1
	// MODE_3 is CIRCULAR_BUFFER_THROUGHPUT_TEST
		// read file "aaaa", create-truncate and write output file "bbbb1", "bbbb2", "bbbb3"


#ifdef STANDALONE_CIRCULAR_BUFFER

#ifdef STANDALONE_CIRCULAR_BUFFER_MODE_1

#define CIRCULAR_BUFFER_MATH_TEST 1
#define CIRCULAR_BUFFER_ALGORITHM_DEBUG 1
#define CIRCULAR_BUFFER_MATH_TEST_BUFFER_SIZE 5

#elif STANDALONE_CIRCULAR_BUFFER_MODE_2	/* STANDALONE_CIRCULAR_BUFFER_MODE_1 */

#define CIRCULAR_BUFFER_THROUGHPUT_TEST 1

// #define CIRCULAR_BUFFER_ALGORITHM_DEBUG 1
// #define CIRCULAR_BUFFER_ALGORITHM_DEBUG_01 1
// #define CIRCULAR_BUFFER_ALGORITHM_DEBUG_02 1
// #define CIRCULAR_BUFFER_ALGORITHM_DEBUG_03 1
// #define CIRCULAR_BUFFER_ALGORITHM_DEBUG_04 1

// #define CIRCULAR_BUFFER_STREAM_READ_BUFFER_DUMP 1
// #define CIRCULAR_BUFFER_STREAM_WRITE_BUFFER_DUMP 1
// #define CIRCULAR_BUFFER_BLOCK_TRANSFER_BUFFER_DUMP 1
// #define CIRCULAR_BUFFER_THROUGPUT_TRACE 1

// #define CIRCULAR_BUFFER_WRAP_DEBUG 1

#elif STANDALONE_CIRCULAR_BUFFER_MODE_3	/* STANDALONE_CIRCULAR_BUFFER_MODE_3 */

#define CIRCULAR_BUFFER_THROUGHPUT_TEST 1

// #define CIRCULAR_BUFFER_ALGORITHM_DEBUG 1
// #define CIRCULAR_BUFFER_ALGORITHM_DEBUG_01 1
// #define CIRCULAR_BUFFER_ALGORITHM_DEBUG_02 1
// #define CIRCULAR_BUFFER_ALGORITHM_DEBUG_03 1
// #define CIRCULAR_BUFFER_ALGORITHM_DEBUG_04 1
// #define CIRCULAR_BUFFER_ALGORITHM_DEBUG_14 1
// #define CIRCULAR_BUFFER_ALGORITHM_DEBUG_24 1

// #define CIRCULAR_BUFFER_STREAM_READ_BUFFER_DUMP 1
// #define CIRCULAR_BUFFER_STREAM_WRITE_BUFFER_DUMP 1
// #define CIRCULAR_BUFFER_BLOCK_TRANSFER_BUFFER_DUMP 1
// #define CIRCULAR_BUFFER_MULTI_BLOCK_TRANSFER_BUFFER_DUMP 1
// #define CIRCULAR_BUFFER_THROUGPUT_TRACE 1

// #define CIRCULAR_BUFFER_WRAP_DEBUG 1

#else

// default to mode_2 if none are specified
#define CIRCULAR_BUFFER_THROUGHPUT_TEST 1

#endif

#endif /* STANDALONE_CIRCULAR_BUFFER */

#ifdef STANDALONE_CIRCULAR_BUFFER
int
main()
{
#ifdef CIRCULAR_BUFFER_MATH_TEST
	int circular_buffer_allocated_size;
	int circular_buffer_used_bytes;

	int unwrapped_read_start_ptr;		/* first used location */

	int max_readable_chunk;
	int max_writeable_chunk;
	int wrapped_write_start_ptr;
#endif

	int total_used_space;
	int total_free_space;

#ifdef CIRCULAR_BUFFER_THROUGHPUT_TEST
	int fd_in;
	int fd_out_1;
#ifdef STANDALONE_CIRCULAR_BUFFER_MODE_3
	int fd_out_2;
	int fd_out_3;
	int max_multi_buffer_transfer;
#endif
	int total_buffered_bytes;
	int read_flags;
	int input_eof;
#endif

	int r;
	int loop_count;

#ifdef CIRCULAR_BUFFER_THROUGHPUT_TEST
	fd_in = open("aaaa", O_RDONLY);
	if (fd_in == -1) {
		printf("fd_in==%d"NEWLINE, fd_in);
		perror("fd_in: ");
		exit(1);
	}
 
	fd_out_1 = open("bbbb1", O_CREAT | O_WRONLY | O_TRUNC,
		S_IRUSR |	/* user has read permission */
		S_IWUSR |	/* user has write permission */
		S_IRGRP |	/* group has read permission */
		S_IWGRP |	/* group has write permission */
		S_IROTH	|	/* others have read permission */
		S_IWOTH		/* others have read permission */
		);
	if (fd_out_1 == -1) {
		printf("fd_out_1==%d"NEWLINE, fd_out_1);
		perror("fd_out_1: ");
		exit(1);
	}

#ifdef STANDALONE_CIRCULAR_BUFFER_MODE_3
	fd_out_2 = open("bbbb2", O_CREAT | O_WRONLY | O_TRUNC,
		S_IRUSR |	/* user has read permission */
		S_IWUSR |	/* user has write permission */
		S_IRGRP |	/* group has read permission */
		S_IWGRP |	/* group has write permission */
		S_IROTH	|	/* others have read permission */
		S_IWOTH		/* others have read permission */
		);
	if (fd_out_2 == -1) {
		printf("fd_out_2==%d"NEWLINE, fd_out_2);
		perror("fd_out_2: ");
		exit(1);
	}

	fd_out_3 = open("bbbb3", O_CREAT | O_WRONLY | O_TRUNC,
		S_IRUSR |	/* user has read permission */
		S_IWUSR |	/* user has write permission */
		S_IRGRP |	/* group has read permission */
		S_IWGRP |	/* group has write permission */
		S_IROTH	|	/* others have read permission */
		S_IWOTH		/* others have read permission */
		);
	if (fd_out_3 == -1) {
		printf("fd_out_3==%d"NEWLINE, fd_out_3);
		perror("fd_out_3: ");
		exit(1);
	}

#endif

	circular_buffer_t *circular_buffer_01_p;
	circular_buffer_t *circular_buffer_02_p;
	circular_buffer_t *circular_buffer_03_p;
	circular_buffer_t *circular_buffer_04_p;

#ifdef STANDALONE_CIRCULAR_BUFFER_MODE_3
	circular_buffer_t *circular_buffer_14_p;
	circular_buffer_t *circular_buffer_24_p;
#endif

#if 0
	r = circular_buffer_init("Buffer-01", 5, &circular_buffer_01_p);
	if (r != 0) {
		printf("Error creating Buffer-01"NEWLINE);
		exit(-1);
	}
	r = circular_buffer_init("Buffer-02", 15, &circular_buffer_02_p);
	if (r != 0) {
		printf("Error creating Buffer-02"NEWLINE);
		exit(-1);
	}
	r = circular_buffer_init("Buffer-03", 25, &circular_buffer_03_p);
	if (r != 0) {
		printf("Error creating Buffer-03"NEWLINE);
		exit(-1);
	}
	r = circular_buffer_init("Buffer-04", 64, &circular_buffer_04_p);
	if (r != 0) {
		printf("Error creating Buffer-04"NEWLINE);
		exit(-1);
	}

#ifdef STANDALONE_CIRCULAR_BUFFER_MODE_3

	r = circular_buffer_init("Buffer-14", 64, &circular_buffer_14_p);
	if (r != 0) {
		printf("Error creating Buffer-14"NEWLINE);
		exit(-1);
	}

	r = circular_buffer_init("Buffer-24", 64, &circular_buffer_24_p);
	if (r != 0) {
		printf("Error creating Buffer-24"NEWLINE);
		exit(-1);
	}
#endif

#else
	r = circular_buffer_init("Buffer-01", 25, &circular_buffer_01_p);
	if (r != 0) {
		printf("Error creating Buffer-01"NEWLINE);
		exit(-1);
	}
	r = circular_buffer_init("Buffer-02", 15, &circular_buffer_02_p);
	if (r != 0) {
		printf("Error creating Buffer-02"NEWLINE);
		exit(-1);
	}
	r = circular_buffer_init("Buffer-03", 14, &circular_buffer_03_p);
	if (r != 0) {
		printf("Error creating Buffer-03"NEWLINE);
		exit(-1);
	}
	r = circular_buffer_init("Buffer-04", 5, &circular_buffer_04_p);
	if (r != 0) {
		printf("Error creating Buffer-04"NEWLINE);
		exit(-1);
	}

#ifdef STANDALONE_CIRCULAR_BUFFER_MODE_3

	r = circular_buffer_init("Buffer-14", 5, &circular_buffer_14_p);
	if (r != 0) {
		printf("Error creating Buffer-10"NEWLINE);
		exit(-1);
	}

	r = circular_buffer_init("Buffer-24", 5, &circular_buffer_24_p);
	if (r != 0) {
		printf("Error creating Buffer-20"NEWLINE);
		exit(-1);
	}
#endif

#endif

	read_flags = 0;
	input_eof = 0;

	loop_count = 0;

	while (1) {
		++loop_count;

#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG_01
		printf("========================================================================="NEWLINE);
		printf("========================================================================="NEWLINE);
#endif

		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		r = circular_buffer_calculations(circular_buffer_01_p);

#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG_01
		r = circular_buffer_dump(circular_buffer_01_p);
#endif

		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

		r = circular_buffer_calculations(circular_buffer_02_p);

#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG_02
		r = circular_buffer_dump(circular_buffer_02_p);
#endif

		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

		r = circular_buffer_calculations(circular_buffer_03_p);

#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG_03
		r = circular_buffer_dump(circular_buffer_03_p);
#endif

		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

		r = circular_buffer_calculations(circular_buffer_04_p);

#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG_04
		r = circular_buffer_dump(circular_buffer_04_p);
#endif

#ifdef STANDALONE_CIRCULAR_BUFFER_MODE_3

		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

		r = circular_buffer_calculations(circular_buffer_14_p);

#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG_14
		r = circular_buffer_dump(circular_buffer_14_p);
#endif

		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

		r = circular_buffer_calculations(circular_buffer_24_p);

#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG_24
		r = circular_buffer_dump(circular_buffer_24_p);
#endif

#endif

		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

// @@@
#ifdef CIRCULAR_BUFFER_THROUGPUT_TRACE
		printf(""NEWLINE);
		printf("READ-INPUT-TO-BUFFER_01: input_eof==0x%04x"NEWLINE, input_eof);
		printf(""NEWLINE);
#endif

		if (input_eof == 0) {
			/* didn't detect input EOF yet */

			/* read input_fd into buffer_01 */
			r = circular_buffer_stream_read(
				circular_buffer_01_p,
				fd_in,	/* input_fd */
				&read_flags
				);

			if (read_flags & 0x0001) {	/* EOF */
				/* detected input EOF */

				input_eof = 1;
			}
		}

#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG_01
		r = circular_buffer_dump(circular_buffer_01_p);
#endif

		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

#ifdef CIRCULAR_BUFFER_THROUGPUT_TRACE
		printf(""NEWLINE);
		printf("TRANSFER-BUFFER-01-02:"NEWLINE);
		printf(""NEWLINE);
#endif

		/* transfer buffer_01 to buffer_02 */
		r = circular_buffer_block_transfer(
			circular_buffer_01_p,
			circular_buffer_02_p
			);

#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG_01
		r = circular_buffer_dump(circular_buffer_01_p);
#endif
#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG_02
		r = circular_buffer_dump(circular_buffer_02_p);
#endif

		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

#ifdef CIRCULAR_BUFFER_THROUGPUT_TRACE
		printf(""NEWLINE);
		printf("TRANSFER-BUFFER-02-03:"NEWLINE);
		printf(""NEWLINE);
#endif

		/* transfer buffer_02 to buffer_03 */
		r = circular_buffer_block_transfer(
			circular_buffer_02_p,
			circular_buffer_03_p
			);

#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG_02
		r = circular_buffer_dump(circular_buffer_02_p);
#endif
#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG_03
		r = circular_buffer_dump(circular_buffer_03_p);
#endif

		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

#ifdef CIRCULAR_BUFFER_THROUGPUT_TRACE
		printf(""NEWLINE);
		printf("TRANSFER-BUFFER-03-04:"NEWLINE);
		printf(""NEWLINE);
#endif

#ifdef STANDALONE_CIRCULAR_BUFFER_MODE_3

#ifdef CIRCULAR_BUFFER_MULTI_BLOCK_TRANSFER_BUFFER_DUMP
		printf("%4d: We Think BUFFER-03 can output up to %d bytes"NEWLINE,
			loop_count,
			circular_buffer_03_p->max_readable_chunk);

		printf("%4d: We Think BUFFER-14 can input up to %d bytes"NEWLINE,
			loop_count,
			circular_buffer_14_p->max_writeable_chunk);

		printf("%4d: We Think BUFFER-24 can input up to %d bytes"NEWLINE,
			loop_count,
			circular_buffer_24_p->max_writeable_chunk);
#endif

		/* calculate max_multi_buffer_transfer */
		max_multi_buffer_transfer = circular_buffer_03_p->max_readable_chunk;

		if (circular_buffer_04_p->max_writeable_chunk < max_multi_buffer_transfer) {
			max_multi_buffer_transfer = circular_buffer_04_p->max_writeable_chunk;
		}

		if (circular_buffer_14_p->max_writeable_chunk < max_multi_buffer_transfer) {
			max_multi_buffer_transfer = circular_buffer_14_p->max_writeable_chunk;
		}

		if (circular_buffer_24_p->max_writeable_chunk < max_multi_buffer_transfer) {
			max_multi_buffer_transfer = circular_buffer_24_p->max_writeable_chunk;
		}

#ifdef CIRCULAR_BUFFER_MULTI_BLOCK_TRANSFER_BUFFER_DUMP
		printf("%4d: max_multi_buffer_transfer is up to %d bytes"NEWLINE,
			loop_count,
			max_multi_buffer_transfer);

		printf(""NEWLINE);
#endif

		if (max_multi_buffer_transfer >= 1) {

			/* transfer buffer_03 to buffer_04 */
#if 0
			r = circular_buffer_block_transfer(
				circular_buffer_03_p,
				circular_buffer_04_p
				);
#else
			r = circular_buffer_multi_block_transfer(
				circular_buffer_03_p,
				circular_buffer_04_p,
				max_multi_buffer_transfer
				);
#endif

			/* transfer buffer_03 to buffer_14 */
			r = circular_buffer_multi_block_transfer(
				circular_buffer_03_p,
				circular_buffer_14_p,
				max_multi_buffer_transfer
				);

			/* transfer buffer_03 to buffer_24 */
			r = circular_buffer_multi_block_transfer(
				circular_buffer_03_p,
				circular_buffer_24_p,
				max_multi_buffer_transfer
				);

			/* advance buffer_03 by max_multi_buffer_transfer */
			r = circular_buffer_multi_block_transfer_complete(
				circular_buffer_03_p,
				max_multi_buffer_transfer
				);

		}
#else
		/* transfer buffer_03 to buffer_04 */
		r = circular_buffer_block_transfer(
			circular_buffer_03_p,
			circular_buffer_04_p
			);
#endif

#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG_03
		r = circular_buffer_dump(circular_buffer_03_p);
#endif
#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG_04
		r = circular_buffer_dump(circular_buffer_04_p);
#endif

		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

#ifdef CIRCULAR_BUFFER_THROUGPUT_TRACE
		printf(""NEWLINE);
		printf("WRITE-OUTPUT-FROM-BUFFER_04:"NEWLINE);
		printf(""NEWLINE);
#endif


		/* write output_fd from buffer_04 */
		r = circular_buffer_stream_write(
			circular_buffer_04_p,
			fd_out_1	/* output_fd */
			);

#ifdef STANDALONE_CIRCULAR_BUFFER_MODE_3

		/* write output_fd from buffer_14 */
		r = circular_buffer_stream_write(
			circular_buffer_14_p,
			fd_out_2	/* output_fd */
			);

		/* write output_fd from buffer_24 */
		r = circular_buffer_stream_write(
			circular_buffer_24_p,
			fd_out_3	/* output_fd */
			);
#endif

#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG_01
		r = circular_buffer_dump(circular_buffer_04_p);
#endif

		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
		/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */



		total_buffered_bytes = circular_buffer_01_p->circular_buffer_used_bytes;
		total_buffered_bytes += circular_buffer_02_p->circular_buffer_used_bytes;
		total_buffered_bytes += circular_buffer_03_p->circular_buffer_used_bytes;
		total_buffered_bytes += circular_buffer_04_p->circular_buffer_used_bytes;

#ifdef CIRCULAR_BUFFER_THROUGPUT_TRACE
		printf(""NEWLINE);
		printf("TOTAL-BUFFERED_BYTES: %d, TOTAL-LOOP-COUNT: %d"NEWLINE, total_buffered_bytes, loop_count);
		printf(""NEWLINE);
#endif

		if (input_eof != 0) {
			if (total_buffered_bytes == 0) {
				break;
			}
		}

#if 0
		// LOOP_COUNT: enable this code section for debugging runaway loop.
		if (loop_count >= 10) {
			printf(""NEWLINE);
			printf("LOOP_COUNT TERMINATION: %d"NEWLINE, loop_count);
			printf(""NEWLINE);
			break;
		}
#endif
	}
#endif /* CIRCULAR_BUFFER_THROUGHPUT_TEST */

#ifdef CIRCULAR_BUFFER_MATH_TEST

	circular_buffer_allocated_size = CIRCULAR_BUFFER_MATH_TEST_BUFFER_SIZE;

	/* for every possible allocated buffer size... */
	for (circular_buffer_used_bytes = 0; circular_buffer_used_bytes <= circular_buffer_allocated_size; ++circular_buffer_used_bytes) {

#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG
		printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"NEWLINE);
		printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"NEWLINE);
#endif

		total_used_space = circular_buffer_used_bytes;
		total_free_space = circular_buffer_allocated_size - circular_buffer_used_bytes;

#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG
		printf("total_used_space==%d, total_free_space==%d"NEWLINE, total_used_space, total_free_space);
		printf(""NEWLINE);
#endif

		/* for every possible start point in the allocated buffer... */
		for (unwrapped_read_start_ptr = 0; unwrapped_read_start_ptr < circular_buffer_allocated_size; ++unwrapped_read_start_ptr) {

			r = circular_buffer_math_calculations (
				circular_buffer_allocated_size,
				unwrapped_read_start_ptr,
				circular_buffer_used_bytes,
				&max_readable_chunk,
				&wrapped_write_start_ptr,
				&max_writeable_chunk
				);

#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG
			printf("\t\tcircular_buffer_allocated_size==%d, unwrapped_read_start_ptr==%d, circular_buffer_used_bytes==%d,"NEWLINE
				"\t\t&max_readable_chunk==%d, &wrapped_write_start_ptr==%d, &max_writeable_chunk==%d"NEWLINE,

				circular_buffer_allocated_size,
				unwrapped_read_start_ptr,
				circular_buffer_used_bytes,
				max_readable_chunk,
				wrapped_write_start_ptr,
				max_writeable_chunk
				);
			printf(""NEWLINE);
#endif

		}
	}
#endif /* CIRCULAR_BUFFER_MATH_TEST */

	return 0;	/* ok */
}

#endif /* STANDALONE_CIRCULAR_BUFFER */

int
circular_buffer_init (
	char *new_buffer_name_p,
	int allocated_size,
	circular_buffer_t **new_circular_buffer_pp
)
{
	circular_buffer_t *create_circular_buffer_p;

	create_circular_buffer_p = malloc(sizeof(circular_buffer_t));

	if (create_circular_buffer_p == NULL) {
		/* out of memory */
		return -1;
	}

	memset(create_circular_buffer_p, 0, sizeof(circular_buffer_t));

	// @@@
	// printf("CREATED CIRCULAR BUFFER HEADER-%s, %ld bytes, address==%p"NEWLINE, new_buffer_name_p, sizeof(circular_buffer_t), create_circular_buffer_p);

	create_circular_buffer_p->circular_buffer_data_p = malloc(allocated_size);
	if (create_circular_buffer_p->circular_buffer_data_p == NULL) {
		/* out of memory */

		free(create_circular_buffer_p);
		return -2;
	}

	// @@@
	// printf("CREATED CIRCULAR BUFFER DATA-%s, %d bytes, address==%p"NEWLINE,
	// new_buffer_name_p, allocated_size, create_circular_buffer_p->circular_buffer_data_p);

	memset(create_circular_buffer_p->circular_buffer_data_p, 0, allocated_size);

	strncpy(create_circular_buffer_p->buffer_name, new_buffer_name_p, sizeof(create_circular_buffer_p->buffer_name));

	create_circular_buffer_p->circular_buffer_allocated_size = allocated_size;

	create_circular_buffer_p->unwrapped_read_start_ptr = 0;
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(create_circular_buffer_p);
	create_circular_buffer_p->circular_buffer_used_bytes = 0;
	create_circular_buffer_p->calculated_buffer_max_readable_chunk = 0;
	create_circular_buffer_p->calculated_buffer_wrapped_write_start_ptr = 0;
	create_circular_buffer_p->calculated_buffer_max_writeable_chunk = 0;

	if (new_circular_buffer_pp != NULL) {
		*new_circular_buffer_pp = create_circular_buffer_p;
	}

	return 0; /* ok */
}

int
circular_buffer_dump (
	circular_buffer_t *circular_buffer_p
)
{
	printf("\t\t%s==%d, unwrapped_read_start_ptr==%d, circular_buffer_used_bytes==%d,"NEWLINE
		"\t\t&max_readable_chunk==%d, &wrapped_write_start_ptr==%d, &max_writeable_chunk==%d"NEWLINE,

		circular_buffer_p->buffer_name,
		circular_buffer_p->circular_buffer_allocated_size,

		circular_buffer_p->unwrapped_read_start_ptr,
		circular_buffer_p->circular_buffer_used_bytes,
		circular_buffer_p->calculated_buffer_max_readable_chunk,
		circular_buffer_p->calculated_buffer_wrapped_write_start_ptr,
		circular_buffer_p->calculated_buffer_max_writeable_chunk
		);
	printf(""NEWLINE);

CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);

	return 0; /* ok */
}

int
circular_buffer_view (
	circular_buffer_t *circular_buffer_p,
	char **buffer_view_pp,
	int *buffer_view_size_p
)
{
	char *malloc_buffer_p;
	int buffer_allocated_bytes;
	int buffer_index_offset;
	int i;

	if (circular_buffer_p == NULL) {
		return -1;
	}

	if (buffer_view_pp == NULL) {
		return -2;
	}

	if (buffer_view_size_p == NULL) {
		return -3;
	}

	malloc_buffer_p = malloc(circular_buffer_p->circular_buffer_used_bytes +2);

	if (malloc_buffer_p == NULL) {

		*buffer_view_pp = NULL;
		*buffer_view_size_p = 0;

		return -4;
	}

// BUG: memset(circular_buffer_p, 0, circular_buffer_p->circular_buffer_used_bytes + 2);
	memset(malloc_buffer_p, 0, circular_buffer_p->circular_buffer_used_bytes + 2);

	buffer_allocated_bytes = circular_buffer_p->circular_buffer_allocated_size;

	for (i = 0; i < circular_buffer_p->circular_buffer_used_bytes; ++i) {

		buffer_index_offset = (circular_buffer_p->unwrapped_read_start_ptr + i) %
			circular_buffer_p->circular_buffer_allocated_size;
//BUG			circular_buffer_p->circular_buffer_used_bytes;

		malloc_buffer_p[i] = circular_buffer_p->circular_buffer_data_p[buffer_index_offset];
	}

	*buffer_view_pp = malloc_buffer_p;
	*buffer_view_size_p = circular_buffer_p->circular_buffer_used_bytes;

#if 0
	printf("\t\t%s==%d, unwrapped_read_start_ptr==%d, circular_buffer_used_bytes==%d,"NEWLINE
		"\t\tcalc_max_readable_chunk==%d, calc_wrapped_write_start_ptr==%d, calc_max_writeable_chunk==%d"NEWLINE,

		circular_buffer_p->buffer_name,
		circular_buffer_p->circular_buffer_allocated_size,

		circular_buffer_p->unwrapped_read_start_ptr,
		circular_buffer_p->circular_buffer_used_bytes,
		circular_buffer_p->calculated_buffer_max_readable_chunk,
		circular_buffer_p->calculated_buffer_wrapped_write_start_ptr,
		circular_buffer_p->calculated_buffer_max_writeable_chunk
		);
	printf(""NEWLINE);
#endif

	return 0; /* ok */
}

int
circular_buffer_calculations (
	circular_buffer_t *circular_buffer_p
)
{
	int r;

	// @@@
	// printf("CALC: CIRCULAR BUFFER HEADER-%s, %ld bytes, address==%p"NEWLINE,
	// circular_buffer_p->buffer_name, sizeof(circular_buffer_t), circular_buffer_p);

	// @@@
	// printf("CALC CIRCULAR BUFFER DATA-%s, %d bytes, address==%p"NEWLINE,
	// circular_buffer_p->buffer_name, circular_buffer_p->circular_buffer_allocated_size,
	// circular_buffer_p->circular_buffer_data_p);

	// @@@
	// printf("CALC-BEFORE: ");
	// circular_buffer_dump(circular_buffer_p);

CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);
	if (circular_buffer_p->unwrapped_read_start_ptr >= circular_buffer_p->circular_buffer_allocated_size) {

#ifdef CIRCULAR_BUFFER_WRAP_DEBUG
		printf("ERROR_1: %s-%d: unwrapped_read_start_ptr==%d, circular_buffer_allocated_size==%d"NEWLINE,
			__FILE__, __LINE__,
			circular_buffer_p->unwrapped_read_start_ptr, circular_buffer_p->circular_buffer_allocated_size);
#endif

//WRAP@@@@@@@
		//@@@THIS_IS_THE_CORRECT_PLACE_TO_FIX_WRAP:
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);
		circular_buffer_p->unwrapped_read_start_ptr %= circular_buffer_p->circular_buffer_allocated_size;
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);
	}

CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);
	if (circular_buffer_p->unwrapped_read_start_ptr < 0) {
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);

#ifdef CIRCULAR_BUFFER_WRAP_DEBUG
		printf("ERROR_1: %s-%d: unwrapped_read_start_ptr<0==%d, circular_buffer_allocated_size==%d"NEWLINE,
			__FILE__, __LINE__,
			circular_buffer_p->unwrapped_read_start_ptr, circular_buffer_p->circular_buffer_allocated_size);
#endif

//WRAP@@@@@@@
		//@@@THIS_IS_THE_CORRECT_PLACE_TO_FIX_WRAP:
		// circular_buffer_p->unwrapped_read_start_ptr %= circular_buffer_p->circular_buffer_allocated_size;
	}

CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);
	r = circular_buffer_math_calculations (
		circular_buffer_p->circular_buffer_allocated_size,
		circular_buffer_p->unwrapped_read_start_ptr,
		circular_buffer_p->circular_buffer_used_bytes,
		&circular_buffer_p->calculated_buffer_max_readable_chunk,
		&circular_buffer_p->calculated_buffer_wrapped_write_start_ptr,
		&circular_buffer_p->calculated_buffer_max_writeable_chunk
		);

	if (r != 0) {
		return -1;	/* error */
	}

	// @@@
	// printf("CALC-AFTER: ");
	// circular_buffer_dump(circular_buffer_p);

	return 0;	/* ok */
}

int
circular_buffer_stream_read (
	circular_buffer_t *circular_buffer_p,
	int fd,
	int *flags_p
)
{
	int i;
	int r;

	if (circular_buffer_p->calculated_buffer_max_writeable_chunk >= 1) {

		// read fd into buffer */
		r = read(fd,
			&circular_buffer_p->circular_buffer_data_p[circular_buffer_p->calculated_buffer_wrapped_write_start_ptr],
			circular_buffer_p->calculated_buffer_max_writeable_chunk);

		if (r == -1) {
			// errno
#ifdef CIRCULAR_BUFFER_STREAM_READ_BUFFER_DUMP
			printf("\tREAD-ERROR: %d"NEWLINE, errno);
			perror("READ-ERROR: ");
#endif
			return -1;
		} else if (r == 0) {

			// EOF
			*flags_p = 0x0001;	/* EOF */
// @@@
#ifdef CIRCULAR_BUFFER_STREAM_READ_BUFFER_DUMP
			printf("\tREAD-EOF"NEWLINE);
#endif

		} else {
// @@@
#ifdef CIRCULAR_BUFFER_STREAM_READ_BUFFER_DUMP
			printf("\tREAD-BYTES: %d, \"", r);
			for (i = 0; i < r; ++i) {
				putc(circular_buffer_p->circular_buffer_data_p[i], stdout);
			}
			printf("\""NEWLINE);
#endif
			circular_buffer_p->circular_buffer_used_bytes += r;

			/* recalc buffer */
			r = circular_buffer_calculations(circular_buffer_p);

			if (r != 0) {
#ifdef CIRCULAR_BUFFER_STREAM_READ_BUFFER_DUMP
				printf("\tcircular_buffer_calculations() returned: %d, \"", r);
#endif
				return -1;	/* error */
			}
		}
	}

	return 0;	/* ok */
}

int
circular_buffer_stream_write (
	circular_buffer_t *circular_buffer_p,
	int fd
)
{
	int i;
	int r;

	if (circular_buffer_p->calculated_buffer_max_readable_chunk >= 1) {

CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);
		// write fd into buffer */
		r = write(fd,
//WRAP@@@@@@@
			&circular_buffer_p->circular_buffer_data_p[circular_buffer_p->unwrapped_read_start_ptr],
			circular_buffer_p->calculated_buffer_max_readable_chunk);

		if (r == -1) {
			// errno
			return -1;
		} else {

#ifdef CIRCULAR_BUFFER_STREAM_WRITE_BUFFER_DUMP
			printf("\tWRITE-BYTES: %d, \"", r);
			for (i = 0; i < r; ++i) {
//WRAP@@@@@@@
//WRAP@@@@@@@
//WRAP@@@@@@@
//WRAP@@@@@@@
//WRAP@@@@@@@
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);
				putc( circular_buffer_p->circular_buffer_data_p[(circular_buffer_p->unwrapped_read_start_ptr + i) %
					circular_buffer_p->circular_buffer_allocated_size], stdout );
			}
			printf("\""NEWLINE);
#endif

			circular_buffer_p->circular_buffer_used_bytes -= r;
//FX@@@@@@@ circular_buffer_p->unwrapped_read_start_ptr;
//WRAP@@@@@@@
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);
			circular_buffer_p->unwrapped_read_start_ptr += r;
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);
			if (circular_buffer_p->unwrapped_read_start_ptr >= circular_buffer_p->circular_buffer_allocated_size) {
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);
				circular_buffer_p->unwrapped_read_start_ptr %= circular_buffer_p->circular_buffer_allocated_size;
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);

#ifdef CIRCULAR_BUFFER_WRAP_DEBUG
				printf("FIX_1: %s-%d: circular_buffer_p->unwrapped_read_start_ptr==%d"NEWLINE,
					__FILE__, __LINE__,
					circular_buffer_p->unwrapped_read_start_ptr);
#endif
			}

CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);
			/* recalc buffer */
			r = circular_buffer_calculations(circular_buffer_p);
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);

			if (r != 0) {
				return -1;	/* error */
			}
		}
	}

	return 0;	/* ok */
}

int
circular_buffer_getc (
	circular_buffer_t *circular_buffer_p,
	int *getc_char_p
)
{
	int r;
	int read_count;

	read_count = 0;

	if (circular_buffer_p->calculated_buffer_max_readable_chunk >= 1) {

CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);
		// write fd into buffer */
		*getc_char_p =
			circular_buffer_p->circular_buffer_data_p[circular_buffer_p->unwrapped_read_start_ptr];

			circular_buffer_p->circular_buffer_used_bytes -= 1;

//WRAP@@@@@@@ r is not initialized!!!
//WRAP@@@@@@@ r is not initialized!!!
//WRAP@@@@@@@ r is not initialized!!!
//WRAP@@@@@@@ r is not initialized!!!
//WRAP@@@@@@@ r is not initialized!!!

CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);

			// BUG: circular_buffer_p->unwrapped_read_start_ptr += r;
			circular_buffer_p->unwrapped_read_start_ptr += 1;

CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);

			if (circular_buffer_p->unwrapped_read_start_ptr >= circular_buffer_p->circular_buffer_allocated_size) {

CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);
				circular_buffer_p->unwrapped_read_start_ptr %= circular_buffer_p->circular_buffer_allocated_size;
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);

#ifdef CIRCULAR_BUFFER_WRAP_DEBUG
				printf("FIX_7: %s-%d: circular_buffer_p->unwrapped_read_start_ptr==%d"NEWLINE,
					__FILE__, __LINE__,
					circular_buffer_p->unwrapped_read_start_ptr);
#endif
			}

			++read_count;

			/* recalc buffer */
			r = circular_buffer_calculations(circular_buffer_p);

			if (r != 0) {
				return -1;	/* error */
			}
	}

	return read_count;	/* ok */
}

int
circular_buffer_putc (
	circular_buffer_t *circular_buffer_p,
	int *putc_char_p
)
{
	int r;
	int write_count;

	write_count = 0;

	if (circular_buffer_p->calculated_buffer_max_writeable_chunk >= 1) {

CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);
		// write fd into buffer */
		circular_buffer_p->circular_buffer_data_p[circular_buffer_p->calculated_buffer_wrapped_write_start_ptr] =
			*putc_char_p;

			circular_buffer_p->circular_buffer_used_bytes += 1;

//WRAP@@@@@@@ r is not initialized!!!
//WRAP@@@@@@@ r is not initialized!!!
//WRAP@@@@@@@ r is not initialized!!!
//WRAP@@@@@@@ r is not initialized!!!
//WRAP@@@@@@@ r is not initialized!!!

CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);

			// BUG: circular_buffer_p->unwrapped_read_start_ptr += r;
			circular_buffer_p->calculated_buffer_wrapped_write_start_ptr += 1;

CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);

			if (circular_buffer_p->calculated_buffer_wrapped_write_start_ptr >= circular_buffer_p->circular_buffer_allocated_size) {

CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);
				circular_buffer_p->calculated_buffer_wrapped_write_start_ptr %= circular_buffer_p->circular_buffer_allocated_size;
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_p);

#ifdef CIRCULAR_BUFFER_WRAP_DEBUG
				printf("FIX_7: %s-%d: circular_buffer_p->calculated_buffer_wrapped_write_start_ptr==%d"NEWLINE,
					__FILE__, __LINE__,
					circular_buffer_p->calculated_buffer_wrapped_write_start_ptr);
#endif
			}

			++write_count;

			/* recalc buffer */
			r = circular_buffer_calculations(circular_buffer_p);

			if (r != 0) {
				return -1;	/* error */
			}
	}

	return write_count;	/* ok */
}

int
circular_buffer_block_transfer (
	circular_buffer_t *circular_buffer_1_p,
	circular_buffer_t *circular_buffer_2_p
)
{
	int i;
	int r;
	int max_buffer_transfer;

// unwrapped_read_start_ptr_01==0 &max_readable_chunk_01==5
// &wrapped_write_start_ptr_02==0, &max_writeable_chunk_02==15

	max_buffer_transfer = circular_buffer_1_p->calculated_buffer_max_readable_chunk;	/* most we can read from buffer_01 */
	if (circular_buffer_2_p->calculated_buffer_max_writeable_chunk <= max_buffer_transfer) {		/* most we can transfer to buffer_02 */
		max_buffer_transfer = circular_buffer_2_p->calculated_buffer_max_writeable_chunk;	/* most we can transfer from buffer_01 to buffer_02 */
	}

	if (max_buffer_transfer >= 1) {
// @@@
#ifdef CIRCULAR_BUFFER_BLOCK_TRANSFER_BUFFER_DUMP
		printf("\tyou can transfer %d bytes from %s[%d] to %s[%d], \"",
			max_buffer_transfer,
			circular_buffer_1_p->buffer_name,
			circular_buffer_1_p->unwrapped_read_start_ptr,
			circular_buffer_2_p->buffer_name,
			circular_buffer_2_p->wrapped_write_start_ptr
			);
		for (i = 0; i < max_buffer_transfer; ++i) {
//WRAP@@@@@@@ no wrap is being done here, may be ok
//WRAP@@@@@@@ no wrap is being done here, may be ok
//WRAP@@@@@@@ no wrap is being done here, may be ok
//WRAP@@@@@@@ no wrap is being done here, may be ok
//WRAP@@@@@@@ no wrap is being done here, may be ok
//WRAP@@@@@@@ no wrap is being done here, may be ok
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_1_p);
			putc(circular_buffer_1_p->circular_buffer_data_p[circular_buffer_1_p->unwrapped_read_start_ptr + i], stdout);
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_1_p);
		}
		printf("\""NEWLINE);
#endif
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_1_p);
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_2_p);
		memcpy(
			&circular_buffer_2_p->circular_buffer_data_p[circular_buffer_2_p->calculated_buffer_wrapped_write_start_ptr],
			&circular_buffer_1_p->circular_buffer_data_p[circular_buffer_1_p->unwrapped_read_start_ptr],
			max_buffer_transfer);

		circular_buffer_1_p->circular_buffer_used_bytes -= max_buffer_transfer;
//FX@@@@@@@ circular_buffer_1_p->unwrapped_read_start_ptr;
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_1_p);
		circular_buffer_1_p->unwrapped_read_start_ptr += max_buffer_transfer;
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_1_p);
		if (circular_buffer_1_p->unwrapped_read_start_ptr >= circular_buffer_1_p->circular_buffer_allocated_size) {
//WRAP@@@@@@@
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_1_p);
			circular_buffer_1_p->unwrapped_read_start_ptr %= circular_buffer_1_p->circular_buffer_allocated_size;
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_1_p);
#ifdef CIRCULAR_BUFFER_WRAP_DEBUG
			printf("FIX_2: %s-%d: circular_buffer_1_p->unwrapped_read_start_ptr==%d"NEWLINE,
				__FILE__, __LINE__,
				circular_buffer_1_p->unwrapped_read_start_ptr);
#endif
		}

		circular_buffer_2_p->circular_buffer_used_bytes += max_buffer_transfer;

		/* recalc buffer_01 */
		r = circular_buffer_calculations(circular_buffer_1_p);

		/* recalc buffer_02 */
		r = circular_buffer_calculations(circular_buffer_2_p);

	} else {
// @@@
#ifdef CIRCULAR_BUFFER_BLOCK_TRANSFER_BUFFER_DUMP
		printf("\tyou can transfer %d bytes from %s to %s"NEWLINE,
			max_buffer_transfer,
			circular_buffer_1_p->buffer_name,
			circular_buffer_2_p->buffer_name
		);
#endif
	}

	return 0;	/* ok */
}

int
circular_buffer_multi_block_transfer (
	circular_buffer_t *circular_buffer_1_p,
	circular_buffer_t *circular_buffer_2_p,
	int max_multi_buffer_transfer
)
{
	int i;
	int r;
	int max_buffer_transfer;

	max_buffer_transfer = max_multi_buffer_transfer;

	if (circular_buffer_1_p->calculated_buffer_max_writeable_chunk <= max_buffer_transfer) {		/* most we can transfer to buffer_02 */
		max_buffer_transfer = circular_buffer_1_p->calculated_buffer_max_readable_chunk;	/* most we can read from buffer_01 */
	}
	if (circular_buffer_2_p->calculated_buffer_max_writeable_chunk <= max_buffer_transfer) {		/* most we can transfer to buffer_02 */
		max_buffer_transfer = circular_buffer_2_p->calculated_buffer_max_writeable_chunk;	/* most we can transfer from buffer_01 to buffer_02 */
	}

	if (max_buffer_transfer != max_multi_buffer_transfer) {
		printf("ERROR_3: %s-%d: max_multi_buffer_transfer==%d, max_buffer_transfer==%d"NEWLINE,
			__FILE__, __LINE__,
			max_multi_buffer_transfer,
			max_buffer_transfer);
	}

	if (max_buffer_transfer >= 1) {
// @@@
#ifdef CIRCULAR_BUFFER_MULTI_BLOCK_TRANSFER_BUFFER_DUMP
		printf("\tyou can transfer %d bytes from %s[%d] to %s[%d], \"",
			max_buffer_transfer,
			circular_buffer_1_p->buffer_name,
			circular_buffer_1_p->unwrapped_read_start_ptr,
			circular_buffer_2_p->buffer_name,
			circular_buffer_2_p->calculated_buffer_wrapped_write_start_ptr
			);
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(create_circular_buffer_p);
		for (i = 0; i < max_buffer_transfer; ++i) {
//WRAP@@@@@@@, not wrapped?
//WRAP@@@@@@@, not wrapped?
//WRAP@@@@@@@, not wrapped?
//WRAP@@@@@@@, not wrapped?
//WRAP@@@@@@@, not wrapped?
//WRAP@@@@@@@, not wrapped?
			putc(circular_buffer_1_p->circular_buffer_data_p[circular_buffer_1_p->unwrapped_read_start_ptr + i], stdout);
		}
		printf("\""NEWLINE);
#endif
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_2_p);
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_1_p);
		memcpy(
			&circular_buffer_2_p->circular_buffer_data_p[circular_buffer_2_p->calculated_buffer_wrapped_write_start_ptr],
			&circular_buffer_1_p->circular_buffer_data_p[circular_buffer_1_p->unwrapped_read_start_ptr],
			max_buffer_transfer);

		circular_buffer_2_p->circular_buffer_used_bytes += max_buffer_transfer;

		/* recalc buffer_01 */
		r = circular_buffer_calculations(circular_buffer_1_p);

		/* recalc buffer_02 */
		r = circular_buffer_calculations(circular_buffer_2_p);

	} else {
// @@@
#ifdef CIRCULAR_BUFFER_BLOCK_TRANSFER_BUFFER_DUMP
		printf("\tyou can transfer %d bytes from %s to %s"NEWLINE,
			max_buffer_transfer,
			circular_buffer_1_p->buffer_name,
			circular_buffer_2_p->buffer_name
		);
#endif
	}

	return 0;	/* ok */
}

int
circular_buffer_multi_block_transfer_complete (
	circular_buffer_t *circular_buffer_1_p,
	int max_multi_buffer_transfer
)
{
	int max_buffer_transfer;
#ifdef CIRCULAR_BUFFER_DUMP_DEBUG
	int r;
	int i;
	char printable_buff[32];
	char *printable_char_abrev;
	unsigned char current_char;
#endif

	max_buffer_transfer = max_multi_buffer_transfer;

	if (max_buffer_transfer >= 1) {

#ifdef CIRCULAR_BUFFER_DUMP_DEBUG
		for (i = 0; i < max_buffer_transfer; ++i) {

			current_char =
				circular_buffer_1_p->circular_buffer_data_p[circular_buffer_1_p->unwrapped_read_start_ptr + i];

			printable_buff[0] = '\0';

			r = get_ascii_control_code_info(current_char, &printable_char_abrev, NULL);

			if (r == 0) {

				/* is a special control char */

				sprintf(printable_buff,
					"<%s>",
					printable_char_abrev
					);

			} else if (r != 0) {

				/* not a special control char */

				if (current_char >= 0x20 && current_char <= 0x7e) {
					sprintf(printable_buff,
						"\"%c\"",
						current_char
						);
				} else {
					sprintf(printable_buff,
						"\"???\""
						);
				}

			}

			printf("DUMP: %s-%d: index==%d/size==%d, ptr==%d, char==0x%02x-%s",
				__FILE__, __LINE__,
				i,
				max_buffer_transfer,
				circular_buffer_1_p->unwrapped_read_start_ptr + i,
				current_char,
				printable_buff);

			printf(""NEWLINE);
		}
#endif

		circular_buffer_1_p->circular_buffer_used_bytes -= max_buffer_transfer;
//FX@@@@@@@ circular_buffer_1_p->unwrapped_read_start_ptr;
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_1_p);
		circular_buffer_1_p->unwrapped_read_start_ptr += max_buffer_transfer;
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_1_p);
		if (circular_buffer_1_p->unwrapped_read_start_ptr >= circular_buffer_1_p->circular_buffer_allocated_size) {
//WRAP@@@@@@@
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_1_p);
			circular_buffer_1_p->unwrapped_read_start_ptr %= circular_buffer_1_p->circular_buffer_allocated_size;
CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(circular_buffer_1_p);
#ifdef CIRCULAR_BUFFER_WRAP_DEBUG
			printf("FIX_2: %s-%d: circular_buffer_1_p->unwrapped_read_start_ptr==%d"NEWLINE,
				__FILE__, __LINE__,
				circular_buffer_1_p->unwrapped_read_start_ptr);
#endif
		}

	}

	return 0;	/* ok */
}

int
circular_buffer_math_calculations (
	int circular_buffer_allocated_size,
	int unwrapped_read_start_ptr,		/* first used location */
	int circular_buffer_used_bytes,
	int *max_readable_chunk_p,
	int *wrapped_write_start_ptr_p,		/* next free location */
	int *max_writeable_chunk_p
)
{
	int unwrapped_read_end_ptr;		/* last used location */

	int unwrapped_write_start_ptr;		/* next free location */

	int wrapped_write_start_ptr;		/* next free location */

	int total_used_space;
	int total_free_space;

	int max_writeable_chunk;
	int max_readable_chunk;
	int max_readable_chunk_data;		/* xxx */
	int max_writeable_chunk_data;

	int chunk_range_readable_start;
	int chunk_range_readable_end;

	int chunk_range_writeable_start;
	int chunk_range_writeable_end;

	int i;
	char *marker_string_read_p;
	char *marker_string_write_p;

	total_used_space = circular_buffer_used_bytes;
	total_free_space = circular_buffer_allocated_size - circular_buffer_used_bytes;

// @@@
//WRAP@@@@@@@
#ifdef CIRCULAR_BUFFER_WRAP_DEBUG
	if (unwrapped_read_start_ptr >= circular_buffer_allocated_size) {
		printf("ERROR_2: %s-%d: unwrapped_read_start_ptr==%d, circular_buffer_allocated_size==%d"NEWLINE,
			__FILE__, __LINE__,
			unwrapped_read_start_ptr,
			circular_buffer_allocated_size);
		//@@@THIS_IS_NOT_THE_CORRECT_PLACE_TO_FIX_WRAP:
		//@@@WRAP unwrapped_read_start_ptr %= circular_buffer_allocated_size;
	}
#endif

#ifdef CIRCULAR_BUFFER_WRAP_DEBUG
	if (unwrapped_read_start_ptr < 0) {
// CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(create_circular_buffer_p);
		printf("ERROR_2: %s-%d: unwrapped_read_start_ptr<0==%d, circular_buffer_allocated_size==%d"NEWLINE,
			__FILE__, __LINE__,
			unwrapped_read_start_ptr,
			circular_buffer_allocated_size);
		//@@@THIS_IS_NOT_THE_CORRECT_PLACE_TO_FIX_WRAP:
		//@@@WRAP unwrapped_read_start_ptr %= circular_buffer_allocated_size;
	}
#endif

	// unwrapped_read_start_ptr = LOOP;
	if (circular_buffer_used_bytes == 0) {

		unwrapped_read_end_ptr = (unwrapped_read_start_ptr);	/* last used read location, no data */

// CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(create_circular_buffer_p);
		unwrapped_write_start_ptr = (unwrapped_read_start_ptr + circular_buffer_used_bytes);	/* next free write location */
// CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(create_circular_buffer_p);

		max_readable_chunk_data = circular_buffer_used_bytes;
		max_writeable_chunk_data = circular_buffer_allocated_size - circular_buffer_used_bytes;

	} else {

		unwrapped_read_end_ptr = (unwrapped_read_start_ptr + circular_buffer_used_bytes - 1);	/* last used read location */

		unwrapped_write_start_ptr = (unwrapped_read_start_ptr + circular_buffer_used_bytes);	/* next free write location */

		max_readable_chunk_data = circular_buffer_used_bytes;
		max_writeable_chunk_data = circular_buffer_allocated_size - circular_buffer_used_bytes;
	}

	wrapped_write_start_ptr = unwrapped_write_start_ptr % circular_buffer_allocated_size;

#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG
	if (unwrapped_read_start_ptr <= wrapped_write_start_ptr) {
		printf("+ ");
	} else {
		printf("- ");
	}
#endif

#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG
	printf("alocated_bytes==%d, used_bytes==%d, read_start_ptr==%d, max_read_bytes==%d, write_start_ptr==%d(%d), max_write_bytes==%d"NEWLINE,
		circular_buffer_allocated_size,
		circular_buffer_used_bytes,

		unwrapped_read_start_ptr,
		max_readable_chunk_data,

		wrapped_write_start_ptr,
		unwrapped_write_start_ptr,
		max_writeable_chunk_data);
#endif

// CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(create_circular_buffer_p);
	if (unwrapped_read_start_ptr <= wrapped_write_start_ptr) {

// CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(create_circular_buffer_p);

		max_readable_chunk = circular_buffer_allocated_size - unwrapped_read_start_ptr;
		if (max_readable_chunk > circular_buffer_used_bytes) {
			max_readable_chunk = circular_buffer_used_bytes;
		}

		if (circular_buffer_used_bytes == 0) {
			/* not limited by read data */
			if ( (wrapped_write_start_ptr + max_writeable_chunk_data) > circular_buffer_allocated_size) {
				max_writeable_chunk = circular_buffer_allocated_size - wrapped_write_start_ptr;
			} else {
				max_writeable_chunk = max_writeable_chunk_data;
			}
		} else {
			max_writeable_chunk = circular_buffer_allocated_size - wrapped_write_start_ptr;
			if (max_writeable_chunk > max_writeable_chunk_data) {
				max_writeable_chunk = max_writeable_chunk_data;
			}
		}

		if (circular_buffer_used_bytes == 0) {
			chunk_range_readable_start = -1;
			chunk_range_readable_end = - 1;
		} else {
			chunk_range_readable_start = unwrapped_read_start_ptr;
			chunk_range_readable_end = unwrapped_read_start_ptr + max_readable_chunk - 1;
		}

		if (circular_buffer_used_bytes == circular_buffer_allocated_size) {
			chunk_range_writeable_start = -1;
			chunk_range_writeable_end = -1;
		} else {
			chunk_range_writeable_start = wrapped_write_start_ptr;
			chunk_range_writeable_end = circular_buffer_allocated_size - 1;
		}

	} else {

// CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(create_circular_buffer_p);
		max_readable_chunk = circular_buffer_allocated_size - unwrapped_read_start_ptr;
		if (max_readable_chunk > circular_buffer_used_bytes) {
			max_readable_chunk = circular_buffer_used_bytes;
		}

		max_writeable_chunk = max_writeable_chunk_data;

		chunk_range_readable_start = unwrapped_read_start_ptr;
		chunk_range_readable_end = unwrapped_read_start_ptr + max_readable_chunk - 1;

		chunk_range_writeable_start = wrapped_write_start_ptr;
		chunk_range_writeable_end = unwrapped_read_start_ptr - 1;
	}

#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG
	printf(""NEWLINE);
#endif

	if (circular_buffer_used_bytes == 0) {
		marker_string_read_p = "BUFFER_EMPTY, (Can't read)";
		marker_string_write_p = "";
	} else if (circular_buffer_used_bytes == circular_buffer_allocated_size) {
		marker_string_read_p = "";
		marker_string_write_p = "BUFFER_FULL, (Can't write)";
	} else {
		marker_string_read_p = "";
		marker_string_write_p = "";
	}

#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG
	printf("\tmax_readable_chunk==%d, [%d::%d]\t\t%s"NEWLINE,
		max_readable_chunk,
		chunk_range_readable_start,
		chunk_range_readable_end,
		marker_string_read_p);

	printf("\tmax_writeable_chunk==%d, [%d::%d]\t\t%s"NEWLINE,
		max_writeable_chunk,
		chunk_range_writeable_start,
		chunk_range_writeable_end,
		marker_string_write_p);
#endif

// @@@
#if 0
#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG
	printf("\tmax_readable_overflow==%d"NEWLINE, total_used_space - max_readable_chunk);
	printf("\tmax_writeable_overflow==%d"NEWLINE, total_free_space - max_writeable_chunk);
#endif
#endif

#ifdef CIRCULAR_BUFFER_ALGORITHM_DEBUG
	printf(""NEWLINE);

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - */

	printf("\t");
	for (i = 0; i < circular_buffer_allocated_size; ++i) {
		printf("%2d", i / 10);
	}
	printf(""NEWLINE);

	printf("\t");
	for (i = 0; i < circular_buffer_allocated_size; ++i) {
		printf("%2d", i % 10);
	}
	printf(""NEWLINE);

	printf("\t");
	for (i = 0; i < circular_buffer_allocated_size; ++i) {
		printf("|-");
	}
	printf("|"NEWLINE);

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - */

	printf("\t ");
	for (i = 0; i < unwrapped_read_start_ptr; ++i) {
		if (i < total_used_space - max_readable_chunk) {
			printf("r ");
		} else {
			printf("  ");
		}
	}
	if (circular_buffer_used_bytes == 0) {
		printf("^ ");
	} else {
		for (i = 0; i < max_readable_chunk; ++i) {
			printf("R ");
		}
	}
	printf(""NEWLINE);

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - */

	printf("\t ");
	for (i = 0; i < wrapped_write_start_ptr; ++i) {
		if (i < total_free_space - max_writeable_chunk) {
			printf("w ");
		} else {
			printf("  ");
		}
	}
	if (circular_buffer_used_bytes == circular_buffer_allocated_size) {
		printf("v ");
	} else {
		for (i = 0; i < max_writeable_chunk; ++i) {
			printf("W ");
		}
	}
	printf(""NEWLINE);

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - */

	printf(""NEWLINE);
#endif

	if (max_readable_chunk_p != NULL) {
		*max_readable_chunk_p = max_readable_chunk;
	}

	if (wrapped_write_start_ptr_p != NULL) {
		*wrapped_write_start_ptr_p = wrapped_write_start_ptr;
	}

	if (max_writeable_chunk_p != NULL) {
		*max_writeable_chunk_p = max_writeable_chunk;
	}

	return 0;	/* ok */
}

