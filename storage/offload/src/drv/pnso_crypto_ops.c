/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "pnso_mpool.h"
#include "pnso_pbuf.h"
#include "pnso_chain.h"
#include "pnso_crypto.h"
#include "pnso_cpdc.h"
#include "pnso_crypto_cmn.h"
#include "pnso_seq.h"
#include "pnso_utils.h"
#include "sonic_api_int.h"

static enum crypto_algo_cmd_hi   crypto_algo_cmd_hi_tbl[PNSO_CRYPTO_TYPE_MAX] = {
	[PNSO_CRYPTO_TYPE_XTS] = CRYPTO_ALGO_CMD_HI_AES_XTS,
	[PNSO_CRYPTO_TYPE_GCM] = CRYPTO_ALGO_CMD_HI_AES_GCM,
};

static enum crypto_algo_cmd_lo   crypto_algo_cmd_lo_tbl[PNSO_CRYPTO_TYPE_MAX] = {
	[PNSO_CRYPTO_TYPE_XTS] = CRYPTO_ALGO_CMD_LO_AES_XTS,
	[PNSO_CRYPTO_TYPE_GCM] = CRYPTO_ALGO_CMD_LO_AES_GCM,
};

static pnso_error_t
crypto_validate_input(struct service_info *svc_info,
		      const struct service_params *svc_params)
{
	struct pnso_crypto_desc	*pnso_crypto_desc;

	if (!svc_info || !svc_params) {
		OSAL_LOG_ERROR("invalid svc_info 0x%llx or svc_params 0x%llx!",
				(uint64_t)svc_info, (uint64_t)svc_params);
		return EINVAL;
	}

	if (!svc_params->sp_src_blist || !svc_params->sp_dst_blist) {
		OSAL_LOG_ERROR("invalid sp_src_blist 0x%llx or "
				"sp_dst_blist 0x%llx!",
				(uint64_t)svc_params->sp_src_blist,
				(uint64_t)svc_params->sp_dst_blist);
		return EINVAL;
	}

	pnso_crypto_desc = svc_params->u.sp_crypto_desc;
	if (!pnso_crypto_desc) {
		OSAL_LOG_ERROR("invalid desc 0x%llx specified!",
				(uint64_t)pnso_crypto_desc);
		return EINVAL;
	}

	if ((pnso_crypto_desc->algo_type == PNSO_CRYPTO_TYPE_NONE) ||
	    (pnso_crypto_desc->algo_type >= PNSO_CRYPTO_TYPE_MAX)) {
		OSAL_LOG_ERROR("invalid algo_type %u specified!",
				pnso_crypto_desc->algo_type);
		return EINVAL;
	}

	if (!pnso_crypto_desc->iv_addr) {
		OSAL_LOG_ERROR("invalid iv_addr 0x%llx specified!",
				pnso_crypto_desc->iv_addr);
		return EINVAL;
	}

	return PNSO_OK;
}

static inline void
crypto_desc_fill(struct service_info *svc_info,
		 struct pnso_crypto_desc *pnso_crypto_desc)
{
	struct crypto_desc *crypto_desc = svc_info->si_desc;
	struct crypto_status_desc *status_desc = svc_info->si_status_desc;

	memset(crypto_desc, 0, sizeof(*crypto_desc));
	memset(status_desc, 0, sizeof(*status_desc));

	crypto_desc->cd_in_aol = osal_virt_to_phy(svc_info->si_src_aol);
	crypto_desc->cd_out_aol = osal_virt_to_phy(svc_info->si_dst_aol);

