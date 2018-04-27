/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_API_H__
#define __PNSO_API_H__

#include <stdint.h>
#include <stdbool.h>

/*
 * WORK_IN_PROGRESS/TODO:
 * 	This header file is in its early stage to initiate discussion
 * 	on the API -- more changes are expected.
 *
 *	Address alignment/packing for cache line size
 *	batching needed or not
 *
 */

/**
 * Pensando offloaders for storage and security
 *
 * Pensando hardware accelerators can boost the storage and security
 * performance.
 *
 * By offloading compute-intensive workloads from the CPU core to Pensando
 * hardware accelerators, performance and efficiency of the applications
 * the applications can significantly be increased.
 *
 * This file contain declarations and functions that are necessary for
 * applications to integrate Pensando hardware acceleration services.
 *
 */

/* Different types of accelerator services */
enum pnso_service_type {
	PNSO_SVC_TYPE_NONE		= 0,
	PNSO_SVC_TYPE_ENCRYPT		= 1,
	PNSO_SVC_TYPE_DECRYPT		= 2,
	PNSO_SVC_TYPE_COMPRESS		= 3,
	PNSO_SVC_TYPE_DECOMPRESS	= 4,
	PNSO_SVC_TYPE_HASH		= 5,
	PNSO_SVC_TYPE_CHKSUM		= 6,
	PNSO_SVC_TYPE_DECOMPACT		= 7,
	PNSO_SVC_TYPE_MAX
};

/**
 * Pensando accelerators can be exercised by submitting one request at a
 * time or a batch of requests.
 *
 * Following constants enable to distinguish the mode of request
 * submission.  When requests are submitted in batch mode, the beginning
 * and ending phase is indicated by start/continue/end markers.
 *
 */
enum pnso_batch_request {
	PNSO_BATCH_REQ_NONE		= 0,
	PNSO_BATCH_REQ_FLUSH		= PNSO_BATCH_REQ_NONE,
	PNSO_BATCH_REQ_CONTINUE		= 1,
	PNSO_BATCH_REQ_MAX
};

/* Algorithms for compression/decompression */
enum pnso_compressor_type {
	PNSO_COMPRESSOR_TYPE_NONE	= 0,
	PNSO_COMPRESSOR_TYPE_LZRW1A	= 1,
	PNSO_COMPRESSOR_TYPE_MAX
};

/* Algorithms for deduplication hash */
enum pnso_hash_type {
	PNSO_HASH_TYPE_NONE		= 0,
	PNSO_HASH_TYPE_SHA2_512		= 1,
	PNSO_HASH_TYPE_SHA2_256		= 2,
	PNSO_HASH_TYPE_MAX
};

/* Algorithms for checksum */
enum pnso_chksum_type {
	PNSO_CHKSUM_TYPE_NONE		= 0,
	PNSO_CHKSUM_TYPE_MCRC64		= 1,
	PNSO_CHKSUM_TYPE_CRC32C		= 2,
	PNSO_CHKSUM_TYPE_ADLER32	= 3,
	PNSO_CHKSUM_TYPE_MADLER32	= 4,
	PNSO_CHKSUM_TYPE_MAX
};

typedef int32_t pnso_error_t;

/* Generic error codes */
#define PNSO_OK 				0

/* Error codes for compression/decompression */
#define PNSO_ERR_CPDC_AXI_TIMEOUT		20001
#define PNSO_ERR_CPDC_AXI_DATA_ERROR		20002
#define PNSO_ERR_CPDC_AXI_ADDR_ERROR		20003
#define PNSO_ERR_CPDC_COMPRESSION_FAILED	20004
#define PNSO_ERR_CPDC_DATA_TOO_LONG		20005
#define PNSO_ERR_CPDC_CHECKSUM_FAILED		20006
#define PNSO_ERR_CPDC_SGL_DESC_ERROR		20007

