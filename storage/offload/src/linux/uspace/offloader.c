#include "offloader.h"

static struct spdk_poller *test_end_poller;
static bool g_app_stopped;
static int g_time_in_sec;
static int g_reqs_eff_thread;
static int g_reqs_xor_thread;
static int g_reqs_read_thread;

void
offload_fn(void *arg1, void *arg2)
{
	uint32_t thread_num = spdk_env_get_current_core();

	switch (thread_num % THREAD_TYPE_MAX) {
		case THREAD_TYPE_IO_INITIATOR: {
			exec_io_initiator(arg1, arg2);
		}
		break;
		case THREAD_TYPE_STORAGE_EFFECIENCY: {
			exec_eff_thread(arg1, arg2);
			g_reqs_eff_thread++;
			if (g_app_stopped) {
				printf("\nHandled %d offload requests in effeciency thread num %d",
					g_reqs_eff_thread, thread_num);
			}
		}
		break;
		case THREAD_TYPE_XOR: {
			exec_xor_thread(arg1, arg2);
			g_reqs_xor_thread++;
			if (g_app_stopped) {
				printf("\nHandled %d offload requests in xor thread num %d",
					g_reqs_xor_thread, thread_num);
			}
		}
		break;
		case THREAD_TYPE_READ: {
			exec_read_thread(arg1, arg2);
			g_reqs_read_thread++;
			if (g_app_stopped) {
				printf("\nHandled %d offload requests in read thread num %d",
					g_reqs_read_thread, thread_num);
			}
		}
		break;
	}
	if (g_app_stopped) {
		return;
	}
}

static void
__test_end(void *arg)
{
	printf("test_end\n");
	g_app_stopped = true;
	spdk_poller_unregister(&test_end_poller);
	spdk_app_stop(0);
}

static void
test_start(void *arg1, void *arg2)
{
	int i;

	printf("test_start\n");

	/* Register a poller that will stop the test after the time has elapsed. */
	test_end_poller = spdk_poller_register(__test_end, NULL,
					       g_time_in_sec * 1000000ULL);

	SPDK_ENV_FOREACH_CORE(i) {
		spdk_event_call(spdk_event_allocate(i, offload_fn,
						    NULL, NULL));
	}
}

static void
test_cleanup(void)
{
	printf("test_abort\n");
	g_app_stopped = true;
	spdk_poller_unregister(&test_end_poller);
	spdk_app_stop(0);
}

static void
usage(char *program_name)
{
	printf("%s options\n", program_name);
	printf("\t[-m core mask for distributing I/O submission/completion work\n");
	printf("\t\t(default: 0x1 - use core 0 only)]\n");
	printf("\t[-t time in seconds]\n");
}

int
main(int argc, char **argv)
{
	struct spdk_app_opts opts = {};
	int op;
	int rc = 0;

	spdk_app_opts_init(&opts);
	opts.name = "shell";
	opts.shutdown_cb = test_cleanup;

	g_time_in_sec = 0;

	while ((op = getopt(argc, argv, "m:t:")) != -1) {
		switch (op) {
		case 'm':
			opts.reactor_mask = optarg;
			break;
		case 't':
			g_time_in_sec = atoi(optarg);
			break;
		default:
			usage(argv[0]);
			exit(1);
		}
	}

	if (!g_time_in_sec) {
		usage(argv[0]);
		exit(1);
	}

	printf("Running I/O for %d seconds...", g_time_in_sec);
	fflush(stdout);

	rc = spdk_app_start(&opts, test_start, NULL, NULL);

	spdk_app_fini();
	printf("\ndone.\n");
	return rc;
}