	OSAL_ASSERT(pnso_crypto_desc->algo_type < PNSO_CRYPTO_TYPE_MAX);
	crypto_desc->cd_db_data = CRYPTO_ENCRYPT_CPL_DATA;
	if (svc_info->si_type == PNSO_SVC_TYPE_DECRYPT) {
		crypto_desc->cd_cmd.cc_is_decrypt = true;
		crypto_desc->cd_db_data = CRYPTO_DECRYPT_CPL_DATA;
	}
	crypto_desc->cd_cmd.cc_token_3 =
			    crypto_algo_cmd_lo_tbl[pnso_crypto_desc->algo_type];
	crypto_desc->cd_cmd.cc_token_4 =
			    crypto_algo_cmd_hi_tbl[pnso_crypto_desc->algo_type];
	crypto_desc->cd_key_desc_idx = 
		     sonic_get_crypto_key_idx(pnso_crypto_desc->key_desc_idx);

	crypto_desc->cd_iv_addr = pnso_crypto_desc->iv_addr;
	crypto_desc->cd_status_addr = osal_virt_to_phy(status_desc);
	crypto_desc->cd_db_addr = crypto_desc->cd_status_addr +
				  sizeof(status_desc->csd_err);

	CRYPTO_PPRINT_DESC(crypto_desc);
}

