/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __KERNEL__
#include <unistd.h>
#endif
#include "osal.h"

#include "pnso_api.h"
#include "pnso_pbuf.h"

/**
 * NOTE/TODO-sonic:
 * 	- this test utitliy may be used only until pencake arrives; thereby,
 * 	almost all the tests are used to only to verify the bring-up.  Revisit
 * 	error checks/bailout, and error reporting, etc. if this utility
 * 	needs to be used for long-term.
 * 	- thread count, block count, etc. are kept at its minimal limits for
 * 	bring up purposes.
 *
 */

/* Input data defaults */
#define PNSO_TEST_BLOCK_SIZE	4096
#define PNSO_TEST_BLOCK_COUNT	12
#define PNSO_TEST_DATA_SIZE (PNSO_TEST_BLOCK_SIZE * PNSO_TEST_BLOCK_COUNT)
#define PNSO_TEST_DESC_SIZE	64
#define PNSO_TEST_ALIGN_DESC(p)	\
	(typeof(p))(((uint64_t)(p) + PNSO_TEST_DESC_SIZE - 1) & ~((uint64_t)(PNSO_TEST_DESC_SIZE - 1)))

/* Compression defaults */
#define PNSO_TEST_CP_HDR_ALGO_VER 123
#define PNSO_TEST_CP_HDR_FMT_IDX 1

/* Encryption defaults */
#define PNSO_TEST_CRYPTO_KEY_IDX 1

unsigned char crypto_key1[32] = {
	0x19, 0xe4, 0xa3, 0x26, 0xa5, 0x0a, 0xf1, 0x29,
	0x06, 0x3c, 0x11, 0x0c, 0x7f, 0x03, 0xf9, 0x5e,
	0x19, 0xe4, 0xa3, 0x26, 0xa5, 0x0a, 0xf1, 0x29,
	0x06, 0x3c, 0x11, 0x0c, 0x7f, 0x03, 0xf9, 0x5e};
unsigned char crypto_key2[32] = {
	0x19, 0xe4, 0xa3, 0x26, 0xa5, 0x0a, 0xf1, 0x29,
	0x06, 0x3c, 0x11, 0x0c, 0x7f, 0x03, 0xf9, 0x5e,
	0x19, 0xe4, 0xa3, 0x26, 0xa5, 0x0a, 0xf1, 0x29,
	0x06, 0x3c, 0x11, 0x0c, 0x7f, 0x03, 0xf9, 0x5e};
unsigned char iv_src[16] = {
	0x19, 0xe4, 0xa3, 0x26, 0xa5, 0x0a, 0xf1, 0x29,
	0x06, 0x3c, 0x11, 0x0c, 0x7f, 0x03, 0xf9, 0x5e};
static uint64_t iv_buf_pa;

/* Thread and batching defaults */
#define PNSO_TEST_THREAD_COUNT 1
#define PNSO_TEST_BATCH_DEPTH 1
#define PNSO_TEST_SVC_COUNT 3
#define PNSO_TEST_BUF_COUNT (PNSO_TEST_BATCH_DEPTH * PNSO_TEST_SVC_COUNT)

/* Structs to avoid extra allocs */
struct pnso_multi_buflist {
	struct pnso_buffer_list *buflist;
	uint8_t data[PNSO_TEST_DATA_SIZE];	/* TODO: */
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
	osal_atomic_int_t test_done;
};

static struct thread_state osal_test_threads[PNSO_TEST_THREAD_COUNT];

int body(void);

OSAL_LICENSE("Dual BSD/GPL");

static void
comp_cb(void *arg1, struct pnso_service_result *svc_res)
{
	struct req_state *rstate = (struct req_state *) arg1;

	/* TODO: iterate through every service */
	OSAL_LOG_INFO("IO: Request(svc %u) completed, err %d, svc_count %u, core %d",
		 svc_res->svc[0].svc_type, svc_res->err, svc_res->num_services,
		 osal_get_coreid());
	if (osal_get_coreid() != rstate->tstate->wafl_thread.core_id) {
		OSAL_LOG_ERROR("IO: ERROR: worker thread running on wrong core.");
	}

	osal_atomic_fetch_add(&rstate->req_done, 1);
}

static void
init_buflist(struct pnso_multi_buflist *mbuf, char fill_byte)
{
	uint32_t i;

	mbuf->buflist = pbuf_aligned_alloc_buffer_list(PNSO_TEST_BLOCK_COUNT,
			PNSO_TEST_BLOCK_SIZE, PNSO_TEST_BLOCK_SIZE);
	for (i = 0; i < mbuf->buflist->count; i++) {
		memset((void *) mbuf->buflist->buffers[i].buf,
				fill_byte, PNSO_TEST_BLOCK_SIZE);
		 fill_byte++;
	}
	pbuf_pprint_buffer_list(mbuf->buflist);
	pbuf_convert_buffer_list_v2p(mbuf->buflist);

	/* TODO: 'bufs' not in-use */
	memset(mbuf->data, fill_byte, PNSO_TEST_DATA_SIZE);
}

static void
deinit_buflist(struct pnso_multi_buflist *mbuf)
{
	pbuf_free_buffer_list(mbuf->buflist);
}

