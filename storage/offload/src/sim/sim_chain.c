/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include "osal_errno.h"
#include "osal_mem.h"

/*#include "LZR_model.h"*/
#ifdef PNSO_SIM_MODEL
#include "third-party/sim_smooth_lzrw1a/lzrw.h"
#else
#include "lzrw.h"
#endif

#include "sim.h"
#include "sim_chain.h"
#include "sim_algo.h"
#include "sim_util.h"
#include "sim_worker.h"

#include "pnso_util.h"
#include "pnso_wafl.h"

/* index is the pnso_algo, value is the hdr_algo */
uint32_t g_algo_map[PNSO_COMPRESSION_TYPE_MAX] = { 0, 0 };

struct sim_cp_header_format g_cp_hdr_formats[SIM_MAX_CP_HEADER_FMTS];

static pnso_error_t svc_exec_noop(struct sim_svc_ctx *ctx,
				  void *opaque);
static pnso_error_t svc_exec_compress(struct sim_svc_ctx *ctx,
				      void *opaque);
static pnso_error_t svc_exec_decompress(struct sim_svc_ctx *ctx,
					void *opaque);
static pnso_error_t svc_exec_encrypt(struct sim_svc_ctx *ctx,
				     void *opaque);
static pnso_error_t svc_exec_decrypt(struct sim_svc_ctx *ctx,
				     void *opaque);
static pnso_error_t svc_exec_hash(struct sim_svc_ctx *ctx,
				  void *opaque);
static pnso_error_t svc_exec_chksum(struct sim_svc_ctx *ctx,
				    void *opaque);
static pnso_error_t svc_exec_decompact(struct sim_svc_ctx *ctx,
				       void *opaque);

#define CMD_SCRATCH_SZ (16 * 1024)
#define SCRATCH_DATA_SZ (2 * 1024 * 1024)
#define SCRATCH_MIN_DATA_SZ (64 * 1024)
#define SCRATCH_PER_SESSION (CMD_SCRATCH_SZ + (2 * SCRATCH_DATA_SZ))
#define SCRATCH_MIN_PER_SESSION (CMD_SCRATCH_SZ + (2 * SCRATCH_MIN_DATA_SZ))


