/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "pnso_mpool.h"
#include "pnso_chain.h"
#include "pnso_xts.h"

/*
 * TODO-xts:
 *	- revisit ... only skeletons are added
 *
 */
#define PNSO_NUM_OBJECTS		8
#define PNSO_NUM_OBJECTS_IN_OBJECT	16

static void
deinit_mpools(struct per_core_resource *pc_res)
{
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_XTS_AOL_VECTOR]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_XTS_AOL]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_XTS_DESC]);
}

static pnso_error_t
init_mpools(struct per_core_resource *pc_res)
{
	pnso_error_t err;
	uint32_t num_objects, num_object_set, object_size, pad_size;
	enum mem_pool_type mpool_type;

	OSAL_ASSERT(pc_res);

	num_objects = PNSO_NUM_OBJECTS;
	mpool_type = MPOOL_TYPE_XTS_DESC;
	err = mpool_create(mpool_type, num_objects,
			sizeof(struct xts_desc), PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_XTS_AOL;
	err = mpool_create(mpool_type, num_objects,
			sizeof(struct xts_aol), PNSO_MEM_ALIGN_DESC,
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
	
	pad_size = mpool_get_pad_size(sizeof(struct xts_aol),
			PNSO_MEM_ALIGN_DESC);
	object_size = (sizeof(struct xts_aol) +
			pad_size) * num_objects;

	mpool_type = MPOOL_TYPE_XTS_AOL_VECTOR;
	err = mpool_create(mpool_type, num_object_set,
			object_size, PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_XTS_DESC]);
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_XTS_AOL]);
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_XTS_AOL_VECTOR]);

	return PNSO_OK;

out:
	OSAL_LOG_ERROR("failed to allocate pool! mpool_type: %d err: %d",
			mpool_type, err);
	return err;
}

pnso_error_t
xts_init_accelerator(const struct xts_init_params *init_params,
		struct per_core_resource *pc_res)
{
	pnso_error_t err;

	OSAL_LOG_INFO("enter ...");

	OSAL_ASSERT(init_params);
	OSAL_ASSERT(pc_res);

	/* TODO-xts: use init params */

	err = init_mpools(pc_res);
	if (err)
		goto out_mpools;

	/* TODO-xts: additional initializations */

	OSAL_LOG_DEBUG("exit!");
	return err;

out_mpools:
	deinit_mpools(pc_res);

	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

void
xts_deinit_accelerator(struct per_core_resource *pc_res)
{
	OSAL_LOG_INFO("enter ...");

	OSAL_ASSERT(pc_res);

	deinit_mpools(pc_res);

	OSAL_LOG_INFO("exit!");
}
