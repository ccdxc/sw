#include "spdk/stdinc.h"

#include "spdk/env.h"
#include "spdk/event.h"
#include "spdk/log.h"
#include "spdk/io_channel.h"
#include "pnso_sim_api.h"
#include "pnso_sim.h"

enum  thread_types {
	THREAD_TYPE_IO_INITIATOR,
	THREAD_TYPE_STORAGE_EFFECIENCY,
	THREAD_TYPE_XOR,
	THREAD_TYPE_READ,
	THREAD_TYPE_MAX,
};

#define PNSO_TEST_SESSION_MEM (1024 * 1024)
#define PNSO_TEST_DATA_SIZE (4 * 1024)

#define PNSO_INVALID_ARG 1
#define PNSO_NUM_TAGS 16

struct thread_chain {
	int num_threads;
	int current_thread;
	enum thread_types threads[THREAD_TYPE_MAX];
};

struct thread_buffer {
	uint8_t buf[PNSO_TEST_SESSION_MEM];
	struct pnso_hash_tag hash_tags[PNSO_NUM_TAGS];
};

struct io_ctx {
	char name[64];
	struct pnso_buffer_list *src_buflist[THREAD_TYPE_MAX];
	struct pnso_buffer_list *dst_buflist[THREAD_TYPE_MAX];
	struct thread_buffer tbuf[THREAD_TYPE_MAX];
	struct thread_chain tchain;
};

struct spdk_poller_ctx {
	struct io_ctx *io;
	void *poll_ctx;
	pnso_poll_fn_t poller;
	struct spdk_poller *spdk_poller_hdl;
};

void offload_fn(void *arg1, void *arg2);
int exec_io_initiator(void *arg1, void *arg2);
int exec_eff_thread(void *arg1, void *arg2);
int exec_xor_thread(void *arg1, void *arg2);
int exec_read_thread(void *arg1, void *arg2);
void completion_fn(void *arg1, void *arg2);

