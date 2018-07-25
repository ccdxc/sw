/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#if 0
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#endif

#include "osal_stdtypes.h"
#include "osal_errno.h"
#include "osal_mem.h"
#include "osal_sys.h"
#include "osal_atomic.h"
#include <time.h>

#include "pnso_test.h"
#include "pnso_test_parse.h"

#ifdef PNSO_SIM
#include "sim.h"
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

static uint32_t get_max_output_len_by_type(uint16_t svc_type,
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
	struct run_context *run_ctx;
	struct test_desc *desc;

	uint8_t *input_buffer;

	pnso_poll_fn_t poll_fn;
	void *poll_ctx;

	/* MUST keep these 2 in order, due to zero-length array */
	struct pnso_service_request svc_req;
	struct pnso_service req_svcs[PNSO_SVC_TYPE_MAX];

	/* MUST keep these 2 in order, due to zero-length array */
	struct pnso_service_result svc_res;
	struct pnso_service_status res_statuses[PNSO_SVC_TYPE_MAX];

	/* MUST keep these 2 in order, due to zero-length array */
	struct pnso_buffer_list src_buflist;
	struct pnso_flat_buffer src_bufs[MAX_INPUT_BUF_COUNT];

	pnso_error_t res_rc;
};

struct run_context {
	struct test_desc *desc;
	osal_atomic_int_t cb_count;
	uint64_t runtime; /* TODO: fix calculation */

	uint32_t req_count;
	struct request_context *req_ctxs[TEST_MAX_BATCH_DEPTH];

	uint32_t vars[TEST_VAR_MAX];
};

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

/* Assumes dst is long enough to contain string */
static pnso_error_t construct_filename(struct run_context *ctx,
				       char *dst, const char *src)
{
	const char *orig_src = src;
	char *orig_dst = dst;
	const char *var_name, *tmp;
	int var_len;
	uint32_t var_id;

	*dst = '\0';

	/* Prepend the global file prefix */
	if (ctx->desc->output_file_prefix[0]) {
		strcpy(dst, ctx->desc->output_file_prefix);
		dst += strlen(ctx->desc->output_file_prefix);
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
			dst += sprintf(dst, "%u", ctx->vars[var_id]);
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
				    struct test_input_desc *input_desc,
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

static struct test_node_file *lookup_file_node(struct test_node_table *table,
					       const char *filename)
{
	struct test_node_file search_fnode;
	struct test_node_file *fnode;

	memset(&search_fnode, 0, sizeof(search_fnode));
	search_fnode.node.type = NODE_FILE;
	search_fnode.node.idx = str_hash(filename);
	strncpy(search_fnode.filename, filename, TEST_MAX_FULL_PATH_LEN);

	fnode = (struct test_node_file *) test_node_table_lookup(table,
							&search_fnode.node,
							cmp_file_node);
	return fnode;
}

static pnso_error_t insert_unique_file_node(struct test_node_table *table,
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

	fnode = (struct test_node_file *) test_node_table_lookup(table,
					&search_fnode.node, cmp_file_node);
	if (fnode) {
		/* Found it, just update checksum and size */
		goto update;
	}

	fnode = (struct test_node_file *) test_node_alloc(sizeof(*fnode),
							  NODE_FILE);
	if (!fnode) {
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
		test_node_table_insert(table, &fnode->node);
	}

	return PNSO_OK;
}

static void output_results(struct request_context *req_ctx,
			   struct test_svc_chain *svc_chain)
{
	uint32_t i;
	struct test_node *node;
	bool wrote_file = false;
	uint64_t checksum = 0;
	uint64_t padded_checksum = 0;
	uint32_t file_size = 0;
	uint32_t padded_size = 0;
	pnso_error_t err;
	uint32_t blk_sz = req_ctx->desc->init_params.block_size;
	char output_path[TEST_MAX_FULL_PATH_LEN] = "";

	/* output to file */
	i = 0;
	FOR_EACH_NODE(svc_chain->svcs) {
		struct test_svc *svc = (struct test_svc *) node;
		struct pnso_service_status *svc_status = &req_ctx->svc_res.svc[i];
		if (svc->output_path[0]) {
			err = construct_filename(req_ctx->run_ctx, output_path,
						 svc->output_path);
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
					&req_ctx->desc->output_file_table,
					output_path, checksum, file_size,
					padded_checksum, padded_size);
			}
		}
		i++;
	}
}