/* Error codes for encryption/decryption */
#define PNSO_ERR_XTS_KEY_INDEX_OUT_OF_RANG	30001
#define PNSO_ERR_XTS_WRONG_KEY_TYPE		30002
#define PNSO_ERR_XTS_AXI_ERROR			30003
#define PNSO_ERR_XTS_AXI_STATUS_ERROR		30004
#define PNSO_ERR_XTS_AOL_DESC_ERROR		30005

/**
 * struct pnso_flat_buffer - describes a buffer with 'address and length'
 * @buf: contains the physical address of a buffer
 * @rsvd: specifies a 'reserved' field meant to be used by Pensando.
 * @len: contains the length of the buffer in bytes.
 *
 */
struct pnso_flat_buffer {
	uint64_t buf;
	uint32_t rsvd;
	uint32_t len;
};

/**
 * struct pnso_buffer_list - describes a scatter/gather buffer list.
 * @rsvd: specifies a 'reserved' field meant to be used by Pensando.
 * @count: specifies the number of buffers in the list
 * @buffers: specifies an unbounded array of flat buffers as defined by
 * 'count'.
 *
 * This structure is typically used to represent a collection of physical
 * memory buffers that are not contiguous.
 *
 */
struct pnso_buffer_list {
	uint32_t rsvd;
	uint32_t count;
	struct pnso_flat_buffer buffers[0];
};

/**
 * struct pnso_compression_header - represents the result of compression
 * and decompression operation
 * @chksum: specifies the data integrity field, i.e. the checksum
 * calculation on input data before compression.
 * @data_len: specifies the compressed length
 * @version: specifies the version of the compression algorithm 
 *
 * Compression operation will insert a 8-byte header (populating the
 * compressed length, the checksum and the version number) at the beginning
 * of the compressed buffer.
 *
 * Decompression operation will extract the 'checksum' and remove the
 * header. The 'data_len' does not include the length of the header.
 *
 */
struct pnso_compression_header {
	uint32_t chksum;
	uint16_t data_len;
	uint16_t version;
};

/**
 * struct pnso_init_params - represents the initialization parameters for
 * Pensando accelerators
 * @cp_hdr_version: specifies the version of the compression algorithm
 * that to be populated in compression header.
 * @per_core_qdepth: specifies the maximum number of parallel requests per
 * core.
 * @block_size: specifies the size of a block in bytes.
 *
 */
struct pnso_init_params {
	uint16_t cp_hdr_version;
	uint16_t per_core_qdepth;
	uint32_t block_size;
};

/**
 * pnso_init() - initializes Pensando accelerators. Before using any of the
 * Pensando accelerator services, this must be the first function to be
 * invoked.
 * @init_params: specifies the initialization parameters for Pensando
 * Offloaders.
 *
 * Return:
 *	PNSO_OK - on success
 *	-EINVAL - on invalid input parameters
 *
 */
pnso_error_t pnso_init(struct pnso_init_params *init_params);

/**
 * struct pnso_crypto_desc - represents the descriptor for encryption or
 * decryption operation
 * @key_desc_idx: specifies the key index in the descriptor table.
 * @rsvd: specifies a 'reserved' field meant to be used by Pensando.
 * @iv_addr: specifies the physical address of the initialization vector.
 *
 */
struct pnso_crypto_desc {
	uint32_t key_desc_idx;
	uint32_t rsvd;
	uint64_t iv_addr;
};

/**
 * struct pnso_compression_desc - represents the descriptor for compression
 * service 
 * @rsvd_1: specifies a 'reserved' field meant to be used by Pensando.
 * @threshold_len: specifies the expected compressed buffer length in
 * bytes.
 * This is to instruct the compression operation, upon its completion, to
 * compress the buffer to a length that must be less than or equal to
 * 'threshold_len'.
 * @zero_pad: specifies whether or not to zero fill the compressed output
 * buffer aligning to block size.
 * @insert_header: specifies whether or not to insert compression header.
 * @rsvd_2: specifies a 'reserved' field meant to be used by Pensando.
 *
 */
struct pnso_compression_desc {
	uint16_t rsvd_1;
	uint16_t threshold_len;
	bool zero_pad;
	bool insert_header;
	uint32_t rsvd_2;
}; 

