
#ifndef __circular_buffer_algorithm_h__
#define __circular_buffer_algorithm_h__ 1

typedef struct circular_buffer_s {
	char buffer_name[128];
	unsigned char *circular_buffer_data_p;
	int circular_buffer_allocated_size;
	int unwrapped_read_start_ptr;
		/* array_index */
	int circular_buffer_used_bytes;
	int calculated_buffer_max_readable_chunk;		/* calculated value */
	int calculated_buffer_wrapped_write_start_ptr;		/* calculated value */
		/* array_index */
	int calculated_buffer_max_writeable_chunk;		/* calculated value */
} circular_buffer_t;

int circular_buffer_init(
	char *new_buffer_name_p,
	int allocated_size,
	circular_buffer_t **new_circular_buffer_pp
	);

int circular_buffer_dump(
	circular_buffer_t *circular_buffer_p
	);

int circular_buffer_view(
	circular_buffer_t *circular_buffer_p,
	char **buffer_view_pp,
	int *buffer_view_size_p
	);

int circular_buffer_calculations(
	circular_buffer_t *circular_buffer_p
	);

int circular_buffer_stream_read(
	circular_buffer_t *circular_buffer_p,
	int fd,
	int *flags_p
	);

int circular_buffer_stream_write(
	circular_buffer_t *circular_buffer_p,
	int fd
	);

int circular_buffer_getc(
	circular_buffer_t *circular_buffer_p,
	int *input_char_p
	);

int circular_buffer_putc(
	circular_buffer_t *circular_buffer_p,
	int *output_char_p
	);

int circular_buffer_block_transfer(
	circular_buffer_t *circular_buffer_1_p,
	circular_buffer_t *circular_buffer_2_p
	);

int circular_buffer_multi_block_transfer(
	circular_buffer_t *circular_buffer_1_p,
	circular_buffer_t *circular_buffer_2_p,
	int max_multi_buffer_transfer
	);

int circular_buffer_multi_block_transfer_complete(
	circular_buffer_t *circular_buffer_1_p,
	int max_multi_buffer_transfer
	);

int circular_buffer_math_calculations(
	int circular_buffer_allocated_size,
	int circular_buffer_start_index,
	int circular_buffer_used_bytes,
	int *max_readable_chunk_p,
	int *max_writeable_chunk_p,
	int *wrapped_write_start_ptr_p
	);

#endif /* __circular_buffer_algorithm_h__ */

