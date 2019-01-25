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

extern void pnso_set_log_level(int level);

static osal_atomic_int_t g_shutdown;
static osal_atomic_int_t g_shutdown_complete;
static osal_atomic_int_t g_testcase_active;

#define TEST_SHUTDOWN_TIMEOUT (5 * OSAL_NSEC_PER_SEC)


void pnso_test_shutdown(void)
{
	uint64_t start;

	osal_atomic_set(&g_shutdown, 1);
	start = osal_get_clock_nsec();
	while (!osal_atomic_read(&g_shutdown_complete)) {
		osal_sched_yield();
		if ((osal_get_clock_nsec() - start) >
		    TEST_SHUTDOWN_TIMEOUT)
			break;
	}
}

void pnso_test_set_shutdown_complete(void)
{
	osal_atomic_set(&g_shutdown_complete, 1);
}

bool pnso_test_is_shutdown(void)
{
	if (osal_atomic_read(&g_shutdown_complete))
		return true;
	if (osal_atomic_read(&g_shutdown))
		return true;
	return false;
}

static bool pnso_test_is_active(void)
{
	return osal_atomic_read(&g_testcase_active) != 0;
}

static struct test_file_table g_output_file_tbl;

/* Forward references */
static void free_file_node(struct test_node_file *fnode);
static pnso_error_t read_file_node_input(struct test_file_table *table,
					 const char *filename,
					 struct pnso_buffer_list *buflist);


static struct test_file_table *test_get_output_file_table(void)
{
	uint32_t i;

	if (!g_output_file_tbl.initialized) {
		memset(&g_output_file_tbl.table, 0, sizeof(g_output_file_tbl.table));
		for (i = 0; i < TEST_TABLE_BUCKET_COUNT; i++)
			osal_atomic_init(&g_output_file_tbl.bucket_locks[i], 0);
		g_output_file_tbl.initialized = true;
	}
	return &g_output_file_tbl;
}

static void test_free_output_file_table(void)
{
	struct test_node *node;
	struct test_node *next_node;
	struct test_node_list *list;
	struct test_file_table *file_tbl = test_get_output_file_table();
	int bucket;

	FOR_EACH_TABLE_BUCKET(file_tbl->table) {
		osal_atomic_lock(&file_tbl->bucket_locks[bucket]);
		FOR_EACH_NODE_SAFE(*list) {
			struct test_node_file *fnode =
				(struct test_node_file *) node;
			free_file_node(fnode);
		}
		memset(&file_tbl->table.buckets[bucket], 0,
		       sizeof(file_tbl->table.buckets[bucket]));
		osal_atomic_unlock(&file_tbl->bucket_locks[bucket]);
	}
	file_tbl->initialized = false;
}

struct lookup_var {
	const char *name;
	uint32_t name_len;
};

#define LOOKUP_VAR(name) { name, sizeof(name) - 1 }

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

static const uint8_t *get_normalized_pattern(const char *pat,
					  uint8_t *dst, uint32_t *dst_len)
{
	if (pat[0] == '0' && (pat[1] == 'x' || pat[1] == 'X')) {
		uint32_t tmp = *dst_len - 1;

		if (parse_hex(pat+2, dst, &tmp) == PNSO_OK) {
			*dst_len = tmp;
			return dst;
		}
	}

	*dst_len = strlen(pat);
	return (const uint8_t *) pat;
}

/* Fill buffer by repeating the given pattern */
static pnso_error_t test_fill_pattern(struct pnso_buffer_list *buflist,
			       const char *pat, uint32_t pat_len)
{
	const uint8_t *pat_data;
	uint8_t hex_pat[TEST_MAX_PATTERN_LEN];

	if (pat_len > TEST_MAX_PATTERN_LEN)
		pat_len = TEST_MAX_PATTERN_LEN;
	pat_data = get_normalized_pattern(pat, hex_pat, &pat_len);

	test_fill_buflist(buflist, pat_data, pat_len);

	return PNSO_OK;
}

/* Compare buffer to the given repeating pattern */
static int test_cmp_pattern(const struct pnso_buffer_list *buflist,
		     uint32_t offset, uint32_t len,
		     const char *pat, uint32_t pat_len)
{
	int ret = -1;
	size_t i, j, pat_i;
	size_t total_len = 0;
	const struct pnso_flat_buffer *buf;
	const uint8_t *pat_data;
	uint8_t *dst;
	uint8_t hex_pat[TEST_MAX_PATTERN_LEN];

	if (pat_len > TEST_MAX_PATTERN_LEN)
		pat_len = TEST_MAX_PATTERN_LEN;
	pat_data = get_normalized_pattern(pat, hex_pat, &pat_len);

	if (!len)
		len = pbuf_get_buffer_list_len(buflist);

	pat_i = 0;
	for (i = 0; i < buflist->count && total_len < (offset+len); i++) {
		buf = &buflist->buffers[i];

		if (!buf->len) {
			continue;
		}

		/* Skip past offset bytes */
		j = 0;
		if (total_len < offset) {
			if (buf->len <= offset - total_len) {
				total_len += buf->len;
				continue;
			} else {
				j = offset - total_len;
				total_len = offset;
			}
		}

		dst = (uint8_t *) buflist->buffers[i].buf;
		for (; j < buf->len && total_len < (offset+len); j++) {
			ret = (int) (dst[j] - pat_data[pat_i % pat_len]);
			pat_i++;
			total_len++;
			if (ret)
				return ret;
		}
	}

	return ret;
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
		/* TODO: what about offset and len? */
		//err = test_read_file(path, buflist, input_desc->offset, input_desc->len);
		err = read_file_node_input(test_get_output_file_table(),
					   path,
					   buflist);
	} else if (input_desc->pattern[0]) {
		uint32_t pat_len = strnlen(input_desc->pattern, TEST_MAX_PATTERN_LEN);
		err = test_fill_pattern(buflist, input_desc->pattern, pat_len);
	} else {
		err = test_fill_random(buflist, input_desc->random_seed, input_desc->random_len);
	}
	return err;
}

static void free_buffer_ctx(struct buffer_context *buf_ctx)
{
	if (buf_ctx->buflist)
		TEST_FREE(buf_ctx->buflist);
	if (buf_ctx->buf.buf)
		TEST_FREE((void *) buf_ctx->buf.buf);
	memset(buf_ctx, 0, sizeof(*buf_ctx));
}

#define POISIN_BYTE 'P'
static pnso_error_t alloc_buffer_ctx(struct buffer_context *buf_ctx,
				     uint32_t count,
				     uint32_t total_bytes,
				     uint32_t alignment,
				     bool poisin)
{
	uint32_t block_size;
	size_t i;

	if (!count || !total_bytes)
		return EINVAL;
	block_size = (total_bytes + count - 1) / count;

	/* Allocate buflist */
	buf_ctx->buflist = TEST_ALLOC(sizeof(struct pnso_buffer_list) +
				      (sizeof(struct pnso_flat_buffer) * count));
	if (!buf_ctx->buflist)
		goto no_mem;
	buf_ctx->buflist_alloc_count = count;

	/* Allocate buf data */
	buf_ctx->buf.buf = (uint64_t) TEST_ALLOC_ALIGNED(alignment, count*block_size);
	if (!buf_ctx->buf.buf)
		goto no_mem;
	buf_ctx->buf_alloc_sz = count*block_size;
	if (poisin)
		memset((void *) buf_ctx->buf.buf, POISIN_BYTE, count*block_size);

	/* Point buflist to data */
	buf_ctx->is_sgl_pa = false;
	buf_ctx->buf.len = total_bytes;
	buf_ctx->buflist->count = count;
	for (i = 0; i < count; i++) {
		buf_ctx->buflist->buffers[i].len = block_size;
		buf_ctx->buflist->buffers[i].buf = buf_ctx->buf.buf + (block_size * i);
	}

	/* Adjust last block, in case total_bytes is not an even multiple */
	if (block_size*count > total_bytes) {
		buf_ctx->buflist->buffers[count-1].len = total_bytes -
						(block_size*(count-1));
	}
	return PNSO_OK;

no_mem:
	free_buffer_ctx(buf_ctx);
	return ENOMEM;
}

const uint64_t TEST_MCRC64_POLY = 0x9a6c9329ac4bc9b5ULL;
static uint64_t compute_checksum(const struct pnso_buffer_list *buflist, uint8_t *data, uint32_t length)
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

static int cmp_file_node_metadata(struct test_node_file *fnode1, struct test_node_file *fnode2)
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

static int cmp_buflists(const struct pnso_buffer_list *buflist1,
			const struct pnso_buffer_list *buflist2,
			uint32_t offset,
			uint32_t len)
{
	int ret = -1;
	size_t i1, i2;
	uint32_t offset1;
	uint32_t offset2;
	uint32_t cmp_len;
	uint32_t total_len = 0;
	const struct pnso_flat_buffer *buf1;
	const struct pnso_flat_buffer *buf2;

	if (!len) {
		len = pbuf_get_buffer_list_len(buflist1);
		if (len != pbuf_get_buffer_list_len(buflist2))
			return -1;
		if (!len)
			return 0;
	}

	/* TODO: ignore offset for now */

	i2 = 0;
	buf2 = &buflist2->buffers[i2++];
	offset2 = 0;
	for (i1 = 0; i1 < buflist1->count; i1++) {
		buf1 = &buflist1->buffers[i1];
		offset1 = 0;
		while (offset1 < buf1->len) {
			cmp_len = buf1->len - offset1;
			if (cmp_len > (buf2->len - offset2))
				cmp_len = buf2->len - offset2;
			if (cmp_len) {
				ret = memcmp((void*)buf1->buf+offset1,
					     (void*)buf2->buf+offset2,
					     cmp_len);
				total_len += cmp_len;
				if (ret || total_len >= len)
					return ret;
				offset1 += cmp_len;
				offset2 += cmp_len;
				if (offset2 >= buf2->len) {
					buf2 = &buflist2->buffers[i2++];
					offset2 = 0;
					if (i2 >= buflist2->count ||
					    buf2->len == 0) {
						/* nothing left to compare */
						return ret;
					}
				}
			}
		}
	}


	return ret;
}

