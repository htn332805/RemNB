
/** \file connsat_spawn_pty.h
 *      \brief Contains Connsat Infrastructure PTY Spawn
 *
 *      Copyright (c) 2024 Cisco Systems, Inc.
 *      All rights reserved
 *
 *      \author
 *              \li Bill Fraser
 *
 *      \date 04/09/2024
 *      \note
 *      \bug
 *      \warning
 *      \todo
 *
 */

#ifndef __CONNSAT_SPAWN_PTY_H__
#define __CONNSAT_SPAWN_PTY_H__ 1

#define CONNSAT_SPAWN_READ_BUF_SIZE 512

int connsat_spawn_pty(
	pid_t *childpid_p,
	int *amaster_fd_p,
	int argc, char *argv[]);

int connsat_spawn_pty_check_pid(
	pid_t childpid,
	int amaster_fd,
	int *child_status_p);

#if 0
int connsat_spawn_getline(
	int *fd_p,
	char *linebuf_p,
	int sizeof_linebuf);
#endif

#endif /* __CONNSAT_SPAWN_PTY_H__ */

