/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <netdevice.h>

#include "sonic_dev.h"
#include "sonic_lif.h"

#include "osal.h"
#include "accel_ring.h"

#include "pnso_mpool.h"
#include "pnso_pbuf.h"
#include "pnso_chain.h"
#include "pnso_cpdc.h"

/*
 * TODO-chain:
 *	- pass/emit req_id across
 *	- update return values in header template
 *
 */
osal_atomic_int_t g_req_id;

#ifdef NDEBUG
#define PPRINT_SERVICE_INFO(s)
#define PPRINT_CHAIN(c)
#else
#define PPRINT_CHAIN(c)			pprint_chain(c)
#define PPRINT_SERVICE_INFO(s)		pprint_service_info(s)
#endif

static void __attribute__((unused))
pprint_service_info(const struct service_info *svc_info)
{
	if (!svc_info)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "=== svc_info",  (u64) svc_info);
	OSAL_LOG_INFO("%30s: %d", "si_type", svc_info->si_type);
	OSAL_LOG_INFO("%30s: %d", "si_flags", svc_info->si_flags);

	OSAL_LOG_INFO("%30s: %d", "si_block_size", svc_info->si_block_size);
	OSAL_LOG_INFO("%30s: %d", "si_desc_flagss", svc_info->si_desc_flags);

	OSAL_LOG_INFO("%30s: 0x%llx", "si_desc",  (u64) svc_info->si_desc);
	OSAL_LOG_INFO("%30s: 0x%llx", "si_status_desc",
			(u64) svc_info->si_status_desc);

	OSAL_LOG_INFO("%30s: 0x%llx", "=== si_src_sgl",
			(u64) svc_info->si_src_sgl);
	OSAL_LOG_INFO("%30s: %llx", "=== si_dst_sgl",
			(u64) svc_info->si_dst_sgl);

	/* TODO-chain: include service status and other members */
}

static void __attribute__((unused))
pprint_chain(const struct service_chain *chain)
{
	uint32_t i;
	struct chain_entry *sc_entry;

	if (!chain)
		return;

	OSAL_LOG_INFO("%30s: %p", "chain", chain);
	OSAL_LOG_INFO("%30s: %d", "chain->sc_req_id", chain->sc_req_id);
	OSAL_LOG_INFO("%30s: %d", "chain->sc_num_services",
			chain->sc_num_services);

	i = 0;
	sc_entry = chain->sc_entry;
	while (sc_entry) {
		OSAL_LOG_INFO("%30s: %d", "service: #", ++i);

		/* chain entry */
		OSAL_LOG_INFO("%30s: %p", "chain->sc_entry", sc_entry);
		OSAL_LOG_INFO("%30s: %p", "chain->sc_entry->ce_chain_head",
			      sc_entry->ce_chain_head);
		OSAL_LOG_INFO("%30s: %p", "chain->sc_entry->ce_next",
			      sc_entry->ce_next);

		/* service info */
		OSAL_LOG_INFO("%30s: %d", "ce_svc_info->si_type",
			      sc_entry->ce_svc_info.si_type);
		OSAL_LOG_INFO("%30s: %d", "ce_svc_info->si_flags",
			      sc_entry->ce_svc_info.si_flags);
		OSAL_LOG_INFO("%30s: %p", "ce_svc_info->si_ops",
			      &sc_entry->ce_svc_info.si_ops);

		sc_entry = sc_entry->ce_next;
	}

	OSAL_LOG_INFO("%30s: %p", "chain->sc_req_cb", chain->sc_req_cb);
	OSAL_LOG_INFO("%30s: %p", "chain->sc_req_cb_ctx",
			chain->sc_req_cb_ctx);
	OSAL_LOG_INFO("%30s: %p", "chain->sc_req_poll_fn",
			chain->sc_req_poll_fn);
	OSAL_LOG_INFO("%30s: %p", "chain->sc_req_poll_ctx",
			chain->sc_req_poll_ctx);
}

static pnso_error_t
init_service_params(const struct pnso_service_request *svc_req,
		const struct pnso_service_status *svc_status,
		struct pnso_service *pnso_svc,
		struct service_params *svc_params)
{
	OSAL_ASSERT(pnso_svc->svc_type == svc_status->svc_type);

