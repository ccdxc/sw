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
#include "pnso_batch.h"
#include "pnso_chain.h"
#include "pnso_crypto.h"
#include "pnso_utils.h"
#include "pnso_crypto_cmn.h"
#include "pnso_cpdc.h"
#include "pnso_seq.h"

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
crypto_aol_packed_get(const struct per_core_resource *pcr,
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
		aol = pc_res_mpool_object_get(pcr, svc_aol->mpool_type);
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
			pc_res_mpool_object_put(pcr, svc_aol->mpool_type, aol);
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
	crypto_aol_put(pcr, svc_aol);
	return err;
}

pnso_error_t
crypto_aol_vec_sparse_get(const struct per_core_resource *pcr,
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
	svc_aol->aol = pc_res_mpool_object_get_with_num_vec_elems(pcr,
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
	crypto_aol_put(pcr, svc_aol);
	return err;
}

void
crypto_aol_put(const struct per_core_resource *pcr,
	       struct service_crypto_aol *svc_aol)
{
	struct crypto_aol *aol_next;
	struct crypto_aol *aol;

	aol = svc_aol->aol;
	while (aol) {
		aol_next = aol->ca_next ? sonic_phy_to_virt(aol->ca_next) :
					  NULL;
		pc_res_mpool_object_put(pcr, svc_aol->mpool_type, aol);
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

pnso_error_t
crypto_setup_batch_desc(struct service_info *svc_info, struct crypto_desc *desc)
{
	struct service_batch_info *svc_batch_info;
	uint32_t batch_size, remaining;

	svc_batch_info = &svc_info->si_batch_info;
	OSAL_ASSERT(svc_batch_info->sbi_num_entries);

	if (svc_batch_info->sbi_desc_idx != 0) {
		OSAL_LOG_DEBUG("sequencer setup not needed!");
		return PNSO_OK;
	}

	remaining = svc_batch_info->sbi_num_entries -
		(svc_batch_info->sbi_bulk_desc_idx * MAX_PAGE_ENTRIES);
	batch_size = (remaining / MAX_PAGE_ENTRIES) ? MAX_PAGE_ENTRIES :
		remaining;

	/* indicate batch processing only for 1st entry in the batch */
	svc_info->si_seq_info.sqi_batch_mode = true;
	svc_info->si_seq_info.sqi_batch_size = batch_size;

	svc_info->si_seq_info.sqi_desc = seq_setup_desc(svc_info,
			desc, sizeof(*desc));
	if (!svc_info->si_seq_info.sqi_desc) {
		OSAL_LOG_ERROR("failed to setup sequencer desc!");
		return EINVAL;
	}

	return PNSO_OK;
}

static struct crypto_desc *
get_desc(struct per_core_resource *pcr, bool per_block)
{
	struct mem_pool *mpool;

	mpool = per_block ? pcr->mpools[MPOOL_TYPE_CRYPTO_DESC_VECTOR] :
		pcr->mpools[MPOOL_TYPE_CRYPTO_DESC];

	return (struct crypto_desc *) mpool_get_object(mpool);
}

static struct crypto_desc *
get_batch_desc(struct service_info *svc_info)
{
	struct service_batch_info *svc_batch_info;
	struct crypto_desc *desc;

	svc_batch_info = &svc_info->si_batch_info;
	desc = &svc_batch_info->u.sbi_crypto_desc[svc_batch_info->sbi_desc_idx];

	OSAL_LOG_DEBUG("num_entries: %d desc_idx: %d bulk_desc: 0x" PRIx64 " desc: 0x" PRIx64 "",
			svc_batch_info->sbi_num_entries,
			svc_batch_info->sbi_desc_idx,
			(uint64_t) svc_batch_info->u.sbi_crypto_desc,
			(uint64_t) desc);
	return desc;
}

struct crypto_desc *
crypto_get_desc(struct service_info *svc_info, bool per_block)
{
	struct crypto_desc *desc;
	bool in_batch = false;

	if (putil_is_service_in_batch(svc_info->si_flags))
		in_batch = true;

	desc = in_batch ? get_batch_desc(svc_info) :
		get_desc(svc_info->si_pcr, per_block);

	OSAL_ASSERT(desc);
	return desc;
}

/* 'batch' is the caller, not the services ... */
struct crypto_desc *
crypto_get_batch_bulk_desc(struct mem_pool *mpool)
{
	struct crypto_desc *desc;

	desc = (struct crypto_desc *) mpool_get_object(mpool);
	if (!desc) {
		OSAL_LOG_ERROR("cannot obtain crypto bulk object from pool!");
		return NULL;
	}

	return desc;
}

static void
put_desc(struct per_core_resource *pcr, bool per_block,
		struct crypto_desc *desc)
{
	struct mem_pool *mpool;

	mpool = per_block ? pcr->mpools[MPOOL_TYPE_CRYPTO_DESC_VECTOR] :
		pcr->mpools[MPOOL_TYPE_CRYPTO_DESC];

	mpool_put_object(mpool, desc);
}

static void
put_batch_desc(const struct service_info *svc_info,
		struct crypto_desc *desc)
{
	struct service_batch_info *svc_batch_info;

	svc_batch_info = (struct service_batch_info *) &svc_info->si_batch_info;

	/* do nothing */

	OSAL_LOG_DEBUG("num_entries: %d desc_idx: %d bulk_desc: 0x" PRIx64 " desc: 0x" PRIx64 "",
			svc_batch_info->sbi_num_entries,
			svc_batch_info->sbi_desc_idx,
			(uint64_t) svc_batch_info->u.sbi_crypto_desc,
			(uint64_t) desc);
}

void
crypto_put_desc(const struct service_info *svc_info, bool per_block,
		struct crypto_desc *desc)
{
	bool in_batch = false;

	if (putil_is_service_in_batch(svc_info->si_flags))
		in_batch = true;

	if (in_batch)
		put_batch_desc(svc_info, desc);
	else
		put_desc(svc_info->si_pcr, per_block, desc);
}

/* 'batch' is the caller, not the services ... */
void
crypto_put_batch_bulk_desc(struct mem_pool *mpool, struct crypto_desc *desc)
{
	return mpool_put_object(mpool, desc);
}

pnso_error_t
crypto_setup_seq_desc(struct service_info *svc_info, struct crypto_desc *desc)
{
	pnso_error_t err = EINVAL;
	uint8_t	flags;

	if (putil_is_service_in_batch(svc_info->si_flags)) {
		err = crypto_setup_batch_desc(svc_info, desc);
		if (err)
			OSAL_LOG_ERROR("failed to setup batch sequencer desc! err: %d",
					err);
		goto out;
	}

	flags = svc_info->si_flags;
	if ((flags & CHAIN_SFLAG_LONE_SERVICE) ||
			(flags & CHAIN_SFLAG_FIRST_SERVICE)) {
		svc_info->si_seq_info.sqi_desc = seq_setup_desc(svc_info,
				desc, sizeof(struct crypto_desc));
		if (!svc_info->si_seq_info.sqi_desc) {
			OSAL_LOG_ERROR("failed to setup sequencer desc! flags: %d err: %d",
						flags, err);
			goto out;
		}

		err = PNSO_OK;
	}

out:
	return err;
}
