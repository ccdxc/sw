
#include "LZR_model.h"

#include "pnso_sim.h"
#include "pnso_sim_algo.h"
#include "pnso_sim_util.h"
#include "pnso_wafl.h"
#include "../osal/pnso_sim_osal.h"

#ifndef PNSO_ASSERT
extern void abort();
#define PNSO_ASSERT(x)  if (!(x)) { abort(); }
#endif

struct pnso_sim_session;

struct pnso_sim_svc_ctx {
	struct pnso_sim_session *sess;
	struct pnso_service cmd;
	struct pnso_service_status status;
	uint32_t is_first:1, is_last:1, rsvrd:30;
	struct pnso_flat_buffer input;
	struct pnso_flat_buffer output;
};

struct pnso_sim_scratch_region {
	uint8_t *cmd;		/* scratch for commands */
	uint8_t *data[2];	/* for bank switching, to avoid duplicate input/output */
	uint32_t data_sz;	/* size of each data region */
};

typedef pnso_error_t(*svc_exec_func_t) (struct pnso_sim_svc_ctx * ctx,
					void *opaque);

struct pnso_sim_session {
	bool is_valid;
	uint32_t block_sz;
	struct pnso_sim_scratch_region scratch;
	svc_exec_func_t funcs[PNSO_SVC_TYPE_MAX];
};

static struct pnso_sim_session g_sessions[PNSO_MAX_SESSIONS];
static uint32_t g_session_count = 0;

static pnso_error_t svc_exec_noop(struct pnso_sim_svc_ctx *ctx,
				  void *opaque);
static pnso_error_t svc_exec_compress(struct pnso_sim_svc_ctx *ctx,
				      void *opaque);
static pnso_error_t svc_exec_decompress(struct pnso_sim_svc_ctx *ctx,
					void *opaque);
static pnso_error_t svc_exec_encrypt(struct pnso_sim_svc_ctx *ctx,
				     void *opaque);
static pnso_error_t svc_exec_decrypt(struct pnso_sim_svc_ctx *ctx,
				     void *opaque);
static pnso_error_t svc_exec_pad(struct pnso_sim_svc_ctx *ctx,
				 void *opaque);
static pnso_error_t svc_exec_hash(struct pnso_sim_svc_ctx *ctx,
				  void *opaque);
static pnso_error_t svc_exec_chksum(struct pnso_sim_svc_ctx *ctx,
				    void *opaque);
static pnso_error_t svc_exec_decompact(struct pnso_sim_svc_ctx *ctx,
				       void *opaque);

#define CMD_SCRATCH_SZ (16 * 1024)

static int pnso_sim_init_session(struct pnso_sim_session *sess,
				 svc_exec_func_t * funcs,
				 uint8_t * scratch, uint32_t scratch_sz)
{
	size_t func_i;

	/* Use default function in case of undefined func */
	for (func_i = 0; func_i < PNSO_SVC_TYPE_MAX; func_i++) {
		/* Set svc_exec command functions */
		sess->funcs[func_i] = funcs[func_i];
		if (sess->funcs[func_i] == NULL) {
			switch (func_i) {
			case PNSO_SVC_TYPE_ENCRYPT:
				sess->funcs[func_i] = svc_exec_encrypt;
				break;
			case PNSO_SVC_TYPE_DECRYPT:
				sess->funcs[func_i] = svc_exec_decrypt;
				break;
			case PNSO_SVC_TYPE_COMPRESS:
				sess->funcs[func_i] = svc_exec_compress;
				break;
			case PNSO_SVC_TYPE_DECOMPRESS:
				sess->funcs[func_i] = svc_exec_decompress;
				break;
			case PNSO_SVC_TYPE_HASH:
				sess->funcs[func_i] = svc_exec_hash;
				break;
			case PNSO_SVC_TYPE_CHKSUM:
				sess->funcs[func_i] = svc_exec_chksum;
				break;
			case PNSO_SVC_TYPE_PAD:
				sess->funcs[func_i] = svc_exec_pad;
				break;
			case PNSO_SVC_TYPE_DECOMPACT:
				sess->funcs[func_i] = svc_exec_decompact;
				break;
			default:
				sess->funcs[func_i] = svc_exec_noop;
				break;
			}
		}
	}

	if (scratch_sz <= CMD_SCRATCH_SZ) {
		return -1;
	}

	sess->scratch.cmd = scratch;
	scratch += CMD_SCRATCH_SZ;
	scratch_sz -= CMD_SCRATCH_SZ;
	sess->scratch.data_sz = scratch_sz / 2;
	sess->scratch.data[0] = scratch;
	sess->scratch.data[1] = scratch + (scratch_sz / 2);

	sess->block_sz = PNSO_DEFAULT_BLOCK_SZ;
	sess->is_valid = true;

	return 0;
}

