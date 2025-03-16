
#ifndef __CONNSAT_MAIN_H__
#define __CONNSAT_MAIN_H__ 1

#define MASTER_DEBUG_FLAG_01_POLL_CONTROL_MENU_ENABLE 1
#define MASTER_DEBUG_FLAG_01_POLL_BUFFER_DISTRIBUTION_ENABLE 1

#define EXPERIMENTAL_CONTROL_MENU_UPDATES_001 1

#define KEEP_ALIVE_TIME_FEATURE 1
// #define KEEP_ALIVE_TIME_TRACE 1
// #define KEEP_ALIVE_TIME_TRACE_RW 1

#if 1 // set 1

// #define MASTER_DEBUG_FLAG_01 1				/* noisy output
// */ #define MASTER_DEBUG_FLAG_01_CALC 1			/* noisy output
// */ #define MASTER_DEBUG_FLAG_01_LOCAL_ECHO 1		/* noisy output, not
// used now */ #define MASTER_DEBUG_FLAG_01_POLL_LOOP_START 1	/* noisy output
// */ #define MASTER_DEBUG_FLAG_01_TRANS 1			/* noisy output
// */ #define MASTER_DEBUG_FLAG_01_SAVED 1			/* noisy output
// */ #define MASTER_DEBUG_FLAG_01_CONTROL_MENU_INPUT 1	/* noisy output */
// #define MASTER_DEBUG_FLAG_01_CONTROL_MENU_EXIT 1	/* noisy output */
// #define MASTER_DEBUG_FLAG_01_POLL_ACTIVE 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_SELF_LOOPBACK 1		/* noisy output */

// #define MASTER_DEBUG_FLAG_01_LIMIT_LOOP 1		/* limit poll loops and
// exit */ #define MASTER_DEBUG_FLAG_01_POLL_SLEEP 1		/* poll loop
// sleep(3) */

// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_01 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_02 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_03 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_04 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_05 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_CB_ALLOCATIONS 1		/* noisy output
// */ #define MASTER_DEBUG_FLAG_01_MULTI_TRANSFER 1		/* noisy output
// */

// #define MAIN_TERMINAL_STREAM_READ_BUFFER_DUMP 1
// #define MAIN_TERMINAL_STREAM_WRITE_BUFFER_DUMP 1
// #define SIDEBAND_TERMINAL_STREAM_READ_BUFFER_DUMP 1
// #define SIDEBAND_TERMINAL_STREAM_WRITE_BUFFER_DUMP 1
// #define CIRCULAR_BUFFER_DUMP_DEBUG_SIDEBAND_OUTPUT 1   /* remove // to enable
// debug */ #define SIDEBAND_TERMINAL_STREAM_READ_BUFFER_CRLF 1 #define
// FORKPTY_STREAM_READ_BUFFER_DUMP 1 #define FORKPTY_STREAM_WRITE_BUFFER_DUMP 1

// #define MASTER_DEBUG_FLAG_02 1
// #define MASTER_DEBUG_FLAG_03 1		/* buffer input and output
// injection test */ #define MASTER_DEBUG_FLAG_07 1
#define CONNSAT_SPAWN_PTY_DEBUG_WAITPID 1
#define CONNSAT_SPAWN_PTY_DEBUG_CHILD_EXITED 1

#endif

#if 0                                // set 2, show buffer transactions:

// #define MASTER_DEBUG_FLAG_01 1				/* noisy output */
// #define MASTER_DEBUG_FLAG_01_CALC 1			/* noisy output */
// #define MASTER_DEBUG_FLAG_01_LOCAL_ECHO 1		/* noisy output, not used now */
// #define MASTER_DEBUG_FLAG_01_POLL_LOOP_START 1	/* noisy output */
#define MASTER_DEBUG_FLAG_01_TRANS 1 /* noisy output */
// #define MASTER_DEBUG_FLAG_01_SAVED 1			/* noisy output */
// #define MASTER_DEBUG_FLAG_01_CONTROL_MENU_INPUT 1	/* noisy output */
// #define MASTER_DEBUG_FLAG_01_CONTROL_MENU_EXIT 1	/* noisy output */
// #define MASTER_DEBUG_FLAG_01_POLL_ACTIVE 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_SELF_LOOPBACK 1		/* noisy output */

// #define MASTER_DEBUG_FLAG_01_LIMIT_LOOP 1		/* limit poll loops and exit */
// #define MASTER_DEBUG_FLAG_01_POLL_SLEEP 1		/* poll loop sleep(3) */

// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_01 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_02 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_03 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_04 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_05 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_CB_ALLOCATIONS 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_MULTI_TRANSFER 1		/* noisy output */

// #define MAIN_TERMINAL_STREAM_READ_BUFFER_DUMP 1
// #define MAIN_TERMINAL_STREAM_WRITE_BUFFER_DUMP 1
// #define SIDEBAND_TERMINAL_STREAM_READ_BUFFER_DUMP 1
// #define SIDEBAND_TERMINAL_STREAM_WRITE_BUFFER_DUMP 1
// #define CIRCULAR_BUFFER_DUMP_DEBUG_SIDEBAND_OUTPUT 1
// #define SIDEBAND_TERMINAL_STREAM_READ_BUFFER_CRLF 1
// #define FORKPTY_STREAM_READ_BUFFER_DUMP 1
// #define FORKPTY_STREAM_WRITE_BUFFER_DUMP 1

