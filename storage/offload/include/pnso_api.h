/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_API_H__
#define __PNSO_API_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Pensando offloaders for storage and security
 *
 * Pensando hardware accelerators can boost the storage and security
 * performance.
 *
 * By offloading compute-intensive workloads from the CPU core to Pensando
 * hardware accelerators, performance and efficiency of the applications
 * can significantly be increased.
 *
 * This file contain declarations and functions that are necessary for
 * applications to integrate Pensando hardware acceleration services.
 *
 */

/* Different types of accelerator services */
enum pnso_service_type {
	PNSO_SVC_TYPE_NONE = 0,
	PNSO_SVC_TYPE_ENCRYPT = 1,
	PNSO_SVC_TYPE_DECRYPT = 2,
	PNSO_SVC_TYPE_COMPRESS = 3,
	PNSO_SVC_TYPE_DECOMPRESS = 4,
	PNSO_SVC_TYPE_HASH = 5,
	PNSO_SVC_TYPE_CHKSUM = 6,
	PNSO_SVC_TYPE_DECOMPACT = 7,
	PNSO_SVC_TYPE_MAX
};

/* Algorithms for encrption/decryption */
enum pnso_crypto_type {
	PNSO_CRYPTO_TYPE_NONE = 0,
	PNSO_CRYPTO_TYPE_XTS = 1,
	PNSO_CRYPTO_TYPE_GCM = 2,
	PNSO_CRYPTO_TYPE_MAX
};

/* Algorithms for compression/decompression */
enum pnso_compressor_type {
	PNSO_COMPRESSOR_TYPE_NONE = 0,
	PNSO_COMPRESSOR_TYPE_LZRW1A = 1,
	PNSO_COMPRESSOR_TYPE_MAX
};

/* Algorithms for deduplication hash */
enum pnso_hash_type {
	PNSO_HASH_TYPE_NONE = 0,
	PNSO_HASH_TYPE_SHA2_512 = 1,
	PNSO_HASH_TYPE_SHA2_256 = 2,
	PNSO_HASH_TYPE_MAX
};

/* Algorithms for checksum */
enum pnso_chksum_type {
	PNSO_CHKSUM_TYPE_NONE = 0,
	PNSO_CHKSUM_TYPE_MCRC64 = 1,
	PNSO_CHKSUM_TYPE_CRC32C = 2,
	PNSO_CHKSUM_TYPE_ADLER32 = 3,
	PNSO_CHKSUM_TYPE_MADLER32 = 4,
	PNSO_CHKSUM_TYPE_MAX
};

typedef int32_t pnso_error_t;

/* Generic error codes */
#define PNSO_OK				0

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
#define PNSO_ERR_XTS_KEY_NOT_REGISTERED		30006

/* Error codes for hash/chksum */
#define PNSO_ERR_SHA_FAILED			40001

/**
 * struct pnso_flat_buffer - describes a buffer with 'address and length'.
 * @len: contains the length of the buffer in bytes.
 * @buf: contains the physical address of a buffer.
 *
 */
struct pnso_flat_buffer {
	uint32_t len;
	uint64_t buf;
} __packed;

/**
 * struct pnso_buffer_list - describes a scatter/gather buffer list.
 * @count: specifies the number of buffers in the list.
 * @buffers: specifies an unbounded array of flat buffers as defined by 'count'.
 *
 * This structure is typically used to represent a collection of physical
 * memory buffers that are not contiguous.
 *
 */
struct pnso_buffer_list {
	uint32_t count;
	struct pnso_flat_buffer buffers[0];
};

#define PNSO_MAX_HEADER_FIELDS	8

/**
 * enum pnso_header_field_type - defines the source for the compression header
 * fields.
 *	PNSO_HDR_FIELD_TYPE_STATIC - the field is used as input to set a fixed
 *	value in the compression header (ex: version).
 *
 *	PNSO_HDR_FIELD_TYPE_INDATA_CHKSUM - the field is used as input to set
 *	checksum value in the compression header derived from previous service.
 *
 *	PNSO_HDR_FIELD_TYPE_OUTDATA_LENGTH - the field is used as input to set
 *	length of the compressed buffer as the value in the compression header.
 *
 */