	svc_params->sp_src_blist = svc_req->sgl;
	svc_params->sp_dst_blist = svc_status->u.dst.sgl;

	switch (pnso_svc->svc_type) {
	case PNSO_SVC_TYPE_ENCRYPT:
	case PNSO_SVC_TYPE_DECRYPT:
		svc_params->u.sp_crypto_desc = &pnso_svc->u.crypto_desc;
		break;
	case PNSO_SVC_TYPE_COMPRESS:
		svc_params->u.sp_cp_desc = &pnso_svc->u.cp_desc;
		break;
	case PNSO_SVC_TYPE_DECOMPRESS:
		svc_params->u.sp_dc_desc = &pnso_svc->u.dc_desc;
		break;
	case PNSO_SVC_TYPE_HASH:
		svc_params->u.sp_hash_desc = &pnso_svc->u.hash_desc;
		break;
	case PNSO_SVC_TYPE_CHKSUM:
		svc_params->u.sp_chksum_desc = &pnso_svc->u.chksum_desc;
		break;
	case PNSO_SVC_TYPE_DECOMPACT:
	case PNSO_SVC_TYPE_NONE:
	default:
		OSAL_ASSERT(0);
		return EINVAL;
	}

	return PNSO_OK;
}

static pnso_error_t
init_service_info(enum pnso_service_type svc_type,
		struct pnso_service_status *svc_status,
		struct service_info *svc_info)
{

	switch (svc_type) {
	case PNSO_SVC_TYPE_ENCRYPT:
		svc_info->si_ops = encrypt_ops;
		svc_info->si_seq_info.sqi_ring_id = ACCEL_RING_XTS0;
		svc_info->si_seq_info.sqi_qtype = SONIC_QTYPE_CRYPTO_ENC_SQ;
		break;
	case PNSO_SVC_TYPE_DECRYPT:
		svc_info->si_ops = decrypt_ops;
		svc_info->si_seq_info.sqi_ring_id = ACCEL_RING_XTS1;
		svc_info->si_seq_info.sqi_qtype = SONIC_QTYPE_CRYPTO_DEC_SQ;
		break;
	case PNSO_SVC_TYPE_COMPRESS:
		svc_info->si_ops = cp_ops;
		svc_info->si_seq_info.sqi_ring_id = ACCEL_RING_CP;
		svc_info->si_seq_info.sqi_qtype = SONIC_QTYPE_CP_SQ;
		break;
	case PNSO_SVC_TYPE_DECOMPRESS:
		svc_info->si_ops = dc_ops;
		svc_info->si_seq_info.sqi_ring_id = ACCEL_RING_DC;
		svc_info->si_seq_info.sqi_qtype = SONIC_QTYPE_DC_SQ;
		break;
	case PNSO_SVC_TYPE_HASH:
		svc_info->si_ops = hash_ops;
		svc_info->si_seq_info.sqi_ring_id = ACCEL_RING_CP_HOT;
		svc_info->si_seq_info.sqi_qtype = SONIC_QTYPE_CP_SQ;
		break;
	case PNSO_SVC_TYPE_CHKSUM:
		svc_info->si_ops = chksum_ops;
		svc_info->si_seq_info.sqi_ring_id = ACCEL_RING_DC_HOT;
		svc_info->si_seq_info.sqi_qtype = SONIC_QTYPE_DC_SQ;
		break;
	case PNSO_SVC_TYPE_DECOMPACT:
	case PNSO_SVC_TYPE_NONE:
	default:
		OSAL_ASSERT(0);
		return EINVAL;
	}

	/* svc_info->si_flags updated outside TODO-chain: leave this here?? */
	svc_info->si_block_size = 4096;	/* TODO-chain: get via init params??  */
	svc_info->si_svc_status = svc_status;

	return PNSO_OK;
}

