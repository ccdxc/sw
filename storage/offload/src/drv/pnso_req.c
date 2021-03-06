/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "pnso_pbuf.h"
#include "pnso_svc.h"
#include "pnso_req.h"
#include "pnso_chain.h"
#include "pnso_batch.h"

#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"
#include "pnso_utils.h"

#ifdef NDEBUG
#define REQ_PPRINT_REQUEST(r)
#define REQ_PPRINT_RESULT(r)
#else
#define REQ_PPRINT_REQUEST(r)	req_pprint_request(r)
#define REQ_PPRINT_RESULT(r)	req_pprint_result(r)
#endif

typedef pnso_error_t (*validate_req_service_fn_t)(struct pnso_service *svc);

typedef pnso_error_t (*validate_res_service_fn_t)(
		struct pnso_service_status *svc, uint32_t num_services);

static void __attribute__((unused))
pprint_crypto_desc(const struct pnso_crypto_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== crypto_desc", (uint64_t) desc);

	OSAL_LOG_DEBUG("%30s: %d", "algo_type", desc->algo_type);
	OSAL_LOG_DEBUG("%30s: %d", "rsvd", desc->rsvd);
	OSAL_LOG_DEBUG("%30s: %d", "key_desc_idx", desc->key_desc_idx);
	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "iv_addr", desc->iv_addr);
}

static void __attribute__((unused))
pprint_cp_desc(const struct pnso_compression_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== cp_desc", (uint64_t) desc);

	OSAL_LOG_DEBUG("%30s: %d", "algo_type", desc->algo_type);
	OSAL_LOG_DEBUG("%30s: %d", "flags", desc->flags);
	OSAL_LOG_DEBUG("%30s: %d", "threshold_len",
			desc->threshold_len);
	OSAL_LOG_DEBUG("%30s: %d", "hdr_fmt_idx", desc->hdr_fmt_idx);
}

static void __attribute__((unused))
pprint_dc_desc(const struct pnso_decompression_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== dc_desc", (uint64_t) desc);

	OSAL_LOG_DEBUG("%30s: %d", "algo_type", desc->algo_type);
	OSAL_LOG_DEBUG("%30s: %d", "flags", desc->flags);
	OSAL_LOG_DEBUG("%30s: %d", "hdr_fmt_idx", desc->hdr_fmt_idx);
	OSAL_LOG_DEBUG("%30s: %d", "rsvd", desc->rsvd);
}

static void __attribute__((unused))
pprint_hash_desc(const struct pnso_hash_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== hash_desc", (uint64_t) desc);

	OSAL_LOG_DEBUG("%30s: %d", "algo_type", desc->algo_type);
	OSAL_LOG_DEBUG("%30s: %d", "flags", desc->flags);
}

static void __attribute__((unused))
pprint_chksum_desc(const struct pnso_checksum_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== chksum_desc", (uint64_t) desc);

	OSAL_LOG_DEBUG("%30s: %d", "algo_type", desc->algo_type);
	OSAL_LOG_DEBUG("%30s: %d", "flags", desc->flags);
}

static void __attribute__((unused))
pprint_decompaction_desc(const struct pnso_decompaction_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== decompact_desc",
			(uint64_t) desc);

	OSAL_LOG_DEBUG("%30s: " PRIu64, "vvbn", (uint64_t) desc->vvbn);
	OSAL_LOG_DEBUG("%30s: %d", "rsvd_1", desc->rsvd_1);
	OSAL_LOG_DEBUG("%30s: %d", "hdr_fmt_idx", desc->hdr_fmt_idx);
	OSAL_LOG_DEBUG("%30s: %d", "rsvd_2", desc->rsvd_2);
}