/* Assumes scratch_sz is a power of two */
int pnso_sim_init(uint32_t sess_count, uint8_t * scratch,
		  uint32_t scratch_sz)
{
	int rc = 0;
	size_t sess_i;
	svc_exec_func_t default_funcs[PNSO_SVC_TYPE_MAX];
	uint8_t *sess_scratch;
	uint32_t sess_scratch_sz;

	memset(g_sessions, 0, sizeof(g_sessions));
	memset(default_funcs, 0, sizeof(default_funcs));

	if (sess_count > PNSO_MAX_SESSIONS) {
		return -1;
	}

	/* carve up the scratch region for use by all sessions */
	sess_scratch_sz = scratch_sz / sess_count;
	for (sess_i = 0; sess_i < sess_count; sess_i++) {
		/* Initialize each session */
		sess_scratch = scratch + (sess_scratch_sz * sess_i);
		rc = pnso_sim_init_session(&g_sessions[sess_i],
					   default_funcs, sess_scratch,
					   sess_scratch_sz);
		if (rc != 0) {
			break;
		}
	}

	pnso_sim_init_queues();

	g_session_count = sess_count;
	return rc;
}

static pnso_error_t svc_exec_memcpy(struct pnso_sim_svc_ctx *ctx,
				    void *opaque)
{
	uint32_t len = ctx->input.len;
	if (len > ctx->output.len) {
		len = ctx->output.len;
	}

	memcpy((uint8_t *) ctx->output.buf, (uint8_t *) ctx->input.buf,
	       len);
	ctx->output.len = len;
	return PNSO_OK;
}

static pnso_error_t svc_exec_noop(struct pnso_sim_svc_ctx *ctx,
				  void *opaque)
{
	ctx->output = ctx->input;
	return PNSO_OK;
}

static pnso_error_t execute_service(struct pnso_sim_session *session,
				    struct pnso_sim_svc_ctx *ctx,
				    void *opaque)
{
	pnso_error_t rc = PNSO_OK;

	PNSO_ASSERT(session->funcs[ctx->cmd.svc_type]);
	rc = session->funcs[ctx->cmd.svc_type] (ctx, opaque);

	ctx->status.output_data_len = ctx->output.len;
	ctx->status.svc_type = ctx->cmd.svc_type;
	ctx->status.err = rc;

	return rc;
}

static struct pnso_sim_session *pnso_sim_get_session(uint32_t sess_id)
{
	/* Use start index of 1 */
	if (sess_id > 0 && sess_id <= g_session_count) {
		return &g_sessions[sess_id - 1];
	}
	return NULL;
}

pnso_error_t pnso_sim_execute_request(uint32_t sess_id,
				      struct pnso_service_request *
				      svc_req,
				      struct pnso_service_result * svc_res,
				      completion_t cb, void *cb_ctx)
{
	struct pnso_sim_session *sess = pnso_sim_get_session(sess_id);
	int rc = 0;
	size_t svc_i, scratch_bank = 0;
	struct pnso_sim_svc_ctx svc_ctxs[2];
	struct pnso_sim_svc_ctx *cur_svc = &svc_ctxs[0];
	struct pnso_sim_svc_ctx *prev_svc = NULL;

	PNSO_ASSERT(svc_req->num_services == svc_res->num_services);
	PNSO_ASSERT(svc_req->num_services >= 1);

	memset(svc_ctxs, 0, sizeof(svc_ctxs));
	svc_ctxs[0].sess = svc_ctxs[1].sess = sess;

	/* Setup first service, copy input data to flat scratch buffer */
	cur_svc = &svc_ctxs[0];
	cur_svc->input.buf = (uint64_t) sess->scratch.data[scratch_bank];
	cur_svc->input.len = sess->scratch.data_sz;
	pnso_memcpy_list_to_flat_buf(&cur_svc->input, svc_req->src_buf);
	cur_svc->is_first = 1;

	for (svc_i = 0; svc_i < svc_req->num_services; svc_i++) {
		/* Setup command */
		cur_svc->cmd = svc_req->svc[svc_i];
		if (cur_svc->cmd.svc_type >= PNSO_SVC_TYPE_MAX) {
			rc = -1;
			goto error;
		}
		cur_svc->status = svc_res->status[svc_i];

		/* Setup input buffer */
		if (prev_svc != NULL) {
			/* Use previous service output  */
			cur_svc->input = prev_svc->output;
			cur_svc->is_first = 0;
		}

		/* Setup output buffer */
		if (cur_svc->input.buf ==
		    (uint64_t) sess->scratch.data[scratch_bank]) {
			/* swap scratch regions, so input and output are different */
			scratch_bank ^= 1;
		}
		cur_svc->output.buf =
		    (uint64_t) sess->scratch.data[scratch_bank];
		cur_svc->output.len = sess->scratch.data_sz;
		if (svc_i == svc_req->num_services - 1) {
			/* Last service */
			cur_svc->is_last = 1;
		}

		/* Execute service */
		rc = execute_service(sess, cur_svc, NULL /* TODO */ );
		svc_res->status[svc_i] = cur_svc->status;
		if (rc != 0) {
			/* TODO: continue?? */
			goto error;
		}

		prev_svc = cur_svc;
	}

	/* Copy final result from scratch buffer to request buffer */
	pnso_memcpy_flat_buf_to_list(svc_req->dst_buf, &cur_svc->output);

      error:
	return rc;
}

