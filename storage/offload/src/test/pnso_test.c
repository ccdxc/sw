/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include "osal_stdtypes.h"
#include "osal_errno.h"
#include "osal_mem.h"
#include "osal_sys.h"
#include "osal_atomic.h"
#include "osal_thread.h"
#include "osal_time.h"
//#include <time.h>

#include "pnso_test.h"
#include "pnso_test_parse.h"
#include "pnso_test_ctx.h"
#include "pnso_pbuf.h"
#include "pnso_util.h"

#ifdef PNSO_SIM
#include "sim.h"
#endif

static osal_atomic_int_t g_shutdown;

static struct test_file_table g_output_file_tbl;

struct test_file_table *test_get_output_file_table(void)
{
	if (!g_output_file_tbl.initialized) {
		g_output_file_tbl.initialized = true;
		osal_atomic_init(&g_output_file_tbl.lookup_lock, 0);
		osal_atomic_init(&g_output_file_tbl.write_lock, 0);
	}
	return &g_output_file_tbl;
}

struct lookup_var {
	const char *name;
	uint32_t name_len;
};

#define LOOKUP_VAR(name) { name, strlen(name) }

struct lookup_var g_lookup_vars[TEST_VAR_MAX] = {
	LOOKUP_VAR("test_iter"),
	LOOKUP_VAR("test_id"),
	LOOKUP_VAR("chain_id"),
	LOOKUP_VAR("block_size"),
};

static uint32_t find_var_name(const char *name, uint32_t name_len)
{
	uint32_t i;
	for (i = 0; i < TEST_VAR_MAX; i++) {
		if (name_len == g_lookup_vars[i].name_len &&
		    0 == strncmp(name, g_lookup_vars[i].name, name_len)) {
			return i;
		}
	}
	return TEST_VAR_MAX;
}

static void copy_vars(const uint32_t *src, uint32_t *dst)
{
	int i;

	for (i = 0; i < TEST_VAR_MAX; i++) {
		dst[i] = src[i];
	}
}


/* Assumes dst is long enough to contain string */
static pnso_error_t construct_filename(const struct test_desc *desc,
				       uint32_t *vars,
				       char *dst, const char *src)
{
	const char *orig_src = src;
	char *orig_dst = dst;
	const char *var_name, *tmp;
	int var_len;
	uint32_t var_id;

	*dst = '\0';

	/* Prepend the global file prefix */
	if (desc->output_file_prefix[0]) {
		strcpy(dst, desc->output_file_prefix);
		dst += strlen(desc->output_file_prefix);
	}

	/* Find and replace %special% variables, and copy literal text */
	while (*src) {
		if (*src == '%') {
			src++;
			var_name = src;
			tmp = strchr(src, '%');
			if (!tmp || tmp == src) {
				PNSO_LOG_ERROR("Illegal percent character in filename %s\n",
					       orig_src);
				goto error;
			}
			var_len = (uint32_t) (tmp - src);
			var_id = find_var_name(var_name, var_len);
			if (var_id >= TEST_VAR_MAX) {
				PNSO_LOG_ERROR("Unknown variable in filename %s\n",
					       orig_src);
				goto error;
			}
			dst += sprintf(dst, "%u", vars[var_id]);
			src += var_len + 1;
		} else {
			*dst = *src;
			src++;
			dst++;
		}
	}
	*dst = '\0';
	return PNSO_OK;

error:
	*orig_dst = '\0';
	return EINVAL;
}


/* Fill buffer by repeating the given pattern */
pnso_error_t test_fill_pattern(struct pnso_buffer_list *buflist,
			       const char *pat, uint32_t pat_len)
{
	size_t i, j, pat_i;
	uint8_t *dst;
	uint8_t hex_pat[TEST_MAX_PATTERN_LEN];

	/* Check whether pattern should be interpreted as binary */
	if (pat[0] == '0' && (pat[1] == 'x' || pat[1] == 'X')) {
		uint32_t tmp = sizeof(hex_pat) - 1;

		if (parse_hex(pat+2, hex_pat, &tmp) == PNSO_OK) {
			pat = (const char *) hex_pat;
			pat_len = tmp;
		}
	}

	pat_i = 0;
	for (i = 0; i < buflist->count; i++) {
		if (!buflist->buffers[i].len) {
			continue;
		}

		dst = (uint8_t *) buflist->buffers[i].buf;
		for (j = 0; j < buflist->buffers[i].len; j++) {
			dst[j] = (uint8_t) pat[pat_i % pat_len];
			pat_i++;
		}
	}

	return PNSO_OK;
}

/*
 * Fill the given buffer list by one of the three supported methods:
 *     file, pattern, or random
 */
static pnso_error_t test_read_input(const char *path,
				    const struct test_input_desc *input_desc,
				    struct pnso_buffer_list *buflist)
{
	pnso_error_t err = PNSO_OK;
	if (path && *path) {
	  err = test_read_file(path, buflist, input_desc->offset, input_desc->len);
	} else if (input_desc->pattern[0]) {
		uint32_t pat_len = strnlen(input_desc->pattern, TEST_MAX_PATTERN_LEN);
		err = test_fill_pattern(buflist, input_desc->pattern, pat_len);
	} else {
		err = test_fill_random(buflist, input_desc->random_seed);
	}
	return err;
}

static struct pnso_buffer_list *test_alloc_buffer_list(uint32_t count, uint32_t total_bytes)
{
	struct pnso_buffer_list *buflist;
	uint8_t *data;
	uint32_t block_size = total_bytes / count;
	size_t i;

	buflist = TEST_ALLOC(sizeof(struct pnso_buffer_list) +
			     (sizeof(struct pnso_flat_buffer) * count) +
			     total_bytes);
	if (buflist) {
		data = ((uint8_t*)buflist) + sizeof(struct pnso_buffer_list) +
			(sizeof(struct pnso_flat_buffer) * count);
		buflist->count = count;
		for (i = 0; i < count; i++) {
			buflist->buffers[i].len = block_size;
			buflist->buffers[i].buf = (uint64_t) data + (block_size * i);
		}
	}
	return buflist;
}

static void test_free_buffer_list(struct pnso_buffer_list *buflist)
{
	if (buflist) {
		TEST_FREE(buflist);
	}
}

const uint64_t TEST_MCRC64_POLY = 0x9a6c9329ac4bc9b5ULL;
uint64_t compute_checksum(const struct pnso_buffer_list *buflist, uint8_t *data, uint32_t length)
{
	size_t i, j;
	uint64_t crc = 0xFFFFFFFFFFFFFFFFULL;
	uint32_t buflist_count = buflist ? buflist->count : 0;
	uint32_t buflist_i = 0;
	uint32_t data_len = buflist ? 0 : length;
	uint32_t data_i = 0;

	if (!buflist && !data) {
		return 0;
	}

	for (i = 0; i < length; i++) {
		if (data_i >= data_len) {
			/* Get next buffer in list */
			if (buflist_i >= buflist_count) {
				/* Not enough data */
				return 0;
			}
			data = (uint8_t *) buflist->buffers[buflist_i].buf;
			data_len = buflist->buffers[buflist_i].len;
			data_i = 0;
			buflist_i++;
		}

		crc ^= data[data_i++];
		for (j = 0; j < 8; j++) {
			if (crc & 1) {
				crc = (crc >> 1) ^ TEST_MCRC64_POLY;
			} else {
				crc = (crc >> 1);
			}
		}
	}
	return (crc ^ 0xFFFFFFFFFFFFFFFFULL);
}

