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
#include "sonic_dev.h"

#include "pnso_api.h"
#include "pnso_init.h"

#ifdef __cplusplus
extern "C" {
#endif

/* status reported by hardware */
#define CP_STATUS_SUCCESS		0
#define CP_STATUS_AXI_TIMEOUT		1
#define CP_STATUS_AXI_DATA_ERROR	2
#define CP_STATUS_AXI_ADDR_ERROR	3
#define CP_STATUS_COMPRESSION_FAILED	4
#define CP_STATUS_DATA_TOO_LONG		5
#define CP_STATUS_CHECKSUM_FAILED	6
#define CP_STATUS_SGL_DESC_ERROR	7

/* HW to update 'partial_data' in status descriptor */
#define CPDC_CP_STATUS_DATA		1234
#define CPDC_DC_STATUS_DATA		2345
#define CPDC_HASH_STATUS_DATA		3456
#define CPDC_CHKSUM_STATUS_DATA		4567
#define CPDC_PAD_STATUS_DATA		0xFFFF1234FFFF4567ULL

#define MAX_CPDC_SRC_BUF_LEN	(1 << 16)
#define MAX_CPDC_DST_BUF_LEN	MAX_CPDC_SRC_BUF_LEN

#define CPDC_POLL_LOOP_TIMEOUT (1500 * OSAL_NSEC_PER_USEC)

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
 * @cs_next: specifies the address of the next sgl, when set to a valid sgl; a
 * NULL indicates the end of the list.
 * @cs_rsvd_swlink: 'reserved' field used for software linkage
 *
 */
#define CPDC_SGL_TUPLE_LEN_MAX	65536

struct cpdc_sgl {
	uint64_t cs_addr_0;	/* 1st buffer */
	uint32_t cs_len_0;
	uint32_t cs_rsvd_0;
	uint64_t cs_addr_1;	/* 2nd buffer */
	uint32_t cs_len_1;
	uint32_t cs_rsvd_1;
	uint64_t cs_addr_2;	/* 3rd buffer */
	uint32_t cs_len_2;
	uint32_t cs_rsvd_2;
	uint64_t cs_next;	/* next sgl */
	uint64_t cs_rsvd_swlink;
} __attribute__((__packed__));

#define CPDC_SGL_SWLINK_SET(sgl, linkage)	\
	(sgl)->cs_rsvd_swlink = (uint64_t)(linkage)

#define CPDC_SGL_SWLINK_GET(linkage, sgl)	\
	linkage = (typeof(linkage))(sgl)->cs_rsvd_swlink

/**
 * struct cpdc_cmd - describes the operations (along with hints and other
 * constraints) to be performed in CPDC accelerator.
 * @cc_enabled:
 *	- when set to 1 compression or decompression engine is enabled;
 *	'cd_datain_len' in the descriptor should specify length of the input
 *	buffer upto 16 bits.
 *
 *	- when set to 0, dedupe or integrity tag generation is enabled;
 *	compression or decompression operation is bypassed; 'extended_len' in
 *	the descriptor should specify the length up to 32 bits.
 * @cc_header_present: set to 1 for decompression, 0 for compression.
 * @cc_insert_header: set to 1 for compression, 0 for decompression.
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
#define CP_CHKSUM_ADLER32		0
#define CP_CHKSUM_CRC32C		1

#define CP_INTEGRITY_M_CRC64		0
#define CP_INTEGRITY_CRC32C		1
#define CP_INTEGRITY_ADLER32		2
#define CP_INTEGRITY_M_ADLER32		3

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
} __attribute__((__packed__));

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
 * 0, then the length is considered to be 64K.
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
} __attribute__((__packed__));

/**
 * struct cpdc_status_desc - represents the descriptor for status of
 * compression, dedupe hash, pad or decompression operation.
 * @csd_rsvd: specifies a 'reserved' field for future use.
 * @csd_err: specifies the error code of the operation.
 * @csd_valid: specifies if the status can be used for further error
 * classification
 * @csd_output_data_len: specifies length of the processed buffer in bytes.
 * @csd_partial_data: specifies the 'cd_status_data' as supplied in the
 * descriptor.
 * @csd_integrity_data: specifies the integrity/checksum value of the input
 * buffer for the checksum algorithm specified in 'cpdc_desc'.
 * @csd_sha: specifies a 256 or 512-bit SHA of the input buffer for the
 * hash algorithm specified in 'cpdc_desc'.
 *
 */
#define CPDC_STATUS_MIN_CLEAR_SZ	8

struct cpdc_status_desc {
	uint16_t csd_rsvd:12;
	uint16_t csd_err:3;
	uint16_t csd_valid:1;
	uint16_t csd_output_data_len;
	uint32_t csd_partial_data;
	uint64_t csd_integrity_data;
	uint8_t  csd_sha[64];
} __attribute__((__packed__));

/**
 * cpdc_init_accelerator() - creates and/or initializes internal data structures
 * that are necessary to CPDC accelerator.
 * @pc_init:		[in]		specifies the initialization parameters
 *					for this accelerator.
 * @pcr:		[in/out]	specifies the per core container-
 *					structure to cache the accelerators'
 *					internal data structures.
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
pnso_error_t cpdc_init_accelerator(struct pc_res_init_params *pc_init,
		struct per_core_resource *pcr);

/**
 * cpdc_deinit_accelerator() - conducts cleanup task specific to CPDC
 * accelerator.
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
void cpdc_deinit_accelerator(struct per_core_resource *pcr);

#ifdef __cplusplus
}
#endif

#endif /* __PNSO_CPDC_H__ */