pnso_error_t sim_init_session(int core_id,
			      const struct pnso_init_params *init_params)
{
	struct sim_worker_ctx *worker_ctx = sim_get_worker_ctx(core_id);
	struct sim_session *sess;
	size_t func_i;
	uint8_t *scratch = NULL;
	uint32_t scratch_sz = SCRATCH_PER_SESSION;

	if (!worker_ctx) {
		return EINVAL;
	}

	sess = worker_ctx->sess;
	if (!sess) {
		/* Allocate both session and scratch */
		void *mem = osal_alloc(sizeof(*sess) + scratch_sz);

		if (!mem) {
			scratch_sz = SCRATCH_MIN_PER_SESSION;
			mem = osal_alloc(sizeof(*sess) + scratch_sz);
			if (!mem)
				return ENOMEM;			
		}
		sess = (struct sim_session *) mem;
		memset(sess, 0, sizeof(*sess));
		scratch = mem + sizeof(*sess);

		sim_workers_spinlock();
		if (worker_ctx->sess) {
			sess = worker_ctx->sess;
			sim_workers_spinunlock();
			osal_free(mem);
			if (osal_atomic_read(&sess->is_valid)) {
				return PNSO_OK;
			}
			return EBUSY;
		}
		worker_ctx->sess = sess;
		sim_workers_spinunlock();
	} else {
		/* Recycle existing session */
		OSAL_ASSERT(!osal_atomic_read(&sess->is_valid));
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

	sess->block_sz = init_params->block_size;
	sess->q_depth = init_params->per_core_qdepth;
	osal_atomic_set(&sess->is_valid, 1);

	return 0;
}

void sim_finit_session(int core_id)
{
	struct sim_worker_ctx *worker_ctx = sim_lookup_worker_ctx(core_id);
	struct sim_session *sess;

	if (!worker_ctx || !worker_ctx->sess) {
		/* Nothing to do */
		return;
	}

	sim_workers_spinlock();
	sess = worker_ctx->sess;
	if (sess) {
		osal_atomic_set(&sess->is_valid, 0);
		worker_ctx->sess = NULL;
	}
	sim_workers_spinunlock();
	if (sess) {
		osal_free(sess);
	}
}

void sim_init_globals(void)
{
	memset(g_cp_hdr_formats, 0, sizeof(g_cp_hdr_formats));
	memset(g_algo_map, 0, sizeof(g_algo_map));
}

struct sim_cp_header_format *sim_lookup_hdr_format(uint32_t hdr_fmt_idx,
						   bool alloc)
{
	size_t i;
	struct sim_cp_header_format *fmt;
	struct sim_cp_header_format *vacant_fmt = NULL;

	if (!hdr_fmt_idx) {
		return NULL;
	}

	/* First try to find entry at the given index */
	if (hdr_fmt_idx <= SIM_MAX_CP_HEADER_FMTS) {
		fmt = &g_cp_hdr_formats[hdr_fmt_idx - 1];
		if (fmt->fmt_idx == hdr_fmt_idx) {
			return fmt;
		}
		if (fmt->fmt_idx == 0) {
			vacant_fmt = fmt;
		}
	}

	/* Second, search all entries */
	for (i = 0; i < SIM_MAX_CP_HEADER_FMTS; i++) {
		fmt = &g_cp_hdr_formats[i];
		if (fmt->fmt_idx == hdr_fmt_idx) {
			return fmt;
		}
		if (fmt->fmt_idx == 0 && !vacant_fmt) {
			vacant_fmt = fmt;
		}
	}

	if (alloc) {
		return vacant_fmt;
	}
	return NULL;
}

void sim_set_algo_mapping(enum pnso_compression_type pnso_algo,
			  uint32_t hdr_algo)
{
	g_algo_map[pnso_algo] = hdr_algo;
}

#if 0
uint32_t sim_pnso_algo_to_header_algo(enum pnso_compression_type pnso_algo)
{
	if (pnso_algo >= PNSO_COMPRESSION_TYPE_MAX) {
		return 0;
	}
	return g_algo_map[pnso_algo];
}
#endif

static uint32_t sim_header_algo_to_pnso_algo(uint32_t header_algo)
{
	size_t i;

	for (i = 0; i < PNSO_COMPRESSION_TYPE_MAX; i++) {
		if (g_algo_map[i] == header_algo) {
			return i;
		}
	}
	return PNSO_COMPRESSION_TYPE_MAX;
}

#if 0
static pnso_error_t svc_exec_memcpy(struct sim_svc_ctx *ctx,
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
#endif

static pnso_error_t svc_exec_noop(struct sim_svc_ctx *ctx,
				  void *opaque)
{
	ctx->output = ctx->input;
	return PNSO_OK;
}

static pnso_error_t execute_service(struct sim_session *session,
				    struct sim_svc_ctx *ctx,
				    void *opaque)
{
	pnso_error_t rc = PNSO_OK;

	OSAL_ASSERT(session->funcs[ctx->cmd.svc_type]);
	rc = session->funcs[ctx->cmd.svc_type](ctx, opaque);

	ctx->status.svc_type = ctx->cmd.svc_type;
	ctx->status.err = rc;

	if (rc != PNSO_OK) {
		if (ctx->cmd.svc_type == PNSO_SVC_TYPE_COMPRESS &&
		    (ctx->cmd.u.cp_desc.flags & PNSO_CP_DFLAG_BYPASS_ONFAIL)) {
			/* Allow errors explicitly for BYPASS_ONFAIL */
			rc = PNSO_OK;
		}
	}

	return rc;
}

pnso_error_t sim_execute_request(struct sim_worker_ctx *worker_ctx,
				 struct pnso_service_request *svc_req,
				 struct pnso_service_result *svc_res,
				 completion_cb_t cb, void *cb_ctx)
{
	struct sim_session *sess = worker_ctx->sess;
	pnso_error_t rc = PNSO_OK;
	size_t svc_i, scratch_bank = 0;
	struct sim_svc_ctx svc_ctxs[2];
	struct sim_svc_ctx *cur_svc = &svc_ctxs[0];
	struct sim_svc_ctx *prev_svc = NULL;
	struct pnso_service_status *status;

	OSAL_ASSERT(svc_req->num_services == svc_res->num_services);
	OSAL_ASSERT(svc_req->num_services >= 1);

	memset(svc_ctxs, 0, sizeof(svc_ctxs));
	svc_ctxs[0].sess = svc_ctxs[1].sess = sess;

	/* Setup first service, copy input data to flat scratch buffer */
	cur_svc = &svc_ctxs[0];
	cur_svc->input.buf = (uint64_t) sess->scratch.data[scratch_bank];
	cur_svc->input.len = sess->scratch.data_sz;
	cur_svc->input.len = sim_memcpy_pa_list_to_flat_buf(&cur_svc->input,
							    svc_req->sgl);
	cur_svc->is_first = 1;

	for (svc_i = 0; svc_i < svc_req->num_services; svc_i++) {
		/* Setup command */
		cur_svc->cmd = svc_req->svc[svc_i];
		if (cur_svc->cmd.svc_type >= PNSO_SVC_TYPE_MAX) {
			rc = EINVAL;
			break;
		}
		cur_svc->status = svc_res->svc[svc_i];

		/* Setup input buffer */
		if (prev_svc != NULL) {
			/* Use previous service output  */
			cur_svc->input = prev_svc->output;
			cur_svc->is_first = 0;
			cur_svc->prev_ctx = prev_svc;
		}

		/* Setup output buffer */
		if (cur_svc->input.buf ==
		    (uint64_t) sess->scratch.data[scratch_bank]) {
			/* swap scratch regions */
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
		rc = execute_service(sess, cur_svc, NULL);

		/* Store result */
		status = &cur_svc->status;
		if (rc == PNSO_OK &&
		    cur_svc->cmd.svc_type != PNSO_SVC_TYPE_HASH &&
		    cur_svc->cmd.svc_type != PNSO_SVC_TYPE_CHKSUM) {
			if (status->u.dst.sgl) {
				/* Intermediate buffer */
				if (sim_buflist_len(status->u.dst.sgl) < 
				    cur_svc->output.len) {
					rc = ENOMEM;
					/* TODO: need generic PNSO error */
				} else {
					sim_memcpy_flat_buf_to_pa_list(
						status->u.dst.sgl,
						&cur_svc->output);
				}
			}
		}
		svc_res->svc[svc_i] = cur_svc->status;

		if (rc != PNSO_OK) {
			break;
		}

		prev_svc = cur_svc;
	}

	svc_res->err = rc;
	return rc;
}

static uint32_t get_ctx_chksum(struct sim_svc_ctx *ctx)
{
	if (ctx &&
	    (ctx->cmd.svc_type == PNSO_SVC_TYPE_CHKSUM) &&
	    !(ctx->cmd.u.chksum_desc.flags & PNSO_CHKSUM_DFLAG_PER_BLOCK) &&
	    (pnso_get_chksum_algo_size(ctx->cmd.u.chksum_desc.algo_type)
	     == sizeof(uint32_t)) &&
	    ctx->status.err == PNSO_OK &&
	    ctx->status.u.chksum.num_tags >= 1) {
		return *((uint32_t *)ctx->status.u.chksum.tags);
	}

	return 0;
}

/* Assumption: hdr is large enough buffer to contain the header */
static void construct_cp_hdr(struct sim_svc_ctx *ctx,
			     struct sim_cp_header_format *fmt,
			     uint32_t cp_len, uint32_t chksum,
			     uint8_t *hdr)
{
	size_t i;

	/* Validation */
	if (!fmt->total_hdr_sz) {
		/* Nothing to do */
		return;
	}

	/* First copy static header */
	memcpy(hdr, fmt->static_hdr, fmt->total_hdr_sz);

	/* Get previously calculated checksum, if needed */
	if (!chksum && (fmt->type_mask & (1 << PNSO_HDR_FIELD_TYPE_INDATA_CHKSUM))) {
		chksum = get_ctx_chksum(ctx->prev_ctx);
	}

	/* Iterate through dynamic fields */
	for (i = 0; i < fmt->fmt.num_fields; i++) {
		struct pnso_header_field *tlv = &fmt->fmt.fields[i];

		switch (tlv->type) {
		case PNSO_HDR_FIELD_TYPE_INDATA_CHKSUM:
			sim_tlv_to_buf(hdr+tlv->offset, tlv->length, chksum);
			break;
		case PNSO_HDR_FIELD_TYPE_OUTDATA_LENGTH:
			sim_tlv_to_buf(hdr+tlv->offset, tlv->length, cp_len);
			break;
		case PNSO_HDR_FIELD_TYPE_ALGO:
			sim_tlv_to_buf(hdr+tlv->offset, tlv->length,
				       ctx->cmd.u.cp_desc.hdr_algo);
			break;
		default:
			/* covered by static header memcpy */
			break;
		}
	}
}

/* Assumption: hdr is large enough buffer to contain the header */
static void parse_cp_hdr(struct sim_svc_ctx *ctx,
			 struct sim_cp_header_format *fmt,
			 uint32_t *cp_len,
			 uint64_t *chksum,
			 uint32_t *algo_type,
			 const uint8_t *hdr)
{
	size_t i;
	uint64_t val;

	/* Iterate through dynamic fields */
	for (i = 0; i < fmt->fmt.num_fields; i++) {
		struct pnso_header_field *tlv = &fmt->fmt.fields[i];

		switch (tlv->type) {
		case PNSO_HDR_FIELD_TYPE_INDATA_CHKSUM:
			sim_buf_to_tlv(hdr+tlv->offset, tlv->length, &val);
			*chksum = val;
			break;
		case PNSO_HDR_FIELD_TYPE_OUTDATA_LENGTH:
			sim_buf_to_tlv(hdr+tlv->offset, tlv->length, &val);
			*cp_len = val;
			break;
		case PNSO_HDR_FIELD_TYPE_ALGO:
			sim_buf_to_tlv(hdr+tlv->offset, tlv->length, &val);
			*algo_type = val;
			break;
		default:
			break;
		}
	}
}


static pnso_error_t svc_exec_compress_lzrw1a(struct sim_svc_ctx *ctx,
					     uint32_t hdr_len,
					     uint32_t dst_len,
					     uint32_t *chksum)
{
	pnso_error_t rc;

	/* Compress data */
	if (lzrw1a_compress(COMPRESS_ACTION_COMPRESS, ctx->sess->scratch.cmd,
	     (uint8_t *) ctx->input.buf, ctx->input.len,
	     (uint8_t *) ctx->output.buf + hdr_len, &dst_len
#ifdef LZR_MODEL
			    , dst_len, chksum
#endif
			    )) {
		ctx->output.len = dst_len + hdr_len;
		rc = PNSO_OK;
		return rc;
	}
	return PNSO_ERR_CPDC_DATA_TOO_LONG;
}

static pnso_error_t svc_exec_decompress_lzrw1a(struct sim_svc_ctx *ctx,
					       uint32_t hdr_len,
					       uint32_t data_len,
					       uint32_t *chksum)
{
	uint32_t dst_len = ctx->sess->scratch.data_sz;

	if (ctx->status.u.dst.sgl) {
		dst_len = sim_buflist_len(ctx->status.u.dst.sgl);
		if (dst_len > ctx->sess->scratch.data_sz) {
			dst_len = ctx->sess->scratch.data_sz;
		}
	}

#ifdef LZR_MODEL
	lzrw1a_compress(COMPRESS_ACTION_DECOMPRESS, ctx->sess->scratch.cmd,
			(uint8_t *) ctx->input.buf + hdr_len, data_len,
			(uint8_t *) ctx->output.buf, &dst_len,
			dst_len, chksum);
	if (dst_len) {
#else
	if (lzrw1a_compress(COMPRESS_ACTION_DECOMPRESS, ctx->sess->scratch.cmd,
			(uint8_t *) ctx->input.buf + hdr_len, data_len,
			(uint8_t *) ctx->output.buf, &dst_len)) {
#endif
		ctx->output.len = dst_len;
		return PNSO_OK;
	}
	return PNSO_ERR_CPDC_DATA_TOO_LONG;
}

static pnso_error_t svc_exec_compress(struct sim_svc_ctx *ctx,
				      void *opaque)
{
	pnso_error_t rc = PNSO_OK;
	struct sim_cp_header_format *hdr_fmt = NULL;
	uint32_t hdr_len = 0;
	uint32_t dst_len = ctx->sess->scratch.data_sz;
	uint32_t chksum = 0;
	uint32_t *pchksum = NULL;

	/* Lookup compression header format */
	if ((ctx->cmd.u.cp_desc.flags & PNSO_CP_DFLAG_INSERT_HEADER) &&
	    ctx->cmd.u.cp_desc.hdr_fmt_idx) {
		hdr_fmt = sim_lookup_hdr_format(ctx->cmd.u.cp_desc.hdr_fmt_idx,
						false);
		if (!hdr_fmt) {
			rc = PNSO_ERR_CPDC_HDR_IDX_INVALID;
			goto cp_error;
		}
		hdr_len = hdr_fmt->total_hdr_sz;
		if (hdr_fmt->type_mask & (1 << PNSO_HDR_FIELD_TYPE_INDATA_CHKSUM)) {
			/* Instruct compressor to generate checksum */
			pchksum = &chksum;
		}
	}
	if (hdr_len >= dst_len) {
		/* super large cp header format, shouldn't happen */
		rc = ENOMEM;
		goto cp_error;
	}
	dst_len -= hdr_len;

	if (ctx->cmd.u.cp_desc.threshold_len &&
	    ctx->cmd.u.cp_desc.threshold_len < dst_len) {
		dst_len = ctx->cmd.u.cp_desc.threshold_len;
	}

	/* Compress data */
	memset(ctx->sess->scratch.cmd, 0, CMD_SCRATCH_SZ);
	switch (ctx->cmd.u.cp_desc.algo_type) {
	case PNSO_COMPRESSION_TYPE_NONE:
		rc = svc_exec_noop(ctx, opaque);
		break;
	case PNSO_COMPRESSION_TYPE_LZRW1A:
		rc = svc_exec_compress_lzrw1a(ctx, hdr_len, dst_len, pchksum);
		if (rc != PNSO_OK) {
			goto cp_error;
		}
		break;
	default:
		rc = PNSO_ERR_CPDC_ALGO_INVALID;
		goto cp_error;
		break;
	}

	/* Construct compression header */
	if (rc == PNSO_OK && hdr_len) {
		construct_cp_hdr(ctx, hdr_fmt, ctx->output.len - hdr_len,
				 chksum, (uint8_t *) ctx->output.buf);
	}

	/* Remember unpadded length */
	ctx->status.u.dst.data_len = ctx->output.len;

	if (rc == PNSO_OK) {
		/* Check that it was really compressed enough */
		if (ctx->cmd.u.cp_desc.threshold_len &&
		    ctx->output.len > ctx->cmd.u.cp_desc.threshold_len) {
			rc = PNSO_ERR_CPDC_DATA_TOO_LONG;
			goto cp_error;
		}

		/* Pad */
		if (ctx->cmd.u.cp_desc.flags & PNSO_CP_DFLAG_ZERO_PAD) {
			sim_flat_buffer_pad(&ctx->output, ctx->sess->block_sz);
		}
	}

	return rc;

cp_error:
	/* Need graceful exit with data, in case of BYPASS_ONFAIL */
	svc_exec_noop(ctx, opaque);
	ctx->status.u.dst.data_len = ctx->output.len;
	return rc;
}

static pnso_error_t svc_exec_decompress(struct sim_svc_ctx *ctx,
					void *opaque)
{
	pnso_error_t rc = PNSO_OK;
	struct sim_cp_header_format *hdr_fmt = NULL;
	uint32_t hdr_len = 0;
	uint32_t hdr_data_len = 0;
	uint64_t hdr_chksum = 0;
	uint32_t hdr_algo_type = 0;
	uint32_t pnso_algo_type = 0;
	uint32_t chksum = 0;
	uint32_t *pchksum = NULL;

	ctx->is_chksum_present = 0;

	/* Lookup compression header format */
	if (ctx->cmd.u.dc_desc.flags & PNSO_DC_DFLAG_HEADER_PRESENT) {
		hdr_fmt = sim_lookup_hdr_format(ctx->cmd.u.dc_desc.hdr_fmt_idx,
						false);
		if (!hdr_fmt) {
			return PNSO_ERR_CPDC_HDR_IDX_INVALID;
		}
		hdr_len = hdr_fmt->total_hdr_sz;
		if (hdr_len >= ctx->input.len) {
			/* super large cp header format, shouldn't happen */
			return ENOMEM;
		}

		/* Parse input header */
		if (hdr_fmt && hdr_len) {
			parse_cp_hdr(ctx, hdr_fmt,
				     &hdr_data_len,
				     &hdr_chksum,
				     &hdr_algo_type,
				     (uint8_t *)ctx->input.buf);

			/* Lookup pnso_algo_type from hdr_algo_type */
			pnso_algo_type =
				sim_header_algo_to_pnso_algo(hdr_algo_type);
			if (pnso_algo_type >= PNSO_COMPRESSION_TYPE_MAX) {
				return PNSO_ERR_CPDC_ALGO_INVALID;
			}

			/* Remember whether checksum is present */
			if (hdr_fmt->type_mask &
			    (1 << PNSO_HDR_FIELD_TYPE_INDATA_CHKSUM)) {
				ctx->is_chksum_present = 1;
				pchksum = &chksum;
			}
		}
	}

	if (!pnso_algo_type) {
		/* Use a default compression algorithm, for header-less case */
		pnso_algo_type = ctx->cmd.u.dc_desc.algo_type;
	}

	memset(ctx->sess->scratch.cmd, 0, CMD_SCRATCH_SZ);

	switch (pnso_algo_type) {
	case PNSO_COMPRESSION_TYPE_NONE:
		rc = svc_exec_noop(ctx, opaque);
		break;
	case PNSO_COMPRESSION_TYPE_LZRW1A:
		rc = svc_exec_decompress_lzrw1a(ctx, hdr_len,
						hdr_data_len ? hdr_data_len :
						ctx->input.len,
						pchksum);
		break;
	default:
		svc_exec_noop(ctx, opaque);
		rc = PNSO_ERR_CPDC_ALGO_INVALID;
		break;
	}

	ctx->status.u.dst.data_len = ctx->output.len;

	return rc;
}

static pnso_error_t svc_exec_encrypt(struct sim_svc_ctx *ctx,
				     void *opaque)
{
	pnso_error_t rc = PNSO_OK;
	uint8_t *iv_data;
	uint8_t *key1, *key2;
	uint32_t key_size;

	if (0 !=
	    sim_get_key_desc_idx((void **) &key1, (void **) &key2,
				 &key_size,
				 ctx->cmd.u.crypto_desc.key_desc_idx)) {
		return PNSO_ERR_CRYPTO_KEY_INDEX_OUT_OF_RANG;
	}

	memset(ctx->sess->scratch.cmd, 0, CMD_SCRATCH_SZ);

	switch (ctx->cmd.u.crypto_desc.algo_type) {
	case PNSO_CRYPTO_TYPE_NONE:
		rc = svc_exec_noop(ctx, opaque);
		break;
	case PNSO_CRYPTO_TYPE_XTS:
		if (key_size != 16 && key_size != 32) {
			rc = PNSO_ERR_CRYPTO_WRONG_KEY_TYPE;
			break;
		}
		iv_data = (uint8_t *)
			osal_phy_to_virt(ctx->cmd.u.crypto_desc.iv_addr);
#ifdef OPENSSL
		rc = algo_openssl_encrypt_xts(ctx->sess->scratch.cmd,
				key1, key2, key_size, iv_data,
				(uint8_t *) ctx->input.buf,
				ctx->input.len,
				(uint8_t *) ctx->output.buf,
				&ctx->output.len);
#else
		/* In-place encryption */
		rc = algo_encrypt_aes_xts(ctx->sess->scratch.cmd,
				key1, key2, key_size, iv_data,
				(uint8_t *) ctx->input.buf, ctx->input.len);
		if (rc == PNSO_OK) {
			ctx->output = ctx->input;
		}
#endif
		break;
	case PNSO_CRYPTO_TYPE_GCM:
	default:
		svc_exec_noop(ctx, opaque);
		rc = PNSO_ERR_CRYPTO_WRONG_KEY_TYPE; /* TODO: ALGO_INVALID */
		break;
	}

	if (rc == PNSO_OK) {
		ctx->status.u.dst.data_len = ctx->output.len;
	}

	return rc;
}

static pnso_error_t svc_exec_decrypt(struct sim_svc_ctx *ctx,
				     void *opaque)
{
	pnso_error_t rc = PNSO_OK;
	uint8_t *iv_data;
	uint8_t *key1, *key2;
	uint32_t key_size;

	if (0 !=
	    sim_get_key_desc_idx((void **) &key1, (void **) &key2,
				 &key_size,
				 ctx->cmd.u.crypto_desc.key_desc_idx)) {
		return PNSO_ERR_CRYPTO_KEY_INDEX_OUT_OF_RANG;
	}

	memset(ctx->sess->scratch.cmd, 0, CMD_SCRATCH_SZ);

	switch (ctx->cmd.u.crypto_desc.algo_type) {
	case PNSO_CRYPTO_TYPE_NONE:
		rc = svc_exec_noop(ctx, opaque);
		break;
	case PNSO_CRYPTO_TYPE_XTS:
		if (key_size != 16 && key_size != 32) {
			rc = PNSO_ERR_CRYPTO_WRONG_KEY_TYPE;
			break;
		}
		iv_data = (uint8_t *)
			osal_phy_to_virt(ctx->cmd.u.crypto_desc.iv_addr);
#ifdef OPENSSL
		rc = algo_openssl_decrypt_xts(ctx->sess->scratch.cmd,
				key1, key2, key_size, iv_data,
				(uint8_t *) ctx->input.buf,
				ctx->input.len,
				(uint8_t *) ctx->output.buf,
				&ctx->output.len);
#else
		/* In-place decryption */
		rc = algo_decrypt_aes_xts(ctx->sess->scratch.cmd,
				key1, key2, key_size, iv_data,
				(uint8_t *) ctx->input.buf, ctx->input.len);
		if (rc == PNSO_OK) {
			ctx->output = ctx->input;
		}
#endif
		break;
	case PNSO_CRYPTO_TYPE_GCM:
	default:
		svc_exec_noop(ctx, opaque);
		rc = PNSO_ERR_CRYPTO_WRONG_KEY_TYPE; /* TODO: ALGO_INVALID */
		break;
	}

	if (rc == PNSO_OK) {
		ctx->status.u.dst.data_len = ctx->output.len;
	}

	return rc;
}

typedef pnso_error_t(*svc_iterator_func_t) (struct sim_svc_ctx *ctx,
					    uint32_t block_idx,
					    struct pnso_flat_buffer *block,
					    void *opaque);

static pnso_error_t svc_iterate_blocks(struct sim_svc_ctx *ctx,
				       svc_iterator_func_t func,
				       void *opaque)
{
	pnso_error_t rc = PNSO_OK;
	size_t block_i;
	uint32_t block_count =
	    sim_flat_buffer_block_count(&ctx->input, ctx->sess->block_sz);
	struct pnso_flat_buffer block;

	for (block_i = 0; block_i < block_count; block_i++) {
		sim_flat_buffer_to_block(&ctx->input, &block,
					 ctx->sess->block_sz, block_i);
		rc = func(ctx, block_i, &block, opaque);
		if (rc != PNSO_OK)
			break;
	}

	return rc;
}

static pnso_error_t svc_exec_hash_one_block(struct sim_svc_ctx *ctx,
					    uint32_t block_idx,
					    struct pnso_flat_buffer *block,
					    void *opaque)
{
	int *call_count = (int *) opaque;
	pnso_error_t rc = PNSO_OK;

	(*call_count)++;

	OSAL_ASSERT(ctx->status.u.hash.num_tags > block_idx);

	memset(ctx->sess->scratch.cmd, 0, CMD_SCRATCH_SZ);

	switch (ctx->cmd.u.hash_desc.algo_type) {
	case PNSO_HASH_TYPE_SHA2_256:
		if (!algo_sha_gen
		    (ctx->sess->scratch.cmd,
		     ctx->status.u.hash.tags[block_idx].hash,
		     (uint8_t *) block->buf, block->len, 256)) {
			rc = PNSO_ERR_SHA_FAILED;
		}
		break;
	case PNSO_HASH_TYPE_SHA2_512:
		if (!algo_sha_gen(ctx->sess->scratch.cmd,
		     ctx->status.u.hash.tags[block_idx].hash,
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

static pnso_error_t svc_exec_hash(struct sim_svc_ctx *ctx,
				  void *opaque)
{
	int opaque_counter = 0;
	pnso_error_t rc;

	if (ctx->cmd.u.hash_desc.flags & PNSO_HASH_DFLAG_PER_BLOCK) {
		rc = svc_iterate_blocks(ctx, svc_exec_hash_one_block,
					&opaque_counter);
	} else {
		rc = svc_exec_hash_one_block(ctx, 0, &ctx->input,
					     &opaque_counter);
	}

	ctx->status.u.hash.num_tags = opaque_counter;

	/* Zero-copy output */
	ctx->output = ctx->input;

	return rc;
}

static pnso_error_t svc_exec_chksum_one_block(struct sim_svc_ctx *ctx,
					      uint32_t block_idx,
					      struct pnso_flat_buffer *block,
					      void *opaque)
{
	int *call_count = (int *) opaque;
	uint32_t temp32;
	uint64_t temp64;
	uint8_t *chksum_buf;
	pnso_error_t rc = PNSO_OK;

	(*call_count)++;

	OSAL_ASSERT(ctx->status.u.chksum.num_tags > block_idx);

	memset(ctx->status.u.chksum.tags[block_idx].chksum, 0, PNSO_CHKSUM_TAG_LEN);
	chksum_buf = ctx->status.u.chksum.tags[block_idx].chksum;

	switch (ctx->cmd.u.chksum_desc.algo_type) {
	case PNSO_CHKSUM_TYPE_NONE:
		rc = EINVAL;
		break;

	case PNSO_CHKSUM_TYPE_MCRC64:
		temp64 = algo_gen_mcrc64((uint8_t *) block->buf, block->len);
		*((uint64_t *) chksum_buf) = temp64;
		break;

	case PNSO_CHKSUM_TYPE_CRC32C:
		temp32 = algo_gen_crc32c((uint8_t *) block->buf, block->len);
		*((uint32_t *) chksum_buf) = temp32;
		break;

	case PNSO_CHKSUM_TYPE_ADLER32:
		temp32 = algo_gen_adler32((uint8_t *) block->buf, block->len);
		*((uint32_t *) chksum_buf) = temp32;
		break;

	case PNSO_CHKSUM_TYPE_MADLER32:
		temp32 = algo_gen_madler_with_pad((uint8_t *) block->buf, block->len);
		*((uint32_t *) chksum_buf) = temp32;
		break;

	default:
		rc = EINVAL;
		break;
	}

	/* Zero-copy output */
	ctx->output = ctx->input;

	return rc;
}

static pnso_error_t svc_exec_chksum(struct sim_svc_ctx *ctx,
				    void *opaque)
{
	int opaque_counter = 0;
	pnso_error_t rc = PNSO_OK;
	bool skip_chksum = false;

	/* Optimization: skip checksum after decompress, if not present */
	if (ctx->prev_ctx &&
	    ctx->prev_ctx->cmd.svc_type == PNSO_SVC_TYPE_DECOMPRESS &&
	    ctx->prev_ctx->is_chksum_present == 0) {
		skip_chksum = true;
	}

	if (!skip_chksum) {
		if (ctx->cmd.u.chksum_desc.flags &
		    PNSO_CHKSUM_DFLAG_PER_BLOCK) {
			rc = svc_iterate_blocks(ctx, svc_exec_chksum_one_block,
						&opaque_counter);
		} else {
			rc = svc_exec_chksum_one_block(ctx, 0, &ctx->input,
						       &opaque_counter);
		}
	}

	ctx->status.u.chksum.num_tags = opaque_counter;

	/* Zero-copy output */
	ctx->output = ctx->input;

	return rc;
}


/* Traverses compacted block to find desired data, and does
 * decompression if necessary.
 */
static pnso_error_t svc_exec_decompact(struct sim_svc_ctx *ctx,
				       void *opaque)
{
	size_t i;
	wafl_packed_blk_t *wafl_blk = (wafl_packed_blk_t *) ctx->input.buf;
	wafl_packed_data_info_t *wafl_data;

	OSAL_ASSERT(ctx->input.len >= sizeof(wafl_packed_blk_t));
	OSAL_ASSERT(ctx->input.len >= sizeof(wafl_packed_blk_t) +
		    wafl_blk->wpb_hdr.wpbh_num_objs *
		    sizeof(wafl_packed_data_info_t));

	for (i = 0; i < wafl_blk->wpb_hdr.wpbh_num_objs; i++) {
		wafl_data = &wafl_blk->wpb_data_info[i];
		if (wafl_data->wpd_vvbn == ctx->cmd.u.decompact_desc.vvbn) {
			/* Found the right data, now decompress or copy */
			OSAL_ASSERT(wafl_data->wpd_len +
				    wafl_data->wpd_off <=
				    ctx->sess->block_sz);
			if ((wafl_data->wpd_flags & WAFL_PACKED_DATA_ENCODED)
			    /*|| !ctx->cmd.u.decompact_desc.is_uncompressed*/) {
				/* Decompress single or multi-block */
				struct sim_cp_header_format *hdr_fmt = NULL;
				uint8_t *hdr = (uint8_t *) (ctx->input.buf +
							    wafl_data->wpd_off);
				uint32_t hdr_len = 0;
				uint32_t dst_len = 0;
				uint32_t hdr_cp_len = 0;
				uint32_t hdr_algo_type;
				uint64_t hdr_chksum;

				if (wafl_data->wpd_flags &
				    WAFL_PACKED_DATA_ENCODED) {
					hdr_fmt = sim_lookup_hdr_format(
						ctx->cmd.u.decompact_desc.hdr_fmt_idx,
						false);
					if (!hdr_fmt) {
						/* Error */
						break;
					}
					hdr_len = hdr_fmt->total_hdr_sz;

					if (wafl_data->wpd_len <= hdr_len) {
						/* Error */
						break;
					}
					parse_cp_hdr(ctx, hdr_fmt,
						     &hdr_cp_len,
						     &hdr_chksum,
						     &hdr_algo_type,
						     hdr);
					if (hdr_cp_len >= wafl_data->wpd_len) {
						/* Error */
						break;
					}
					if (hdr_algo_type !=
					    PNSO_COMPRESSION_TYPE_LZRW1A) {
						/* Error */
						break;
					}
				}

				memset(ctx->sess->scratch.cmd, 0,
				       CMD_SCRATCH_SZ);
				lzrw1a_compress(COMPRESS_ACTION_DECOMPRESS,
						ctx->sess->scratch.cmd,
						(uint8_t *) ctx->input.buf +
						wafl_data->wpd_off +
						hdr_len,
						hdr_cp_len ? hdr_cp_len :
						wafl_data->wpd_len,
						(uint8_t *) ctx->output.buf,
						&dst_len
#ifdef LZR_MODEL
						, dst_len, (uint32_t *) &hdr_chksum
#endif
						);
				ctx->output.len = dst_len;
			} else {
				/* Uncompressed.  Just copy. */
				OSAL_ASSERT(wafl_data->wpd_len <
					    ctx->sess->block_sz);
				memcpy((void *) ctx->output.buf,
				       (void *) ctx->input.buf +
				       wafl_data->wpd_off,
				       wafl_data->wpd_len);
				ctx->output.len = wafl_data->wpd_len;
			}

			ctx->status.u.dst.data_len = ctx->output.len;

			return PNSO_OK;
		}
	}

	return EINVAL;
}
