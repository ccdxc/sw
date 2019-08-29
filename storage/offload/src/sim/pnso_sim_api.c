/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include "osal_errno.h"
#include "osal_mem.h"
#include "osal_sys.h"
#include "osal_setup.h"

#include "sim.h"
#include "sim_chain.h"
#include "sim_util.h"
#include "sim_worker.h"

#ifndef ENABLE_PNSO_SONIC_TEST
OSAL_LICENSE("Dual BSD/GPL");
#define SIM_EXPORT(fn) OSAL_EXPORT_SYMBOL(fn)
#define SIM_EXPORT_API(fn) OSAL_EXPORT_SYMBOL(SIM_API(fn))
#else
#define SIM_EXPORT(fn)
#define SIM_EXPORT_API(fn)
#endif

static struct pnso_init_params g_init_params;

pnso_error_t SIM_API(init)(struct pnso_init_params *init_params)
{
	pnso_error_t rc = PNSO_OK;

	g_init_params = *init_params;
	if (init_params->per_core_qdepth == 0) {
		g_init_params.per_core_qdepth = SIM_DEFAULT_SQ_DEPTH;
	} else if (init_params->per_core_qdepth > SIM_MAX_Q_DEPTH) {
		g_init_params.per_core_qdepth = SIM_MAX_Q_DEPTH;
	}

	sim_init_globals();
	if ((rc = sim_init_req_pool(SIM_DEFAULT_REQ_COUNT)) != PNSO_OK) {
		return rc;
	}
	if ((rc = sim_init_worker_pool(g_init_params.per_core_qdepth)) !=
	    PNSO_OK) {
		return rc;
	}

	rc = sim_key_store_init(SIM_KEY_STORE_SZ);

	return rc;
}
SIM_EXPORT_API(init);

pnso_error_t SIM_API(register_compression_header_format)(
		struct pnso_compression_header_format *cp_hdr_fmt,
		uint16_t hdr_fmt_idx)
{
	size_t i, total_hdr_len;
	uint32_t tmp;
	struct sim_cp_header_format *format;

	/* Basic validation */
	if (!hdr_fmt_idx) {
		return PNSO_ERR_CPDC_HDR_IDX_INVALID;
	}
	if (!cp_hdr_fmt || cp_hdr_fmt->num_fields > PNSO_MAX_HEADER_FIELDS) {
		return EINVAL;
	}

	/* Find the total header length */
	total_hdr_len = 0;
	for (i = 0; i < cp_hdr_fmt->num_fields; i++) {
		tmp = cp_hdr_fmt->fields[i].offset +
			cp_hdr_fmt->fields[i].length;
		if (tmp > total_hdr_len) {
			total_hdr_len = tmp;
		}
	}
	if (!total_hdr_len)
		return EINVAL;

	/* Find a suitable table entry */
	format = sim_lookup_hdr_format(hdr_fmt_idx, true);
	if (!format) {
		return ENOMEM;
	}

	/* Fill the entry */
	format->fmt_idx = hdr_fmt_idx;
	format->fmt = *cp_hdr_fmt;


	/* Generate static header */
	if (total_hdr_len > SIM_MAX_CP_HDR_SZ) {
		return ENOMEM;
	}
	format->total_hdr_sz = total_hdr_len;

	/* Populate static header and store aggregate flags */
	format->type_mask = 0;
	memset(format->static_hdr, 0, total_hdr_len);
	for (i = 0; i < cp_hdr_fmt->num_fields; i++) {
		struct pnso_header_field *tlv = &cp_hdr_fmt->fields[i];

		sim_tlv_to_buf(format->static_hdr+tlv->offset, tlv->length,
			       tlv->value);
		format->type_mask |= 1 << tlv->type;
	}

	return PNSO_OK;
}
SIM_EXPORT_API(register_compression_header_format);

/* Assumes mapping is 1:1 */
pnso_error_t SIM_API(add_compression_algo_mapping)(
		enum pnso_compression_type pnso_algo,
		uint32_t header_algo)
{
	if (pnso_algo >= PNSO_COMPRESSION_TYPE_MAX) {
		return EINVAL;
	}
	sim_set_algo_mapping(pnso_algo, header_algo);
	return PNSO_OK;
}
SIM_EXPORT_API(add_compression_algo_mapping);

