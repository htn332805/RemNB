
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "connsat_child_args.h"

// #define STANDALONE_CONNSAT_CHILD_ARGS 1

// #define DEBUG_CHILD_ARGS_01 1
// #define DEBUG_CHILD_ARGS_02 1
// #define DEBUG_CHILD_ARGS_03 1
// #define DEBUG_CHILD_ARGS_04 1
// #define DEBUG_CHILD_ARGS_05 1

#define DEBUG_CHILD_ARGS_10 1
#define DEBUG_CHILD_ARGS_11 1
#define DEBUG_CHILD_ARGS_12 1

#ifdef STANDALONE_CONNSAT_CHILD_ARGS
int
main ()
{
	char *test1 = "/bin/bash -i -o --something asdf qwert1";
	char *test2 = "/bin/bash -i  -o --something asdf  qwert1";
	int return_malloc_size;
	int return_argc;
	char **return_argv;
	// int word_count;
	int r;

	printf("==============================\n\n");
	r = child_args_parse_string(test1, &return_malloc_size, &return_argc, &return_argv);

#ifdef DEBUG_CHILD_ARGS_10
	printf("\n");
	printf("PARSE_DUMP:\n");
	printf("\tInput_String==\"%s\"\n", test1);
	printf("\tbuff_size==%d; argc==%d, argv[%d]==%p\n", return_malloc_size, return_argc, return_argc + 1, return_argv);
#endif

#ifdef DEBUG_CHILD_ARGS_11
	printf("\n");
	printf("HEX_DUMP:\n");
	// hd_buff((unsigned char *) argv_array_pointer_pp, (long unsigned int) 0, malloc_argv_size, 0, 1);
	hd_buff((unsigned char *) return_argv, (unsigned char *) return_argv, return_malloc_size, 0, 1);
#endif

#ifdef DEBUG_CHILD_ARGS_12
	printf("\n");
	printf("ARGV_DUMP:\n");
	// child_args_argv_dump(&word_count, &argv_array_pointer_pp);
	child_args_argv_dump(&return_argc, &return_argv);
#endif

	free(return_argv);
	return_argv = NULL;



	printf("==============================\n\n");
	r = child_args_parse_string(test2, &return_malloc_size, &return_argc, &return_argv);

#ifdef DEBUG_CHILD_ARGS_10
	printf("\n");
	printf("PARSE_DUMP:\n");
	printf("\tInput_String==\"%s\"\n", test2);
	printf("\tbuff_size==%d; argc==%d, argv[%d]==%p\n", return_malloc_size, return_argc, return_argc + 1, return_argv);
#endif

#ifdef DEBUG_CHILD_ARGS_11
	printf("\n");
	printf("HEX_DUMP:\n");
	// hd_buff((unsigned char *) argv_array_pointer_pp, (long unsigned int) 0, malloc_argv_size, 0, 1);
	hd_buff((unsigned char *) return_argv, (unsigned char *) return_argv, return_malloc_size, 0, 1);
#endif

#ifdef DEBUG_CHILD_ARGS_12
	printf("\n");
	printf("ARGV_DUMP:\n");
	// child_args_argv_dump(&word_count, &argv_array_pointer_pp);
	child_args_argv_dump(&return_argc, &return_argv);
#endif

	free(return_argv);
	return_argv = NULL;

	return 0; /* ok */
}

#endif /* STANDALONE_CONNSAT_CHILD_ARGS */

