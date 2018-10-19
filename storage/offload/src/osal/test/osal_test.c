/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef __KERNEL__
#include <unistd.h>
#endif
#include "osal_assert.h"
#include "osal_mem.h"
#include "osal_thread.h"
#include "osal_atomic.h"
#include "osal_setup.h"
#include "osal_logger.h"
#include "osal_sys.h"
#include "osal_errno.h"

#include "pnso_api.h"
#include "sim.h"

OSAL_LICENSE("Dual BSD/GPL");

/* Input data defaults */
#define PNSO_TEST_BLOCK_SIZE 4096
#define PNSO_TEST_BLOCK_COUNT 1
#define PNSO_TEST_DATA_SIZE (PNSO_TEST_BLOCK_SIZE * PNSO_TEST_BLOCK_COUNT)

/* Compression defaults */
#define PNSO_TEST_CP_HDR_ALGO_VER 123
#define PNSO_TEST_CP_HDR_FMT_IDX 1

/* Encryption defaults */
static uint8_t iv[64];
#define PNSO_TEST_CRYPTO_IV_ADDR ((uint64_t)iv)
#define PNSO_TEST_CRYPTO_KEY_IDX 1

/* Thread and batching defaults */
#define PNSO_TEST_THREAD_COUNT 2
#define PNSO_TEST_BATCH_DEPTH 2
#define PNSO_TEST_SVC_COUNT 3
#define PNSO_TEST_BUF_COUNT (PNSO_TEST_BATCH_DEPTH * PNSO_TEST_SVC_COUNT)

/* Structs to avoid extra allocs */
struct pnso_multi_buflist {
	struct pnso_buffer_list buflist;
	struct pnso_flat_buffer bufs[PNSO_TEST_BLOCK_COUNT];
	uint8_t data[PNSO_TEST_DATA_SIZE];
};
struct pnso_multi_service_request {
	struct pnso_service_request req;
	struct pnso_service svcs[PNSO_TEST_SVC_COUNT];
};
struct pnso_multi_service_result {
	struct pnso_service_result res;
	struct pnso_service_status svcs[PNSO_TEST_SVC_COUNT];
};

/* Thread and request context */
struct req_state {
	struct thread_state *tstate;
	osal_atomic_int_t req_done;
	struct pnso_multi_buflist buflists[PNSO_TEST_SVC_COUNT+1];
	struct pnso_multi_service_request req;
	struct pnso_multi_service_result res;
	struct pnso_hash_tag hash_tags[PNSO_TEST_BLOCK_COUNT];
	struct pnso_chksum_tag chksum_tag[PNSO_TEST_BLOCK_COUNT];
};
struct thread_state {
	struct req_state reqs[PNSO_TEST_BATCH_DEPTH];
	osal_thread_t wafl_thread;
};

static struct thread_state osal_test_threads[PNSO_TEST_THREAD_COUNT];

static void comp_cb(void *arg1, struct pnso_service_result *svc_res)
{
	struct req_state *rstate = (struct req_state *) arg1;

	OSAL_LOG_INFO("IO: Request(svc %u) completed, err %d, svc_count %u, core %d\n",
		 svc_res->svc[0].svc_type, svc_res->err, svc_res->num_services,
		 osal_get_coreid());
	if (osal_get_coreid() != rstate->tstate->wafl_thread.core_id) {
		OSAL_LOG_ERROR("IO: ERROR: sim worker running on wrong core.\n");
	}
#if 0
	OSAL_LOG_INFO("IO: Final svc status is %u\n", svc_res->svc[2].err);
	OSAL_LOG_INFO("IO: Final length is %d\n",
			svc_res->svc[2].u.dst.data_len);
#endif
	osal_atomic_fetch_add(&rstate->req_done, 1);
}

static void init_buflist(struct pnso_multi_buflist *mbuf, char fill_byte)
{
	size_t i;

	mbuf->buflist.count = PNSO_TEST_BLOCK_COUNT;
	for (i = 0; i < PNSO_TEST_BLOCK_COUNT; i++) {
		mbuf->buflist.buffers[i].len = PNSO_TEST_BLOCK_SIZE;
		mbuf->buflist.buffers[i].buf = (uint64_t) mbuf->data +
			(i * PNSO_TEST_BLOCK_SIZE);
	}
	memset(mbuf->data, fill_byte, PNSO_TEST_DATA_SIZE);
}