/* Fill in service defaults */
static void
init_svc_desc(struct pnso_service *svc, uint16_t svc_type)
{
	memset(svc, 0, sizeof(*svc));

	svc->svc_type = svc_type;
	switch (svc_type) {
	case PNSO_SVC_TYPE_ENCRYPT:
	case PNSO_SVC_TYPE_DECRYPT:
		svc->u.crypto_desc.algo_type = PNSO_CRYPTO_TYPE_XTS;
		svc->u.crypto_desc.key_desc_idx = PNSO_TEST_CRYPTO_KEY_IDX;
		svc->u.crypto_desc.iv_addr = iv_buf_pa;
		break;
	case PNSO_SVC_TYPE_COMPRESS:
		svc->u.cp_desc.algo_type = PNSO_COMPRESSION_TYPE_LZRW1A;
		svc->u.cp_desc.flags = PNSO_CP_DFLAG_ZERO_PAD |
			PNSO_CP_DFLAG_INSERT_HEADER;
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

static pnso_error_t
submit_requests(struct thread_state *tstate)
{
	int err;
	size_t batch_id;
	struct req_state *rstate = NULL;
	struct pnso_service_request *svc_req = NULL;
	struct pnso_service_result *svc_res = NULL;

	for (batch_id = 0; batch_id < PNSO_TEST_BATCH_DEPTH; batch_id++) {
		rstate = &tstate->reqs[batch_id];
		svc_req = &rstate->req.req;
		svc_res = &rstate->res.res;

		err = pnso_submit_request(svc_req, svc_res, comp_cb, rstate,
					 NULL, NULL);
		if (err != 0) {
			OSAL_LOG_ERROR("pnso_submit_request(svc %u) failed with %d",
				 svc_req->svc[0].svc_type, err);
			return err;
		}
	}

	return 0;
}

static int
verify_cp_dc_result(void)
{
	int i, err = 0;
	size_t tid, bid, count;
	struct thread_state *tstate;
	struct req_state *rstate;
	bool cmp_success;

	OSAL_LOG_INFO("verify cp & dc ...");

	osal_yield();
	count = 0;
	for (tid = 0; tid < PNSO_TEST_THREAD_COUNT; tid++) {
		tstate = &osal_test_threads[tid];
		for (bid = 0; bid < PNSO_TEST_BATCH_DEPTH; bid++) {
			rstate = &tstate->reqs[bid];

			pbuf_convert_buffer_list_p2v(rstate->buflists[0].buflist);
			pbuf_convert_buffer_list_p2v(rstate->buflists[1].buflist);
			pbuf_convert_buffer_list_p2v(rstate->buflists[2].buflist);

			pbuf_pprint_buffer_list(rstate->buflists[0].buflist);
			pbuf_pprint_buffer_list(rstate->buflists[1].buflist);
			pbuf_pprint_buffer_list(rstate->buflists[2].buflist);

			cmp_success = true;
			for (i = 0; cmp_success && i < PNSO_TEST_BLOCK_COUNT; i++) {
				if (memcmp((void *)rstate->buflists[0].buflist->buffers[i].buf,
					   (void *)rstate->buflists[2].buflist->buffers[i].buf,
					   rstate->buflists[0].buflist->buffers[i].len)) {
					cmp_success = false;
					break;
				}
			}
			if (cmp_success)
				count++;
		}
	}

	osal_yield();
	if (count == (PNSO_TEST_BATCH_DEPTH*PNSO_TEST_THREAD_COUNT)) {
		OSAL_LOG_INFO("IO: Final memcmp passed");
	} else {
		OSAL_LOG_ERROR("IO: Final memcmp failed");
		err = EINVAL;
	}
	osal_atomic_fetch_add(&tstate->test_done, 1);

	return err;
}

static int
verify_hash_result(void)
{
	int err = 0;
	size_t tid, bid, count;
	struct thread_state *tstate;
	struct req_state *rstate;
	struct pnso_service_result *svc_res = NULL;

	OSAL_LOG_INFO("verify hash ...");

	osal_yield();
	count = 0;
	for (tid = 0; tid < PNSO_TEST_THREAD_COUNT; tid++) {
		tstate = &osal_test_threads[tid];
		for (bid = 0; bid < PNSO_TEST_BATCH_DEPTH; bid++) {
			rstate = &tstate->reqs[bid];

			svc_res = &rstate->res.res;
			if (svc_res->svc[0].svc_type == PNSO_SVC_TYPE_HASH) {
				if (svc_res->svc[0].u.hash.num_tags) {
					count++;
					OSAL_LOG_INFO("IO: Hash done!  num_tags: %d count: %lu",
						    svc_res->svc[0].u.hash.num_tags, count);
				}
			}

			// deinit_buflist(&rstate->buflists[0]);
		}
	}

	osal_yield();
	if (count == (PNSO_TEST_BATCH_DEPTH*PNSO_TEST_THREAD_COUNT)) {
		OSAL_LOG_INFO("IO: Final hash passed");
	} else {
		OSAL_LOG_ERROR("IO: Final hash failed");
		err = EINVAL;
	}
	osal_atomic_fetch_add(&tstate->test_done, 1);

	return err;
}

static int
verify_hash_per_block_result(void)
{
	int err = 0;
	size_t tid, bid, count;
	struct thread_state *tstate;
	struct req_state *rstate;
	struct pnso_service_result *svc_res = NULL;

	OSAL_LOG_INFO("verify hash ...");

	osal_yield();
	count = 0;
	for (tid = 0; tid < PNSO_TEST_THREAD_COUNT; tid++) {
		tstate = &osal_test_threads[tid];
		for (bid = 0; bid < PNSO_TEST_BATCH_DEPTH; bid++) {
			rstate = &tstate->reqs[bid];

			svc_res = &rstate->res.res;
			if (svc_res->svc[0].svc_type == PNSO_SVC_TYPE_HASH) {
				if (svc_res->svc[0].u.hash.num_tags >= 1) {
					count++;
					OSAL_LOG_INFO("IO: Hash done!  num_tags: %d count: %lu",
						    svc_res->svc[0].u.hash.num_tags, count);
				}
			}

			// deinit_buflist(&rstate->buflists[0]);
		}
	}

	osal_yield();
	if (count == (PNSO_TEST_BATCH_DEPTH*PNSO_TEST_THREAD_COUNT)) {
		OSAL_LOG_INFO("IO: Final hash passed");
	} else {
		OSAL_LOG_ERROR("IO: Final hash failed");
		err = EINVAL;
	}
	osal_atomic_fetch_add(&tstate->test_done, 1);

	return err;
}

static int
verify_chksum_result(void)
{
	int err = 0;
	size_t tid, bid, count;
	struct thread_state *tstate;
	struct req_state *rstate;
	struct pnso_service_result *svc_res = NULL;

	OSAL_LOG_INFO("verify checksum ...");

	osal_yield();
	count = 0;
	for (tid = 0; tid < PNSO_TEST_THREAD_COUNT; tid++) {
		tstate = &osal_test_threads[tid];
		for (bid = 0; bid < PNSO_TEST_BATCH_DEPTH; bid++) {
			rstate = &tstate->reqs[bid];

			svc_res = &rstate->res.res;
			if (svc_res->svc[0].svc_type == PNSO_SVC_TYPE_CHKSUM) {
				if (svc_res->svc[0].u.chksum.num_tags) {
					count++;
					OSAL_LOG_INFO("IO: Checksum done!  num_tags: %d count: %lu",
						    svc_res->svc[0].u.chksum.num_tags, count);
				}
			}

			// deinit_buflist(&rstate->buflists[0]);
		}
	}

	osal_yield();
	if (count == (PNSO_TEST_BATCH_DEPTH*PNSO_TEST_THREAD_COUNT)) {
		OSAL_LOG_INFO("IO: Final checksum passed");
	} else {
		OSAL_LOG_ERROR("IO: Final checksum failed");
		err = EINVAL;
	}
	osal_atomic_fetch_add(&tstate->test_done, 1);

	return err;
}

static int
verify_chksum_per_block_result(void)
{
	int err = 0;
	size_t tid, bid, count;
	struct thread_state *tstate;
	struct req_state *rstate;
	struct pnso_service_result *svc_res = NULL;

	OSAL_LOG_INFO("verify checksum ...");

	osal_yield();
	count = 0;
	for (tid = 0; tid < PNSO_TEST_THREAD_COUNT; tid++) {
		tstate = &osal_test_threads[tid];
		for (bid = 0; bid < PNSO_TEST_BATCH_DEPTH; bid++) {
			rstate = &tstate->reqs[bid];

			svc_res = &rstate->res.res;
			if (svc_res->svc[0].svc_type == PNSO_SVC_TYPE_CHKSUM) {
				if (svc_res->svc[0].u.chksum.num_tags >= 1) {
					count++;
					OSAL_LOG_INFO("IO: Checksum done!  num_tags: %d count: %lu",
						    svc_res->svc[0].u.chksum.num_tags, count);
				}
			}

			// deinit_buflist(&rstate->buflists[0]);
		}
	}

	osal_yield();
	if (count == (PNSO_TEST_BATCH_DEPTH*PNSO_TEST_THREAD_COUNT)) {
		OSAL_LOG_INFO("IO: Final checksum passed");
	} else {
		OSAL_LOG_ERROR("IO: Final checksum failed");
		err = EINVAL;
	}
	osal_atomic_fetch_add(&tstate->test_done, 1);

	return err;
}

static int
verify_cp_hash_result(void)
{
	int err = 0;
	size_t tid, bid, count;
	struct thread_state *tstate;
	struct req_state *rstate;
	struct pnso_service_request *svc_req = NULL;
	struct pnso_service_result *svc_res = NULL;
	uint32_t src_len;

	OSAL_LOG_INFO("verify CP/Hash ...");

	osal_yield();
	count = 0;
	for (tid = 0; tid < PNSO_TEST_THREAD_COUNT; tid++) {
		tstate = &osal_test_threads[tid];
		for (bid = 0; bid < PNSO_TEST_BATCH_DEPTH; bid++) {
			rstate = &tstate->reqs[bid];

			svc_res = &rstate->res.res;
			if (svc_res->err) {
				OSAL_LOG_INFO("IO: CP/Hash failed! err: %d",
						svc_res->err);
				goto out;
			}

			if (svc_res->svc[0].svc_type == PNSO_SVC_TYPE_COMPRESS) {
				svc_req = &rstate->req.req;
				src_len = pbuf_get_buffer_list_len(svc_req->sgl);
				if (svc_res->svc[0].u.dst.data_len < src_len) {
					count++;
					OSAL_LOG_INFO("IO: CP done! src_len: %d dst_len: %d",
							src_len, svc_res->svc[0].u.dst.data_len);
					goto out;
				}
			}

			if (svc_res->svc[1].svc_type == PNSO_SVC_TYPE_HASH) {
				if (svc_res->svc[1].u.hash.num_tags) {
					count++;
					OSAL_LOG_INFO("IO: Hash done!  num_tags: %d count: %lu",
						    svc_res->svc[0].u.hash.num_tags, count);
				} else
					goto out;
			}

			// deinit_buflist(&rstate->buflists[0]);
			// deinit_buflist(&rstate->buflists[1]);
		}
	}

out:
	osal_yield();
	if (count == (PNSO_TEST_BATCH_DEPTH*PNSO_TEST_THREAD_COUNT)) {
		OSAL_LOG_INFO("IO: Final CP/Hash passed");
	} else {
		OSAL_LOG_ERROR("IO: Final CP/Hash failed");
		err = EINVAL;
	}
	osal_atomic_fetch_add(&tstate->test_done, 1);

	return err;
}

static int
verify_cp_hash_per_block_result(void)
{
	int err = 0;
	size_t tid, bid, count;
	struct thread_state *tstate;
	struct req_state *rstate;
	struct pnso_service_request *svc_req = NULL;
	struct pnso_service_result *svc_res = NULL;
	uint32_t src_len;

	OSAL_LOG_INFO("verify CP/Hash (per-block) ...");

	osal_yield();
	count = 0;
	for (tid = 0; tid < PNSO_TEST_THREAD_COUNT; tid++) {
		tstate = &osal_test_threads[tid];
		for (bid = 0; bid < PNSO_TEST_BATCH_DEPTH; bid++) {
			rstate = &tstate->reqs[bid];

			svc_res = &rstate->res.res;
			if (svc_res->err) {
				OSAL_LOG_INFO("IO: CP/Hash failed! err: %d",
						svc_res->err);
				goto out;
			}

			if (svc_res->svc[0].svc_type == PNSO_SVC_TYPE_COMPRESS) {
				svc_req = &rstate->req.req;
				src_len = pbuf_get_buffer_list_len(svc_req->sgl);
				if (svc_res->svc[0].u.dst.data_len < src_len) {
					count++;
					OSAL_LOG_INFO("IO: CP done! src_len: %d dst_len: %d",
							src_len, svc_res->svc[0].u.dst.data_len);
					goto out;
				}
			}

			if (svc_res->svc[1].svc_type == PNSO_SVC_TYPE_HASH) {
				if (svc_res->svc[1].u.hash.num_tags >= 1) {
					count++;
					OSAL_LOG_INFO("IO: Hash done!  num_tags: %d count: %lu",
						    svc_res->svc[0].u.hash.num_tags, count);
				} else
					goto out;
			}

			deinit_buflist(&rstate->buflists[0]);
			deinit_buflist(&rstate->buflists[1]);
		}
	}

out:
	osal_yield();
	if (count == (PNSO_TEST_BATCH_DEPTH*PNSO_TEST_THREAD_COUNT)) {
		OSAL_LOG_INFO("IO: Final CP/Hash passed");
	} else {
		OSAL_LOG_ERROR("IO: Final CP/Hash failed");
		err = EINVAL;
	}
	osal_atomic_fetch_add(&tstate->test_done, 1);

	return err;
}

static int
verify_crypto_result(void)
{
	int i, err = 0;
	size_t tid, bid, count;
	struct thread_state *tstate;
	struct req_state *rstate;
	bool cmp_success;

	OSAL_LOG_INFO("verify crypto ...");

	osal_yield();
	count = 0;
	for (tid = 0; tid < PNSO_TEST_THREAD_COUNT; tid++) {
		tstate = &osal_test_threads[tid];
		for (bid = 0; bid < PNSO_TEST_BATCH_DEPTH; bid++) {
			rstate = &tstate->reqs[bid];

			pbuf_convert_buffer_list_p2v(rstate->buflists[0].buflist);
			pbuf_convert_buffer_list_p2v(rstate->buflists[1].buflist);
			pbuf_convert_buffer_list_p2v(rstate->buflists[2].buflist);

			pbuf_pprint_buffer_list(rstate->buflists[0].buflist);
			pbuf_pprint_buffer_list(rstate->buflists[2].buflist);

			cmp_success = true;
			for (i = 0; cmp_success && i < PNSO_TEST_BLOCK_COUNT; i++) {
				if (memcmp((void *)rstate->buflists[0].buflist->buffers[i].buf,
					   (void *)rstate->buflists[2].buflist->buffers[i].buf,
					   rstate->buflists[0].buflist->buffers[i].len)) {
					cmp_success = false;
					break;
				}
			}
			if (cmp_success)
				count++;
		}
	}

	osal_yield();
	if (count == (PNSO_TEST_BATCH_DEPTH*PNSO_TEST_THREAD_COUNT)) {
		OSAL_LOG_INFO("IO: Final memcmp passed");
	} else {
		OSAL_LOG_ERROR("IO: Final memcmp failed");
		err = EINVAL;
	}
	osal_atomic_fetch_add(&tstate->test_done, 1);

	return err;
}

static int
exec_cp_req(struct thread_state *tstate)
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

		svc_req->sgl = rstate->buflists[0].buflist;
		svc_req->num_services = 1;
		svc_res->num_services = 1;

		init_svc_desc(&svc_req->svc[0], PNSO_SVC_TYPE_COMPRESS);
		svc_res->svc[0].svc_type = PNSO_SVC_TYPE_COMPRESS;
		svc_res->svc[0].u.dst.sgl = rstate->buflists[1].buflist;
	}

	return submit_requests(tstate);
}

