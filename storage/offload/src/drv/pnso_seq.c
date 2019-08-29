/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "pnso_seq_ops.h"
#include "pnso_seq.h"
#include "pnso_utils.h"

/* run on model/dol or on real hardware */
#ifdef PNSO_API_ON_MODEL
const struct sequencer_ops *g_sequencer_ops = &model_seq_ops;
#else
const struct sequencer_ops *g_sequencer_ops = &hw_seq_ops;
#endif

pnso_error_t
seq_setup_desc(struct service_info *svc_info, const void *src_desc,
		size_t desc_size, void **seq_desc_new)
{
	return g_sequencer_ops->setup_desc(svc_info, src_desc,
			desc_size, seq_desc_new);
}

void
seq_cleanup_desc(struct service_info *svc_info)
{
	return g_sequencer_ops->cleanup_desc(svc_info);
}

pnso_error_t
seq_ring_db(struct service_info *svc_info)
{
	pnso_error_t err;

	if (pnso_lif_reset_ctl_pending()) {
		err = PNSO_LIF_IO_ERROR;
		OSAL_LOG_ERROR("pnso pending error reset! err: %d", err);
		goto out;
	}
	err = g_sequencer_ops->ring_db(svc_info);
out:
	return err;
}

pnso_error_t
seq_setup_cp_chain_params(struct service_info *svc_info,
			struct cpdc_desc *cp_desc,
			struct cpdc_status_desc *status_desc)
{
	return g_sequencer_ops->setup_cp_chain_params(svc_info,
			cp_desc, status_desc);
}

pnso_error_t
seq_setup_cpdc_chain(struct service_info *svc_info,
		   struct cpdc_desc *cp_desc)
{
	return g_sequencer_ops->setup_cpdc_chain(svc_info, cp_desc);
}

pnso_error_t
seq_setup_cp_pad_chain_params(struct service_info *svc_info,
			struct cpdc_desc *cp_desc,
			struct cpdc_status_desc *status_desc)
{
	return g_sequencer_ops->setup_cp_pad_chain_params(svc_info,
			cp_desc, status_desc);
}

pnso_error_t
seq_setup_hash_chain_params(struct cpdc_chain_params *chain_params,
			struct service_info *svc_info,
			struct cpdc_desc *hash_desc, struct cpdc_sgl *sgl)
{
	return g_sequencer_ops->setup_hash_chain_params(chain_params, svc_info,
			hash_desc, sgl);
}

pnso_error_t
seq_setup_chksum_chain_params(struct cpdc_chain_params *chain_params,
			struct service_info *svc_info,
			struct cpdc_desc *chksum_desc, struct cpdc_sgl *sgl)
{
	return g_sequencer_ops->setup_chksum_chain_params(chain_params,
			svc_info, chksum_desc, sgl);
}
pnso_error_t
seq_setup_cpdc_chain_status_desc(struct service_info *svc_info)
{
	return g_sequencer_ops->setup_cpdc_chain_status_desc(svc_info);
}

void
seq_cleanup_cpdc_chain(struct service_info *svc_info)
{
	g_sequencer_ops->cleanup_cpdc_chain(svc_info);
}

pnso_error_t
seq_setup_crypto_chain(struct service_info *svc_info,
			struct crypto_desc *desc)
{
	return g_sequencer_ops->setup_crypto_chain(svc_info, desc);
}

void
seq_cleanup_crypto_chain(struct service_info *svc_info)
{
	g_sequencer_ops->cleanup_crypto_chain(svc_info);
}

pnso_error_t
seq_get_statusq(struct service_info *svc_info,
		struct queue **ret_statusq)
{
	struct service_info	*svc_prev;
	struct lif		*lif;
	struct queue		*statusq;
	pnso_error_t		err = PNSO_OK;

	statusq = NULL;
	lif = sonic_get_lif();
	if (!lif) {
		err = EINVAL;
		OSAL_LOG_ERROR("failed to find lif! err: %d", err);
		goto out;
	}

	/*
	 * If parent service is of the same type, share its statusq
	 * if available. Note the typical case is either a chain
	 * of at most 2 services, or the parent would be the same
	 * type so the below loop should execute just once in the
	 * majority of cases.
	 */
	svc_prev = chn_service_prev_svc_get(svc_info);
	while (svc_prev) {
		if (svc_prev->si_seq_info.sqi_status_qtype ==
		    svc_info->si_seq_info.sqi_status_qtype) {

			switch (svc_prev->si_seq_info.sqi_status_qtype) {

			case SONIC_QTYPE_CPDC_STATUS:
				statusq = svc_prev->si_cpdc_chain.
						ccp_seq_spec.sqs_seq_status_q;
				break;

			case SONIC_QTYPE_CRYPTO_STATUS:
				statusq = svc_prev->si_crypto_chain.
						ccp_seq_spec.sqs_seq_status_q;
				break;

			default:
				break;
			}
		}
		if (statusq)
			break;

                svc_prev = chn_service_prev_svc_get(svc_prev);
	}

	/*
	 * Even if statusq had been found above, pass it to
	 * sonic_get_seq_statusq() for ref counting purposes.
	 */
	err = sonic_get_seq_statusq(lif,
		svc_info->si_seq_info.sqi_status_qtype, &statusq);
out:
	*ret_statusq = statusq;
	return err;
}

