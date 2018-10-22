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

	OSAL_LOG_DEBUG("%30s: 0x"PRIx64" ==> 0x"PRIx64, "", (uint64_t) aol, aol_pa);
	while (aol) {
		OSAL_LOG_DEBUG("%30s: 0x"PRIx64"/%d/%d 0x"PRIx64"/%d/%d 0x"PRIx64
				"/%d/%d", "",
				aol->ca_addr_0, aol->ca_off_0, aol->ca_len_0,
				aol->ca_addr_1, aol->ca_off_1, aol->ca_len_1,
				aol->ca_addr_2, aol->ca_off_2, aol->ca_len_2);
		OSAL_LOG_DEBUG("%30s: 0x"PRIx64"/0x"PRIx64, "",
				aol->ca_next, aol->ca_rsvd);

		aol = aol->ca_next ? sonic_phy_to_virt(aol->ca_next) : NULL;
	}
}

static void
crypto_pprint_cmd(const struct crypto_cmd *cmd)
{
	if (!cmd)
		return;

	OSAL_LOG_DEBUG("%30s: %d", "cc_enable_crc", cmd->cc_enable_crc);
	OSAL_LOG_DEBUG("%30s: %d", "cc_bypass_aes", cmd->cc_bypass_aes);
	OSAL_LOG_DEBUG("%30s: %d", "cc_is_decrypt", cmd->cc_is_decrypt);
	OSAL_LOG_DEBUG("%30s: %d", "cc_token_3", cmd->cc_token_3);
	OSAL_LOG_DEBUG("%30s: %d", "cc_token_4", cmd->cc_token_4);
}

void
crypto_pprint_desc(const struct crypto_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_DEBUG("%30s: 0x"PRIx64, "crypto_desc", (uint64_t) desc);

	OSAL_LOG_DEBUG("%30s: 0x"PRIx64, "cd_src", desc->cd_in_aol);
	OSAL_LOG_DEBUG("%30s: 0x"PRIx64, "cd_dst", desc->cd_out_aol);

	OSAL_LOG_DEBUG("%30s: 0x"PRIx64, "=== crypto_cmd", (uint64_t)&desc->cd_cmd);
	crypto_pprint_cmd(&desc->cd_cmd);

	OSAL_LOG_DEBUG("%30s: %d", "cd_key_desc_idx", desc->cd_key_desc_idx);
	OSAL_LOG_DEBUG("%30s: 0x"PRIx64, "cd_iv_addr", desc->cd_iv_addr);

	OSAL_LOG_DEBUG("%30s: 0x"PRIx64, "cd_auth_tag", desc->cd_auth_tag);

	OSAL_LOG_DEBUG("%30s: %d", "cd_hdr_size", desc->cd_hdr_size);
	OSAL_LOG_DEBUG("%30s: 0x"PRIx64, "cd_status_addr", desc->cd_status_addr);

	OSAL_LOG_DEBUG("%30s: %d", "cd_otag", desc->cd_otag);
	OSAL_LOG_DEBUG("%30s: %d", "cd_otag_on", desc->cd_otag_on);

	OSAL_LOG_DEBUG("%30s: %d", "cd_sector_size", desc->cd_sector_size);

	OSAL_LOG_DEBUG("%30s: %d", "cd_app_tag", desc->cd_app_tag);
	OSAL_LOG_DEBUG("%30s: %d", "cd_sector_num", desc->cd_sector_num);

	OSAL_LOG_DEBUG("%30s: 0x"PRIx64, "cd_db_addr", desc->cd_db_addr);
	OSAL_LOG_DEBUG("%30s: 0x"PRIx64, "cd_db_data", desc->cd_db_data);

	OSAL_LOG_DEBUG("%30s: 0x"PRIx64, "=== cd_in_aol", desc->cd_in_aol);
	crypto_pprint_aol(desc->cd_in_aol);

	OSAL_LOG_DEBUG("%30s: 0x"PRIx64, "=== cd_out_aol", desc->cd_out_aol);
	crypto_pprint_aol(desc->cd_out_aol);
}

pnso_error_t
crypto_aol_packed_get(const struct per_core_resource *pc_res,
		      const struct service_buf_list *svc_blist,
		      struct service_crypto_aol *svc_aol)
{
	struct buffer_list_iter buffer_list_iter;
	struct buffer_list_iter *iter;
	struct crypto_aol *aol_prev = NULL;
	struct crypto_aol *aol;
	uint32_t total_len;
	pnso_error_t err = ENOMEM;

