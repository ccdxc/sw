/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_CRYPTO_H__
#define __PNSO_CRYPTO_H__

/*
 * This file contain constants, declarations and functions that are necessary
 * Pensando's Encryption and Decryption accelerator.
 *
 * Declarations of command descriptors are derived from 'Crypto Co-processor
 * Description v1.22' document and/or from DOL framework.
 *
 */
#include "sonic_dev.h"

#include "pnso_api.h"
#include "pnso_init.h"

/**
 * struct crypto_aol - represents set of address-offset-length (aol) of data
 * buffers for encryption or decryption operations. Note: address of the
 * buffer must be physical address.
 * @ca_addr_0: specifies the page address of the 1st block of data on which the
 * request will operate on.
 * @ca_off_0: specifies offset within the page of 1st block of data.
 * @ca_len_0: specifies length in bytes.
 * @ca_addr_1: specifies the page address of the 2nd block of data on which the
 * request will operate on.
 * @ca_off_1: specifies offset within the page of 2nd block of data.
 * @ca_len_1: specifies length in bytes.
 * @ca_addr_2: specifies the page address of the 3rd block of data on which the
 * request will operate on.
 * @ca_off_2: specifies offset within the page of 3rd block of data.
 * @ca_len_2: specifies length in bytes.
 * @ca_next: specifies the address of the next descriptor in the chained list
 * of descriptors; a NULL indicates the end of the list.
 * @ca_rsvd_swlink: 'reserved' field used for software linkage
 *
 */
#define CRYPTO_AOL_TUPLE_LEN_MAX	((1 << 28) - 1)

struct crypto_aol {
	uint64_t ca_addr_0;	/* 1st block of data */
	uint32_t ca_off_0;
	uint32_t ca_len_0;
	uint64_t ca_addr_1;	/* 2nd block of data */
	uint32_t ca_off_1;
	uint32_t ca_len_1;
	uint64_t ca_addr_2;	/* 3rd block of data */
	uint32_t ca_off_2;
	uint32_t ca_len_2;
	uint64_t ca_next;	/* next aol */
	uint64_t ca_rsvd_swlink;
} __attribute__((__packed__));

#define CRYPTO_AOL_SWLINK_SET(aol, linkage)	\
	(aol)->ca_rsvd_swlink = (uint64_t)(linkage)

#define CRYPTO_AOL_SWLINK_GET(linkage, aol)	\
	linkage = (typeof(linkage))(aol)->ca_rsvd_swlink
        
enum crypto_algo_cmd_hi {
	CRYPTO_ALGO_CMD_HI_AES_GCM = 3,
	CRYPTO_ALGO_CMD_HI_AES_XTS = 4,
};

enum crypto_algo_cmd_lo {
	CRYPTO_ALGO_CMD_LO_AES_GCM = 0,
	CRYPTO_ALGO_CMD_LO_AES_XTS = 0,
};

/**
 * struct crypto_cmd - describes the operations to be performed in encryption
 * and decryption accelerator.
 * @cc_enable_crc: set to 1 to enable CRC block.
 * @cc_bypass_aes: set to 1 to bypass encryption/decryption.
 * @cc_rsvd_1: specifies a 'reserved' field for internal or future use.
 * @cc_is_decrypt: set to 1 for decryption; 0 for encryption.
 * @cc_rsvd_2: specifies a 'reserved' field for internal or future use.
 * @cc_token_3: specifies AES-GCM or AES-CRYPTO depending on the value set in
 * 'cc_token_4'.
 * @cc_token_4: set to 0x3 for AES-GCM or 0x4 for AES-CRYPTO co-processor.
 *
 */
struct crypto_cmd {
	uint32_t cc_enable_crc:1;
	uint32_t cc_bypass_aes:1;
	uint32_t cc_rsvd_1:18;
	uint32_t cc_is_decrypt:1;
	uint32_t cc_rsvd_2:3;
	uint32_t cc_token_3:4;
	uint32_t cc_token_4:4;
} __attribute__((__packed__));

