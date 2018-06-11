/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "pnso_global.h"
#include "pnso_logger.h"
#include "pnso_osal.h"

#include "pnso_api.h"
#include "pnso_cpdc.h"
#include "pnso_xts.h"
#include "pnso_req.h"		/* TODO: break req-chain circular dependency */
#include "pnso_chain.h"
#include "pnso_emul.h"

#include "../../dol/test/storage/dol_wrap.h"

extern uint64_t g_rid;

static void
__pprint_sgl(struct cpdc_sgl *sgl)
{
	if (!sgl)
		return;

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "cs_addr_0", sgl->cs_addr_0);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cs_len_0", sgl->cs_len_0);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "cs_addr_1", sgl->cs_addr_1);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cs_len_1", sgl->cs_len_1);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "cs_addr_2", sgl->cs_addr_2);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "cs_len_2", sgl->cs_len_2);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "cs_next", sgl->cs_next);
}

static void
__pprint_service_info(struct service_info *svc_info)
{
	if (!svc_info)
		return;

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "=== svc_info", svc_info);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "si_type", svc_info->si_type);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "si_flags", svc_info->si_flags);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "si_desc", svc_info->si_desc);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "si_status_buf",
			svc_info->si_status_buf);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "si_sbuf", svc_info->si_sbuf);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "si_dbuf", svc_info->si_dbuf);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "=== si_src_sgl",
			svc_info->si_src_sgl);
	__pprint_sgl(svc_info->si_src_sgl);

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "=== si_dst_sgl",
			svc_info->si_dst_sgl);
	__pprint_sgl(svc_info->si_dst_sgl);
}

static void
__pprint_chain(struct service_chain *chain)
{
	uint32_t i;
	struct chain_entry *sc_entry;

	if (!chain)
		return;

	PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "chain", chain);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "chain->sc_req_id",
		      chain->sc_req_id);
	PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "chain->sc_num_services",
		      chain->sc_num_services);

	i = 0;
	sc_entry = chain->sc_entry;
	while (sc_entry) {
		PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "service: #", ++i);

		/* chain entry */
		PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "chain->sc_entry", sc_entry);
		PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "chain->sc_entry->ce_head",
			      sc_entry->ce_chead);
		PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "chain->sc_entry->ce_next",
			      sc_entry->ce_next);

		/* service info */
		PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "ce_sinfo->si_type",
			      sc_entry->ce_sinfo.si_type);
		PNSO_LOG_INFO(PNSO_OK, "%30s: %d", "ce_sinfo->si_flags",
			      sc_entry->ce_sinfo.si_flags);
		PNSO_LOG_INFO(PNSO_OK, "%30s: %p", "ce_sinfo->si_ops",
			      sc_entry->ce_sinfo.si_ops);

		sc_entry = sc_entry->ce_next;
	}
}

static pnso_error_t
__init_service_info(enum pnso_service_type svc_type,
		struct pnso_service_status *svc_status,
		struct service_info *svc_info)
{
	svc_info->si_type = svc_type;
	svc_info->si_status = svc_status;

	switch (svc_type) {
	case PNSO_SVC_TYPE_ENCRYPT:
		svc_info->si_ops = encrypt_ops;
		break;
	case PNSO_SVC_TYPE_DECRYPT:
		svc_info->si_ops = decrypt_ops;
		break;
	case PNSO_SVC_TYPE_COMPRESS:
		svc_info->si_ops = cp_ops;
		break;
	case PNSO_SVC_TYPE_DECOMPRESS:
		svc_info->si_ops = dc_ops;
		break;
	case PNSO_SVC_TYPE_CHKSUM:
		svc_info->si_ops = chksum_ops;
		break;
	case PNSO_SVC_TYPE_HASH:
		svc_info->si_ops = hash_ops;
		break;
	case PNSO_SVC_TYPE_DECOMPACT:
	case PNSO_SVC_TYPE_NONE:
	default:
		assert(0);
		return EINVAL;
	}

	return PNSO_OK;
}

static void
__destroy_chain(struct service_chain *chain)
{
	struct chain_entry *sc_entry;
	struct chain_entry *sc_next;
	struct service_info *svc_info;
	uint32_t i;

	if (!chain)
		return;

	PNSO_LOG_INFO(PNSO_OK, "start ... chain: %p num_services: %d ",
		      chain, chain->sc_num_services);

	i = 0;
	sc_entry = chain->sc_entry;
	while (sc_entry) {
		i++;

		svc_info = &sc_entry->ce_sinfo;
		svc_info->si_ops.teardown(svc_info->si_desc);

		sc_next = sc_entry->ce_next;
		pnso_free(sc_entry);

		sc_entry = sc_next;
	}
	assert(i == chain->sc_num_services);

	pnso_free(chain);

	PNSO_LOG_INFO(PNSO_OK, "done");
}

