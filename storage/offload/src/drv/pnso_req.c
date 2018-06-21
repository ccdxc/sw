/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "pnso_global.h"
#include "pnso_logger.h"
#include "pnso_osal.h"

#include "pnso_api.h"
#include "pnso_req.h"
#include "pnso_chain.h"
#include "pnso_pbuf.h"

typedef pnso_error_t (*validate_req_service_fn_t)(struct pnso_service *svc);

typedef pnso_error_t (*validate_res_service_fn_t)(struct pnso_service_status *svc);

static void
__pprint_crypto_desc(struct pnso_crypto_desc *desc)
{
	if (!desc)
		return;

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "=== crypto_desc", desc);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "algo_type", desc->algo_type);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "rsvd", desc->rsvd);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "key_desc_idx", desc->key_desc_idx);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %ju", "key_desc_idx", desc->iv_addr);
}

static void
__pprint_cp_desc(struct pnso_compression_desc *desc)
{
	if (!desc)
		return;

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "=== cp_desc", desc);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "algo_type", desc->algo_type);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "flags", desc->flags);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "threshold_len",
			desc->threshold_len);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "hdr_fmt_idx", desc->hdr_fmt_idx);
}

static void
__pprint_dc_desc(struct pnso_decompression_desc *desc)
{
	if (!desc)
		return;

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "=== dc_desc", desc);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "algo_type", desc->algo_type);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "flags", desc->flags);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "hdr_fmt_idx", desc->hdr_fmt_idx);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "rsvd", desc->rsvd);
}

static void
__pprint_hash_desc(struct pnso_hash_desc *desc)
{
	if (!desc)
		return;

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "=== hash_desc", desc);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "algo_type", desc->algo_type);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "flags", desc->flags);
}

static void
__pprint_chksum_desc(struct pnso_checksum_desc *desc)
{
	if (!desc)
		return;

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "=== chksum_desc", desc);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "algo_type", desc->algo_type);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "flags", desc->flags);
}

static void
__pprint_decompaction_desc(struct pnso_decompaction_desc *desc)
{
	if (!desc)
		return;

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "=== decompact_desc", desc);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "algo_type", desc->vvbn);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "rsvc_1", desc->rsvd_1);
}

void
req_pprint_request(struct pnso_service_request *req)
{
	uint32_t i;

	if (!req)
		return;

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "=== pnso_service_request", req);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "=== sgl", req->sgl);
	pbuf_pprint_buffer_list(req->sgl);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "num_services", req->num_services);
	for (i = 0; i < req->num_services; i++) {
		PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "service #", i+1);

		PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "svc_type",
				req->svc[i].svc_type);
		PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "rsvd", req->svc[i].rsvd);

		switch (req->svc[i].svc_type) {
		case PNSO_SVC_TYPE_ENCRYPT:
			__pprint_crypto_desc(&req->svc[i].u.crypto_desc);
			break;
		case PNSO_SVC_TYPE_DECRYPT:
			__pprint_crypto_desc(&req->svc[i].u.crypto_desc);
			break;
		case PNSO_SVC_TYPE_COMPRESS:
			__pprint_cp_desc(&req->svc[i].u.cp_desc);
			break;
		case PNSO_SVC_TYPE_DECOMPRESS:
			__pprint_dc_desc(&req->svc[i].u.dc_desc);
			break;
		case PNSO_SVC_TYPE_HASH:
			__pprint_hash_desc(&req->svc[i].u.hash_desc);
			break;
		case PNSO_SVC_TYPE_CHKSUM:
			__pprint_chksum_desc(&req->svc[i].u.chksum_desc);
			break;
		case PNSO_SVC_TYPE_DECOMPACT:
			__pprint_decompaction_desc(
					&req->svc[i].u.decompact_desc);
			break;
		default:
			assert(0);
			break;
		}
	}
}

void
req_pprint_result(struct pnso_service_result *res)
{
	uint32_t i;

	if (!res)
		return;

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "=== pnso_service_result", res);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "err", res->err);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "num_services", res->num_services);
	for (i = 0; i < res->num_services; i++) {
		PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "service #", i+1);

		PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "err", res->svc[i].err);
		PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "svc_type",
				res->svc[i].svc_type);
		PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "rsvd_1",
				res->svc[i].rsvd_1);

		switch (res->svc[i].svc_type) {
		case PNSO_SVC_TYPE_ENCRYPT:
		case PNSO_SVC_TYPE_DECRYPT:
		case PNSO_SVC_TYPE_COMPRESS:
		case PNSO_SVC_TYPE_DECOMPRESS:
		case PNSO_SVC_TYPE_DECOMPACT:
			PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "data_len",
					res->svc[i].u.dst.data_len);
			pbuf_pprint_buffer_list(res->svc[i].u.dst.sgl);
			break;
		case PNSO_SVC_TYPE_HASH:
			PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "num_tags",
					res->svc[i].u.hash.num_tags);
			PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "rsvd_2",
					res->svc[i].u.hash.rsvd_2);
			/* TODO: print SHA/tags */
			break;
		case PNSO_SVC_TYPE_CHKSUM:
			PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "num_tags",
					res->svc[i].u.chksum.num_tags);
			PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "rsvd_3",
					res->svc[i].u.chksum.rsvd_3);
			/* TODO: print chksum/tags */
			break;
		default:
			assert(0);
			break;
		}
	}
}