/* Fill in service defaults */
static void init_svc_desc(struct pnso_service *svc, uint16_t svc_type)
{
	memset(svc, 0, sizeof(*svc));
	svc->svc_type = svc_type;
	switch (svc_type) {
	case PNSO_SVC_TYPE_ENCRYPT:
	case PNSO_SVC_TYPE_DECRYPT:
		svc->u.crypto_desc.algo_type = PNSO_CRYPTO_TYPE_XTS;
		svc->u.crypto_desc.key_desc_idx = PNSO_TEST_CRYPTO_KEY_IDX;
		svc->u.crypto_desc.iv_addr = PNSO_TEST_CRYPTO_IV_ADDR;
		break;
	case PNSO_SVC_TYPE_COMPRESS:
		svc->u.cp_desc.algo_type = PNSO_COMPRESSION_TYPE_LZRW1A;
		svc->u.cp_desc.flags = PNSO_CP_DFLAG_ZERO_PAD | PNSO_CP_DFLAG_INSERT_HEADER;
		svc->u.cp_desc.threshold_len = PNSO_TEST_DATA_SIZE - 8;
		svc->u.cp_desc.hdr_fmt_idx = PNSO_TEST_CP_HDR_FMT_IDX;
		svc->u.cp_desc.hdr_algo = PNSO_TEST_CP_HDR_ALGO_VER;
		break;
	case PNSO_SVC_TYPE_DECOMPRESS:
		svc->u.dc_desc.algo_type = PNSO_COMPRESSION_TYPE_LZRW1A;
		svc->u.dc_desc.flags = PNSO_DC_DFLAG_HEADER_PRESENT;
		svc->u.dc_desc.hdr_fmt_idx = PNSO_TEST_CP_HDR_FMT_IDX;
		break;
	case PNSO_SVC_TYPE_HASH:
		svc->u.hash_desc.algo_type = PNSO_HASH_TYPE_SHA2_256;
		svc->u.hash_desc.flags = PNSO_HASH_DFLAG_PER_BLOCK;
		break;
	case PNSO_SVC_TYPE_CHKSUM:
		svc->u.chksum_desc.algo_type = PNSO_CHKSUM_TYPE_CRC32C;
		svc->u.chksum_desc.flags = PNSO_CHKSUM_DFLAG_PER_BLOCK;
		break;
	case PNSO_SVC_TYPE_DECOMPACT:
		svc->u.decompact_desc.hdr_fmt_idx = PNSO_TEST_CP_HDR_FMT_IDX;
		break;
	default:
		break;
	}
}

static pnso_error_t submit_requests(struct thread_state *tstate)
{
	int rc;
	size_t batch_id;
	struct req_state *rstate = NULL;
	struct pnso_service_request *svc_req = NULL;
	struct pnso_service_result *svc_res = NULL;

	for (batch_id = 0; batch_id < PNSO_TEST_BATCH_DEPTH; batch_id++) {
		rstate = &tstate->reqs[batch_id];
		svc_req = &rstate->req.req;
		svc_res = &rstate->res.res;

#if (PNSO_TEST_BATCH_DEPTH == 1)
		rc = pnso_submit_request(svc_req, svc_res,
					 comp_cb, rstate,
					 NULL, NULL);
		if (rc != 0) {
			OSAL_LOG_ERROR("pnso_submit_request(svc %u) failed with %d\n",
				 svc_req->svc[0].svc_type, rc);
			return rc;
		}
	}
#else
		rc = pnso_add_to_batch(svc_req, svc_res);
		if (rc != 0) {
			OSAL_LOG_ERROR("pnso_add_to_batch(svc %u) failed with %d\n",
				 svc_req->svc[0].svc_type, rc);
			return rc;
		}
	}

	rc = pnso_flush_batch(comp_cb, rstate, NULL, NULL);
	if (rc != 0) {
		OSAL_LOG_ERROR("pnso_flush_batch(svc %u) failed with %d\n",
			 svc_req->svc[0].svc_type, rc);
		return rc;
	}
#endif

	return 0;
}