void __attribute__((unused))
req_pprint_request(const struct pnso_service_request *req)
{
	uint32_t i;

	if (!req)
		return;

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== pnso_service_request",
			(uint64_t) req);

	OSAL_LOG_DEBUG("%30s: %d", "num_services", req->num_services);
	for (i = 0; i < req->num_services; i++) {
		OSAL_LOG_DEBUG("%30s: %d", "service #", i+1);

		OSAL_LOG_DEBUG("%30s: %d", "svc_type",
				req->svc[i].svc_type);
		OSAL_LOG_DEBUG("%30s: %d", "rsvd", req->svc[i].rsvd);

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

#define MAX_NUM_TAGS	16
static void __attribute__((unused))
pprint_hash_tags(const struct pnso_hash_tag *tags, uint32_t num_tags)
{
	uint32_t i;

	if (!tags || (num_tags > MAX_NUM_TAGS))
		return;

	for (i = 0; i < num_tags; i++)
		OSAL_LOG_DEBUG("%30s: %*phN", "hash", 64, tags[i].hash);
}

static void __attribute__((unused))
pprint_chksum_tags(const struct pnso_chksum_tag *tags, uint32_t num_tags)
{
	uint32_t i;

	if (!tags || (num_tags > MAX_NUM_TAGS))
		return;

	for (i = 0; i < num_tags; i++)
		OSAL_LOG_DEBUG("%30s: %*phN", "checksum", 8, tags[i].chksum);
}

void __attribute__((unused))
req_pprint_result(const struct pnso_service_result *res)
{
	uint32_t i;

	if (!res)
		return;

	OSAL_LOG_DEBUG("%30s: 0x" PRIx64, "=== pnso_service_result",
			(uint64_t) res);

	OSAL_LOG_DEBUG("%30s: %d", "err", res->err);

	OSAL_LOG_DEBUG("%30s: %d", "num_services", res->num_services);
	for (i = 0; i < res->num_services; i++) {
		OSAL_LOG_DEBUG("%30s: %d", "service #", i+1);

		OSAL_LOG_DEBUG("%30s: %d", "err", res->svc[i].err);
		OSAL_LOG_DEBUG("%30s: %d", "svc_type",
				res->svc[i].svc_type);
		OSAL_LOG_DEBUG("%30s: %d", "rsvd_1",
				res->svc[i].rsvd_1);

		switch (res->svc[i].svc_type) {
		case PNSO_SVC_TYPE_ENCRYPT:
		case PNSO_SVC_TYPE_DECRYPT:
			break;
		case PNSO_SVC_TYPE_COMPRESS:
		case PNSO_SVC_TYPE_DECOMPRESS:
		case PNSO_SVC_TYPE_DECOMPACT:
			OSAL_LOG_DEBUG("%30s: %d", "data_len",
					res->svc[i].u.dst.data_len);
			break;
		case PNSO_SVC_TYPE_HASH:
			OSAL_LOG_DEBUG("%30s: %d", "num_tags",
					res->svc[i].u.hash.num_tags);
			pprint_hash_tags(res->svc[i].u.hash.tags,
					res->svc[i].u.hash.num_tags);
			OSAL_LOG_DEBUG("%30s: %d", "rsvd_2",
					res->svc[i].u.hash.rsvd_2);
			break;
		case PNSO_SVC_TYPE_CHKSUM:
			OSAL_LOG_DEBUG("%30s: %d", "num_tags",
					res->svc[i].u.chksum.num_tags);
			pprint_chksum_tags(res->svc[i].u.chksum.tags,
					res->svc[i].u.chksum.num_tags);
			OSAL_LOG_DEBUG("%30s: %d", "rsvd_3",
					res->svc[i].u.chksum.rsvd_3);
			break;
		default:
			OSAL_ASSERT(0);
			break;
		}
	}
}

static pnso_error_t
validate_req_source_buffer(enum pnso_service_type svc_type,
		struct pnso_buffer_list *sgl)
{
	pnso_error_t err = EINVAL;
	size_t len;

	if (sgl->count < 1) {
		OSAL_LOG_DEBUG("invalid # of buffers in req sgl specified! count: %d err: %d",
				sgl->count, err);
		goto out;
	}
	len = pbuf_get_buffer_list_len(sgl);

	if ((svc_type == PNSO_SVC_TYPE_COMPRESS) &&
			(len >= CPDC_MIN_USER_BUFFER_LEN)) {
		if (!sgl->buffer_0_va) {
			OSAL_LOG_DEBUG("invalid base/flat buffer virtual address specified! sgl: 0x" PRIx64 " err: %d",
					(uint64_t) sgl, err);
			goto out;
		}
	}

	switch (svc_type) {
	case PNSO_SVC_TYPE_ENCRYPT:
		break;
	case PNSO_SVC_TYPE_DECRYPT:
		break;
	case PNSO_SVC_TYPE_COMPRESS:
	case PNSO_SVC_TYPE_DECOMPRESS:
		if (len == 0 || len > MAX_CPDC_DST_BUF_LEN_EX)
			goto out_len;
		break;
	case PNSO_SVC_TYPE_HASH:
	case PNSO_SVC_TYPE_CHKSUM:
		if (len == 0 || len > MAX_CPDC_SRC_BUF_LEN)
			goto out_len;
		break;
	case PNSO_SVC_TYPE_DECOMPACT:
		break;
	default:
		OSAL_ASSERT(0);
		break;
	}

	err = PNSO_OK;
	return err;

out_len:
	OSAL_LOG_ERROR("invalid src buf len specified! svc_type: %d len: %zu err: %d",
					svc_type, len, err);
out:
	return err;
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
	pnso_error_t err = EINVAL;
	struct pnso_crypto_desc *pnso_encrypt_desc;

	if (!svc) {
		OSAL_LOG_DEBUG("invalid service! svc: 0x" PRIx64 " err: %d",
				(uint64_t) svc, err);
		goto out;
	}

	if (svc->svc_type != PNSO_SVC_TYPE_ENCRYPT) {
		OSAL_LOG_DEBUG("invalid service type specified! svc: 0x" PRIx64 " svc_type: %d err: %d",
				(uint64_t) svc, svc->svc_type, err);
		goto out;
	}

	pnso_encrypt_desc = (struct pnso_crypto_desc *) &svc->u.crypto_desc;
	if (!svc_is_crypto_desc_valid(pnso_encrypt_desc)) {
		OSAL_LOG_DEBUG("invalid crypto/encrypt desc specified! err: %d",
				err);
		goto out;
	}

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
validate_req_decryption_service(struct pnso_service *svc)
{
	pnso_error_t err = EINVAL;
	struct pnso_crypto_desc *pnso_decrypt_desc;

	if (!svc) {
		OSAL_LOG_DEBUG("invalid service! svc: 0x" PRIx64 " %d",
				(uint64_t) svc, err);
		goto out;
	}

	if (svc->svc_type != PNSO_SVC_TYPE_DECRYPT) {
		OSAL_LOG_DEBUG("invalid service type specified! svc: 0x" PRIx64 " svc_type: %d err: %d",
				(uint64_t) svc, svc->svc_type, err);
		goto out;
	}

	pnso_decrypt_desc = (struct pnso_crypto_desc *) &svc->u.crypto_desc;
	if (!svc_is_crypto_desc_valid(pnso_decrypt_desc)) {
		OSAL_LOG_DEBUG("invalid crypto/decrypt desc specified! err: %d",
				err);
		goto out;
	}

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
validate_req_compression_service(struct pnso_service *svc)
{
	pnso_error_t err = EINVAL;
	struct pnso_compression_desc *pnso_cp_desc;

	if (!svc) {
		OSAL_LOG_DEBUG("invalid service! svc: 0x" PRIx64 " err: %d",
				(uint64_t) svc, err);
		goto out;
	}

	if (svc->svc_type != PNSO_SVC_TYPE_COMPRESS) {
		OSAL_LOG_DEBUG("invalid service type specified! svc: 0x" PRIx64 " svc_type: %d err: %d",
				(uint64_t) svc, svc->svc_type, err);
		goto out;
	}

	pnso_cp_desc = (struct pnso_compression_desc *) &svc->u.cp_desc;
	err = svc_is_cp_desc_valid(pnso_cp_desc);
	if (err != PNSO_OK) {
		OSAL_LOG_DEBUG("invalid cp desc specified! err: %d", err);
		goto out;
	}

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
validate_req_decompression_service(struct pnso_service *svc)
{
	pnso_error_t err = EINVAL;
	struct pnso_decompression_desc *pnso_dc_desc;

	if (!svc) {
		OSAL_LOG_DEBUG("invalid service! svc: 0x" PRIx64 " %d",
				(uint64_t) svc, err);
		goto out;
	}

	if (svc->svc_type != PNSO_SVC_TYPE_DECOMPRESS) {
		OSAL_LOG_DEBUG("invalid service type specified! svc: 0x" PRIx64 " svc_type: %d err: %d",
				(uint64_t) svc, svc->svc_type, err);
		goto out;
	}

	pnso_dc_desc = (struct pnso_decompression_desc *) &svc->u.dc_desc;
	if (!svc_is_dc_desc_valid(pnso_dc_desc)) {
		OSAL_LOG_DEBUG("invalid dc desc specified! err: %d", err);
		goto out;
	}

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
validate_req_hash_service(struct pnso_service *svc)
{
	pnso_error_t err = EINVAL;
	struct pnso_hash_desc *pnso_hash_desc;

	if (!svc) {
		OSAL_LOG_DEBUG("invalid service! svc: 0x" PRIx64 " err: %d",
				(uint64_t) svc, err);
		goto out;
	}

	if (svc->svc_type != PNSO_SVC_TYPE_HASH) {
		OSAL_LOG_DEBUG("invalid service type specified! svc: 0x" PRIx64 " svc_type: %d err: %d",
				(uint64_t) svc, svc->svc_type, err);
		goto out;
	}

	pnso_hash_desc = (struct pnso_hash_desc *) &svc->u.hash_desc;
	if (!svc_is_hash_desc_valid(pnso_hash_desc)) {
		OSAL_LOG_DEBUG("invalid hash desc specified! err: %d", err);
		goto out;
	}

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
validate_req_chksum_service(struct pnso_service *svc)
{
	pnso_error_t err = EINVAL;
	struct pnso_checksum_desc *pnso_chksum_desc;

	if (!svc) {
		OSAL_LOG_DEBUG("invalid service! svc: 0x" PRIx64 " err: %d",
				(uint64_t) svc, err);
		goto out;
	}

	if (svc->svc_type != PNSO_SVC_TYPE_CHKSUM) {
		OSAL_LOG_DEBUG("invalid service type specified! svc: 0x" PRIx64 " svc_type: %d err: %d",
				(uint64_t) svc, svc->svc_type, err);
		goto out;
	}

	pnso_chksum_desc =
		(struct pnso_checksum_desc *) &svc->u.chksum_desc;
	if (!svc_is_chksum_desc_valid(pnso_chksum_desc)) {
		OSAL_LOG_DEBUG("invalid chksum desc specified! err: %d", err);
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
validate_res_dummy_service(struct pnso_service_status *status,
		uint32_t num_services)
{
	OSAL_ASSERT(0);
	return EINVAL;
}

static pnso_error_t
validate_res_status(struct pnso_service_status *status)
{
	pnso_error_t err = EINVAL;
	struct pnso_buffer_list *sgl;
	size_t len;

	sgl = status->u.dst.sgl;
	if (!sgl) {
		OSAL_LOG_DEBUG("invalid dst buffer list specified! sgl: 0x" PRIx64 " err: %d",
				(uint64_t) sgl, err);
		goto out;
	}

#if 0	/* TODO-req: verify */
	if (((uint64_t) sgl % PNSO_MEM_ALIGN_BUF) != 0) {
		OSAL_LOG_DEBUG("invalid output buffer alignment! sgl: 0x" PRIx64 " err: %d",
				(uint64_t) sgl, err);
		goto out;
	}
#endif

	if (sgl->count < 1) {
		OSAL_LOG_DEBUG("invalid # of buffers in res sgl specified! count: %d err: %d",
				sgl->count, err);
		goto out;
	}

	len = pbuf_get_buffer_list_len(sgl);
	if (((status->svc_type == PNSO_SVC_TYPE_COMPRESS) ||
		(status->svc_type == PNSO_SVC_TYPE_DECOMPRESS)) &&
			((len == 0) || (len > MAX_CPDC_DST_BUF_LEN_EX))) {
		goto out_len;
	} else {
		if (len == 0 || len > MAX_CPDC_DST_BUF_LEN)
			goto out_len;
	}

	err = PNSO_OK;
	return err;
out_len:
	OSAL_LOG_ERROR("invalid src buf len specified! svc_type: %d len: %zu err: %d",
					status->svc_type, len, err);
out:
	return err;
}

static pnso_error_t
validate_res_encryption_service(struct pnso_service_status *status,
		uint32_t num_services)
{
	pnso_error_t err = PNSO_OK;

	/*
	 * Dst buffer list is optional in a service chain and since not all
	 * services uniformly handle it (by supplying interm buffers), it is
	 * privately dealt with here.
	 *
	 */
	if (num_services <= 1)
		err = validate_res_status(status);
	if (err) {
		OSAL_LOG_DEBUG("invalid output params specified for encryption! err: %d",
				err);
		goto out;
	}

out:
	return err;
}

static pnso_error_t
validate_res_decryption_service(struct pnso_service_status *status,
		uint32_t num_services)
{
	pnso_error_t err = PNSO_OK;

	/*
	 * Dst buffer list is optional in a service chain and since not all services
	 * uniformly handle it (by supplying interm buffers), it is privately
	 * dealt with here.
	 */
	if (num_services <= 1)
		err = validate_res_status(status);
	if (err) {
		OSAL_LOG_DEBUG("invalid output params specified for decryption! err: %d",
				err);
		goto out;
	}

out:
	return err;
}

static pnso_error_t
validate_res_compression_service(struct pnso_service_status *status,
		uint32_t num_services)
{
	pnso_error_t err = PNSO_OK;

	/* refer to comments in encryption service validation */
	if (num_services <= 1)
		err = validate_res_status(status);
	if (err) {
		OSAL_LOG_DEBUG("invalid output params specified for compression! err: %d",
				err);
		goto out;
	}

out:
	return err;
}

static pnso_error_t
validate_res_decompression_service(struct pnso_service_status *status,
		uint32_t num_services)
{
	pnso_error_t err = EINVAL;

	err = validate_res_status(status);
	if (err) {
		OSAL_LOG_DEBUG("invalid output params specified for decompression! err: %d",
				err);
		goto out;
	}

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
validate_res_hash_service(struct pnso_service_status *status,
		uint32_t num_services)
{
	pnso_error_t err = EINVAL;

	if (!status->u.hash.tags) {
		OSAL_LOG_DEBUG("invalid tags specified for hash! tags: 0x" PRIx64 " err: %d",
				(uint64_t) status->u.hash.tags, err);
		goto out;
	}

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
validate_res_chksum_service(struct pnso_service_status *status,
		uint32_t num_services)
{
	pnso_error_t err = EINVAL;

	if (!status->u.chksum.tags) {
		OSAL_LOG_DEBUG("invalid tags specified for chksum! tags: 0x" PRIx64 " err: %d",
				(uint64_t) status->u.chksum.tags, err);
		goto out;
	}

	err = PNSO_OK;
out:
	return err;
}

static pnso_error_t
validate_res_decompaction_service(struct pnso_service_status *status,
		uint32_t num_services)
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

static pnso_error_t
validate_service_request(struct pnso_service_request *svc_req)
{
	pnso_error_t err = EINVAL;
	enum pnso_service_type svc_type;
	uint32_t num_services, i;

	OSAL_LOG_DEBUG("enter ...");

	if (!svc_req) {
		OSAL_LOG_DEBUG("invalid service request specified! req: 0x" PRIx64 " err: %d",
				(uint64_t) svc_req, err);
		goto out;
	}

	if (!svc_req->sgl) {
		OSAL_LOG_DEBUG("invalid sgl in service request specified! sgl: 0x" PRIx64 " err: %d",
				(uint64_t) svc_req->sgl, err);
		goto out;
	}

	num_services = svc_req->num_services;
	if (num_services < 1) {
		OSAL_LOG_DEBUG("invalid # of services in req specified! num_services: %d err: %d",
				svc_req->num_services, err);
		goto out;
	}

	for (i = 0; i < num_services; i++) {
		svc_type = svc_req->svc[i].svc_type;
		if (svc_type <= PNSO_SVC_TYPE_NONE ||
				svc_type >= PNSO_SVC_TYPE_MAX) {
			err = EINVAL;
			OSAL_LOG_DEBUG("invalid service type specified! service #: %d svc_type: %d err: %d",
					i+1, svc_type, err);
			goto out;
		}

		err = validate_req_service_fn[svc_type](&svc_req->svc[i]);
		if (err)
			goto out;
	}

	svc_type = svc_req->svc[0].svc_type;
	err = validate_req_source_buffer(svc_type, svc_req->sgl);
	if (err) {
		OSAL_LOG_DEBUG("invalid # of buffers in req sgl specified! count: %d err: %d",
				svc_req->sgl->count, err);
		goto out;
	}

	OSAL_LOG_DEBUG("exit!");
	return PNSO_OK;
out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
validate_service_result(struct pnso_service_result *svc_res)
{
	pnso_error_t err = EINVAL;
	enum pnso_service_type svc_type;
	uint32_t num_services, i;

	OSAL_LOG_DEBUG("enter ...");

	if (!svc_res) {
		OSAL_LOG_DEBUG("invalid service result specified! res: 0x" PRIx64 " err: %d",
				(uint64_t) svc_res, err);
		goto out;
	}

	num_services = svc_res->num_services;
	if (num_services < 1) {
		OSAL_LOG_DEBUG("invalid # of services in res specified! num_services: %d err: %d",
				svc_res->num_services, err);
		goto out;
	}

	for (i = 0; i < num_services; i++) {
		svc_type = svc_res->svc[i].svc_type;

		if (svc_type <= PNSO_SVC_TYPE_NONE ||
				svc_type >= PNSO_SVC_TYPE_MAX) {
			err = EINVAL;
			OSAL_LOG_DEBUG("invalid service type specified! service #: %d svc_type: %d err: %d",
					i+1, svc_type, err);
			OSAL_ASSERT(0);
			goto out;
		}

		err = validate_res_service_fn[svc_type](&svc_res->svc[i],
				svc_res->num_services);
		if (err)
			goto out;

		svc_res->svc[i].err = PNSO_OK;
	}

	OSAL_LOG_DEBUG("exit!");
	return PNSO_OK;
out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static void
init_request_params(uint16_t req_flags,
		struct per_core_resource *pcr,
		struct pnso_service_request *svc_req,
		struct pnso_service_result *svc_res,
		completion_cb_t cb, void *cb_ctx,
		pnso_poll_fn_t *pnso_poll_fn, void **pnso_poll_ctx,
		struct request_params *req_params)
{
	req_params->rp_flags = req_flags;
	req_params->rp_pcr = pcr;

	req_params->rp_svc_req = svc_req;
	req_params->rp_svc_res = svc_res;

	if (!(req_flags & REQUEST_RFLAG_MODE_SYNC)) {
		req_params->rp_cb = cb;
		req_params->rp_cb_ctx = cb_ctx;

		req_params->rp_poll_fn = pnso_poll_fn;
		req_params->rp_poll_ctx = pnso_poll_ctx;
	}
}

static pnso_error_t
get_request_mode(completion_cb_t cb, void *cb_ctx,
		pnso_poll_fn_t *pnso_poll_fn, void **pnso_poll_ctx,
		uint32_t *req_mode)
{
	pnso_error_t err = PNSO_OK;

	if (!cb && !cb_ctx && !pnso_poll_fn && !pnso_poll_ctx)
		*req_mode = REQUEST_RFLAG_MODE_SYNC;
	else if (cb && cb_ctx && !pnso_poll_fn && !pnso_poll_ctx)
		*req_mode = REQUEST_RFLAG_MODE_ASYNC;
	else if (cb && cb_ctx && pnso_poll_fn && pnso_poll_ctx)
		*req_mode = REQUEST_RFLAG_MODE_POLL;
	else {
		err = EINVAL;
		OSAL_LOG_DEBUG("invalid sync/async/poll params! cb: 0x" PRIx64 " cb_ctx: 0x" PRIx64 " pnso_poll_fn: 0x" PRIx64 " pnso_poll_ctx: 0x" PRIx64 " err: %d",
				(uint64_t) cb, (uint64_t) cb_ctx,
				(uint64_t) pnso_poll_fn,
				(uint64_t) pnso_poll_ctx, err);
	}

	return err;
}

static pnso_error_t
submit_chain(struct request_params *req_params)
{
	pnso_error_t err;
	struct per_core_resource *pcr = req_params->rp_pcr;
	struct service_chain *chain = NULL;
	bool is_sync_mode;

	err = chn_create_chain(req_params, &chain);
	if (err) {
		OSAL_LOG_DEBUG("failed to create request/chain! err: %d",
				err);
		goto out;
	}
	PAS_INC_NUM_CHAINS(pcr);

	is_sync_mode = (chain->sc_flags & CHAIN_CFLAG_MODE_SYNC) != 0;
	err = chn_execute_chain(chain);
	if (err) {
		OSAL_LOG_DEBUG("failed to complete request/chain! err: %d",
				err);
		PAS_INC_NUM_CHAIN_FAILURES(pcr);

		if ((err == ETIMEDOUT || err == PNSO_LIF_IO_ERROR) &&
		    pnso_lif_reset_ctl_pending()) {
			OSAL_LOG_DEBUG("skip destroying chain 0x" PRIx64,
				       (uint64_t) chain);
			goto out;
		}
		goto out_chain;
	}

	if (!is_sync_mode) {
		OSAL_LOG_DEBUG("in non-sync mode ... sc_flags: %d",
				chain->sc_flags);
		goto out;
	}

out_chain:
	chn_destroy_chain(chain);
	return err;
out:
	return err;
}

pnso_error_t
pnso_submit_request(struct pnso_service_request *svc_req,
		struct pnso_service_result *svc_res,
		completion_cb_t cb, void *cb_ctx,
		pnso_poll_fn_t *pnso_poll_fn, void **pnso_poll_ctx)
{
	pnso_error_t err;
	struct request_params req_params = { 0 };
	uint32_t req_flags = 0;
	struct per_core_resource *pcr;

	PAS_DECL_SW_PERF();

	OSAL_LOG_DEBUG("enter...");
	PAS_START_SW_PERF(req_params.rp_sw_latency_ts);

	pcr = putil_get_per_core_resource();
	if (!pcr || pnso_lif_reset_ctl_pending()) {
		err = PNSO_LIF_IO_ERROR;
		OSAL_LOG_DEBUG("pnso pending error reset! err: %d", err);
		goto out;
	}

	PAS_INC_NUM_REQUESTS(pcr);

	REQ_PPRINT_REQUEST(svc_req);
	REQ_PPRINT_RESULT(svc_res);

	/* validate each service request */
	err = validate_service_request(svc_req);
	if (err) {
		OSAL_LOG_ERROR("invalid service request specified! err: %d",
				err);
		svc_res->err = err;
		goto out;
	}

	/* validate each service result */
	err = validate_service_result(svc_res);
	if (err) {
		OSAL_LOG_ERROR("invalid service result specified! err: %d",
				err);
		goto out;
	}

	if (svc_req->num_services != svc_res->num_services) {
		err = EINVAL;
		OSAL_LOG_ERROR("mismatch in # of services listed in request/result! req: %d res: %d err: %d",
				svc_req->num_services, svc_res->num_services,
				err);
		goto out;
	}

	err = get_request_mode(cb, cb_ctx, pnso_poll_fn,
			pnso_poll_ctx, &req_flags);
	if (err) {
		OSAL_LOG_ERROR("invalid submit request params! err: %d", err);
		goto out;
	}

	req_flags |= REQUEST_RFLAG_TYPE_CHAIN;
	init_request_params(req_flags, pcr, svc_req, svc_res, cb, cb_ctx,
			pnso_poll_fn, pnso_poll_ctx, &req_params);

	err = submit_chain(&req_params);
	if (err)
		goto out;

	REQ_PPRINT_RESULT(svc_res);

	if (req_flags & REQUEST_RFLAG_MODE_SYNC)
		PAS_END_SW_PERF(pcr);
	putil_put_per_core_resource(pcr);

	OSAL_LOG_DEBUG("exit!");
	return err;
out:
	if (pcr) {
		PAS_INC_NUM_REQUEST_FAILURES(pcr);
		PAS_END_SW_PERF(pcr);
		putil_put_per_core_resource(pcr);
	}

	OSAL_LOG_DEBUG("exit! err: %d", err);

	return err;
}
OSAL_EXPORT_SYMBOL(pnso_submit_request);

pnso_error_t
pnso_add_to_batch(struct pnso_service_request *svc_req,
		struct pnso_service_result *svc_res)
{
	pnso_error_t err;
	struct per_core_resource *pcr;

	OSAL_LOG_DEBUG("enter...");

	pcr = putil_get_per_core_resource();
	if (!pcr || pnso_lif_reset_ctl_pending()) {
		err = PNSO_LIF_IO_ERROR;
		OSAL_LOG_DEBUG("pnso pending error reset! err: %d", err);
		goto out;
	}

	REQ_PPRINT_REQUEST(svc_req);
	REQ_PPRINT_RESULT(svc_res);

	/* validate each service request */
	err = validate_service_request(svc_req);
	if (err) {
		OSAL_LOG_ERROR("invalid service request specified! err: %d",
				err);
		svc_res->err = err;
		goto out;
	}

	/* validate each service result */
	err = validate_service_result(svc_res);
	if (err) {
		OSAL_LOG_ERROR("invalid service result specified! err: %d",
				err);
		goto out;
	}

	if (svc_req->num_services != svc_res->num_services) {
		err = EINVAL;
		OSAL_LOG_ERROR("mismatch in # of services listed in request/result! req: %d res: %d err: %d",
				svc_req->num_services, svc_res->num_services,
				err);
		goto out;
	}

	err = bat_add_to_batch(pcr, svc_req, svc_res);
	if (err)
		goto out;

	putil_put_per_core_resource(pcr);
	OSAL_LOG_DEBUG("exit!");
	return err;

out:
	/* cleanup batch, if it was created for previous requests */
	if (pcr) {
		bat_destroy_batch(pcr);
		putil_put_per_core_resource(pcr);
	}

	OSAL_LOG_DEBUG("exit! err: %d", err);

	return err;
}
OSAL_EXPORT_SYMBOL(pnso_add_to_batch);

pnso_error_t
pnso_flush_batch(completion_cb_t cb, void *cb_ctx, pnso_poll_fn_t *pnso_poll_fn,
		void **pnso_poll_ctx)
{
	pnso_error_t err;
	struct request_params req_params = { 0 };
	uint32_t req_flags = 0;
	struct per_core_resource *pcr;

	PAS_DECL_SW_PERF();

	OSAL_LOG_DEBUG("enter...");
	PAS_START_SW_PERF(req_params.rp_sw_latency_ts);

	pcr = putil_get_per_core_resource();
	if (!pcr || pnso_lif_reset_ctl_pending()) {
		err = PNSO_LIF_IO_ERROR;
		OSAL_LOG_DEBUG("pnso pending error reset! err: %d", err);
		goto out;
	}

	err = get_request_mode(cb, cb_ctx, pnso_poll_fn,
			pnso_poll_ctx, &req_flags);
	if (err) {
		OSAL_LOG_ERROR("invalid flush request params! err: %d", err);
		goto out;
	}

	req_flags |= REQUEST_RFLAG_TYPE_BATCH;
	init_request_params(req_flags, pcr, NULL, NULL, cb, cb_ctx,
			pnso_poll_fn, pnso_poll_ctx, &req_params);

	err = bat_flush_batch(&req_params);
	if (err) {
		OSAL_LOG_DEBUG("flush request failed! err: %d", err);
		if ((err == ETIMEDOUT || err == PNSO_LIF_IO_ERROR) &&
		    pnso_lif_reset_ctl_pending()) {
			/* Let the lif_reset logic do cleanup */
			OSAL_LOG_DEBUG("skip destroying batch for pcr 0x" PRIx64,
				       (uint64_t) pcr);
			bat_clear_batch(pcr);
		}
		goto out;
	}
	PAS_INC_NUM_BATCHES(pcr);

	/* TODO-batch: temp hack to free-up batch in sync mode */
	if (!cb)
		bat_destroy_batch(pcr);

	err = PNSO_OK;
	PAS_END_SW_PERF(pcr);
	putil_put_per_core_resource(pcr);
	OSAL_LOG_DEBUG("exit!");
	return err;

out:
	/* cleanup batch, if it was created for previous requests */
	if (pcr) {
		bat_destroy_batch(pcr);
		PAS_INC_NUM_BATCH_FAILURES(pcr);
		PAS_END_SW_PERF(pcr);
		putil_put_per_core_resource(pcr);
	}

	OSAL_LOG_DEBUG("exit! err: %d", err);
	return err;
}
OSAL_EXPORT_SYMBOL(pnso_flush_batch);

static inline pnso_error_t
req_poller(void *poll_ctx, bool is_expired)
{
	pnso_error_t err = EINVAL;
	struct per_core_resource *pcr;
	void *object;
	uint8_t mpool_type;
	uint16_t gen_id;

	object = poll_ctx_to_req_obj(poll_ctx, &mpool_type, &gen_id, &pcr);
	if (!object) {
		OSAL_LOG_ERROR("invalid poll context! poll_ctx: 0x" PRIx64 " err: %d",
				(uint64_t) poll_ctx, err);
		goto out;
	}

	if (!sonic_try_reserve_per_core_res(pcr)) {
		err = PNSO_LIF_IO_ERROR;
		goto out;
	}

	switch (mpool_type) {
	case MPOOL_TYPE_BATCH_INFO:
		err = bat_poller((struct batch_info *) object, gen_id,
				 is_expired);
		break;
	case MPOOL_TYPE_SERVICE_CHAIN:
		err = chn_poller((struct service_chain *) object, gen_id,
				 is_expired);
		break;
	default:
		OSAL_LOG_ERROR("invalid poll context type! poll_ctx: 0x" PRIx64 " err: %d",
				(uint64_t) poll_ctx, err);
		break;
	}

	sonic_unreserve_per_core_res(pcr);
out:
	OSAL_LOG_DEBUG("exit! err: %d", err);
	return err;
}

pnso_error_t
pnso_request_poller(void *poll_ctx)
{
	return req_poller(poll_ctx, false);
}
OSAL_EXPORT_SYMBOL(pnso_request_poller);

pnso_error_t
pnso_request_poll_timeout(void *poll_ctx)
{
	return req_poller(poll_ctx, true);
}
OSAL_EXPORT_SYMBOL(pnso_request_poll_timeout);
