/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_CHAIN_H__
#define __PNSO_CHAIN_H__

/*
 * This file contain constants, declarations and functions that are necessary
 * for chaining and processing the services within a request.
 *
 * The design of processing a request within the driver i.e. chaining the
 * individual service-requests, submitting the chained request to the
 * hardware, handling the hardware status update, and propagating the status
 * back to the caller follow some _shades_ of 'facade' and 'chain of
 * responsibility' design patterns.
 *
 * As Pensando driver exports just one interface to let the callers submit one
 * or more service-requests, via pnso_submit_request(), leaving the driver to
 * handle the chaining and completing the service-requests.  Thereby, all the
 * processing complexities are hidden behind this one interface.  The design of
 * chaining described here attempts to decompose the complexities into parts
 * that are easier to conceive, understand, program and maintain.  As a result,
 * this approach also enables to:
 *	- avoid coupling the caller to individual service handlers
 *	- separate the services that are dependent on each other
 *	- assure consistency between services
 *	- add or remove services with minimal impact
 *
 * 'struct service_ops' provides a small set of service-specific operations,
 * which will be invoked by the caller's equest processing thread in various
 * steps.  Processing of a service request in the driver relies on setting up
 * the P4+ programs for chaining and handling the status completion.
 *	The first step of processing the request will be to initialize
 *	every service listed in the request via setup().  Upon initializing,
 *	the services need to be chained, via chain().
 *
 *	Chaining involves walking through the list of services and configuring
 *	them such that current service provides sequencer specific info (struct
 *	sequencer_info/chain_params) to the next service in line.
 *
 *	Once the chain is setup, the chained request will be submitted to the
 *	hardware by ringing the door bell of the first service in the chain,
 *	via ring_db(). Subsequently, the request processing thread will return
 *	to the caller.
 *
 *	In the caller's polling thread, the driver will poll the last services'
 *	status for completion. When the hardware updates the status upon its
 *	completion, the polling thread will bail out from polling, and it will
 *	verify the hardware provided status for success/failure.  The thread
 *	will continue to walk through the list of services and update the
 *	per-service status to the caller.  Before exiting the polling thread,
 *	the chain specific structures will be destroyed.
 *
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "pnso_req.h"
#include "pnso_batch.h"
#include "pnso_chain_params.h"
#include "pnso_init.h"

/* service flags */
#define CHAIN_SFLAG_LONE_SERVICE	(1 << 0)
#define CHAIN_SFLAG_FIRST_SERVICE	(1 << 1)
#define CHAIN_SFLAG_LAST_SERVICE	(1 << 2)
#define CHAIN_SFLAG_IN_BATCH		(1 << 3)
#define CHAIN_SFLAG_MODE_SYNC		(1 << 4)
#define CHAIN_SFLAG_MODE_POLL		(1 << 5)
#define CHAIN_SFLAG_MODE_ASYNC		(1 << 6)
#define CHAIN_SFLAG_BYPASS_ONFAIL	(1 << 7)
#define CHAIN_SFLAG_PER_BLOCK		(1 << 8)	/* for hash/chksum */

/* chain flags */
#define CHAIN_CFLAG_MODE_SYNC		(1 << 0)
#define CHAIN_CFLAG_MODE_POLL		(1 << 1)
#define CHAIN_CFLAG_MODE_ASYNC		(1 << 2)
#define CHAIN_CFLAG_RESERVED		(1 << 3) /* rsvd for batch/poll-async */
#define CHAIN_CFLAG_IN_BATCH		(1 << 4)
#define CHAIN_CFLAG_POLLED		(1 << 5)
#define CHAIN_CFLAG_RANG_DB		(1 << 6)

#ifdef NDEBUG
#define PPRINT_CHAIN(c)
#else
#define PPRINT_CHAIN(c)							\
	do {								\
		OSAL_LOG_INFO("%.*s", 30, "=========================================");\
		chn_pprint_chain(c);					\
		OSAL_LOG_INFO("%.*s", 30, "=========================================");\
	} while (0)
#endif

extern struct service_ops cp_ops;
extern struct service_ops dc_ops;
extern struct service_ops hash_ops;
extern struct service_ops chksum_ops;
extern struct service_ops encrypt_ops;
extern struct service_ops decrypt_ops;
extern struct service_ops decompact_ops;

struct service_info;
struct chain_entry;