// #define MASTER_DEBUG_FLAG_02 1
// #define MASTER_DEBUG_FLAG_03 1		/* buffer input and output injection test */
// #define MASTER_DEBUG_FLAG_07 1
#define CONNSAT_SPAWN_PTY_DEBUG_WAITPID 1
#define CONNSAT_SPAWN_PTY_DEBUG_CHILD_EXITED 1

#endif

#if 0 // set 3

#define MASTER_DEBUG_FLAG_01 1                    /* noisy output */
#define MASTER_DEBUG_FLAG_01_CALC 1               /* noisy output */
// #define MASTER_DEBUG_FLAG_01_LOCAL_ECHO 1		/* noisy output, not used now */
// #define MASTER_DEBUG_FLAG_01_POLL_LOOP_START 1	/* noisy output */
#define MASTER_DEBUG_FLAG_01_TRANS 1              /* noisy output */
#define MASTER_DEBUG_FLAG_01_SAVED 1              /* noisy output */
#define MASTER_DEBUG_FLAG_01_CONTROL_MENU_INPUT 1 /* noisy output */
#define MASTER_DEBUG_FLAG_01_CONTROL_MENU_EXIT 1  /* noisy output */
#define MASTER_DEBUG_FLAG_01_POLL_ACTIVE 1        /* noisy output */
// #define MASTER_DEBUG_FLAG_01_SELF_LOOPBACK 1		/* noisy output */

// #define MASTER_DEBUG_FLAG_01_LIMIT_LOOP 1		/* limit poll loops and exit */
// #define MASTER_DEBUG_FLAG_01_POLL_SLEEP 1		/* poll loop sleep(3) */

// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_01 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_02 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_03 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_04 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_05 1		/* noisy output */
#define MASTER_DEBUG_FLAG_01_CB_ALLOCATIONS 1     /* noisy output */
#define MASTER_DEBUG_FLAG_01_MULTI_TRANSFER 1     /* noisy output */

#define MAIN_TERMINAL_STREAM_READ_BUFFER_DUMP 1
#define MAIN_TERMINAL_STREAM_WRITE_BUFFER_DUMP 1
#define SIDEBAND_TERMINAL_STREAM_READ_BUFFER_DUMP 1
#define SIDEBAND_TERMINAL_STREAM_WRITE_BUFFER_DUMP 1
// #define CIRCULAR_BUFFER_DUMP_DEBUG_SIDEBAND_OUTPUT 1
// #define SIDEBAND_TERMINAL_STREAM_READ_BUFFER_CRLF 1
#define FORKPTY_STREAM_READ_BUFFER_DUMP 1
#define FORKPTY_STREAM_WRITE_BUFFER_DUMP 1

#define MASTER_DEBUG_FLAG_02 1
// #define MASTER_DEBUG_FLAG_03 1		/* buffer input and output injection test */
// #define MASTER_DEBUG_FLAG_07 1
#define CONNSAT_SPAWN_PTY_DEBUG_WAITPID 1
#define CONNSAT_SPAWN_PTY_DEBUG_CHILD_EXITED 1

#endif

#if 0 // set 4

#define MASTER_DEBUG_FLAG_01 1                    /* noisy output */
#define MASTER_DEBUG_FLAG_01_CALC 1               /* noisy output */
// #define MASTER_DEBUG_FLAG_01_LOCAL_ECHO 1		/* noisy output, not used now */
// #define MASTER_DEBUG_FLAG_01_POLL_LOOP_START 1	/* noisy output */
#define MASTER_DEBUG_FLAG_01_TRANS 1              /* noisy output */
#define MASTER_DEBUG_FLAG_01_SAVED 1              /* noisy output */
#define MASTER_DEBUG_FLAG_01_CONTROL_MENU_INPUT 1 /* noisy output */
#define MASTER_DEBUG_FLAG_01_CONTROL_MENU_EXIT 1  /* noisy output */
#define MASTER_DEBUG_FLAG_01_POLL_ACTIVE 1        /* noisy output */

#define MASTER_DEBUG_FLAG_01_LIMIT_LOOP 1 /* limit poll loops and exit */
#define MASTER_DEBUG_FLAG_01_POLL_SLEEP 1 /* poll loop sleep(3) */

// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_01 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_02 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_03 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_04 1		/* noisy output */
// #define MASTER_DEBUG_FLAG_01_POLL_DEBUG_05 1		/* noisy output */

#define MASTER_DEBUG_FLAG_01_CB_ALLOCATIONS 1 /* noisy output */
#define MASTER_DEBUG_FLAG_01_MULTI_TRANSFER 1 /* noisy output */