	iter = buffer_list_iter_init(&buffer_list_iter, svc_blist);
	svc_aol->mpool_type = MPOOL_TYPE_CRYPTO_AOL;
	svc_aol->aol = NULL;
	total_len = 0;
	while (iter) {
		aol = pc_res_mpool_object_get(pc_res, svc_aol->mpool_type);
		if (!aol) {
			OSAL_LOG_ERROR("cannot obtain crypto aol_vec from pool, current_len %u",
				       total_len);
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
		/*
		 * Crypto requires ca_addr_0 to be populated for the entire AOL
		 * to be valid. If it contains zero, then not only that we've
		 * reached the end of the buffer list, we must exclude this AOL.
		 * This could happen if the app's buffer list contains some
		 * zero length at the end of the list. For example:
		 * count=4, buf0/len0>0, buf1/len1>0, buf2/len2>0, buf3/len3=0.
		 * Note that list iterator would automatically have trimmed any
		 * intervening zero-length buffers in the list.
		 */
		if (!aol->ca_len_0) {
			pc_res_mpool_object_put(pc_res, svc_aol->mpool_type, aol);
			break;
		}

		total_len += aol->ca_len_0 + aol->ca_len_1 + aol->ca_len_2;
		if (!svc_aol->aol)
			svc_aol->aol = aol;
		else
			aol_prev->ca_next = sonic_virt_to_phy(aol);
		aol_prev = aol;
	}

	/*
	 * Caller must have ensured that svc_blist had non-zero length to begin with.
	 */
	if (!total_len) {
		OSAL_LOG_ERROR("buffer_list is empty");
		err = EINVAL;
		goto out;
	}
	return PNSO_OK;
out:
	crypto_aol_put(pc_res, svc_aol);
	return err;
}

pnso_error_t
crypto_aol_vec_sparse_get(const struct per_core_resource *pc_res,
			  uint32_t block_size,
			  const struct service_buf_list *svc_blist,
			  struct service_crypto_aol *svc_aol)
{
	struct buffer_list_iter buffer_list_iter;
	struct buffer_list_iter *iter;
	struct crypto_aol *aol_prev = NULL;
	struct crypto_aol *aol_vec;
	uint32_t num_vec_elems;
	uint32_t cur_count;
	uint32_t total_len;
	pnso_error_t err = ENOMEM;

	OSAL_ASSERT(is_power_of_2(block_size));
	svc_aol->mpool_type = MPOOL_TYPE_CRYPTO_AOL_VECTOR;
	svc_aol->aol = pc_res_mpool_object_get_with_num_vec_elems(pc_res,
				svc_aol->mpool_type, &num_vec_elems);
	if (!svc_aol->aol) {
		OSAL_LOG_ERROR("cannot obtain crypto aol_vec from pool");
		goto out;
	}

	iter = buffer_list_iter_init(&buffer_list_iter, svc_blist);
	aol_vec = svc_aol->aol;
	total_len = 0;
	cur_count = 0;
	while (iter && (cur_count < num_vec_elems)) {
		memset(aol_vec, 0, sizeof(*aol_vec));
		iter = buffer_list_iter_addr_len_get(iter, block_size,
					&aol_vec->ca_addr_0, &aol_vec->ca_len_0);
		/*
		 * Crypto requires ca_addr_0 to be populated for the entire AOL
		 * to be valid. If it contains zero, then not only that we've
		 * reached the end of the buffer list, we must exclude this AOL.
		 * This could happen if the app's buffer list contains some
		 * zero length at the end of the list. For example:
		 * count=4, buf0/len0>0, buf1/len1>0, buf2/len2>0, buf3/len3=0.
		 * Note that list iterator would automatically have trimmed any
		 * intervening zero-length buffers in the list.
		 */
		if (!aol_vec->ca_len_0)
			break;

		total_len += aol_vec->ca_len_0;

		/*
		 * For padding purposes, every element of a sparse AOL
		 * must be a full block size.
		 */
		if (aol_vec->ca_len_0 != block_size) {
			OSAL_LOG_ERROR("Sparse AOL fails to make full block_size %u",
					block_size);
			err = EINVAL;
			goto out;
		}

		if (aol_prev)
			aol_prev->ca_next = sonic_virt_to_phy(aol_vec);

		aol_prev = aol_vec++;
		cur_count++;
	}

	if (iter) {
		OSAL_LOG_ERROR("buffer_list total length exceeds AOL vector, current_len %u",
			       total_len);
		err = EINVAL;
		goto out;
	}

	/*
	 * Caller must have ensured that svc_blist had non-zero length to begin with.
	 */
	if (!total_len) {
		OSAL_LOG_ERROR("buffer_list is empty");
		err = EINVAL;
		goto out;
	}
	return PNSO_OK;
out:
	crypto_aol_put(pc_res, svc_aol);
	return err;
}

void
crypto_aol_put(const struct per_core_resource *pc_res,
	       struct service_crypto_aol *svc_aol)
{
	struct crypto_aol *aol_next;
	struct crypto_aol *aol;

	aol = svc_aol->aol;
	while (aol) {
		aol_next = aol->ca_next ? sonic_phy_to_virt(aol->ca_next) :
					  NULL;
		pc_res_mpool_object_put(pc_res, svc_aol->mpool_type, aol);
		aol = aol_next;
	}
	svc_aol->aol = NULL;
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

