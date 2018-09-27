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

/*
 * TODO-req:
 *	- print SHA/checksum tags in req_pprint_result()
 *	- revisit validate_xxxx() and asserts
 *	- share validate_xxxx() to simulator as needed
 *
 */

#ifdef NDEBUG
#define REQ_PPRINT_REQUEST(r)
#define REQ_PPRINT_RESULT(r)
#else
#define REQ_PPRINT_REQUEST(r)	req_pprint_request(r)
#define REQ_PPRINT_RESULT(r)	req_pprint_result(r)
#endif

typedef pnso_error_t (*validate_req_service_fn_t)(struct pnso_service *svc);

typedef pnso_error_t (*validate_res_service_fn_t)(
		struct pnso_service_status *svc);

static void __attribute__((unused))
pprint_crypto_desc(const struct pnso_crypto_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "=== crypto_desc", (uint64_t) desc);

	OSAL_LOG_INFO("%30s: %d", "algo_type", desc->algo_type);
	OSAL_LOG_INFO("%30s: %d", "rsvd", desc->rsvd);
	OSAL_LOG_INFO("%30s: %d", "key_desc_idx", desc->key_desc_idx);
	OSAL_LOG_INFO("%30s: 0x%llx", "key_desc_idx", desc->iv_addr);
}

static void __attribute__((unused))
pprint_cp_desc(const struct pnso_compression_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "=== cp_desc", (uint64_t) desc);

	OSAL_LOG_INFO("%30s: %d", "algo_type", desc->algo_type);
	OSAL_LOG_INFO("%30s: %d", "flags", desc->flags);
	OSAL_LOG_INFO("%30s: %d", "threshold_len",
			desc->threshold_len);
	OSAL_LOG_INFO("%30s: %d", "hdr_fmt_idx", desc->hdr_fmt_idx);
}

static void __attribute__((unused))
pprint_dc_desc(const struct pnso_decompression_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "=== dc_desc", (uint64_t) desc);

	OSAL_LOG_INFO("%30s: %d", "algo_type", desc->algo_type);
	OSAL_LOG_INFO("%30s: %d", "flags", desc->flags);
	OSAL_LOG_INFO("%30s: %d", "hdr_fmt_idx", desc->hdr_fmt_idx);
	OSAL_LOG_INFO("%30s: %d", "rsvd", desc->rsvd);
}

static void __attribute__((unused))
pprint_hash_desc(const struct pnso_hash_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "=== hash_desc", (uint64_t) desc);

	OSAL_LOG_INFO("%30s: %d", "algo_type", desc->algo_type);
	OSAL_LOG_INFO("%30s: %d", "flags", desc->flags);
}

static void __attribute__((unused))
pprint_chksum_desc(const struct pnso_checksum_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "=== chksum_desc", (uint64_t) desc);

	OSAL_LOG_INFO("%30s: %d", "algo_type", desc->algo_type);
	OSAL_LOG_INFO("%30s: %d", "flags", desc->flags);
}

static void __attribute__((unused))
pprint_decompaction_desc(const struct pnso_decompaction_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "=== decompact_desc", (uint64_t) desc);

	OSAL_LOG_INFO("%30s: %llu", "vvbn", (uint64_t) desc->vvbn);
	OSAL_LOG_INFO("%30s: %d", "rsvd_1", desc->rsvd_1);
	OSAL_LOG_INFO("%30s: %d", "hdr_fmt_idx", desc->hdr_fmt_idx);
	OSAL_LOG_INFO("%30s: %d", "rsvd_2", desc->rsvd_2);
}