enum pnso_header_field_type {
	PNSO_HDR_FIELD_TYPE_NONE = 0,
	PNSO_HDR_FIELD_TYPE_STATIC = 1,
	PNSO_HDR_FIELD_TYPE_INDATA_CHKSUM = 2,
	PNSO_HDR_FIELD_TYPE_OUTDATA_LENGTH = 3,
	PNSO_HDR_FIELD_TYPE_MAX
};

/**
 * struct pnso_header_field - defines the value for each field in the
 * compression header.
 * @type: specifies the source for the header fields. Refer to 'enum
 * pnso_header_field_type' section for more details on the type.
 * @offset: specifies the offset of the value from the beginning of the header.
 * @length: specifies the length of the value.
 * @value: specifies the value.
 *
 */
struct pnso_header_field {
	enum pnso_header_field_type type;
	uint32_t offset;
	uint32_t length;
	uint32_t value;
};

/**
 * struct pnso_compression_header_format - represents the format of the
 * compression header.
 * @num_fields: specifies the number of fields in the bounded array.
 * @fields: specifies an array of fields.
 *
 */
struct pnso_compression_header_format {
	uint32_t num_fields;
	struct pnso_header_field fields[PNSO_MAX_HEADER_FIELDS];
};

/**
 * struct pnso_init_params - represents the initialization parameters for
 * Pensando accelerators.
 * @cp_hdr_version: specifies the version of the compression algorithm that to
 * be populated in compression header.
 * @per_core_qdepth: specifies the maximum number of parallel requests per core.
 * @block_size: specifies the size of a block in bytes.
 * @cp_hdr_fmt: specifies the set of fields for compression header.
 *
 */
struct pnso_init_params {
	uint16_t cp_hdr_version;
	uint16_t per_core_qdepth;
	uint32_t block_size;
	struct pnso_compression_header_format *cp_hdr_fmt;
};

/**
 * pnso_init() - initializes Pensando accelerators. Before using any of the
 * Pensando accelerator services, this must be the first function to be invoked.
 * @init_params:	[in]	specifies the initialization parameters for
 *				Pensando Offloaders.
 *
 * Caller is responsible for allocation and deallocation of memory for input
 * parameters.
 *
 * Return:
 *	PNSO_OK - on success
 *	-EINVAL - on invalid input parameters
 *
 */
pnso_error_t pnso_init(struct pnso_init_params *init_params);

/**
 * struct pnso_crypto_desc - represents the descriptor for encryption or
 * decryption operation.
 * @algo_type: specifies one of the enumerated values of the crypto type
 * (i.e. pnso_crypto_type).
 * @rsvd: specifies a 'reserved' field meant to be used by Pensando.
 * @key_desc_idx: specifies the key index in the descriptor table.
 * @rsvd: specifies a 'reserved' field meant to be used by Pensando.
 * @iv_addr: specifies the physical address of the initialization vector.
 *
 */
struct pnso_crypto_desc {
	uint16_t algo_type;
	uint16_t rsvd;
	uint32_t key_desc_idx;
	uint64_t iv_addr;
};

/* compression descriptor flags */
#define PNSO_CP_DFLAG_ZERO_PAD				(1 << 0)
#define PNSO_CP_DFLAG_INSERT_HEADER			(1 << 1)
#define PNSO_CP_DFLAG_BYPASS_ONFAIL			(1 << 2)

/**
 * struct pnso_compression_desc - represents the descriptor for compression
 * service.
 * @algo_type: specifies one of the enumerated values of the compressor
 * algorithm (i.e. pnso_compressor_type).
 * @flags: specifies the following applicable descriptor flags to compression
 * descriptor.
 *	PNSO_CP_DFLAG_ZERO_PAD - indicates to zero fill the compressed output
 *	buffer aligning to block size.
 *
 *	PNSO_CP_DFLAG_INSERT_HEADER - indicates to insert compression header
 *	defined by the format supplied in 'struct pnso_init_params'.
 *
 *	PNSO_CP_DFLAG_BYPASS_ONFAIL - indicates to use the source buffer as
 *	input buffer to hash and/or checksum, services, when compression
 *	operation fails.  This flag is effective only when compression, hash
 *	and/or checksum operation is requested.
 *
 * @threshold_len: specifies the expected compressed buffer length in bytes.
 * This is to instruct the compression operation, upon its completion, to
 * compress the buffer to a length that must be less than or equal to
 * 'threshold_len'.
 * @rsvd: specifies a 'reserved' field meant to be used by Pensando.
 *
 */
