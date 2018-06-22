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

	/* Must be last */
	TEST_VAR_MAX
};

#define MAX_OUTPUT_BUF_LEN (64 * 1024)
#define DEFAULT_BUF_COUNT 16
#define MAX_BUF_COUNT 1024

struct run_context {
	struct test_desc *desc;
	uint8_t *input_buffer;
	uint8_t *output_buffer;
	uint64_t runtime;

	/* MUST keep these 2 in order, due to zero-length array */
	struct pnso_service_request svc_req;
	struct pnso_service req_svcs[PNSO_SVC_TYPE_MAX];

	/* MUST keep these 2 in order, due to zero-length array */
	struct pnso_service_result svc_res;
	struct pnso_service_status res_statuses[PNSO_SVC_TYPE_MAX];

	/* MUST keep these 2 in order, due to zero-length array */
	struct pnso_buffer_list src_buflist;
	struct pnso_flat_buffer src_bufs[MAX_BUF_COUNT];

	/* MUST keep these 2 in order, due to zero-length array */
	struct pnso_buffer_list dst_buflist;
	struct pnso_flat_buffer dst_bufs[DEFAULT_BUF_COUNT];

	pnso_error_t res_rc;

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
static pnso_error_t construct_filename(struct run_context *ctx, char *dst, const char *src)
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
	size_t i, j;
	uint8_t *dst;

