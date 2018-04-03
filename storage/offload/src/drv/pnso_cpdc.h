/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_CPDC_H__
#define __PNSO_CPDC_H__

/*
 * This file contain constants, declarations and functions that are necessary
 * for Pensando's Compression and Decompression (CPDC) accelerator.
 *
 */
#include "pnso_api.h"

#define CPDC_MAX_BUFFER_LEN	(64 * 1024)

/**
 * struct cpdc_init_params - used to initialize CPDC accelerator.
 * @cip_version: to be populated in 'struct psno_compression_header'
 * @cip_qdepth: maximum parallel requests
 *
 */
struct cpdc_init_params {
	uint16_t cip_version;
	uint16_t cip_qdepth;
};

/**
 * struct cpdc_sgl - represents scatter-gather list (sgl) of buffers for
 * compression, dedupe hash, pad and decompression operations. Note: address
 * of the buffer(s) must be physical address(es).
 * @cs_addr_0: specifies the 1st buffer address within the sgl on which the
 * request will operate on.
 * @cs_len_0: specifies length of the 1st buffer in bytes.
 * @cs_rsvd_0: specifies a 'reserved' field for future use.
 * @cs_addr_1: specifies the 2nd buffer address within the sgl on which the
 * request will operate on.
 * @cs_len_1: specifies length of the 2nd buffer in bytes.
 * @cs_rsvd_1: specifies a 'reserved' field for future use.
 * @cs_addr_2: specifies the 3rd buffer address within the sgl on which the
 * request will operate on.
 * @cs_len_2: specifies length of the 3rd buffer in bytes.
 * @cs_rsvd_2: specifies a 'reserved' field for future use.
 * @cs_next: pointer to the next sgl, when set to a valid sgl.
 * @cs_rsvd_3: specifies a 'reserved' field for future use.
 *
 */
struct cpdc_sgl {
	uint64_t cs_addr_0;	/* buffer 1 */
	uint32_t cs_len_0;
	uint32_t cs_rsvd_0;
	uint64_t cs_addr_1;	/* buffer 2 */
	uint32_t cs_len_1;
	uint32_t cs_rsvd_1;
	uint64_t cs_addr_2;	/* buffer 3 */
	uint32_t cs_len_2;
	uint32_t cs_rsvd_2;
	uint64_t cs_next;	/* next sgl */
	uint64_t cs_rsvd_3;
};

/**
 * struct cpdc_cmd - describes the operations (along with hints and other
 * constraints) to be performed in CPDC accelerators.
 * @cc_enabled:
 *	- when set to 1 compression or decompression engine is enabled;
 *	'cd_datain_len' in the descriptor should specify length of the input
 *	buffer upto 16 bits.
 *
 *	- when set to 0, dedupe or integrity tag generation is enabled;
 *	compression or decompression operation is bypassed; 'extended_len' in
 *	the descriptor should specify the length up to 32 bits.
 * @cc_header_present: set to 1 for decompression, 0 for compression.
 * @cc_insert_header: set to 1 for compression, 0 for decompression..
 * @cc_db_on: set to 1 to ring the hardware doorbell.
 * @cc_otag_on: set to 1 to instruct the PCIe interrupt controller that the
 * opaque tag is set.
 * @cc_src_is_list: set to 1 to indicate 'source address' is sgl, 0 to indicate
 * a simple buffer address (i.e. non-sgl).
 * @cc_dst_is_list: set to 1 to indicate 'destination address' is sgl, 0 to
 * indicate a simple buffer address (i.e. non-sgl).
 * @cc_chksum_verify_enabled: set to 1 to verify checksum during decompression,
 * 0 to skip checksum verification.
 * @cc_chksum_adler: set 1 for CRC32, 0 for ADLER32.
 * @cc_hash_enabled: set 1 to append dedupe signature to 'status'.
 * @cc_hash_type: set to 1 for SHA256, 0 for SHA512.
 * @cc_integrity_src: set to 1 for uncompressed data as the source buffer for
 * checksum, 0 for compressed data.
 * @cc_integrity_type: set to one of the following checksum algorithm types
 *	000 - MCRC64
 *	001 - CRC32C
 *	010 - ADLER32
 *	011 - MADLER32
 *	100 to 111 - reserved
 *
 */
struct cpdc_cmd {
	uint16_t cc_enabled:1;
	uint16_t cc_header_present:1;
	uint16_t cc_insert_header:1;
	uint16_t cc_db_on:1;
	uint16_t cc_otag_on:1;
	uint16_t cc_src_is_list:1;
	uint16_t cc_dst_is_list:1;
	uint16_t cc_chksum_verify_enabled:1;
	uint16_t cc_chksum_adler:1;
	uint16_t cc_hash_enabled:1;
	uint16_t cc_hash_type:2;
	uint16_t cc_integrity_src:1;
	uint16_t cc_integrity_type:3;
};

