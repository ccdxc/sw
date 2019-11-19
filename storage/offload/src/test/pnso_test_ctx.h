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
#define MAX_CPDC_OUTPUT_BUF_LEN (60 * 1024)
#define DEFAULT_BUF_COUNT 16
#define DEFAULT_BLOCK_SIZE 4096
#define MAX_INPUT_BUF_COUNT 1024
#define MAX_OUTPUT_BUF_COUNT (MAX_OUTPUT_BUF_LEN / 4096)
#define MAX_COMPRESSION_FACTOR 10
#define MAX_CP_HDR_LEN 8

static inline uint32_t get_max_output_len_by_svc(const struct test_svc *svc,
						 uint32_t input_len,
						 uint32_t uncompressed_size)
{
	uint32_t hdr_len;

	if (svc->output_len)
		return svc->output_len;

	switch (svc->svc.svc_type) {
	case PNSO_SVC_TYPE_ENCRYPT:
	case PNSO_SVC_TYPE_DECRYPT:
		if (svc->output_flags & TEST_OUTPUT_FLAG_TINY) {
			return DEFAULT_BLOCK_SIZE;
		} else if (svc->output_flags & TEST_OUTPUT_FLAG_JUMBO) {
			return MAX_OUTPUT_BUF_LEN;
		} else {
			return input_len;
		}
	case PNSO_SVC_TYPE_COMPRESS:
		hdr_len = 0;
		if (svc->svc.u.cp_desc.flags & PNSO_CP_DFLAG_INSERT_HEADER)
			hdr_len = MAX_CP_HDR_LEN;
		if (svc->output_flags & TEST_OUTPUT_FLAG_TINY) {
			return DEFAULT_BLOCK_SIZE;
		} else if (svc->output_flags & TEST_OUTPUT_FLAG_JUMBO) {
			return MAX_OUTPUT_BUF_LEN;
		} else if (svc->u.cpdc.threshold_delta) {
			if (input_len > svc->u.cpdc.threshold_delta) {
				return hdr_len + input_len -
					svc->u.cpdc.threshold_delta;
			} else {
				return DEFAULT_BLOCK_SIZE;
			}
		} else if (svc->svc.u.cp_desc.threshold_len) {
			return hdr_len +
				svc->svc.u.cp_desc.threshold_len;
		} else {
			return input_len;
		}
	case PNSO_SVC_TYPE_DECOMPRESS:
		if (svc->output_flags & TEST_OUTPUT_FLAG_TINY) {
			return DEFAULT_BLOCK_SIZE;
		} else if (svc->output_flags & TEST_OUTPUT_FLAG_JUMBO) {
			return MAX_OUTPUT_BUF_LEN;
		} else if (uncompressed_size) {
			return uncompressed_size;
		} else if ((input_len * MAX_COMPRESSION_FACTOR > MAX_CPDC_OUTPUT_BUF_LEN)) {
			return MAX_CPDC_OUTPUT_BUF_LEN;
		} else {
			/* Cannot deduce */
			return 0;
			//return input_len * MAX_COMPRESSION_FACTOR;
		}
	case PNSO_SVC_TYPE_HASH:
		if (svc->output_flags & TEST_OUTPUT_FLAG_TINY) {
			return PNSO_HASH_TAG_LEN;
		} else {
			return MAX_OUTPUT_BUF_COUNT * PNSO_HASH_TAG_LEN;
		}
	case PNSO_SVC_TYPE_CHKSUM:
		if (svc->output_flags & TEST_OUTPUT_FLAG_TINY) {
			return PNSO_CHKSUM_TAG_LEN;
		} else {
			return MAX_OUTPUT_BUF_COUNT * PNSO_CHKSUM_TAG_LEN;
		}
	case PNSO_SVC_TYPE_DECOMPACT:
	default:
		if (svc->output_flags & TEST_OUTPUT_FLAG_TINY) {
			return PNSO_CHKSUM_TAG_LEN;
		} else {
			return MAX_OUTPUT_BUF_LEN;
		}
	}
}

struct buffer_context {
	bool initialized;
	uint32_t seed;
	uint32_t svc_chain_idx;
	uint32_t uncompressed_sz;
	uint32_t buf_alloc_sz;
	uint32_t buflist_alloc_count;
	struct pnso_buffer_list *va_buflist;
	struct pnso_buffer_list *pa_buflist;
	struct pnso_flat_buffer buf;
};

struct chain_context {
	const struct test_svc_chain *svc_chain;
	uint32_t batch_weight; /* cached and normalized from svc_chain */

	uint32_t input_count;
	struct buffer_context *inputs;
};

struct request_context {
	struct batch_context *batch_ctx;
	const struct test_svc_chain *svc_chain;
	uint64_t req_id;

	struct buffer_context *input; /* reference to chain_context or input_data */
	struct buffer_context input_data;

	struct buffer_context outputs[PNSO_SVC_TYPE_MAX];

	/* MUST keep these 2 in order, due to zero-length array */
	struct pnso_service_request svc_req;
	struct pnso_service req_svcs[PNSO_SVC_TYPE_MAX];

	/* MUST keep these 2 in order, due to zero-length array */
	struct pnso_service_result svc_res;
	struct pnso_service_status res_statuses[PNSO_SVC_TYPE_MAX];