int
child_args_parse_string (
	char *command_string_p,
	int *return_malloc_size_p,
	int *return_argc_p,
	char ***return_argv_ppp
)
{
	int len;
	int real_len;
	int char_len;
	int last_c_space_flag;
	int word_count;
	int i;	/* char index */
	int x;	/* word index */
	char *malloc_argv_space_buffer_p;
	int malloc_argv_size;
	char **argv_array_pointer_pp;
	char *argv_array_string_p;
	char *new_argv_array_string_p;
	char **new_argv_array_string_pp;
	int new_word_count;

	// convert string 41: "/bin/bash -i  -o --something asdf  qwert1
		// argv[0] = /bin/bash
		// argv[1] = -i
		// argv[2] = -o
		// argv[3] = --something
		// argv[4] = asdf
		// argv[5] = qwert1
		// argv[6] = NULL

	len = strlen(command_string_p);

	real_len = len;
	last_c_space_flag = -1;
	char_len = 0;
	word_count = 0;
	malloc_argv_size = 0;

#ifdef DEBUG_CHILD_ARGS_01
	printf("convert string %d: \"%s\"\n", len, command_string_p);
#endif

	for (i = 0; i < len; ++i) {

		if ( command_string_p[i] == 0x20 ) {		// space-key

			if (last_c_space_flag == 1) {
				--real_len;

#ifdef DEBUG_CHILD_ARGS_01
				printf("\t%d: real_len==%d\n", i, real_len);
#endif
			}

			last_c_space_flag = 1;	// last char is a WS

		} else if ( command_string_p[i] == 0x09 ) {	// horizontal-tab-key

			last_c_space_flag = 1;	// last char is a WS

			if (last_c_space_flag == 1) {
				--real_len;

#ifdef DEBUG_CHILD_ARGS_01
				printf("\t%d: real_len==%d\n", i, real_len);
#endif
			}

		} else {
			++char_len;	// count non-WS chars

			if (last_c_space_flag == -1) {
				// start of first word

				++word_count;

#ifdef DEBUG_CHILD_ARGS_01
				printf("\t%d: word_count==%d\n", i, word_count);
#endif

			} else if (last_c_space_flag == 1) {
				// start of new word

				++word_count;

#ifdef DEBUG_CHILD_ARGS_01
				printf("\t%d: word_count==%d\n", i, word_count);
#endif
			}

			last_c_space_flag = 0;	// last char not a WS
		}
	}

#ifdef DEBUG_CHILD_ARGS_02
	printf("\n");
	printf("PARAMS:\n");
	printf("\tlen==%d, real_len==%d, char_len==%d, word_count==%d\n", len, real_len, char_len, word_count);

	printf("\tWe need %d pointers of size==%ld for argv[] pointer array, TOTAL_POINTERS_BYTES==%ld.\n",
		word_count + 1,
		sizeof(char *),
		(word_count + 1) * sizeof(char *)
	);
	printf("\tWe need %d bytes for argv[] strings, including last '\\0'\n", real_len + 1);
#endif

	malloc_argv_size =
		(word_count + 1) * sizeof(char *) + (real_len + 1);

#ifdef DEBUG_CHILD_ARGS_02
	printf("\tTOTAL_ARGV[]_ARRAY_BYTES==%d.\n",
		malloc_argv_size
	);
#endif

	malloc_argv_space_buffer_p = malloc(malloc_argv_size);
	if (malloc_argv_space_buffer_p == NULL) {

		printf("Out of Memory, abort\n");

		return -1;
	}

#ifdef DEBUG_CHILD_ARGS_02
	printf("\n");
	printf("Successfully Malloc'd new space:\n");
	printf("\tmalloc'd[%d 0x%x]==%p - %p\n",
		malloc_argv_size,
		malloc_argv_size,
		malloc_argv_space_buffer_p,
		malloc_argv_space_buffer_p + malloc_argv_size - 1
	);
#endif

#ifdef DEBUG_CHILD_ARGS_02
	printf("\n");
	printf("Clear new space:\n");
#endif
	/* clear the allocated space */
	memset( (void *) malloc_argv_space_buffer_p, 0, malloc_argv_size);

#ifdef DEBUG_CHILD_ARGS_02
	printf("\n");
	printf("New argv[%d] buffer at %p, of %d bytes\n",
		word_count + 1,
		malloc_argv_space_buffer_p,
		malloc_argv_size
	);
#endif

	argv_array_pointer_pp = (char **) malloc_argv_space_buffer_p;

	/*
	** x = {0 to (word_count + 0)}	argv[0] to argv[x-1]
	** x = {(word_count + 1)}	argv[x], NULL pointer at the end of the list
	** x = {(word_count + 2)}	argv[x+1], NULL terminated strings that the argv[0...] point to
	*/
	for (x = 0; x <= (word_count + 1); ++x) {

		if (x < (word_count)) {
			argv_array_pointer_pp[x] = "qwerty";
#ifdef DEBUG_CHILD_ARGS_02
			printf("\targv[%d]==%p\t%p\n", x, &argv_array_pointer_pp[x], argv_array_pointer_pp[x]);
#endif
		} else if (x == (word_count)) {
			argv_array_pointer_pp[x] = NULL;

#ifdef DEBUG_CHILD_ARGS_02
			// printf("\targv[%d]==%p\t%%p\n", x, &argv_array_pointer_pp[x], argv_array_pointer_pp[x]);
			printf("\targv[%d]==%p\tNULL_PTR\n", x, &argv_array_pointer_pp[x]);
#endif
		} else {
			argv_array_pointer_pp[x] = NULL;

#ifdef DEBUG_CHILD_ARGS_02
			// printf("\targv[%d]==%p\t%%p\n", x, &argv_array_pointer_pp[x], argv_array_pointer_pp[x]);
			printf("\targv[%d]==%p\tSTRINGS\n", x, &argv_array_pointer_pp[x]);
#endif
		}

	}

	/* start of the new *argv[] chars */
	argv_array_string_p = (char *) (argv_array_pointer_pp + (word_count + 1));

#ifdef DEBUG_CHILD_ARGS_02
	printf("\n");
	printf("STRINGS_STORAGE\n");
	printf("\targv[%d]==%p\t# argv_array_string_p, START_OF_STRINGS_ARRAY_STORAGE\n", word_count + 1, argv_array_string_p);
#endif

#ifdef DEBUG_CHILD_ARGS_03
	printf("\n");
	printf("PREPARE_TO_COPY_ARGV_STRINGS:\n");
	printf("\tcopy input string [%d]: %p \"%s\"\n", len, command_string_p, command_string_p);
	printf("\n");
#endif

	new_argv_array_string_p = argv_array_string_p;
	last_c_space_flag = -1;
	new_word_count = 0;

	// note: zero to len, not to len-1
	for (i = 0; i <= len; ++i) {

		if ( command_string_p[i] == 0x20 ) {		// space-key

			if (last_c_space_flag == 1) {
				// --real_len;

			} else {
				/* copy in the new char */
				// *new_argv_array_string_p++ = command_string_p[i];
#ifdef DEBUG_CHILD_ARGS_03
				printf("\t@ copy input string [%d/%d]: %p == '\\0'\t from %p\n", i, len, new_argv_array_string_p, &command_string_p[i]);
#endif
				*new_argv_array_string_p = '\0';
				++new_argv_array_string_p;

			}

			last_c_space_flag = 1;	// last char is a WS

		} else if ( command_string_p[i] == 0x09 ) {	// horizontal-tab-key

			last_c_space_flag = 1;	// last char is a WS

			if (last_c_space_flag == 1) {
				// --real_len;

			} else {
				/* copy in the new char */

				// *new_argv_array_string_p++ = command_string_p[i];
#ifdef DEBUG_CHILD_ARGS_03
				printf("\t@ copy input string [%d/%d]: %p == '\\0'\t from %p\n", i, len, new_argv_array_string_p, &command_string_p[i]);
#endif
				*new_argv_array_string_p = '\0';
				++new_argv_array_string_p;
			}

		} else if ( command_string_p[i] == 0x00 ) {	// end of string

				/* copy in the new char */

#ifdef DEBUG_CHILD_ARGS_03
				printf("\t$ copy input string [%d/%d]: %p == '\\0'\t from %p\n", i, len, new_argv_array_string_p, &command_string_p[i]);
#endif
				*new_argv_array_string_p = '\0';
				++new_argv_array_string_p;
		} else {
			// ++char_len;	// count non-WS chars

			if (last_c_space_flag == -1) {
				// start of first word

				// ++word_count;

				/* copy in the new char */
#ifdef DEBUG_CHILD_ARGS_03
				printf("\t* copy input string [%d/%d]: %p == '%c'\t from %p\n", i, len, new_argv_array_string_p, command_string_p[i], &command_string_p[i]);
#endif

#ifdef DEBUG_CHILD_ARGS_04
					printf("\t\tSET[%d]: %p to %p\n", new_word_count, argv_array_pointer_pp[new_word_count], new_argv_array_string_p);
#endif

				argv_array_pointer_pp[new_word_count] = new_argv_array_string_p;
				++new_word_count;

				*new_argv_array_string_p = command_string_p[i];
				++new_argv_array_string_p;

			} else if (last_c_space_flag == 1) {
				// start of new word

				// ++word_count;

				/* copy in the new char */
#ifdef DEBUG_CHILD_ARGS_03
				printf("\t> copy input string [%d/%d]: %p == '%c'\t from %p\n", i, len, new_argv_array_string_p, command_string_p[i], &command_string_p[i]);
#endif

#ifdef DEBUG_CHILD_ARGS_04
					printf("\t\tSET[%d]: %p to %p\n", new_word_count, argv_array_pointer_pp[new_word_count], new_argv_array_string_p);
#endif

				argv_array_pointer_pp[new_word_count] = new_argv_array_string_p;
				++new_word_count;

				*new_argv_array_string_p = command_string_p[i];
				++new_argv_array_string_p;

			} else {
				// continuation of word

				/* copy in the new char */
#ifdef DEBUG_CHILD_ARGS_03
				printf("\t+ copy input string [%d/%d]: %p == '%c'\t from %p\n", i, len, new_argv_array_string_p, command_string_p[i], &command_string_p[i]);
#endif
				*new_argv_array_string_p = command_string_p[i];
				++new_argv_array_string_p;
			}

			last_c_space_flag = 0;	// last char not a WS
		}
	}

#ifdef DEBUG_CHILD_ARGS_05
	printf("\n");
	printf("FINAL_DUMP:\n");

	/*
	** x = {0 to (word_count + 0)}	argv[0] to argv[x-1]
	** x = {(word_count + 1)}	argv[x], NULL pointer at the end of the list
	** x = {(word_count + 2)}	argv[x+1], NULL terminated strings that the argv[0...] point to
	*/
	for (x = 0; x < (word_count + 1); ++x) {

		if ( argv_array_pointer_pp[x] != NULL ) {
			printf("\targv[%d]==%p\t%p\t\t\"%s\"\n", x, &argv_array_pointer_pp[x], argv_array_pointer_pp[x], argv_array_pointer_pp[x]);
		} else {
			printf("\targv[%d]==%p\tNULL\n", x, &argv_array_pointer_pp[x]);
		}
	}
#endif

#ifdef DEBUG_CHILD_ARGS_05
	printf("\n");
	printf("HEX_DUMP:\n");
	// hd_buff((unsigned char *) argv_array_pointer_pp, (long unsigned int) 0, malloc_argv_size, 0, 1);
	hd_buff((unsigned char *) argv_array_pointer_pp, (unsigned char *) argv_array_pointer_pp, malloc_argv_size, 0, 1);
#endif

#if 0
	printf("\n");
	printf("ARGV_DUMP:\n");
	child_args_argv_dump(&word_count, &argv_array_pointer_pp);
#endif

#ifdef DEBUG_CHILD_ARGS_03
//	printf("DONE\n");
#endif

	if (return_malloc_size_p) {
		*return_malloc_size_p = malloc_argv_size;
	}

	if (return_argc_p) {
		*return_argc_p = word_count;
	}

	if (return_argv_ppp) {
		*return_argv_ppp = argv_array_pointer_pp;
	}

	return 0;
}

