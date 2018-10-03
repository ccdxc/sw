/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"
#include "sonic_api_int.h"

#include "pnso_pbuf.h"
#include "pnso_mpool.h"
#include "pnso_chain.h"
#include "pnso_crypto.h"
#include "pnso_utils.h"
#include "pnso_crypto_cmn.h"

void
crypto_pprint_aol(uint64_t aol_pa)
{
	const struct crypto_aol *aol;

	aol = (const struct crypto_aol *) sonic_phy_to_virt(aol_pa);
	while (aol) {
		OSAL_LOG_INFO("%30s: 0x%llx", "ca_addr_0", aol->ca_addr_0);
		OSAL_LOG_INFO("%30s: %d", "ca_off_0", aol->ca_off_0);
		OSAL_LOG_INFO("%30s: %d", "ca_len_0", aol->ca_len_0);

		OSAL_LOG_INFO("%30s: 0x%llx", "ca_addr_1", aol->ca_addr_1);
		OSAL_LOG_INFO("%30s: %d", "ca_off_1", aol->ca_off_1);
		OSAL_LOG_INFO("%30s: %d", "ca_len_1", aol->ca_len_1);

		OSAL_LOG_INFO("%30s: 0x%llx", "ca_addr_2", aol->ca_addr_2);
		OSAL_LOG_INFO("%30s: %d", "ca_off_2", aol->ca_off_2);
		OSAL_LOG_INFO("%30s: %d", "ca_len_2", aol->ca_len_2);

		OSAL_LOG_INFO("%30s: 0x%llx", "ca_next", aol->ca_next);
		OSAL_LOG_INFO("%30s: 0x%llx", "ca_rsvd", aol->ca_rsvd);
		aol = aol->ca_next ? sonic_phy_to_virt(aol->ca_next) : NULL;
	}
}

static void
crypto_pprint_cmd(const struct crypto_cmd *cmd)
{
	if (!cmd)
		return;

	OSAL_LOG_INFO("%30s: %d", "cc_enable_crc", cmd->cc_enable_crc);
	OSAL_LOG_INFO("%30s: %d", "cc_bypass_aes", cmd->cc_bypass_aes);
	OSAL_LOG_INFO("%30s: %d", "cc_is_decrypt", cmd->cc_is_decrypt);
	OSAL_LOG_INFO("%30s: %d", "cc_token_3", cmd->cc_token_3);
	OSAL_LOG_INFO("%30s: %d", "cc_token_4", cmd->cc_token_4);
}

void
crypto_pprint_desc(const struct crypto_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: 0x%llx", "crypto_desc", (uint64_t) desc);

	OSAL_LOG_INFO("%30s: 0x%llx", "cd_src", desc->cd_in_aol);
	OSAL_LOG_INFO("%30s: 0x%llx", "cd_dst", desc->cd_out_aol);

	OSAL_LOG_INFO("%30s: 0x%llx", "=== crypto_cmd", (uint64_t)&desc->cd_cmd);
	crypto_pprint_cmd(&desc->cd_cmd);

	OSAL_LOG_INFO("%30s: %d", "cd_key_desc_idx", desc->cd_key_desc_idx);
	OSAL_LOG_INFO("%30s: 0x%llx", "cd_iv_addr", desc->cd_iv_addr);

	OSAL_LOG_INFO("%30s: 0x%llx", "cd_auth_tag", desc->cd_auth_tag);

	OSAL_LOG_INFO("%30s: %d", "cd_hdr_size", desc->cd_hdr_size);
	OSAL_LOG_INFO("%30s: 0x%llx", "cd_status_addr", desc->cd_status_addr);

	OSAL_LOG_INFO("%30s: %d", "cd_otag", desc->cd_otag);
	OSAL_LOG_INFO("%30s: %d", "cd_otag_on", desc->cd_otag_on);

	OSAL_LOG_INFO("%30s: %d", "cd_sector_size", desc->cd_sector_size);

	OSAL_LOG_INFO("%30s: %d", "cd_app_tag", desc->cd_app_tag);
	OSAL_LOG_INFO("%30s: %d", "cd_sector_num", desc->cd_sector_num);

	OSAL_LOG_INFO("%30s: 0x%llx", "cd_db_addr", desc->cd_db_addr);
	OSAL_LOG_INFO("%30s: 0x%llx", "cd_db_data", desc->cd_db_data);

	OSAL_LOG_INFO("%30s: 0x%llx", "=== cd_in_aol", desc->cd_in_aol);
	crypto_pprint_aol(desc->cd_in_aol);

	OSAL_LOG_INFO("%30s: 0x%llx", "=== cd_out_aol", desc->cd_out_aol);
	crypto_pprint_aol(desc->cd_out_aol);
}

struct crypto_aol *
crypto_aol_packed_get(const struct per_core_resource *pc_res,
		      const struct pnso_buffer_list *buf_list,
		      uint32_t *ret_total_len)
{
	struct buffer_list_iter buffer_list_iter;
	struct buffer_list_iter *iter;
	struct crypto_aol *aol_head = NULL;
	struct crypto_aol *aol_prev = NULL;
	struct crypto_aol *aol;

	iter = buffer_list_iter_init(&buffer_list_iter, buf_list);
	*ret_total_len = 0;
	while (iter) {
		aol = pc_res_mpool_object_get(pc_res, MPOOL_TYPE_CRYPTO_AOL);
		if (!aol) {
			OSAL_LOG_ERROR("cannot obtain crypto aol from pool!");
			goto out;
		}
		memset(aol, 0, sizeof(*aol));
		iter = buffer_list_iter_addr_len_get(iter,
					CRYPTO_AOL_TUPLE_LEN_MAX,
					&aol->ca_addr_0, &aol->ca_len_0);
		if (iter)
			iter = buffer_list_iter_addr_len_get(iter,
					CRYPTO_AOL_TUPLE_LEN_MAX,
					&aol->ca_addr_1, &aol->ca_len_1);
		if (iter)
			iter = buffer_list_iter_addr_len_get(iter,
					CRYPTO_AOL_TUPLE_LEN_MAX,
					&aol->ca_addr_2, &aol->ca_len_2);
		*ret_total_len += aol->ca_len_0 + aol->ca_len_1 + aol->ca_len_2;

		if (!aol_head)
			aol_head = aol;
		else
			aol_prev->ca_next = sonic_virt_to_phy(aol);
		aol_prev = aol;
	}

	return aol_head;
out:
	crypto_aol_put(pc_res, aol_head);
	return NULL;
}

void
crypto_aol_put(const struct per_core_resource *pc_res,
	       struct crypto_aol *aol)
{
	struct crypto_aol *aol_next;

	while (aol) {
		aol_next = aol->ca_next ? sonic_phy_to_virt(aol->ca_next) :
					  NULL;
		pc_res_mpool_object_put(pc_res, MPOOL_TYPE_CRYPTO_AOL,
					(void *)aol);
		aol = aol_next;
	}
}

pnso_error_t
crypto_desc_status_convert(uint64_t status)
{
        pnso_error_t err = PNSO_OK;

	if (status & CRYPTO_LEN_NOT_MULTI_SECTORS)
		err = PNSO_ERR_CRYPTO_LEN_NOT_MULTI_SECTORS;
	else if (status)
		err = PNSO_ERR_CRYPTO_GENERAL_ERROR;

	return err;
}