#define MAIN_TERMINAL_STREAM_READ_BUFFER_DUMP 1
#define MAIN_TERMINAL_STREAM_WRITE_BUFFER_DUMP 1
#define SIDEBAND_TERMINAL_STREAM_READ_BUFFER_DUMP 1
#define SIDEBAND_TERMINAL_STREAM_WRITE_BUFFER_DUMP 1
// #define CIRCULAR_BUFFER_DUMP_DEBUG_SIDEBAND_OUTPUT 1
// #define SIDEBAND_TERMINAL_STREAM_READ_BUFFER_CRLF 1
#define FORKPTY_STREAM_READ_BUFFER_DUMP 1
#define FORKPTY_STREAM_WRITE_BUFFER_DUMP 1

#define MASTER_DEBUG_FLAG_02 1
// #define MASTER_DEBUG_FLAG_03 1		/* buffer input and output injection test */
// #define MASTER_DEBUG_FLAG_07 1
#define CONNSAT_SPAWN_PTY_DEBUG_WAITPID 1
#define CONNSAT_SPAWN_PTY_DEBUG_CHILD_EXITED 1

#endif

#if 0 // set 5, almost everything

#define MASTER_DEBUG_FLAG_01 1                    /* noisy output */
#define MASTER_DEBUG_FLAG_01_CALC 1               /* noisy output */
// #define MASTER_DEBUG_FLAG_01_LOCAL_ECHO 1            /* noisy output, not used now */
#define MASTER_DEBUG_FLAG_01_POLL_LOOP_START 1    /* noisy output */
#define MASTER_DEBUG_FLAG_01_TRANS 1              /* noisy output */
#define MASTER_DEBUG_FLAG_01_SAVED 1              /* noisy output */
#define MASTER_DEBUG_FLAG_01_CONTROL_MENU_INPUT 1 /* noisy output */
#define MASTER_DEBUG_FLAG_01_CONTROL_MENU_EXIT 1  /* noisy output */
#define MASTER_DEBUG_FLAG_01_POLL_ACTIVE 1        /* noisy output */

#define MASTER_DEBUG_FLAG_01_LIMIT_LOOP 1 /* limit poll loops and exit */
#define MASTER_DEBUG_FLAG_01_POLL_SLEEP 1 /* poll loop sleep(3) */

#define MASTER_DEBUG_FLAG_01_POLL_DEBUG_01 1 /* noisy output */
#define MASTER_DEBUG_FLAG_01_POLL_DEBUG_02 1 /* noisy output */
#define MASTER_DEBUG_FLAG_01_POLL_DEBUG_03 1 /* noisy output */
#define MASTER_DEBUG_FLAG_01_POLL_DEBUG_04 1 /* noisy output */
#define MASTER_DEBUG_FLAG_01_POLL_DEBUG_05 1 /* noisy output */

#define MASTER_DEBUG_FLAG_01_CB_ALLOCATIONS 1 /* noisy output */
#define MASTER_DEBUG_FLAG_01_MULTI_TRANSFER 1 /* noisy output */

#define MAIN_TERMINAL_STREAM_READ_BUFFER_DUMP 1
#define MAIN_TERMINAL_STREAM_WRITE_BUFFER_DUMP 1
#define SIDEBAND_TERMINAL_STREAM_READ_BUFFER_DUMP 1
#define SIDEBAND_TERMINAL_STREAM_WRITE_BUFFER_DUMP 1
// #define CIRCULAR_BUFFER_DUMP_DEBUG_SIDEBAND_OUTPUT 1
// #define SIDEBAND_TERMINAL_STREAM_READ_BUFFER_CRLF 1
#define FORKPTY_STREAM_READ_BUFFER_DUMP 1
#define FORKPTY_STREAM_WRITE_BUFFER_DUMP 1

#define MASTER_DEBUG_FLAG_02 1
// #define MASTER_DEBUG_FLAG_03 1               /* buffer input and output injection test */
// #define MASTER_DEBUG_FLAG_07 1
#define CONNSAT_SPAWN_PTY_DEBUG_WAITPID 1
#define CONNSAT_SPAWN_PTY_DEBUG_CHILD_EXITED 1

#endif

#if 0
#define CONNSAT_TRACE                                                          \
  printf("CTRACE: %s-%d%s", __FILE__, __LINE__, newline_string_p)
#else
#define CONNSAT_TRACE
#endif

#if 0
#define CONNSAT_TRACE_DUMP                                                     \
  printf("CTD_TRACE: %s-%d%s", __FILE__, __LINE__, newline_string_p);          \
  connsat_connection_dump_one(connsat_connection_terminal_p,                   \
                              CONNSAT_CONNECTION_DUMP_ONE_FLAGS_TYPE,          \
                              raw_mode_flag)

#else
#define CONNSAT_TRACE_DUMP
#endif

#if 1
#define CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(buffer_p)                \
  if (buffer_p->unwrapped_read_start_ptr < 0) {                                \
    printf("BUFFER_TRACE: %s-%d: %s / unwrapped_read_start_ptr==%d%s",         \
           __FILE__, __LINE__, buffer_p->buffer_name,                          \
           buffer_p->unwrapped_read_start_ptr, "\r\n");                        \
  }

#else
#define CONNSAT_BUFFER_TRACE_UNWRAPPED_READ_START_PTR(buffer_p)
#endif

#endif /* __CONNSAT_MAIN_H__ */
