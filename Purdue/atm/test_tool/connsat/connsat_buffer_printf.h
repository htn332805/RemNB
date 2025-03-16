
#ifndef __CONNSAT_BUFFER_PRINTF_H__
#define __CONNSAT_BUFFER_PRINTF_H__ 1

#include "connsat_circular_buffer_algorithm.h"

int connsat_buffer_printf(circular_buffer_t *circular_buffer_p, char *fmt, ...);

#endif /* __CONNSAT_BUFFER_PRINTF_H__ */