struct service_params {
	struct pnso_buffer_list *sp_src_blist;
	struct pnso_buffer_list *sp_dst_blist;
	struct pnso_buffer_list *sp_bof_blist;	/* src for bypass onfail */
	union {
		struct pnso_crypto_desc *sp_crypto_desc;
		struct pnso_compression_desc *sp_cp_desc;
		struct pnso_decompression_desc *sp_dc_desc;
		struct pnso_hash_desc *sp_hash_desc;
		struct pnso_checksum_desc *sp_chksum_desc;
		struct pnso_decompaction_desc *sp_decompact_desc;
	} u;
};

struct service_ops {
	/* obtain and initialize op-descriptor, seq and seq queues, etc. */
	pnso_error_t (*setup)(struct service_info *svc_info,
			const struct service_params *svc_params);

	/* chain the service(s) */
	pnso_error_t (*chain)(struct chain_entry *centry);

	pnso_error_t (*sub_chain_from_cpdc)(struct service_info *svc_info,
			struct cpdc_chain_params *cpdc_chain);

	pnso_error_t (*sub_chain_from_crypto)(struct service_info *svc_info,
			struct crypto_chain_params *crypto_chain);

	/* configure the service for async/interrupt handling */
	pnso_error_t (*enable_interrupt)(struct service_info *svc_info,
			void *poll_ctx);
	void (*disable_interrupt)(struct service_info *svc_info);

	/* a NULL-op for all services except the first within the chain */
	pnso_error_t (*ring_db)(struct service_info *svc_info);

	/* a NULL-op for all services except the last within the chain */
	pnso_error_t (*poll)(struct service_info *svc_info);

	/* get hardware status of the service */
	pnso_error_t (*read_status)(struct service_info *svc_info);

	/* copy the status to update the caller */
	pnso_error_t (*write_result)(struct service_info *svc_info);

	/* releases descriptor, etc. to the pool and conducts cleanup task */
	void (*teardown)(struct service_info *svc_info);
};

struct sequencer_info {
	struct queue *sqi_seq_q;
	struct sonic_accel_ring *sqi_ring;	/* CPDC hot/cold, XTS, etc.  */

	uint16_t sqi_qtype;
	uint16_t sqi_status_qtype;
	uint16_t sqi_index;

	uint8_t sqi_batch_mode;
	uint16_t sqi_batch_size;

	void *sqi_desc;			/* sequencer descriptor */
	uint8_t *sqi_status_desc;

	uint32_t sqi_hw_dflt_takes;
	uint32_t sqi_hw_total_takes;
	uint32_t sqi_seq_total_takes;
	uint32_t sqi_status_total_takes;
};

struct service_batch_info {
	uint16_t sbi_num_entries;	/* total # of requests */
	uint16_t sbi_bulk_desc_idx;	/* index within batch info descs */
	uint16_t sbi_desc_idx;	/* index within bulk desc */
	struct batch_info *sbi_batch_info; /* back ptr to batch info if any */
	union {
		struct cpdc_desc *sbi_cpdc_desc;
		struct crypto_desc *sbi_crypto_desc;
	} u;
};

struct service_deps {
	uint32_t sd_data_len;
};

enum service_buf_list_type {
	SERVICE_BUF_LIST_TYPE_HOST,
	SERVICE_BUF_LIST_TYPE_RMEM,
	SERVICE_BUF_LIST_TYPE_DFLT = SERVICE_BUF_LIST_TYPE_HOST
};

struct service_buf_list {
	enum service_buf_list_type type;
	uint32_t                   len;
	struct pnso_buffer_list    *blist;
};

struct interm_buf_list {
	enum mem_pool_type buf_type;
	union {
		struct pnso_buffer_list blist;
		uint8_t b[sizeof(struct pnso_buffer_list) +
			  (sizeof(struct pnso_flat_buffer) *
			   INTERM_BUF_MAX_NUM_NOMINAL_BUFS)];
	};
};

struct service_status_desc {
	void			*desc;
	uint64_t		status_addr;
	uint32_t		elem_size;
	uint32_t		num_elems;
	enum mem_pool_type	mpool_type;
};

struct service_cpdc_sgl {
	struct cpdc_sgl    *sgl;
	enum mem_pool_type mpool_type;
};

struct service_crypto_aol {
	struct crypto_aol  *aol;
	enum mem_pool_type mpool_type;
};

struct service_info {
	uint8_t si_type;
	uint16_t si_flags;		/* service flags (SFLAGS) */

	uint16_t si_block_size;
	uint16_t si_desc_flags;		/* caller supplied desc flags */
	bool tags_updated;
	uint32_t si_num_tags;		/* for tracking # of hash or checksum */