void
chn_destroy_chain(struct service_chain *chain)
{
	pnso_error_t err;
	struct per_core_resource *pc_res;
	struct mem_pool *svc_chain_mpool;
	struct mem_pool *svc_chain_entry_mpool;
	struct chain_entry *sc_entry;
	struct chain_entry *sc_next;
	struct service_info *svc_info;
	uint32_t i;

	if (!chain)
		return;

	OSAL_LOG_DEBUG("enter ...");
	OSAL_LOG_INFO("chain: %p num_services: %d ", chain,
			chain->sc_num_services);

	pc_res = chain->sc_pc_res;
	OSAL_ASSERT(pc_res);

	svc_chain_mpool = pc_res->mpools[MPOOL_TYPE_SERVICE_CHAIN];
	OSAL_ASSERT(svc_chain_mpool);

	svc_chain_entry_mpool = pc_res->mpools[MPOOL_TYPE_SERVICE_CHAIN_ENTRY];
	OSAL_ASSERT(svc_chain_entry_mpool);

	i = 0;
	sc_entry = chain->sc_entry;
	while (sc_entry) {
		i++;

		svc_info = &sc_entry->ce_svc_info;
		svc_info->si_ops.teardown(svc_info);

		sc_next = sc_entry->ce_next;

		err = mpool_put_object(svc_chain_entry_mpool, sc_entry);
		if (err) {
			OSAL_LOG_ERROR("failed to return service chain entry object to pool! err: %d",
				err);
			OSAL_ASSERT(0);
		}
		sc_entry = sc_next;
	}
	OSAL_ASSERT(i == chain->sc_num_services);

	err = mpool_put_object(svc_chain_mpool, chain);
	if (err) {
		OSAL_LOG_ERROR("failed to return service chain object to pool! err: %d",
				err);
		OSAL_ASSERT(0);
	}

	OSAL_LOG_DEBUG("exit!");
}

