/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_UTIL_H__
#define __PNSO_UTIL_H__

#include "osal_stdtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline size_t pnso_get_chksum_algo_size(uint16_t algo_type)
{
	switch (algo_type) {
	case PNSO_CHKSUM_TYPE_MCRC64:
		return sizeof(uint64_t);
	case PNSO_CHKSUM_TYPE_CRC32C:
	case PNSO_CHKSUM_TYPE_ADLER32:
	case PNSO_CHKSUM_TYPE_MADLER32:
		return sizeof(uint32_t);
	default:
		break;
	}
	return 0;
}

static inline size_t pnso_get_hash_algo_size(uint16_t algo_type)
{
	switch (algo_type) {
	case PNSO_HASH_TYPE_SHA2_512:
		return 512/8;
	case PNSO_HASH_TYPE_SHA2_256:
		return 256/8;
	default:
		break;
	}
	return 0;
}

static inline bool pnso_svc_type_is_data(uint16_t svc_type)
{
	return (svc_type != PNSO_SVC_TYPE_HASH && svc_type != PNSO_SVC_TYPE_CHKSUM);
}


#ifdef __cplusplus
}
#endif

#endif /* __PNSO_UTIL_H__ */