struct pnso_compression_desc {
	uint16_t algo_type;
	uint16_t flags;
	uint16_t threshold_len;
	uint16_t rsvd;
};

/* decompression descriptor flag(s) */
#define PNSO_DC_DFLAG_HEADER_PRESENT		(1 << 0)

/**
 * struct pnso_decompression_desc - represents the descriptor for decompression
 * operation.
 * @algo_type: specifies one of the enumerated values of the compressor
 * algorithm (i.e. pnso_compressor_type) for decompression.
 * @flags: specifies the following applicable descriptor flags to decompression
 * descriptor.
 *	PNSO_DC_DFLAG_HEADER_PRESENT - indicates the compression header is
 *	present.
 * @rsvd: specifies a 'reserved' field meant to be used by Pensando.
 *
 */
struct pnso_decompression_desc {
	uint16_t algo_type;
	uint16_t flags;
};

/* hash descriptor flag(s) */
#define PNSO_HASH_DFLAG_PER_BLOCK		(1 << 0)

/**
 * struct pnso_hash_desc - represents the descriptor for data deduplication hash
 * operation.
 * @algo_type: specifies one of the enumerated values of the hash algorithm
 * (i.e. pnso_hash_type) for data deduplication.
 * @flags: specifies the following applicable descriptor flag(s) to hash
 * descriptor.
 *	PNSO_HASH_DFLAG_PER_BLOCK - indicates to produce one hash per block.
 *	When this flag is not specified, hash for the entire buffer will be
 *	produced.
 *
 */
struct pnso_hash_desc {
	uint16_t algo_type;
	uint16_t flags;
};

/* chksum descriptor flag(s) */
#define PNSO_CHKSUM_DFLAG_PER_BLOCK		(1 << 0)

/**
 * struct pnso_checksum_desc - represents the descriptor for checksum operation.
 * @algo_type: specifies one of the enumerated values of the checksum
 * algorithm (i.e. pnso_chksum_type).
 * @flags: specifies the following applicable descriptor flag(s) to checksum
 * descriptor.
 *	PNSO_CHKSUM_DFLAG_PER_BLOCK - indicates to produce one checksum
 *	per block. When this flag is not specified, checksum for the entire
 *	buffer will be produced.
 *
 */
struct pnso_checksum_desc {
	uint16_t algo_type;
	uint16_t flags;
};

/**
 * struct pnso_decompaction_desc - represents the descriptor for
 * decompaction operation.
 * @vvbn: specifies the block number within the Netapp's packed block header,
 * with which the offset and length of data can be retrieved.
 *
 */
struct pnso_decompaction_desc {
	uint64_t vvbn:48;
	uint64_t rsvd_1:16;
};

/*
 * SHA512 and SHA256 hash need 64 and 32 bytes respectively.  Checksum need a
 * maximum of 4 or 8 bytes.
 *
 * Depending on the hash and checksum algorithm, Pensando accelerator can
 * produce either 64 or 32-byte hash, and 4 or 8-byte checksums for every
 * 4KB block.
 *
 * NOTE: Netapp will specify a max of 32KB buffer per request, so will require
 * space for maximum of only 8 hashes or checksums.
 *
 */
#define PNSO_HASH_TAG_LEN	64
#define PNSO_CHKSUM_TAG_LEN	8

/**
 * struct pnso_hash_tag - represents the SHA hash tag.
 * @hash: specifies a 64 or 32-byte hash.
 *
 */
struct pnso_hash_tag {
	uint8_t hash[PNSO_HASH_TAG_LEN];
};

/**
 * struct pnso_chksum_tag - represents the checksum tag.
 * @chksum: specifies 4 or 8-byte checksum.
 *
 */
struct pnso_chksum_tag {
	uint8_t chksum[PNSO_CHKSUM_TAG_LEN];
};

