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



#ifdef __cplusplus
}
#endif

#endif /* __PNSO_UTIL_H__ */