static pnso_error_t
crypto_setup(struct service_info *svc_info,
	     const struct service_params *svc_params)
{
	uint32_t		src_total_len;
	uint32_t		dst_total_len;
	pnso_error_t		err;

	OSAL_LOG_DEBUG("enter ...");

	err = crypto_validate_input(svc_info, svc_params);
	if (err)
		goto out;

	svc_info->si_desc = pc_res_mpool_object_get(svc_info->si_pc_res,
						    MPOOL_TYPE_CRYPTO_DESC);
	if (!svc_info->si_desc) {
		err = ENOMEM;
		goto out;
	}

	svc_info->si_status_desc = pc_res_mpool_object_get(svc_info->si_pc_res,
					  MPOOL_TYPE_CRYPTO_STATUS_DESC);
	if (!svc_info->si_status_desc) {
		err = ENOMEM;
		goto out;
	}

	svc_info->si_src_aol = crypto_aol_packed_get(svc_info->si_pc_res,
				      svc_params->sp_src_blist, &src_total_len);
	if (!svc_info->si_src_aol) {
		err = ENOMEM;
		goto out;
	}

	svc_info->si_dst_aol = crypto_aol_packed_get(svc_info->si_pc_res,
				      svc_params->sp_dst_blist, &dst_total_len);
	if (!svc_info->si_dst_aol) {
		err = ENOMEM;
		goto out;
	}

	OSAL_LOG_INFO("src_total_len %u dst_total_len %u",
		      src_total_len, dst_total_len);
	if ((src_total_len == 0) || (dst_total_len < src_total_len)) {
		OSAL_LOG_ERROR("length error: src_total_len %u dst_total_len %u",
				(int)src_total_len, (int)dst_total_len);
		err = EINVAL;
		goto out;
	}

	svc_info->si_desc_flags = 0;
	crypto_desc_fill(svc_info, svc_params->u.sp_crypto_desc);

	if ((svc_info->si_flags & CHAIN_SFLAG_LONE_SERVICE) ||
			(svc_info->si_flags & CHAIN_SFLAG_FIRST_SERVICE)) {
		svc_info->si_seq_info.sqi_desc = seq_setup_desc(svc_info,
				svc_info->si_desc, sizeof(struct crypto_desc));
		if (!svc_info->si_seq_info.sqi_desc) {
			err = EINVAL;
			OSAL_LOG_ERROR("failed to setup sequencer desc! err: %d",
					err);
			goto out;
		}
	}

	err = PNSO_OK;
	OSAL_LOG_DEBUG("exit! service initialized!");
	return err;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static pnso_error_t
crypto_encrypt_setup(struct service_info *svc_info,
		     const struct service_params *svc_params)
{
	svc_info->si_type = PNSO_SVC_TYPE_ENCRYPT;
	return crypto_setup(svc_info, svc_params);
}

static pnso_error_t
crypto_decrypt_setup(struct service_info *svc_info,
		     const struct service_params *svc_params)
{
	svc_info->si_type = PNSO_SVC_TYPE_DECRYPT;
	return crypto_setup(svc_info, svc_params);
}

static pnso_error_t
crypto_chain(struct chain_entry *centry)
{
	return PNSO_OK;
}

static pnso_error_t
crypto_schedule(const struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	const struct sequencer_info *seq_info;
	bool ring_db;

	OSAL_LOG_INFO("enter ... ");

	ring_db = svc_info->si_flags & (CHAIN_SFLAG_LONE_SERVICE |
					CHAIN_SFLAG_FIRST_SERVICE);
	if (ring_db) {
		seq_info = &svc_info->si_seq_info;
		seq_ring_db(svc_info);
		err = PNSO_OK;
	}

	OSAL_LOG_DEBUG("exit!");
	return err;
}

static pnso_error_t
crypto_poll(const struct service_info *svc_info)
{
	volatile struct crypto_status_desc *status_desc;
	uint64_t cpl_data;

	OSAL_LOG_DEBUG("enter ...");

	status_desc = svc_info->si_status_desc;
	cpl_data = svc_info->si_type == PNSO_SVC_TYPE_DECRYPT ?
		   CRYPTO_DECRYPT_CPL_DATA : CRYPTO_ENCRYPT_CPL_DATA;
	while (status_desc->csd_cpl_data != cpl_data)
		osal_yield();

	OSAL_LOG_DEBUG("exit!");
	return PNSO_OK;
}

static pnso_error_t
crypto_read_status(const struct service_info *svc_info)
{
	struct crypto_status_desc *status_desc;
	pnso_error_t err;

	OSAL_LOG_DEBUG("enter ...");

	status_desc = svc_info->si_status_desc;
	err = status_desc->csd_err;
	if (err) {
		OSAL_LOG_ERROR("hw error reported! csd_err: %d", err);
	}
	return err;
}

static pnso_error_t
crypto_write_result(struct service_info *svc_info)
{
	pnso_error_t err = EINVAL;
	struct pnso_service_status *svc_status;
	struct crypto_status_desc *status_desc;

	OSAL_LOG_DEBUG("enter ...");
	svc_status = svc_info->si_svc_status;
	if (svc_status->svc_type != svc_info->si_type) {
		OSAL_LOG_ERROR("service type mismatch! svc_type: %d si_type: %d",
			svc_status->svc_type,  svc_info->si_type);
	}

	status_desc = svc_info->si_status_desc;
	if (status_desc->csd_err) {
		svc_status->err = crypto_desc_status_convert(status_desc->csd_err);
		OSAL_LOG_ERROR("service failed! err: %d", svc_status->err);
		goto out;
	}

	OSAL_LOG_DEBUG("exit! status/result update success!");
	return PNSO_OK;

out:
	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

static void
crypto_teardown(const struct service_info *svc_info)
{
	OSAL_LOG_DEBUG("enter ...");

	crypto_aol_put(svc_info->si_pc_res, svc_info->si_dst_aol);
	crypto_aol_put(svc_info->si_pc_res, svc_info->si_src_aol);
	pc_res_mpool_object_put(svc_info->si_pc_res, MPOOL_TYPE_CRYPTO_STATUS_DESC,
				svc_info->si_status_desc);
	pc_res_mpool_object_put(svc_info->si_pc_res, MPOOL_TYPE_CRYPTO_DESC,
				svc_info->si_desc);

	OSAL_LOG_DEBUG("exit!");
}

struct service_ops encrypt_ops = {
	.setup = crypto_encrypt_setup,
	.chain = crypto_chain,
	.schedule = crypto_schedule,
	.poll = crypto_poll,
	.read_status = crypto_read_status,
	.write_result = crypto_write_result,
	.teardown = crypto_teardown,
};

struct service_ops decrypt_ops = {
	.setup = crypto_decrypt_setup,
	.chain = crypto_chain,
	.schedule = crypto_schedule,
	.poll = crypto_poll,
	.read_status = crypto_read_status,
	.write_result = crypto_write_result,
	.teardown = crypto_teardown,
};