static int exec_cp_req(struct thread_state *tstate)
{
	size_t batch_id;
	struct req_state *rstate;
	struct pnso_service_request *svc_req = NULL;
	struct pnso_service_result *svc_res = NULL;

	for (batch_id = 0; batch_id < PNSO_TEST_BATCH_DEPTH; batch_id++) {
		rstate = &tstate->reqs[batch_id];
		svc_req = &rstate->req.req;
		svc_res = &rstate->res.res;
		memset(svc_req, 0, sizeof(*svc_req));
		memset(svc_res, 0, sizeof(*svc_res));

		init_buflist(&rstate->buflists[0], 'A');
		init_buflist(&rstate->buflists[1], 'B');
		init_buflist(&rstate->buflists[2], 'C');

		/* Setup 3 services */
		svc_req->sgl = &rstate->buflists[0].buflist;
		svc_req->num_services = 3;
		svc_res->num_services = 3;

		/* Setup compression service */
		init_svc_desc(&svc_req->svc[0], PNSO_SVC_TYPE_COMPRESS);
		svc_res->svc[0].u.dst.sgl = &rstate->buflists[1].buflist;

		/* Setup hash service */
		init_svc_desc(&svc_req->svc[1], PNSO_SVC_TYPE_HASH);
		svc_res->svc[1].u.hash.num_tags = PNSO_TEST_BLOCK_COUNT;
		svc_res->svc[1].u.hash.tags = rstate->hash_tags;

		/* Setup encryption service */
		init_svc_desc(&svc_req->svc[2], PNSO_SVC_TYPE_ENCRYPT);
		svc_res->svc[2].u.dst.sgl = &rstate->buflists[2].buflist;
	}

	return submit_requests(tstate);
}

static int exec_dc_req(struct thread_state *tstate)
{
	size_t batch_id;
	struct req_state *rstate;
	struct pnso_service_request *svc_req = NULL;
	struct pnso_service_result *svc_res = NULL;

	for (batch_id = 0; batch_id < PNSO_TEST_BATCH_DEPTH; batch_id++) {
		rstate = &tstate->reqs[batch_id];
		svc_req = &rstate->req.req;
		svc_res = &rstate->res.res;
		memset(svc_req, 0, sizeof(*svc_req));
		memset(svc_res, 0, sizeof(*svc_res));

		init_buflist(&rstate->buflists[1], 'X');
		init_buflist(&rstate->buflists[3], 'Y');

		/* Assume encrypt result from previous step is in buflists[2] */
		svc_req->sgl = &rstate->buflists[2].buflist;

		/* Setup 3 services */
		svc_req->num_services = 3;
		svc_res->num_services = 3;

		/* Setup decrypt service */
		init_svc_desc(&svc_req->svc[0], PNSO_SVC_TYPE_DECRYPT);
		svc_res->svc[0].u.dst.sgl = &rstate->buflists[1].buflist;

		/* Setup hash service */
		init_svc_desc(&svc_req->svc[1], PNSO_SVC_TYPE_HASH);
		svc_res->svc[1].u.hash.num_tags = PNSO_TEST_BLOCK_COUNT;
		svc_res->svc[1].u.hash.tags = rstate->hash_tags;

		/* Setup decompression service */
		init_svc_desc(&svc_req->svc[2], PNSO_SVC_TYPE_DECOMPRESS);
		svc_res->svc[2].u.dst.sgl = &rstate->buflists[3].buflist;
	}

	return submit_requests(tstate);
}

static void init_tstate(struct thread_state *tstate)
{
	size_t i;

	memset(tstate, 0, sizeof(*tstate));
	for (i = 0; i < PNSO_TEST_BATCH_DEPTH; i++) {
		osal_atomic_init(&tstate->reqs[i].req_done, 0);
		tstate->reqs[i].tstate = tstate;
	}
}

