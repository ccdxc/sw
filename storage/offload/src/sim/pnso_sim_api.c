/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include "osal_errno.h"
#include "osal_mem.h"
#include "osal_sys.h"

#include "sim.h"
#include "sim_chain.h"
#include "sim_util.h"
#include "sim_worker.h"

struct pnso_init_params g_init_params;

pnso_error_t pnso_init(struct pnso_init_params *init_params)
{
	pnso_error_t rc = PNSO_OK;

	g_init_params = *init_params;
	sim_init_globals();
	if ((rc = sim_init_req_pool(SIM_DEFAULT_REQ_COUNT)) != PNSO_OK) {
		return rc;
	}
	if ((rc = sim_init_worker_pool(SIM_DEFAULT_SQ_DEPTH)) != PNSO_OK) {
		return rc;
	}
	rc = sim_key_store_init(SIM_KEY_STORE_SZ);

	return rc;
}

pnso_error_t pnso_register_compression_header_format(
		struct pnso_compression_header_format *cp_hdr_fmt,
		uint16_t hdr_fmt_idx)
{
	size_t i, total_hdr_len;
	struct sim_cp_header_format *format;

	/* Basic validation */
	if (!hdr_fmt_idx) {
		return PNSO_ERR_CPDC_HDR_IDX_INVALID;
	}
	if (!cp_hdr_fmt || cp_hdr_fmt->num_fields > PNSO_MAX_HEADER_FIELDS) {
		return EINVAL;
	}

	/* Find a suitable table entry */
	format = sim_lookup_hdr_format(hdr_fmt_idx, true);
	if (!format) {
		return ENOMEM;
	}

	/* Fill the entry */
	format->fmt_idx = hdr_fmt_idx;
	format->fmt = *cp_hdr_fmt;

	/* Find the total header length */
	total_hdr_len = 0;
	for (i = 0; i < cp_hdr_fmt->num_fields; i++) {
		uint32_t tmp = cp_hdr_fmt->fields[i].offset +
			       cp_hdr_fmt->fields[i].length;
		if (tmp > total_hdr_len) {
			total_hdr_len = tmp;
		}
	}

	/* Allocate static header */
	if (format->static_hdr && (total_hdr_len > format->total_hdr_sz)) {
		/* Avoid memory leak of previously allocated format */
		osal_free(format->static_hdr);
		format->static_hdr = NULL;
		format->total_hdr_sz = 0;
	}
	if (total_hdr_len && !format->static_hdr) {
		format->static_hdr = osal_alloc(total_hdr_len);
		if (!format->static_hdr) {
			return ENOMEM;
		}
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

/* Assumes mapping is 1:1 */
pnso_error_t pnso_add_compression_algo_mapping(
		enum pnso_compression_type pnso_algo,
		uint32_t header_algo)
{
	if (pnso_algo >= PNSO_COMPRESSION_TYPE_MAX) {
		return EINVAL;
	}
	sim_set_algo_mapping(pnso_algo, header_algo);
	return PNSO_OK;
}

pnso_error_t pnso_sim_thread_init(int core_id)
{
	pnso_error_t rc;

	rc = sim_init_session(core_id);
	if (rc != PNSO_OK) {
		return rc;
	}
	rc = sim_start_worker_thread(core_id);
	return rc;
}

void pnso_sim_thread_finit(int core_id)
{
	if (PNSO_OK == sim_stop_worker_thread(core_id)) {
		sim_finit_session(core_id);
	}
}

/* Free resources used by sim.  Assumes no worker threads running. */
void pnso_sim_finit(void)
{
	sim_key_store_finit();
}

pnso_error_t pnso_add_to_batch(struct pnso_service_request *svc_req,
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

pnso_error_t pnso_flush_batch(completion_cb_t cb,
		void *cb_ctx,
		pnso_poll_fn_t *pnso_poll_fn,
		void **pnso_poll_ctx)
{
	int core_id = osal_get_coreid();

	if (!sim_is_worker_running(core_id)) {
		return EINVAL;
	}

	if (pnso_poll_fn) {
		*pnso_poll_fn = pnso_sim_poll;
	}
	return sim_sq_flush(core_id, cb, cb_ctx, pnso_poll_ctx);
}

pnso_error_t pnso_submit_request(struct pnso_service_request *svc_req,
				 struct pnso_service_result *svc_res,
				 completion_cb_t cb,
				 void *cb_ctx,
				 pnso_poll_fn_t *poll_fn,
				 void **poll_ctx)
{
	pnso_error_t rc;
	int core_id = osal_get_coreid();

	if (!sim_is_worker_running(core_id)) {
		if ((rc = pnso_sim_thread_init(core_id)) != PNSO_OK) {
			return rc;
		}
	}

	if (cb == NULL) {
		void *priv_poll_ctx;

		/* Synchronous request */
#if 0
		if (!sim_is_worker_running(core_id)) {
			/* No worker thread, run directly in this thread */
			return sim_execute_request(sim_get_worker_ctx(core_id),
						   svc_req, svc_res,
						   NULL, NULL);
		}
#endif

		/* Local polling mode */
		rc = sim_sq_enqueue(core_id, svc_req, svc_res, NULL,
				    NULL, &priv_poll_ctx, true);
		if (rc == PNSO_OK) {
			rc = pnso_sim_poll_wait(priv_poll_ctx, core_id);
		}
		return rc;
	}

	/* Asynchronous request */
	rc = sim_sq_enqueue(core_id, svc_req, svc_res, cb,
			    cb_ctx, poll_ctx, true);
	if (poll_fn) {
		*poll_fn = pnso_sim_poll;
	}
	return rc;
}

