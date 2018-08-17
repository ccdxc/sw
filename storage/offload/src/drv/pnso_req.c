/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "pnso_pbuf.h"
#include "pnso_req.h"
#include "pnso_chain.h"

typedef pnso_error_t (*validate_req_service_fn_t)(struct pnso_service *svc);

typedef pnso_error_t (*validate_res_service_fn_t)(
		struct pnso_service_status *svc);

static void
__pprint_crypto_desc(struct pnso_crypto_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "=== crypto_desc", (u64) desc);

	OSAL_LOG_INFO("%30s: %d", "algo_type", desc->algo_type);
	OSAL_LOG_INFO("%30s: %d", "rsvd", desc->rsvd);
	OSAL_LOG_INFO("%30s: %d", "key_desc_idx", desc->key_desc_idx);
	OSAL_LOG_INFO("%30s: %llx", "key_desc_idx", desc->iv_addr);
}

static void
__pprint_cp_desc(struct pnso_compression_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "=== cp_desc", (u64) desc);

	OSAL_LOG_INFO("%30s: %d", "algo_type", desc->algo_type);
	OSAL_LOG_INFO("%30s: %d", "flags", desc->flags);
	OSAL_LOG_INFO("%30s: %d", "threshold_len",
			desc->threshold_len);
	OSAL_LOG_INFO("%30s: %d", "hdr_fmt_idx", desc->hdr_fmt_idx);
}

static void
__pprint_dc_desc(struct pnso_decompression_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "=== dc_desc", (u64) desc);

	OSAL_LOG_INFO("%30s: %d", "algo_type", desc->algo_type);
	OSAL_LOG_INFO("%30s: %d", "flags", desc->flags);
	OSAL_LOG_INFO("%30s: %d", "hdr_fmt_idx", desc->hdr_fmt_idx);
	OSAL_LOG_INFO("%30s: %d", "rsvd", desc->rsvd);
}

static void
__pprint_hash_desc(struct pnso_hash_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "=== hash_desc", (u64) desc);

	OSAL_LOG_INFO("%30s: %d", "algo_type", desc->algo_type);
	OSAL_LOG_INFO("%30s: %d", "flags", desc->flags);
}

static void
__pprint_chksum_desc(struct pnso_checksum_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "=== chksum_desc", (u64) desc);

	OSAL_LOG_INFO("%30s: %d", "algo_type", desc->algo_type);
	OSAL_LOG_INFO("%30s: %d", "flags", desc->flags);
}

static void
__pprint_decompaction_desc(struct pnso_decompaction_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "=== decompact_desc", (u64) desc);

	OSAL_LOG_INFO("%30s: %llu", "vvbn", (uint64_t) desc->vvbn);
	OSAL_LOG_INFO("%30s: %d", "rsvd_1", desc->rsvd_1);
	OSAL_LOG_INFO("%30s: %d", "hdr_fmt_idx", desc->hdr_fmt_idx);
	OSAL_LOG_INFO("%30s: %d", "rsvd_2", desc->rsvd_2);
}

