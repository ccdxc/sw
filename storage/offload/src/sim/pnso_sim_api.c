
#include <errno.h>
#include "LZR_model.h"

#include "pnso_sim.h"
#include "pnso_sim_algo.h"
#include "pnso_sim_util.h"
#include "pnso_wafl.h"
#include "pnso_sim_osal.h"

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

struct pnso_init_params g_init_params;

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
static pnso_error_t svc_exec_hash(struct pnso_sim_svc_ctx *ctx,
				  void *opaque);
static pnso_error_t svc_exec_chksum(struct pnso_sim_svc_ctx *ctx,
				    void *opaque);
static pnso_error_t svc_exec_decompact(struct pnso_sim_svc_ctx *ctx,
				       void *opaque);

#define CMD_SCRATCH_SZ (16 * 1024)
#define SCRATCH_PER_SESSION (4 * PNSO_MAX_BUFFER_LEN)


static pnso_error_t pnso_sim_init_session()
{
	struct pnso_sim_worker_ctx *worker_ctx = pnso_sim_get_worker_ctx();
	struct pnso_sim_session *sess = worker_ctx->sess;
	size_t func_i;
	uint8_t *scratch = NULL;
	uint32_t scratch_sz = SCRATCH_PER_SESSION;

	if (!sess) {
		/* Allocate both session and scratch */
		void *mem = pnso_sim_alloc(sizeof(*sess) + scratch_sz);
		if (!mem) {
			return ENOMEM;
		}
		sess = (struct pnso_sim_session *) mem;
		memset(sess, 0, sizeof(*sess));
		scratch = mem + sizeof(*sess);
		worker_ctx->sess = sess;
	} else {
		/* Recycle existing session */
		PNSO_ASSERT(!sess->is_valid);
		scratch = sess->scratch.cmd;
	}

	/* Use default function in case of undefined func */
	for (func_i = 0; func_i < PNSO_SVC_TYPE_MAX; func_i++) {
		/* Set svc_exec command functions */
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
		case PNSO_SVC_TYPE_DECOMPACT:
			sess->funcs[func_i] = svc_exec_decompact;
			break;
		default:
			sess->funcs[func_i] = svc_exec_noop;
			break;
		}
	}

	sess->scratch.cmd = scratch;
	scratch += CMD_SCRATCH_SZ;
	scratch_sz -= CMD_SCRATCH_SZ;
	sess->scratch.data_sz = scratch_sz / 2;
	sess->scratch.data[0] = scratch;
	sess->scratch.data[1] = scratch + (scratch_sz / 2);

	sess->block_sz = g_init_params.block_size;
	sess->is_valid = true;

	return 0;
}

static void pnso_sim_finit_session()
{
	struct pnso_sim_worker_ctx *worker_ctx = pnso_sim_get_worker_ctx();
	struct pnso_sim_session *sess = worker_ctx->sess;

	if (!sess) {
		/* Nothing to do */
		return;
	}

	sess->is_valid = false;
	worker_ctx->sess = NULL;
	pnso_sim_free(sess);
}

pnso_error_t pnso_init(struct pnso_init_params *init_params)
{
	pnso_error_t rc = PNSO_OK;

	g_init_params = *init_params;
	pnso_sim_init_req_pool();

	return rc;
}

pnso_error_t pnso_sim_thread_init()
{
	pnso_error_t rc;
	rc = pnso_sim_init_session();
	if (rc != PNSO_OK) {
		return rc;
	}
	rc = pnso_sim_start_worker_thread();
	return rc;
}

void pnso_sim_thread_finit()
{
	pnso_sim_stop_worker_thread();
	pnso_sim_finit_session();
}

/* Free resources used by sim.  Assumes no worker threads running. */
void pnso_sim_finit()
{
}

static inline pnso_error_t svc_exec_memcpy(struct pnso_sim_svc_ctx *ctx,
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
	rc = session->funcs[ctx->cmd.svc_type](ctx, opaque);

	ctx->status.num_outputs = 1;
	ctx->status.svc_type = ctx->cmd.svc_type;
	ctx->status.err = rc;

	return rc;
}

