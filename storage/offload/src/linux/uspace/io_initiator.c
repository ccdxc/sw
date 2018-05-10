#include "offloader.h"

static bool g_sim_inited;
uint8_t session_mem[PNSO_TEST_SESSION_MEM];
static struct io_ctx io1, io2;

//Initialize io_ctx such that it can be consumed by function threads
int fill_io_ctx(struct io_ctx *io, char *name, int seed)
{
	memset(io, 0, sizeof(*io));
	io->tchain.num_threads = THREAD_TYPE_MAX;
	io->tchain.current_thread = 0;
	for (int i = 0; i < THREAD_TYPE_MAX; i++) {
		io->tchain.threads[i] = i;
		io->src_buflist[i] = malloc(sizeof(struct pnso_buffer_list));
		io->dst_buflist[i] = malloc(sizeof(struct pnso_buffer_list));
		io->src_buflist[i]->count = 1;
		io->dst_buflist[i]->count = 1;
		if (i > 0) {
			io->src_buflist[i]->buffers[0].buf = (uint64_t)io->dst_buflist[i-1]->buffers[0].buf;
			io->src_buflist[i]->buffers[0].len = io->dst_buflist[i-1]->buffers[0].len;
			io->dst_buflist[i]->buffers[0].buf = (uint64_t)io->tbuf[i].buf;
			io->dst_buflist[i]->buffers[0].len = PNSO_TEST_DATA_SIZE;
		} else {
			io->src_buflist[i]->buffers[0].buf = (uint64_t)io->tbuf[i].buf;
			io->src_buflist[i]->buffers[0].len = PNSO_TEST_DATA_SIZE;
			io->dst_buflist[i]->buffers[0].buf = (uint64_t)io->tbuf[i].buf;
			io->dst_buflist[i]->buffers[0].len = PNSO_TEST_DATA_SIZE;
		}
	}
	// Feeding random data currently seems to fail due to compression threshold len issue
	/*srand(time(NULL));
	for(int i = 0; i < PNSO_TEST_DATA_SIZE/sizeof(int); i++) {
		((int*)io1.tbuf[0].buf)[i] = rand();
	}*/
	strncpy(io->name, name, 64);
	memset(io->tbuf[0].buf, seed, PNSO_TEST_DATA_SIZE);
	return 0;
}

int exec_io_initiator(void *arg1, void *arg2)
{
	int seed = 1;

	if (!g_sim_inited) {
		struct pnso_init_params init_params;

		memset(&init_params, 0, sizeof(init_params));
		/* Initialize session */
		init_params.cp_hdr_version = 1;
		init_params.per_core_qdepth = 16;
		init_params.block_size = 4096;
		pnso_init(&init_params);

		/* Initialize key store */
		char *tmp_key = NULL;
		uint32_t tmp_key_size = 0;
		char abcd[] = "abcd";

		pnso_sim_key_store_init((uint8_t *)malloc(64*1024), 64*1024);
		pnso_set_key_desc_idx(abcd, abcd, 4, 1);
		pnso_sim_get_key_desc_idx((void **)&tmp_key, (void **)&tmp_key, &tmp_key_size, 1);
	}
	fill_io_ctx(&io1, "io1", seed++);
	fill_io_ctx(&io2, "io2", seed++);

	spdk_event_call(spdk_event_allocate(
			io1.tchain.threads[++io1.tchain.current_thread],
			offload_fn, &io1, NULL));
	spdk_event_call(spdk_event_allocate(
			io2.tchain.threads[++io2.tchain.current_thread],
			offload_fn, &io2, NULL));

	return 0;
}

void completion_fn(void *arg1, void *arg2)
{
	struct io_ctx *io = (struct io_ctx *)arg1;
	int rc;

	if (arg1 == NULL) return;
	printf("IO %s completed\n", io->name);

	rc = memcmp(io->tbuf[0].buf, io->tbuf[io->tchain.num_threads-1].buf, PNSO_TEST_DATA_SIZE);
	if (rc != 0) printf("IO src and dest buffer comparison failed\n");
	else printf("IO src and dest buffer comparison passed\n");
	return;
}