pnso_error_t
chn_build_service_chain(struct pnso_service_request *svc_req,
			struct pnso_service_result *svc_res,
			completion_cb_t cb,
			void *cb_ctx, void *pnso_poll_fn, void *pnso_poll_ctx)
{
	pnso_error_t err;
	struct pnso_service_request *req;
	struct pnso_service_result *res;
	struct service_chain *chain;
	struct chain_entry *centry = NULL;
	struct chain_entry *centry_prev = NULL;
	struct service_info *svc_info;
	uint32_t i;
	void *cp_desc_hack, *hash_desc_hack;

	PNSO_LOG_INFO(PNSO_OK, "start ...");

	chain = pnso_malloc(sizeof(struct service_chain));
	if (!chain) {
		err = ENOMEM;
		goto out;
	}

	req = svc_req;
	res = svc_res;

	chain->sc_num_services = req->num_services;
	chain->sc_entry = NULL;
	chain->sc_res = res;

	chain->sc_req_cb = cb;
	chain->sc_req_cb_ctx = cb_ctx;
	chain->sc_req_poll_fn = pnso_poll_fn;
	chain->sc_req_poll_ctx = pnso_poll_ctx;

	/* init services in the chain  */
	for (i = 0; i < chain->sc_num_services; i++) {
		centry = pnso_malloc(sizeof(struct chain_entry));
		if (!centry) {
			err = ENOMEM;
			goto out_free_chain;
		}
		memset(centry, 0, sizeof(struct chain_entry));

		centry->ce_chead = chain;
		centry->ce_next = NULL;
		svc_info = &centry->ce_sinfo;

		if (i == 0) {
			svc_info->si_flags = (chain->sc_num_services == 1) ?
				CHAIN_SFLAG_LONE_SERVICE :
				CHAIN_SFLAG_FIRST_SERVICE;
			chain->sc_entry = centry;
		} else
			centry_prev->ce_next = centry;

		centry_prev = centry;

		__init_service_info(req->svc[i].svc_type,
				&res->svc[i],
				svc_info);

		err = svc_info->si_ops.setup(svc_info,
				req->sgl,
				&req->svc[i].u,
				res->svc[i].u.dst.sgl);
		if (err)
			goto out_free_chain;


		if (i == 0)
			cp_desc_hack = svc_info->si_desc;
		if (i == 1)
			hash_desc_hack = svc_info->si_desc;

		if (!(svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE))
			if (i+1 == chain->sc_num_services)
				svc_info->si_flags |= CHAIN_SFLAG_LAST_SERVICE;

		__pprint_service_info(svc_info);
	}
	chain->sc_req_id = g_rid++;

	/* chain the services  */
	centry = chain->sc_entry;
	svc_info = &centry->ce_sinfo;
	err = svc_info->si_ops.chain(centry);
	if (err)
		goto out_free_chain;

#if CHAIN_BY_NAME
	emul_setup_comp_hash_chain(cp_desc_hack, 64 * sizeof(char),
			hash_desc_hack, 64 * sizeof(char));
#endif
	/* execute the chain  */
	err = chn_execute_chain(chain);
	if (err) {
		PNSO_LOG_ERROR(err, "failed to complete the request rid: %d",
				chain->sc_req_id);
		goto out_free_chain;
	}

	PNSO_LOG_INFO(err, "done");
	return err;

out_free_chain:
	__pprint_chain(chain);	/* TODO: get rid of this */
	__destroy_chain(chain);
out:
	return err;
}

static void
__update_overall_result(struct service_chain *chain)
{
	struct pnso_service_result *res;
	uint32_t i;

	assert(chain);

	res = chain->sc_res;
	assert(res);

	res->err = PNSO_OK;
	for (i = 0; i < res->num_services; i++) {
		if (res->svc[i].err) {
			res->err = res->svc[i].err;
			break;
		}
	}
}

pnso_error_t
chn_execute_chain(struct service_chain *chain)
{
	pnso_error_t err;
	struct chain_entry *sc_entry;
	struct chain_entry *ce_first, *ce_last;
	struct service_ops *svc_ops;

	PNSO_LOG_INFO(PNSO_OK, "enter ...");

	assert(chain);

	ce_first = ce_last = chain->sc_entry;
	if (!ce_first)
		return EINVAL;

	sc_entry = chain->sc_entry;
	while (sc_entry) {
		ce_last = sc_entry;
		sc_entry = sc_entry->ce_next;
	}

	/* ring 'first' service door bell */
	sc_entry = ce_first;
	err = ce_first->ce_sinfo.si_ops.schedule(&sc_entry->ce_sinfo);
	if (err) {
		PNSO_LOG_ERROR(err,
			       "failed to schedule the service svc_type: %d",
			       ce_first->ce_sinfo.si_type);
		goto out;
	}

	/* wait for 'last' service completion */
	sc_entry = ce_last;
	err = ce_last->ce_sinfo.si_ops.poll(&sc_entry->ce_sinfo);
	if (err) {
		PNSO_LOG_ERROR(err, "service failed to poll svc_type: %d",
			       ce_last->ce_sinfo.si_type);
		goto out;
	}

	/* update status of individual service(s) */
	sc_entry = chain->sc_entry;
	while (sc_entry) {
		svc_ops = &sc_entry->ce_sinfo.si_ops;
		err = svc_ops->read_status(&sc_entry->ce_sinfo);
		if (err) {
			PNSO_LOG_ERROR(err,
				       "status verification failed svc_type: %d",
				       sc_entry->ce_sinfo.si_type);
			goto out;
		}

		err = svc_ops->write_result(&sc_entry->ce_sinfo);
		if (err) {
			PNSO_LOG_ERROR(err,
				       "status verification failed svc_type: %d",
				       sc_entry->ce_sinfo.si_type);
			goto out;
		}
		sc_entry = sc_entry->ce_next;
	}

	/* update over all status of the chain */
	__update_overall_result(chain);

// out_free_chain: TODO
	__destroy_chain(chain);
out:
	PNSO_LOG_INFO(err, "exit");
	return PNSO_OK;
}