pnso_error_t
chn_build_chain(struct pnso_service_request *svc_req,
		struct pnso_service_result *svc_res,
		const completion_cb_t cb, void *cb_ctx,
		void *pnso_poll_fn, void *pnso_poll_ctx)
{
	pnso_error_t err = EINVAL;
	struct lif *lif;
	struct per_core_resource *pc_res;
	struct mem_pool *svc_chain_mpool;
	struct mem_pool *svc_chain_entry_mpool;
	struct pnso_service_request *req;
	struct pnso_service_result *res;
	struct service_chain *chain;
	struct chain_entry *centry = NULL;
	struct chain_entry *centry_prev = NULL;
	struct service_info *svc_info;
	struct service_params svc_params;
	uint32_t i;

	OSAL_LOG_DEBUG("enter ...");

	lif = sonic_get_lif();
	if (!lif) {
		OSAL_ASSERT(0);
		goto out;
	}

	pc_res = sonic_get_per_core_res(lif);
	if (!pc_res) {
		OSAL_ASSERT(0);
		goto out;
	}

	svc_chain_mpool = pc_res->mpools[MPOOL_TYPE_SERVICE_CHAIN];
	if (!svc_chain_mpool) {
		OSAL_ASSERT(0);
		goto out;
	}

	svc_chain_entry_mpool = pc_res->mpools[MPOOL_TYPE_SERVICE_CHAIN_ENTRY];
	if (!svc_chain_entry_mpool) {
		OSAL_ASSERT(0);
		goto out;
	}

	chain = (struct service_chain *) mpool_get_object(svc_chain_mpool);
	if (!chain) {
		err = ENOMEM;
		OSAL_LOG_ERROR("cannot obtain service chain object from pool! err: %d",
				err);
		goto out;
	}

	req = svc_req;
	res = svc_res;

	chain->sc_num_services = req->num_services;
	chain->sc_entry = NULL;
	chain->sc_res = res;

	chain->sc_pc_res = pc_res;

	chain->sc_req_cb = cb;
	chain->sc_req_cb_ctx = cb_ctx;
	chain->sc_req_poll_fn = pnso_poll_fn;
	chain->sc_req_poll_ctx = pnso_poll_ctx;

	/* init services in the chain  */
	for (i = 0; i < chain->sc_num_services; i++) {
		centry = (struct chain_entry *)
			mpool_get_object(svc_chain_entry_mpool);
		if (!centry) {
			err = ENOMEM;
			OSAL_LOG_ERROR("cannot obtain service chain entry object from pool! err: %d",
				err);
			goto out_free_chain;
		}
		memset(centry, 0, sizeof(struct chain_entry));

		centry->ce_chain_head = chain;
		centry->ce_next = NULL;
		svc_info = &centry->ce_svc_info;

		if (i == 0) {
			svc_info->si_flags = (chain->sc_num_services == 1) ?
				CHAIN_SFLAG_LONE_SERVICE :
				CHAIN_SFLAG_FIRST_SERVICE;
			chain->sc_entry = centry;
		} else
			centry_prev->ce_next = centry;

		centry_prev = centry;

		init_service_params(req, &res->svc[i],
				&req->svc[i], &svc_params);

		init_service_info(req->svc[i].svc_type, &res->svc[i], svc_info);
		svc_info->si_pc_res = chain->sc_pc_res;

		err = svc_info->si_ops.setup(svc_info, &svc_params);
		if (err)
			goto out_free_chain;

		if (!(svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE))
			if (i+1 == chain->sc_num_services)
				svc_info->si_flags |= CHAIN_SFLAG_LAST_SERVICE;

		PPRINT_SERVICE_INFO(svc_info);
	}
	chain->sc_req_id = osal_atomic_fetch_add(&g_req_id, 1);

	/* chain the services  */
	centry = chain->sc_entry;
	svc_info = &centry->ce_svc_info;
	err = svc_info->si_ops.chain(centry);
	if (err)
		goto out_free_chain;

	/* execute the chain  */
	chn_execute_chain(chain);
	err = PNSO_OK;

	OSAL_LOG_DEBUG("exit!");
	return err;

out_free_chain:
	PPRINT_CHAIN(chain);
	chn_destroy_chain(chain);
out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static void
update_overall_result(struct service_chain *chain)
{
	struct pnso_service_result *res;
	uint32_t i;

	OSAL_ASSERT(chain);

	res = chain->sc_res;
	OSAL_ASSERT(res);

	res->err = PNSO_OK;
	for (i = 0; i < res->num_services; i++) {
		if (res->svc[i].err) {
			res->err = res->svc[i].err;
			break;
		}
	}
}

void
chn_execute_chain(struct service_chain *chain)
{
	pnso_error_t err;
	struct chain_entry *sc_entry;
	struct chain_entry *ce_first, *ce_last;
	struct service_ops *svc_ops;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(chain);

	ce_first = ce_last = chain->sc_entry;
	if (!ce_first)
		return;

	sc_entry = chain->sc_entry;
	while (sc_entry) {
		ce_last = sc_entry;
		sc_entry = sc_entry->ce_next;
	}

	/* ring 'first' service door bell */
	sc_entry = ce_first;
	err = ce_first->ce_svc_info.si_ops.schedule(&sc_entry->ce_svc_info);
	if (err) {
		OSAL_LOG_ERROR("failed to schedule the service svc_type: %d err: %d",
			       ce_first->ce_svc_info.si_type, err);
		goto out;
	}

	/* wait for 'last' service completion */
	sc_entry = ce_last;
	err = ce_last->ce_svc_info.si_ops.poll(&sc_entry->ce_svc_info);
	if (err) {
		OSAL_LOG_ERROR("service failed to poll svc_type: %d err: %d",
			       ce_last->ce_svc_info.si_type, err);
		goto out;
	}

	/* update status of individual service(s) */
	sc_entry = chain->sc_entry;
	while (sc_entry) {
		svc_ops = &sc_entry->ce_svc_info.si_ops;
		err = svc_ops->read_status(&sc_entry->ce_svc_info);
		if (err)
			OSAL_LOG_ERROR("read status failed svc_type: %d err: %d",
				       sc_entry->ce_svc_info.si_type, err);

		err = svc_ops->write_result(&sc_entry->ce_svc_info);
		if (err)
			OSAL_LOG_ERROR("write result failed svc_type: %d err: %d",
				       sc_entry->ce_svc_info.si_type, err);

		sc_entry = sc_entry->ce_next;
	}

	/* update over all status of the chain */
	update_overall_result(chain);

out:
	/* TODO-chain: revisit this on error handling path */
	if (chain->sc_req_cb)
		chain->sc_req_cb(chain->sc_req_cb_ctx, chain->sc_res);

	chn_destroy_chain(chain);
	OSAL_LOG_DEBUG("exit");
}
