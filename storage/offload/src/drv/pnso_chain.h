/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_CHAIN_H__
#define __PNSO_CHAIN_H__

#include "pnso_global.h"

#include "pnso_api.h"

/*
 * The design of handling a client-request within the driver, and forwarding
 * it to hardware, and then handling of its response follows some 'shades' of
 * following design patterns:
 *	- Facade
 *	- Chain of responsibility
 *	- Obeserver
 *
 */

/* service flags */
#define CHAIN_SFLAG_LONE_SERVICE	(1 << 0)
#define CHAIN_SFLAG_FIRST_SERVICE	(1 << 1)
#define CHAIN_SFLAG_LAST_SERVICE	(1 << 2)

struct service_info;
struct chain_entry;

struct service_ops {
	/* obtain and initialize op-descriptor, seq and seq queues, etc. */
	pnso_error_t (*setup)(struct service_info *svc_info,
			struct pnso_buffer_list *src_buf,
			void *desc,
			void *dst_buf);

	/* chain the service(s) */
	pnso_error_t (*chain)(struct chain_entry *centry);

	/* a NULL-op for all services except the first within the chain */
	pnso_error_t (*schedule)(struct service_info *svc_info);

	/* a NULL-op for all services except the last within the chain */
	pnso_error_t (*poll)(struct service_info *svc_info);

	/* get hardware status of the service */
	pnso_error_t (*read_status)(struct service_info *svc_info);

	/* set status to update the caller */
	pnso_error_t (*write_result)(struct service_info *svc_info);

	/* releases descriptor, etc. to the pool and conducts cleanup task */
	void (*teardown)(struct service_info *svc_info);
};

struct sequencer_info {
	uint32_t qid;
	uint16_t index;
	void *seq_desc;
};

struct service_info {
	uint8_t			si_type;
	uint8_t			si_flags;

	struct service_ops	si_ops;
	void			*si_desc;
	void			*si_status_buf;
	struct pnso_buffer_list *si_sbuf;
	struct pnso_buffer_list *si_dbuf;
	struct cpdc_sgl		*si_src_sgl;
	struct cpdc_sgl		*si_dst_sgl;
	struct sequencer_info	si_seq_info;

	struct pnso_service_status *si_status;
};

struct chain_entry {
	struct service_chain	*ce_chead;	/* back pointer to chain head */
	struct chain_entry	*ce_next;
	struct service_info	ce_sinfo;	/* service within the chain */
};

struct service_chain {
	uint32_t		sc_req_id;	/* unique request id */
	uint32_t		sc_num_services;
	struct chain_entry	*sc_entry;	/* list of services */
	struct pnso_service_result *sc_res;	/* vendor res */

	completion_cb_t		sc_req_cb;	/* vendor call-back */
	void			*sc_req_cb_ctx;	/* vendor cb context */
	void			*sc_req_poll_fn;
	void			*sc_req_poll_ctx;
	/* TODO: keep only the relevant ones */
};

/**
 * chn_build_service_chain() -
 * @svc_req:		[in]	specifies the
 * @svc_res:		[in]	specifies the
 * @cb:			[in]	specifies the
 * @pnso_poll_fn:	[in]	specifies the
 * @pnso_poll_ctx:	[in]	specifies the
 *
 * Return Value:
 *	PNSO_OK	-
 *
 */
pnso_error_t chn_build_service_chain(struct pnso_service_request *svc_req,
		struct pnso_service_result *svc_res,
		completion_cb_t cb,
		void *cb_ctx,
		void *pnso_poll_fn,
		void *pnso_poll_ctx);

/**
 * chn_create_chain() -
 * @svc_req:	[in]	specifies the
 * @out_chain:	[in]	specifies the
 *
 * Return Value:
 *	PNSO_OK	-
 *
 */
pnso_error_t chn_create_chain(struct pnso_service_request *req,
		struct service_chain **out_chain);
/**
 * chn_execute_chain() -
 * @chain:	[in]	specifies the
 *
 * Return Value:
 *	PNSO_OK	-
 *
 */
pnso_error_t chn_execute_chain(struct service_chain *chain);

/**
 * chn_destroy_chain() -
 * @chain:	[in]	specifies the
 *
 * Return Value:
 *	None
 *
 */
void chn_destroy_chain(struct service_chain *chain);

#endif /* __PNSO_CHAIN_H__ */
