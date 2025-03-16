
#ifndef __CONNSAT_TERMINAL_CONTROL_MENU_H__
#define __CONNSAT_TERMINAL_CONTROL_MENU_H__ 1

#include "connsat_circular_buffer_algorithm.h"
#include "connsat_buffer_printf.h"

int connsat_control_menu(
	circular_buffer_t *menu_input_buffer_p,
	circular_buffer_t *menu_output_buffer_p,
	char *(*env_p)[],
	int debug_output_flag);

int connsat_control_menu_help(circular_buffer_t *menu_output_buffer_p);

#endif /* __CONNSAT_TERMINAL_CONTROL_MENU_H__ */