/**
 * struct pnso_service_status - represents the result of a specific service
 * within a request.
 * @err: specifies the error code of a service within the service request.
 * When 'err' is set to '0', the processing of this service can be considered
 * successful.  Otherwise, this service in the request is failed, and any output
 * data should be discarded.
 * @svc_type: specifies one of the enumerated values for the accelerator service
 * type.
 * @rsvd_1: specifies a 'reserved' field meant to be used by Pensando.
 * @hash: specifies a pointer to an allocated memory for number of
 * hashes as specified in 'num_hashes'.  When 'num_hashes' is 0, this
 * parameter is NULL.
 * @chksum: specifies a pointer to an allocated memory for number of
 * checksums as specified in 'num_chksums'.  When 'num_chksums' is 0, this
 * parameter is NULL.
 * @dst: specifies a sgl that to be used as output buffer for this service.
 * 'data_len' specifies the length of the data within the the sgl.
 *
 * Note: Hash or checksum tags will be packed one after another.
 *
 */
struct pnso_service_status {
	pnso_error_t err;
	uint16_t svc_type;
	uint16_t rsvd_1;
	union {
		struct {
			uint16_t num_tags;
			uint16_t rsvd_2;
			struct pnso_hash_tag *tags;
		} hash;
		struct {
			uint16_t num_tags;
			uint16_t rsvd_3;
			struct pnso_chksum_tag *tags;
		} chksum;
		struct {
			uint32_t data_len;
			struct pnso_buffer_list *sgl;
		} dst;
	} u;
} __attribute__ ((__packed__));

/**
 * struct pnso_service_result - represents the result of the request upon
 * completion one or all services.
 * @err: specifies the overall error code of the request. When set to '0', the
 * request processing can be considered successful.  Otherwise, one of the
 * services in the request is failed, and any output data should be discarded.
 * @rsvd: specifies a 'reserved' field meant to be used by Pensando.
 * @num_services: specifies the number of services in the request.
 * @svc: specifies an array of service status structures to report the status of
 * each service within a request upon its completion.
 *
 * When 'err' is set to '0', the overall request processing can be considered
 * successful.  Otherwise, one of the services in the request is failed, and
 * any output data should be discarded.
 *
 */
struct pnso_service_result {
	pnso_error_t err;
	uint32_t num_services;
	struct pnso_service_status svc[0];
};

/**
 * struct pnso_service - describes various parameters of the service chosen for
 * acceleration.
 * @svc_type: specifies one of the enumerated values for the accelerator service
 * type.
 * @crypto_desc: specifies the descriptor for encryption/decryption service.
 * @cp_desc: specifies the descriptor for compression service.
 * @dc_desc: specifies the descriptor for decompression service.
 * @hash_desc: specifies the descriptor for deduplication service.
 * @chksum_desc: specifies the descriptor for checksum service.
 * @decompact_desc: specifies the descriptor for decompaction service.
 *
 */
struct pnso_service {
	uint16_t svc_type;
	uint16_t rsvd;
	union {
		struct pnso_crypto_desc crypto_desc;
		struct pnso_compression_desc cp_desc;
		struct pnso_decompression_desc dc_desc;
		struct pnso_hash_desc hash_desc;
		struct pnso_checksum_desc chksum_desc;
		struct pnso_decompaction_desc decompact_desc;
	} u;
} __attribute__ ((__packed__));

/**
 * struct pnso_service_request - represents an array of services that are to be
 * handled in a request.
 * @sgl: specifies input buffer list on which the request will operate on.
 * @num_services: specifies the number of services in the input service request.
 * @svc: specifies the information about each service within the service
 * request.
 *
 */
struct pnso_service_request {
	struct pnso_buffer_list *sgl;
	uint32_t num_services;
	struct pnso_service svc[0];
};

/**
 * typedef completion_cb_t: caller-supplied completion callback.
 * @cb_ctx: specifies the callback args for the caller-supplied callback
 * routine.
 * @svc_res: specifies a set of service results structures to report the status
 * of each service within a request upon its completion.
 *
 * TODO: discuss further on SPDK-model/pnso_poll_fn integration with Netapp.
 *
 */
typedef void (*completion_cb_t) (void *cb_ctx,
			      struct pnso_service_result *svc_res);

/**
 * typedef pnso_poll_fn_t: the caller to use this polling function to detect
 * completion of a request.
 * @pnso_poll_ctx:	[in]	specifies the context passed as arg to the
 *				polling function. This context becomes invalid
 *				after exiting from completion callback.
 *
 * Return:
 *	PNSO_OK - on success
 *	-EAGAIN - on request not done
 */
typedef pnso_error_t (*pnso_poll_fn_t) (void *pnso_poll_ctx);