static pnso_error_t
__validate_req_encryption_service(struct pnso_service *svc)
{
	pnso_error_t err;

	/* TODO: validate service input */
	err = PNSO_OK;

	return err;
}

static pnso_error_t
__validate_req_decryption_service(struct pnso_service *svc)
{
	pnso_error_t err;

	/* TODO: validate service input */
	err = PNSO_OK;

	return err;
}

static pnso_error_t
__validate_req_compression_service(struct pnso_service *svc)
{
	pnso_error_t err;

	if (!svc) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "invalid service! svc: %p", svc);
		assert(0);
		goto out;
	}

	if (svc->svc_type != PNSO_SVC_TYPE_COMPRESS) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "invalid service type specified! svc: %p svc_type: %d",
				svc, svc->svc_type);
		assert(0);
		goto out;
	}

	/* TODO: validate threshold len, hdr format index */

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
__validate_req_decompression_service(struct pnso_service *svc)
{
	pnso_error_t err;

	if (!svc) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "invalid service! svc: %p", svc);
		assert(0);
		goto out;
	}

	if (svc->svc_type != PNSO_SVC_TYPE_DECOMPRESS) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "invalid service type specified! svc: %p svc_type: %d",
				svc, svc->svc_type);
		assert(0);
		goto out;
	}

	/* TODO: validate threshold len, hdr format index */

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
__validate_req_hash_service(struct pnso_service *svc)
{
	pnso_error_t err;

	if (!svc) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "invalid service! svc: %p", svc);
		assert(0);
		goto out;
	}

	if (svc->svc_type != PNSO_SVC_TYPE_HASH) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "invalid service type specified! svc: %p svc_type: %d",
				svc, svc->svc_type);
		assert(0);
		goto out;
	}

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
__validate_req_chksum_service(struct pnso_service *svc)
{
	pnso_error_t err;

	if (!svc) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "invalid service! svc: %p", svc);
		assert(0);
		goto out;
	}

	if (svc->svc_type != PNSO_SVC_TYPE_CHKSUM) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "invalid service type specified! svc: %p svc_type: %d",
				svc, svc->svc_type);
		assert(0);
		goto out;
	}

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
__validate_req_decompaction_service(struct pnso_service *svc)
{
	pnso_error_t err;

	/* TODO: validate service input */
	err = PNSO_OK;

	return err;
}

static validate_req_service_fn_t validate_req_service_fn[PNSO_SVC_TYPE_MAX] = {
	__validate_req_encryption_service,
	__validate_req_decryption_service,
	__validate_req_compression_service,
	__validate_req_decompression_service,
	__validate_req_hash_service,
	__validate_req_chksum_service,
	__validate_req_decompaction_service,
};

static bool __attribute__ ((unused))
__is_service_type_valid(enum pnso_service_type svc_type)
{
	switch (svc_type) {
	case PNSO_SVC_TYPE_ENCRYPT:
	case PNSO_SVC_TYPE_DECRYPT:
	case PNSO_SVC_TYPE_COMPRESS:
	case PNSO_SVC_TYPE_DECOMPRESS:
	case PNSO_SVC_TYPE_HASH:
	case PNSO_SVC_TYPE_CHKSUM:
	case PNSO_SVC_TYPE_DECOMPACT:
		return true;
	default:
		assert(0);
		break;
	}

	return false;
}

static pnso_error_t __attribute__ ((unused))
__validate_res_service(struct pnso_service_status *status)
{
	pnso_error_t err;

	if (!status) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "invalid status! status: %p", status);
		assert(0);
		goto out;
	}

	if (!__is_service_type_valid(status->svc_type)) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "invalid service type specified! status: %p svc_type: %d",
				status, status->svc_type);
		assert(0);
		goto out;
	}

	err = PNSO_OK;

out:
	return err;
}

static pnso_error_t
__validate_res_dummy_service(struct pnso_service_status *status)
{
	assert(0);
	return EINVAL;
}

static pnso_error_t
__validate_res_encryption_service(struct pnso_service_status *status)
{
	assert(0);
	return EINVAL;
}

static pnso_error_t
__validate_res_decryption_service(struct pnso_service_status *status)
{
	assert(0);
	return EINVAL;
}

static pnso_error_t
__validate_res_compression_service(struct pnso_service_status *status)
{
	pnso_error_t err;

	if (!status->u.dst.sgl) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "invalid dst buffer list specified for compression! sgl: %p",
				status->u.dst.sgl);
		return err;
	}

	return PNSO_OK;
}

