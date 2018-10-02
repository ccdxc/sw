/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "pnso_mpool.h"
#include "pnso_chain.h"
#include "pnso_crypto.h"
#include "sonic_api_int.h"

/*
 * TODO-crypto:
 *	- revisit ... only skeletons are added
 *
 */
#define PNSO_NUM_OBJECTS		8
#define PNSO_NUM_OBJECTS_IN_OBJECT	16

static void
deinit_mpools(struct per_core_resource *pc_res)
{
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CRYPTO_AOL_VECTOR]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CRYPTO_AOL]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CRYPTO_STATUS_DESC]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CRYPTO_DESC]);
}

static pnso_error_t
init_mpools(struct per_core_resource *pc_res)
{
	pnso_error_t err;
	uint32_t num_objects, num_object_set, object_size, pad_size;
	enum mem_pool_type mpool_type;

	OSAL_ASSERT(pc_res);

	num_objects = PNSO_NUM_OBJECTS;
	mpool_type = MPOOL_TYPE_CRYPTO_DESC;
	err = mpool_create(mpool_type, num_objects,
			sizeof(struct crypto_desc), PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_CRYPTO_STATUS_DESC;
	err = mpool_create(mpool_type, num_objects,
			sizeof(struct crypto_status_desc),
			sizeof(struct crypto_status_desc),
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_CRYPTO_AOL;
	err = mpool_create(mpool_type, num_objects,
			sizeof(struct crypto_aol), PNSO_MEM_ALIGN_AOL,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	/*
	 * following pools are for special type of objects
	 * i.e. set of objects to be in contiguous memory
	 *
	 */
	num_object_set = PNSO_NUM_OBJECTS;
	num_objects = PNSO_NUM_OBJECTS_IN_OBJECT;
	
	pad_size = mpool_get_pad_size(sizeof(struct crypto_aol),
			PNSO_MEM_ALIGN_DESC);
	object_size = (sizeof(struct crypto_aol) +
			pad_size) * num_objects;

	mpool_type = MPOOL_TYPE_CRYPTO_AOL_VECTOR;
	err = mpool_create(mpool_type, num_object_set,
			object_size, PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CRYPTO_DESC]);
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CRYPTO_STATUS_DESC]);
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CRYPTO_AOL]);
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CRYPTO_AOL_VECTOR]);

	return PNSO_OK;

out:
	OSAL_LOG_ERROR("failed to allocate pool! mpool_type: %d err: %d",
			mpool_type, err);
	return err;
}

pnso_error_t
crypto_init_accelerator(const struct crypto_init_params *init_params,
		struct per_core_resource *pc_res)
{
	pnso_error_t err;

	OSAL_LOG_INFO("enter ...");

	OSAL_ASSERT(init_params);
	OSAL_ASSERT(pc_res);

	/* TODO-crypto: use init params */

	err = init_mpools(pc_res);
	if (err)
		goto out_mpools;

	/* TODO-crypto: additional initializations */

	OSAL_LOG_DEBUG("exit!");
	return err;

out_mpools:
	deinit_mpools(pc_res);

	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

void
crypto_deinit_accelerator(struct per_core_resource *pc_res)
{
	OSAL_LOG_INFO("enter ...");

	OSAL_ASSERT(pc_res);

	deinit_mpools(pc_res);

	OSAL_LOG_INFO("exit!");
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
