/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "pnso_svc.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"

const char __attribute__ ((unused)) *pnso_service_types[] = {
	[PNSO_SVC_TYPE_NONE] = "None (invalid)",
	[PNSO_SVC_TYPE_ENCRYPT] = "ENCRYPT",
	[PNSO_SVC_TYPE_DECRYPT] = "DECRYPT",
	[PNSO_SVC_TYPE_COMPRESS] = "COMPRESS",
	[PNSO_SVC_TYPE_DECOMPRESS] = "DECOMPRESS",
	[PNSO_SVC_TYPE_HASH] = "HASH",
	[PNSO_SVC_TYPE_CHKSUM] = "CHECKSUM",
	[PNSO_SVC_TYPE_DECOMPACT] = "DECOMPACT",
	[PNSO_SVC_TYPE_MAX] = "Max (invalid)"
};

bool
svc_type_is_valid(enum pnso_service_type svc_type)
{
	return (svc_type >  PNSO_SVC_TYPE_NONE) &&
		(svc_type < PNSO_SVC_TYPE_MAX);
}

const char *
svc_get_type_str(enum pnso_service_type svc_type)
{
	return svc_type_is_valid(svc_type) ?
		pnso_service_types[svc_type] : "unknown";
}

/* -------------------------------------------------------------------------- */
bool
svc_is_crypto_desc_valid(const struct pnso_crypto_desc *desc)
{
	pnso_error_t err = EINVAL;

	if ((desc->algo_type == PNSO_CRYPTO_TYPE_NONE) ||
	    (desc->algo_type >= PNSO_CRYPTO_TYPE_MAX)) {
		OSAL_LOG_ERROR("invalid algo_type %u specified! err: %d",
				desc->algo_type, err);
		return false;
	}

	if (!desc->iv_addr) {
		OSAL_LOG_ERROR("invalid iv_addr 0x" PRIx64 " specified! err: %d",
				desc->iv_addr, err);
		return false;
	}

	OSAL_LOG_DEBUG("crypto desc is valid algo_type: %hu", desc->algo_type);

	return true;
}

/* -------------------------------------------------------------------------- */
static inline bool
is_cp_algo_type_valid(uint16_t algo_type)
{
	return (algo_type == PNSO_COMPRESSION_TYPE_LZRW1A) ? true : false;
}

static inline bool
is_cp_threshold_len_valid(uint16_t threshold_len)
{
	return (threshold_len > (MAX_CPDC_SRC_BUF_LEN -
			sizeof(struct pnso_compression_header))) ? false : true;
}

static inline bool
is_cp_flags_valid(uint16_t flags)
{
	/* no contracdicting flags to reject the desc, so skip any checks */
	return true;
}

static inline pnso_error_t
is_cp_hdr_idx_valid(const struct pnso_compression_desc *desc)
{
	struct cp_header_format *format;
	if(desc->flags & PNSO_CP_DFLAG_INSERT_HEADER) {
		format = lookup_hdr_format(desc->hdr_fmt_idx, true);
		if (!format)
			return ENOMEM;
		if (format->fmt_idx != desc->hdr_fmt_idx) {
			return PNSO_ERR_CPDC_HDR_IDX_INVALID;
		}
	}
	return PNSO_OK;
}

pnso_error_t
svc_is_cp_desc_valid(const struct pnso_compression_desc *desc)
{
	pnso_error_t err = EINVAL;

	if (!is_cp_algo_type_valid(desc->algo_type)) {
		err = PNSO_ERR_CPDC_ALGO_INVALID;
		OSAL_LOG_ERROR("invalid cp algo type specified! algo_type: %hu err: %d",
				desc->algo_type, err);
		return err;
	}

	if (!is_cp_threshold_len_valid(desc->threshold_len)) {
		OSAL_LOG_ERROR("invalid cp threshold len specified! threshold_len: %hu err: %d",
				desc->threshold_len, err);
		return err;
	}

	if (!is_cp_flags_valid(desc->flags)) {
		OSAL_LOG_ERROR("invalid cp flags specified! flags: %hu err: %d",
				desc->flags, err);
		return err;
	}

	err = is_cp_hdr_idx_valid(desc);
	if (err != PNSO_OK) {
		OSAL_LOG_ERROR("invalid hdr index specified! hdr_idx : %hu err: %d",
				desc->hdr_fmt_idx, err);
		return err;
	}

	OSAL_LOG_DEBUG("compression desc is valid algo_type: %hu threshold_len: %hu flags: %hu",
			desc->algo_type, desc->threshold_len, desc->flags);

	return PNSO_OK;
}

