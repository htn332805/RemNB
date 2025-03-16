
TARGET = connsat

CC=		${CROSS_COMPILE}gcc
GCC=		${CROSS_COMPILE}gcc
STRIP=		${CROSS_COMPILE}strip
AR=		${CROSS_COMPILE}ar

CONNSAT_RELEASE_MAJOR   := 0
CONNSAT_RELEASE_MINOR   := 0
CONNSAT_RELEASE_SWREL   := 4
CONNSAT_RELEASE_STRING  := $(CONNSAT_RELEASE_MAJOR).$(CONNSAT_RELEASE_MINOR).$(CONNSAT_RELEASE_SWREL)
CONNSAT_BUILD_HOST      := $(shell hostname)
CONNSAT_BUILD_DATE      := $(shell date '+%Y%m%d_%H%M%S')

CONNSAT_VERSION_STRING  := $(CONNSAT_RELEASE_STRING)-$(CONNSAT_BUILD_DATE)-$(USER)@$(CONNSAT_BUILD_HOST)
CONNSAT_TOOLCHAIN_VERSION_STRING := "`$(CROSS_COMPILE)gcc --version | head -1`"

CFLAGS = -g
CFLAGS+=        -DCONNSAT_VERSION=\"$(CONNSAT_VERSION_STRING)\"
CFLAGS+=        -DCONNSAT_TARGET=\"$(TARGET)\"
CFLAGS+=        -DCONNSAT_TOOLCHAIN_VERSION=\"$(CONNSAT_TOOLCHAIN_VERSION_STRING)\"

OBJECTS = connsat_main.o			\
	connsat_termios.o			\
	connsat_options.o			\
	connsat_spawn_pty.o			\
	connsat_ascii.o				\
	connsat_sideband_server.o		\
	connsat_connections.o			\
	connsat_terminal_control_menu.o		\
	connsat_circular_buffer_algorithm.o	\
	connsat_child_args.o			\
	connsat_buffer_printf.o			\
	connsat_connection_main_terminal.o	\
	connsat_connection_control_menu.o	\
	connsat_connection_forkpty.o		\
	connsat_connection_sideband_channel.o	\
	connsat_connection_sideband_server.o	\
	connsat_connection_session_log.o	\
	connsat_connection_capture_log.o	\
	connsat_connection_init.o		\
	connsat_timestamp.o


SOURCES = connsat_main.c			\
	connsat_termios.c			\
	connsat_options.c			\
	connsat_spawn_pty.c			\
	connsat_ascii.c				\
	connsat_sideband_server.c		\
	connsat_connections.c			\
	connsat_terminal_control_menu.c		\
	connsat_circular_buffer_algorithm.c	\
	connsat_child_args.c			\
	connsat_buffer_printf.c			\
	connsat_connection_main_terminal.c	\
	connsat_connection_control_menu.c	\
	connsat_connection_forkpty.c		\
	connsat_connection_sideband_channel.c	\
	connsat_connection_sideband_server.c	\
	connsat_connection_session_log.c	\
	connsat_connection_capture_log.c	\
	connsat_connection_init.c		\
	connsat_timestamp.c

LIBRARIES = \
	-lutil

INCLUDES = \
	connsat_termios.h			\
	connsat_options.h			\
	connsat_spawn_pty.h			\
	connsat_ascii.h				\
	connsat_sideband_server.h		\
	connsat_connections.h			\
	connsat_circular_buffer_algorithm.h	\
	connsat_child_args.h			\
	connsat_buffer_printf.h			\
	connsat_connection_main_terminal.h	\
	connsat_connection_control_menu.h	\
	connsat_connection_forkpty.h		\
	connsat_connection_sideband_channel.h	\
	connsat_connection_sideband_server.h	\
	connsat_connection_session_log.h	\
	connsat_connection_capture_log.h	\
	connsat_connection_init.h		\
	connsat_timestamp.h

$(TARGET): $(OBJECTS)
	# gcc -BSTATIC ${OBJECTS} -o ${TARGET} ${LIBRARIES}
	gcc -BSTATIC -g ${OBJECTS} -o ${TARGET} ${LIBRARIES}

tags: ${INCLUDES} ${SOURCES}
	ctags -X -d ${INCLUDES} ${SOURCES} > xtags

clean:
	rm -f ${OBJECTS} ${TARGET} tags xtags cscope.files cscope.out a.out

sc: clean

.c.o: gcc -c ${CFLAGS} $*.c