static void
fixup_dc_buffer(struct pnso_buffer_list *blist, size_t len)
{
	uint32_t i;

	for (i = 0; i < blist->count; i++) {
		if (len >= PNSO_TEST_BLOCK_SIZE) {
			blist->buffers[i].len = PNSO_TEST_BLOCK_SIZE;
			len -= PNSO_TEST_BLOCK_SIZE;
		} else {
			blist->buffers[i].len = len;
			len = 0;
		}
	}
}

static int
exec_dc_req(struct thread_state *tstate)
{
	size_t batch_id;
	struct req_state *rstate;
	struct pnso_service_request *svc_req = NULL;
	struct pnso_service_result *svc_res = NULL;
	uint32_t data_len;

	for (batch_id = 0; batch_id < PNSO_TEST_BATCH_DEPTH; batch_id++) {
		rstate = &tstate->reqs[batch_id];
		svc_req = &rstate->req.req;
		svc_res = &rstate->res.res;

		/* save len from previous compress result */
		data_len = svc_res->svc[0].u.dst.data_len;

		memset(svc_req, 0, sizeof(*svc_req));
		memset(svc_res, 0, sizeof(*svc_res));

		pbuf_convert_buffer_list_p2v(rstate->buflists[1].buflist);
		fixup_dc_buffer(rstate->buflists[1].buflist, data_len);
		pbuf_pprint_buffer_list(rstate->buflists[1].buflist);
		pbuf_convert_buffer_list_v2p(rstate->buflists[1].buflist);
	
		svc_req->sgl = rstate->buflists[1].buflist;
		init_buflist(&rstate->buflists[2], 'X');

		svc_req->num_services = 1;
		svc_res->num_services = 1;

		init_svc_desc(&svc_req->svc[0], PNSO_SVC_TYPE_DECOMPRESS);
		memset(&svc_res->svc[0], 0, sizeof(struct pnso_service_status));
		svc_res->svc[0].svc_type = PNSO_SVC_TYPE_DECOMPRESS;
		svc_res->svc[0].u.dst.sgl = rstate->buflists[2].buflist;
	}

	return submit_requests(tstate);
}