static int cmp_file_node(struct test_node *node1, struct test_node *node2)
{
	if (node1->idx != node2->idx) {
		return -1;
	}
	return strcmp(((struct test_node_file *) node1)->filename,
		      ((struct test_node_file *) node2)->filename);
}

static int cmp_file_node_data(struct test_node_file *fnode1, struct test_node_file *fnode2)
{
	if (fnode1->file_size == fnode2->file_size &&
	    fnode1->checksum == fnode2->checksum) {
		return 0;
	}

	/* Try padded size instead (required for compression) */
	if (fnode1->padded_size &&
	    fnode1->padded_size == fnode2->file_size &&
	    fnode1->padded_checksum == fnode2->checksum) {
		return 0;
	}
	if (fnode2->padded_size &&
	    fnode2->padded_size == fnode1->file_size &&
	    fnode2->padded_checksum == fnode1->checksum) {
		return 0;
	}
	if (fnode1->padded_size && fnode2->padded_size &&
	    fnode1->padded_size == fnode2->padded_size &&
	    fnode1->padded_checksum == fnode2->padded_checksum) {
		return 0;
	}
	return -1;
}

static struct test_node_file *lookup_file_node(struct test_file_table *table,
					       const char *filename)
{
	struct test_node_file search_fnode;
	struct test_node_file *fnode;

	memset(&search_fnode, 0, sizeof(search_fnode));
	search_fnode.node.type = NODE_FILE;
	search_fnode.node.idx = str_hash(filename);
	strncpy(search_fnode.filename, filename, TEST_MAX_FULL_PATH_LEN);

	osal_atomic_lock(&table->lookup_lock);
	fnode = (struct test_node_file *) test_node_table_lookup(&table->table,
							&search_fnode.node,
							cmp_file_node);
	osal_atomic_unlock(&table->lookup_lock);
	return fnode;
}

static pnso_error_t insert_unique_file_node(struct test_file_table *table,
					    const char *filename,
					    uint64_t checksum,
					    uint32_t file_size,
					    uint64_t padded_checksum,
					    uint32_t padded_size)
{
	struct test_node_file search_fnode;
	struct test_node_file *fnode;
	bool is_new = false;

	memset(&search_fnode, 0, sizeof(search_fnode));
	search_fnode.node.type = NODE_FILE;
	search_fnode.node.idx = str_hash(filename);
	strncpy(search_fnode.filename, filename, TEST_MAX_FULL_PATH_LEN);

	osal_atomic_lock(&table->lookup_lock);
	fnode = (struct test_node_file *) test_node_table_lookup(&table->table,
					&search_fnode.node, cmp_file_node);
	if (fnode) {
		/* Found it, just update checksum and size */
		goto update;
	}

	fnode = (struct test_node_file *) test_node_alloc(sizeof(*fnode),
							  NODE_FILE);
	if (!fnode) {
		osal_atomic_unlock(&table->lookup_lock);
		return ENOMEM;
	}
	is_new = true;
	*fnode = search_fnode;

update:
	fnode->checksum = checksum;
	fnode->file_size = file_size;
	fnode->padded_checksum = padded_checksum;
	fnode->padded_size = padded_size;

	if (is_new) {
		test_node_table_insert(&table->table, &fnode->node);
	}
	osal_atomic_unlock(&table->lookup_lock);

	return PNSO_OK;
}

static void output_results(struct request_context *req_ctx,
			   const struct test_svc_chain *svc_chain)
{
	uint32_t i;
	struct test_node *node;
	bool wrote_file = false;
	uint64_t checksum = 0;
	uint64_t padded_checksum = 0;
	uint32_t file_size = 0;
	uint32_t padded_size = 0;
	pnso_error_t err;
	struct batch_context *batch_ctx = req_ctx->batch_ctx;
	struct testcase_context *test_ctx = batch_ctx->test_ctx;
	uint32_t blk_sz = batch_ctx->desc->init_params.block_size;
	char output_path[TEST_MAX_FULL_PATH_LEN] = "";

	/* output to file */
	i = 0;
	FOR_EACH_NODE(svc_chain->svcs) {
		struct test_svc *svc = (struct test_svc *) node;
		struct pnso_service_status *svc_status = &req_ctx->svc_res.svc[i];
		if (svc->output_path[0]) {
			err = construct_filename(batch_ctx->desc, req_ctx->vars,
						 output_path, svc->output_path);
			if (err != PNSO_OK) {
				break;
			}
			wrote_file = false;
			padded_size = 0;
			if (svc_status->svc_type == PNSO_SVC_TYPE_HASH ||
			    svc_status->svc_type == PNSO_SVC_TYPE_CHKSUM) {
				if (svc_status->u.hash.tags) {
					size_t tag_size = sizeof(struct pnso_hash_tag);
					if (svc_status->svc_type == PNSO_SVC_TYPE_CHKSUM) {
						tag_size = sizeof(struct pnso_chksum_tag);
					}
					/* Hack alert: static allocation for zero-len array */
					struct {
						struct pnso_buffer_list bl;
						struct pnso_flat_buffer b;
					} buflist;
					buflist.bl.count = 1;
					buflist.b.buf = (uint64_t) svc_status->u.hash.tags;
					buflist.b.len = svc_status->u.hash.num_tags *
							tag_size;
					file_size = buflist.b.len;
					checksum = compute_checksum(NULL,
							(uint8_t *) buflist.b.buf,
							file_size);
					err = test_write_file(output_path,
							      &buflist.bl,
							      buflist.b.len,
							      svc->output_flags);
					wrote_file = (err == PNSO_OK);
				}
			} else if (svc_status->u.dst.sgl) {
				file_size = svc_status->u.dst.data_len;
				checksum = compute_checksum(svc_status->u.dst.sgl,
							    NULL, file_size);
				err = test_write_file(output_path,
						      svc_status->u.dst.sgl,
						      svc_status->u.dst.data_len,
						      svc->output_flags);
				wrote_file = (err == PNSO_OK);
				if (svc->svc.svc_type == PNSO_SVC_TYPE_COMPRESS &&
				    (svc->svc.u.cp_desc.flags & PNSO_CP_DFLAG_ZERO_PAD)) {
					/* Round up */
					padded_size = ((file_size + blk_sz - 1) /
						       blk_sz) * blk_sz;
					padded_checksum = compute_checksum(
						svc_status->u.dst.sgl,
						NULL, padded_size);
				}
			}
			if (err != PNSO_OK) {
				PNSO_LOG_ERROR("Cannot write data to %s\n",
					       output_path);
				break;
			}
			if (wrote_file) {
				insert_unique_file_node(
					test_ctx->output_file_tbl,
					output_path, checksum, file_size,
					padded_checksum, padded_size);
			}
		}
		i++;
	}
}

static struct worker_context *batch_get_worker_ctx(struct batch_context *batch_ctx)
{
	if (batch_ctx->worker_id >= batch_ctx->test_ctx->worker_count)
		return NULL;

	return batch_ctx->test_ctx->worker_ctxs[batch_ctx->worker_id];
}