static int exec_req(void *arg)
{
	int rc;
	struct thread_state *tstate = (struct thread_state *) arg;
	int local_core_id = osal_get_coreid();

	/* Prep the polling thread */
	OSAL_LOG_INFO("PNSO: starting worker thread on core %d\n",
		 osal_get_coreid());

	/* Submit compression requests */
	rc = exec_cp_req(tstate);
	if (rc != 0) {
		OSAL_LOG_ERROR("PNSO: Compression request submit FAILED\n");
		goto error;
	}
	while (1) {
		int cp_done = osal_atomic_read(&tstate->reqs[PNSO_TEST_BATCH_DEPTH-1].req_done);

		if (!cp_done) {
			osal_yield();
		} else {
			break;
		}
	}
	OSAL_LOG_INFO("PNSO: Compression requests done, core %d\n",
		 osal_get_coreid());
	osal_yield();
	if (osal_get_coreid() != local_core_id) {
		OSAL_LOG_ERROR("PNSO: ERROR core id changed unexpectedly\n");
		rc = EINVAL;
		goto error;
	}

	/* Submit decompression requests */
	rc = exec_dc_req(tstate);
	if (rc != 0) {
		OSAL_LOG_ERROR("PNSO: Decompression request submit FAILED\n");
		goto error;
	}
	while (1) {
		int dc_done = osal_atomic_read(&tstate->reqs[PNSO_TEST_BATCH_DEPTH-1].req_done);

		if (dc_done < 2) {
			osal_yield();
		} else {
			break;
		}
	}
	OSAL_LOG_INFO("PNSO: Decompression requests done, core %d\n",
		 osal_get_coreid());
	osal_yield();
	if (osal_get_coreid() != local_core_id) {
		OSAL_LOG_ERROR("PNSO: ERROR core id changed unexpectedly\n");
		rc = EINVAL;
		goto error;
	}

	pnso_sim_thread_finit(local_core_id);	
	OSAL_LOG_INFO("PNSO: Worker thread finished, core %d\n", osal_get_coreid());
	return 0;

error:
	pnso_sim_thread_finit(local_core_id);	
	OSAL_LOG_ERROR("PNSO: Worker thread failed, core %d or %d\n",
		       local_core_id, osal_get_coreid());
	return rc;
}

static pnso_error_t init_crypto(void)
{
	pnso_error_t rc;
	char key1[32] = "abcdefghijklmnopqrstuvwxyz78901";

	if ((rc = pnso_set_key_desc_idx(key1, key1, 32,
					PNSO_TEST_CRYPTO_KEY_IDX)) != 0) {
		return rc;
	}
	return PNSO_OK;
}

static pnso_error_t init_cp_hdr_fmt(void)
{
	pnso_error_t rc;
	struct pnso_compression_header_format cp_hdr_fmt = { 3, {
		{PNSO_HDR_FIELD_TYPE_INDATA_CHKSUM, 0, 4, 0},
		{PNSO_HDR_FIELD_TYPE_OUTDATA_LENGTH, 4, 2, 0},
		{PNSO_HDR_FIELD_TYPE_ALGO, 6, 2, 0}
	} };

	rc = pnso_register_compression_header_format(&cp_hdr_fmt,
						     PNSO_TEST_CP_HDR_FMT_IDX);
	if (rc) {
		return rc;
	}
	
	return pnso_add_compression_algo_mapping(PNSO_COMPRESSION_TYPE_LZRW1A,
						 PNSO_TEST_CP_HDR_ALGO_VER);
}

#define MAX_NUM_THREADS 128
static uint8_t thread_id_arr[MAX_NUM_THREADS];
static osal_atomic_int_t thread_done[MAX_NUM_THREADS];
static int nthreads;

static int thread_test_fn(void* arg) 
{
	int core = osal_get_coreid();
	int id = (int)((uint64_t)arg);

	thread_id_arr[nthreads++] = core;
#ifndef __KERNEL__
	OSAL_ASSERT(core == osal_get_coreid());
#endif
	if (core != osal_get_coreid())
	{
		OSAL_LOG_ERROR("Core id mismatch\n");
		return EINVAL;
	}
	osal_atomic_set(&thread_done[id], 1);
	return 0;
}

static osal_thread_t ot[MAX_NUM_THREADS];

static int osal_thread_test(void)
{
	int done = 0;
	void *arg = NULL;
	int i, rv;
	int max_threads = osal_get_core_count();

	for (i = 0; i < max_threads; i++)
	{
		arg = (void *)((uint64_t)i);
		if ((rv = osal_thread_create(&ot[i], thread_test_fn,
					     arg)) == 0) {
			if ((rv = osal_thread_bind(&ot[i], i)) == 0) {
				rv = osal_thread_start(&ot[i]);
			}
		}
		if(rv != 0)
		{
			return rv;
		}
		do
		{
			done = osal_atomic_read(&thread_done[i]);
			osal_yield();
		} while (done != 1);
	}
	for (i = 0; i < max_threads; i++)
	{
		rv = osal_thread_stop(&ot[i]);
		if(rv != 0)
		{
			return rv;
		}
#ifndef __KERNEL__
		OSAL_ASSERT(thread_id_arr[i] == (i + thread_id_arr[0]));
#endif
	}
	return 0;
}