void __attribute__((unused))
req_pprint_request(const struct pnso_service_request *req)
{
	uint32_t i;

	if (!req)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "=== pnso_service_request", (uint64_t) req);

	OSAL_LOG_INFO("%30s: %d", "num_services", req->num_services);
	for (i = 0; i < req->num_services; i++) {
		OSAL_LOG_INFO("%30s: %d", "service #", i+1);

		OSAL_LOG_INFO("%30s: %d", "svc_type",
				req->svc[i].svc_type);
		OSAL_LOG_INFO("%30s: %d", "rsvd", req->svc[i].rsvd);

		switch (req->svc[i].svc_type) {
		case PNSO_SVC_TYPE_ENCRYPT:
			pprint_crypto_desc(&req->svc[i].u.crypto_desc);
			break;
		case PNSO_SVC_TYPE_DECRYPT:
			pprint_crypto_desc(&req->svc[i].u.crypto_desc);
			break;
		case PNSO_SVC_TYPE_COMPRESS:
			pprint_cp_desc(&req->svc[i].u.cp_desc);
			break;
		case PNSO_SVC_TYPE_DECOMPRESS:
			pprint_dc_desc(&req->svc[i].u.dc_desc);
			break;
		case PNSO_SVC_TYPE_HASH:
			pprint_hash_desc(&req->svc[i].u.hash_desc);
			break;
		case PNSO_SVC_TYPE_CHKSUM:
			pprint_chksum_desc(&req->svc[i].u.chksum_desc);
			break;
		case PNSO_SVC_TYPE_DECOMPACT:
			pprint_decompaction_desc(
					&req->svc[i].u.decompact_desc);
			break;
		default:
			OSAL_ASSERT(0);
			break;
		}
	}
}

#define MAX_NUM_TAGS	16	/* TODO-cpdc: Move hash/chksum */
static void __attribute__((unused))
pprint_hash_tags(const struct pnso_hash_tag *tags, uint32_t num_tags)
{
	uint32_t i;

	if (!tags || (num_tags > MAX_NUM_TAGS))
		return;

	for (i = 0; i < num_tags; i++)
		OSAL_LOG_INFO("%30s: %*phN", "hash", 64, tags[i].hash);
}

static void __attribute__((unused))
pprint_chksum_tags(const struct pnso_chksum_tag *tags, uint32_t num_tags)
{
	uint32_t i;

	if (!tags || (num_tags > MAX_NUM_TAGS))
		return;

	for (i = 0; i < num_tags; i++)
		OSAL_LOG_INFO("%30s: %*phN", "checksum", 8, tags[i].chksum);
}

void __attribute__((unused))
req_pprint_result(const struct pnso_service_result *res)
{
	uint32_t i;

	if (!res)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "=== pnso_service_result", (uint64_t) res);

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
			break;
		case PNSO_SVC_TYPE_HASH:
			OSAL_LOG_INFO("%30s: %d", "num_tags",
					res->svc[i].u.hash.num_tags);
			pprint_hash_tags(res->svc[i].u.hash.tags,
					res->svc[i].u.hash.num_tags);
			OSAL_LOG_INFO("%30s: %d", "rsvd_2",
					res->svc[i].u.hash.rsvd_2);
			break;
		case PNSO_SVC_TYPE_CHKSUM:
			OSAL_LOG_INFO("%30s: %d", "num_tags",
					res->svc[i].u.chksum.num_tags);
			pprint_chksum_tags(res->svc[i].u.chksum.tags,
					res->svc[i].u.chksum.num_tags);
			OSAL_LOG_INFO("%30s: %d", "rsvd_3",
					res->svc[i].u.chksum.rsvd_3);
			break;
		default:
			OSAL_ASSERT(0);
			break;
		}
	}
}

static pnso_error_t
validate_req_dummy_service(struct pnso_service *svc)
{
	OSAL_ASSERT(0);
	return EINVAL;
}

static pnso_error_t
validate_req_encryption_service(struct pnso_service *svc)
{
	OSAL_ASSERT(0);
	return EINVAL;
}

static pnso_error_t
validate_req_decryption_service(struct pnso_service *svc)
{
	OSAL_ASSERT(0);
	return EINVAL;
}