static int
exec_hash_req(struct thread_state *tstate)
{
	size_t batch_id;
	struct req_state *rstate;
	struct pnso_service_request *svc_req = NULL;
	struct pnso_service_result *svc_res = NULL;

	for (batch_id = 0; batch_id < PNSO_TEST_BATCH_DEPTH; batch_id++) {
		rstate = &tstate->reqs[batch_id];
		init_buflist(&rstate->buflists[0], 'A');

		svc_req = &rstate->req.req;
		memset(svc_req, 0, sizeof(*svc_req));

		svc_req->sgl = rstate->buflists[0].buflist;
		svc_req->num_services = 1;
		init_svc_desc(&svc_req->svc[0], PNSO_SVC_TYPE_HASH);
		svc_req->svc[0].u.hash_desc.flags = 0;	/* reset per block */

		svc_res = &rstate->res.res;
		memset(svc_res, 0, sizeof(*svc_res));

		svc_res->num_services = 1;
		memset(&svc_res->svc[0], 0, sizeof(struct pnso_service_status));
		svc_res->svc[0].svc_type = PNSO_SVC_TYPE_HASH;
		svc_res->svc[0].u.hash.tags = rstate->hash_tags;
	}

	return submit_requests(tstate);
}

static int
exec_hash_per_block_req(struct thread_state *tstate)
{
	size_t batch_id;
	struct req_state *rstate;
	struct pnso_service_request *svc_req = NULL;
	struct pnso_service_result *svc_res = NULL;

	for (batch_id = 0; batch_id < PNSO_TEST_BATCH_DEPTH; batch_id++) {
		rstate = &tstate->reqs[batch_id];
		init_buflist(&rstate->buflists[0], 'A');

		svc_req = &rstate->req.req;
		memset(svc_req, 0, sizeof(*svc_req));

		svc_req->sgl = rstate->buflists[0].buflist;
		svc_req->num_services = 1;
		init_svc_desc(&svc_req->svc[0], PNSO_SVC_TYPE_HASH);

		svc_res = &rstate->res.res;
		memset(svc_res, 0, sizeof(*svc_res));

		svc_res->num_services = 1;
		memset(&svc_res->svc[0], 0, sizeof(struct pnso_service_status));
		svc_res->svc[0].svc_type = PNSO_SVC_TYPE_HASH;
		svc_res->svc[0].u.hash.tags = rstate->hash_tags;
	}

	return submit_requests(tstate);
}