pnso_error_t pnso_submit_request(uint32_t sess_id,
				 enum pnso_batch_request batch_req,
				 struct pnso_service_request * svc_req,
				 struct pnso_service_result * svc_res,
				 completion_t cb,
				 void *cb_ctx,
				 poller_t * poll_fn, void **poll_ctx)
{
	pnso_error_t rc;

	if (cb == NULL) {
		/* Synchronous request, execute now.  Batching and polling ignored. */
		return pnso_sim_execute_request(sess_id, svc_req, svc_res,
						NULL, NULL);
	}

	rc = pnso_sim_sq_enqueue(sess_id, batch_req, svc_req, svc_res, cb,
				 cb_ctx, poll_ctx);
	if (poll_fn) {
		*poll_fn = pnso_sim_poll;
	}
	return rc;
}

static inline uint32_t flat_buffer_block_count(const struct
					       pnso_flat_buffer *buf,
					       uint32_t block_sz)
{
	return (buf->len + (block_sz - 1)) / block_sz;
}

static inline void flat_buffer_to_block(const struct pnso_flat_buffer *src,
					struct pnso_flat_buffer *dst,
					uint32_t block_sz,
					uint32_t block_idx)
{
	dst->buf = src->buf + (block_sz * block_idx);
	if (src->len >= (block_sz * (block_idx + 1))) {
		/* Full block */
		dst->len = block_sz;
	} else if (src->len >= (block_sz * block_idx)) {
		/* Last and partial block */
		dst->len = src->len % block_sz;
	} else {
		/* Beyond last block */
		dst->len = 0;
	}
}

/* Modifies buffer to add zero padding to end of last block.
 * Returns number of bytes appended.
 */
static uint32_t flat_buffer_pad(struct pnso_flat_buffer *buf,
				uint32_t block_sz)
{
	uint32_t pad_len = 0;
	uint32_t block_count = flat_buffer_block_count(buf, block_sz);
	struct pnso_flat_buffer block;

	flat_buffer_to_block(buf, &block, block_sz, block_count - 1);

	if (block.len && block.len < block_sz) {
		/* Pad block with zeroes in-place */
		uint32_t pad_len = block_sz - block.len;
		memset((uint8_t *) (block.buf + block.len), 0, pad_len);
		buf->len += pad_len;
	}

	return pad_len;
}

/* Modifies input buffer to add zero padding to end of last block,
 * where block size is given by sess->block_sz.
 */
static pnso_error_t svc_exec_pad(struct pnso_sim_svc_ctx *ctx,
				 void *opaque)
{
	flat_buffer_pad(&ctx->input, ctx->sess->block_sz);

	/* Zero-copy output */
	ctx->output = ctx->input;

	return PNSO_OK;
}

