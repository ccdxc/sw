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

#define CRYPTO_NUM_DESCS_PER_AOL_VEC	PNSO_NOMINAL_NUM_BUFS
#define CRYPTO_NUM_DESCS_PER_SGL_VEC	PNSO_NOMINAL_NUM_BUFS

static void
deinit_mpools(struct per_core_resource *pc_res)
{
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_RMEM_INTERM_CRYPTO_STATUS]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CRYPTO_SGL_VECTOR]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CRYPTO_AOL_VECTOR]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CRYPTO_AOL]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CRYPTO_STATUS_DESC]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CRYPTO_DESC]);
}

static pnso_error_t
init_mpools(struct pc_res_init_params *pc_init,
	    struct per_core_resource *pc_res)
{
	uint32_t object_size;
	pnso_error_t err;

	err = mpool_create(MPOOL_TYPE_CRYPTO_DESC, pc_init->max_seq_sq_descs,
				sizeof(struct crypto_desc), PNSO_MEM_ALIGN_DESC,
				&pc_res->mpools[MPOOL_TYPE_CRYPTO_DESC]);
	if (!err)
		err = mpool_create(MPOOL_TYPE_CRYPTO_STATUS_DESC,
				pc_init->max_seq_sq_descs,
				sizeof(struct crypto_status_desc),
				sizeof(struct crypto_status_desc),
				&pc_res->mpools[MPOOL_TYPE_CRYPTO_STATUS_DESC]);
	if (!err)
		err = mpool_create(MPOOL_TYPE_CRYPTO_AOL,
				pc_init->max_seq_sq_descs,
				sizeof(struct crypto_aol), PNSO_MEM_ALIGN_DESC,
				&pc_res->mpools[MPOOL_TYPE_CRYPTO_AOL]);
	if (!err) {
		object_size = sizeof(struct crypto_aol) *
			      CRYPTO_NUM_DESCS_PER_AOL_VEC;

		err = mpool_create(MPOOL_TYPE_CRYPTO_AOL_VECTOR,
				pc_init->max_seq_sq_descs,
				object_size, PNSO_MEM_ALIGN_DESC,
				&pc_res->mpools[MPOOL_TYPE_CRYPTO_AOL_VECTOR]);
	}

	if (!err) {
		object_size = sizeof(struct cpdc_sgl) *
			      CRYPTO_NUM_DESCS_PER_SGL_VEC;

		err = mpool_create(MPOOL_TYPE_CRYPTO_SGL_VECTOR,
				pc_init->max_seq_sq_descs,
				object_size, PNSO_MEM_ALIGN_DESC,
				&pc_res->mpools[MPOOL_TYPE_CRYPTO_SGL_VECTOR]);
	}

	if (!err)
		err = mpool_create(MPOOL_TYPE_RMEM_INTERM_CRYPTO_STATUS,
				pc_init->max_seq_sq_descs,
				sizeof(struct crypto_status_desc),
				sizeof(struct crypto_status_desc),
				&pc_res->mpools[MPOOL_TYPE_RMEM_INTERM_CRYPTO_STATUS]);
	if (!err) {
		MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CRYPTO_DESC]);
		MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CRYPTO_STATUS_DESC]);
		MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CRYPTO_AOL]);
		MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CRYPTO_AOL_VECTOR]);
		MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CRYPTO_SGL_VECTOR]);
		MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_RMEM_INTERM_CRYPTO_STATUS]);
	}
	return PNSO_OK;
}

pnso_error_t
crypto_init_accelerator(struct pc_res_init_params *pc_init,
		struct per_core_resource *pc_res)
{
	pnso_error_t err;

	err = init_mpools(pc_init, pc_res);
	if (err)
		deinit_mpools(pc_res);
	return err;
}

void
crypto_deinit_accelerator(struct per_core_resource *pc_res)
{
	deinit_mpools(pc_res);
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