static int
exec_chksum_req(struct thread_state *tstate)
{
	size_t batch_id;
	struct req_state *rstate;
	struct pnso_service_request *svc_req = NULL;
	struct pnso_service_result *svc_res = NULL;

	for (batch_id = 0; batch_id < PNSO_TEST_BATCH_DEPTH; batch_id++) {
		rstate = &tstate->reqs[batch_id];
		init_buflist(&rstate->buflists[0], 'A');

		svc_req = &rstate->req.req;
		memset(svc_req, 0, sizeof(*svc_req));

		svc_req->sgl = rstate->buflists[0].buflist;
		svc_req->num_services = 1;
		init_svc_desc(&svc_req->svc[0], PNSO_SVC_TYPE_CHKSUM);
		svc_req->svc[0].u.chksum_desc.flags = 0; /* reset per block */

		svc_res = &rstate->res.res;
		memset(svc_res, 0, sizeof(*svc_res));

		svc_res->num_services = 1;
		memset(&svc_res->svc[0], 0, sizeof(struct pnso_service_status));
		svc_res->svc[0].svc_type = PNSO_SVC_TYPE_CHKSUM;
		svc_res->svc[0].u.chksum.tags = rstate->chksum_tag;
	}

	return submit_requests(tstate);
}

static int
exec_chksum_per_block_req(struct thread_state *tstate)
{
	size_t batch_id;
	struct req_state *rstate;
	struct pnso_service_request *svc_req = NULL;
	struct pnso_service_result *svc_res = NULL;

	for (batch_id = 0; batch_id < PNSO_TEST_BATCH_DEPTH; batch_id++) {
		rstate = &tstate->reqs[batch_id];
		init_buflist(&rstate->buflists[0], 'A');

		svc_req = &rstate->req.req;
		memset(svc_req, 0, sizeof(*svc_req));

		svc_req->sgl = rstate->buflists[0].buflist;
		svc_req->num_services = 1;
		init_svc_desc(&svc_req->svc[0], PNSO_SVC_TYPE_CHKSUM);

		svc_res = &rstate->res.res;
		memset(svc_res, 0, sizeof(*svc_res));

		svc_res->num_services = 1;
		memset(&svc_res->svc[0], 0, sizeof(struct pnso_service_status));
		svc_res->svc[0].svc_type = PNSO_SVC_TYPE_CHKSUM;
		svc_res->svc[0].u.chksum.tags = rstate->chksum_tag;
	}

	return submit_requests(tstate);
}

static int
exec_cp_hash_req(struct thread_state *tstate)
{
	size_t batch_id;
	struct req_state *rstate;
	struct pnso_service_request *svc_req = NULL;
	struct pnso_service_result *svc_res = NULL;

	for (batch_id = 0; batch_id < PNSO_TEST_BATCH_DEPTH; batch_id++) {
		rstate = &tstate->reqs[batch_id];

		init_buflist(&rstate->buflists[0], 'A');
		init_buflist(&rstate->buflists[1], 'B');

		svc_req = &rstate->req.req;
		memset(svc_req, 0, sizeof(*svc_req));
	
		svc_res = &rstate->res.res;
		memset(svc_res, 0, sizeof(*svc_res));

		svc_req->num_services = 2;
		svc_res->num_services = 2;

		svc_req->sgl = rstate->buflists[0].buflist;

		init_svc_desc(&svc_req->svc[0], PNSO_SVC_TYPE_COMPRESS);
		memset(&svc_res->svc[0], 0, sizeof(struct pnso_service_status));
		svc_res->svc[0].svc_type = PNSO_SVC_TYPE_COMPRESS;
		svc_res->svc[0].u.dst.sgl = rstate->buflists[1].buflist;

		init_svc_desc(&svc_req->svc[1], PNSO_SVC_TYPE_HASH);
		svc_req->svc[1].u.hash_desc.flags = 0;	/* reset per block */
		memset(&svc_res->svc[1], 0, sizeof(struct pnso_service_status));
		svc_res->svc[1].svc_type = PNSO_SVC_TYPE_HASH;
		svc_res->svc[1].u.hash.tags = rstate->hash_tags;
	}

	return submit_requests(tstate);
}

static int
exec_cp_hash_per_block_req(struct thread_state *tstate)
{
	size_t batch_id;
	struct req_state *rstate;
	struct pnso_service_request *svc_req = NULL;
	struct pnso_service_result *svc_res = NULL;

	for (batch_id = 0; batch_id < PNSO_TEST_BATCH_DEPTH; batch_id++) {
		rstate = &tstate->reqs[batch_id];

		init_buflist(&rstate->buflists[0], 'A');
		init_buflist(&rstate->buflists[1], 'B');

		svc_req = &rstate->req.req;
		memset(svc_req, 0, sizeof(*svc_req));

		svc_res = &rstate->res.res;
		memset(svc_res, 0, sizeof(*svc_res));

		svc_req->num_services = 2;
		svc_res->num_services = 2;

		svc_req->sgl = rstate->buflists[0].buflist;

		init_svc_desc(&svc_req->svc[0], PNSO_SVC_TYPE_COMPRESS);
		memset(&svc_res->svc[0], 0, sizeof(struct pnso_service_status));
		svc_res->svc[0].svc_type = PNSO_SVC_TYPE_COMPRESS;
		svc_res->svc[0].u.dst.sgl = rstate->buflists[1].buflist;

		init_svc_desc(&svc_req->svc[1], PNSO_SVC_TYPE_HASH);
		memset(&svc_res->svc[1], 0, sizeof(struct pnso_service_status));
		svc_res->svc[1].svc_type = PNSO_SVC_TYPE_HASH;
		svc_res->svc[1].u.hash.tags = rstate->hash_tags;
	}

	return submit_requests(tstate);
}