static pnso_error_t svc_exec_compress_lzrw1a(struct pnso_sim_svc_ctx *ctx)
{
	struct pnso_compression_header *hdr =
	    (struct pnso_compression_header *) ctx->output.buf;
	uint32_t hdr_len = ctx->cmd.req.cp_desc.header_len;
	uint32_t dst_len = ctx->sess->scratch.data_sz - hdr_len;

	if (lzrw1a_compress
	    (COMPRESS_ACTION_COMPRESS, ctx->sess->scratch.cmd,
	     (uint8_t *) ctx->input.buf, ctx->input.len,
	     (uint8_t *) ctx->output.buf + hdr_len, &dst_len, 0)) {
		ctx->output.len = dst_len + hdr_len;
		if (hdr_len >= sizeof(*hdr)) {
			hdr->chksum = 0;
			hdr->data_len = dst_len;
			hdr->version = 1;	/* TODO */
		}
		return PNSO_OK;
	}
	return PNSO_ERR_CPDC_COMPRESSION_FAILED;
}

static pnso_error_t svc_exec_decompress_lzrw1a(struct pnso_sim_svc_ctx
					       *ctx)
{
	struct pnso_compression_header *hdr =
	    (struct pnso_compression_header *) ctx->input.buf;
	uint32_t hdr_len = ctx->cmd.req.dc_desc.header_len;
	uint32_t dst_len = ctx->sess->scratch.data_sz;

	PNSO_ASSERT(ctx->input.len >= hdr_len);
	PNSO_ASSERT(!hdr_len || ctx->input.len - hdr_len >= hdr->data_len);

	/* Note: currently lzrw1a decompress does not provide error status */
	lzrw1a_compress(COMPRESS_ACTION_DECOMPRESS, ctx->sess->scratch.cmd,
			(uint8_t *) ctx->input.buf + hdr_len,
			hdr_len ? hdr->data_len : ctx->input.len,
			(uint8_t *) ctx->output.buf, &dst_len, 0);
	ctx->output.len = dst_len;
	return PNSO_OK;
}

static pnso_error_t svc_exec_compress(struct pnso_sim_svc_ctx *ctx,
				      void *opaque)
{
	pnso_error_t rc = PNSO_OK;

	memset(ctx->sess->scratch.cmd, 0, CMD_SCRATCH_SZ);

	/* TODO: obey threshold value */

	switch (ctx->cmd.algo_type) {
	case PNSO_COMPRESSOR_TYPE_NONE:
		rc = svc_exec_noop(ctx, opaque);
		break;
	case PNSO_COMPRESSOR_TYPE_LZRW1A:
		rc = svc_exec_compress_lzrw1a(ctx);
		break;
	default:
		rc = PNSO_ERR_CPDC_AXI_DATA_ERROR;	/* TODO */
		break;
	}

	if (rc == PNSO_OK && ctx->cmd.req.cp_desc.do_pad) {
		flat_buffer_pad(&ctx->output, ctx->sess->block_sz);
	}

	return rc;
}

static pnso_error_t svc_exec_decompress(struct pnso_sim_svc_ctx *ctx,
					void *opaque)
{
	pnso_error_t rc = 0;

	memset(ctx->sess->scratch.cmd, 0, CMD_SCRATCH_SZ);

	switch (ctx->cmd.algo_type) {
	case PNSO_COMPRESSOR_TYPE_NONE:
		rc = svc_exec_noop(ctx, opaque);
		break;
	case PNSO_COMPRESSOR_TYPE_LZRW1A:
		rc = svc_exec_decompress_lzrw1a(ctx);
		break;
	default:
		rc = PNSO_ERR_CPDC_AXI_DATA_ERROR;	/* TODO */
		break;
	}

	return rc;
}

static pnso_error_t svc_exec_encrypt(struct pnso_sim_svc_ctx *ctx,
				     void *opaque)
{
	uint8_t *key1, *key2;
	uint32_t key_size;

	if (0 !=
	    pnso_sim_get_key_desc_idx((void **) &key1, (void **) &key2,
				      &key_size,
				      ctx->cmd.req.crypto_desc.
				      key_desc_idx)) {
		return PNSO_ERR_XTS_KEY_INDEX_OUT_OF_RANG;
	}

	memset(ctx->sess->scratch.cmd, 0, CMD_SCRATCH_SZ);

	if (0 != algo_encrypt_xts(ctx->sess->scratch.cmd, key1,
				  (uint8_t *) ctx->cmd.req.crypto_desc.
				  iv_addr, (uint8_t *) ctx->input.buf,
				  ctx->input.len,
				  (uint8_t *) ctx->output.buf,
				  &ctx->output.len)) {
		return PNSO_ERR_XTS_AXI_ERROR;
	}
	return PNSO_OK;
}