pnso_error_t pnso_sim_execute_request(struct pnso_sim_worker_ctx *worker_ctx,
				      struct pnso_service_request *svc_req,
				      struct pnso_service_result *svc_res,
				      completion_t cb, void *cb_ctx)
{
	struct pnso_sim_session *sess = worker_ctx->sess;
	int rc = 0;
	size_t svc_i, scratch_bank = 0;
	struct pnso_sim_svc_ctx svc_ctxs[2];
	struct pnso_sim_svc_ctx *cur_svc = &svc_ctxs[0];
	struct pnso_sim_svc_ctx *prev_svc = NULL;
	struct pnso_service_status *status;
	struct pnso_output_buf *output_buf;

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
			rc = EINVAL;
			goto error;
		}
		cur_svc->status = svc_res->svc[svc_i];

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

		/* Store result */
		status = &cur_svc->status;
		if (status->num_outputs) {
			output_buf = status->o.output_buf;
			if (output_buf && output_buf->buf_list)
				/* Intermediate buffer */
				pnso_memcpy_flat_buf_to_list(output_buf->buf_list,
						&cur_svc->output);
		}
		svc_res->svc[svc_i] = cur_svc->status;

		if (rc != 0) {
			/* TODO: continue?? */
			goto error;
		}

		prev_svc = cur_svc;
	}

error:
	return rc;
}

