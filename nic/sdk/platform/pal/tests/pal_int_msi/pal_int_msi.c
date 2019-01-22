/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <sys/time.h>

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "platform/pal/include/pal_int.h"
#include "platform/pal/include/pal.h"

struct pal_int_msi_item {
	struct pal_int_msi_item	*next;
	uint64_t 		addr;
	uint32_t 		data;
	struct pal_int		pal_int;
	pthread_t		pthread;
	unsigned		n;
	const struct timespec	*sleeptime;
	bool			multiple;
	bool			verbose;
};

static void error_exit(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));
static void error_exit(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(1);
}

static void *test_fn(void *arg)
{
	struct pal_int_msi_item *item = arg;
	int rc;
	unsigned last_int_count;
	unsigned i;
	unsigned total_writes;
	unsigned total_ints;
	bool first;

	last_int_count = 0;
	total_writes = 0;
	total_ints = 0;
	first = true;

	for (i = 0; i < item->n; i++) {
		unsigned int_count;
		unsigned delta;
		unsigned j;

		if (item->multiple) {
			for (j = 1; j <= i + 1; j++) {
				total_writes += 1;
				if (item->verbose) {
					printf(".");
					fflush(stdout);
				}

				total_writes += 1;
				pal_reg_wr32(item->addr, item->data);
				rc = nanosleep(item->sleeptime, NULL);
				if (rc == -1)
					error_exit("nanosleep failed: %m\n");
			}
			if (item->verbose)
				printf("\n");
		} else {
			total_writes += 1;
			pal_reg_wr32(item->addr, item->data);
			rc = nanosleep(item->sleeptime, NULL);
			if (rc == -1)
				error_exit("nanosleep failed: %m\n");
		}

		int_count = pal_int_start(&item->pal_int);
		if (first) {
			last_int_count = int_count - 1;	// Fake for first intr
			first = false;
		}

		delta = int_count - last_int_count;
		total_ints += delta;
		pal_int_end(&item->pal_int);

		last_int_count = int_count;
	}

	if (item->verbose)
		printf("fd %d: total writes %u total interrupts %u\n",
			item->pal_int.fd, total_writes, total_ints);
	while (total_writes != total_ints) {
		unsigned int_count;
		unsigned delta;

		int_count = pal_int_start(&item->pal_int);

		delta = int_count - last_int_count;
		printf("int_count %u delta %u\n", int_count, delta);
		total_ints += delta;
		pal_int_end(&item->pal_int);

		last_int_count = int_count;
	}	

	printf("fd %d: all writes accounted for\n", item->pal_int.fd);
	return NULL;
}


/*
 * Start up threads for all tests and then reap them all
 * n_ops:	Number of interrupt handling operations to perform
 * sleeptime	Time to sleep between operations
 * multiple:	If false, trigger one interrupt per pal_int_start(),
 *		otherwise trigger multiple interrupts per pal_int_start
 * verbose:	Print informatoin about what is happening.
 */
static void run_test(unsigned n, const struct timespec *sleeptime,
	bool multiple, bool verbose)
{
	struct pal_int_msi_item *item;
	struct pal_int_msi_item	*head;
	struct pal_int_msi_item *p;
	unsigned count;

	if (verbose)
		printf("Start PAL MSI interrupt tests\n");

	/*
	 * Loop until we open a device, get an error, or run out of devices
	 * to open
	 */
	head = NULL;
	count = 0;
	for (;;) {
		int rc;

		item = calloc(1, sizeof(*item));
		if (item == NULL)
			error_exit("Unable to allocate %zu bytes\n",
				sizeof(*item));

		rc = pal_int_open_msi(&item->pal_int, &item->addr,
			&item->data);
		if (rc == -1) {
			free(item);
			break;
		}

		if (item->verbose)
			printf("fd %d: opened\n", item->pal_int.fd);
		/* Insert in linked list */
		item->next = head;
		head = item;

		item->n = n;
		item->sleeptime = sleeptime;
		item->multiple = multiple;
		item->verbose = verbose;
		count++;
	}

	printf("Opened %u devices\n", count);

	/* Start threads */
	for (p = head; p!= NULL; p = p->next) {
		int rc;

		rc = pthread_create(&p->pthread, NULL, test_fn, p);
		if (rc != 0)
			error_exit("pthread_create failed\n");

		if (p->verbose)
			printf("fd %d: created thread\n", p->pal_int.fd);
	}

	/* Wait for threads to exit */
	for (p = head; p!= NULL; p = p->next) {
		void *retval;
		int rc;

		rc = pthread_join(p->pthread, &retval);
		if (rc != 0)
			error_exit("pthread_join failed\n");
		if (verbose)
			printf("fd %d: closing\n", p->pal_int.fd);
		rc = pal_int_close(&p->pal_int);
		if (rc != 0)
			error_exit("fd %d: pal_int_close failed: %m\n",
				p->pal_int.fd);
	}

	printf("All file descriptors closed\n");
}

static void usage(const char *name)
{
	error_exit("usage: %s -d ns-delay -m-v\n"
		   "where:\n"
		   "-d ns-delay	Nanosecond delay between interrupt triggers\n"
		   "-m		Do multiple triggers per pal_int_start()\n"
		   "-v		Be verbose\n",
		name);
}

int main(int argc, char *argv[])
{
	struct timespec req;
	char *endptr;
	unsigned n_args;
	int opt;
	bool verbose;
	long delay_ns;
	bool multiple;

	verbose = false;
	multiple = false;
	delay_ns = 0;

	while ((opt = getopt(argc, argv, "d:mv")) != -1) {
		switch (opt) {
		case 'v':
			verbose = 1;
			break;

		case 'd':
			delay_ns = strtoul(optarg, &endptr, 0);
			
			if (*optarg == '\0' || *endptr != '\0')
				error_exit("Bad delay value\n");
			break;

		case 'm':
			multiple = true;
			break;

		default:
			usage(argv[0]);
			break;
		}
	}

	n_args = argc - optind;
	if (n_args != 0)
		usage(argv[0]);

	req = (struct timespec) {.tv_sec = 0, .tv_nsec = delay_ns};
	run_test(50, &req, multiple, verbose);

	return 0;
}