static void testcase_completion_cb(void *cb_ctx, struct pnso_service_result *svc_res)
{
	struct request_context *req_ctx = (struct request_context *) cb_ctx;

	osal_atomic_fetch_add(&req_ctx->run_ctx->cb_count, 1);
}

static pnso_error_t test_submit_request(struct request_context *req_ctx,
					uint16_t sync_mode,
					bool is_batched,
					bool flush)
{
	pnso_error_t rc = PNSO_OK;
	completion_cb_t req_cb;
	void *req_cb_ctx;
	pnso_poll_fn_t *req_poll_fn;
	void **req_poll_ctx;

	switch (sync_mode) {
	case SYNC_MODE_SYNC:
		req_cb = NULL;
		req_cb_ctx = NULL;
		req_poll_fn = NULL;
		req_poll_ctx = NULL;
		break;
	case SYNC_MODE_POLL:
		req_cb = testcase_completion_cb;
		req_cb_ctx = req_ctx;
		req_poll_fn = &req_ctx->poll_fn;
		req_poll_ctx = &req_ctx->poll_ctx;
		break;
	case SYNC_MODE_ASYNC:
	default:
		req_cb = testcase_completion_cb;
		req_cb_ctx = req_ctx;
		req_poll_fn = NULL;
		req_poll_ctx = NULL;
		break;
	}

	if (is_batched) {
		pnso_error_t flush_rc;

		rc = pnso_add_to_batch(&req_ctx->svc_req, &req_ctx->svc_res);
		if (flush) {
			/* Flush regardless of batch error */
			flush_rc = pnso_flush_batch(req_cb, req_cb_ctx,
						    req_poll_fn, req_poll_ctx);
			if (flush_rc != PNSO_OK) {
				rc = flush_rc;
			}
		}
	} else {
		rc = pnso_submit_request(&req_ctx->svc_req,
					 &req_ctx->svc_res,
					 req_cb, req_cb_ctx,
					 req_poll_fn, req_poll_ctx);
	}

	return rc;
}