void
req_pprint_request(struct pnso_service_request *req)
{
	uint32_t i;

	if (!req)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "=== pnso_service_request", (u64) req);

	OSAL_LOG_INFO("%30s: 0x%llx", "=== sgl", (u64) req->sgl);
	pbuf_pprint_buffer_list(req->sgl);

	OSAL_LOG_INFO("%30s: %d", "num_services", req->num_services);
	for (i = 0; i < req->num_services; i++) {
		OSAL_LOG_INFO("%30s: %d", "service #", i+1);

		OSAL_LOG_INFO("%30s: %d", "svc_type",
				req->svc[i].svc_type);
		OSAL_LOG_INFO("%30s: %d", "rsvd", req->svc[i].rsvd);

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
			OSAL_ASSERT(0);
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

	OSAL_LOG_INFO("%30s: 0x%llx", "=== pnso_service_result", (u64) res);

	OSAL_LOG_INFO("%30s: %d", "err", res->err);

	OSAL_LOG_INFO("%30s: %d", "num_services", res->num_services);
	for (i = 0; i < res->num_services; i++) {
		OSAL_LOG_INFO("%30s: %d", "service #", i+1);

		OSAL_LOG_INFO("%30s: %d", "err", res->svc[i].err);
		OSAL_LOG_INFO("%30s: %d", "svc_type",
				res->svc[i].svc_type);
		OSAL_LOG_INFO("%30s: %d", "rsvd_1",
				res->svc[i].rsvd_1);

		switch (res->svc[i].svc_type) {
		case PNSO_SVC_TYPE_ENCRYPT:
		case PNSO_SVC_TYPE_DECRYPT:
		case PNSO_SVC_TYPE_COMPRESS:
		case PNSO_SVC_TYPE_DECOMPRESS:
		case PNSO_SVC_TYPE_DECOMPACT:
			OSAL_LOG_INFO("%30s: %d", "data_len",
					res->svc[i].u.dst.data_len);
			pbuf_pprint_buffer_list(res->svc[i].u.dst.sgl);
			break;
		case PNSO_SVC_TYPE_HASH:
			OSAL_LOG_INFO("%30s: %d", "num_tags",
					res->svc[i].u.hash.num_tags);
			OSAL_LOG_INFO("%30s: %d", "rsvd_2",
					res->svc[i].u.hash.rsvd_2);
			/* TODO: print SHA/tags */
			break;
		case PNSO_SVC_TYPE_CHKSUM:
			OSAL_LOG_INFO("%30s: %d", "num_tags",
					res->svc[i].u.chksum.num_tags);
			OSAL_LOG_INFO("%30s: %d", "rsvd_3",
					res->svc[i].u.chksum.rsvd_3);
			/* TODO: print chksum/tags */
			break;
		default:
			OSAL_ASSERT(0);
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
		OSAL_LOG_ERROR("invalid service! svc: 0x%llx err: %d", (u64) svc, err);
		OSAL_ASSERT(0);
		goto out;
	}

	if (svc->svc_type != PNSO_SVC_TYPE_COMPRESS) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid service type specified! svc: 0x%llx svc_type: %d err: %d",
				(u64) svc, svc->svc_type, err);
		OSAL_ASSERT(0);
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
		OSAL_LOG_ERROR("invalid service! svc: 0x%llx %d", (u64) svc, err);
		OSAL_ASSERT(0);
		goto out;
	}

	if (svc->svc_type != PNSO_SVC_TYPE_DECOMPRESS) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid service type specified! svc: 0x%llx svc_type: %d err: %d",
				(u64) svc, svc->svc_type, err);
		OSAL_ASSERT(0);
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
		OSAL_LOG_ERROR("invalid service! svc: 0x%llx err: %d", (u64) svc, err);
		OSAL_ASSERT(0);
		goto out;
	}

	if (svc->svc_type != PNSO_SVC_TYPE_HASH) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid service type specified! svc: 0x%llx svc_type: %d err: %d",
				(u64) svc, svc->svc_type, err);
		OSAL_ASSERT(0);
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
		OSAL_LOG_ERROR("invalid service! svc: 0x%llx err: %d", (u64) svc, err);
		OSAL_ASSERT(0);
		goto out;
	}

	if (svc->svc_type != PNSO_SVC_TYPE_CHKSUM) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid service type specified! svc: 0x%llx svc_type: %d err: %d",
				(u64) svc, svc->svc_type, err);
		OSAL_ASSERT(0);
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
		OSAL_ASSERT(0);
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
		OSAL_LOG_ERROR("invalid status! status: 0x%llx err: %d",
				(u64) status, err);
		OSAL_ASSERT(0);
		goto out;
	}

	if (!__is_service_type_valid(status->svc_type)) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid service type specified! status: 0x%llx svc_type: %d err: %d",
				(u64) status, status->svc_type, err);
		OSAL_ASSERT(0);
		goto out;
	}

	err = PNSO_OK;

out:
	return err;
}

static pnso_error_t
__validate_res_dummy_service(struct pnso_service_status *status)
{
	OSAL_ASSERT(0);
	return EINVAL;
}

static pnso_error_t
__validate_res_encryption_service(struct pnso_service_status *status)
{
	OSAL_ASSERT(0);
	return EINVAL;
}

static pnso_error_t
__validate_res_decryption_service(struct pnso_service_status *status)
{
	OSAL_ASSERT(0);
	return EINVAL;
}

static pnso_error_t
__validate_res_compression_service(struct pnso_service_status *status)
{
	pnso_error_t err;

	if (!status->u.dst.sgl) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid dst buffer list specified for compression! sgl: 0x%llx err: %d",
				(u64) status->u.dst.sgl, err);
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
		OSAL_LOG_ERROR("invalid dst buffer list specified for decompression! sgl: 0x%llx err: %d",
				(u64) status->u.dst.sgl, err);
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
		OSAL_LOG_ERROR("invalid number of tags specified for hash! num_ttags: %d err: %d",
				status->u.hash.num_tags, err);
		goto out;
	}

	if (!status->u.hash.tags) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid tags specified for hash! tags: 0x%llx err: %d",
				(u64) status->u.hash.tags, err);
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
		OSAL_LOG_ERROR("invalid number of tags specified for chksum! num_ttags: %d err: %d",
				status->u.chksum.num_tags, err);
		goto out;
	}

	if (!status->u.chksum.tags) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid tags specified for chksum! tags: 0x%llx err: %d",
				(u64) status->u.chksum.tags, err);
		goto out;
	}

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
__validate_res_decompaction_service(struct pnso_service_status *status)
{
	OSAL_ASSERT(0);
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
		OSAL_LOG_ERROR("invalid service result specified! res: 0x%llx err: %d",
				(u64) svc_res, err);
		goto out;
	}

	num_services = svc_res->num_services;
	for (i = 0; i < num_services; i++) {
		svc_type = svc_res->svc[i].svc_type;

		if (svc_type <= PNSO_SVC_TYPE_NONE ||
				svc_type >= PNSO_SVC_TYPE_MAX) {
			err = EINVAL;
			OSAL_LOG_ERROR("invalid service type specified! service #: %d svc_type: %d err: %d",
					i+1, svc_type, err);
			OSAL_ASSERT(0);
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

	OSAL_LOG_INFO("start ...");

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
	err = chn_build_chain(svc_req, svc_res, cb, cb_ctx,
			pnso_poll_fn, pnso_poll_ctx);

	OSAL_LOG_INFO("done");

	return err;
}