	for (i = 0; i < buflist->count; i++) {
		if (!buflist->buffers[i].len) {
			continue;
		}

		dst = (uint8_t *) buflist->buffers[i].buf;
		for (j = 0; j < buflist->buffers[i].len; j++) {
			dst[j] = (uint8_t) pat[j % pat_len];
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

static pnso_error_t run_testcase_svc_chain(struct run_context *ctx,
					   struct test_testcase *testcase,
					   struct test_svc_chain *svc_chain)
{
	pnso_error_t err = PNSO_OK;
	uint32_t input_len, remain_len, buf_len;
	uint32_t min_block, max_block, block_count;
	uint32_t i;
	clock_t clock_start, clock_finish;
	uint8_t *buf;
	struct test_node *node;
	char input_path[MAX_FILE_PREFIX_LEN + TEST_MAX_PATH_LEN + 1] = "";
	char output_path[MAX_FILE_PREFIX_LEN + TEST_MAX_PATH_LEN + 1] = "";

	ctx->vars[TEST_VAR_CHAIN] = svc_chain->node.idx;

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
				PNSO_LOG_ERROR("Invalid input file %s\n", input_path);
				return EINVAL;
			}
		} else if (svc_chain->input.pattern[0]) {
			input_len = strlen(svc_chain->input.pattern);
		} else {
			input_len = ctx->desc->init_params.block_size;
		}
	}
	ctx->input_buffer = TEST_ALLOC(input_len);
	if (!ctx->input_buffer) {
		PNSO_LOG_ERROR("Failed to alloc %u bytes for input_buffer\n",
			       input_len);
		return ENOMEM;
	}
	min_block = svc_chain->input.min_block_size;
	max_block = svc_chain->input.max_block_size;
	block_count = svc_chain->input.block_count;
	if (!block_count) {
		block_count = DEFAULT_BUF_COUNT;
	}
	if (!min_block) {
		min_block = ctx->desc->init_params.block_size;
	}
	if (!max_block) {
		max_block = ctx->desc->init_params.block_size;
	}
	if (max_block < min_block) {
		max_block = min_block;
	}
	if ((max_block * block_count) < input_len) {
		PNSO_LOG_ERROR("Cannot represent %u bytes input with %u byte blocks\n",
			       input_len, max_block);
		return EINVAL;
	}
	remain_len = input_len;
	buf = ctx->input_buffer; /* TODO */
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
		ctx->src_bufs[i].buf = (uint64_t) buf;
		ctx->src_bufs[i].len = buf_len;
		buf += buf_len;
		remain_len -= buf_len;
	}
	ctx->src_buflist.count = i;

	/* setup dest buffers */
	ctx->output_buffer = TEST_ALLOC(MAX_OUTPUT_BUF_LEN);
	if (!ctx->output_buffer) {
		PNSO_LOG_ERROR("Failed to alloc %u bytes for output_buffer\n",
			       MAX_OUTPUT_BUF_LEN);
		return ENOMEM;
	}
	ctx->dst_bufs[0].len = MAX_OUTPUT_BUF_LEN;
	ctx->dst_bufs[0].buf = (uint64_t) ctx->output_buffer;
	ctx->dst_buflist.count = 1;

	/* populate input buffer */
	err = test_read_input(input_path, &svc_chain->input, &ctx->src_buflist);
	if (err != PNSO_OK) {
		return err;
	}

	/* setup request */
	ctx->svc_req.sgl = &ctx->src_buflist;
	ctx->svc_req.num_services = svc_chain->num_services;
	i = 0;
	FOR_EACH_NODE(svc_chain->svcs) {
		struct test_svc *svc = (struct test_svc *) node;
		ctx->svc_req.svc[i] = svc->svc;
		switch (ctx->svc_req.svc[i].svc_type) {
		case PNSO_SVC_TYPE_COMPRESS:
			if ((!svc->svc.u.cp_desc.threshold_len ||
			     svc->svc.u.cp_desc.threshold_len > input_len) &&
			    input_len > svc->u.cpdc.threshold_delta) {
				ctx->svc_req.svc[i].u.cp_desc.threshold_len =
					input_len - svc->u.cpdc.threshold_delta;
#if 0
				PNSO_LOG_DEBUG("Using threshold_len of %u (input %u delta %u)\n",
			       		       ctx->svc_req.svc[i].u.cp_desc.threshold_len,
					       input_len, svc->u.cpdc.threshold_delta);
#endif
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
	ctx->svc_res.err = PNSO_OK;
	ctx->svc_res.num_services = svc_chain->num_services;
	i = 0;
	FOR_EACH_NODE(svc_chain->svcs) {
		struct test_svc *svc = (struct test_svc *) node;
		struct pnso_service_status *svc_status = &ctx->svc_res.svc[i];
		svc_status->svc_type = svc->svc.svc_type; /* TODO: needed? */
		if (svc->output_path[0]) {
			if (svc->svc.svc_type == PNSO_SVC_TYPE_HASH ||
			    svc->svc.svc_type == PNSO_SVC_TYPE_CHKSUM) {
				/* TODO */
				if (!svc_status->u.hash.tags) {
					svc_status->u.hash.tags = TEST_ALLOC(
						sizeof(struct pnso_hash_tag) *
						DEFAULT_BUF_COUNT);
					if (!svc_status->u.hash.tags) {
						PNSO_LOG_ERROR(
							"Out of memory for output tags\n");
						return ENOMEM;
					}
					svc_status->u.hash.num_tags = DEFAULT_BUF_COUNT;
				}
			} else if (!svc_status->u.dst.sgl) {
				svc_status->u.dst.sgl =
					test_alloc_buffer_list(
						DEFAULT_BUF_COUNT,
						MAX_OUTPUT_BUF_LEN);
				if (!svc_status->u.dst.sgl) {
					PNSO_LOG_ERROR(
						"Out of memory for output_buf\n");
					return ENOMEM;
				}
			}
		}
		i++;
	}


	/* Execute synchronously */
	clock_start = clock();
	ctx->res_rc = pnso_submit_request(&ctx->svc_req, &ctx->svc_res,
					  NULL, NULL, NULL, NULL);
	clock_finish = clock();
	ctx->runtime += (uint64_t) (clock_finish - clock_start);

	/* output to file */
	i = 0;
	FOR_EACH_NODE(svc_chain->svcs) {
		struct test_svc *svc = (struct test_svc *) node;
		struct pnso_service_status *svc_status = &ctx->svc_res.svc[i];
		if (svc->output_path[0]) {
			err = construct_filename(ctx, output_path,
						 svc->output_path);
			if (err != PNSO_OK) {
				break;
			}
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
					err = test_write_file(output_path,
							      &buflist.bl,
							      buflist.b.len,
							      svc->output_flags);
				}
			} else if (svc_status->u.dst.sgl) {
				err = test_write_file(output_path,
						      svc_status->u.dst.sgl,
						      svc_status->u.dst.data_len,
						      svc->output_flags);
			}
			if (err != PNSO_OK) {
				PNSO_LOG_ERROR("Cannot write data to %s\n",
					       output_path);
				break;
			}
		}
		i++;
	}

	return err;
}

static pnso_error_t run_data_validation(struct run_context *ctx,
					struct test_testcase *testcase,
					struct test_validation *validation)
{
	pnso_error_t err = PNSO_OK;
	char path1[MAX_FILE_PREFIX_LEN + TEST_MAX_PATH_LEN + 1] = "";
	char path2[MAX_FILE_PREFIX_LEN + TEST_MAX_PATH_LEN + 1] = "";

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
	case VALIDATION_DATA_EQUAL:
		if (test_compare_files(path1, path2, validation->offset,
				       validation->len)) {
			err = -1;
		}
		break;
	case VALIDATION_SIZE_EQUAL:
		if (validation->len) {
			/* Test static length */
			if (test_file_size(path1) != validation->len) {
				err = -1;
			}
		} else {
			/* Compare size of 2 files */
			if (test_file_size(path1) != test_file_size(path2)) {
				err = -1;
			}
		}
		break;
	default:
		PNSO_LOG_ERROR("Invalid validation type %u\n",
			       validation->type);
		break;
	}

	if (err == PNSO_OK) {
		validation->rt_success_count++;
	} else {
		validation->rt_failure_count++;
	}
	return err;
}

