#define _GNU_SOURCE
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#include "relf.h"

/* We used to use 'atexit'. Some programs exit without running the handler.
 * So try attribute(destructor). Same! */
static void print_exit_summary(void) __attribute__((destructor));
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
			/* We "just print" the whole line if we weren't asked to only print for ptreetime. */
			if (!getenv("LIBDUMPSCHED_ONLY_PTREETIME")) { fprintf(stderr, "%s", linebuf); goto continue_loop; }
			/* ... or if it's the line naming the process -- FIXME: reformat onto one line */
			if (first_line)
			{
				char *saveptr;
				char *line_content = strtok_r(linebuf, "\n", &saveptr);
				char *newline_or_end = strtok_r(NULL, "\n", &saveptr);
				if (newline_or_end) line_content[newline_or_end - line_content - 1] = '\0';
				fprintf(stderr, "terminated: %s, with se.sum_exec_runtime ", line_content); goto continue_loop;
			}
			if (0 == strncmp(linebuf, "se.sum_exec_runtime", sizeof "se.sum_exec_runtime" - 1))
			{
				/* NOTE that nread is the realloc'd buffer size, NOT the line length.
				 * So use fprintf to print only up to the null terminator. */
				char *saveptr;
				char *se_sum = strtok_r(linebuf, " \t", &saveptr);
				char *colon = strtok_r(NULL, " \t", &saveptr);
				char *number = strtok_r(NULL, " \t", &saveptr);
				fprintf(stderr, "%s", number);
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

extern char **environ;
static void init(void) __attribute__((constructor));
static void init(void)
{
	if (getenv("LIBDUMPSCHED_DELAY_STARTUP"))
	{
		sleep(1);
	}
	int dummy;
	ElfW(auxv_t) *p_auxv = get_auxv(environ, &dummy);
	struct auxv_limits lims = get_auxv_limits(p_auxv);
	fprintf(stderr, "pid %d, command: ", getpid());
	for (const char **p = lims.argv_vector_start; *p; ++p)
	{
		fprintf(stderr, "%s'%s'", (p != lims.argv_vector_start) ? ", " : "", *p);
	}
	fprintf(stderr, "\n");

	//atexit(print_exit_summary);
}

