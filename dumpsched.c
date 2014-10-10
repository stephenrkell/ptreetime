#define _GNU_SOURCE
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <elf.h>
#include <sys/auxv.h>
#include <link.h>          /* for ElfW(...) macro */
#include <linux/auxvec.h>  /* For AT_xxx definitions */

static char *getprogname(void)
{
	/* Dig into the auxv. We could find it within our address space,
	 * but that's hard work. Read it, one record at a time, from /proc. */
	ElfW(auxv_t) buf;
	FILE *auxv = fopen("/proc/self/auxv", "r");
	if (!auxv) return "(no auxv)";

	size_t nread;
	while (0 != (nread = fread(&buf, sizeof buf, 1, auxv)))
	{
		if (buf.a_type == AT_EXECFN)
		{
			return basename((char*) buf.a_un.a_val);
		}
	}
	return "(auxv but no AT_EXECFN)";
}

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
	else
	{
		FILE *schedstat = fopen("/proc/self/schedstat", "r");
		
		if (schedstat)
		{
			fprintf(stderr, "%s (%d)\n", getprogname(), getpid());
			/* https://www.kernel.org/doc/Documentation/scheduler/sched-stats.txt */
			long int t_cpu, t_runqueue, n_slices_on_this_cpu;
			int ret = fscanf(schedstat, "%ld %ld %ld\n", 
				&t_cpu, &t_runqueue, &n_slices_on_this_cpu);
			if (ret == 3)
			{
				fprintf(stderr, "schedstat total time: %ld\n", t_cpu + t_runqueue);
			}
			else
			{
				fprintf(stderr, "Could not parse schedstat\n");
			}

		}
		else
		{
			fprintf(stderr, "Couldn't read from sched!\n");
		}

	}
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