static int body(void)
{
	size_t tid, bid, count, prev_count;
	int rv;
	struct thread_state *tstate;
	struct req_state *rstate;
	struct pnso_init_params init_params;

	/* Initialize session */
	memset(&init_params, 0, sizeof(init_params));
	init_params.per_core_qdepth = 16;
	init_params.block_size = PNSO_TEST_BLOCK_SIZE;
	if ((rv = pnso_init(&init_params)) != 0) {
		OSAL_LOG_ERROR("PNSO: pnso_init failed\n");
		return rv;
	}
	if ((rv = init_crypto()) != 0) {
		OSAL_LOG_ERROR("PNSO: init_crypto failed\n");
		goto finit;
	}
	if ((rv = init_cp_hdr_fmt()) != 0) {
		OSAL_LOG_ERROR("PNSO: init_cp_hdr_fmt failed\n");
		goto finit;
	}

        OSAL_LOG_INFO("PNSO: starting %d threads on %d core machine\n",
                 PNSO_TEST_THREAD_COUNT, osal_get_core_count());
	/* Start threads */
	for (tid = 0; tid < PNSO_TEST_THREAD_COUNT; tid++) {
		tstate = &osal_test_threads[tid];
		init_tstate(tstate);
		if ((rv = osal_thread_create(&tstate->wafl_thread,
					     exec_req, tstate)) == 0) {
			if ((rv = osal_thread_bind(&tstate->wafl_thread,
					tid % osal_get_core_count())) == 0) {
				rv = osal_thread_start(&tstate->wafl_thread);
			}
		}
		if (rv) {
			OSAL_LOG_ERROR("PNSO: FAILED to start thread %d\n",
				       (int) tid);
			goto finit;
		}
	}

	prev_count = 0;
	while (1) {
		int running_count = 0;

		count = 0;
		for (tid = 0; tid < PNSO_TEST_THREAD_COUNT; tid++) {
			tstate = &osal_test_threads[tid];
			rstate = &tstate->reqs[PNSO_TEST_BATCH_DEPTH-1];
			count += osal_atomic_read(&rstate->req_done);
			if (osal_thread_is_running(&tstate->wafl_thread)) {
				running_count++;
			}
		}

		if (prev_count != count) {
			OSAL_LOG_INFO("PNSO: new thread completion count %lu, running %d\n",
				 count, running_count);
			osal_yield();
		}

		if (count < PNSO_TEST_THREAD_COUNT*2) {
			if ((prev_count != count) && (running_count == 0)) {
				OSAL_LOG_DEBUG("PNSO: running threads exited early?\n");
				goto finit;
			}
			osal_yield();
		} else {
			break;
		}
		prev_count = count;
	}
	OSAL_LOG_INFO("IO: Completed all sim tests\n");

	osal_yield();
	count = 0;
	for (tid = 0; tid < PNSO_TEST_THREAD_COUNT; tid++) {
		tstate = &osal_test_threads[tid];
		for (bid = 0; bid < PNSO_TEST_BATCH_DEPTH; bid++) {
			rstate = &tstate->reqs[bid];
			if (memcmp((void*)rstate->buflists[0].
					buflist.buffers[0].buf,
				   (void*)rstate->buflists[3].
					buflist.buffers[0].buf,
				   PNSO_TEST_BLOCK_SIZE) == 0) {
				count++;
			}
		}
	}

	osal_yield();
	if (count == (PNSO_TEST_BATCH_DEPTH*PNSO_TEST_THREAD_COUNT)) {
		OSAL_LOG_INFO("IO: Final memcmp passed\n");
	} else {
		OSAL_LOG_ERROR("IO: Final memcmp failed\n");
		rv = EINVAL;
		goto finit;
	}

	rv = osal_thread_test();
	if(rv == 0)
	{
		OSAL_LOG_INFO("PNSO: Osal test complete\n");
	}

finit:
	osal_yield(); /* flush logs */
	pnso_sim_finit();
	return rv;
}

static int
test_init(void)
{
	return osal_log_init(OSAL_LOG_LEVEL_DEBUG);
}

static int
test_fini(void)
{
	osal_log_deinit();

	return PNSO_OK;
}

osal_init_fn_t init_fp = test_init;
osal_init_fn_t fini_fp = test_fini;

OSAL_SETUP(init_fp, body, fini_fp);