	/* MUST keep these 2 in order, due to zero-length array */
#if 0
	struct pnso_buffer_list src_buflist;
	struct pnso_flat_buffer src_bufs[MAX_INPUT_BUF_COUNT];
#endif

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
	uint64_t retries;
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

union callback_context {
	uint64_t val;
	struct {
		uint16_t gen_id;
		uint16_t test_id;
		uint32_t batch_id;
	} s;
};

struct batch_context {
	const struct test_desc *desc;
	struct testcase_context *test_ctx;
	uint64_t first_req_id;
	uint32_t batch_id;
	uint32_t worker_id;
	uint16_t sync_mode;
	bool initialized;
	union callback_context cb_ctx;
	osal_atomic_int_t cb_count;
	pnso_error_t req_rc;
	pnso_error_t res_rc;

	pnso_poll_fn_t poll_fn;
	void *poll_ctx;

	uint64_t start_time;
	struct testcase_stats stats;

	/* Initial values inherited from parent */
	uint32_t vars[TEST_VAR_MAX];

	uint16_t req_count;
	uint16_t max_req_count;
	struct request_context *req_ctxs[TEST_MAX_BATCH_DEPTH];
};

/* No locking required iff one producer and one consumer */
struct worker_queue {
	osal_spinlock_t lock;
	//uint32_t count;
	osal_atomic_int_t atomic_count;
	uint32_t head;
	uint32_t tail;
	uint32_t max_count;
	uint32_t idx_mask;
	uint64_t enqueue_count;
	uint64_t enqueue_full_count;
	uint64_t enqueue_empty_count;
	uint64_t dequeue_count;
	uint64_t dequeue_full_count;
	uint64_t dequeue_empty_count;
	struct batch_context *batch_ctxs[0];
};

static inline bool worker_queue_is_full(struct worker_queue *q)
{
	if (osal_atomic_read(&q->atomic_count) >= q->max_count) {
		q->enqueue_full_count++;
		return true;
	}
	return false;
}

static inline bool worker_queue_is_empty(struct worker_queue *q)
{
	if (osal_atomic_read(&q->atomic_count) <= 0) {
		q->dequeue_empty_count++;
		return true;
	}
	return false;
}

static inline struct batch_context *_worker_queue_dequeue(struct worker_queue *q)
{
	struct batch_context *batch;

	batch = q->batch_ctxs[q->tail & q->idx_mask];
	q->tail++;
	osal_atomic_fetch_sub(&q->atomic_count, 1);
	q->dequeue_count++;
	return batch;
}

static inline struct batch_context *worker_queue_dequeue(struct worker_queue *q)
{
	int count = osal_atomic_read(&q->atomic_count);

	if (count <= 0) {
		q->dequeue_empty_count++;
		return NULL;
	} else if (count >= q->max_count) {
		q->dequeue_full_count++;
	}

	return _worker_queue_dequeue(q);
}

static inline struct batch_context *worker_queue_dequeue_safe(struct worker_queue *q)
{
	struct batch_context *ret;

	osal_spin_lock(&q->lock);
	ret = worker_queue_dequeue(q);
	osal_spin_unlock(&q->lock);

	return ret;
}

static inline void _worker_queue_enqueue(struct worker_queue *q,
					 struct batch_context *batch)
{
	q->batch_ctxs[q->head & q->idx_mask] = batch;
	q->head++;
	osal_atomic_fetch_add(&q->atomic_count, 1);
	q->enqueue_count++;
}

static inline bool worker_queue_enqueue(struct worker_queue *q,
					struct batch_context *batch)
{
	int count = osal_atomic_read(&q->atomic_count);

	if (count <= 0) {
		q->enqueue_empty_count++;
	} else if (count >= q->max_count) {
		q->enqueue_full_count++;
		return false;
	}

	_worker_queue_enqueue(q, batch);
	return true;
}

static inline bool worker_queue_enqueue_safe(struct worker_queue *q,
					     struct batch_context *batch)
{
	bool ret;

	osal_spin_lock(&q->lock);
	ret = worker_queue_enqueue(q, batch);
	osal_spin_unlock(&q->lock);

	return ret;
}

struct worker_context {
	const struct test_desc *desc;
	const struct testcase_context *test_ctx;
	osal_thread_t worker_thread;
	uint32_t worker_id;
	uint32_t pending_batch_count;
	uint64_t last_active_ts;

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

	uint32_t total_chain_weight; /* sum of svc_chain batch_weight */
	uint16_t max_chain_weight;
	uint16_t min_chain_weight;
	uint32_t chain_count;
	struct chain_context *chain_ctxs[MAX_SVC_CHAINS_PER_TESTCASE];

	/* list of chain ids for weighted-round-robin */
	uint32_t chain_lb_table_count;
	uint32_t *chain_lb_table;

	uint32_t worker_count;
	struct worker_context *worker_ctxs[TEST_MAX_CORE_COUNT];

	/*
	 * testcase_context owns these batch contexts,
	 * but they are distributed evenly between the worker_contexts
	 */
	uint16_t max_batch_depth;
	uint32_t batch_concurrency;
	struct worker_queue *batch_ctx_freelist;
	uint32_t batch_ctx_count;
	struct batch_context **batch_ctxs;

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
	uint32_t uncompressed_size;
	uint32_t input_seed;
	int sysfs_fd;
	uint32_t sysfs_gen_id;
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