static pnso_error_t svc_exec_decrypt(struct pnso_sim_svc_ctx *ctx,
				     void *opaque)
{
	uint8_t *key1, *key2;
	uint32_t key_size;

	if (0 !=
	    pnso_sim_get_key_desc_idx((void **) &key1, (void **) &key2,
				      &key_size,
				      ctx->cmd.req.crypto_desc.
				      key_desc_idx)) {
		return PNSO_ERR_XTS_KEY_INDEX_OUT_OF_RANG;
	}

	memset(ctx->sess->scratch.cmd, 0, CMD_SCRATCH_SZ);

	if (0 != algo_decrypt_xts(ctx->sess->scratch.cmd, key1,
				  (uint8_t *) ctx->cmd.req.crypto_desc.
				  iv_addr, (uint8_t *) ctx->input.buf,
				  ctx->input.len,
				  (uint8_t *) ctx->output.buf,
				  &ctx->output.len)) {
		return PNSO_ERR_XTS_AXI_ERROR;
	}
	return PNSO_OK;
}

typedef pnso_error_t(*svc_iterator_func_t) (struct pnso_sim_svc_ctx * ctx,
					    uint32_t block_idx,
					    struct pnso_flat_buffer *
					    block, void *opaque);

static pnso_error_t svc_iterate_blocks(struct pnso_sim_svc_ctx *ctx,
				       svc_iterator_func_t func,
				       void *opaque)
{
	pnso_error_t rc;
	size_t block_i;
	uint32_t block_count =
	    flat_buffer_block_count(&ctx->input, ctx->sess->block_sz);
	struct pnso_flat_buffer block;

	for (block_i = 0; block_i < block_count; block_i++) {
		flat_buffer_to_block(&ctx->input, &block,
				     ctx->sess->block_sz, block_i);
		rc = func(ctx, block_i, &block, opaque);
		if (rc != PNSO_OK)
			break;
	}

	return rc;
}

static pnso_error_t svc_exec_hash_one_block(struct pnso_sim_svc_ctx *ctx,
					    uint32_t block_idx,
					    struct pnso_flat_buffer *block,
					    void *opaque)
{
	int *call_count = (int *) opaque;
	pnso_error_t rc = PNSO_OK;

	(*call_count)++;

	PNSO_ASSERT(ctx->status.num_tags > block_idx);

	memset(ctx->sess->scratch.cmd, 0, CMD_SCRATCH_SZ);

	switch (ctx->cmd.algo_type) {
	case PNSO_HASH_TYPE_SHA2_256:
		if (!algo_sha_gen
		    (ctx->sess->scratch.cmd,
		     ctx->status.tags[block_idx].hash_or_chksum,
		     (uint8_t *) block->buf, block->len, 256)) {
			rc = PNSO_ERR_HASH_BAD_PARAM;
		}
		break;
	case PNSO_HASH_TYPE_SHA2_512:
		if (!algo_sha_gen
		    (ctx->sess->scratch.cmd,
		     ctx->status.tags[block_idx].hash_or_chksum,
		     (uint8_t *) block->buf, block->len, 512)) {
			rc = PNSO_ERR_HASH_BAD_PARAM;
		}
		break;
	default:
		rc = PNSO_ERR_HASH_UNSUPPORTED;
		break;
	}

	return rc;
}

static pnso_error_t svc_exec_hash(struct pnso_sim_svc_ctx *ctx,
				  void *opaque)
{
	int opaque_counter = 0;
	pnso_error_t rc =
	    svc_iterate_blocks(ctx, svc_exec_hash_one_block,
			       &opaque_counter);

	ctx->status.num_tags = opaque_counter;

	/* Zero-copy output */
	ctx->output = ctx->input;

	return rc;
}