	void *si_desc;			/* desc of cp/dc/encrypt/etc. */
	struct service_status_desc si_status_desc;	/* status desc of cp/dc/encrypt/etc. */
	struct service_status_desc si_istatus_desc;	/* intermediate status desc */
	void *si_bof_desc;		/* bypass onfail desc for hash/chksum */

	struct service_cpdc_sgl	si_src_sgl;	/* src input buffer converted to sgl */
	struct service_cpdc_sgl	si_dst_sgl;	/* dst input buffer converted to sgl */
	struct service_cpdc_sgl	si_bof_sgl;

	struct service_crypto_aol si_src_aol;	/* src input buffer converted to aol */
	struct service_crypto_aol si_dst_aol;	/* dst input buffer converted to aol */

	union {
		struct cpdc_chain_params   si_cpdc_chain;
		struct crypto_chain_params si_crypto_chain;
	};

	struct cpdc_sgl	*si_p4_sgl;	/* for per-block hash/checksum */
	struct cpdc_sgl	*si_p4_bof_sgl;	/* for bypass onfail hash/chksum */

	struct sequencer_info si_seq_info;
	struct service_batch_info si_batch_info;

	struct per_core_resource *si_pcr;	/* to access lif/pool/etc. */
	struct chain_entry *si_centry;	/* back pointer to chain entry */

	struct service_ops si_ops;
	struct pnso_service_status *si_svc_status;

	struct service_buf_list si_src_blist;
	struct service_buf_list si_dst_blist;
	struct service_buf_list si_bof_blist;	/* bypass onfail */
	struct interm_buf_list si_iblist;	/* rmem/hbm buffer */

	struct chain_sgl_pdma *si_sgl_pdma;
	struct service_deps si_svc_deps;	/* to share dependent params */
};

struct chain_entry {
	struct service_chain *ce_chain_head;	/* back pointer to chain head */
	struct chain_entry *ce_next;		/* next service in the chain */
	struct chain_entry *ce_prev;		/* previous service in the chain */
	struct service_info ce_svc_info;	/* service within the chain */
};

struct service_chain {
	uint16_t sc_flags;		/* chain flags (CFLAGS) */
	uint16_t sc_num_services;	/* number of services in the chain */
	uint32_t sc_req_id;		/* unique request id */
	uint64_t sc_submit_ts;		/* submission timestamp */

	struct chain_entry *sc_entry;	/* list of services */
	struct chain_entry *sc_last_entry;	/* last service in chain */

	struct pnso_service_request *sc_req;	/* caller supplied request */
	struct pnso_service_result *sc_res;	/* caller supplied result */

	struct per_core_resource *sc_pcr;	/* to access pool/etc. */
	struct batch_info *sc_batch_info;	/* backpointer to  batch info */

	completion_cb_t	sc_req_cb;	/* caller supplied call-back */
	void *sc_req_cb_ctx;		/* caller supplied cb context */
};

struct service_chain *chn_create_chain(struct request_params *req_params);

pnso_error_t chn_execute_chain(struct service_chain *chain);

void chn_destroy_chain(struct service_chain *chain);

pnso_error_t chn_build_batch_chain(struct batch_info *batch_info,
		struct batch_page_entry *page_entry,
		uint16_t batch_index, const completion_cb_t cb, void *cb_ctx,
		void *pnso_poll_fn, void **pnso_poll_ctx);

struct service_chain *chn_get_first_service_chain(
		struct batch_info *batch_info, uint32_t idx);

struct service_chain *chn_get_last_service_chain(
		struct batch_info *batch_info);

struct chain_entry *chn_get_first_centry(struct service_chain *chain);

struct chain_entry *chn_get_last_centry(struct service_chain *chain);

pnso_error_t chn_poll_all_services(struct service_chain *chain);

void chn_read_write_result(struct service_chain *chain);

void chn_update_overall_result(struct service_chain *chain);

void chn_notify_caller(struct service_chain *chain);

bool chn_is_poll_done(struct service_chain *chain);

pnso_error_t chn_poller(void *poll_ctx);

void chn_pprint_chain(const struct service_chain *chain);

pnso_error_t chn_service_hw_ring_take(struct service_info *svc_info);

pnso_error_t chn_service_hw_ring_give(struct service_info *svc_info);

static inline bool
chn_service_is_in_chain(const struct service_info *svc_info)
{
	return !(svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE);
}

static inline bool
chn_service_is_first(const struct service_info *svc_info)
{
	return !!(svc_info->si_flags & CHAIN_SFLAG_FIRST_SERVICE);
}

static inline bool
chn_service_is_last(const struct service_info *svc_info)
{
	return !!(svc_info->si_flags & CHAIN_SFLAG_LAST_SERVICE);
}