pnso_error_t pnso_submit_request(enum pnso_batch_request batch_req,
				 struct pnso_service_request *svc_req,
				 struct pnso_service_result *svc_res,
				 completion_t cb,
				 void *cb_ctx,
				 pnso_poll_fn_t *poll_fn,
				 void **poll_ctx)
{
	pnso_error_t rc;

	if (cb == NULL) {
		void *priv_poll_ctx;

		/* Synchronous request */
#if 0
		if (!pnso_sim_is_worker_running()) {
			/* No worker thread, run directly in this thread */
			return pnso_sim_execute_request(pnso_sim_get_worker_ctx(),
							svc_req, svc_res,
							NULL, NULL);
		}
#endif

		/* Local polling mode */
		rc = pnso_sim_sq_enqueue(batch_req, svc_req, svc_res, NULL,
					 NULL, &priv_poll_ctx);
		if (rc == PNSO_OK) {
			rc = pnso_sim_poll_wait(priv_poll_ctx);
		}
		return rc;
	}

	/* Asynchronous request */
	rc = pnso_sim_sq_enqueue(batch_req, svc_req, svc_res, cb,
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

static pnso_error_t svc_exec_compress_lzrw1a(struct pnso_sim_svc_ctx *ctx)
{
	struct pnso_compression_header *hdr =
	    (struct pnso_compression_header *) ctx->output.buf;
	uint32_t hdr_len = ctx->cmd.d.cp_desc.flags & PNSO_DFLAG_INSERT_HEADER ?
				sizeof(*hdr) : 0;
	uint32_t dst_len = ctx->sess->scratch.data_sz - hdr_len;

	if (lzrw1a_compress(COMPRESS_ACTION_COMPRESS, ctx->sess->scratch.cmd,
	     (uint8_t *) ctx->input.buf, ctx->input.len,
	     (uint8_t *) ctx->output.buf + hdr_len, &dst_len, 0)) {
		ctx->output.len = dst_len + hdr_len;
		if (hdr_len >= sizeof(*hdr)) {
			hdr->chksum = 0;
			hdr->data_len = dst_len;
			hdr->version = g_init_params.cp_hdr_version;
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
	uint32_t hdr_len = ctx->cmd.d.dc_desc.flags & PNSO_DFLAG_HEADER_PRESENT ?
				sizeof(*hdr) : 0;
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

	switch (ctx->cmd.d.cp_desc.algo_type) {
	case PNSO_COMPRESSOR_TYPE_NONE:
		rc = svc_exec_noop(ctx, opaque);
		break;
	case PNSO_COMPRESSOR_TYPE_LZRW1A:
		rc = svc_exec_compress_lzrw1a(ctx);
		break;
	default:
		rc = EINVAL;
		break;
	}
	
	ctx->status.o.output_buf->data_len = ctx->output.len;

	if (rc == PNSO_OK) {
		/* Check that it was compressed enough */
		if (ctx->output.len > ctx->cmd.d.cp_desc.threshold_len) {
			svc_exec_noop(ctx, opaque);
			rc = PNSO_ERR_CPDC_DATA_TOO_LONG;
		}

		/* Pad */
		if (ctx->cmd.d.cp_desc.flags & PNSO_DFLAG_ZERO_PAD) {
			flat_buffer_pad(&ctx->output, ctx->sess->block_sz);
		}
	}

	return rc;
}

static pnso_error_t svc_exec_decompress(struct pnso_sim_svc_ctx *ctx,
					void *opaque)
{
	pnso_error_t rc = 0;

	memset(ctx->sess->scratch.cmd, 0, CMD_SCRATCH_SZ);

	switch (ctx->cmd.d.dc_desc.algo_type) {
	case PNSO_COMPRESSOR_TYPE_NONE:
		rc = svc_exec_noop(ctx, opaque);
		break;
	case PNSO_COMPRESSOR_TYPE_LZRW1A:
		rc = svc_exec_decompress_lzrw1a(ctx);
		break;
	default:
		rc = EINVAL;
		break;
	}

	ctx->status.o.output_buf->data_len = ctx->output.len;

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
				      ctx->cmd.d.crypto_desc.
				      key_desc_idx)) {
		return PNSO_ERR_XTS_KEY_INDEX_OUT_OF_RANG;
	}

	memset(ctx->sess->scratch.cmd, 0, CMD_SCRATCH_SZ);

	if (0 != algo_encrypt_xts(ctx->sess->scratch.cmd, key1,
				  (uint8_t *) ctx->cmd.d.crypto_desc.
				  iv_addr, (uint8_t *) ctx->input.buf,
				  ctx->input.len,
				  (uint8_t *) ctx->output.buf,
				  &ctx->output.len)) {
		return PNSO_ERR_XTS_AXI_ERROR;
	}

	ctx->status.o.output_buf->data_len = ctx->output.len;

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
				      ctx->cmd.d.crypto_desc.
				      key_desc_idx)) {
		return PNSO_ERR_XTS_KEY_INDEX_OUT_OF_RANG;
	}

	memset(ctx->sess->scratch.cmd, 0, CMD_SCRATCH_SZ);

	if (0 != algo_decrypt_xts(ctx->sess->scratch.cmd, key1,
				  (uint8_t *) ctx->cmd.d.crypto_desc.
				  iv_addr, (uint8_t *) ctx->input.buf,
				  ctx->input.len,
				  (uint8_t *) ctx->output.buf,
				  &ctx->output.len)) {
		return PNSO_ERR_XTS_AXI_ERROR;
	}

	ctx->status.o.output_buf->data_len = ctx->output.len;

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
	pnso_error_t rc = PNSO_OK;
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

	PNSO_ASSERT(ctx->status.num_outputs > block_idx);

	memset(ctx->sess->scratch.cmd, 0, CMD_SCRATCH_SZ);

	switch (ctx->cmd.d.hash_desc.algo_type) {
	case PNSO_HASH_TYPE_SHA2_256:
		if (!algo_sha_gen
		    (ctx->sess->scratch.cmd,
		     ctx->status.o.hashes[block_idx].hash_tag,
		     (uint8_t *) block->buf, block->len, 256)) {
			rc = PNSO_ERR_SHA_FAILED;
		}
		break;
	case PNSO_HASH_TYPE_SHA2_512:
		if (!algo_sha_gen
		    (ctx->sess->scratch.cmd,
		     ctx->status.o.hashes[block_idx].hash_tag,
		     (uint8_t *) block->buf, block->len, 512)) {
			rc = PNSO_ERR_SHA_FAILED;
		}
		break;
	default:
		rc = EINVAL;
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

	ctx->status.num_outputs= opaque_counter;

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

	PNSO_ASSERT(ctx->status.num_outputs> block_idx);

	hash_buf = ctx->status.o.chksums[block_idx].chksum_tag;

	switch (ctx->cmd.d.chksum_desc.algo_type) {
	case PNSO_CHKSUM_TYPE_NONE:
		/* TODO */
		rc = EINVAL;
		break;
	case PNSO_CHKSUM_TYPE_MCRC64:
		/* TODO */
		rc = EINVAL;
		break;
	case PNSO_CHKSUM_TYPE_CRC32C:
		/* TODO */
		rc = EINVAL;
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
		rc = EINVAL;
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

	ctx->status.num_outputs= opaque_counter;

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
		if (wafl_data->wpd_vvbn == ctx->cmd.d.decompact_desc.vvbn) {
			/* Found the right data, now decompress or copy raw to output */
			PNSO_ASSERT(wafl_data->wpd_len +
				    wafl_data->wpd_off <=
				    ctx->sess->block_sz);
			if ((wafl_data->wpd_flags & WAFL_PACKED_DATA_ENCODED)
			    /*|| !ctx->cmd.d.decompact_desc.is_uncompressed*/) {
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
			
			ctx->status.o.output_buf->data_len = ctx->output.len;

			return PNSO_OK;
		}
	}

	return EINVAL;
}
