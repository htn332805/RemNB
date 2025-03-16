
#ifndef __CONNSAT_CONNECTION_INIT_H__
#define __CONNSAT_CONNECTION_INIT_H__ 1

extern struct connsat_connection_s *connsat_connection_terminal_p;
extern struct connsat_connection_s *connsat_connection_control_menu_p;
extern struct connsat_connection_s *connsat_connection_forkpty_p;
extern struct connsat_connection_s *connsat_connection_sideband_server_p;
extern struct connsat_connection_s *connsat_connection_sideband_channel_p;

int connsat_connection_init(int raw_mode_flag);

#endif /* __CONNSAT_CONNECTION_INIT_H__ */