int
child_args_argv_dump (
	int *argc_p,
	char ***argv_ppp,
	int raw_mode_flag)
{
	int x;
	int y;

	char *newline_string_p;

	if (raw_mode_flag) {
		newline_string_p = "\r\n";
	} else {
		newline_string_p = "\n";
	}

#ifdef DEBUG_CHILD_ARGS_03
	printf("Size of a pointer on this target is %ld bytes or %ld bits%s",
		sizeof(void *), sizeof(void *) * 8,
		newline_string_p);
	printf("%s",
		newline_string_p);
#endif

	printf("Address of main's argc formal parameter is %p%s", argc_p, newline_string_p);
	printf("\tValue in main's argc formal parameter is %d%s", *argc_p, newline_string_p);
	printf("%s", newline_string_p);
	printf("Address of main's argv formal parameter is %p%s", argv_ppp, newline_string_p);
	printf("\tValue in main's argv formal parameter is %p%s", *argv_ppp, newline_string_p);
	printf("%s", newline_string_p);

	/* count number of array elements in argv[], up to last NULL pointer */
	for (y=0; (*argv_ppp)[y] != 0; ++y) {
	}

#if 1
	/* dump argv[] array of pointers */
	printf("argv is an array [%d] of pointers to (char *)%s", y+1, newline_string_p);
	printf("\tSize of entire %d element argv array is %d bytes%s", y+1, (y+1) * 8, newline_string_p);
	printf("%s", newline_string_p);

	for (x=0; x<= *argc_p; ++x) {
		printf("\tAddress of argv[%d] array cell is %p%s",
			x,
			&(*argv_ppp)[x],
			newline_string_p);
		printf("\t\tValue of argv[%d] array cell is %p:%s",
			x,
			(*argv_ppp)[x],
			newline_string_p);
	}
	printf("%s", newline_string_p);
#endif

	printf("argv is an array [%d] of pointers to (char *)%s", y+1, newline_string_p);
	printf("\t&argv[?] Array-cell-address  Cell-value(pointer) String pointed to%s", newline_string_p);
	printf("\t-------  ------------------  ------------------  -----------------%s", newline_string_p);
	for (x = 0; x < *argc_p; ++x) {
		printf("\t&argv[%d]: %p: %p \"%s\"%s",
			x,
			&(*argv_ppp)[x],
			(*argv_ppp)[x],
			(*argv_ppp)[x],
			newline_string_p);
	}

	if ((*argv_ppp)[x] == 0) {
		/* correct */
		printf("\t&argv[%d]: %p: %p: (NULL_PTR)%s",
			x,
			&(*argv_ppp)[x],
			(*argv_ppp)[x],
			newline_string_p);
	} else {
		/* This would be wrong, last element
  		** in argv array is zero pointer
		*/
		printf("\t&argv[%d]: %p: %p: \"%s\"\t#ERROR, last pointer is not NULL!!!%s",
			x,
			&(*argv_ppp)[x],
			(*argv_ppp)[x],
			(*argv_ppp)[x],
			newline_string_p);
	}

	return 0;
}


