#define _GNU_SOURCE
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

static void print_exit_summary(void)
{
	FILE *sched = fopen("/proc/self/sched", "r");
	_Bool first_line = 1;
	
	if (sched)
	{
		/* read lines from it */
		char *linebuf = NULL;
		ssize_t nread;
		while (getline(&linebuf, &nread, sched) != -1)
		{
			if (
					!getenv("LIBDUMPSCHED_ONLY_PTREETIME")
					|| first_line
					|| 0 == strncmp(linebuf, "se.sum_exec_runtime", sizeof "se.sum_exec_runtime" - 1)
				)
			{
				/* NOTE that nread is the realloc'd buffer size, NOT the line length.
				 * So use fprintf to print only up to the null terminator. */
				fprintf(stderr, "%s", linebuf);
			}
	continue_loop:
			first_line = 0;
		} // end while
		if (linebuf) free(linebuf);

		fclose(sched);
	}
	else fprintf(stderr, "Couldn't read from sched!\n");
	fflush(stderr);
}

static void init(void) __attribute__((constructor));
static void init(void)
{
	if (getenv("LIBDUMPSCHED_DELAY_STARTUP"))
	{
		sleep(1);
	}
	atexit(print_exit_summary);
}

