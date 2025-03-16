
TARGET = connsat_client

CC=             ${CROSS_COMPILE}gcc
GCC=            ${CROSS_COMPILE}gcc
STRIP=          ${CROSS_COMPILE}strip
AR=             ${CROSS_COMPILE}ar

CONNSAT_RELEASE_MAJOR   := 0
CONNSAT_RELEASE_MINOR   := 0
CONNSAT_RELEASE_SWREL   := 2
CONNSAT_RELEASE_STRING  := $(CONNSAT_RELEASE_MAJOR).$(CONNSAT_RELEASE_MINOR).$(CONNSAT_RELEASE_SWREL)
CONNSAT_BUILD_HOST      := $(shell hostname)
CONNSAT_BUILD_DATE      := $(shell date '+%Y%m%d_%H%M%S')

CONNSAT_VERSION_STRING  := $(CONNSAT_RELEASE_STRING)-$(CONNSAT_BUILD_DATE)-$(USER)@$(CONNSAT_BUILD_HOST)
CONNSAT_TOOLCHAIN_VERSION_STRING := "`$(CROSS_COMPILE)gcc --version | head -1`"

CFLAGS = -g
CFLAGS+=        -DCONNSAT_VERSION=\"$(CONNSAT_VERSION_STRING)\"
CFLAGS+=        -DCONNSAT_TARGET=\"$(TARGET)\"
CFLAGS+=        -DCONNSAT_TOOLCHAIN_VERSION=\"$(CONNSAT_TOOLCHAIN_VERSION_STRING)\"

OBJECTS = 					\
	connsat_client.o			\
	connsat_termios.o			\
	connsat_options.o			\
	connsat_circular_buffer_algorithm.o	\
	connsat_ascii.o				\

SOURCES =					\
	connsat_client.c			\
	connsat_termios.c			\
	connsat_options.c			\
	connsat_circular_buffer_algorithm.c	\
	connsat_ascii.c				\

LIBRARIES = \

#	-lutil

INCLUDES = \
	connsat_termios.h			\
	connsat_options.h			\
	connsat_circular_buffer_algorithm.h	\
	connsat_ascii.h				\

$(TARGET): $(OBJECTS)
	gcc -BSTATIC ${CFLAGS} ${OBJECTS} -o ${TARGET} ${LIBRARIES}

tags: ${INCLUDES} ${SOURCES}
	ctags -X -d ${INCLUDES} ${SOURCES} > xtags

clean:
	rm -f ${OBJECTS} ${TARGET} tags xtags cscope.files cscope.out a.out

sc: clean

.c.o: gcc -c ${CFLAGS} $*.c