/* +++++++++++++++++++++++ ut_hd.c +++++++++++++++++++++++++++ */

/****************************************************************
 *                                                              *
 *              Copyright (c) 1990 William A. Fraser            *
 *                      All Rights Reserved                     *
 *                                                              *
 *      THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF WAF      *
 *    The copyright notice above does not evidence any actual   *
 *        or intended publication of such source code.          *
 *                                                              *
 * This program contains information proprietary to             *
 * William A. Fraser.                                           *
 * It may not be distributed or used without permission         *
 * of William A. Fraser.                                        *
 ****************************************************************/

/*
 * This is a hex data dump package.  There are two fundamental ways
 * to use this package.  With the first a pointer to a buffer is
 * specified along with the display address, the size of the buffer,
 * and two mode of operation integer flags.  The first flag "do_stars",
 * if not zero, enables a mode where duplicate display lines are shown
 * as one instance of that line followed by a single line containing
 * only an asterisk.  The second flag "show_eof signifies that the
 * address of the last address is to be displayed as the end-of-file (EOF).
 *    void hd_buff(bf, da, sz, do_stars, show_eof)
 *
 * The second method of use is implemented with three function calls. 
 * The main function for hex dump display is hd_putc().  It takes a
 * single character as an argument.  Before using this function to
 * display data, a call to hd_init() must be done first.  It takes the
 * display address and an integer flag as arguments.  The flag, if not
 * zero, enables a mode where duplicate display lines are shown as one
 * instance of that line followed by a single line containing only an
 * asterisk.  After as many calls to hd_putc() as desired a call to
 * the function hd_done() is required to flush the output buffers.  This
 * function requires one argument which is an integer flag.  This flag
 * signifies that the address of the last address is to be displayed as
 * the end-of-file (EOF).
 *    void hd_init(da, do_stars)
 *    void hd_putc(c)
 *    void hd_done(show_eof)
 *
 * Typical output with a "da" of 0, "do_stars" of true, and a "show_eof"
 * of true:
 *    00000000:    5090010b  00004800  00001000  00007b74    P.....H.......{t
 *    00000010:    00000480  00000000  00000000  00000000    ................
 *    00000020:    00000000  00000000  00000000  00000000    ................
 *    *
 *    00000800:    01000430  01000471  010004b2  a1000480    ...0...q........
 *    00000810:    00004800  4040003e  000007c4  4040003e    ..H.@@.>....@@.>
 *    00000820:    000045c0  11000070  2060003e  00000000    ..E....p `.>....
 *    00000830:    00000000  00000000  00000000  00000000    ................
 *    *
 *    00006800:    @
 */