static int cmp_file_node_data(struct test_node_file *fnode1, struct test_node_file *fnode2,
			      uint32_t offset, uint32_t len)
{
	int ret;

	if (!fnode1 || !fnode2)
		return -1;

	if (fnode1 == fnode2)
		return 0;

	/* To avoid AB,BA deadlocks, use file index as tie-breaker */
	if (fnode1->node.idx <= fnode2->node.idx) {
		osal_atomic_lock(&fnode1->lock);
		osal_atomic_lock(&fnode2->lock);
	} else {
		osal_atomic_lock(&fnode2->lock);
		osal_atomic_lock(&fnode1->lock);
	}

	/* First check whether metadata matches */
	ret = cmp_file_node_metadata(fnode1, fnode2);
	if (ret != 0) {
		/* Only compare data if we have to */
		ret = cmp_buflists(fnode1->buflist,
				   fnode2->buflist,
				   offset, len);
	}

	/* Unlock in reverse order */
	if (fnode1->node.idx <= fnode2->node.idx) {
		osal_atomic_unlock(&fnode2->lock);
		osal_atomic_unlock(&fnode1->lock);
	} else {
		osal_atomic_unlock(&fnode1->lock);
		osal_atomic_unlock(&fnode2->lock);
	}

	return ret;
}

static void pprint_file_node(struct test_node_file *fnode)
{
	struct test_node_file fnode_dup;
	char hexstr[129] = "";

	if (g_osal_log_level < OSAL_LOG_LEVEL_DEBUG)
		return;

	osal_atomic_lock(&fnode->lock);
	fnode_dup = *fnode;
	safe_bintohex(hexstr, 128, fnode->data, fnode->file_size);
	osal_atomic_unlock(&fnode->lock);

	OSAL_LOG_DEBUG("File node: name %s, size %u, padded_size %u, data:\n",
		       fnode_dup.filename, fnode_dup.file_size, fnode_dup.padded_size);
	OSAL_LOG_DEBUG("0x%s\n", hexstr);
}

static struct test_node_file *lookup_file_node(struct test_file_table *table,
					       const char *filename,
					       bool do_create)
{
	struct test_node_file search_fnode;
	struct test_node_file *fnode;
	int bucket;

	memset(&search_fnode, 0, sizeof(search_fnode));
	search_fnode.node.type = NODE_FILE;
	search_fnode.node.idx = str_hash(filename);
	strncpy(search_fnode.filename, filename, TEST_MAX_FULL_PATH_LEN);

	bucket = search_fnode.node.idx % TEST_TABLE_BUCKET_COUNT;
	osal_atomic_lock(&table->bucket_locks[bucket]);
	fnode = (struct test_node_file *) test_node_table_lookup(&table->table,
							&search_fnode.node,
							cmp_file_node);
	if (do_create && !fnode) {
		fnode = (struct test_node_file *) test_node_alloc(sizeof(*fnode),
								  NODE_FILE);
		if (fnode) {
			*fnode = search_fnode;
			osal_atomic_init(&fnode->lock, 0);
			test_node_table_insert(&table->table, &fnode->node);
		}
	}

	osal_atomic_unlock(&table->bucket_locks[bucket]);
	return fnode;
}

static uint32_t lookup_file_node_size(struct test_file_table *table,
				      const char *filename)
{
	uint32_t ret = 0;
	struct test_node_file *fnode;

	fnode = lookup_file_node(table, filename, false);
	if (fnode) {
		osal_atomic_lock(&fnode->lock);
		ret = fnode->file_size;
		osal_atomic_unlock(&fnode->lock);
	}

	return ret;
}

static void free_file_node(struct test_node_file *fnode)
{
	if (!fnode)
		return;
	if (fnode->buflist) {
		TEST_FREE(fnode->buflist);
	}
	if (fnode->data) {
		TEST_FREE(fnode->data);
	}
	TEST_FREE(fnode);
}

static pnso_error_t read_file_node_input(struct test_file_table *table,
					 const char *filename,
					 struct pnso_buffer_list *buflist)
{
	struct test_node_file *fnode;
	uint32_t len;

	fnode = lookup_file_node(table, filename, false);
	if (!fnode) {
		PNSO_LOG_INFO("Input file %s not found\n", filename);
		return ENOENT;
	}

	osal_atomic_lock(&fnode->lock);
	len = pbuf_copy_buffer_list(fnode->buflist, buflist);	
	osal_atomic_unlock(&fnode->lock);

	PNSO_LOG_DEBUG("Copied %u input bytes from file %s\n",
		       len, filename);

	return PNSO_OK;
}

static pnso_error_t copy_file_node_data(struct test_node_file *fnode,
					const struct pnso_buffer_list *buflist)
{
	uint32_t dst_i;
	uint32_t len, max_len;
	uint32_t blk_count;
	uint32_t alloc_size;
	struct pnso_flat_buffer *fbuf;

	if (!fnode || !buflist)
		return EINVAL;

	len = pbuf_get_buffer_list_len(buflist);
	max_len = fnode->padded_size ? fnode->padded_size : fnode->file_size;
	if (len > max_len)
		len = max_len;
	if (!len)
		return EINVAL;

	blk_count = (len + fnode->block_size - 1) / fnode->block_size;
	alloc_size = blk_count * fnode->block_size;

	/* Resize if not enough room */
	if (fnode->alloc_size < alloc_size) {
		if (fnode->buflist)
			TEST_FREE(fnode->buflist);
		if (fnode->data)
			TEST_FREE(fnode->data);
		fnode->buflist = (struct pnso_buffer_list *) TEST_ALLOC(
			sizeof(struct pnso_buffer_list) +
			(blk_count * sizeof(struct pnso_flat_buffer)));
		fnode->data = TEST_ALLOC(alloc_size);
		if (!fnode->buflist || !fnode->data) {
			goto no_mem;
		}
		fnode->alloc_size = alloc_size;
		fnode->buflist->count = blk_count;
	}

	/* Set buffer list info */
	for (dst_i = 0; dst_i < fnode->buflist->count; dst_i++) {
		fbuf = &fnode->buflist->buffers[dst_i];
		if (dst_i < blk_count) {
			fbuf->len = fnode->block_size;
			fbuf->buf = (uint64_t) fnode->data +
				(dst_i * fnode->block_size);
		} else {
			fbuf->len = 0;
			fbuf->buf = 0;
		}
	}
	/* Update last block to odd size */
	fbuf = &fnode->buflist->buffers[blk_count - 1];
	fbuf->len = len - (fnode->block_size * (blk_count - 1));	

	/* Copy data */
	pbuf_copy_buffer_list(buflist, fnode->buflist);

	return PNSO_OK;

no_mem:
	if (fnode->buflist) {
		TEST_FREE(fnode->buflist);
		fnode->buflist = NULL;
	}
	if (fnode->data) {
		TEST_FREE(fnode->data);
		fnode->data = NULL;
	}
	fnode->alloc_size = 0;
	return ENOMEM;
}

static pnso_error_t update_file_node(struct test_file_table *table,
				     const char *filename,
				     uint64_t checksum,
				     uint32_t file_size,
				     uint64_t padded_checksum,
				     uint32_t padded_size,
				     uint32_t blk_sz,
				     const struct pnso_buffer_list *buflist)
{
	pnso_error_t err = PNSO_OK;
	struct test_node_file search_fnode;
	struct test_node_file *fnode;

	fnode = lookup_file_node(table, filename, true);
	if (!fnode)
		return ENOMEM;

	osal_atomic_lock(&fnode->lock);

	search_fnode.checksum = checksum;
	search_fnode.file_size = file_size;
	search_fnode.padded_checksum = padded_checksum;
	search_fnode.padded_size = padded_size;

	if (0 != cmp_file_node_metadata(fnode, &search_fnode)) {
		fnode->checksum = checksum;
		fnode->file_size = file_size;
		fnode->padded_checksum = padded_checksum;
		fnode->padded_size = padded_size;
		fnode->block_size = blk_sz;
		err = copy_file_node_data(fnode, buflist);
	}

	osal_atomic_unlock(&fnode->lock);

	return err;
}

static void buffer_ctx_p2v(struct buffer_context *buf_ctx)
{
	if (buf_ctx->is_sgl_pa && buf_ctx->buflist) {
		pbuf_convert_buffer_list_p2v(buf_ctx->buflist);
		buf_ctx->is_sgl_pa = false;
	}
}

static void buffer_ctx_v2p(struct buffer_context *buf_ctx)
{
	if (!buf_ctx->is_sgl_pa && buf_ctx->buflist) {
		pbuf_convert_buffer_list_v2p(buf_ctx->buflist);
		buf_ctx->is_sgl_pa = true;
	}
}

static void request_ctx_p2v(struct request_context *req_ctx)
{
	size_t i;

	if (!req_ctx)
		return;

	/* Normalize sgls from physical to virtual addresses */
	buffer_ctx_p2v(&req_ctx->input);
	for (i = 0; i < req_ctx->svc_res.num_services; i++) {
		buffer_ctx_p2v(&req_ctx->outputs[i]);
	}
}

static void request_ctx_v2p(struct request_context *req_ctx)
{
	size_t i;

	if (!req_ctx)
		return;

  	/* Normalize sgls from virtual to physical addresses */
	buffer_ctx_v2p(&req_ctx->input);
	for (i = 0; i < req_ctx->svc_res.num_services; i++) {
		buffer_ctx_v2p(&req_ctx->outputs[i]);
	}
}

static void batch_ctx_p2v(struct batch_context *batch_ctx)
{
	size_t i;

	for (i = 0; i < batch_ctx->req_count; i++) {
		request_ctx_p2v(batch_ctx->req_ctxs[i]);
	}
}

static uint32_t get_svc_status_data_len(const struct pnso_service_status *svc_status)
{
	uint32_t ret = 0;

	switch (svc_status->svc_type) {
	case PNSO_SVC_TYPE_HASH:
		ret = svc_status->u.hash.num_tags *
			sizeof(struct pnso_hash_tag);
		break;
	case PNSO_SVC_TYPE_CHKSUM:
		ret = svc_status->u.chksum.num_tags *
			sizeof(struct pnso_chksum_tag);
		break;
	case PNSO_SVC_TYPE_ENCRYPT:
	case PNSO_SVC_TYPE_DECRYPT:
	case PNSO_SVC_TYPE_COMPRESS:
	case PNSO_SVC_TYPE_DECOMPRESS:
		ret = svc_status->u.dst.data_len;
		break;
	default:
		break;
	}

	return ret;
}