/**
 * struct cpdc_desc - represents the descriptor for compression, dedupe hash,
 * pad and decompression operation(s).
 * @cd_src: physical address of the source buffer or address of 1st source sgl
 * buffer.
 * @cd_dst: physical address of the destination buffer or address of 1st
 * destination sgl buffer.
 * @cd_cmd: 16-bit command descriptor.  Refer to 'struct cpdc_cmd' for more
 * details.
 * @cd_datain_len: total length of the input source buffer in bytes. If set to
 * 0, then the length is considered to be CPDC_MAX_BUFFER_LEN bytes.
 * @cd_extended_len: total length of the input source buffer in bytes, when
 * the requested operation is not compression or decompression.  The length is
 * extended to 32-bits.
 * @cd_threshold_len: expected length of the compressed buffer upon
 * compression operation.
 * @cd_status_addr: byte-aligned address; this is the 1st end-of-DMA. When
 * 'hash_enabled' is set, either 512 or 256 bit SHA is written by the hardware.
 * @cd_db_addr: byte-aligned door bell address; this is the 2nd end-of-DMA.
 * @cd_db_data: byte-aligned door bell data.
 * @cd_otag_addr: byte-aligned opaque tag address; this is the 3rd end-of-DMA.
 * @cd_otag_data: byte-aligned opaque tag data.
 * @cd_status_data: helps to determine when to stop polling the hardware. The
 * hardware will write this data in status descriptor as 'partial_data'.
 *
 */
struct cpdc_desc {
	uint64_t cd_src;
	uint64_t cd_dst;
	union {
		struct cpdc_cmd cd_bits;
		uint16_t cd_cmd;
	} u;
	uint16_t cd_datain_len;
	uint16_t cd_extended_len;
	uint16_t cd_threshold_len;
	uint64_t cd_status_addr;
	uint64_t cd_db_addr;
	uint64_t cd_db_data;
	uint64_t cd_otag_addr;
	uint32_t cd_otag_data;
	uint32_t cd_status_data;
};

/**
 * struct cpdc_status_nosha - represents status of compression, pad or
 * decompression operation.
 * @csn_rsvd: specifies a 'reserved' field for future use.
 * @csn_err: specifies the error code of the operation.
 * @csn_valid: specifies if the status can be used for further error
 * classification
 * @csn_output_data_len: specifies length of the processed buffer in bytes.
 * @csn_partial_data: specifies the 'cd_status_data' as supplied in the
 * descriptor.
 * @csn_integrity_data: specifies the integrity/checksum value of the input
 * buffer for the requested checksum algorithm in the descriptor.
 *
 */
struct cpdc_status_nosha {
	uint16_t csn_rsvd:12;
	uint16_t csn_err:3;
	uint16_t csn_valid:1;
	uint16_t csn_output_data_len;
	uint32_t csn_partial_data;
	uint64_t csn_integrity_data;
};

/**
 * struct cpdc_status_sha512 - represents status of compression, dedupe hash,
 * pad or decompression operation.
 * @cssh512_rsvd: specifies a 'reserved' field for future use.
 * @cssh512_err: specifies the error code of the operation.
 * @cssh512_valid: specifies if the status can be used for further error
 * classification
 * @cssh512_output_data_len: specifies length of the processed buffer in bytes.
 * @cssh512_partial_data: specifies the 'cd_status_data' as supplied in the
 * descriptor.
 * @cssh512_integrity_data: specifies the integrity/checksum value of the input
 * buffer for the requested checksum algorithm in the descriptor.
 * @cssh512_sha: specifies a 512-bit SHA.
 *
 */
struct cpdc_status_sha512 {
	uint16_t css512_rsvd:12;
	uint16_t css512_err:3;
	uint16_t css512_valid:1;
	uint16_t css512_output_data_len;
	uint32_t css512_partial_data;
	uint64_t css512_integrity_data;
	uint8_t  css512_sha[64];
};

/**
 * struct cpdc_status_sha256 - represents status of compression, dedupe hash,
 * pad or decompression operation.
 * @cssh256_rsvd: specifies a 'reserved' field for future use.
 * @cssh256_err: specifies the error code of the operation.
 * @cssh256_valid: specifies if the status can be used for further error
 * classification
 * @cssh256_output_data_len: specifies length of the processed buffer in bytes.
 * @cssh256_partial_data: specifies the 'cd_status_data' as supplied in the
 * descriptor.
 * @cssh256_integrity_data: specifies the integrity/checksum value of the input
 * buffer for the requested checksum algorithm in the descriptor.
 * @cssh256_sha: specifies a 256-bit SHA.
 *
 */
struct cpdc_status_sha256 {
	uint16_t css256_rsvd:12;
	uint16_t css256_err:3;
	uint16_t css256_valid:1;
	uint16_t css256_output_data_len;
	uint32_t css256_partial_data;
	uint64_t css256_integrity_data;
	uint8_t  css256_sha[32];
};

/**
 * cpdc_start_accelerator() - creates and initializes internal data structures
 * that are necessary to CPDC accelerator.
 * @init_params:	[in]	specifies the initialization parameters for
 *				this accelerator.
 *
 * This MUST be the first function to be invoked prior to exercising any other
 * APIs for this accelerator.
 *
 * Return Value:
 *	PNSO_OK	- on success
 *	-EINVAL	- if invalid depth is specified as input param
 *	-ENOMEM	- if failed to allocate memory for accelerators' data
 *		structures
 *	-EPERM	- if accelerator is already initialized
 *
 */
pnso_error_t cpdc_start_accelerator(const struct cpdc_init_params *init_params);

/**
 * cpdc_stop_accelerator() - conducts cleanup task specific to CPDC accelerator.
 *
 * This routine will wait for in-flight operations to complete for a graceful
 * shutdown, or canceling the incomplete operations, and taking care of
 * freeing up the allocated resources.
 *
 * Return Value:
 *	None
 *
 */
void cpdc_stop_accelerator(void);

#endif /* __PNSO_CPDC_H__ */