static void batch_completion_cb(void *cb_ctx, struct pnso_service_result *svc_res)
{
	struct batch_context *batch_ctx = (struct batch_context *) cb_ctx;
	struct worker_context *worker_ctx;

	batch_ctx->stats.agg_stats.total_latency = osal_get_clock_nsec() - batch_ctx->start_time;

	osal_atomic_fetch_add(&batch_ctx->cb_count, 1);

	worker_ctx = batch_ctx->test_ctx->worker_ctxs[batch_ctx->worker_id];
	worker_queue_enqueue(worker_ctx->complete_q, batch_ctx);
}

static pnso_error_t test_submit_request(struct request_context *req_ctx,
					uint16_t sync_mode,
					bool is_batched,
					bool flush)
{
	pnso_error_t rc = PNSO_OK;
	struct batch_context *batch_ctx = req_ctx->batch_ctx;
	completion_cb_t cb;
	void *cb_ctx;
	pnso_poll_fn_t *poll_fn;
	void **poll_ctx;

	switch (sync_mode) {
	case SYNC_MODE_SYNC:
		cb = NULL;
		cb_ctx = NULL;
		poll_fn = NULL;
		poll_ctx = NULL;
		break;
	case SYNC_MODE_POLL:
		cb = batch_completion_cb;
		cb_ctx = batch_ctx;
		poll_fn = &batch_ctx->poll_fn;
		poll_ctx = &batch_ctx->poll_ctx;
		break;
	case SYNC_MODE_ASYNC:
	default:
		cb = batch_completion_cb;
		cb_ctx = batch_ctx;
		poll_fn = NULL;
		poll_ctx = NULL;
		break;
	}

	if (is_batched) {
		rc = pnso_add_to_batch(&req_ctx->svc_req, &req_ctx->svc_res);
		if (flush && rc == PNSO_OK) {
			req_ctx->batch_ctx->start_time = osal_get_clock_nsec();
			rc = pnso_flush_batch(cb, cb_ctx,
					      poll_fn, poll_ctx);
		}
	} else {
		req_ctx->batch_ctx->start_time = osal_get_clock_nsec();
		rc = pnso_submit_request(&req_ctx->svc_req,
					 &req_ctx->svc_res,
					 cb, cb_ctx,
					 poll_fn, poll_ctx);
	}

	return rc;
}

static pnso_error_t run_testcase_svc_chain(struct request_context *req_ctx,
					   const struct test_testcase *testcase,
					   const struct test_svc_chain *svc_chain,
					   uint32_t batch_iter,
					   uint32_t batch_count)
{
	struct batch_context *batch_ctx = req_ctx->batch_ctx;
	pnso_error_t err = PNSO_OK;
	uint32_t input_len, remain_len, buf_len;
	uint32_t min_block, max_block, block_count;
	uint32_t i;
	uint8_t *buf;
	struct test_node *node;
	char input_path[TEST_MAX_FULL_PATH_LEN] = "";

	/* construct input filename */
	if (svc_chain->input.pathname[0]) {
		err = construct_filename(batch_ctx->desc, req_ctx->vars,
					 input_path, svc_chain->input.pathname);
		if (err != PNSO_OK) {
			return err;
		}
	}

	/* setup source buffers */
	input_len = svc_chain->input.len;
	if (!input_len) {
		/* Try to infer the length, for user convenience */
		if (input_path[0]) {
			input_len = test_file_size(input_path);
			if (!input_len) {
				PNSO_LOG_DEBUG("Invalid input file %s\n", input_path);
				return EINVAL;
			}
		} else if (svc_chain->input.pattern[0]) {
			input_len = strlen(svc_chain->input.pattern);
		} else {
			input_len = batch_ctx->desc->init_params.block_size;
		}
	}
	req_ctx->input_buffer = TEST_ALLOC(input_len);
	if (!req_ctx->input_buffer) {
		PNSO_LOG_TRACE("Failed to alloc %u bytes for input_buffer\n",
			       input_len);
		return ENOMEM;
	}
	min_block = svc_chain->input.min_block_size;
	max_block = svc_chain->input.max_block_size;
	block_count = svc_chain->input.block_count;
	if (!min_block) {
		min_block = batch_ctx->desc->init_params.block_size;
	}
	if (!max_block) {
		max_block = batch_ctx->desc->init_params.block_size;
	}
	if (max_block < min_block) {
		max_block = min_block;
	}
	if (!block_count) {
		/* Pick smallest block count that works */
		block_count = (input_len + max_block - 1) / max_block;
	}

	if ((block_count > MAX_INPUT_BUF_COUNT) ||
	    (max_block * block_count) < input_len) {
		PNSO_LOG_ERROR("Cannot represent %u bytes input with %u byte blocks\n",
			       input_len, max_block);
		return EINVAL;
	}
	remain_len = input_len;
	buf = req_ctx->input_buffer;
	for (i = 0; i < block_count && remain_len; i++) {
		/* Prefer min_block size, if we have enough blocks for it */
		buf_len = remain_len;
		if (remain_len <= (min_block * (block_count - i))) {
			if (min_block < remain_len) {
				buf_len = min_block;
			}
		} else {
			if (max_block < remain_len) {
				buf_len = max_block;
			}
		}
		req_ctx->src_bufs[i].buf = (uint64_t) buf;
		req_ctx->src_bufs[i].len = buf_len;
		buf += buf_len;
		remain_len -= buf_len;
	}
	req_ctx->src_buflist.count = i;

	/* populate input buffer */
	err = test_read_input(input_path, &svc_chain->input, &req_ctx->src_buflist);
	if (err != PNSO_OK) {
		return err;
	}

	/* setup request */
	req_ctx->svc_req.sgl = &req_ctx->src_buflist;
	req_ctx->svc_req.num_services = svc_chain->num_services;
	i = 0;
	FOR_EACH_NODE(svc_chain->svcs) {
		struct test_svc *svc = (struct test_svc *) node;
		req_ctx->svc_req.svc[i] = svc->svc;
		switch (req_ctx->svc_req.svc[i].svc_type) {
		case PNSO_SVC_TYPE_COMPRESS:
			if (svc->u.cpdc.threshold_delta) {
				if (svc->u.cpdc.threshold_delta >= input_len) {
					PNSO_LOG_ERROR("CP threshold_delta %u "
						"larger than input_len %u.\n",
						svc->u.cpdc.threshold_delta,
						input_len);
					return EINVAL;
				}
				req_ctx->svc_req.svc[i].u.cp_desc.threshold_len =
					input_len - svc->u.cpdc.threshold_delta;
			}
			break;
		case PNSO_SVC_TYPE_ENCRYPT:
		case PNSO_SVC_TYPE_DECRYPT:
			/* TODO: iv_data_len */
			break;
		default:
			break;
		}
		i++;
	}

	/* setup response */
	req_ctx->svc_res.err = PNSO_OK;
	req_ctx->svc_res.num_services = svc_chain->num_services;
	i = 0;
	FOR_EACH_NODE(svc_chain->svcs) {
		struct test_svc *svc = (struct test_svc *) node;
		struct pnso_service_status *svc_status = &req_ctx->svc_res.svc[i];
		svc_status->svc_type = svc->svc.svc_type; /* TODO: needed? */
		if (svc->output_path[0]) {
			if (svc->svc.svc_type == PNSO_SVC_TYPE_HASH ||
			    svc->svc.svc_type == PNSO_SVC_TYPE_CHKSUM) {
				if (!svc_status->u.hash.tags) {
					svc_status->u.hash.tags = TEST_ALLOC(
						get_max_output_len_by_type(
							svc->svc.svc_type,
							svc->output_flags));
					if (!svc_status->u.hash.tags) {
						PNSO_LOG_TRACE(
							"Out of memory for output tags\n");
						return ENOMEM;
					}
					svc_status->u.hash.num_tags =
						(svc->output_flags &
						 TEST_OUTPUT_FLAG_TINY) ? 1 :
						MAX_OUTPUT_BUF_COUNT;
				}
			} else if (!svc_status->u.dst.sgl) {
				svc_status->u.dst.sgl =
					test_alloc_buffer_list(
						(svc->output_flags &
						 TEST_OUTPUT_FLAG_TINY) ? 1 :
						MAX_OUTPUT_BUF_COUNT,
						get_max_output_len_by_type(
							svc->svc.svc_type,
							svc->output_flags));
				if (!svc_status->u.dst.sgl) {
					PNSO_LOG_TRACE(
						"Out of memory for output_buf\n");
					return ENOMEM;
				}
			}
		}
		i++;
	}

	/* stats */
	batch_ctx->stats.io_stats[0].svcs += svc_chain->num_services;
	batch_ctx->stats.io_stats[0].reqs += 1;
	if (batch_iter == (batch_count - 1)) {
		batch_ctx->stats.io_stats[0].batches += 1;
	}
	batch_ctx->stats.io_stats[0].bytes += pbuf_get_buffer_list_len(req_ctx->svc_req.sgl);	

	/* Execute */
	err = test_submit_request(req_ctx, testcase->sync_mode,
				  (batch_count > 1),
				  (batch_iter == batch_count - 1));
	req_ctx->res_rc = err;

	return err;
}