static int
exec_encrypt_req(struct thread_state *tstate)
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

		svc_req->sgl = rstate->buflists[0].buflist;
		svc_req->num_services = 1;
		svc_res->num_services = 1;

		init_svc_desc(&svc_req->svc[0], PNSO_SVC_TYPE_ENCRYPT);
		svc_res->svc[0].svc_type = PNSO_SVC_TYPE_ENCRYPT;
		svc_res->svc[0].u.dst.sgl = rstate->buflists[1].buflist;
	}

	return submit_requests(tstate);
}

static int
exec_decrypt_req(struct thread_state *tstate)
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

		svc_req->sgl = rstate->buflists[1].buflist;
		init_buflist(&rstate->buflists[2], 'X');

		svc_req->num_services = 1;
		svc_res->num_services = 1;

		init_svc_desc(&svc_req->svc[0], PNSO_SVC_TYPE_DECRYPT);
		memset(&svc_res->svc[0], 0, sizeof(struct pnso_service_status));
		svc_res->svc[0].svc_type = PNSO_SVC_TYPE_DECRYPT;
		svc_res->svc[0].u.dst.sgl = rstate->buflists[2].buflist;
	}

	return submit_requests(tstate);
}

static void
init_tstate(struct thread_state *tstate)
{
	size_t i;

	memset(tstate, 0, sizeof(*tstate));
	for (i = 0; i < PNSO_TEST_BATCH_DEPTH; i++) {
		osal_atomic_init(&tstate->reqs[i].req_done, 0);
		osal_atomic_init(&tstate->test_done, 0);
		tstate->reqs[i].tstate = tstate;
	}
}

static void
exec_req_and_wait(struct thread_state *tstate)
{
	int req_done;
	osal_atomic_int_t *addr;

	addr = &tstate->reqs[PNSO_TEST_BATCH_DEPTH-1].req_done;
	while (1) {
		req_done = osal_atomic_read(addr);
		if (req_done)
			break;

		osal_yield();
	}
}

static int __attribute__((unused))
exec_cp_dc_test(void *arg)
{
	int err;
	struct thread_state *tstate = (struct thread_state *) arg;
	int local_core_id = osal_get_coreid();

	OSAL_LOG_INFO("PNSO: starting worker thread on core %d",
		 osal_get_coreid());

	/* Submit compression requests */
	err = exec_cp_req(tstate);
	if (err != 0) {
		OSAL_LOG_ERROR("PNSO: Compression request submit FAILED");
		goto error;
	}

	exec_req_and_wait(tstate);
	OSAL_LOG_INFO("PNSO: Compression requests done, core %d",
		 osal_get_coreid());

	osal_yield();
	if (osal_get_coreid() != local_core_id) {
		OSAL_LOG_ERROR("PNSO: ERROR core id changed unexpectedly");
		err = EINVAL;
		goto error;
	}

	/* TODO: two requests involved, so decrement to keep the count intact */
	osal_atomic_fetch_sub(
			&tstate->reqs[PNSO_TEST_BATCH_DEPTH-1].req_done, 1);

	/* Submit decompression requests */
	err = exec_dc_req(tstate);
	if (err != 0) {
		OSAL_LOG_ERROR("PNSO: Decompression request submit FAILED");
		goto error;
	}

	exec_req_and_wait(tstate);
	OSAL_LOG_INFO("PNSO: Decompression requests done, core %d",
		 osal_get_coreid());

	osal_yield();
	if (osal_get_coreid() != local_core_id) {
		OSAL_LOG_ERROR("PNSO: ERROR core id changed unexpectedly");
		err = EINVAL;
		goto error;
	}

	err = verify_cp_dc_result();
	if (err)
		goto error;

	OSAL_LOG_INFO("PNSO: Worker thread finished, core %d",
			osal_get_coreid());
	return 0;

error:
	OSAL_LOG_ERROR("PNSO: Worker thread failed, core %d or %d",
		       local_core_id, osal_get_coreid());
	return err;
}

static int __attribute__((unused))
exec_hash_test(void *arg)
{
	int err;
	struct thread_state *tstate = (struct thread_state *) arg;
	int local_core_id = osal_get_coreid();

	OSAL_LOG_INFO("PNSO: starting worker thread on core %d",
		 osal_get_coreid());

	err = exec_hash_req(tstate);
	if (err != 0) {
		OSAL_LOG_ERROR("PNSO: Hash request submit FAILED");
		goto error;
	}

	exec_req_and_wait(tstate);
	OSAL_LOG_INFO("PNSO: Hash requests done, core %d",
		 osal_get_coreid());

	osal_yield();
	if (osal_get_coreid() != local_core_id) {
		OSAL_LOG_ERROR("PNSO: ERROR core id changed unexpectedly");
		err = EINVAL;
		goto error;
	}

	err = verify_hash_result();
	if (err)
		goto error;

	OSAL_LOG_INFO("PNSO: Worker thread finished, core %d", osal_get_coreid());
	return 0;

error:
	OSAL_LOG_ERROR("PNSO: Worker thread failed, core %d or %d",
		       local_core_id, osal_get_coreid());
	return err;
}

static int __attribute__((unused))
exec_hash_per_block_test(void *arg)
{
	int err;
	struct thread_state *tstate = (struct thread_state *) arg;
	int local_core_id = osal_get_coreid();

	OSAL_LOG_INFO("PNSO: starting worker thread on core %d",
		 osal_get_coreid());

	err = exec_hash_per_block_req(tstate);
	if (err != 0) {
		OSAL_LOG_ERROR("PNSO: Hash request submit FAILED");
		goto error;
	}

	exec_req_and_wait(tstate);
	OSAL_LOG_INFO("PNSO: Hash requests done, core %d",
		 osal_get_coreid());

	osal_yield();
	if (osal_get_coreid() != local_core_id) {
		OSAL_LOG_ERROR("PNSO: ERROR core id changed unexpectedly");
		err = EINVAL;
		goto error;
	}

	err = verify_hash_per_block_result();
	if (err)
		goto error;

	OSAL_LOG_INFO("PNSO: Worker thread finished, core %d", osal_get_coreid());
	return 0;

error:
	OSAL_LOG_ERROR("PNSO: Worker thread failed, core %d or %d",
		       local_core_id, osal_get_coreid());
	return err;
}