/* -------------------------------------------------------------------------- */
static inline bool
is_dc_algo_type_valid(uint16_t algo_type)
{
	return (algo_type == PNSO_COMPRESSION_TYPE_LZRW1A) ? true : false;
}

static inline bool
is_dc_flags_valid(uint16_t flags)
{
	/* no contracdicting flags to reject the desc, so skip any checks */
	return true;
}

bool
svc_is_dc_desc_valid(const struct pnso_decompression_desc *desc)
{
	pnso_error_t err = EINVAL;

	if (!is_dc_algo_type_valid(desc->algo_type)) {
		OSAL_LOG_ERROR("invalid dc algo type specified! algo_type: %hu err: %d",
				desc->algo_type, err);
		return false;
	}

	if (!is_dc_flags_valid(desc->flags)) {
		OSAL_LOG_ERROR("invalid dc flags specified! flags: %hu err: %d",
				desc->flags, err);
		return false;
	}

	OSAL_LOG_DEBUG("decompression desc is valid algo_type: %hu flags: %hu",
			desc->algo_type, desc->flags);

	return true;
}

/* -------------------------------------------------------------------------- */
static inline bool
is_hash_algo_type_valid(uint16_t algo_type)
{
	switch (algo_type) {
	case PNSO_HASH_TYPE_SHA2_512:
	case PNSO_HASH_TYPE_SHA2_256:
		return true;
	default:
		return false;
	}

	return false;
}

static inline bool
is_hash_flags_valid(uint16_t flags)
{
	/* no contracdicting flags to reject the desc, so skip any checks */
	return true;
}

bool
svc_is_hash_per_block_enabled(uint16_t flags)
{
	return (flags & PNSO_HASH_DFLAG_PER_BLOCK) ? true : false;
}

bool
svc_is_hash_desc_valid(const struct pnso_hash_desc *desc)
{
	pnso_error_t err = EINVAL;

	if (!is_hash_algo_type_valid(desc->algo_type)) {
		OSAL_LOG_ERROR("invalid hash algo type specified! algo_type: %hu err: %d",
				desc->algo_type, err);
		return false;
	}

	if (!is_hash_flags_valid(desc->flags)) {
		OSAL_LOG_ERROR("invalid hash flags specified! flags: %hu err: %d",
				desc->flags, err);
		return false;
	}

	OSAL_LOG_DEBUG("hash desc is valid algo_type: %hu flags: %hu",
			desc->algo_type, desc->flags);

	return true;
}

/* -------------------------------------------------------------------------- */
static inline bool
is_chksum_algo_type_valid(uint16_t algo_type)
{
	switch (algo_type) {
	case PNSO_CHKSUM_TYPE_MCRC64:
	case PNSO_CHKSUM_TYPE_CRC32C:
	case PNSO_CHKSUM_TYPE_ADLER32:
	case PNSO_CHKSUM_TYPE_MADLER32:
		return true;
	default:
		return false;
	}

	return false;
}

static inline bool
is_chksum_flags_valid(uint16_t flags)
{
	/* no contracdicting flags to reject the desc, so skip any checks */
	return true;
}

bool
svc_is_chksum_per_block_enabled(uint16_t flags)
{
	return (flags & PNSO_CHKSUM_DFLAG_PER_BLOCK) ? true : false;
}

bool
svc_is_chksum_desc_valid(const struct pnso_checksum_desc *desc)
{
	pnso_error_t err = EINVAL;

	if (!is_chksum_algo_type_valid(desc->algo_type)) {
		OSAL_LOG_ERROR("invalid chksum algo type specified! algo_type: %hu err: %d",
				desc->algo_type, err);
		return false;
	}

	if (!is_chksum_flags_valid(desc->flags)) {
		OSAL_LOG_ERROR("invalid chksum flags specified! flags: %hu err: %d",
				desc->flags, err);
		return false;
	}

	OSAL_LOG_DEBUG("chksum desc is valid algo_type: %hu flags: %hu",
			desc->algo_type, desc->flags);

	return true;
}