static const char *testcase_stats_names[TESTCASE_STATS_COUNT] = {
	"elapsed_time",
	"total_latency",
	"avg_latency",
	"bytes_per_sec",
	"svcs_per_sec",
	"reqs_per_sec",
	"batches_per_sec",

	"in_svc_count",
	"in_req_count",
	"in_batch_count",
	"in_byte_count",
	"in_failures",

	"out_svc_count",
	"out_req_count",
	"out_batch_count",
	"out_byte_count",
	"out_failures",
};

static uint64_t calculate_bytes_per_sec(uint64_t bytes, uint64_t ns)
{
	if (ns == 0)
		return 0;

	if (ns < (10 * OSAL_NSEC_PER_SEC)) {
		return (bytes * OSAL_NSEC_PER_SEC) / ns;
	} else {
		return bytes / (ns / OSAL_NSEC_PER_SEC);
	}
}

static void calculate_completion_stats(struct batch_context *batch_ctx)
{
	uint32_t i, j;
	struct request_context *req_ctx;
	struct pnso_service_status *svc_status;
	struct testcase_io_stats *stats = &batch_ctx->stats.io_stats[1];

	stats->batches += 1;
	for (i = 0; i < batch_ctx->req_count; i++) {
		req_ctx = batch_ctx->req_ctxs[i];
		if (!req_ctx)
			goto error;
		stats->reqs += 1;
		if (req_ctx->svc_res.num_services != req_ctx->svc_req.num_services)
			goto error;
		for (j = 0; j < req_ctx->svc_res.num_services; j++) {
			svc_status = &req_ctx->svc_res.svc[j];
			stats->svcs += 1;
			if (pnso_svc_type_is_data(svc_status->svc_type)) {
				stats->bytes += svc_status->u.dst.data_len;
			} else if (svc_status->svc_type == PNSO_SVC_TYPE_CHKSUM) {
				stats->bytes += svc_status->u.chksum.num_tags *
					pnso_get_chksum_algo_size(req_ctx->svc_req.svc[j].u.chksum_desc.algo_type);
			} else if (svc_status->svc_type == PNSO_SVC_TYPE_HASH) {
				stats->bytes += svc_status->u.hash.num_tags *
					pnso_get_hash_algo_size(req_ctx->svc_req.svc[j].u.hash_desc.algo_type);
			} else {
				goto error;
			}
		}
	}
	return;

error:
	batch_ctx->stats.io_stats[1].failures += 1;
	return;
}

static void aggregate_testcase_stats(struct testcase_stats *ts1,
				     const struct testcase_stats *ts2,
				     uint64_t elapsed_time)
{
	uint32_t i;

	ts1->elapsed_time = elapsed_time;
	ts1->agg_stats.total_latency += ts2->agg_stats.total_latency;
	for (i = 0; i < 2; i++) {
		ts1->io_stats[i].svcs += ts2->io_stats[i].svcs;
		ts1->io_stats[i].reqs += ts2->io_stats[i].reqs;
		ts1->io_stats[i].batches += ts2->io_stats[i].batches;
		ts1->io_stats[i].bytes += ts2->io_stats[i].bytes;
		ts1->io_stats[i].failures += ts2->io_stats[i].failures;
	}

	/* calculate latency and throughput */
	if (ts1->io_stats[1].batches) {
		ts1->agg_stats.avg_latency = ts1->agg_stats.total_latency /
				ts1->io_stats[1].batches;
	}
	ts1->agg_stats.bytes_per_sec = calculate_bytes_per_sec(
		       ts1->io_stats[1].bytes, elapsed_time);
	ts1->agg_stats.svcs_per_sec = calculate_bytes_per_sec(
		       ts1->io_stats[1].svcs, elapsed_time);
	ts1->agg_stats.reqs_per_sec = calculate_bytes_per_sec(
		       ts1->io_stats[1].reqs, elapsed_time);
	ts1->agg_stats.batches_per_sec = calculate_bytes_per_sec(
		       ts1->io_stats[1].batches, elapsed_time);
}

static bool is_compare_true(uint16_t cmp_type, int cmp)
{
	bool success = true;

	switch (cmp_type) {
	case COMPARE_TYPE_EQ:
		success = (cmp == 0);
		break;
	case COMPARE_TYPE_NE:
		success = (cmp != 0);
		break;
	case COMPARE_TYPE_LT:
		success = (cmp < 0);
		break;
	case COMPARE_TYPE_LE:
		success = (cmp <= 0);
		break;
	case COMPARE_TYPE_GT:
		success = (cmp > 0);
		break;
	case COMPARE_TYPE_GE:
		success = (cmp >= 0);
		break;
	default:
		success = false;
		break;
	}

	return success;
}

static pnso_error_t run_data_validation(struct batch_context *ctx,
					const struct test_testcase *testcase,
					struct test_validation *validation)
{
	pnso_error_t err = PNSO_OK;
	struct testcase_context *test_ctx = ctx->test_ctx;
	struct test_node_file *fnode1, *fnode2;
	char path1[TEST_MAX_FULL_PATH_LEN] = "";
	char path2[TEST_MAX_FULL_PATH_LEN] = "";
	int cmp = 0;

	if (validation->svc_chain_idx) {
		ctx->vars[TEST_VAR_CHAIN] = validation->svc_chain_idx;
	}

	/* construct dynamic pathnames */
	if (validation->file1[0]) {
		err = construct_filename(ctx->desc, ctx->vars,
					 path1, validation->file1);
	}
	if (err == PNSO_OK && validation->file2[0]) {
		err = construct_filename(ctx->desc, ctx->vars,
					 path2, validation->file2);
	}
	if (err != PNSO_OK) {
		goto done;
	}