/**
 * struct pnso_decompression_desc - represents the descriptor for
 * decompression operation
 *
 */
struct pnso_decompression_desc {
}; 

/**
 * struct pnso_hash_desc - represents the descriptor for data deduplication
 * operation
 * @per_block: specifies whether to produce one hash per block or one for
 * the entire buffer.
 *
 */
struct pnso_hash_desc {
	bool per_block;
};

/**
 * struct pnso_checksum_desc - represents the descriptor for checksum
 * operation
 * @per_block: specifies whether to produce one checksum per block or
 * one for the entire buffer.
 *
 */
struct pnso_checksum_desc {
	bool per_block;
};

/**
 * struct pnso_decompaction_desc - represents the descriptor for
 * decompaction operation
 * @vvbn: specifies the block number within the Netapp's packed block
 * header, with which the offset and length of data can be retrieved.
 *
 */
struct pnso_decompaction_desc {
	uint64_t vvbn:48,
		 rsvd_1:16;
};

/*
 * SHA512 and SHA256 hash need 64 and 32 bytes respectively.  Checksum
 * need a maximum of 4 bytes.
 *
 * Depending on the hash and checksum algorithm, Pensando accelerator can 
 * produce either 64 or 32-byte hash, and 4-byte checksums for every 4KB
 * block.
 *
 * NOTE: Netapp will specify a max of 32KB buffer per request, so will
 * require space for maximum of only 8 hashes.
 *
 */
#define PNSO_HASH_OR_CHKSUM_TAG_LEN	64

/**
 * struct pnso_hash_or_chksum_tag - represents the SHA or checksum tag
 * @hash_or_chksum: specifies an array of either hashes or checksums.
 *
 */
struct pnso_hash_or_chksum_tag {
	uint8_t hash_or_chksum[PNSO_HASH_OR_CHKSUM_TAG_LEN];
};

/**
 * struct pnso_service_status - represents the result of a specific service
 * within a request
 * @err: specifies the error code of a service within the service request.
 * @svc_type: specifies one of the enumerated values for the accelerator
 * service type.
 * @output_data_len: specifies the length of the output buffer processed in
 * bytes depending on the service type.
 * @interim_buf: specifies a temporary scatter/gather buffer list that to
 * be used as output buffer for this service. Valid only for non-last
 * services.
 * @num_tags: specifies number of SHAs or checksums.
 * @tags: specifies a pointer to an allocated memory for number of
 * 'num_tags' hashes or checksums.  When 'num_tags' is 0, this parameter is
 * NULL.
 *
 * Note: Hash or checksum tags will be packed one after another. In other
 * words, consecutive SHA512 and SHA256 hashes will be packed in 64 and
 * 32-bytes apart respectively. Similarly, consecutive checksums will be
 * packed 4-bytes apart.
 *
 */
struct pnso_service_status {
	pnso_error_t err;
	uint8_t svc_type;
	uint32_t output_data_len;
	struct pnso_buffer_list *interim_buf;
	uint16_t num_tags;
	struct pnso_hash_or_chksum_tag *tags;
};

/**
 * struct pnso_service_result - represents the result of the request upon
 * completion of a service within the service request 
 * @err: specifies the error code of the service request.
 * @num_services: specifies the number of services in the request.
 * @svc: specifies an array of service status structures to report the
 * status of each service within a request upon its completion.
 *
 */
struct pnso_service_result {
	pnso_error_t err;
	uint32_t num_services;
	struct pnso_service_status svc[0];
};

/**
 * struct pnso_service - describes various parameters of the service chosen
 * for acceleration
 * @svc_type: specifies one of the enumerated values for the accelerator
 * service type.
 * @algo_type: specifies one of the enumerated values of the compressor or
 * dedupe or chksum algorithm (i.e. enum pnso_compressor_type/
 * pnso_chksum_type/pnso_hash_type) based on the chosen accelerator
 * service type.
 * @crypto_desc: specifies the descriptor for encryption/decryption
 * service.
 * @cp_desc: specifies the descriptor for compression service.
 * @dc_desc: specifies the descriptor for decompression service.
 * @hash_desc: specifies the descriptor for deduplication service.
 * @chksum_desc: specifies the descriptor for checksum service.
 * @decompact_desc: specifies the descriptor for decompaction service.
 *
 */
