/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#ifndef __PNSO_TEST_CTX_H__
#define __PNSO_TEST_CTX_H__

#include "pnso_test_desc.h"
#include "osal_atomic.h"
#include "osal_thread.h"

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

#define MAX_INPUT_BUF_LEN (2 * 1024 * 1024)
#define MAX_OUTPUT_BUF_LEN (64 * 1024)
#define DEFAULT_BUF_COUNT 16
#define DEFAULT_BLOCK_SIZE 4096
#define MAX_INPUT_BUF_COUNT 1024
#define MAX_OUTPUT_BUF_COUNT (MAX_OUTPUT_BUF_LEN / 4096)
#define MAX_COMPRESSION_FACTOR 10

static inline uint32_t get_max_output_len_by_type(uint16_t svc_type,
						  uint32_t output_flags,
						  uint32_t input_len)
{
	switch (svc_type) {
	case PNSO_SVC_TYPE_ENCRYPT:
	case PNSO_SVC_TYPE_DECRYPT:
		if (output_flags & TEST_OUTPUT_FLAG_TINY) {
			return DEFAULT_BLOCK_SIZE;
		} else if (output_flags & TEST_OUTPUT_FLAG_JUMBO) {
			return MAX_OUTPUT_BUF_LEN;
		} else {
			return input_len;
		}
	case PNSO_SVC_TYPE_COMPRESS:
		if (output_flags & TEST_OUTPUT_FLAG_TINY) {
			return DEFAULT_BLOCK_SIZE;
		} else if (output_flags & TEST_OUTPUT_FLAG_JUMBO) {
			return MAX_OUTPUT_BUF_LEN;
		} else {
			return input_len;
		}
	case PNSO_SVC_TYPE_DECOMPRESS:
		if (output_flags & TEST_OUTPUT_FLAG_TINY) {
			return DEFAULT_BLOCK_SIZE;
		} else if ((output_flags & TEST_OUTPUT_FLAG_JUMBO) ||
			   (input_len * MAX_COMPRESSION_FACTOR > MAX_OUTPUT_BUF_LEN)) {
			return MAX_OUTPUT_BUF_LEN;
		} else {
			return input_len * MAX_COMPRESSION_FACTOR;
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
			return MAX_OUTPUT_BUF_LEN;
		}
	}
}

struct input_buffer_context {
	bool initialized;
	uint32_t svc_chain_idx;
	uint32_t alloc_sz;
	uint32_t len;
	uint8_t *buf;
};

struct request_context {
	struct batch_context *batch_ctx;
	const struct test_svc_chain *svc_chain;

	struct input_buffer_context input;

	bool is_req_sgl_pa; /* physical addr or not */
	bool is_res_sgl_pa; /* physical addr or not */

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

	pnso_error_t req_rc;
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
	uint64_t min_latency;
	uint64_t max_latency;
	uint64_t in_bytes_per_sec;
	uint64_t out_bytes_per_sec;
	uint64_t max_in_bytes_per_sec;
	uint64_t max_out_bytes_per_sec;
	uint64_t svcs_per_sec;
	uint64_t reqs_per_sec;
	uint64_t batches_per_sec;
	uint64_t validation_successes;
	uint64_t validation_failures;
};

#define TESTCASE_IO_STATS_COUNT (sizeof(struct testcase_io_stats)/sizeof(uint64_t))
#define TESTCASE_AGG_STATS_COUNT (sizeof(struct testcase_aggregate_stats)/sizeof(uint64_t))
#define TESTCASE_STATS_COUNT ((2*TESTCASE_IO_STATS_COUNT) + TESTCASE_AGG_STATS_COUNT + 1)

struct testcase_stats {
	uint64_t elapsed_time;
	struct testcase_aggregate_stats agg_stats;
	struct testcase_io_stats io_stats[2];
};

struct batch_context {
	const struct test_desc *desc;
	struct testcase_context *test_ctx;
	uint32_t batch_id;
	uint32_t worker_id;
	osal_atomic_int_t cb_count;
	pnso_error_t req_rc;

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
	//osal_atomic_int_t lock;
	//uint32_t count;
	osal_atomic_int_t atomic_count;
	uint32_t head;
	uint32_t tail;
	uint32_t max_count;
	struct batch_context *batch_ctxs[0];
};

static inline bool worker_queue_is_full(struct worker_queue *q)
{
	return osal_atomic_read(&q->atomic_count) >= q->max_count;
}

static inline bool worker_queue_is_empty(struct worker_queue *q)
{
	return osal_atomic_read(&q->atomic_count) <= 0;
}

static inline struct batch_context *_worker_queue_dequeue(struct worker_queue *q)
{
	struct batch_context *batch;

	batch = q->batch_ctxs[q->tail % q->max_count];
	q->tail++;
	osal_atomic_fetch_sub(&q->atomic_count, 1);
	return batch;
}

static inline struct batch_context *worker_queue_dequeue(struct worker_queue *q)
{
	if (worker_queue_is_empty(q))
		return NULL;

	return _worker_queue_dequeue(q);
}

static inline void _worker_queue_enqueue(struct worker_queue *q,
					 struct batch_context *batch)
{
	q->batch_ctxs[q->head % q->max_count] = batch;
	q->head++;
	osal_atomic_fetch_add(&q->atomic_count, 1);
}

static inline bool worker_queue_enqueue(struct worker_queue *q,
					struct batch_context *batch)
{
	if (worker_queue_is_full(q))
		return false;

	_worker_queue_enqueue(q, batch);
	return true;
}

struct worker_context {
	const struct test_desc *desc;
	const struct testcase_context *test_ctx;
	osal_thread_t worker_thread;
	uint32_t worker_id;
	uint64_t last_active_ts;

	struct worker_queue *submit_q;
	struct worker_queue *complete_q;
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
	struct worker_queue *batch_ctx_freelist;

	/* queue for polling batch_ctx, local to ctl thread */
	struct worker_queue *poll_q;

	uint32_t vars[TEST_VAR_MAX];
};

//#define FILE_NODE_BLOCK_SZ 4096
//struct test_node_block {
//	struct test_node node;
//	uint64_t file_offset;
//	struct pnso_flat_buffer buf;
//	uint8_t data[FILE_NODE_BLOCK_SZ];
//};

struct test_node_file {
	struct test_node node;
	osal_atomic_int_t lock;
	uint64_t checksum;
	uint64_t padded_checksum;
	uint32_t file_size;
	uint32_t padded_size;
	uint32_t alloc_size;
	uint32_t block_size;
	char filename[TEST_MAX_FULL_PATH_LEN+1];

	struct pnso_buffer_list *buflist;
	uint8_t *data;
#if 0
	/* MUST keep these 2 in order, due to zero-length array */
	struct pnso_buffer_list buflist;
	struct pnso_flat_buffer bufs[MAX_INPUT_BUF_COUNT];
#endif
};

struct test_file_table {
	bool initialized;
	osal_atomic_int_t bucket_locks[TEST_TABLE_BUCKET_COUNT];
	struct test_node_table table;
};

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* __PNSO_TEST_CTX_H__ */