	switch (validation->type) {
	case VALIDATION_DATA_COMPARE:
		if (path1[0] && path2[0]) {
			cmp = -1;
			/* Try to compare just local metadata */
			fnode1 = lookup_file_node(test_ctx->output_file_tbl, path1);
			fnode2 = lookup_file_node(test_ctx->output_file_tbl, path2);
			if (fnode1 && fnode2) {
				cmp = cmp_file_node_data(fnode1, fnode2);
			}
			/* Metadata not available or inconclusive, do full file compare */
			if (cmp != 0) {
				cmp = test_compare_files(path1, path2,
							 validation->offset,
							 validation->len);
			}
		} else if (validation->pattern[0] && (path1[0] || path2[0])) {
			char *path = path1;
			const char *pat = validation->pattern;
			uint32_t pat_len = strlen(validation->pattern);
			uint8_t hex_pat[TEST_MAX_PATTERN_LEN];

			if (!path1[0])
				path = path2;

			if (pat[0] == '0' && (pat[1] == 'x' || pat[1] == 'X')) {
				uint32_t tmp = sizeof(hex_pat) - 1;

				if (parse_hex(pat+2, hex_pat, &tmp) == PNSO_OK) {
					pat = (const char *) hex_pat;
					pat_len = tmp;
				}
			}

			cmp = test_compare_file_data(path,
						     validation->offset,
						     validation->len,
						     (const uint8_t *) pat,
						     pat_len);
		} else {
			err = EINVAL;
		}
		break;
	case VALIDATION_SIZE_COMPARE:
		if (validation->len) {
			/* Test static length */
			cmp = (int) test_file_size(path1) -
				(int) validation->len;
		} else {
			/* Compare size of 2 files */
			cmp = (int) test_file_size(path1) -
				(int) test_file_size(path2);
		}
		break;
	default:
		PNSO_LOG_ERROR("Invalid validation type %u\n",
			       validation->type);
		err = EINVAL;
		break;
	}

done:
	osal_atomic_lock(&ctx->test_ctx->stats_lock);
	if (err == PNSO_OK) {
		if (is_compare_true(validation->cmp_type, cmp)) {
			validation->rt_success_count++;
		} else {
			validation->rt_failure_count++;
		}
	} else {
		validation->rt_failure_count++;
	}
	osal_atomic_unlock(&ctx->test_ctx->stats_lock);
	return err;
}

static pnso_error_t run_retcode_validation(struct request_context *req_ctx,
					   const struct test_testcase *testcase,
					   struct test_validation *validation)
{
	pnso_error_t err = PNSO_OK;
	size_t i;
	int cmp = 0;
	struct batch_context *batch_ctx = req_ctx->batch_ctx;

	if (req_ctx->svc_res.num_services < validation->svc_count) {
		err = EINVAL;
		goto done;
	}

	cmp = (int) req_ctx->svc_res.err - (int) validation->retcode;
	if (cmp != 0) {
		goto done;
	}

	for (i = 0; i < validation->svc_count; i++) {
		cmp = (int) req_ctx->svc_res.svc[i].err -
			(int) validation->svc_retcodes[i];
		if (cmp != 0) {
			break;
		}
	}

done:
	osal_atomic_lock(&batch_ctx->test_ctx->stats_lock);
	if (err == PNSO_OK) {
		if (is_compare_true(validation->cmp_type, cmp)) {
			validation->rt_success_count++;
		} else {
			validation->rt_failure_count++;
		}
	} else {
		validation->rt_failure_count++;
	}
	osal_atomic_unlock(&batch_ctx->test_ctx->stats_lock);
	return err;
}

static pnso_error_t run_req_validation(struct request_context *req_ctx)
{
	const struct test_testcase *testcase;
	struct test_node *node;

	if (!req_ctx || !req_ctx->svc_chain)
		return EINVAL;

	testcase = req_ctx->batch_ctx->test_ctx->testcase;

	output_results(req_ctx, req_ctx->svc_chain);

	FOR_EACH_NODE(testcase->validations) {
		struct test_validation *validation =
			(struct test_validation *) node;
		if (validation->type ==	VALIDATION_RETCODE_COMPARE &&
		    (validation->svc_chain_idx == 0 ||
		     validation->svc_chain_idx == req_ctx->svc_chain->node.idx)) {
			run_retcode_validation(req_ctx, testcase, validation);
		}
	}

	return PNSO_OK;
}

static pnso_error_t run_batch_validation(struct batch_context *batch_ctx)
{
	pnso_error_t err = PNSO_OK;
	const struct test_testcase *testcase = batch_ctx->test_ctx->testcase;
	struct test_node *node;
	struct request_context *req_ctx;
	int i;

	/* run the corresponding result output and retcode validation */
	for (i = 0; i < batch_ctx->req_count; i++) {
		req_ctx = batch_ctx->req_ctxs[i];

		err = run_req_validation(req_ctx);
		if (err)
			break;
	}

	/* run file validations */
	FOR_EACH_NODE(testcase->validations) {
		struct test_validation *validation =
				(struct test_validation *) node;
		if (validation->type != VALIDATION_RETCODE_COMPARE) {
			run_data_validation(batch_ctx, testcase, validation);
		}
	}

	return err;
}

static void reset_req_context(struct request_context *ctx)
{
	uint32_t i;

	if (!ctx) {
		return;
	}

	if (ctx->input_buffer) {
		TEST_FREE(ctx->input_buffer);
	}

	for (i = 0; i < PNSO_SVC_TYPE_MAX; i++) {
		if (ctx->svc_res.svc[i].svc_type == PNSO_SVC_TYPE_HASH ||
		    ctx->svc_res.svc[i].svc_type == PNSO_SVC_TYPE_CHKSUM) {
			if (ctx->svc_res.svc[i].u.hash.tags) {
				TEST_FREE(ctx->svc_res.svc[i].u.hash.tags);
			}
		} else {
			test_free_buffer_list(ctx->svc_res.svc[i].u.dst.sgl);
		}
	}

	memset(ctx, 0, sizeof(*ctx));
}

static void init_req_context(struct request_context *req_ctx,
			     struct batch_context *batch_ctx,
			     const struct test_svc_chain *svc_chain)
{
	reset_req_context(req_ctx);
	req_ctx->batch_ctx = batch_ctx;
	req_ctx->svc_chain = svc_chain;
	copy_vars(batch_ctx->vars, req_ctx->vars);
	req_ctx->vars[TEST_VAR_CHAIN] = svc_chain->node.idx;
}