static pnso_error_t run_retcode_validation(struct run_context *ctx,
					   struct test_testcase *testcase,
					   struct test_svc_chain *svc_chain,
					   struct test_validation *validation)
{
	pnso_error_t err = PNSO_OK;
	size_t i;

	if (ctx->svc_res.err != validation->retcode) {
		err = -1;
		goto done;
	}

	if (ctx->svc_res.num_services < validation->svc_count) {
		err = -1;
		goto done;
	}

	for (i = 0; i < validation->svc_count; i++) {
		if (ctx->svc_res.svc[i].err != validation->svc_retcodes[i]) {
			err = -1;
			break;
		}
	}

done:
	if (err == PNSO_OK) {
		validation->rt_success_count++;
	} else {
		validation->rt_failure_count++;
	}
	return err;
}

static pnso_error_t run_testcase_once(struct run_context *ctx, struct test_testcase *testcase)
{
	pnso_error_t err = PNSO_OK;
	struct test_node *node;
	struct test_svc_chain *svc_chain;
	size_t i;

	ctx->vars[TEST_VAR_ID] = testcase->node.idx;
	
	/* Run each svc_chain */
	for (i = 0; i < testcase->svc_chain_count && !err; i++) {
		NODE_FIND_ID(ctx->desc->svc_chains, testcase->svc_chains[i]);
		svc_chain = (struct test_svc_chain *) node;
		if (!svc_chain) {
			PNSO_LOG_ERROR("Svc_chain %u not found for testcase %u\n",
				       testcase->svc_chains[i], testcase->node.idx);
			err = EINVAL;
		} else {
			err = run_testcase_svc_chain(ctx, testcase, svc_chain);
		}

		/* run the corresponding retcode validation */
		FOR_EACH_NODE(testcase->validations) {
			struct test_validation *validation =
				(struct test_validation *) node;
			if (validation->type ==	VALIDATION_RETCODE_EQUAL &&
			    validation->svc_chain_idx == testcase->svc_chains[i]) {
				run_retcode_validation(ctx, testcase,
					svc_chain, validation);
			}
		}
	}

	/* run file validations */
	FOR_EACH_NODE(testcase->validations) {
		struct test_validation *validation =
				(struct test_validation *) node;
		if (validation->type != VALIDATION_RETCODE_EQUAL) {
			run_data_validation(ctx, testcase, validation);
		}
	}

	return err;
}

static struct run_context *alloc_run_context()
{
	return (struct run_context *) TEST_ALLOC(sizeof(struct run_context));
}

static void free_run_context(struct run_context *ctx)
{
	size_t i;
	if (!ctx) {
		return;
	}

	if (ctx->input_buffer) {
		TEST_FREE(ctx->input_buffer);
	}
	if (ctx->output_buffer) {
		TEST_FREE(ctx->output_buffer);
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

static pnso_error_t pnso_test_run_testcase(struct test_desc *desc, struct test_testcase *testcase)
{
	pnso_error_t err = PNSO_OK;
	uint32_t iter;
	struct test_node *node;
	struct run_context *ctx;

	ctx = alloc_run_context();
	if (!ctx) {
		PNSO_LOG_ERROR("Cannot allocate run_context for testcase %u\n",
			       testcase->node.idx);
		return ENOMEM;
	}

	for (iter = 0; iter < testcase->repeat; iter++) {
		memset(ctx, 0, sizeof(*ctx));
		ctx->desc = desc;
		ctx->vars[TEST_VAR_ITER] = iter;
		err = run_testcase_once(ctx, testcase);
		if (err != PNSO_OK) {
			PNSO_LOG_ERROR("Testcase %u failed on iteration %u\n",
				       testcase->node.idx, iter);
			break;
		}



	}

	if (err == PNSO_OK) {
		PNSO_LOG_INFO("Successfully ran testcase %u for %u iterations\n",
			      testcase->node.idx, iter);
	}

	/* Print summary of validation success/failure */
	FOR_EACH_NODE(testcase->validations) {
		struct test_validation *validation =
				(struct test_validation *) node;
		PNSO_LOG_INFO("  validation %u: successes=%u, failures=%u\n",
			      validation->node.idx,
			      validation->rt_success_count,
			      validation->rt_failure_count);
	}

	/* Print runtime summary */
	PNSO_LOG_INFO("  runtime: total %lu s\n",
		      ctx->runtime / CLOCKS_PER_SEC);

	free_run_context(ctx);

	return err;
}

pnso_error_t pnso_test_run_all(void *parsed_test)
{
	pnso_error_t err = PNSO_OK;
	struct test_node *node;
	struct test_desc *desc = (struct test_desc *) parsed_test;

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
	err = pnso_sim_thread_init();
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
		err = pnso_test_run_testcase(desc, (struct test_testcase *) node);
		if (err != PNSO_OK) {
			/* TODO: continue? */
			break;
		}
	}




	return PNSO_OK;
}