pnso_error_t pnso_sim_thread_init(int core_id)
{
	pnso_error_t rc;

	rc = sim_init_session(core_id, &g_init_params);
	if (rc != PNSO_OK) {
		return rc;
	}
	rc = sim_start_worker_thread(core_id);

	return rc;
}
SIM_EXPORT(pnso_sim_thread_init);

void pnso_sim_thread_finit(int core_id)
{
	if (PNSO_OK == sim_stop_worker_thread(core_id)) {
		sim_finit_session(core_id);
	}
}
SIM_EXPORT(pnso_sim_thread_finit);

/* Free resources used by sim.  Assumes no worker threads running. */
void pnso_sim_finit(void)
{
	uint32_t i;

	for (i = 0; i < osal_get_core_count(); i++) {
		pnso_sim_thread_finit(i);
	}

	sim_key_store_finit();

	sim_finit_worker_pool();
	sim_finit_req_pool();
}
SIM_EXPORT(pnso_sim_finit);

pnso_error_t SIM_API(add_to_batch)(struct pnso_service_request *svc_req,
		struct pnso_service_result *svc_res)
{
	pnso_error_t rc;
	int core_id = osal_get_coreid();

	if (!sim_is_worker_running(core_id)) {
		if ((rc = pnso_sim_thread_init(core_id)) != PNSO_OK) {
			return rc;
		}
	}

	return sim_sq_enqueue(core_id, svc_req, svc_res,
			      NULL, NULL, NULL, false);
}
SIM_EXPORT_API(add_to_batch);

pnso_error_t SIM_API(flush_batch)(completion_cb_t cb,
		void *cb_ctx,
		pnso_poll_fn_t *pnso_poll_fn,
		void **pnso_poll_ctx)
{
	pnso_error_t rc;
	bool is_sync = (cb == NULL) && (pnso_poll_fn == NULL);
	int core_id = osal_get_coreid();

	if (!sim_is_worker_running(core_id)) {
		return EINVAL;
	}

	if (is_sync) {
		/* Synchronous request, wait for completion */
		rc = sim_sq_flush(core_id, pnso_sim_sync_completion_cb,
				  (void *) (uint64_t) core_id, NULL);
		if (rc == PNSO_OK) {
#ifdef PNSO_SIM_THREADLESS
			pnso_sim_run_worker_once(sim_lookup_worker_ctx(core_id));
#else
			rc = pnso_sim_sync_wait(core_id);
#endif
		}
	} else {
		rc = sim_sq_flush(core_id, cb, cb_ctx, pnso_poll_ctx);
	}

	if (pnso_poll_fn) {
		*pnso_poll_fn = pnso_sim_poll;
	}

	return rc;
}
SIM_EXPORT_API(flush_batch);

pnso_error_t SIM_API(submit_request)(struct pnso_service_request *svc_req,
				 struct pnso_service_result *svc_res,
				 completion_cb_t cb,
				 void *cb_ctx,
				 pnso_poll_fn_t *poll_fn,
				 void **poll_ctx)
{
	pnso_error_t rc;
	bool is_sync = (cb == NULL) && (poll_fn == NULL);
	int core_id = osal_get_coreid();

	if (!sim_is_worker_running(core_id)) {
		if ((rc = pnso_sim_thread_init(core_id)) != PNSO_OK) {
			return rc;
		}
	}

	if (is_sync) {
		/* Synchronous request, wait for completion */
		rc = sim_sq_enqueue(core_id, svc_req, svc_res,
				    pnso_sim_sync_completion_cb,
				    (void *) (uint64_t) core_id,
				    NULL, true);
		if (rc == PNSO_OK) {
#ifdef PNSO_SIM_THREADLESS
			pnso_sim_run_worker_once(sim_lookup_worker_ctx(core_id));
#else
			rc = pnso_sim_sync_wait(core_id);
#endif
		}
	} else {
		rc = sim_sq_enqueue(core_id, svc_req, svc_res,
				    cb, cb_ctx, poll_ctx, true);
		if (poll_fn) {
			*poll_fn = pnso_sim_poll;
		}
	}

	return rc;
}
SIM_EXPORT_API(submit_request);

#ifdef __KERNEL__
#ifndef ENABLE_PNSO_SONIC_TEST
static int
sim_init(void)
{
	return 0;
}

static int
sim_fini(void)
{
	pnso_sim_finit();
	return 0;
}

static int
body(void)
{
	return 0;
}

osal_init_fn_t init_fp = sim_init;
osal_init_fn_t fini_fp = sim_fini;

OSAL_SETUP(init_fp, body, fini_fp);
#endif
#endif