static int __attribute__((unused))
exec_chksum_test(void *arg)
{
	int err;
	struct thread_state *tstate = (struct thread_state *) arg;
	int local_core_id = osal_get_coreid();

	OSAL_LOG_INFO("PNSO: starting worker thread on core %d",
		 osal_get_coreid());

	err = exec_chksum_req(tstate);
	if (err != 0) {
		OSAL_LOG_ERROR("PNSO: Checksum request submit FAILED");
		goto error;
	}

	exec_req_and_wait(tstate);
	OSAL_LOG_INFO("PNSO: Checksum requests done, core %d",
		 osal_get_coreid());

	osal_yield();
	if (osal_get_coreid() != local_core_id) {
		OSAL_LOG_ERROR("PNSO: ERROR core id changed unexpectedly");
		err = EINVAL;
		goto error;
	}

	err = verify_chksum_result();
	if (err)
		goto error;

	OSAL_LOG_INFO("PNSO: Worker thread finished, core %d",
			osal_get_coreid());
	return 0;

error:
	OSAL_LOG_ERROR("PNSO: Worker thread failed, core %d or %d",
		       local_core_id, osal_get_coreid());
	return err;
}

static int __attribute__((unused))
exec_chksum_per_block_test(void *arg)
{
	int err;
	struct thread_state *tstate = (struct thread_state *) arg;
	int local_core_id = osal_get_coreid();

	OSAL_LOG_INFO("PNSO: starting worker thread on core %d",
		 osal_get_coreid());

	err = exec_chksum_per_block_req(tstate);
	if (err != 0) {
		OSAL_LOG_ERROR("PNSO: Checksum request submit FAILED");
		goto error;
	}

	exec_req_and_wait(tstate);
	OSAL_LOG_INFO("PNSO: Checksum requests done, core %d",
		 osal_get_coreid());

	osal_yield();
	if (osal_get_coreid() != local_core_id) {
		OSAL_LOG_ERROR("PNSO: ERROR core id changed unexpectedly");
		err = EINVAL;
		goto error;
	}

	err = verify_chksum_per_block_result();
	if (err)
		goto error;

	OSAL_LOG_INFO("PNSO: Worker thread finished, core %d",
			osal_get_coreid());
	return 0;

error:
	OSAL_LOG_ERROR("PNSO: Worker thread failed, core %d or %d",
		       local_core_id, osal_get_coreid());
	return err;
}
static int __attribute__((unused))
exec_cp_hash_test(void *arg)
{
	int err;
	struct thread_state *tstate = (struct thread_state *) arg;
	int local_core_id = osal_get_coreid();

	OSAL_LOG_INFO("PNSO: starting worker thread on core %d",
		 osal_get_coreid());

	err = exec_cp_hash_req(tstate);
	if (err != 0) {
		OSAL_LOG_ERROR("PNSO: CP/Hash request submit FAILED");
		goto error;
	}

	exec_req_and_wait(tstate);
	OSAL_LOG_INFO("PNSO: CP/Hash request done, core %d",
		 osal_get_coreid());

	osal_yield();
	if (osal_get_coreid() != local_core_id) {
		OSAL_LOG_ERROR("PNSO: ERROR core id changed unexpectedly");
		err = EINVAL;
		goto error;
	}

	err = verify_cp_hash_result();
	if (err)
		goto error;

	OSAL_LOG_INFO("PNSO: Worker thread finished, core %d",
			osal_get_coreid());
	return 0;

error:
	OSAL_LOG_ERROR("PNSO: Worker thread failed, core %d or %d",
		       local_core_id, osal_get_coreid());
	return err;
}

static int __attribute__((unused))
exec_cp_hash_per_block_test(void *arg)
{
	int err;
	struct thread_state *tstate = (struct thread_state *) arg;
	int local_core_id = osal_get_coreid();

	OSAL_LOG_INFO("PNSO: starting worker thread on core %d",
		 osal_get_coreid());

	err = exec_cp_hash_per_block_req(tstate);
	if (err != 0) {
		OSAL_LOG_ERROR("PNSO: CP/Hash request submit FAILED");
		goto error;
	}

	exec_req_and_wait(tstate);
	OSAL_LOG_INFO("PNSO: CP/Hash request done, core %d",
		 osal_get_coreid());

	osal_yield();
	if (osal_get_coreid() != local_core_id) {
		OSAL_LOG_ERROR("PNSO: ERROR core id changed unexpectedly");
		err = EINVAL;
		goto error;
	}

	err = verify_cp_hash_per_block_result();
	if (err)
		goto error;

	OSAL_LOG_INFO("PNSO: Worker thread finished, core %d",
			osal_get_coreid());
	return 0;

error:
	OSAL_LOG_ERROR("PNSO: Worker thread failed, core %d or %d",
		       local_core_id, osal_get_coreid());
	return err;
}

static int __attribute__((unused))
exec_crypto_test(void *arg)
{
	int err;
	struct thread_state *tstate = (struct thread_state *) arg;
	int local_core_id = osal_get_coreid();

	OSAL_LOG_INFO("PNSO: starting worker thread on core %d",
		 osal_get_coreid());

	/* Submit encryption requests */
	err = exec_encrypt_req(tstate);
	if (err != 0) {
		OSAL_LOG_ERROR("PNSO: encryption request submit FAILED");
		goto error;
	}

	exec_req_and_wait(tstate);
	OSAL_LOG_INFO("PNSO: encryption requests done, core %d",
		 osal_get_coreid());

	osal_yield();
	if (osal_get_coreid() != local_core_id) {
		OSAL_LOG_ERROR("PNSO: ERROR core id changed unexpectedly");
		err = EINVAL;
		goto error;
	}

	/* TODO: two requests involved, so decrement to keep the count intact */
	osal_atomic_fetch_sub(
			&tstate->reqs[PNSO_TEST_BATCH_DEPTH-1].req_done, 1);

	/* Submit decryption requests */
	err = exec_decrypt_req(tstate);
	if (err != 0) {
		OSAL_LOG_ERROR("PNSO: decryption request submit FAILED");
		goto error;
	}

	exec_req_and_wait(tstate);
	OSAL_LOG_INFO("PNSO: decryption requests done, core %d",
		 osal_get_coreid());

	osal_yield();
	err = verify_crypto_result();
	if (err)
		goto error;

	OSAL_LOG_INFO("PNSO: Worker thread finished, core %d",
			osal_get_coreid());
	return 0;

error:
	OSAL_LOG_ERROR("PNSO: Worker thread failed, core %d or %d",
		       local_core_id, osal_get_coreid());
	return err;
}