static pnso_error_t run_testcase_batch(struct batch_context *batch_ctx)
{
	pnso_error_t err = PNSO_OK;
	struct test_node *node;
	struct test_svc_chain *svc_chain;
	struct request_context *req_ctx;
	struct worker_context *worker_ctx = batch_get_worker_ctx(batch_ctx);
	const struct test_testcase *testcase = batch_ctx->test_ctx->testcase;
	uint32_t i, chain_i;

	if (!worker_ctx) {
		err = EINVAL;
		goto error;
	}

	batch_ctx->vars[TEST_VAR_ID] = testcase->node.idx;

	/* Run each request, alternating svc_chain */
	chain_i = 0;
	for (i = 0; i < batch_ctx->req_count; i++) {
		/* get svc_chain */
		NODE_FIND_ID(batch_ctx->desc->svc_chains, testcase->svc_chains[chain_i]);
		svc_chain = (struct test_svc_chain *) node;
		if (!svc_chain) {
			PNSO_LOG_ERROR("Svc_chain %u not found for testcase %u\n",
				      testcase->svc_chains[chain_i], testcase->node.idx);
			err = EINVAL;
			goto error;
		}
		if (++chain_i >= testcase->svc_chain_count) {
			chain_i = 0;
		}
		batch_ctx->vars[TEST_VAR_CHAIN] = svc_chain->node.idx;

		req_ctx = batch_ctx->req_ctxs[i];
		if (!req_ctx) {
			PNSO_LOG_ERROR("Request context %u not found "
				       "for testcase %u\n",
				       i, testcase->node.idx);
			err = EINVAL;
			goto error;
		}
		init_req_context(req_ctx, batch_ctx, svc_chain);
		err = run_testcase_svc_chain(req_ctx, testcase, svc_chain,
					     i, batch_ctx->req_count);
		if (err != PNSO_OK)
			goto error;
	}

	/* Special handling for SYNC and POLL */
	if (testcase->sync_mode == SYNC_MODE_SYNC) {
		/* Manually call completion callback */
		batch_completion_cb(batch_ctx,
				    req_ctx ? &req_ctx->svc_res : NULL);
	} else if (testcase->sync_mode == SYNC_MODE_POLL) {
		/* Place on special polling queue */
		worker_queue_enqueue(worker_ctx->poll_q, batch_ctx);
	}

	return PNSO_OK;

error:
	/* Return the batch_ctx to the testcase thread, for stats aggregation */
	batch_ctx->stats.io_stats[0].failures++;
	if (worker_ctx)
		worker_queue_enqueue(worker_ctx->complete_q, batch_ctx);

	return err;
}

static struct request_context *alloc_req_context()
{
	struct request_context *req_ctx;

	req_ctx = (struct request_context *) TEST_ALLOC(sizeof(*req_ctx));
	if (req_ctx) {
		memset(req_ctx, 0, sizeof(*req_ctx));
	} else {
		PNSO_LOG_WARN("Failed to allocate request_context.\n");
	}

	return req_ctx;
}

static void free_req_context(struct request_context *ctx)
{
	reset_req_context(ctx);

	if (ctx) {
		TEST_FREE(ctx);
	}
}

static void free_batch_context(struct batch_context *ctx)
{
	uint32_t i;

	if (!ctx) {
		return;
	}

	for (i = 0; i < ctx->req_count; i++) {
		free_req_context(ctx->req_ctxs[i]);
	}

	TEST_FREE(ctx);
}

static struct batch_context *alloc_batch_context(const struct test_desc *desc,
						 struct testcase_context *test_ctx)
{
	uint32_t i;
	struct batch_context *ctx;
	struct request_context *req_ctx;
	const struct test_testcase *testcase = test_ctx->testcase;
	uint32_t batch_depth = testcase->batch_depth;

	if (batch_depth > TEST_MAX_BATCH_DEPTH) {
		batch_depth = TEST_MAX_BATCH_DEPTH;
	} else if (batch_depth < 1) {
		batch_depth = 1;
	}

	ctx = (struct batch_context *) TEST_ALLOC(sizeof(*ctx));
	if (!ctx)
		return NULL;

	memset(ctx, 0, sizeof(*ctx));
	ctx->desc = desc;
	ctx->test_ctx = test_ctx;
	osal_atomic_init(&ctx->batch_state, BATCH_STATE_INIT);
	osal_atomic_init(&ctx->cb_count, 0);

	for (i = 0; i < batch_depth; i++) {
		req_ctx = alloc_req_context();
		if (!req_ctx) {
			goto error;
		}
		req_ctx->batch_ctx = ctx;
		ctx->req_ctxs[i] = req_ctx;
		ctx->req_count = i+1;
	}

	return ctx;

error:
	free_batch_context(ctx);
	return NULL;
}

static void init_batch_context(struct batch_context *ctx,
			       struct worker_context *work_ctx)
{
	osal_atomic_set(&ctx->batch_state, BATCH_STATE_INIT);
	osal_atomic_set(&ctx->cb_count, 0);
	ctx->worker_id = work_ctx->worker_id;
	ctx->poll_fn = NULL;
	ctx->poll_ctx = NULL;
	ctx->start_time = osal_get_clock_nsec();
	memset(&ctx->stats, 0, sizeof(ctx->stats));
	copy_vars(work_ctx->test_ctx->vars, ctx->vars);
}

static int worker_loop(void *param)
{
	struct worker_context *ctx = (struct worker_context *) param;
	struct batch_context *batch_ctx;

	while (!osal_thread_should_stop(&ctx->worker_thread)) {
		batch_ctx = worker_queue_dequeue(ctx->submit_q);
		if (batch_ctx) {
			run_testcase_batch(batch_ctx);
		}
		osal_yield();
	}

	return 0;
}

static void free_worker_queue(struct worker_queue *q)
{
	if (q) {
		TEST_FREE(q);
	}
}

static struct worker_queue *alloc_worker_queue(uint32_t max_entries)
{
	struct worker_queue *q;

	q = TEST_ALLOC(sizeof(*q) + (max_entries * sizeof(struct batch_context *)));
	if (q) {
		osal_atomic_init(&q->lock, 0);
		q->count = 0;
		q->head = 0;
		q->tail = 0;
		q->max_count = max_entries;
	}

	return q;
}

static void free_worker_context(struct worker_context *ctx)
{
	if (osal_thread_is_running(&ctx->worker_thread)) {
		osal_thread_stop(&ctx->worker_thread);
	}
	free_worker_queue(ctx->submit_q);
	free_worker_queue(ctx->complete_q);
	free_worker_queue(ctx->poll_q);

	TEST_FREE(ctx);
}

static struct worker_context *alloc_worker_context(const struct test_desc *desc,
						   struct testcase_context *test_ctx,
						   int core_id)
{
	int err;
	struct worker_context *ctx;

	ctx = (struct worker_context *) TEST_ALLOC(sizeof(*ctx));
	if (!ctx)
		return NULL;

	memset(ctx, 0, sizeof(*ctx));
	ctx->desc = desc;
	ctx->test_ctx = test_ctx;
	ctx->submit_q = alloc_worker_queue(TEST_MAX_BATCH_COUNT_PER_CORE);
	ctx->complete_q = alloc_worker_queue(TEST_MAX_BATCH_COUNT_PER_CORE);
	ctx->poll_q = alloc_worker_queue(TEST_MAX_BATCH_COUNT_PER_CORE);
	if (!ctx->submit_q || !ctx->complete_q || !ctx->poll_q) {
		goto error;
	}

	err = osal_thread_create(&ctx->worker_thread, worker_loop, ctx);
	if (err)
		goto error;
	err = osal_thread_bind(&ctx->worker_thread, core_id);
	if (err)
		goto error;

	return ctx;

error:
	free_worker_context(ctx);
	return NULL;
}

