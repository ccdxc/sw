/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"
#include "pnso_init.h"

#include "pnso_mpool.h"
#include "pnso_chain.h"
#include "pnso_crypto.h"
#include "pnso_cpdc.h"
#include "sonic_api_int.h"

#define CRYPTO_NUM_DESCS_PER_AOL_VEC	INTERM_BUF_MAX_NUM_BUFS
#define CRYPTO_NUM_DESCS_PER_SGL_VEC	INTERM_BUF_MAX_NUM_BUFS

#define PNSO_NUM_OBJECTS_IN_OBJECT	16

static void
deinit_mpools(struct per_core_resource *pcr)
{
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_CRYPTO_AOL_VECTOR]);
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_CRYPTO_DESC_VECTOR]);
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_RMEM_INTERM_CRYPTO_STATUS]);
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_CRYPTO_AOL]);
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_CRYPTO_STATUS_DESC]);
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_CRYPTO_DESC]);
}

static pnso_error_t
init_mpools(struct pc_res_init_params *pc_init,
		struct per_core_resource *pcr)
{
	pnso_error_t err;
	uint32_t num_objects;
	enum mem_pool_type mpool_type;

	num_objects = pc_init->max_seq_sq_descs;

	mpool_type = MPOOL_TYPE_CRYPTO_DESC;
	err = mpool_create(mpool_type, num_objects, MPOOL_VEC_ELEM_SINGLE,
				sizeof(struct crypto_desc), PNSO_MEM_ALIGN_DESC,
				&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_CRYPTO_STATUS_DESC;
	err = mpool_create(mpool_type, num_objects, MPOOL_VEC_ELEM_SINGLE,
				sizeof(struct crypto_status_desc),
				sizeof(struct crypto_status_desc),
				&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_CRYPTO_AOL;
	err = mpool_create(mpool_type, num_objects * MAX_CRYPTO_SGLS_PER_REQ,
			MPOOL_VEC_ELEM_SINGLE,
			sizeof(struct crypto_aol), PNSO_MEM_ALIGN_DESC,
			&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_RMEM_INTERM_CRYPTO_STATUS;
	err = mpool_create(mpool_type, num_objects, MPOOL_VEC_ELEM_SINGLE,
			sizeof(struct crypto_status_desc),
			sizeof(struct crypto_status_desc),
			&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	/*
	 * following pools are for special type of objects
	 * i.e. set of objects to be in contiguous memory
	 *
	 */
	mpool_type = MPOOL_TYPE_CRYPTO_DESC_VECTOR;
	err = mpool_create(mpool_type, num_objects *
			MAX_CRYPTO_DESC_VEC_PER_REQ,
			PNSO_NUM_OBJECTS_IN_OBJECT,
			sizeof(struct crypto_desc), PNSO_MEM_ALIGN_DESC,
			&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_CRYPTO_AOL_VECTOR;
	err = mpool_create(mpool_type, num_objects * MAX_CRYPTO_SGL_VEC_PER_REQ,
			CRYPTO_NUM_DESCS_PER_AOL_VEC,
			sizeof(struct crypto_aol), PNSO_MEM_ALIGN_DESC,
			&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CRYPTO_DESC]);
	MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CRYPTO_STATUS_DESC]);
	MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CRYPTO_AOL]);
	MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_RMEM_INTERM_CRYPTO_STATUS]);
	MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CRYPTO_DESC_VECTOR]);
	MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CRYPTO_AOL_VECTOR]);

	return PNSO_OK;

out:
	OSAL_LOG_ERROR("failed to allocate pool! mpool_type: %d num_objects: %d err: %d",
			mpool_type, num_objects, err);
	return err;
}

pnso_error_t
crypto_init_accelerator(struct pc_res_init_params *pc_init,
		struct per_core_resource *pcr)
{
	pnso_error_t err;

	OSAL_LOG_DEBUG("enter ...");

	err = init_mpools(pc_init, pcr);
	if (err)
		goto out_mpools;

	OSAL_LOG_DEBUG("exit!");
	return err;

out_mpools:
	deinit_mpools(pcr);

	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

void
crypto_deinit_accelerator(struct per_core_resource *pcr)
{
	OSAL_LOG_DEBUG("enter ...");

	deinit_mpools(pcr);

	OSAL_LOG_DEBUG("exit!");
}

pnso_error_t
crypto_key_index_update(const void *key1,
			const void *key2,
			uint32_t key_size,
			uint32_t key_idx)
{
	return sonic_crypto_key_index_update(key1, key2, key_size,
					     sonic_get_crypto_key_idx(key_idx));
}