static void output_results(struct request_context *req_ctx,
			   const struct test_svc_chain *svc_chain)
{
	uint32_t i;
	struct test_node *node;
	uint64_t checksum = 0;
	uint64_t padded_checksum = 0;
	uint32_t file_size = 0;
	uint32_t padded_size = 0;
	pnso_error_t err;
	struct batch_context *batch_ctx = req_ctx->batch_ctx;
	struct testcase_context *test_ctx = batch_ctx->test_ctx;
	const struct test_testcase *testcase = test_ctx->testcase;
	uint32_t blk_sz = batch_ctx->desc->init_params.block_size;
	char output_path[TEST_MAX_FULL_PATH_LEN] = "";

	/* output to file */
	i = 0;
	FOR_EACH_NODE(svc_chain->svcs) {
		struct test_svc *svc = (struct test_svc *) node;
		struct pnso_service_status *svc_status = &req_ctx->svc_res.svc[i];
		if (svc->output_path[0] && svc_status->err == PNSO_OK) {
			err = construct_filename(batch_ctx->desc, req_ctx->vars,
						 output_path, svc->output_path);
			if (err != PNSO_OK) {
				break;
			}
			padded_size = 0;
			if (svc_status->svc_type == PNSO_SVC_TYPE_HASH ||
			    svc_status->svc_type == PNSO_SVC_TYPE_CHKSUM) {
				if (svc_status->u.hash.tags) {
					/* Hack alert: static allocation for zero-len array */
					struct {
						struct pnso_buffer_list bl;
						struct pnso_flat_buffer b;
					} buflist;

					buflist.bl.count = 1;
					buflist.b.buf = (uint64_t) svc_status->u.hash.tags;
					buflist.b.len = get_svc_status_data_len(svc_status);
					file_size = buflist.b.len;
					if (!testcase->turbo) {
						checksum = compute_checksum(NULL,
							(uint8_t *) buflist.b.buf,
							file_size);
					}
					err = update_file_node(
						test_ctx->output_file_tbl,
						output_path, checksum,
						file_size,
						padded_checksum, padded_size,
						blk_sz,
						&buflist.bl);
				}
			} else if (svc_status->u.dst.sgl) {
				file_size = get_svc_status_data_len(svc_status);
				if (!testcase->turbo) {
					checksum = compute_checksum(svc_status->u.dst.sgl,
								    NULL, file_size);
				}
				if (svc->svc.svc_type == PNSO_SVC_TYPE_COMPRESS &&
				    (svc->svc.u.cp_desc.flags & PNSO_CP_DFLAG_ZERO_PAD)) {
					/* Round up */
					padded_size = roundup_len(file_size, blk_sz);
					if (!testcase->turbo) {
						padded_checksum = compute_checksum(
							svc_status->u.dst.sgl,
							NULL, padded_size);
					}
				}
				err = update_file_node(
					test_ctx->output_file_tbl,
					output_path, checksum,
					file_size,
					padded_checksum, padded_size,
					blk_sz,
					svc_status->u.dst.sgl);
			}
			if (err != PNSO_OK) {
				PNSO_LOG_ERROR("Cannot write data to %s\n",
					       output_path);
				break;
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

static void batch_completion_common(struct batch_context *batch_ctx)
{
	int count;

	if (batch_ctx->req_rc == PNSO_OK)
		batch_ctx->stats.agg_stats.total_latency =
			osal_get_clock_nsec() - batch_ctx->start_time;

	count = osal_atomic_fetch_add(&batch_ctx->cb_count, 1);
	if (count != 0) {
		PNSO_LOG_ERROR("Unexpected batch cb_count %d\n", count);
	}
}

static void batch_completion_safe_cb(void *cb_ctx, struct pnso_service_result *svc_res)
{
	struct batch_context *batch_ctx = (struct batch_context *) cb_ctx;
	struct worker_context *worker_ctx;

	PNSO_LOG_DEBUG("batch_completion_safe_cb cb_ctx=0x%llx\n",
		       (unsigned long long) cb_ctx);

	if (!pnso_test_is_active()) {
		PNSO_LOG_ERROR("Batch completion callback while test inactive!\n");
		return;
	}

	batch_completion_common(batch_ctx);

	worker_ctx = batch_get_worker_ctx(batch_ctx);
	if (!worker_ctx || !worker_queue_enqueue_safe(worker_ctx->complete_q, batch_ctx))
		PNSO_LOG_ERROR("Failed to enqueue batch_ctx to complete_q!\n");	
}

static void batch_completion_cb(void *cb_ctx, struct pnso_service_result *svc_res)
{
	struct batch_context *batch_ctx = (struct batch_context *) cb_ctx;
	struct worker_context *worker_ctx;

	PNSO_LOG_DEBUG("batch_completion_cb cb_ctx=0x%llx\n",
		       (unsigned long long) cb_ctx);

	if (!pnso_test_is_active()) {
		PNSO_LOG_ERROR("Batch completion callback while test inactive!\n");
		return;
	}

	batch_completion_common(batch_ctx);

	worker_ctx = batch_get_worker_ctx(batch_ctx);
	if (!worker_ctx || !worker_queue_enqueue(worker_ctx->complete_q, batch_ctx))
		PNSO_LOG_ERROR("Failed to enqueue batch_ctx to complete_q!\n");	
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

	PNSO_LOG_DEBUG("submitting request, mode %u\n",
			(uint32_t) sync_mode);

	/* Normalize sgls from virtual to physical addresses */
	request_ctx_v2p(req_ctx);

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
		cb = batch_completion_safe_cb;
		cb_ctx = batch_ctx;
		poll_fn = NULL;
		poll_ctx = NULL;
		break;
	}

	if (is_batched) {
		rc = pnso_add_to_batch(&req_ctx->svc_req, &req_ctx->svc_res);
		if (flush && rc == PNSO_OK) {
			PNSO_LOG_DEBUG("calling flush_batch with cb_ctx=0x%llx\n",
				       (unsigned long long) cb_ctx);
			req_ctx->batch_ctx->start_time = osal_get_clock_nsec();
			rc = pnso_flush_batch(cb, cb_ctx,
					      poll_fn, poll_ctx);
		}
	} else {
		PNSO_LOG_DEBUG("calling submit_request with cb_ctx=0x%llx\n",
			       (unsigned long long) cb_ctx);
		req_ctx->batch_ctx->start_time = osal_get_clock_nsec();
		rc = pnso_submit_request(&req_ctx->svc_req,
					 &req_ctx->svc_res,
					 cb, cb_ctx,
					 poll_fn, poll_ctx);
	}

	return rc;
}

static pnso_error_t init_input_context(struct buffer_context *input,
				       struct testcase_context *test_ctx,
				       const struct test_svc_chain *svc_chain)
{
	pnso_error_t err = PNSO_OK;
	uint32_t input_len;
	uint32_t min_block, max_block, block_count;
	//uint32_t remain_len, buf_len;
	//uint32_t i;
	//uint8_t *buf;
	char input_path[TEST_MAX_FULL_PATH_LEN] = "";

	OSAL_ASSERT(!input->initialized);

	/* construct input filename */
	if (svc_chain->input.pathname[0]) {
		err = construct_filename(test_ctx->desc, test_ctx->vars,
					 input_path, svc_chain->input.pathname);
		if (err != PNSO_OK) {
			return err;
		}
	}

	/* get or infer input_len */
	input_len = svc_chain->input.len;
	if (!input_len) {
		/* Try to infer the length, for user convenience */
		if (input_path[0]) {
			input_len = lookup_file_node_size(
					test_ctx->output_file_tbl,
					input_path);

			/* TODO: separate file table for input files */
#if 0
			input_len = test_file_size(input_path);
#endif
			if (!input_len) {
				PNSO_LOG_DEBUG("Invalid input file %s\n", input_path);
				return EINVAL;
			}
		} else if (svc_chain->input.pattern[0]) {
			input_len = strlen(svc_chain->input.pattern);
		} else {
			input_len = test_ctx->desc->init_params.block_size;
		}
	}

	/* setup input sgl */
	min_block = svc_chain->input.min_block_size;
	max_block = svc_chain->input.max_block_size;
	block_count = svc_chain->input.block_count;
	if (!min_block) {
		min_block = test_ctx->desc->init_params.block_size;
	}
	if (!max_block) {
		max_block = test_ctx->desc->init_params.block_size;
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

	err = alloc_buffer_ctx(input, block_count, input_len,
			       test_ctx->desc->init_params.block_size,
			       false);
	if (err != PNSO_OK) {
		PNSO_LOG_ERROR("Failed to allocate buffer_ctx with %u blocks, %u bytes\n",
			       block_count, input_len);
		return err;
	}

	/* populate input buffer */
	err = test_read_input(input_path, &svc_chain->input, input->buflist);
	if (err != PNSO_OK) {
		PNSO_LOG_ERROR("Failed to read input from %s\n", input_path);
		return err;
	}

	input->svc_chain_idx = svc_chain->node.idx;
	input->initialized = true;

	return PNSO_OK;
}

static pnso_error_t init_req_input(struct request_context *req_ctx)
{
	struct buffer_context *chain_input = &req_ctx->chain_ctx->input;
	struct buffer_context *buf_ctx = &req_ctx->input;
	uint32_t i, count;

	if (buf_ctx->initialized &&
	    buf_ctx->svc_chain_idx == chain_input->svc_chain_idx) {
		/* Already initialized */
		return PNSO_OK;
	}
	buf_ctx->initialized = false;

	/* Clone from chain */

	/* Resize buflist if necessary */
	count = chain_input->buflist_alloc_count;
	if (count > buf_ctx->buflist_alloc_count) {
		if (buf_ctx->buflist)
			TEST_FREE(buf_ctx->buflist);
		buf_ctx->buflist = TEST_ALLOC(sizeof(struct pnso_buffer_list) +
					      (sizeof(struct pnso_flat_buffer) * count));
		if (!buf_ctx->buflist)
			goto no_mem;
		buf_ctx->buflist_alloc_count = count;
	}

	/* Point buflist to chain data */
	buf_ctx->is_sgl_pa = false;
	buf_ctx->buf = chain_input->buf;
	buf_ctx->buflist->count = count;
	for (i = 0; i < count; i++) {
		buf_ctx->buflist->buffers[i] = chain_input->buflist->buffers[i];
	}
	buf_ctx->svc_chain_idx = chain_input->svc_chain_idx;
	buf_ctx->initialized = true;

	return PNSO_OK;

no_mem:
	return ENOMEM;
}

static pnso_error_t run_testcase_svc_chain(struct request_context *req_ctx,
					   const struct test_testcase *testcase,
					   uint32_t batch_iter,
					   uint32_t batch_count)
{
	pnso_error_t err = PNSO_OK;
	struct batch_context *batch_ctx = req_ctx->batch_ctx;
	const struct test_svc_chain *svc_chain = req_ctx->chain_ctx->svc_chain;
	struct test_node *node;
	uint32_t input_len;
	uint32_t i;

	/* setup source buffers */
	err = init_req_input(req_ctx);
	if (err)
		return err;
	input_len = req_ctx->input.buf.len;

	/* setup request */
	req_ctx->svc_req.sgl = req_ctx->input.buflist;
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
			if (svc->u.crypto.iv_data) {
				req_ctx->svc_req.svc[i].u.crypto_desc.iv_addr =
					osal_virt_to_phy(svc->u.crypto.iv_data);
			}
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
		struct buffer_context *buf_ctx = &req_ctx->outputs[i];

		svc_status->svc_type = svc->svc.svc_type; /* TODO: needed? */
		if (svc->output_path[0]) {
			uint32_t output_len = get_max_output_len_by_type(
							svc->svc.svc_type,
							svc->output_flags,
							input_len);

			if (svc->svc.svc_type == PNSO_SVC_TYPE_HASH ||
			    svc->svc.svc_type == PNSO_SVC_TYPE_CHKSUM) {
				err = alloc_buffer_ctx(buf_ctx, 1, output_len, 1, false);
				if (err != PNSO_OK) {
					PNSO_LOG_TRACE(
						"Out of memory for output tags\n");
					return ENOMEM;
				}
				svc_status->u.hash.tags = (void *) buf_ctx->buf.buf;
				svc_status->u.hash.num_tags =
						(svc->output_flags &
						 TEST_OUTPUT_FLAG_TINY) ? 1 :
						MAX_OUTPUT_BUF_COUNT;
			} else {
				output_len = roundup_len(output_len,
					batch_ctx->desc->init_params.block_size);
				err = alloc_buffer_ctx(
						buf_ctx,
						(svc->output_flags &
						 TEST_OUTPUT_FLAG_TINY) ? 1 :
						(output_len /
						 batch_ctx->desc->init_params.block_size),
						output_len,
						batch_ctx->desc->init_params.block_size,
						!testcase->turbo &&
						svc->svc.svc_type == PNSO_SVC_TYPE_COMPRESS &&
						(svc->svc.u.cp_desc.flags & PNSO_CP_DFLAG_ZERO_PAD));
				if (err != PNSO_OK) {
					PNSO_LOG_TRACE(
						"Out of memory for output_buf\n");
					return ENOMEM;
				}
				svc_status->u.dst.sgl = buf_ctx->buflist;
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
	req_ctx->req_rc = err;
	if (err)
		batch_ctx->req_rc = err;

	return err;
}

static const char *testcase_stats_names[TESTCASE_STATS_COUNT] = {
	"elapsed_time",
	"total_latency",
	"avg_latency",
	"min_latency",
	"max_latency",
	"in_bytes_per_sec",
	"out_bytes_per_sec",
	"max_in_bytes_per_sec",
	"max_out_bytes_per_sec",
	"svcs_per_sec",
	"reqs_per_sec",
	"batches_per_sec",
	"validation_successes",
	"validation_failures",

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
	uint64_t bytes_per;

	if (ns == 0)
		return 0;

	bytes_per = bytes / ns;
	if (bytes_per >= 1000)
		return (bytes_per * OSAL_NSEC_PER_SEC);

	bytes_per = bytes / (1 + (ns / OSAL_NSEC_PER_USEC));
	if (bytes_per >= 1000)
		return bytes_per * OSAL_USEC_PER_SEC;
	
	bytes_per = bytes / (1 + (ns / OSAL_NSEC_PER_MSEC));
	if (bytes_per >= 1000)
		return bytes_per * OSAL_MSEC_PER_SEC;

	return bytes / (1 + (ns / OSAL_NSEC_PER_SEC));
}

static void calculate_completion_stats(struct batch_context *batch_ctx)
{
	uint32_t i, j;
	struct request_context *req_ctx;
	struct pnso_service *req_svc;
	struct pnso_service_status *svc_status;
	struct testcase_io_stats *stats = &batch_ctx->stats.io_stats[1];

	/* submission failed, so no completion stats */
	if (batch_ctx->req_rc != PNSO_OK)
		return;

	stats->batches += 1;
	for (i = 0; i < batch_ctx->req_count; i++) {
		req_ctx = batch_ctx->req_ctxs[i];
		if (!req_ctx)
			goto error;
		stats->reqs += 1;
		if (req_ctx->svc_res.num_services != req_ctx->svc_req.num_services)
			goto error;
		for (j = 0; j < req_ctx->svc_res.num_services; j++) {
			req_svc = &req_ctx->svc_req.svc[j];
			svc_status = &req_ctx->svc_res.svc[j];
			stats->svcs += 1;
			if (pnso_svc_type_is_data(svc_status->svc_type)) {
				if (svc_status->u.dst.sgl)
					stats->bytes += svc_status->u.dst.data_len;
			} else if (svc_status->svc_type == PNSO_SVC_TYPE_CHKSUM) {
				if (svc_status->u.chksum.tags)
					stats->bytes += svc_status->u.chksum.num_tags *
						pnso_get_chksum_algo_size(req_svc->u.chksum_desc.algo_type);
			} else if (svc_status->svc_type == PNSO_SVC_TYPE_HASH) {
				if (svc_status->u.hash.tags)
					stats->bytes += svc_status->u.hash.num_tags *
						pnso_get_hash_algo_size(req_svc->u.hash_desc.algo_type);
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
	uint64_t ns;
	uint64_t tmp;

	ts1->elapsed_time = elapsed_time;
	if (ts2->agg_stats.total_latency) {
		tmp = ts1->agg_stats.total_latency;
		ts1->agg_stats.total_latency += ts2->agg_stats.total_latency;
		if (ts1->agg_stats.min_latency == 0 ||
		    ts1->agg_stats.min_latency > ts2->agg_stats.total_latency) {
			ts1->agg_stats.min_latency = ts2->agg_stats.total_latency;
		}
		if (ts1->agg_stats.max_latency == 0 ||
		    ts1->agg_stats.max_latency < ts2->agg_stats.total_latency) {
			ts1->agg_stats.max_latency = ts2->agg_stats.total_latency;
		}
		if (tmp > ts1->agg_stats.total_latency) {
			PNSO_LOG_WARN("Total latency counter wrapped around\n");
		}
	}
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

	if (ts1->agg_stats.total_latency &&
	    ts1->agg_stats.total_latency < elapsed_time) {
		ns = ts1->agg_stats.total_latency;
	} else {
		ns = elapsed_time;
	}

	/* Calculate maximum burst speed */
	tmp = calculate_bytes_per_sec(ts2->io_stats[0].bytes,
				      ts2->agg_stats.total_latency);
	if (tmp > ts1->agg_stats.max_in_bytes_per_sec)
		ts1->agg_stats.max_in_bytes_per_sec = tmp;
	tmp = calculate_bytes_per_sec(ts2->io_stats[1].bytes,
				      ts2->agg_stats.total_latency);
	if (tmp > ts1->agg_stats.max_out_bytes_per_sec)
		ts1->agg_stats.max_out_bytes_per_sec = tmp;

	ts1->agg_stats.in_bytes_per_sec = calculate_bytes_per_sec(
		       ts1->io_stats[0].bytes, ns);
	ts1->agg_stats.out_bytes_per_sec = calculate_bytes_per_sec(
		       ts1->io_stats[1].bytes, ns);
	ts1->agg_stats.svcs_per_sec = calculate_bytes_per_sec(
		       ts1->io_stats[1].svcs, ns);
	ts1->agg_stats.reqs_per_sec = calculate_bytes_per_sec(
		       ts1->io_stats[1].reqs, ns);
	ts1->agg_stats.batches_per_sec = calculate_bytes_per_sec(
		       ts1->io_stats[1].batches, ns);
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

static bool validation_is_per_req(const struct test_validation *validation)
{
	return (validation->type == VALIDATION_RETCODE_COMPARE) ||
		(validation->type == VALIDATION_DATA_LEN_COMPARE);
}

static void update_validation_stats(struct testcase_context *test_ctx,
				    struct test_validation *validation,
				    int cmp,
				    pnso_error_t err)
{
	osal_atomic_lock(&test_ctx->stats_lock);
	if (err == PNSO_OK) {
		if (is_compare_true(validation->cmp_type, cmp)) {
			validation->rt_success_count++;
			test_ctx->stats.agg_stats.validation_successes++;
		} else {
			validation->rt_failure_count++;
			test_ctx->stats.agg_stats.validation_failures++;
		}
	} else {
		validation->rt_failure_count++;
		test_ctx->stats.agg_stats.validation_failures++;
	}
	osal_atomic_unlock(&test_ctx->stats_lock);
}

static pnso_error_t run_data_validation(struct batch_context *ctx,
					const struct test_testcase *testcase,
					struct test_validation *validation)
{
	pnso_error_t err = PNSO_OK;
	struct testcase_context *test_ctx = ctx->test_ctx;
	struct test_node_file *fnode1 = NULL, *fnode2;
	char path1[TEST_MAX_FULL_PATH_LEN] = "";
	char path2[TEST_MAX_FULL_PATH_LEN] = "";
	int cmp = 0;
	uint32_t offset = validation->offset;
	uint32_t len = validation->len;

	if (ctx->req_rc != PNSO_OK) {
		err = ctx->req_rc;
		goto done;
	}

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

	/* calculate dynamic offset/len */
	if (validation->offset >= DYN_OFFSET_START ||
	    validation->len >= DYN_OFFSET_START) {
		uint32_t file_len = lookup_file_node_size(
					test_ctx->output_file_tbl, path1);

		if (offset == DYN_OFFSET_EOF)
			offset = file_len;
		else if (offset == DYN_OFFSET_EOB)
			offset = roundup_len(file_len,
					ctx->desc->init_params.block_size);

		if (len == DYN_OFFSET_EOF)
			len = file_len - offset;
		else if (len == DYN_OFFSET_EOB)
			len = roundup_len(file_len,
				ctx->desc->init_params.block_size) - offset;
		OSAL_LOG_DEBUG("data_compare using dynamic offset %u, len %u, file_len %u\n",
				offset, len, file_len);
	}

	/* Validate */
	switch (validation->type) {
	case VALIDATION_DATA_COMPARE:
		if (path1[0] && path2[0]) {
			/* Try to compare just local metadata */
			fnode1 = lookup_file_node(test_ctx->output_file_tbl, path1, false);
			fnode2 = lookup_file_node(test_ctx->output_file_tbl, path2, false);
			cmp = cmp_file_node_data(fnode1, fnode2,
						 offset, len);
#if 0 //#ifndef __KERNEL__
			/* Metadata not available or inconclusive, do full file compare */
			if (cmp != 0) {
				cmp = test_compare_files(path1, path2,
							 offset, len);
			}
#endif
		} else if (validation->pattern[0] && (path1[0] || path2[0])) {
			char *path = path1;
			const char *pat = validation->pattern;
			uint32_t pat_len = strlen(validation->pattern);

			if (!path1[0])
				path = path2;
			fnode1 = lookup_file_node(test_ctx->output_file_tbl, path, false);
			if (fnode1) {
				osal_atomic_lock(&fnode1->lock);
				cmp = test_cmp_pattern(fnode1->buflist,
						       offset, len,
						       pat, pat_len);
				osal_atomic_unlock(&fnode1->lock);
				if (cmp) {
					pprint_file_node(fnode1);
				}
			} else {
				cmp = -1;
			}

#if 0 //#ifndef __KERNEL__
			/* Metadata not available, do full file compare */
			if (!fnode1) {
				uint8_t hex_pat[TEST_MAX_PATTERN_LEN];

				if (pat_len > TEST_MAX_PATTERN_LEN)
					pat_len = TEST_MAX_PATTERN_LEN;
				pat = (const char *) get_normalized_pattern(pat,
							hex_pat, &pat_len);
				cmp = test_compare_file_data(path,
						     offset, len,
						     (const uint8_t *) pat,
						     pat_len);
			}
#endif
		} else {
			err = EINVAL;
		}
		break;
	case VALIDATION_SIZE_COMPARE:
		if (path1[0] && path2[0]) {
			/* Compare size of 2 files */
			cmp = (int) lookup_file_node_size(
					test_ctx->output_file_tbl, path1) -
				(int) lookup_file_node_size(
					test_ctx->output_file_tbl, path2);
		} else {
			/* Test static length */
			cmp = (int) lookup_file_node_size(
					test_ctx->output_file_tbl, path1) -
				(int) len;
		}
		break;
	default:
		PNSO_LOG_ERROR("Invalid validation type %u\n",
			       validation->type);
		err = EINVAL;
		break;
	}

done:
	update_validation_stats(test_ctx, validation, cmp, err);
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
	struct testcase_context *test_ctx = batch_ctx->test_ctx;

	cmp = (int) batch_ctx->req_rc - (int) validation->req_retcode;
	if (cmp != 0 || batch_ctx->req_rc != PNSO_OK) {
		goto done;
	}

	cmp = (int) req_ctx->svc_res.err - (int) validation->retcode;
	if (cmp != 0) {
		goto done;
	}

	if (req_ctx->svc_res.num_services < validation->svc_count) {
		err = EINVAL;
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
	update_validation_stats(test_ctx, validation, cmp, err);
	return err;
}

static pnso_error_t run_data_len_validation(struct request_context *req_ctx,
					    const struct test_testcase *testcase,
					    struct test_validation *validation)
{
	pnso_error_t err = PNSO_OK;
	size_t i;
	int cmp = 0;
	int total_len = 0;
	struct batch_context *batch_ctx = req_ctx->batch_ctx;
	struct testcase_context *test_ctx = batch_ctx->test_ctx;

	if (batch_ctx->req_rc != PNSO_OK) {
		err = batch_ctx->req_rc;
		goto done;
	}

	if (validation->len) {
		/* get total len */
		for (i = 0; i < req_ctx->svc_res.num_services; i++)
			total_len += get_svc_status_data_len(&req_ctx->svc_res.svc[i]);

		cmp = (int) total_len - (int) validation->len;
		if (cmp != 0) {
			goto done;
		}
	}

	if (req_ctx->svc_res.num_services < validation->svc_count) {
		err = EINVAL;
		goto done;
	}

	for (i = 0; i < validation->svc_count; i++) {
		cmp = (int) get_svc_status_data_len(&req_ctx->svc_res.svc[i]) -
			(int) validation->svc_retcodes[i];
		if (cmp != 0) {
			break;
		}
	}

done:
	update_validation_stats(test_ctx, validation, cmp, err);
	return err;
}

static pnso_error_t run_req_validation(struct request_context *req_ctx)
{
	const struct test_testcase *testcase;
	struct batch_context *batch_ctx;
	struct test_node *node;

	if (!req_ctx || !req_ctx->chain_ctx)
		return EINVAL;

	batch_ctx = req_ctx->batch_ctx;
	testcase = batch_ctx->test_ctx->testcase;

	/* Output at least the first result of each worker or chain */
	if (batch_ctx->req_rc == PNSO_OK &&
	    req_ctx->svc_res.err == PNSO_OK &&
	    (!testcase->turbo ||
	     (batch_ctx->batch_id <
	      batch_ctx->test_ctx->worker_count) ||
	     (req_ctx->req_id <
	      batch_ctx->test_ctx->chain_lb_table_count))) {
		output_results(req_ctx, req_ctx->chain_ctx->svc_chain);
	}

	FOR_EACH_NODE(testcase->validations) {
		struct test_validation *validation =
			(struct test_validation *) node;

		if (!validation_is_per_req(validation) ||
		    (validation->svc_chain_idx &&
		     validation->svc_chain_idx !=
		     req_ctx->chain_ctx->svc_chain->node.idx)) {
			continue;
		}
		switch (validation->type) {
		case VALIDATION_RETCODE_COMPARE:
			run_retcode_validation(req_ctx, testcase, validation);
			break;
		case VALIDATION_DATA_LEN_COMPARE:
			run_data_len_validation(req_ctx, testcase, validation);
			break;
		default:
			PNSO_LOG_ERROR("Unknown validation type %u\n", validation->type);
			break;
		}

	}

	return PNSO_OK;
}

static bool is_svc_chain_in_batch(struct batch_context *batch_ctx,
				  uint32_t svc_chain_idx)
{
	struct request_context *req_ctx;
	uint32_t req_count;
	int i;

	req_count = batch_ctx->req_count;

	/* in single chain case, check just first req */
	if (req_count && batch_ctx->test_ctx->chain_count == 1)
		req_count = 1;

	/* check each request */
	for (i = 0; i < req_count; i++) {
		req_ctx = batch_ctx->req_ctxs[i];
		if (!req_ctx || !req_ctx->chain_ctx)
			break;

		if (req_ctx->chain_ctx->svc_chain->node.idx ==
		    svc_chain_idx) {
			return true;
		}
	}

	return false;
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

	/* skip ALL file validations in turbo mode */
	if (testcase->turbo)
		return err;

	/* run file validations */
	FOR_EACH_NODE(testcase->validations) {
		struct test_validation *validation =
				(struct test_validation *) node;

		if (validation_is_per_req(validation) ||
		    (validation->svc_chain_idx &&
		     !is_svc_chain_in_batch(batch_ctx,
				validation->svc_chain_idx))) {
			continue;
		}

		run_data_validation(batch_ctx, testcase, validation);
	}

	return err;
}

static void init_req_context(struct request_context *req_ctx,
			     struct batch_context *batch_ctx,
			     struct chain_context *chain_ctx,
			     uint64_t req_id)
{
	req_ctx->batch_ctx = batch_ctx;
	req_ctx->chain_ctx = chain_ctx;
	req_ctx->req_id = req_id;
	memset(&req_ctx->svc_req, 0, sizeof(req_ctx->svc_req));
	memset(req_ctx->req_svcs, 0, sizeof(req_ctx->req_svcs));
	memset(&req_ctx->svc_res, 0, sizeof(req_ctx->svc_res));
	memset(req_ctx->res_statuses, 0, sizeof(req_ctx->res_statuses));
	req_ctx->req_rc = 0;
	copy_vars(batch_ctx->vars, req_ctx->vars);
	req_ctx->vars[TEST_VAR_CHAIN] = chain_ctx->svc_chain->node.idx;
}

static pnso_error_t run_testcase_batch(struct batch_context *batch_ctx)
{
	pnso_error_t err = PNSO_OK;
	const struct test_svc_chain *svc_chain;
	struct chain_context *chain_ctx;
	struct request_context *req_ctx = NULL;
	struct worker_context *worker_ctx = batch_get_worker_ctx(batch_ctx);
	struct testcase_context *test_ctx = batch_ctx->test_ctx;
	const struct test_testcase *testcase = test_ctx->testcase;
	uint32_t i, chain_i, lb_idx;

	PNSO_LOG_DEBUG("enter run_testcase_batch ...\n");

	if (!worker_ctx) {
		err = EINVAL;
		goto error;
	}

	batch_ctx->vars[TEST_VAR_ID] = testcase->node.idx;

	/* Run each request, alternating svc_chain */
	lb_idx = batch_ctx->first_req_id % test_ctx->chain_lb_table_count;
	for (i = 0; i < batch_ctx->req_count; i++, lb_idx++) {
		/* get svc_chain */
		chain_i = test_ctx->chain_lb_table[lb_idx % test_ctx->chain_lb_table_count];
		chain_ctx = test_ctx->chain_ctxs[chain_i];
		if (!chain_ctx) {
			PNSO_LOG_ERROR("Svc_chain %u not found for testcase %u\n",
				      chain_i, testcase->node.idx);
			err = EINVAL;
			goto error;
		}
		svc_chain = chain_ctx->svc_chain;
		batch_ctx->vars[TEST_VAR_CHAIN] = svc_chain->node.idx;

		req_ctx = batch_ctx->req_ctxs[i];
		if (!req_ctx) {
			PNSO_LOG_ERROR("Request context %u not found "
				       "for testcase %u\n",
				       i, testcase->node.idx);
			err = EINVAL;
			goto error;
		}
		init_req_context(req_ctx, batch_ctx, chain_ctx,
				 batch_ctx->first_req_id + i);
		err = run_testcase_svc_chain(req_ctx, testcase,
					     i, batch_ctx->req_count);
		if (err != PNSO_OK) {
			PNSO_LOG_INFO("Failed to submit request for req %u/%u, batch_id %u, worker_id %u\n",
				      i+1, batch_ctx->req_count, batch_ctx->batch_id, batch_ctx->worker_id);
			goto error;
		}
	}

	/* Special handling for SYNC and POLL */
	if (testcase->sync_mode == SYNC_MODE_SYNC) {
		/* Manually call completion callback */
		batch_completion_cb(batch_ctx,
				    req_ctx ? &req_ctx->svc_res : NULL);
	} else if (testcase->sync_mode == SYNC_MODE_POLL) {
		/* Place on poll queue */
		if (!worker_queue_enqueue(worker_ctx->poll_q, batch_ctx)) {
			PNSO_LOG_ERROR("Failed to enqueue batch_ctx to poll_q!\n");
			err = ENOENT;
			goto error;
		}
	}

	PNSO_LOG_DEBUG("... exit run_testcase_batch\n");

	return PNSO_OK;

error:
	/* Return the batch_ctx to the testcase thread, for stats aggregation */
	batch_ctx->stats.io_stats[0].failures++;
	batch_ctx->poll_fn = NULL; /* prevent polling on error */
	/* Manually call completion callback */
	if (testcase->sync_mode == SYNC_MODE_ASYNC) {
		batch_completion_safe_cb(batch_ctx,
			req_ctx ? &req_ctx->svc_res : NULL);
	} else {
		batch_completion_cb(batch_ctx,
			req_ctx ? &req_ctx->svc_res : NULL);
	}

	PNSO_LOG_DEBUG("... exit run_testcase_batch with err %d\n", err);

	return err;
}

static struct request_context *alloc_req_context(void)
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

	if (!ctx)
		return;

	if (ctx->input.buflist) {
		TEST_FREE(ctx->input.buflist);
	}
	for (i = 0; i < PNSO_SVC_TYPE_MAX; i++) {
		free_buffer_ctx(&ctx->outputs[i]);
	}

	TEST_FREE(ctx);
}

static void free_batch_context(struct batch_context *ctx)
{
	uint32_t i;

	if (!ctx) {
		return;
	}

	for (i = 0; i < ctx->max_req_count; i++) {
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

	ctx = (struct batch_context *) TEST_ALLOC(sizeof(*ctx));
	if (!ctx)
		return NULL;

	memset(ctx, 0, sizeof(*ctx));
	ctx->desc = desc;
	ctx->test_ctx = test_ctx;
	osal_atomic_init(&ctx->cb_count, 0);

	for (i = 0; i < test_ctx->max_batch_depth; i++) {
		req_ctx = alloc_req_context();
		if (!req_ctx) {
			goto error;
		}
		req_ctx->batch_ctx = ctx;
		ctx->req_ctxs[i] = req_ctx;
		ctx->max_req_count = i+1;
	}

	return ctx;

error:
	free_batch_context(ctx);
	return NULL;
}

static void init_batch_context(struct batch_context *ctx,
			       struct worker_context *work_ctx,
			       uint32_t batch_id,
			       uint64_t req_id, uint16_t req_count)
{
	OSAL_ASSERT(req_count && req_count <= ctx->max_req_count);
	osal_atomic_set(&ctx->cb_count, 0);
	ctx->worker_id = work_ctx->worker_id;
	ctx->batch_id = batch_id;
	ctx->first_req_id = req_id;
	ctx->req_count = req_count;
	ctx->req_rc = 0;
	ctx->poll_fn = NULL;
	ctx->poll_ctx = NULL;
	ctx->start_time = osal_get_clock_nsec();
	memset(&ctx->stats, 0, sizeof(ctx->stats));
	copy_vars(work_ctx->test_ctx->vars, ctx->vars);

	/* These were initialized during alloc */
	OSAL_ASSERT(ctx->desc);
	OSAL_ASSERT(ctx->test_ctx);
	OSAL_ASSERT(ctx->max_req_count);
}

static int worker_loop(void *param)
{
	struct worker_context *ctx = (struct worker_context *) param;
	struct batch_context *batch_ctx;
	int poll_err;
	bool is_busy;
	uint64_t cur_ts;

	while (pnso_test_is_active() &&
	       !osal_thread_should_stop(&ctx->worker_thread)) {
		is_busy = false;

		/* Poll for finished work item */
		batch_ctx = worker_queue_dequeue(ctx->poll_q);
		if (batch_ctx) {
			cur_ts = osal_get_clock_nsec();
			if (batch_ctx->req_rc != PNSO_OK)
				PNSO_LOG_ERROR("ERROR! Should not be polling for errored out batch_ctx!\n");

			poll_err = batch_ctx->poll_fn(batch_ctx->poll_ctx);
			if (poll_err == EBUSY) {
				/* Not ready yet, re-enqueue */
				_worker_queue_enqueue(ctx->poll_q, batch_ctx);
				batch_ctx = NULL;
			} else if (poll_err == PNSO_OK) {
				/* completion handler called by poll_fn */
				PNSO_LOG_DEBUG("DEBUG: polled batch completion\n");
				ctx->last_active_ts = cur_ts;
				is_busy = true;
			} else {
				/* error case, completion handler called by poll_fn */
				PNSO_LOG_INFO("DEBUG: poll_fn returned status %d\n",
					       poll_err);
				//batch_completion_cb(batch_ctx, NULL);
				ctx->last_active_ts = cur_ts;
				is_busy = true;
			}
		}

		/* Process next work item */
		batch_ctx = worker_queue_dequeue(ctx->submit_q);
		if (batch_ctx) {
#ifdef __KERNEL__
			int core_id = osal_get_coreid();

			if (core_id != ctx->worker_thread.core_id) {
				PNSO_LOG_ERROR("Unexpected core_id %d for worker on core %d.\n",
					       core_id, ctx->worker_thread.core_id);
				return -1;
			}
#endif
			cur_ts = osal_get_clock_nsec();
			ctx->last_active_ts = cur_ts;
			run_testcase_batch(batch_ctx);
			is_busy = true;
		}

		/* Be a good citizen */
		if (is_busy)
			osal_yield();
		else
			osal_sched_yield();
	}

	return 0;
}

static void free_worker_queue(struct worker_queue *q, bool free_entries)
{
	struct batch_context *batch_ctx;

	if (q) {
		if (free_entries) {
			while ((batch_ctx = worker_queue_dequeue(q))) {
				free_batch_context(batch_ctx);
			}
		}
		TEST_FREE(q);
	}
}

static struct worker_queue *alloc_worker_queue(uint32_t max_entries)
{
	struct worker_queue *q;

	q = TEST_ALLOC(sizeof(*q) + (max_entries * sizeof(struct batch_context *)));
	if (q) {
		memset(q, 0, sizeof(*q));
		osal_spin_lock_init(&q->lock);
		osal_atomic_init(&q->atomic_count, 0);
		q->head = 0;
		q->tail = 0;
		q->max_count = max_entries;
	}

	return q;
}

static void free_worker_context(struct worker_context *ctx)
{
	if (!ctx)
		return;
	if (osal_thread_is_running(&ctx->worker_thread)) {
		osal_thread_stop(&ctx->worker_thread);
	}
	free_worker_queue(ctx->submit_q, false);
	free_worker_queue(ctx->complete_q, false);
	free_worker_queue(ctx->poll_q, false);

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
	ctx->submit_q = alloc_worker_queue(test_ctx->batch_concurrency);
	ctx->complete_q = alloc_worker_queue(test_ctx->batch_concurrency);
	ctx->poll_q = alloc_worker_queue(test_ctx->batch_concurrency);
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

static void
free_chain_context(struct chain_context *ctx)
{
	if (!ctx)
		return;

	free_buffer_ctx(&ctx->input);
	TEST_FREE(ctx);
}

static void free_testcase_context(struct testcase_context *ctx)
{
	int i;

	if (!ctx)
		return;

	if (ctx->chain_lb_table)
		TEST_FREE(ctx->chain_lb_table);
	for (i = 0; i < ctx->chain_count; i++) {
		free_chain_context(ctx->chain_ctxs[i]);
	}
	for (i = 0; i < ctx->worker_count; i++) {
		free_worker_context(ctx->worker_ctxs[i]);
	}

	free_worker_queue(ctx->batch_ctx_freelist, false);
	free_worker_queue(ctx->batch_ctx_alloclist, true);
	TEST_FREE(ctx);
}

/* return max cores supported for given cpu_mask */
static uint16_t cpu_mask_to_core_count(uint64_t cpu_mask)
{
	uint16_t max_core_count = 0;

	/* mask off invalid cores */
	if (osal_get_core_count() < 64)
		cpu_mask &= ((1ULL << osal_get_core_count()) - 1);

	/* count bits */
	while (cpu_mask) {
		if (cpu_mask & 1)
			max_core_count++;
		cpu_mask >>= 1;
	}

	/* truncate to max supported */
	if (max_core_count > TEST_MAX_CORE_COUNT) {
		max_core_count = TEST_MAX_CORE_COUNT;
	}
	if (max_core_count > osal_get_core_count()) {
		max_core_count = osal_get_core_count();
	}

	return max_core_count;
}

static pnso_error_t init_testcase_svc_chains(struct testcase_context *test_ctx)
{
	pnso_error_t err = PNSO_OK;
	const struct test_testcase *testcase = test_ctx->testcase;
	uint32_t chain_i, lb_i;
	struct chain_context *chain_ctx;
	uint16_t weight;

	OSAL_ASSERT(testcase->svc_chain_count == test_ctx->chain_count);

	for (chain_i = 0; chain_i < test_ctx->chain_count; chain_i++) {
		chain_ctx = test_ctx->chain_ctxs[chain_i];

		/* Initialize input buffer */
		err = init_input_context(&chain_ctx->input, test_ctx,
					 chain_ctx->svc_chain);
		if (err != PNSO_OK) {
			PNSO_LOG_ERROR("Failed to init input context, testcase %u svc_chain %u\n",
				       testcase->node.idx, chain_ctx->svc_chain->node.idx);
			goto done;
		}
	}

	/* Initialize load balancing table for chains */
	test_ctx->chain_lb_table_count = test_ctx->total_chain_weight;
	test_ctx->chain_lb_table = TEST_ALLOC(sizeof(*test_ctx->chain_lb_table) *
					      test_ctx->chain_lb_table_count);
	if (!test_ctx->chain_lb_table) {
		err = ENOMEM;
		goto done;
	}
	chain_i = 0;
	weight = test_ctx->chain_ctxs[0]->batch_weight;
	for (lb_i = 0; lb_i < test_ctx->chain_lb_table_count; lb_i++) {
		test_ctx->chain_lb_table[lb_i] = chain_i;

		if (--weight == 0) {
			/* rotate to next chain */
			if (++chain_i >= test_ctx->chain_count) {
				chain_i = 0; /* wrap */
			}
			weight = test_ctx->chain_ctxs[chain_i]->batch_weight;
		}
	}

	PNSO_LOG_DEBUG("Init testcase %u with total_chain_weight %u\n",
		       testcase->node.idx, test_ctx->total_chain_weight);
done:
	return err;
}

static struct chain_context *
alloc_chain_context(struct testcase_context *test_ctx,
		    const struct test_svc_chain *svc_chain)
{
	struct chain_context *ctx;

	ctx = TEST_ALLOC(sizeof(*ctx));
	if (!ctx)
		return NULL;

	memset(ctx, 0, sizeof(*ctx));
	ctx->svc_chain = svc_chain;
	ctx->batch_weight = svc_chain->batch_weight ?
		svc_chain->batch_weight : 1;

	return ctx;
}

static struct testcase_context *alloc_testcase_context(const struct test_desc *desc,
						       const struct test_testcase *testcase)
{
	struct testcase_context *test_ctx;
	struct batch_context *batch_ctx;
	struct worker_context *worker_ctx;
	struct chain_context *chain_ctx;
	struct test_node *node;
	const struct test_svc_chain *svc_chain;
	int i, core_id;
	uint16_t max_core_count;
	uint32_t chain_idx;
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

	if (!testcase->svc_chain_count) {
		PNSO_LOG_ERROR("Need at least 1 svc_chain to run testcase %u\n",
			       testcase->node.idx);
		goto error;
	}

	/* Allocate chain contexts */
	test_ctx->max_chain_weight = 1;
	test_ctx->min_chain_weight = 1;
	for (i = 0; i < testcase->svc_chain_count; i++) {
		chain_idx = testcase->svc_chains[i];
		NODE_FIND_ID(test_ctx->desc->svc_chains, chain_idx);
		svc_chain = (const struct test_svc_chain *) node;
		if (!svc_chain) {
			PNSO_LOG_ERROR("Unable to find svc_chain %u used by testcase %u\n",
				       chain_idx, testcase->node.idx);
			goto error;
		}
		chain_ctx = alloc_chain_context(test_ctx, svc_chain);
		if (!chain_ctx) {
			PNSO_LOG_ERROR("Failed to allocate chain_context for testcase %u\n",
				       testcase->node.idx);
			goto error;
		}
		test_ctx->chain_ctxs[i] = chain_ctx;
		test_ctx->chain_count = i+1;

		/* Sum all the batch_weights */
		test_ctx->total_chain_weight += chain_ctx->batch_weight;
		if (chain_ctx->batch_weight > test_ctx->max_chain_weight)
			test_ctx->max_chain_weight = chain_ctx->batch_weight;
		if (chain_ctx->batch_weight < test_ctx->min_chain_weight)
			test_ctx->min_chain_weight = chain_ctx->batch_weight;
	}
	if (testcase->batch_depth) {
		test_ctx->max_batch_depth = testcase->batch_depth;
	} else {
		test_ctx->max_batch_depth = test_ctx->max_chain_weight;
		PNSO_LOG_INFO("Setting max_batch_depth to %u for testcase %u\n",
			      test_ctx->max_batch_depth, testcase->node.idx);
	}
	OSAL_ASSERT(test_ctx->max_batch_depth);
	OSAL_ASSERT(test_ctx->max_batch_depth <= TEST_MAX_BATCH_DEPTH);

	max_core_count = cpu_mask_to_core_count(desc->cpu_mask);
	if (!max_core_count) {
		PNSO_LOG_ERROR("Cannot run testcase %u with 0 cores\n",
			       testcase->node.idx);
		goto error;
	}

	test_ctx->batch_concurrency = testcase->batch_concurrency;
	if (!test_ctx->batch_concurrency) {
		/* Pick a sane value automatically */
		test_ctx->batch_concurrency =
			desc->init_params.per_core_qdepth / test_ctx->max_batch_depth;
		if (!test_ctx->batch_concurrency)
			test_ctx->batch_concurrency = 1;
		else if (test_ctx->batch_concurrency > TEST_MAX_BATCH_CONCURRENCY)
			test_ctx->batch_concurrency = TEST_MAX_BATCH_CONCURRENCY;
		PNSO_LOG_INFO("Setting batch_concurrency to %u for testcase %u\n",
			      test_ctx->batch_concurrency, testcase->node.idx);
	}

	/* Allocate freelist and alloclist and fill with batch_ctx entries */
	test_ctx->batch_ctx_freelist = alloc_worker_queue(max_core_count*test_ctx->batch_concurrency);
	test_ctx->batch_ctx_alloclist = alloc_worker_queue(max_core_count*test_ctx->batch_concurrency);
	if (!test_ctx->batch_ctx_freelist || !test_ctx->batch_ctx_alloclist) {
		goto error;
	}
	for (core_id = 0;
	     core_id < osal_get_core_count() && worker_count < max_core_count;
	     core_id++) {
		if ((desc->cpu_mask & (1 << core_id)) == 0) {
			continue;
		}
		for (i = 0; i < test_ctx->batch_concurrency; i++) {
			batch_ctx = alloc_batch_context(desc, test_ctx);
			if (!batch_ctx) {
				goto error;
			}
			_worker_queue_enqueue(test_ctx->batch_ctx_freelist, batch_ctx);
			_worker_queue_enqueue(test_ctx->batch_ctx_alloclist, batch_ctx);
		}

		worker_ctx = alloc_worker_context(desc, test_ctx, core_id);
		if (!worker_ctx) {
			goto error;
		}
		worker_ctx->worker_id = worker_count;
		test_ctx->worker_ctxs[worker_count++] = worker_ctx;
		test_ctx->worker_count = worker_count;
	}

	PNSO_LOG_INFO("Allocated %u worker contexts for testcase %u\n",
		      worker_count, testcase->node.idx);

	return test_ctx;

error:
	PNSO_LOG_ERROR("Failed to allocate testcase context for testcase %u\n",
		       testcase->node.idx);
	free_testcase_context(test_ctx);
	return NULL;
}

static pnso_error_t start_worker_thread(struct worker_context *ctx)
{
	if (ctx == NULL)
		return EINVAL;

	ctx->last_active_ts = osal_get_clock_nsec();
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

static void print_worker_queue(struct worker_queue *q, const char *name, bool verbose)
{
	int count = osal_atomic_read(&q->atomic_count);

	OSAL_LOG_ERROR("  Queue %s, count %d:\n", name, count);
	if (count || verbose) {
		OSAL_LOG_ERROR("    enqueues: success %llu, full %llu, empty %llu\n",
			       (unsigned long long) q->enqueue_count,
			       (unsigned long long) q->enqueue_full_count,
			       (unsigned long long) q->enqueue_empty_count);
		OSAL_LOG_ERROR("    dequeues: success %llu, full %llu, empty %llu\n",
			       (unsigned long long) q->dequeue_count,
			       (unsigned long long) q->dequeue_full_count,
			       (unsigned long long) q->dequeue_empty_count);
		OSAL_LOG_ERROR("    head %u, tail %u, depth %u\n",
			       q->head, q->tail, q->max_count);
	}
}

static void print_worker_ctx(struct worker_context *work_ctx, uint64_t cur_ts, bool verbose)
{
	OSAL_LOG_ERROR("Worker %u, pending_batch_count %u, idle_time %llums\n",
		       work_ctx->worker_id, work_ctx->pending_batch_count,
		       (unsigned long long) (cur_ts - work_ctx->last_active_ts)/OSAL_NSEC_PER_MSEC);
	if (work_ctx->pending_batch_count || verbose) {
		print_worker_queue(work_ctx->submit_q, "submit_q", verbose);
		print_worker_queue(work_ctx->complete_q, "complete_q", verbose);
		print_worker_queue(work_ctx->poll_q, "poll_q", verbose);
	}
}

static void print_testcase_ctx(struct testcase_context *ctx)
{
	int i;
	uint64_t cur_ts = osal_get_clock_nsec();
	bool verbose = (g_osal_log_level >= OSAL_LOG_LEVEL_INFO);

	OSAL_LOG_ERROR("Testcase %u\n", ctx->testcase->node.idx);
	print_worker_queue(ctx->batch_ctx_freelist, "batch_ctx_freelist", verbose);

	for (i = 0; i < ctx->worker_count; i++) {
		print_worker_ctx(ctx->worker_ctxs[i], cur_ts, verbose);
	}
}

static bool need_rate_limit(struct testcase_context *ctx, uint64_t elapsed_time)
{
	uint64_t in_rate, out_rate;

	if (ctx->desc->limit_rate == 0)
		return false;

	in_rate = calculate_bytes_per_sec(ctx->stats.io_stats[0].bytes, elapsed_time);
	out_rate = calculate_bytes_per_sec(ctx->stats.io_stats[1].bytes, elapsed_time);

	return (in_rate > ctx->desc->limit_rate || out_rate > ctx->desc->limit_rate);
}

#define TESTCASE_IDLE_LOOP_TIMEOUT (5 * OSAL_NSEC_PER_SEC)
#define TESTCASE_LOOP_RESOLUTION_MASK ((1 << 6) - 1)

static pnso_error_t pnso_test_run_testcase(const struct test_desc *desc,
					   const struct test_testcase *testcase)
{
	pnso_error_t err = PNSO_OK;
	struct testcase_context *ctx;
	struct worker_context *worker_ctx;
	struct batch_context *batch_ctx;
	struct chain_context *chain_ctx;
	uint64_t batch_completion_count = 0;
	uint64_t batch_completion_empty_count = 0;
	uint64_t batch_submit_count = 0;
	uint64_t batch_submit_full_count = 0;
	uint64_t req_submit_count = 0;
	uint64_t req_completion_count = 0;
	uint64_t fail_count = 0;
	uint64_t last_active_ts = 0;
	uint64_t cur_ts;
	uint64_t idle_time = 0;
	uint64_t elapsed_time = 0;
	uint64_t rate_limit_loop_count = 0;
	uint64_t max_idle_time = 0;
	uint64_t loop_count = 0;
	uint32_t next_status_time;
	uint32_t cur_batch_depth;
	bool b_shutting_down = false;
	int worker_id;

	PNSO_LOG_DEBUG("enter pnso_test_run_testcase(%u) ...\n",
		       testcase ? testcase->node.idx : 0);

	ctx = alloc_testcase_context(desc, testcase);
	if (!ctx) {
		PNSO_LOG_ERROR("Cannot allocate context for testcase %u\n",
			       testcase->node.idx);
		return ENOMEM;
	}
	init_testcase_svc_chains(ctx);

	ctx->vars[TEST_VAR_BLOCK_SIZE] = desc->init_params.block_size;
	ctx->vars[TEST_VAR_ITER] = 0;

	/* Start worker threads */
	osal_atomic_set(&g_testcase_active, 1);
	if (start_worker_threads(ctx) != PNSO_OK) {
		PNSO_LOG_WARN("Unable to start all worker threads\n");
		/* continue execution, since some threads may be functional */
	} else {
		PNSO_LOG_INFO("Starting %u worker threads\n", ctx->worker_count);
	}

	/* Submit batches to each worker thread in turn, until done */
	cur_batch_depth = testcase->batch_depth;
	worker_id = 0;
	last_active_ts = osal_get_clock_nsec();
	cur_ts = last_active_ts;
	ctx->start_time = last_active_ts;
	next_status_time = desc->status_interval;
	PNSO_LOG_DEBUG("DEBUG: entering testcase while loop\n");
	while (req_completion_count < testcase->repeat) {
		loop_count++;
		worker_ctx = ctx->worker_ctxs[worker_id];

		/* Batch completion */
		batch_ctx = worker_queue_dequeue(worker_ctx->complete_q);
		if (batch_ctx) {
			PNSO_LOG_DEBUG("DEBUG: batch completed, batch_count %llu\n",
				(unsigned long long) batch_completion_count+1);
			last_active_ts = osal_get_clock_nsec();
			batch_ctx_p2v(batch_ctx);
			/* process finished batch, and restore to freelist */
			run_batch_validation(batch_ctx);
			calculate_completion_stats(batch_ctx);
			aggregate_testcase_stats(&ctx->stats, &batch_ctx->stats,
					last_active_ts - ctx->start_time);
			batch_completion_count++;
			req_completion_count += batch_ctx->req_count;
			_worker_queue_enqueue(ctx->batch_ctx_freelist, batch_ctx);
			worker_ctx->pending_batch_count--;
		} else {
			batch_completion_empty_count++;
		}

		/* Batch submission */
		if (b_shutting_down ||
		    (req_submit_count >= testcase->repeat)) {
			/* No more requests to submit */
		} else if (need_rate_limit(ctx, elapsed_time)) {
			/* skip idle timeout during active rate limiting */
//			last_active_ts = osal_get_clock_nsec();
			rate_limit_loop_count++;
		} else if (worker_ctx->pending_batch_count < ctx->batch_concurrency) {
			/* submit new batch request */
			batch_ctx = worker_queue_dequeue(ctx->batch_ctx_freelist);
			if (batch_ctx) {
				PNSO_LOG_DEBUG("DEBUG: begin batch submission, worker %u, batch_count %llu\n",
					       worker_id, (unsigned long long) batch_submit_count+1);
				ctx->vars[TEST_VAR_ITER]++;
				if (!testcase->batch_depth) {
					/* dynamic batch depth, alternate chains */
					chain_ctx = ctx->chain_ctxs[batch_submit_count % ctx->chain_count];
					cur_batch_depth = chain_ctx->batch_weight;
				}
				init_batch_context(batch_ctx, worker_ctx, batch_submit_count,
						   req_submit_count, cur_batch_depth);

				if (worker_queue_enqueue(worker_ctx->submit_q, batch_ctx)) {
					batch_submit_count++;
					req_submit_count += cur_batch_depth;
					last_active_ts = osal_get_clock_nsec();
					worker_ctx->pending_batch_count++;
				} else {
					PNSO_LOG_ERROR("Fail batch submission, worker %u, batch_count %llu\n",
						worker_id, (unsigned long long) batch_submit_count+1);
					fail_count++;
					_worker_queue_enqueue(ctx->batch_ctx_freelist, batch_ctx);
				}
			}
		} else if (osal_clock_delta(cur_ts, worker_ctx->last_active_ts) >
			   TESTCASE_IDLE_LOOP_TIMEOUT) {
			PNSO_LOG_WARN("break out of run_testcase loop due to "
				      "idle timeout on worker %u\n",
				      worker_id);
			err = ETIMEDOUT;
			print_testcase_ctx(ctx);
			break;
		} else {
			batch_submit_full_count++;
		}

		/* Operations limited to run occasionally */
		if ((loop_count & TESTCASE_LOOP_RESOLUTION_MASK) == 0) {
			cur_ts = osal_get_clock_nsec();

			/* Update elapsed_time */
			elapsed_time = cur_ts - ctx->start_time;

			/* Break out if nothing's happening */
			idle_time = cur_ts - last_active_ts;
			if (idle_time > max_idle_time) {
				max_idle_time = idle_time;
			}
			if (idle_time >= TESTCASE_IDLE_LOOP_TIMEOUT) {
				PNSO_LOG_WARN("break out of run_testcase loop due to idle timeout\n");
				err = ETIMEDOUT;
				print_testcase_ctx(ctx);
				break;
			}

			/* Check whether it's time to output stats */
			if (desc->status_interval) {
				if ((elapsed_time / OSAL_NSEC_PER_SEC) > next_status_time) {
					pnso_test_stats_to_yaml(testcase,
						(uint64_t*) (&ctx->stats),
						testcase_stats_names,
						TESTCASE_STATS_COUNT, false, ctx);
					next_status_time += desc->status_interval;

					PNSO_LOG("PenCAKE Testcase %u %s: status %d, elapsed_time %llums, "
						 "req_submit_count %llu, max_idle_time_ns %llu, "
						 "batch_submit_count %llu, batch_completion_count %llu, "
						 "rate_limit_loops %llu\n",
						 testcase->node.idx, testcase->name, err,
						 (unsigned long long) (elapsed_time /
								       OSAL_NSEC_PER_MSEC),
						 (unsigned long long) req_submit_count,
						 (unsigned long long) max_idle_time,
						 (unsigned long long) batch_submit_count,
						 (unsigned long long) batch_completion_count,
						 (unsigned long long) rate_limit_loop_count);
				}
			}
			osal_sched_yield();
		} else {
			osal_yield();
		}

		/* Iterate workers on each loop */
		if (++worker_id >= ctx->worker_count) {
			worker_id = 0;
			b_shutting_down = (bool) osal_atomic_read(&g_shutdown);
		}
	}
	osal_atomic_set(&g_testcase_active, 0);
	PNSO_LOG_DEBUG("DEBUG: exiting testcase while loop\n");

	//if (err != ETIMEDOUT &&
	//    g_osal_log_level >= OSAL_LOG_LEVEL_INFO) {
	//	print_testcase_ctx(ctx);
	//}

	/* Final tally for stats */
	elapsed_time = osal_get_clock_nsec() - ctx->start_time;
	pnso_test_stats_to_yaml(testcase,
				(uint64_t*) (&ctx->stats),
				testcase_stats_names,
				TESTCASE_STATS_COUNT,
				true, ctx);

	PNSO_LOG("PenCAKE Testcase %u %s: status %d, elapsed_time %llums, "
		 "req_submit_count %llu, max_idle_time_ns %llu, "
		 "batch_submit_count %llu, batch_submit_full_count %llu, "
		 "batch_completion_count %llu, batch_completion_empty_count %llu, "
		 "rate_limit_loops %llu\n",
		 testcase->node.idx, testcase->name, err,
		 (unsigned long long) (elapsed_time /
				       OSAL_NSEC_PER_MSEC),
		 (unsigned long long) req_submit_count,
		 (unsigned long long) max_idle_time,
		 (unsigned long long) batch_submit_count,
		 (unsigned long long) batch_submit_full_count,
		 (unsigned long long) batch_completion_count,
		 (unsigned long long) batch_completion_empty_count,
		 (unsigned long long) rate_limit_loop_count);

	PNSO_LOG_DEBUG("DEBUG: pnso_test_run_testcase freeing test context\n");
	free_testcase_context(ctx);

	PNSO_LOG_DEBUG("... exit pnso_test_run_testcase(%u)\n",
		       testcase ? testcase->node.idx : 0);

	return err;
}

pnso_error_t pnso_test_run_all(struct test_desc *desc)
{
	pnso_error_t err = PNSO_OK;
	struct test_node *node;
	struct pnso_init_params init_params;

	if (desc->node.type != NODE_ROOT) {
		PNSO_LOG_ERROR("Invalid test description\n");
		return EINVAL;
	}

	init_params = desc->init_params;
	init_params.core_count = cpu_mask_to_core_count(desc->cpu_mask);
	err = pnso_init(&init_params);
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
				return PNSO_ERR_CRYPTO_WRONG_KEY_TYPE;
			}
			if (0 != memcmp(tmp_key, key->key1, tmp_key_size)) {
				PNSO_LOG_ERROR("key data doesn't match\n");
				return PNSO_ERR_CRYPTO_KEY_NOT_REGISTERED;
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
		if (osal_atomic_read(&g_shutdown))
			break;
	}

	/* Store output files */
#ifndef __KERNEL__
	if (desc->store_output_files) {
		struct test_node_list *list;
		struct test_file_table *file_tbl = test_get_output_file_table();
		int bucket;
		int success_count = 0;
		int fail_count = 0;

		FOR_EACH_TABLE_BUCKET(file_tbl->table) {
			FOR_EACH_NODE(*list) {
				struct test_node_file *fnode =
					(struct test_node_file *) node;
				if (test_write_file(fnode->filename,
					fnode->buflist,
					fnode->file_size > fnode->padded_size ?
					fnode->file_size : fnode->padded_size,
					0) == PNSO_OK) {
					success_count++;
				} else {
					fail_count++;
				}
			}
		}
		PNSO_LOG_INFO("Stored %d out of %d output files.\n",
			      success_count, success_count + fail_count);
	} else {
		PNSO_LOG_INFO("Skipping storage of '%s*%s' output files.\n",
			      desc->output_file_prefix, desc->output_file_suffix);
	}
#endif
	test_free_output_file_table();

	return err;
}

#define UNIT_TEST_BUFLIST_PATTERN "abcdefghijklmnopqrstuvwxyz01345"
#define UNIT_TEST_BUFLIST_COUNT 2
#define UNIT_TEST_BUFLIST_SIZE 16384
pnso_error_t pnso_run_unit_tests(struct test_desc *desc)
{
	pnso_error_t err = PNSO_OK;
	size_t i;
	char reason[80] = "success";
	struct buffer_context buf_ctxs[UNIT_TEST_BUFLIST_COUNT];

	memset(buf_ctxs, 0, sizeof(buf_ctxs));
	for (i = 0; i < UNIT_TEST_BUFLIST_COUNT; i++) {
		if (alloc_buffer_ctx(&buf_ctxs[i],
				     (i+1)*2, UNIT_TEST_BUFLIST_SIZE,
				     desc->init_params.block_size,
				     false) != PNSO_OK) {
			err = ENOMEM;
			safe_strcpy(reason, "alloc_buffer_ctx",
				    sizeof(reason));
			goto done;
		}
		err = test_fill_pattern(buf_ctxs[i].buflist, UNIT_TEST_BUFLIST_PATTERN,
					strlen(UNIT_TEST_BUFLIST_PATTERN));
		if (err) {
			safe_strcpy(reason, "test_fill_pattern",
				    sizeof(reason));
			goto done;
		}
		if (0 != test_cmp_pattern(buf_ctxs[i].buflist, 0, UNIT_TEST_BUFLIST_SIZE,
					  UNIT_TEST_BUFLIST_PATTERN,
					  strlen(UNIT_TEST_BUFLIST_PATTERN))) {
			err = EINVAL;
			safe_strcpy(reason, "test_cmp_pattern",
				    sizeof(reason));
			goto done;
		}
		if (i >= 1) {
			if (0 != cmp_buflists(buf_ctxs[i-1].buflist, buf_ctxs[i].buflist,
					      0, UNIT_TEST_BUFLIST_SIZE)) {
				err = EINVAL;
				safe_strcpy(reason, "cmp_buflists",
					    sizeof(reason));
				goto done;
			}
		}
	}

done:
	for (i = 0; i < UNIT_TEST_BUFLIST_COUNT; i++) {
		free_buffer_ctx(&buf_ctxs[i]);
	}
	PNSO_LOG_INFO("Unit tests completed with status %d, reason %s.\n",
		      err, reason);
	return err;
}
