/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_CHAIN_H__
#define __PNSO_CHAIN_H__

#include "osal.h"
#include "pnso_api.h"

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
 *	via schedule(). Subsequently, the request processing thread will return
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
 *	TODO-chain: defer handling interrupts, batch processing, etc.  for now.
 *
 */

/* service flags */
#define CHAIN_SFLAG_LONE_SERVICE	(1 << 0)
#define CHAIN_SFLAG_FIRST_SERVICE	(1 << 1)
#define CHAIN_SFLAG_LAST_SERVICE	(1 << 2)

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
	struct pnso_flat_buffer *sp_interm_fbuf;
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

	/* a NULL-op for all services except the first within the chain */
	pnso_error_t (*schedule)(const struct service_info *svc_info);

	/* a NULL-op for all services except the last within the chain */
	pnso_error_t (*poll)(const struct service_info *svc_info);

	/* get hardware status of the service */
	pnso_error_t (*read_status)(const struct service_info *svc_info);

	/* copy the status to update the caller */
	pnso_error_t (*write_result)(struct service_info *svc_info);

	/* releases descriptor, etc. to the pool and conducts cleanup task */
	void (*teardown)(const struct service_info *svc_info);
};

struct service_info {
	uint8_t si_type;
	uint8_t	si_flags;
	uint16_t si_block_size;

	void *si_desc;			/* desc of cp/dc/encrypt/etc. */
	void *si_status_desc;		/* status desc of cp/dc/encrypt/etc. */

	struct pnso_flat_buffer *si_interm_fbuf; /* flat buffer in HBM */
	bool si_per_block;		/* hash or chksum per block */

	struct cpdc_sgl	*si_src_sgl;	/* src input buffer converted to sgl */
	struct cpdc_sgl	*si_dst_sgl;	/* dst input buffer converted to sgl */

	struct service_ops si_ops;
	struct pnso_service_status *si_svc_status;
	/* TODO-chain: add sequencer_info, etc. */
};

struct chain_entry {
	struct service_chain *ce_chain_head;	/* back pointer to chain head */
	struct chain_entry *ce_next;		/* next service in the chain */
	struct service_info ce_svc_info;	/* service within the chain */
};

struct service_chain {
	uint32_t sc_req_id;		/* unique request id */
	uint32_t sc_num_services;	/* number of services in the chain */
	struct chain_entry *sc_entry;	/* list of services */
	struct pnso_service_result *sc_res;	/* vendor result */

	completion_cb_t	sc_req_cb;	/* vendor call-back */
	void *sc_req_cb_ctx;		/* vendor cb context */
	pnso_poll_fn_t *sc_req_poll_fn;	/* poller to run in caller's thread */
	void *sc_req_poll_ctx;		/* request context for poller */
};

/**
 * chn_create_chain() - creates a chain structure with the specified list of
 * service chained in order and caches the user supplied parameters.
 * @svc_req:		[in]	specifies a set of service requests that to be
 *				used to complete the services within the
 *				request.
 * @svc_res:		[in]	specifies a set of service results structures to
 *				report the status of each service within the
 *				request upon its completion.
 * @cb:			[in]	specifies the caller-supplied completion
 *				callback routine.
 * @cb_ctx:		[in]	specifies the caller-supplied context
 *				information.
 * @pnso_poll_fn:	[in]	specifies the polling function, which the caller
 *				will use to poll for completion of the request.
 * @pnso_poll_ctx:	[in]	specifies the context for the polling function.
 * @out_chain:		[out]	specifies a chain structure comprising list of
 *				services that are chained in the user specified
 *				order.
 *
 * Return Value:
 *	PNSO_OK	- on success
 *	-ENOMEM - on failing to allocate memory
 *	-EINVAL - on invalid input parameters
 *
 */
pnso_error_t chn_create_chain(struct pnso_service_request *svc_req,
		struct pnso_service_result *svc_res,
		completion_cb_t cb,
		void *cb_ctx,
		void *pnso_poll_fn,
		void *pnso_poll_ctx,
		struct service_chain **out_chain);

/**
 * chn_build_service_chain() - initializes the services within the chain.
 * @chain:	[in]	specifies the chain structure.
 *
 * Return Value:
 *	PNSO_OK	- on success
 *	-ENOMEM - on failing to allocate memory
 *	-EINVAL - on invalid input parameters
 *
 */
pnso_error_t chn_build_service_chain(struct service_chain *chain);

/**
 * chn_execute_chain() - notifies the hardware to process the first service
 * in the chain.
 * @chain:	[in]	specifies the chain structure.
 *
 * Return Value:
 *	None
 *
 */
void chn_execute_chain(struct service_chain *chain);

/**
 * chn_destroy_chain() - destroys the chain structure.
 * @chain:	[in]	specifies the chain structure.
 *
 * Return Value:
 *	None
 *
 */
void chn_destroy_chain(struct service_chain *chain);

#endif /* __PNSO_CHAIN_H__ */
