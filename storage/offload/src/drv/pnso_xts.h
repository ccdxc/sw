/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_XTS_H__
#define __PNSO_XTS_H__

/*
 * This file contain constants, declarations and functions that are necessary
 * Pensando's Encryption and Decryption accelerator.
 *
 * Declarations of command descriptors are derived from 'Crypto Co-processor
 * Description v1.22' document and/or from DOL framework.
 *
 */
#include "pnso_api.h"

/**
 * struct xts_init_params - used to initialize accelerator.
 * @xip_qdepth: maximum parallel requests
 *
 */
struct xts_init_params {
	uint16_t xip_queue_depth;
	/* TODO-xts: more might be added here */
};

/**
 * struct xts_aol - represents set of address-offset-length (aol) of data
 * buffers for encryption or decryption operations. Note: address of the
 * buffer must be physical address.
 * @xa_addr_0: specifies the page address of the 1st block of data on which the
 * request will operate on.
 * @xa_off_0: specifies offset within the page of 1st block of data.
 * @xa_len_0: specifies length in bytes.
 * @xa_addr_1: specifies the page address of the 2nd block of data on which the
 * request will operate on.
 * @xa_off_1: specifies offset within the page of 2nd block of data.
 * @xa_len_1: specifies length in bytes.
 * @xa_addr_2: specifies the page address of the 3rd block of data on which the
 * request will operate on.
 * @xa_off_2: specifies offset within the page of 3rd block of data.
 * @xa_len_2: specifies length in bytes.
 * @xa_next: specifies the address of the next descriptor in the chained list
 * of descriptors; a NULL indicates the end of the list.
 * @xa_rsvd: specifies a 'reserved' field for future use.
 *
 */
struct xts_aol {
	uint64_t xa_addr_0;	/* 1st block of data */
	uint32_t xa_off_0;
	uint32_t xa_len_0;
	uint64_t xa_addr_1;	/* 2nd block of data */
	uint32_t xa_off_1;
	uint32_t xa_len_1;
	uint64_t xa_addr_2;	/* 3rd block of data */
	uint32_t xa_off_2;
	uint32_t xa_len_2;
	uint64_t xa_next;	/* next aol */
	uint64_t xa_rsvd;
} __attribute__((__packed__));

/**
 * struct xts_cmd - describes the operations to be performed in encryption and
 * decryption accelerator.
 * @xc_enable_crc: set to 1 to enable CRC block.
 * @xc_bypass_aes: set to 1 to bypass encryption/decryption.
 * @xc_rsvd_1: specifies a 'reserved' field for internal or future use.
 * @xc_is_decrypt: set to 1 for decryption; 0 for encryption.
 * @xc_rsvd_2: specifies a 'reserved' field for internal or future use.
 * @xc_token_3: specifies AES-GCM or AES-XTS depending on the value set in
 * 'xc_token_4'.
 * @xc_token_4: set to 0x3 for AES-GCM or 0x4 for AES-XTS co-processor.
 *
 */
struct xts_cmd {
	uint32_t xc_enable_crc:1;
	uint32_t xc_bypass_aes:1;
	uint32_t xc_rsvd_1:18;
	uint32_t xc_is_decrypt:1;
	uint32_t xc_rsvd_2:3;
	uint32_t xc_token_3:4;
	uint32_t xc_token_4:4;
} __attribute__((__packed__));

/**
 * struct xts_desc - represents the descriptor for encryption and decryption
 * operation.
 * @xd_in_aol: specifies physical address of the first descriptor in the
 * chained-list of descriptors of the input message.
 * @xd_out_aol: specifies physical address of the first descriptor in the
 * chained-list of descriptors of the output message.
 * @xd_cmd: specifies the command identifying the operation to be performed.
 * Refer to 'struct xts_cmd' for more details.
 * @xd_key_desc_idx: specifies the index of the key descriptor within the key
 * descriptor table.
 * @xd_iv_addr: specifies the address of initialization vector.
 * @xd_auth_tag: specifies the address of the authentication tag.
 * @xd_hdr_size: specifies the number of bytes at the beginning of the message
 * that need not be encrypted.
 * @xd_status_addr: specifies the address where the status of the operation will
 * be written by the hardware.  The status indicates if any error occurred or if
 * the authentication tag does not match.
 * @xd_otag_value: specifies a user-supplied value, which is written by hardware
 * at the opaque tag address when the operation completes.
 * @xd_otag_on: set to 1 to allow the hardware write to opaque tag address.
 * @xd_rsvd_1: specifies a 'reserved' field for future use.
 * @xd_sector_size: specifies the size of a sector in bytes. Sector size must be
 * a multiple of 16.
 * @xd_app_tag: specifies the user-supplied application tag.
 * @xd_sector_num: specifies the starting sector number.
 * @xd_db_addr: specifies the address of the door bell.
 * @xd_db_data: specifies the door bell data.
 * @xd_rsvd_2: specifies a 'reserved' field for future use.
 *
 */
struct xts_desc {
	uint64_t xd_in_aol;
	uint64_t xd_out_aol;
	struct xts_cmd xd_cmd;
	uint32_t xd_key_desc_idx;
	uint64_t xd_iv_addr;
	uint64_t xd_auth_tag;
	uint32_t xd_hdr_size;
	uint64_t xd_status_addr;
	uint32_t xd_otag;
	uint32_t xd_otag_on:1;
	uint32_t xd_rsvd_1:31;
	uint16_t xd_sector_size;
	uint16_t xd_app_tag;
	uint32_t xd_sector_num;
	uint64_t xd_db_addr;
	uint64_t xd_db_data;
	char xd_rsvd_2[44];
} __attribute__((__packed__));

/**
 * xts_start_accelerator() - creates and initializes internal data structures
 * that are necessary to encryption and decryption accelerator.
 * @init_params:	[in]	specifies the initialization parameters for
 *				this accelerator.
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
pnso_error_t xts_start_accelerator(const struct xts_init_params *init_params);

/**
 * xts_stop_accelerator() - conducts cleanup task specific to encryption and
 * decryption accelerator.
 *
 * This routine will wait for in-flight operations to complete for a graceful
 * shutdown, or canceling the incomplete operations, and taking care of
 * freeing up the allocated resources.
 *
 * Return Value:
 *	None
 *
 */
void xts_stop_accelerator(void);

#endif /* __PNSO_XTS_H__ */