static void free_testcase_context(struct testcase_context *ctx)
{
	int i;
	struct batch_context *batch_ctx;

	if (!ctx)
		return;

	if (ctx->batch_ctx_freelist) {
		while ((batch_ctx = worker_queue_dequeue(ctx->batch_ctx_freelist))) {
			free_batch_context(batch_ctx);
		}
	}
	for (i = 0; i < ctx->worker_count; i++) {
		free_worker_context(ctx->worker_ctxs[i]);
	}
	TEST_FREE(ctx);
}

static struct testcase_context *alloc_testcase_context(const struct test_desc *desc,
						       const struct test_testcase *testcase)
{
	struct testcase_context *test_ctx;
	struct batch_context *batch_ctx;
	struct worker_context *worker_ctx;
	int i, max_core_count, core_id;
	uint32_t worker_count = 0;

	test_ctx = (struct testcase_context *) TEST_ALLOC(sizeof(*test_ctx));
	if (!test_ctx) {
		return NULL;
	}

	memset(test_ctx, 0, sizeof(*test_ctx));
	test_ctx->desc = desc;
	test_ctx->testcase = testcase;
	osal_atomic_init(&test_ctx->stats_lock, 0);
	test_ctx->output_file_tbl = test_get_output_file_table();

	max_core_count = osal_get_core_count();
	if (max_core_count > TEST_MAX_CORE_COUNT) {
		max_core_count = TEST_MAX_CORE_COUNT;
	} else if (max_core_count < 1) {
		max_core_count = 1;
	}

	/* Allocate freelist and fill it with batch_ctx entries */
	test_ctx->batch_ctx_freelist = alloc_worker_queue(max_core_count*TEST_MAX_BATCH_COUNT_PER_CORE);
	if (!test_ctx->batch_ctx_freelist) {
		goto error;
	}
	for (core_id = 0; core_id < max_core_count; core_id++) {
		if ((desc->cpu_mask & (1 << core_id)) == 0) {
			continue;
		}
		for (i = 0; i < TEST_MAX_BATCH_COUNT_PER_CORE; i++) {
			batch_ctx = alloc_batch_context(desc, test_ctx);
			if (!batch_ctx) {
				goto error;
			}
			worker_queue_enqueue(test_ctx->batch_ctx_freelist, batch_ctx);
		}

		worker_ctx = alloc_worker_context(desc, test_ctx, core_id);
		if (!worker_ctx) {
			goto error;
		}
		worker_ctx->worker_id = worker_count;
		test_ctx->worker_ctxs[worker_count++] = worker_ctx;
		test_ctx->worker_count = worker_count;
	}

	return test_ctx;

error:
	free_testcase_context(test_ctx);
	return NULL;
}

static pnso_error_t start_worker_thread(struct worker_context *ctx)
{
	if (ctx == NULL)
		return EINVAL;

	return osal_thread_start(&ctx->worker_thread);
}

static pnso_error_t start_worker_threads(struct testcase_context *ctx)
{
	int i;
	pnso_error_t err = 0;

	for (i = 0; i < ctx->worker_count; i++) {
		err = start_worker_thread(ctx->worker_ctxs[i]);
		if (err)
			break;
	}

	return err;
}

static bool need_rate_limit(struct testcase_context *ctx)
{
	uint64_t in_rate, out_rate;

	if (ctx->desc->limit_rate == 0)
		return false;

	in_rate = calculate_bytes_per_sec(ctx->stats.io_stats[0].bytes, ctx->stats.elapsed_time);
	out_rate = calculate_bytes_per_sec(ctx->stats.io_stats[1].bytes, ctx->stats.elapsed_time);

	return (in_rate > ctx->desc->limit_rate || out_rate > ctx->desc->limit_rate);
}

#define TESTCASE_IDLE_LOOP_TIMEOUT (5 * OSAL_NSEC_PER_SEC)
#define TESTCASE_LOOP_RESOLUTION_MASK ((1 << 8) - 1)

static pnso_error_t pnso_test_run_testcase(const struct test_desc *desc,
					   const struct test_testcase *testcase)
{
	pnso_error_t err = PNSO_OK;
	struct testcase_context *ctx;
	struct worker_context *worker_ctx;
	struct batch_context *batch_ctx;
	uint64_t batch_completion_count = 0;
	uint64_t batch_submit_count = 0;
	uint64_t req_submit_count = 0;
	uint64_t req_completion_count = 0;
	uint64_t fail_count = 0;
	uint64_t last_active_ts = 0;
	uint64_t cur_ts;
	uint64_t idle_time = 0;
	uint64_t rate_limit_loop_count = 0;
	uint64_t max_idle_time = 0;
	uint64_t loop_count = 0;
	uint32_t next_status_time;
	int worker_id;

	ctx = alloc_testcase_context(desc, testcase);
	if (!ctx) {
		PNSO_LOG_ERROR("Cannot allocate context for testcase %u\n",
			       testcase->node.idx);
		return ENOMEM;
	}

	ctx->vars[TEST_VAR_BLOCK_SIZE] = desc->init_params.block_size;
	ctx->vars[TEST_VAR_ITER] = 0;

	/* Start worker threads */
	if (start_worker_threads(ctx) != PNSO_OK) {
		PNSO_LOG_WARN("Unable to start all worker threads\n");
		/* continue execution, since some threads may be functional */
	} else {
		PNSO_LOG_INFO("Starting %u worker threads\n", ctx->worker_count);
	}

	/* Submit batches to each worker thread in turn, until done */
	worker_id = 0;
	last_active_ts = osal_get_clock_nsec();
	ctx->start_time = osal_get_clock_nsec();
	next_status_time = desc->status_interval;
	while (req_completion_count < testcase->repeat) {
		loop_count++;
		worker_ctx = ctx->worker_ctxs[worker_id];

		/* Polling mode */
		if (testcase->sync_mode == SYNC_MODE_POLL) {
			batch_ctx = worker_queue_dequeue(worker_ctx->poll_q);
			if (batch_ctx) {
				if (batch_ctx->poll_fn(batch_ctx->poll_ctx)
						!= PNSO_OK) {
					/* Not ready yet, re-enqueue */
					worker_queue_enqueue(worker_ctx->poll_q, batch_ctx);
				}
				/* else completion handler will put entry on complete_q */
			}
		}

		if ((batch_ctx = worker_queue_dequeue(worker_ctx->complete_q))) {
			/* process finished batch, and restore to freelist */
			run_batch_validation(batch_ctx);
			calculate_completion_stats(batch_ctx);
			aggregate_testcase_stats(&ctx->stats, &batch_ctx->stats,
					osal_get_clock_nsec() - ctx->start_time);
			worker_queue_enqueue(ctx->batch_ctx_freelist, batch_ctx);
			last_active_ts = osal_get_clock_nsec();
			batch_completion_count++;
			req_completion_count += testcase->batch_depth;
		} else if (osal_atomic_read(&g_shutdown) ||
			   (req_submit_count >= testcase->repeat)) {
			/* No more requests to submit */
		} else if (need_rate_limit(ctx)) {
			/* skip idle timeout during active rate limiting */
//			last_active_ts = osal_get_clock_nsec();
			rate_limit_loop_count++;
		} else if (!worker_queue_is_full(worker_ctx->submit_q)) {
			/* submit new batch request */
			batch_ctx = worker_queue_dequeue(ctx->batch_ctx_freelist);
			if (batch_ctx) {
				ctx->vars[TEST_VAR_ITER]++;
				init_batch_context(batch_ctx, worker_ctx);

				if (worker_queue_enqueue(worker_ctx->submit_q, batch_ctx)) {
					batch_submit_count++;
					req_submit_count += testcase->batch_depth;
					last_active_ts = osal_get_clock_nsec();
				} else {
					fail_count++;
					worker_queue_enqueue(ctx->batch_ctx_freelist, batch_ctx);
				}
			}
		}

		/* Operations limited to run occasionally */
		if ((loop_count & TESTCASE_LOOP_RESOLUTION_MASK) == 0) {
			cur_ts = osal_get_clock_nsec();

			/* Update elapsed_time */
			ctx->stats.elapsed_time = cur_ts - ctx->start_time;

			/* Break out if nothing's happening */
			idle_time = cur_ts - last_active_ts;
			if (idle_time > max_idle_time) {
				max_idle_time = idle_time;
			}
			if (idle_time >= TESTCASE_IDLE_LOOP_TIMEOUT) {
				break;
			}

			/* Check whether it's time to output stats */
			if (desc->status_interval) {
				if ((ctx->stats.elapsed_time / OSAL_NSEC_PER_SEC) > next_status_time) {
					pnso_test_stats_to_yaml(testcase,
						(uint64_t*) (&ctx->stats),
						testcase_stats_names,
						TESTCASE_STATS_COUNT, false);
					next_status_time += desc->status_interval;
				}
			}
		}

		/* Iterate workers on each loop */
		if (++worker_id >= ctx->worker_count)
			worker_id = 0;

		osal_yield();
	}

	/* Final tally for stats */
	ctx->stats.elapsed_time = osal_get_clock_nsec() - ctx->start_time;
	pnso_test_stats_to_yaml(testcase,
				(uint64_t*) (&ctx->stats),
				testcase_stats_names,
				TESTCASE_STATS_COUNT,
				true);

	PNSO_LOG_DEBUG("Testcase %u %s: status %d, elapsed_time %lu, "
		       "req_submit_count %lu, max_idle_time_ns %lu, "
		       "batch_submit_count %lu, batch_completion_count %lu, "
		       "rate_limit_loops %lu\n",
		       testcase->node.idx, testcase->name, err,
		       ctx->stats.elapsed_time / OSAL_NSEC_PER_MSEC,
		       req_submit_count, max_idle_time,
		       batch_submit_count, batch_completion_count,
		       rate_limit_loop_count);

	/* Print summary of validation success/failure */
#if 0
	if (testcase->validations.head) {
		struct test_node *node;

		FOR_EACH_NODE(testcase->validations) {
			struct test_validation *validation =
				(struct test_validation *) node;
			PNSO_LOG_INFO("  validation %u(%s): successes=%lu, failures=%lu\n",
				      validation->node.idx,
				      validation->name,
				      validation->rt_success_count,
				      validation->rt_failure_count);
		}
	}

	/* Print runtime summary */
	PNSO_LOG_INFO("  runtime: total %lu ms\n",
		      ctx->stats.elapsed_time / OSAL_NSEC_PER_MSEC);
#endif

	free_testcase_context(ctx);

	return err;
}