/**
 * pnso_submit_request() - routine that accepts one or more service(s) as a
 * request and submits the request for further processing.
 * @svc_req:		[in]	specifies a set of service requests that to be
 *				used to complete the services within the
 *				request.
 * @svc_res:		[out]	specifies a set of service results structures to
 *				report the status of each service within a
 *				request upon its completion.
 * @cb:			[in]	specifies the caller-supplied completion
 *				callback routine.
 * @cb_ctx:		[in]	specifies the caller-supplied context
 *				information.
 * @pnso_poll_fn:	[in]	specifies the polling function, which the caller
 *				will use to poll for completion of the request.
 * @pnso_poll_ctx:	[in]	specifies the context for the polling function.
 *
 * Caller is responsible for allocation and deallocation of memory for both
 * input and output parameters. Caller should keep the memory intact (ex:
 * svc_req/svc_res) until the Pensando accelerator returns the result via
 * completion callback.
 *
 * Refer to 'pnso_service_result' and 'pnso_service_status' notes above for
 * handling the output data.
 *
 * Return:
 *	PNSO_OK - on success
 *	-EINVAL - on invalid input parameters
 *	-ENOMEM - on failing to allocate memory
 *
 */
pnso_error_t pnso_submit_request(struct pnso_service_request *svc_req,
				 struct pnso_service_result *svc_res,
				 completion_cb_t cb,
				 void *cb_ctx,
				 pnso_poll_fn_t *pnso_poll_fn,
				 void **pnso_poll_ctx);
/**
 * pnso_add_to_batch() - routine that batches multiple requests and defers
 * processing.
 * @svc_req:		[in]	specifies a set of service requests that to be
 *				used to complete the services within the
 *				request.
 * @svc_res:		[out]	specifies a set of service results structures to
 *				report the status of each service within a
 *				request upon its completion.
 *
 * Caller is responsible for allocation and deallocation of memory for both
 * input and output parameters. Caller should keep the memory intact (ex:
 * svc_req/svc_res) until the Pensando accelerator returns the result via
 * completion callback.
 *
 * None of the requests will be processed until the caller triggers a 'post'
 * operation.
 *
 * Even if any one of the request processing fails, the entire batch of requests
 * will be dropped from further processing.
 *
 * Return:
 *	PNSO_OK - on success
 *	-EINVAL - on invalid input parameters
 *	-ENOMEM - on failing to allocate memory
 *
 */
pnso_error_t pnso_add_to_batch(struct pnso_service_request *svc_req,
		struct pnso_service_result *svc_res);

/**
 * pnso_flush_batch() - routine that starts submitting the batched requests for
 * further processing.
 * @cb:			[in]	specifies the caller-supplied completion
 *				callback routine.
 * @cb_ctx:		[in]	specifies the caller-supplied context
 *				information.
 * @pnso_poll_fn:	[in]	specifies the polling function, which the caller
 *				will use to poll for completion of the request.
 * @pnso_poll_ctx:	[in]	specifies the context for the polling function.
 *
 * Refer to 'pnso_service_result' and 'pnso_service_status' notes above for
 * handling the output data.
 *
 * Even if just the processing of flush request fails, the entire batch of
 * requests will be dropped from further processing.
 *
 * Return:
 *	PNSO_OK - on success
 *	-EINVAL - on invalid input parameters
 *
 */
pnso_error_t pnso_flush_batch(completion_cb_t cb,
		void *cb_ctx,
		pnso_poll_fn_t *pnso_poll_fn,
		void **pnso_poll_ctx);

/**
 * pnso_set_key_desc_idx() - sets the key descriptor index.
 * @key1:	[in]	specifies the key that will be used to encrypt the data.
 * @key2:	[in]	specifies the key that will be used to encrypt
 *			initialization vector.
 * @key_size:	[in]	specifies the size of the key in bytes -- 16 and 32
 *			bytes for AES128 and AES256 respectively.
 * @key_idx:	[in]	specifies the key index in the descriptor table.
 *
 * Caller is responsible for allocation and deallocation of memory for input
 * parameters.
 *
 * Return:
 *	PNSO_OK - on success
 *	-EINVAL - on invalid input parameters
 *
 */
pnso_error_t pnso_set_key_desc_idx(const void *key1,
				   const void *key2,
				   uint32_t key_size, uint32_t key_idx);

#ifdef __cplusplus
}
#endif

#endif /* __PNSO_API_H__ */