static pnso_error_t
__validate_res_decompression_service(struct pnso_service_status *status)
{
	pnso_error_t err;

	if (!status->u.dst.sgl) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "invalid dst buffer list specified for decompression! sgl: %p",
				status->u.dst.sgl);
		return err;
	}

	return PNSO_OK;
}

static pnso_error_t
__validate_res_hash_service(struct pnso_service_status *status)
{
	pnso_error_t err;

	if (status->u.hash.num_tags <= 0) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "invalid number of tags specified for hash! num_ttags: %d",
				status->u.hash.num_tags);
		goto out;
	}

	if (!status->u.hash.tags) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "invalid tags specified for hash! tags: %p",
				status->u.hash.tags);
		goto out;
	}

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
__validate_res_chksum_service(struct pnso_service_status *status)
{
	pnso_error_t err;

	if (status->u.chksum.num_tags <= 0) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "invalid number of tags specified for chksum! num_ttags: %d",
				status->u.chksum.num_tags);
		goto out;
	}

	if (!status->u.chksum.tags) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "invalid tags specified for chksum! tags: %p",
				status->u.chksum.tags);
		goto out;
	}

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
__validate_res_decompaction_service(struct pnso_service_status *status)
{
	assert(0);
	return EINVAL;
}

static validate_res_service_fn_t validate_res_service_fn[PNSO_SVC_TYPE_MAX] = {
	[PNSO_SVC_TYPE_NONE] = __validate_res_dummy_service,
	[PNSO_SVC_TYPE_ENCRYPT] = __validate_res_encryption_service,
	[PNSO_SVC_TYPE_DECRYPT] = __validate_res_decryption_service,
	[PNSO_SVC_TYPE_COMPRESS] = __validate_res_compression_service,
	[PNSO_SVC_TYPE_DECOMPRESS] = __validate_res_decompression_service,
	[PNSO_SVC_TYPE_HASH] = __validate_res_hash_service,
	[PNSO_SVC_TYPE_CHKSUM] = __validate_res_chksum_service,
	[PNSO_SVC_TYPE_DECOMPACT] = __validate_res_decompaction_service,
};

pnso_error_t
validate_service_request(struct pnso_service_request *svc_req)
{
	pnso_error_t err;
	uint32_t num_services;
	uint32_t i;

	if (!svc_req)
		return EINVAL;

	num_services = svc_req->num_services;
	if (num_services <= PNSO_SVC_TYPE_NONE ||
	    num_services >= PNSO_SVC_TYPE_MAX)
		return EINVAL;

	if (!svc_req->sgl)
		return EINVAL;

	/* sgl here means buffer list ... */
	if (svc_req->sgl->count < 1)
		return EINVAL;

	for (i = 0; i < num_services; i++) {
		err = validate_req_service_fn[i](&svc_req->svc[i]);
		if (err)
			return err;
	}

	return err;
}

pnso_error_t
validate_service_result(struct pnso_service_result *svc_res)
{
	pnso_error_t err = EINVAL;
	enum pnso_service_type svc_type;
	uint32_t num_services, i;

	if (!svc_res) {
		PNSO_LOG_ERROR(err, "invalid service result specified! res: %p", svc_res);
		goto out;
	}

	num_services = svc_res->num_services;
	for (i = 0; i < num_services; i++) {
		svc_type = svc_res->svc[i].svc_type;

		if (svc_type <= PNSO_SVC_TYPE_NONE ||
				svc_type >= PNSO_SVC_TYPE_MAX) {
			err = EINVAL;
			PNSO_LOG_ERROR(err, "invalid service type specified! service #: %d svc_type: %d",
					i+1, svc_type);
			assert(0);
			goto out;
		}

		err = validate_res_service_fn[svc_type](&svc_res->svc[i]);
		if (err)
			return err;
	}

out:
	return err;
}

pnso_error_t pnso_submit_request(struct pnso_service_request *svc_req,
				 struct pnso_service_result *svc_res,
				 completion_cb_t cb,
				 void *cb_ctx,
				 pnso_poll_fn_t *pnso_poll_fn,
				 void **pnso_poll_ctx)
{
	pnso_error_t err;

	PNSO_LOG_INFO(PNSO_OK, "start ...");

	/* validate each service request */
	err = validate_service_request(svc_req);
	if (err)
		return err;

	/* validate each service result */
	err = validate_service_result(svc_res);
	if (err)
		return err;

	/* TODO: this one is hanging out ... */
	if (svc_req->num_services != svc_res->num_services)
		return EINVAL;

	/* TODO:
	 *      sanitize cb, cb_ctx, etc. here and bail out from this layer
	 *      before going deeper into other layers
	 */

	/* build service chain */
	err = chn_build_service_chain(svc_req, svc_res, cb, cb_ctx,
				      pnso_poll_fn, pnso_poll_ctx);

	PNSO_LOG_INFO(err, "done");

	return err;
}
