
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

/* REDHAT_LINUX */
struct termios termios_saved;
struct termios termios_changed;

#define CONNSAT_TERMIOS_ENABLE 1

int
connsat_termios_init ()
{

	/* REDHAT_LINUX */
	/* Get existing terminal configuration, so we can restore it on exit */
	tcgetattr(0, &termios_saved);

	/* REDHAT_LINUX */
	/* Get existing terminal configuration, so we can modify it */
	tcgetattr(0, &termios_changed);

	/* REDHAT_LINUX */
	/* Modify terminal configuration flags */
	termios_changed.c_iflag &= ~(
		IGNBRK |
		BRKINT |
		PARMRK |
		ISTRIP |
		INLCR |
		IGNCR |
		ICRNL |
		IXON);

	/* REDHAT_LINUX */
	/* Modify terminal configuration flags */
	termios_changed.c_oflag &= ~OPOST;

	/* REDHAT_LINUX */
	/* Modify terminal configuration flags */
	termios_changed.c_lflag &= ~(
		ECHO |
		ECHONL |
		ICANON |
		ISIG |
		IEXTEN);

	/* REDHAT_LINUX */
	/* Modify terminal configuration flags */
	termios_changed.c_cflag &= ~(
		CSIZE |
		PARENB);

	/* REDHAT_LINUX */
	/* Modify terminal configuration flags */
	termios_changed.c_cflag |= CS8;

	/* REDHAT_LINUX */
	/* Implement terminal configuration flags */
#ifdef CONNSAT_TERMIOS_ENABLE
	tcsetattr(0, TCSANOW, &termios_changed);
#endif

	return 0;	/* ok */
}

int
connsat_termios_done ()
{
	/* REDHAT_LINUX */
#ifdef CONNSAT_TERMIOS_ENABLE
	tcsetattr(0, TCSANOW, &termios_saved);
#endif

	return 0;	/* ok */
}

