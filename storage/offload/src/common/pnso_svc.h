/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_SVC_H__
#define __PNSO_SVC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* service descriptor validation routines */
bool svc_is_crypto_desc_valid(const struct pnso_crypto_desc *desc);

bool svc_is_cp_desc_valid(const struct pnso_compression_desc *desc);

bool svc_is_dc_desc_valid(const struct pnso_decompression_desc *desc);

bool svc_is_hash_desc_valid(const struct pnso_hash_desc *desc);

bool svc_is_chksum_desc_valid(const struct pnso_checksum_desc *desc);

/* service descriptor flag support outines */
bool svc_is_hash_per_block_enabled(uint16_t flags);

bool svc_is_chksum_per_block_enabled(uint16_t flags);

#ifdef __cplusplus
}
#endif


#endif /* __PNSO_SVC_H__ */