static inline bool
chn_service_is_starter(const struct service_info *svc_info)
{
	return !!(svc_info->si_flags & (CHAIN_SFLAG_LONE_SERVICE |
					CHAIN_SFLAG_FIRST_SERVICE));
}

static inline bool
chn_service_is_mode_async(const struct service_info *svc_info)
{
	return !!(svc_info->si_flags & CHAIN_SFLAG_MODE_ASYNC);
}

static inline bool
chn_service_has_sub_chain(const struct service_info *svc_info)
{
	return chn_service_is_in_chain(svc_info) &&
	       !chn_service_is_last(svc_info);
}

static inline bool
chn_service_has_interm_blist(const struct service_info *svc_info)
{
	return !!svc_info->si_iblist.blist.count;
}

static inline bool
chn_service_has_interm_status(const struct service_info *svc_info)
{
	return !!svc_info->si_istatus_desc.desc;
}

static inline bool
chn_service_type_is_cpdc(const struct service_info *svc_info)
{
	return (svc_info->si_type == PNSO_SVC_TYPE_COMPRESS) ||
	       (svc_info->si_type == PNSO_SVC_TYPE_DECOMPRESS);
}

static inline bool
chn_service_type_is_hashchksum(const struct service_info *svc_info)
{
	return (svc_info->si_type == PNSO_SVC_TYPE_HASH) ||
	       (svc_info->si_type == PNSO_SVC_TYPE_CHKSUM);
}

static inline bool
chn_service_type_is_encrypt(const struct service_info *svc_info)
{
	return svc_info->si_type == PNSO_SVC_TYPE_ENCRYPT;
}

static inline bool
chn_service_type_is_decrypt(const struct service_info *svc_info)
{
	return svc_info->si_type == PNSO_SVC_TYPE_DECRYPT;
}

static inline bool
chn_service_type_is_crypto(const struct service_info *svc_info)
{
	return chn_service_type_is_encrypt(svc_info) ||
	       chn_service_type_is_decrypt(svc_info);
}

static inline void
chn_service_hw_ring_take_set(struct service_info *svc_info,
			     uint32_t count)
{
	svc_info->si_seq_info.sqi_hw_dflt_takes = count;
}

static inline struct service_info *
chn_service_next_svc_get(struct service_info *svc_info)
{
	struct chain_entry *ce_next = svc_info->si_centry->ce_next;

	return ce_next ? &ce_next->ce_svc_info : NULL;
}

static inline struct service_info *
chn_service_prev_svc_get(struct service_info *svc_info)
{
	struct chain_entry *ce_prev = svc_info->si_centry->ce_prev;

	return ce_prev ? &ce_prev->ce_svc_info : NULL;
}

static inline bool
chn_service_is_padding_applic(const struct service_info *svc_info)
{
	return (svc_info->si_type == PNSO_SVC_TYPE_COMPRESS) &&
	       (svc_info->si_desc_flags & PNSO_CP_DFLAG_ZERO_PAD);
}

uint32_t chn_service_deps_data_len_get(struct service_info *svc_info);

static inline void
chn_service_deps_data_len_set(struct service_info *svc_info,
			      uint32_t data_len)
{
	svc_info->si_svc_deps.sd_data_len = data_len;
	if (chn_service_is_padding_applic(svc_info)) {
		OSAL_ASSERT(is_power_of_2(svc_info->si_block_size));
		svc_info->si_svc_deps.sd_data_len =
			REQ_SZ_ROUND_UP_TO_BLK_SZ(data_len,
						  svc_info->si_block_size);
	}
}

static inline bool
chn_service_deps_data_len_set_from_parent(struct service_info *svc_info)
{
	struct service_info	*svc_prev;

	/*
	 * In chaining case, the actual data length may have been determined
	 * in the parent service and propagated down the chain. For example,
	 * if the parent were the CP service, the output compressed data length
	 * plus any padding would have been ccomputed and stored.
	 */
	svc_prev = chn_service_prev_svc_get(svc_info);
	if (svc_prev)
		chn_service_deps_data_len_set(svc_info,
				 chn_service_deps_data_len_get(svc_prev));
	return !!svc_prev;
}

static inline uint32_t
chn_service_deps_num_blks_get(const struct service_info *svc_info)
{
	OSAL_ASSERT(is_power_of_2(svc_info->si_block_size));
	return REQ_SZ_TO_NUM_BLKS(svc_info->si_svc_deps.sd_data_len,
				  svc_info->si_block_size);
}

#ifdef __cplusplus
}
#endif

#endif /* __PNSO_CHAIN_H__ */