pnso_error_t pnso_test_run_all(struct test_desc *desc)
{
	pnso_error_t err = PNSO_OK;
	struct test_node *node;

	if (desc->node.type != NODE_ROOT) {
		PNSO_LOG_ERROR("Invalid test description\n");
		return EINVAL;
	}

	err = pnso_init(&desc->init_params);
	if (err != PNSO_OK) {
		PNSO_LOG_ERROR("pnso_init failed with rc = %d\n", err);
		return err;
	}
#ifdef PNSO_SIM
	err = pnso_sim_thread_init(osal_get_coreid());
	if (err != PNSO_OK) {
		PNSO_LOG_ERROR("pnso_thread_init failed with rc = %d\n", err);
		return err;
	}
#endif

	/* Initialize compression header formats */
	FOR_EACH_NODE(desc->cp_hdrs) {
		struct test_cp_header *cp_hdr = (struct test_cp_header *) node;
		err = pnso_register_compression_header_format(&cp_hdr->fmt,
							      cp_hdr->node.idx);
		if (err != PNSO_OK) {
			PNSO_LOG_ERROR("pnso_register_compression_header_format"
				       "(%u) failed with rc = %d\n",
				       cp_hdr->node.idx, err);
			return err;
		}
	}

	/* Initialize compression algo mapping */
	FOR_EACH_NODE(desc->cp_hdr_map) {
		struct test_cp_hdr_mapping *mapping = (struct test_cp_hdr_mapping *) node;
		err = pnso_add_compression_algo_mapping(mapping->pnso_algo,
							mapping->hdr_algo);
		if (err != PNSO_OK) {
			PNSO_LOG_ERROR("pnso_add_compression_algo_mapping"
				       "(%u,%u) failed with rc = %d\n",
				       mapping->pnso_algo, mapping->hdr_algo,
				       err);
			return err;
		}
	}

	/* Initialize key store */
	FOR_EACH_NODE(desc->crypto_keys) {
		struct test_crypto_key *key = (struct test_crypto_key *) node;
		err = pnso_set_key_desc_idx(key->key1, key->key2, key->key1_len, key->node.idx);
		if (err != PNSO_OK) {
			PNSO_LOG_ERROR("pnso_set_key_desc_idx(%u) failed with rc = %d\n",
				       key->node.idx, err);
			return err;
		}
#ifdef PNSO_SIM
		if (key->key1_len) {
			char *tmp_key = NULL;
			uint32_t tmp_key_size = 0;
			sim_get_key_desc_idx((void**)&tmp_key,
					(void**)&tmp_key, &tmp_key_size, key->node.idx);
			if (tmp_key_size != key->key1_len) {
				PNSO_LOG_ERROR("key size %u doesn't match expected %u\n",
					       tmp_key_size, key->key1_len);
				return PNSO_ERR_XTS_WRONG_KEY_TYPE;
			}
			if (0 != memcmp(tmp_key, key->key1, tmp_key_size)) {
				PNSO_LOG_ERROR("key data doesn't match\n");
				return PNSO_ERR_XTS_KEY_NOT_REGISTERED;
			}
		}
#endif
	}

	/* Run all testcases, one by one */
	FOR_EACH_NODE(desc->tests) {
		if (((const struct test_testcase *) node)->repeat) {
			err = pnso_test_run_testcase(desc,
					(const struct test_testcase *) node);
			if (err != PNSO_OK) {
				/* TODO: continue? */
				break;
			}
		}
	}

	/* Delete output files */
	if (desc->delete_output_files) {
		struct test_node_list *list;
		struct test_file_table *file_tbl = test_get_output_file_table();
		int bucket;
		int success_count = 0;
		int fail_count = 0;

		FOR_EACH_TABLE_BUCKET(file_tbl->table) {
			FOR_EACH_NODE(*list) {
				struct test_node_file *fname_node =
					(struct test_node_file *) node;
				if (test_delete_file(fname_node->filename) ==
				    PNSO_OK) {
					success_count++;
				} else {
					fail_count++;
				}
			}
		}
		PNSO_LOG_INFO("Deleted %d out of %d output files.\n",
			      success_count, success_count + fail_count);
	} else {
		PNSO_LOG_INFO("Skipping deletion of '%s*%s' output files.\n",
			      desc->output_file_prefix, desc->output_file_suffix);
	}

	return PNSO_OK;
}