static pnso_error_t
validate_req_compression_service(struct pnso_service *svc)
{
	pnso_error_t err;

	if (!svc) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid service! svc: 0x%llx err: %d",
				(uint64_t) svc, err);
		OSAL_ASSERT(0);
		goto out;
	}

	if (svc->svc_type != PNSO_SVC_TYPE_COMPRESS) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid service type specified! svc: 0x%llx svc_type: %d err: %d",
				(uint64_t) svc, svc->svc_type, err);
		OSAL_ASSERT(0);
		goto out;
	}

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
validate_req_decompression_service(struct pnso_service *svc)
{
	pnso_error_t err;

	if (!svc) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid service! svc: 0x%llx %d",
				(uint64_t) svc, err);
		OSAL_ASSERT(0);
		goto out;
	}

	if (svc->svc_type != PNSO_SVC_TYPE_DECOMPRESS) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid service type specified! svc: 0x%llx svc_type: %d err: %d",
				(uint64_t) svc, svc->svc_type, err);
		OSAL_ASSERT(0);
		goto out;
	}

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
validate_req_hash_service(struct pnso_service *svc)
{
	pnso_error_t err;

	if (!svc) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid service! svc: 0x%llx err: %d",
				(uint64_t) svc, err);
		OSAL_ASSERT(0);
		goto out;
	}

	if (svc->svc_type != PNSO_SVC_TYPE_HASH) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid service type specified! svc: 0x%llx svc_type: %d err: %d",
				(uint64_t) svc, svc->svc_type, err);
		OSAL_ASSERT(0);
		goto out;
	}

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
validate_req_chksum_service(struct pnso_service *svc)
{
	pnso_error_t err;

	if (!svc) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid service! svc: 0x%llx err: %d",
				(uint64_t) svc, err);
		OSAL_ASSERT(0);
		goto out;
	}

	if (svc->svc_type != PNSO_SVC_TYPE_CHKSUM) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid service type specified! svc: 0x%llx svc_type: %d err: %d",
				(uint64_t) svc, svc->svc_type, err);
		OSAL_ASSERT(0);
		goto out;
	}

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
validate_req_decompaction_service(struct pnso_service *svc)
{
	OSAL_ASSERT(0);
	return EINVAL;
}

static validate_req_service_fn_t validate_req_service_fn[PNSO_SVC_TYPE_MAX] = {
	[PNSO_SVC_TYPE_NONE] = validate_req_dummy_service,
	[PNSO_SVC_TYPE_ENCRYPT] = validate_req_encryption_service,
	[PNSO_SVC_TYPE_DECRYPT] = validate_req_decryption_service,
	[PNSO_SVC_TYPE_COMPRESS] = validate_req_compression_service,
	[PNSO_SVC_TYPE_DECOMPRESS] = validate_req_decompression_service,
	[PNSO_SVC_TYPE_HASH] = validate_req_hash_service,
	[PNSO_SVC_TYPE_CHKSUM] = validate_req_chksum_service,
	[PNSO_SVC_TYPE_DECOMPACT] = validate_req_decompaction_service,
};

static pnso_error_t
validate_res_dummy_service(struct pnso_service_status *status)
{
	OSAL_ASSERT(0);
	return EINVAL;
}

static pnso_error_t
validate_res_encryption_service(struct pnso_service_status *status)
{
	OSAL_ASSERT(0);
	return EINVAL;
}

static pnso_error_t
validate_res_decryption_service(struct pnso_service_status *status)
{
	OSAL_ASSERT(0);
	return EINVAL;
}

static pnso_error_t
validate_res_compression_service(struct pnso_service_status *status)
{
	pnso_error_t err;

	if (!status->u.dst.sgl) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid dst buffer list specified for compression! sgl: 0x%llx err: %d",
				(uint64_t) status->u.dst.sgl, err);
		return err;
	}

	return PNSO_OK;
}

static pnso_error_t
validate_res_decompression_service(struct pnso_service_status *status)
{
	pnso_error_t err;

	if (!status->u.dst.sgl) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid dst buffer list specified for decompression! sgl: 0x%llx err: %d",
				(uint64_t) status->u.dst.sgl, err);
		return err;
	}

	return PNSO_OK;
}

static pnso_error_t
validate_res_hash_service(struct pnso_service_status *status)
{
	pnso_error_t err;

	if (!status->u.hash.tags) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid tags specified for hash! tags: 0x%llx err: %d",
				(uint64_t) status->u.hash.tags, err);
		goto out;
	}

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
validate_res_chksum_service(struct pnso_service_status *status)
{
	pnso_error_t err;

	if (!status->u.chksum.tags) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid tags specified for chksum! tags: 0x%llx err: %d",
				(uint64_t) status->u.chksum.tags, err);
		goto out;
	}

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
validate_res_decompaction_service(struct pnso_service_status *status)
{
	OSAL_ASSERT(0);
	return EINVAL;
}