#include <stdio.h>
#include <ctype.h>

#define MAXCOUNT 32
static char conv[]="0123456789abcdef";
static int s_flg=1; /* enable stars for duplicated lines */

static void hd_flush();
static void hd_output();

static char in[MAXCOUNT];
static int incnt;

static char last[MAXCOUNT];
static int lastincnt=0;

static unsigned char *hd_offset = 0; /* buffer print address offset */

static unsigned char *laddr;   /* current line address */
static int addr; /* current byte address */
static int star=0;  /* last line was a star line */

static int maxcount=MAXCOUNT/2;

static FILE *fp;

void
hd_init(da, do_stars)
unsigned char *da;       /* display address */
int do_stars; /* duplicate lines indicated by single line with '*' */
{
	int x;

	lastincnt = 0;
	incnt = 0;
	addr = 0;
	hd_offset = da;

	for(x = 0; x < (MAXCOUNT); ++x) {
		in[x]= 0;
		last[x]= 0;
	}

	if( !do_stars ) {
		s_flg=0;
	}
}

void
hd_done(show_eof)
int show_eof;
{
	if(incnt) {
		hd_flush();
	}

	if(show_eof) {
		printf("%p:    @\n", addr + hd_offset);
	}
}

void
hd_putc(int c)
{
	if (incnt == 0) {
		laddr = addr + hd_offset;
	}
	in[incnt++] = c;
	++addr;
	if (incnt >= maxcount) {
		hd_flush();
	}
}