struct pnso_service {
	uint8_t svc_type;
	uint8_t algo_type;	/* pnso_compressor_type
				 * pnso_hash_type
				 * pnso_chksum_type
				 */
	union {
		struct pnso_crypto_desc crypto_desc;
		struct pnso_compression_desc cp_desc;
		struct pnso_decompression_desc dc_desc;
		struct pnso_hash_desc hash_desc;
		struct pnso_checksum_desc chksum_desc;
		struct pnso_decompaction_desc decompact_desc;
	} d;
};

/**
 * struct pnso_service_request - represents an array of services that are
 * to be handled in a request.
 * @src_buf: specifies input buffer on which the request will operate on.
 * @dst_buf: specifies output buffer on which the request will operate on.
 * @num_services: specifies the number of services in the input service
 * request.
 * @svc: specifies the information about each service within the service
 * request.
 *
 */
struct pnso_service_request {
	struct pnso_buffer_list *src_buf;
	struct pnso_buffer_list *dst_buf;
	uint32_t num_services;
	struct pnso_service svc[0];
};

/**
 * completion_t: caller-supplied completion callback 
 * @cb_ctx: specifies the callback args for the caller-supplied callback
 * routine.
 * @svc_res: specifies a set of service results structures to report the
 * status of each service within a request upon its completion.
 *
 * TODO: discuss further on SPDK-model integration with Netapp.
 *
 */
typedef void (*completion_t)(void *cb_ctx,
		struct pnso_service_result *svc_res);

/**
 * pnso_poll_fn: the caller to use this polling function to detect
 * completion of a request
 * @pnso_poll_ctx: specifies the context passed as arg to the polling
 * function
 *
 */
typedef void (*pnso_poller_fn)(void *pnso_poll_ctx);

/**
 * pnso_submit_request() - routine that accepts one or more service(s) as a
 * request, and batches two or more such requests internally.
 * @batch_req: specifies whether the request is an independent one or
 * belongs to a group of requests.
 * @svc_req: specifies a set of service requests that to be used to
 * complete the services within the request.
 * @svc_res: specifies a set of service results structures to report the
 * status of each service within a request upon its completion. 
 * @cb: specifies the caller-supplied completion callback routine.
 * @cb_ctx: specifies the caller-supplied context information.
 * @pnso_poll_fn: specifies the polling function, which the caller will
 * use to poll for completion of the request.
 * @pnso_poll_ctx: specifies the context for the polling function.
 *
 * Return:
 *	PNSO_OK - on success
 *	-EINVAL - on invalid input parameters
 *	-ENOMEM - on failing to allocate memory
 *
 */
pnso_error_t pnso_submit_request(enum pnso_batch_request batch_req,
		struct pnso_service_request *svc_req,
		struct pnso_service_result *svc_res,
		completion_t cb,
		void *cb_ctx,
		void *pnso_poll_fn,
		void *pnso_poll_ctx);
/**
 * pnso_set_key_desc_idx() - sets the key descriptor index
 * @key1: specifies the key that will be used to encrypt the data.
 * @key2: specifies the key that will be used to encrypt initialization
 * vector.
 * @key_size: specifies the size of the key in bytes -- 16 and 32 bytes for
 * AES128 and AES256 respectively.
 * @key_idx: specifies the key index in the descriptor table.
 *
 * Return:
 *	PNSO_OK - on success
 *	-EINVAL - on invalid input parameters
 *
 */
pnso_error_t pnso_set_key_desc_idx(void *key1,
		void *key2,
		uint32_t key_size,
		uint32_t key_idx);

#endif /* __PNSO_API_H__ */