static validate_res_service_fn_t validate_res_service_fn[PNSO_SVC_TYPE_MAX] = {
	[PNSO_SVC_TYPE_NONE] = validate_res_dummy_service,
	[PNSO_SVC_TYPE_ENCRYPT] = validate_res_encryption_service,
	[PNSO_SVC_TYPE_DECRYPT] = validate_res_decryption_service,
	[PNSO_SVC_TYPE_COMPRESS] = validate_res_compression_service,
	[PNSO_SVC_TYPE_DECOMPRESS] = validate_res_decompression_service,
	[PNSO_SVC_TYPE_HASH] = validate_res_hash_service,
	[PNSO_SVC_TYPE_CHKSUM] = validate_res_chksum_service,
	[PNSO_SVC_TYPE_DECOMPACT] = validate_res_decompaction_service,
};

pnso_error_t
validate_service_request(struct pnso_service_request *svc_req)
{
	pnso_error_t err = EINVAL;
	enum pnso_service_type svc_type;
	uint32_t num_services, i;

	OSAL_LOG_DEBUG("enter ...");

	if (!svc_req) {
		OSAL_LOG_ERROR("invalid service request specified! req: 0x%llx err: %d",
				(uint64_t) svc_req, err);
		goto out;
	}

	if (!svc_req->sgl) {
		OSAL_LOG_ERROR("invalid sgl in service request specified! sgl: 0x%llx err: %d",
				(uint64_t) svc_req->sgl, err);
		goto out;
	}

	num_services = svc_req->num_services;
	if (num_services < 1) {
		OSAL_LOG_ERROR("invalid # of services in req specified! num_services: %d err: %d",
				svc_req->num_services, err);
		goto out;
	}

	/* sgl here means buffer list ... */
	if (svc_req->sgl->count < 1) {
		OSAL_LOG_ERROR("invalid # of buffers in req sgl specified! count: %d err: %d",
				svc_req->sgl->count, err);
		goto out;
	}

	for (i = 0; i < num_services; i++) {
		svc_type = svc_req->svc[i].svc_type;
		if (svc_type <= PNSO_SVC_TYPE_NONE ||
				svc_type >= PNSO_SVC_TYPE_MAX) {
			err = EINVAL;
			OSAL_LOG_ERROR("invalid service type specified! service #: %d svc_type: %d err: %d",
					i+1, svc_type, err);
			OSAL_ASSERT(0);
			goto out;
		}

		err = validate_req_service_fn[svc_type](&svc_req->svc[i]);
		if (err)
			goto out;
	}

	OSAL_LOG_DEBUG("exit!");
	return PNSO_OK;
out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

pnso_error_t
validate_service_result(struct pnso_service_result *svc_res)
{
	pnso_error_t err = EINVAL;
	enum pnso_service_type svc_type;
	uint32_t num_services, i;

	OSAL_LOG_DEBUG("enter ...");

	if (!svc_res) {
		OSAL_LOG_ERROR("invalid service result specified! res: 0x%llx err: %d",
				(uint64_t) svc_res, err);
		goto out;
	}

	num_services = svc_res->num_services;
	if (num_services < 1) {
		OSAL_LOG_ERROR("invalid # of services in res specified! num_services: %d err: %d",
				svc_res->num_services, err);
		goto out;
	}

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
			goto out;
	}

	OSAL_LOG_DEBUG("exit!");
	return PNSO_OK;
out:
	OSAL_LOG_ERROR("exit! err: %d", err);
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

	OSAL_LOG_DEBUG("enter...");

	REQ_PPRINT_REQUEST(svc_req);
	REQ_PPRINT_RESULT(svc_res);

	/* validate each service request */
	err = validate_service_request(svc_req);
	if (err) {
		OSAL_LOG_ERROR("invalid service request specified! err: %d",
				err);
		return err;
	}

	/* validate each service result */
	err = validate_service_result(svc_res);
	if (err) {
		OSAL_LOG_ERROR("invalid service result specified! err: %d",
				err);
		return err;
	}

	if (svc_req->num_services != svc_res->num_services) {
		err = EINVAL;
		OSAL_LOG_ERROR("mismatch in # of services listed in request/result! req: %d res: %d err: %d",
				svc_req->num_services, svc_res->num_services, err);
		return err;
	}

	/* TODO-req:
	 *      sanitize cb, cb_ctx, etc. here and bail out from this layer
	 *      before going deeper into other layers
	 */

	/* build service chain */
	err = chn_build_chain(svc_req, svc_res, cb, cb_ctx,
			pnso_poll_fn, pnso_poll_ctx);

	REQ_PPRINT_RESULT(svc_res);

	OSAL_LOG_DEBUG("exit!");
	return err;
}