static pnso_error_t svc_exec_chksum_one_block(struct pnso_sim_svc_ctx *ctx,
					      uint32_t block_idx,
					      struct pnso_flat_buffer
					      *block, void *opaque)
{
	int *call_count = (int *) opaque;
	uint32_t temp32;
	uint8_t *hash_buf;
	pnso_error_t rc = PNSO_OK;

	(*call_count)++;

	PNSO_ASSERT(ctx->status.num_tags > block_idx);

	hash_buf = ctx->status.tags[block_idx].hash_or_chksum;

	switch (ctx->cmd.algo_type) {
	case PNSO_CHKSUM_TYPE_NONE:
		/* TODO */
		rc = PNSO_ERR_HASH_UNSUPPORTED;
		break;
	case PNSO_CHKSUM_TYPE_MCRC64:
		/* TODO */
		rc = PNSO_ERR_HASH_UNSUPPORTED;
		break;
	case PNSO_CHKSUM_TYPE_CRC32C:
		/* TODO */
		rc = PNSO_ERR_HASH_UNSUPPORTED;
		break;

	case PNSO_CHKSUM_TYPE_ADLER32:
		temp32 =
		    algo_gen_adler32((uint8_t *) block->buf, block->len);
		*((uint32_t *) hash_buf) = temp32;
		break;

	case PNSO_CHKSUM_TYPE_MADLER32:
		temp32 =
		    algo_gen_madler((uint64_t *) block->buf, block->len);
		*((uint32_t *) hash_buf) = temp32;
		break;

	default:
		rc = PNSO_ERR_HASH_UNSUPPORTED;
		break;
	}

	/* Zero-copy output */
	ctx->output = ctx->input;

	return rc;
}

static pnso_error_t svc_exec_chksum(struct pnso_sim_svc_ctx *ctx,
				    void *opaque)
{
	int opaque_counter = 0;
	pnso_error_t rc =
	    svc_iterate_blocks(ctx, svc_exec_chksum_one_block,
			       &opaque_counter);

	ctx->status.num_tags = opaque_counter;

	/* Zero-copy output */
	ctx->output = ctx->input;

	return rc;
}


/* Traverses compacted block to find desired data, and does
 * decompression if necessary.
 */
static pnso_error_t svc_exec_decompact(struct pnso_sim_svc_ctx *ctx,
				       void *opaque)
{
	size_t i;
	wafl_packed_blk_t *wafl_blk = (wafl_packed_blk_t *) ctx->input.buf;
	wafl_packed_data_info_t *wafl_data;

	PNSO_ASSERT(ctx->input.len >= sizeof(wafl_packed_blk_t));
	PNSO_ASSERT(ctx->input.len >= sizeof(wafl_packed_blk_t) +
		    wafl_blk->wpb_hdr.wpbh_num_objs *
		    sizeof(wafl_packed_data_info_t));

	for (i = 0; i < wafl_blk->wpb_hdr.wpbh_num_objs; i++) {
		wafl_data = &wafl_blk->wpb_data_info[i];
		if (wafl_data->wpd_vvbn == ctx->cmd.req.decompact_desc.key) {
			/* Found the right data, now decompress or copy raw to output */
			PNSO_ASSERT(wafl_data->wpd_len +
				    wafl_data->wpd_off <=
				    ctx->sess->block_sz);
			if ((wafl_data->
			     wpd_flags & WAFL_PACKED_DATA_ENCODED)
			    || !ctx->cmd.req.decompact_desc.
			    is_uncompressed) {
				/* Compressed (either single or multi-block).  Decompress it. */
				struct pnso_compression_header *hdr =
				    (struct pnso_compression_header *)
				    (ctx->input.buf + wafl_data->wpd_off);
				uint32_t hdr_len =
				    wafl_data->
				    wpd_flags & WAFL_PACKED_DATA_ENCODED ?
				    0 : sizeof(*hdr);
				uint32_t dst_len = 0;
				PNSO_ASSERT(wafl_data->wpd_len > hdr_len);
				PNSO_ASSERT(!hdr_len
					    || hdr->data_len <
					    wafl_data->wpd_len);
				memset(ctx->sess->scratch.cmd, 0,
				       CMD_SCRATCH_SZ);
				lzrw1a_compress(COMPRESS_ACTION_DECOMPRESS,
						ctx->sess->scratch.cmd,
						(uint8_t *) ctx->input.
						buf + wafl_data->wpd_off +
						hdr_len,
						hdr_len ? hdr->
						data_len : wafl_data->
						wpd_len,
						(uint8_t *) ctx->output.
						buf, &dst_len, 0);
				ctx->output.len = dst_len;
			} else {
				/* Uncompressed.  Just copy. */
				PNSO_ASSERT(wafl_data->wpd_len <
					    ctx->sess->block_sz);
				memcpy((void *) ctx->output.buf,
				       (void *) ctx->input.buf +
				       wafl_data->wpd_off,
				       wafl_data->wpd_len);
				ctx->output.len = wafl_data->wpd_len;
			}
			return PNSO_OK;
		}
	}

	return PNSO_ERR_DECOMPACT_BAD_INPUT;
}