static
void
hd_flush()
{
	int diff;
	int x;

	if (fp == 0) {
		fp = stdout;
	}
	diff=0;
	if (lastincnt != incnt) {
		diff=1;
	} else {
		for(x=0;x<incnt;++x) { /* is this line the diff than the last line? */
			if (last[x] != in[x]) {
				diff=1;
				break;
			}
		}
	}
	if (incnt) {
		if (diff || s_flg==0) { /* if diff then print line */
			hd_output(in);
			star=0;
		} else { /* if not diff then print star line */
			if (star==0) { /* only if last was not a star line also */
				putc('*',fp);
				putc('\n',fp);
				star=1;
			}
		}
	}
	for(x=0;x<incnt;++x) { /* save line for future duplicate check */
		last[x] = in[x];
		lastincnt=incnt;
	}
	incnt=0;
}

static
void
hd_output(in)
char *in;
{
	char ch[MAXCOUNT];
	char hx[2*MAXCOUNT];
	int i;
	int x;

	if (fp == 0) {
		fp = stdout;
	}
	for(x=0;x<maxcount;++x) ch[x]=' ';
	for(x=0;x<2*maxcount;x+=2) {hx[x]=' '; hx[x+1]=' ';}
	for(x=0;x<incnt;++x) {
		if (in[x]>=0 && isprint(in[x])) {
			ch[x]=in[x];
		} else {
			ch[x]='.';
		}
		hx[2*x]   = conv[(in[x] & 0xf0)>>4];
		hx[2*x+1] = conv[in[x] & 0x0f];
	}

	printf("%p:    ",laddr);
	for(x=0;x<maxcount;++x) {
		putc(hx[2*x],fp);
		putc(hx[2*x+1],fp);
		/* putc(' ',fp); */
		/* if (x==maxcount/2-1) putc(' ',fp); */
		if ((x+1)%4==0) {
			putc(' ',fp);
			putc(' ',fp);
		}
	}
	putc(' ',fp);
	putc(' ',fp);
	for(x=0;x<maxcount;++x) {
		putc(ch[x],fp);
		/* if (x==maxcount/2-1) putc(' ',fp); */
	}
	putc('\n',fp);
}

void
hd_buff (
	unsigned char *bf, /* address of buffer */
	unsigned char *da, /* print address of buffer */
	int sz, /* size of buffer */
	int do_stars, /* duplicate lines indicated by single line with '*' */
	int show_eof
)
{
	unsigned char *bf_p;

	hd_init(da, do_stars);
	for(bf_p = bf; bf_p < (bf + sz); ++bf_p) {
		hd_putc(*bf_p);
	}
	hd_done(show_eof);
}

#ifdef STANDALONE_HEXDUMP

#ifdef TCFIX
#include <fcntl.h>
#endif

int
main (int ac, char **av)
{
	int c;

#ifdef TCFIX
	setmode(0, O_BINARY);
#endif

	hd_init(0, 2 - ac);
	while( (c = getc(stdin)) != EOF) {
		hd_putc(c);
	}
	hd_done(1);
}
#endif /* STANDALONE_HEXDUMP */