/**
 * struct crypto_desc - represents the descriptor for encryption and decryption
 * operation.
 * @cd_in_aol: specifies physical address of the first descriptor in the
 * chained-list of descriptors of the input message.
 * @cd_out_aol: specifies physical address of the first descriptor in the
 * chained-list of descriptors of the output message.
 * @cd_cmd: specifies the command identifying the operation to be performed.
 * Refer to 'struct crypto_cmd' for more details.
 * @cd_key_desc_idx: specifies the index of the key descriptor within the key
 * descriptor table.
 * @cd_iv_addr: specifies the address of initialization vector.
 * @cd_auth_tag: specifies the address of the authentication tag.
 * @cd_hdr_size: specifies the number of bytes at the beginning of the message
 * that need not be encrypted.
 * @cd_status_addr: specifies the address where the status of the operation will
 * be written by the hardware.  The status indicates if any error occurred or if
 * the authentication tag does not match.
 * @cd_otag_value: specifies a user-supplied value, which is written by hardware
 * at the opaque tag address when the operation completes.
 * @cd_otag_on: set to 1 to allow the hardware write to opaque tag address.
 * @cd_rsvd_1: specifies a 'reserved' field for future use.
 * @cd_sector_size: specifies the size of a sector in bytes. Sector size must be
 * a multiple of 16.
 * @cd_app_tag: specifies the user-supplied application tag.
 * @cd_sector_num: specifies the starting sector number.
 * @cd_db_addr: specifies the address of the door bell.
 * @cd_db_data: specifies the door bell data.
 * @cd_rsvd_2: specifies a 'reserved' field for future use.
 *
 */
struct crypto_desc {
	uint64_t cd_in_aol;
	uint64_t cd_out_aol;
	struct crypto_cmd cd_cmd;
	uint32_t cd_key_desc_idx;
	uint64_t cd_iv_addr;
	uint64_t cd_auth_tag;
	uint32_t cd_hdr_size;
	uint64_t cd_status_addr;
	uint32_t cd_otag;
	uint32_t cd_otag_on:1;
	uint32_t cd_rsvd_1:31;
	uint16_t cd_sector_size;
	uint16_t cd_app_tag;
	uint32_t cd_sector_num;
	uint64_t cd_db_addr;
	uint64_t cd_db_data;
	char cd_rsvd_2[44];
} __attribute__((__packed__));

/**
 * struct crypto_status_desc - represents the descriptor for status of crypto.
 * @csd_err: specifies the error code of the operation.
 * @csd_cpl_data: completion data
 *
 */
#define CRYPTO_ENCRYPT_CPL_DATA		0x0102030405060708ull
#define CRYPTO_DECRYPT_CPL_DATA		0x1112131415161718ull

struct crypto_status_desc {
	uint64_t csd_err;
	uint64_t csd_cpl_data;
} __attribute__((__packed__));

/**
 * crypto_init_accelerator() - creates and/or initializes internal data
 * structures that are necessary to encryption and decryption accelerator.
 * @init_params:	[in]	specifies the initialization parameters for
 *				this accelerator.
 * @pcr:		[in]	specifies the per core container-structure to
 *				cache the accelerators' internal data
 *				structures.
 *
 * This MUST be the first function to be invoked prior to exercising any other
 * APIs for this accelerator.
 *
 * Return Value:
 *	PNSO_OK	- on success
 *	-EINVAL	- if invalid depth is specified as input parameter
 *	-ENOMEM	- if failed to allocate memory for accelerators' data
 *		structures
 *	-EPERM	- if accelerator is already initialized
 *
 */
pnso_error_t crypto_init_accelerator(struct pc_res_init_params *pc_init,
		struct per_core_resource *pcr);

/**
 * crypto_deinit_accelerator() - conducts cleanup task specific to encryption
 * and decryption accelerator.
 * @pcr:	[in/out]	specifies the per core container-structure from
 *				which the accelerators' internal data structures
 *				need to be released.
 *
 * This routine will wait for in-flight operations to complete for a graceful
 * shutdown, or canceling the incomplete operations, and taking care of
 * freeing up the allocated resources.
 *
 * Return Value:
 *	None
 *
 */
void crypto_deinit_accelerator(struct per_core_resource *pcr);

pnso_error_t crypto_key_index_update(const void *key1,
				     const void *key2,
				     uint32_t key_size,
				     uint32_t key_idx);
#endif /* __PNSO_CRYPTO_H__ */
