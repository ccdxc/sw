/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef __PNSO_TEST_CTX_H__
#define __PNSO_TEST_CTX_H__

#include "pnso_test_desc.h"

#ifdef __cplusplus
extern "C"
{
#endif

enum {
	TEST_VAR_ITER,
	TEST_VAR_ID,
	TEST_VAR_CHAIN,
	TEST_VAR_BLOCK_SIZE,

	/* Must be last */
	TEST_VAR_MAX
};

#define MAX_BUF_LEN (2 * 1024 * 1024)
#define DEFAULT_BUF_COUNT 16
#define DEFAULT_BLOCK_SIZE 4096
#define MAX_INPUT_BUF_COUNT 1024
#define MAX_OUTPUT_BUF_COUNT (MAX_BUF_LEN / 4096)

static inline uint32_t get_max_output_len_by_type(uint16_t svc_type,
						  uint32_t output_flags)
{
	switch (svc_type) {
	case PNSO_SVC_TYPE_ENCRYPT:
	case PNSO_SVC_TYPE_DECRYPT:
		if (output_flags & TEST_OUTPUT_FLAG_TINY) {
			return DEFAULT_BLOCK_SIZE;
		} else {
			return 64 * 1024;
		}
	case PNSO_SVC_TYPE_COMPRESS:
		if (output_flags & TEST_OUTPUT_FLAG_TINY) {
			return DEFAULT_BLOCK_SIZE;
		} else {
			return MAX_BUF_LEN / 8;
		}
	case PNSO_SVC_TYPE_DECOMPRESS:
		if (output_flags & TEST_OUTPUT_FLAG_TINY) {
			return DEFAULT_BLOCK_SIZE;
		} else {
			return MAX_BUF_LEN;
		}
	case PNSO_SVC_TYPE_HASH:
		if (output_flags & TEST_OUTPUT_FLAG_TINY) {
			return PNSO_HASH_TAG_LEN;
		} else {
			return MAX_OUTPUT_BUF_COUNT * PNSO_HASH_TAG_LEN;
		}
	case PNSO_SVC_TYPE_CHKSUM:
		if (output_flags & TEST_OUTPUT_FLAG_TINY) {
			return PNSO_CHKSUM_TAG_LEN;
		} else {
			return MAX_OUTPUT_BUF_COUNT * PNSO_CHKSUM_TAG_LEN;
		}
	case PNSO_SVC_TYPE_DECOMPACT:
	default:
		if (output_flags & TEST_OUTPUT_FLAG_TINY) {
			return PNSO_CHKSUM_TAG_LEN;
		} else {
			return 64 * 1024;
		}
	}
}

struct request_context {
	struct batch_context *batch_ctx;
	const struct test_svc_chain *svc_chain;

	uint8_t *input_buffer;

	/* MUST keep these 2 in order, due to zero-length array */
	struct pnso_service_request svc_req;
	struct pnso_service req_svcs[PNSO_SVC_TYPE_MAX];

	/* MUST keep these 2 in order, due to zero-length array */
	struct pnso_service_result svc_res;
	struct pnso_service_status res_statuses[PNSO_SVC_TYPE_MAX];

	/* MUST keep these 2 in order, due to zero-length array */
	struct pnso_buffer_list src_buflist;
	struct pnso_flat_buffer src_bufs[MAX_INPUT_BUF_COUNT];

	/* Initial values inherited from parent */
	uint32_t vars[TEST_VAR_MAX];

	pnso_error_t res_rc;
};

struct testcase_io_stats {
	uint64_t svcs;
	uint64_t reqs;
	uint64_t batches;
	uint64_t bytes;
	uint64_t failures;
};

struct testcase_aggregate_stats {
	/* calculated only in aggregate */
	uint64_t total_latency;
	uint64_t avg_latency;
	uint64_t bytes_per_sec;
	uint64_t svcs_per_sec;
	uint64_t reqs_per_sec;
	uint64_t batches_per_sec;
};

#define TESTCASE_IO_STATS_COUNT (sizeof(struct testcase_io_stats)/sizeof(uint64_t))
#define TESTCASE_AGG_STATS_COUNT (sizeof(struct testcase_aggregate_stats)/sizeof(uint64_t))
#define TESTCASE_STATS_COUNT ((2*TESTCASE_IO_STATS_COUNT) + TESTCASE_AGG_STATS_COUNT + 1)

struct testcase_stats {
	uint64_t elapsed_time;
	struct testcase_aggregate_stats agg_stats;
	struct testcase_io_stats io_stats[2];
};

enum {
	BATCH_STATE_INIT,
	BATCH_STATE_PENDING,
	BATCH_STATE_INPROGRESS,
	BATCH_STATE_DONE,
	BATCH_STATE_INVALID
};

struct batch_context {
	const struct test_desc *desc;
	struct testcase_context *test_ctx;
	uint32_t batch_id;
	uint32_t worker_id;
	osal_atomic_int_t batch_state;
	osal_atomic_int_t cb_count;

	pnso_poll_fn_t poll_fn;
	void *poll_ctx;

	uint64_t start_time;
	struct testcase_stats stats;

	/* Initial values inherited from parent */
	uint32_t vars[TEST_VAR_MAX];

	uint32_t req_count;
	struct request_context *req_ctxs[TEST_MAX_BATCH_DEPTH];
};

/* Assumes one producer, one consumer */
struct worker_queue {
	osal_atomic_int_t lock;
	uint32_t count;
	uint32_t head;
	uint32_t tail;
	uint32_t max_count;
	struct batch_context *batch_ctxs[0];
};

static inline bool worker_queue_is_full(struct worker_queue *q)
{
	bool rc;

	osal_atomic_lock(&q->lock);
	rc = (q->count >= q->max_count);
	osal_atomic_unlock(&q->lock);

	return rc;
}

static inline bool worker_queue_is_empty(struct worker_queue *q)
{
	bool rc;

	osal_atomic_lock(&q->lock);
	rc = (q->count == 0);
	osal_atomic_unlock(&q->lock);

	return rc;
}

static inline struct batch_context *worker_queue_dequeue(struct worker_queue *q)
{
	struct batch_context *batch = NULL;

	osal_atomic_lock(&q->lock);
	if (q->count) {
		batch = q->batch_ctxs[q->tail++];
		if (q->tail >= q->max_count) {
			q->tail = 0;
		}
		q->count--;
	}
	osal_atomic_unlock(&q->lock);
	return batch;
}

static inline bool worker_queue_enqueue(struct worker_queue *q, struct batch_context *batch)
{
	bool rc = false;

	osal_atomic_lock(&q->lock);
	if (q->count < q->max_count) {
		q->batch_ctxs[q->head++] = batch;
		if (q->head >= q->max_count) {
			q->head = 0;
		}
		q->count++;
		rc = true;
	}
	osal_atomic_unlock(&q->lock);

	return rc;
}

struct worker_context {
	const struct test_desc *desc;
	const struct testcase_context *test_ctx;
	osal_thread_t worker_thread;
	uint32_t worker_id;

	struct worker_queue *submit_q;
	struct worker_queue *complete_q;
	struct worker_queue *poll_q;
};

struct testcase_context {
	const struct test_desc *desc;
	const struct test_testcase *testcase;
	struct test_file_table *output_file_tbl;

	uint64_t start_time;
	osal_atomic_int_t stats_lock;
	struct testcase_stats stats;

	uint32_t worker_count;
	struct worker_context *worker_ctxs[TEST_MAX_CORE_COUNT];

	/*
	 * testcase_context owns these batch contexts,
	 * but they are distributed evenly between the worker_contexts
	 */
	struct batch_context batch_ctx_pool[TEST_MAX_BATCH_COUNT_TOTAL];
	struct worker_queue *batch_ctx_freelist;

	uint32_t vars[TEST_VAR_MAX];
};

struct test_file_table {
	bool initialized;
	osal_atomic_int_t lookup_lock;
	osal_atomic_int_t write_lock;
	struct test_node_table table;
};

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __PNSO_TEST_CTX_H__ */