static pnso_error_t
init_crypto(void)
{
	uint8_t *iv_buf;
	pnso_error_t err;

	/*
	 * Temp workaround for osal_aligned_alloc not correctly aligning iv_buf
	 * (which is no more than 16 bytes long) on a 64-byte boundary.
	 */
	iv_buf = osal_aligned_alloc(PNSO_TEST_BLOCK_SIZE, PNSO_TEST_BLOCK_SIZE);
	memcpy(PNSO_TEST_ALIGN_DESC(iv_buf), iv_src, sizeof(iv_src));
	iv_buf_pa = PNSO_TEST_ALIGN_DESC(osal_virt_to_phy(iv_buf));
	OSAL_LOG_INFO("crypto iv_buf 0x%llx iv_buf_pa 0x%llx",
		      (uint64_t)iv_buf, iv_buf_pa);
	if ((err = pnso_set_key_desc_idx(crypto_key1, crypto_key2,
					 sizeof(crypto_key1),
					 PNSO_TEST_CRYPTO_KEY_IDX)) != 0) {
		OSAL_LOG_ERROR("pnso_set_key_desc_idx failed");
		return err;
	}
	return PNSO_OK;
}

static pnso_error_t
init_cp_hdr_fmt(void)
{
	pnso_error_t err;
	struct pnso_compression_header_format cp_hdr_fmt = { 3, {
		{PNSO_HDR_FIELD_TYPE_INDATA_CHKSUM, 0, 4, 0},
		{PNSO_HDR_FIELD_TYPE_OUTDATA_LENGTH, 4, 2, 0},
		{PNSO_HDR_FIELD_TYPE_ALGO, 6, 2, 0}
	} };

	err = pnso_register_compression_header_format(&cp_hdr_fmt,
						     PNSO_TEST_CP_HDR_FMT_IDX);
	if (err)
		return err;

	return pnso_add_compression_algo_mapping(PNSO_COMPRESSION_TYPE_LZRW1A,
						 PNSO_TEST_CP_HDR_ALGO_VER);
}

typedef int (*exec_test_fn_t)(void *arg);

static exec_test_fn_t exec_test_fn[] = {
	// exec_cp_dc_test,
	// exec_hash_test,
	// exec_hash_per_block_test,
	// exec_chksum_test,
	// exec_chksum_per_block_test,
	// exec_cp_hash_test,
	// exec_cp_hash_per_block_test,
	exec_crypto_test,
};

static int
exec_test(struct thread_state *tstate, size_t tid, uint32_t test_id)
{
	int err;
	int test_done;
	osal_atomic_int_t *addr;
	exec_test_fn_t test_fn = exec_test_fn[test_id];

	err = osal_thread_create(&tstate->wafl_thread, test_fn, tstate);
	if (err)
		return err;
	
	err = osal_thread_bind(&tstate->wafl_thread,
			tid % osal_get_core_count());
	if (err)
		return err;

	err = osal_thread_start(&tstate->wafl_thread);
	if (err)
		return err;

	addr = &tstate->test_done;
	while (1) {
		test_done = osal_atomic_read(addr);
		if (test_done == 1) {
			osal_atomic_fetch_sub(&tstate->test_done, 1);
			break;
		}

		osal_yield();
	}

	return err;
}

int __attribute__((unused))
body(void)
{
	size_t tid, count, prev_count;
	int err, test_id, num_tests;
	struct thread_state *tstate;
	struct req_state *rstate;
	struct pnso_init_params init_params;

	/* Initialize session */
	memset(&init_params, 0, sizeof(init_params));
	init_params.per_core_qdepth = 16;
	init_params.block_size = PNSO_TEST_BLOCK_SIZE;

	if ((err = init_crypto()) != 0) {
		OSAL_LOG_ERROR("PNSO: init_crypto failed");
		goto finit;
	}

	if ((err = init_cp_hdr_fmt()) != 0) {
		OSAL_LOG_ERROR("PNSO: init_cp_hdr_fmt failed");
		goto finit;
	}

	OSAL_LOG_INFO("PNSO: starting %d threads on %d core machine",
		 PNSO_TEST_THREAD_COUNT, osal_get_core_count());

	num_tests = sizeof(exec_test_fn) / sizeof(exec_test_fn[0]);

	/* Start threads */
	for (tid = 0; tid < PNSO_TEST_THREAD_COUNT; tid++) {
		tstate = &osal_test_threads[tid];
		init_tstate(tstate);
	
		for (test_id = 0 ; test_id < num_tests; test_id++) {
			err = exec_test(tstate, tid, test_id);
			if (err) {
				OSAL_LOG_ERROR("PNSO: FAILED to start thread %d",
						(int) tid);
				goto finit;
			}
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
			if (osal_thread_is_running(&tstate->wafl_thread))
				running_count++;
		}

		if (prev_count != count) {
			OSAL_LOG_INFO("PNSO: new thread completion count %lu, running %d",
				 count, running_count);
			osal_yield();
		}

		if (count < PNSO_TEST_THREAD_COUNT*num_tests) {
			if ((prev_count != count) && (running_count == 0)) {
				OSAL_LOG_DEBUG("PNSO: running threads exited early?");
				goto finit;
			}
			osal_yield();
		} else {
			break;
		}
		prev_count = count;
	}
	OSAL_LOG_INFO("IO: Completed all tests");

finit:
	OSAL_LOG_INFO("PNSO: Osal test complete!! err:%d", err);
	osal_yield(); /* flush logs */
	return err;
}

static int
test_init(void)
{
	int err;

#ifndef __KERNEL__
	err = osal_log_init(OSAL_LOG_LEVEL_DEBUG);
#else
	err = osal_log_init(OSAL_LOG_LEVEL_NONE);
#endif

	return err;
}

static int
test_fini(void)
{
	osal_log_deinit();

	return PNSO_OK;
}

osal_init_fn_t init_fp = test_init;
osal_init_fn_t fini_fp = test_fini;

#ifndef ENABLE_PNSO_SONIC_TEST
OSAL_SETUP(init_fp, body, fini_fp);
#endif
