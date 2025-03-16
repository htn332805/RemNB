
/** \file connsat_spawn_pty.c
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

// #define DEBUG_CONNSAT_SPAWN 1

#include <stdio.h>
#include <stdlib.h>

// EXECL
#include <unistd.h>
extern char **environ;

#include <errno.h>

// WAITPID
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/stat.h>

// FORKPTY
#include <pty.h>

// PSIGNAL:
#include <signal.h>

#include "connsat_main.h"
#include "connsat_spawn_pty.h"
#include "connsat_child_args.h"

int
connsat_spawn_pty (
	pid_t *child_pid_p,
	int *amaster_fd_p,
	int argc,
	char *argv[])
{
	struct stat stat_buf;
	pid_t child_pid;
	char name_buf[256];

	int raw_mode_flag;

	argc = argc;

	*amaster_fd_p = -1;

	raw_mode_flag = 1;

	if (argv != NULL) {

		/* See if the executable exists. */
		if (stat(argv[0], &stat_buf) != 0) {
			perror("Error: accessing sub-program");
			return -1;
		}
	}

	/*
	** Create a pseudo terminal and fork
	** a process attached to it.
	*/
	child_pid = forkpty(amaster_fd_p, name_buf, NULL, NULL);
	if (child_pid < 0) {
		perror("Error: forking sub-program");

		return -2;

	} else if (child_pid == 0) {

		/* Inside the child process */

#if 0
		/* enabling this temporarily can check forkpty pipe streams */

		fprintf(stdout, "CHILD_STDOUT: stdout test message out\n");
		fflush(stdout);

		fprintf(stderr, "CHILD_STDERR: stderr test message out\n");
		fflush(stderr);

		// system_r = system("sleep 10");
		sleep(10);

		fprintf(stdout, "CHILD_STDOUT: Exiting...\n");
		fflush(stdout);

#else

#if 1

#if 0
		child_args_argv_dump(
			&argc,
			&argv,
			raw_mode_flag);
#endif          


		if (execvp(argv[0], &argv[0])) {

			perror("execvp");

			return -3;
		}
#else
		/* ???NOTE: this uses environ by default */

		char *child_args[] = { "/bin/bash", "-i", NULL};
		// char *child_args[] = { "/bin/sh", "-i", NULL};
		execve(child_args[0], child_args, NULL);
#endif
#endif

		/* Child shouldn't get here, but if it does, exit */

		exit(0);
	}

	/* Inside the parent process */

	if (child_pid_p != NULL) {
		*child_pid_p = child_pid;
	}

	return 0; /* ok */
}

int
connsat_spawn_pty_check_pid (
	pid_t child_pid,
	int amaster_fd,
	int *child_status_p)
{
	pid_t waitpid_return_status;
	int waitpid_wstatus;
	int waitpid_options;

	int child_running_flag;

	waitpid_options = WNOHANG;

	waitpid_return_status = waitpid(child_pid, &waitpid_wstatus, waitpid_options);

	child_running_flag = 1;

	if (1) {
		if (WIFEXITED(waitpid_wstatus) ) {

			/* returns true if the child terminated normally,
			** that is, by calling exit(3) or _exit(2), or by
			** returning from main().
			*/
#ifdef CONNSAT_SPAWN_PTY_DEBUG_CHILD_EXITED
			fprintf(stdout, "\n\tCHILD_EXIT_STATUS: WIFEXITED TRUE\n");
			fflush(stdout);
#endif

			if (WEXITSTATUS(waitpid_wstatus) ) {
				/* returns  the  exit  status  of  the child.
				** This consists of the least significant 8
				** bits of the status argument that the child
				** specified in a call to exit(3) or _exit(2)
				** or as the argument for a return statement
				** in main().  This macro should be  employed
				** only if WIFEXITED returned true.
				*/
#ifdef CONNSAT_SPAWN_PTY_DEBUG_WAITPID
				fprintf(stdout,
					"\n\tCHILD_EXIT_STATUS: WEXITSTATUS %d\n",
					WEXITSTATUS(waitpid_wstatus) );
				fflush(stdout);
#endif

				child_running_flag = 0;
				if (child_status_p != NULL) {
					*child_status_p = child_running_flag;
				}
			}
		}

		if (WIFSIGNALED(waitpid_wstatus) ) {
			/* returns true if the child process was terminated by a signal. */

#ifdef CONNSAT_SPAWN_PTY_DEBUG_WAITPID
			fprintf(stdout,
				"\n\tCHILD_EXIT_STATUS: WIFSIGNALED TRUE\n");
			fflush(stdout);
#endif

			if (WTERMSIG(waitpid_wstatus) ) {
				/* returns the number of the signal that caused
				** the child process to terminate.  This macro
				** should be employed only if WIFSIGNALED
				** returned true.
				*/

#ifdef CONNSAT_SPAWN_PTY_DEBUG_WAITPID
				fprintf(stdout,
					"\n\tCHILD_EXIT_STATUS: WTERMSIG %d\n",
					WTERMSIG(waitpid_wstatus) );
				psignal(
					WTERMSIG(waitpid_wstatus),
					"WTERMSIG");
				fflush(stdout);
#endif
				child_running_flag = 0;
				if (child_status_p != NULL) {
					*child_status_p = child_running_flag;
				}
			}
		}

		if (WCOREDUMP(waitpid_wstatus) ) {
			/* Returns true if the child produced a core
			** dump.  This macro should be employed only
			** if WIFSIGNALED returned true. */

			/* This macro is not specified in POSIX.1-2001
			** and is not available on some UNIX
			** implementations (e.g., AIX,  SunOS).
			** Therefore, enclose its use inside
			** #ifdef WCOREDUMP ... #endif.
			*/

#ifdef CONNSAT_SPAWN_PTY_DEBUG_WAITPID
			fprintf(stdout,
				"\n\tCHILD_EXIT_STATUS: WCOREDUMP TRUE !!!!!!! core-dump !!!!!!!\n");
			fflush(stdout);
#endif
			child_running_flag = 0;
			if (child_status_p != NULL) {
				*child_status_p = child_running_flag;
			}
		}

		if (WIFSTOPPED(waitpid_wstatus) ) {
			/* Returns  true  if  the child process was
			** stopped by delivery of a signal; this is
			** possible only if the call was done using
			** WUN‐TRACED or when the child is being
			** traced (see ptrace(2)).
			*/

#ifdef CONNSAT_SPAWN_PTY_DEBUG_WAITPID
			fprintf(stdout,
				"\n\tCHILD_EXIT_STATUS: WIFSTOPPED TRUE\n");
			fflush(stdout);
#endif

			if (WSTOPSIG(waitpid_wstatus) ) {
				/* Returns the number of the signal which
				** caused the child to stop. This macro
				** should be employed only if WIFSTOPPED
				** returned true.
				*/

#ifdef CONNSAT_SPAWN_PTY_DEBUG_WAITPID
				fprintf(stdout,
					"\n\tCHILD_EXIT_STATUS: WSTOPSIG %d\n",
					WSTOPSIG(waitpid_wstatus) );
				fflush(stdout);
#endif
			}

		}

		if (WIFCONTINUED(waitpid_wstatus) ) {
			/* (Since Linux 2.6.10) returns true if the
			** child process was resumed by delivery of
			** SIGCONT.
			*/

#ifdef CONNSAT_SPAWN_PTY_DEBUG_WAITPID
			fprintf(stdout,
				"\n\tCHILD_EXIT_STATUS: WIFCONTINUED TRUE\n");
			fflush(stdout);
#endif
		}
	}

	if (child_status_p != NULL) {
		*child_status_p = child_running_flag;
	}

	return 0; /* ok */
}