static pnso_error_t run_testcase_svc_chain(struct request_context *req_ctx,
					   struct test_testcase *testcase,
					   struct test_svc_chain *svc_chain,
					   uint32_t batch_iter,
					   uint32_t batch_count)
{
	struct run_context *ctx = req_ctx->run_ctx;
	pnso_error_t err = PNSO_OK;
	uint32_t input_len, remain_len, buf_len;
	uint32_t min_block, max_block, block_count;
	uint32_t i;
	clock_t clock_start, clock_finish;
	uint8_t *buf;
	struct test_node *node;
	char input_path[TEST_MAX_FULL_PATH_LEN] = "";

	/* construct input filename */
	if (svc_chain->input.pathname[0]) {
		err = construct_filename(ctx, input_path,
					 svc_chain->input.pathname);
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
			input_len = ctx->desc->init_params.block_size;
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
		min_block = ctx->desc->init_params.block_size;
	}
	if (!max_block) {
		max_block = ctx->desc->init_params.block_size;
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

	/* Execute */
	clock_start = clock();

	req_ctx->res_rc = test_submit_request(req_ctx, testcase->sync_mode,
					      (batch_count > 1),
					      (batch_iter == batch_count - 1));

	clock_finish = clock();
	ctx->runtime += (uint64_t) (clock_finish - clock_start);

	return err;
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

static pnso_error_t run_data_validation(struct run_context *ctx,
					struct test_testcase *testcase,
					struct test_validation *validation)
{
	pnso_error_t err = PNSO_OK;
	struct test_node_file *fnode1, *fnode2;
	char path1[TEST_MAX_FULL_PATH_LEN] = "";
	char path2[TEST_MAX_FULL_PATH_LEN] = "";
	int cmp = 0;

	/* construct dynamic pathnames */
	if (validation->file1[0]) {
		err = construct_filename(ctx, path1, validation->file1);
	}
	if (err == PNSO_OK && validation->file2[0]) {
		err = construct_filename(ctx, path2, validation->file2);
	}
	if (err != PNSO_OK) {
		validation->rt_failure_count++;
		return err;
	}

	switch (validation->type) {
	case VALIDATION_DATA_COMPARE:
		/* Try to compare just local metadata */
		fnode1 = lookup_file_node(&ctx->desc->output_file_table, path1);
		fnode2 = lookup_file_node(&ctx->desc->output_file_table, path2);
		if (fnode1 && fnode2) {
			cmp = cmp_file_node_data(fnode1, fnode2);
		} else {
			/* Metadata not available, do full file compare */
			cmp = test_compare_files(path1, path2,
						 validation->offset,
						 validation->len);
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

	if (err == PNSO_OK) {
		if (is_compare_true(validation->cmp_type, cmp)) {
			validation->rt_success_count++;
		} else {
			validation->rt_failure_count++;
		}
	} else {
		validation->rt_failure_count++;
	}
	return err;
}

static pnso_error_t run_retcode_validation(struct request_context *req_ctx,
					   struct test_testcase *testcase,
					   struct test_validation *validation)
{
	pnso_error_t err = PNSO_OK;
	size_t i;
	int cmp = 0;

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
	if (err == PNSO_OK) {
		if (is_compare_true(validation->cmp_type, cmp)) {
			validation->rt_success_count++;
		} else {
			validation->rt_failure_count++;
		}
	} else {
		validation->rt_failure_count++;
	}
	return err;
}

static pnso_error_t run_testcase_batch(struct run_context *ctx,
			struct test_testcase *testcase, uint32_t batch_count,
			uint32_t *req_submit_count)
{
	pnso_error_t err = PNSO_OK;
	struct test_node *node;
	struct test_svc_chain *svc_chain;
	struct request_context *req_ctx;
	uint32_t i, j;

	ctx->vars[TEST_VAR_ID] = testcase->node.idx;

	/* Run each svc_chain */
	for (i = 0; i < testcase->svc_chain_count; i++) {
		NODE_FIND_ID(ctx->desc->svc_chains, testcase->svc_chains[i]);
		svc_chain = (struct test_svc_chain *) node;
		if (!svc_chain) {
			PNSO_LOG_ERROR("Svc_chain %u not found for testcase %u\n",
				      testcase->svc_chains[i], testcase->node.idx);
			return EINVAL;
		}

		ctx->vars[TEST_VAR_CHAIN] = svc_chain->node.idx;

		osal_atomic_set(&ctx->cb_count, 0);
		for (j = 0; j < batch_count; j++) {
			req_ctx = ctx->req_ctxs[j];
			if (!req_ctx) {
				PNSO_LOG_ERROR("Request context %u not found "
					       "for testcase %u\n",
					       j, testcase->node.idx);
				return EINVAL;
			}
			if (run_testcase_svc_chain(req_ctx, testcase, svc_chain,
						   j, batch_count) == PNSO_OK &&
			    req_ctx->res_rc == PNSO_OK) {
				(*req_submit_count)++;
			}
		}

		/* Poll for completion, if necessary */
		switch (testcase->sync_mode) {
		case SYNC_MODE_SYNC:
			/* Nothing to do */
			break;
		case SYNC_MODE_POLL:
			if (req_ctx->poll_fn) {
				while (!req_ctx->poll_fn(req_ctx->poll_ctx)) {
					osal_yield();
				}
			}
			break;
		case SYNC_MODE_ASYNC:
		default:
			/* wait for completion handler to be called */
			while (osal_atomic_read(&ctx->cb_count) == 0) {
				osal_yield();
			}
			break;
		}

		/* run the corresponding result output and retcode validation */
		for (j = 0; j < batch_count; j++) {
			req_ctx = ctx->req_ctxs[j];

			output_results(req_ctx, svc_chain);

			FOR_EACH_NODE(testcase->validations) {
				struct test_validation *validation =
					(struct test_validation *) node;
				if (validation->type ==	VALIDATION_RETCODE_COMPARE &&
				    (validation->svc_chain_idx == 0 ||
				     validation->svc_chain_idx == testcase->svc_chains[i])) {
					run_retcode_validation(req_ctx, testcase, validation);
				}
			}
		}
	}

	/* run file validations */
	FOR_EACH_NODE(testcase->validations) {
		struct test_validation *validation =
				(struct test_validation *) node;
		if (validation->type != VALIDATION_RETCODE_COMPARE) {
			run_data_validation(ctx, testcase, validation);
		}
	}

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

	TEST_FREE(ctx);
}

static struct run_context *alloc_run_context(struct test_desc *desc,
					     struct test_testcase *testcase)
{
	uint32_t i;
	struct run_context *ctx;
	struct request_context *req_ctx;
	uint32_t batch_depth = testcase->batch_depth;

	if (batch_depth > TEST_MAX_BATCH_DEPTH) {
		batch_depth = TEST_MAX_BATCH_DEPTH;
	} else if (batch_depth < 1) {
		batch_depth = 1;
	}

	ctx = (struct run_context *) TEST_ALLOC(sizeof(*ctx));
	if (ctx) {
		memset(ctx, 0, sizeof(*ctx));
		ctx->desc = desc;
		osal_atomic_init(&ctx->cb_count, 0);

		for (i = 0; i < batch_depth; i++) {
			req_ctx = alloc_req_context();
			if (!req_ctx) {
				break;
			}
			req_ctx->desc = desc;
			req_ctx->run_ctx = ctx;
			ctx->req_ctxs[i] = req_ctx;
		}
		ctx->req_count = i;
		if (i == 0) {
			TEST_FREE(ctx);
			ctx = NULL;
		}
	}

	return ctx;
}

static void free_run_context(struct run_context *ctx)
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

static pnso_error_t pnso_test_run_testcase(struct test_desc *desc, struct test_testcase *testcase)
{
	pnso_error_t err = PNSO_OK;
	uint32_t iter;
	struct test_node *node;
	struct run_context *ctx;
	uint32_t batch_count;
	uint32_t req_submit_count = 0;

	ctx = alloc_run_context(desc, testcase);
	if (!ctx) {
		PNSO_LOG_ERROR("Cannot allocate run_context for testcase %u\n",
			       testcase->node.idx);
		return ENOMEM;
	}

	ctx->vars[TEST_VAR_BLOCK_SIZE] = desc->init_params.block_size;

	batch_count = testcase->batch_depth;
	for (iter = 0; iter < testcase->repeat; iter += batch_count) {
		ctx->vars[TEST_VAR_ITER] = iter;
		/* TODO: iter doesn't increment by 1 */
		if (iter + batch_count > testcase->repeat) {
			batch_count = testcase->repeat - iter;
		}
		err = run_testcase_batch(ctx, testcase, batch_count, &req_submit_count);
		if (err != PNSO_OK) {
			PNSO_LOG_ERROR("Testcase %u %s: Failed on iteration %u\n",
				       testcase->node.idx, testcase->name,
				       iter);
			break;
		}
	}

	if (err == PNSO_OK) {
		PNSO_LOG_INFO("Testcase %u %s:\n"
                              "  Successfully ran for %u iterations,"
			      " with %u reqs submitted\n",
			      testcase->node.idx, testcase->name, iter,
			      req_submit_count);
	}

	/* Print summary of validation success/failure */
	FOR_EACH_NODE(testcase->validations) {
		struct test_validation *validation =
				(struct test_validation *) node;
		PNSO_LOG_INFO("  validation %u(%s): successes=%u, failures=%u\n",
			      validation->node.idx,
			      validation->name,
			      validation->rt_success_count,
			      validation->rt_failure_count);
	}

	/* Print runtime summary */
	PNSO_LOG_INFO("  runtime: total %lu ms\n",
		      (ctx->runtime * 1000) / CLOCKS_PER_SEC);

	free_run_context(ctx);

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
				return -1; /* TODO */
			}
			if (0 != memcmp(tmp_key, key->key1, tmp_key_size)) {
				PNSO_LOG_ERROR("key data doesn't match\n");
				return -1; /* TODO */
			}
		}
#endif
	}

	/* Run all testcases, one by one */
	FOR_EACH_NODE(desc->tests) {
		if (((struct test_testcase *) node)->repeat) {
			err = pnso_test_run_testcase(desc,
					(struct test_testcase *) node);
			if (err != PNSO_OK) {
				/* TODO: continue? */
				break;
			}
		}
	}

	/* Delete output files */
	if (desc->delete_output_files) {
		struct test_node_list *list;
		int bucket;
		int success_count = 0;
		int fail_count = 0;

		FOR_EACH_TABLE_BUCKET(desc->output_file_table) {
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