#if 0
int
connsat_spawn_getline (
	int *fd_p,
	char *linebuf_p,
	int sizeof_linebuf)
{
	static char inbuf[CONNSAT_SPAWN_READ_BUF_SIZE];
	static char *inbuf_p = NULL;
	static int inbuf_count = 0;
	int first_pos;
	int current_count;
	int r;
	int x;

	for (x = 0; x < (sizeof_linebuf -1); ++x) {

		if (inbuf_p == NULL) {
			inbuf_p = inbuf;
		}

		first_pos = inbuf_p - inbuf;

		current_count = inbuf_count - first_pos;

		if (current_count == 0) {

			/* fill or refill inbuf */
			r = read(*fd_p, inbuf, sizeof inbuf);
#ifdef DEBUG_CONNSAT_SPAWN
			printf("<fill:%d/%d>", r, x);
#endif
			if (r == -1) {
				close(*fd_p);

				*fd_p = -1;

				return -1;

				/* break; */
			} else if (r == 0) {
				/* Null Terminate string */
				linebuf_p[x] = 0;

				return x;
			}

			inbuf_p = inbuf;
			inbuf_count = r;
		}

		if (*inbuf_p == '\r') {

			/* we're going to skip over these characters */
			++inbuf_p;
			--x;

			continue;
		}

		linebuf_p[x] = *inbuf_p;
		if (*inbuf_p == '\n') {

			++inbuf_p;

			/* Null Terminate string */
			linebuf_p[x+1] = 0;

			return x+1;
		}

		++inbuf_p;
	}

	/* Null Terminate string */
	linebuf_p[x] = 0;

	return x;
}
#endif

#if 0
int
connsat_spawn_expect (int argc, char *argv[])
{
	int array_flag;
	int amaster_fd;
	int y;
	int r;
	char line_buf[256];
	// char key[16];

	r = connsat_spawn_pty(&amaster_fd, argc, argv);
	//printf("connsat_spawn_pty (%d) returned %d\n", amaster_fd, r);

	for (y = 1; ; ++y) {
		r = connsat_spawn_getline(&amaster_fd, line_buf, sizeof line_buf);
		if (r == -1) {
			break;
		}
		if (r >= 1) {
			if (line_buf[r-1] == '\n') {
				//printf("F: ");   /* Full newline terminated line */
				line_buf[--r] = 0; /* strip newline */
			} else {
				//printf("P: ");   /* Partial unterminated line segment */
			}
		} else {
			//printf("E: ");           /* Empty line */
			break;
		}

		//printf("read line %3d bytes: ", r);
		//printf("%3d: \"%s\"\n", y, line_buf);

		if (array_flag) {
			printf("%3d: \"%s\"\r\n", y, line_buf);
		} else {
			printf("%3d: \"%s\"\r\n", y, line_buf);
		}
	}

	return 0;	/* ok */
}
#endif

