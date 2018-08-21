/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "pnso_api.h"

#include "pnso_mpool.h"
#include "pnso_chain.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"

/*
 * TODO-cpdc:
 *	- stitch batch/init params for PNSO_NUM_OBJECTS during pool creation
 *
 */
#define PNSO_NUM_OBJECTS		8
#define PNSO_NUM_OBJECTS_IN_OBJECT	16

static void
deinit_mpools(struct per_core_resource *pc_res)
{
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_SGL_VECTOR]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC_VECTOR]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_DESC_VECTOR]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_SERVICE_CHAIN_ENTRY]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_SERVICE_CHAIN]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_SGL]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_DESC]);
}

static pnso_error_t
init_mpools(struct per_core_resource *pc_res)
{
	pnso_error_t err;
	uint32_t num_objects, num_object_set, object_size, pad_size;
	enum mem_pool_type mpool_type;

	OSAL_LOG_INFO("enter ...");

	OSAL_ASSERT(pc_res);

	num_objects = PNSO_NUM_OBJECTS;
	mpool_type = MPOOL_TYPE_CPDC_DESC;
	err = mpool_create(mpool_type, num_objects,
			sizeof(struct cpdc_desc), PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_CPDC_SGL;
	err = mpool_create(mpool_type, num_objects,
			sizeof(struct cpdc_sgl), PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_CPDC_STATUS_DESC;
	err = mpool_create(mpool_type, num_objects,
			sizeof(struct cpdc_status_desc), PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_SERVICE_CHAIN;
	err = mpool_create(mpool_type, num_objects,
			sizeof(struct service_chain), PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_SERVICE_CHAIN_ENTRY;
	err = mpool_create(mpool_type, num_objects,
			sizeof(struct chain_entry), PNSO_MEM_ALIGN_DESC,
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
	pad_size = mpool_get_pad_size(sizeof(struct cpdc_desc),
			PNSO_MEM_ALIGN_DESC);
	object_size = (sizeof(struct cpdc_desc) + pad_size) * num_objects;

	mpool_type = MPOOL_TYPE_CPDC_DESC_VECTOR;
	err = mpool_create(mpool_type, num_object_set,
			object_size, PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	pad_size = mpool_get_pad_size(sizeof(struct cpdc_status_desc),
			PNSO_MEM_ALIGN_DESC);
	object_size = (sizeof(struct cpdc_status_desc) +
			pad_size) * num_objects;

	mpool_type = MPOOL_TYPE_CPDC_STATUS_DESC_VECTOR;
	err = mpool_create(mpool_type, num_object_set,
			object_size, PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	pad_size = mpool_get_pad_size(sizeof(struct cpdc_sgl),
			PNSO_MEM_ALIGN_DESC);
	object_size = (sizeof(struct cpdc_sgl) +
			pad_size) * num_objects;

	mpool_type = MPOOL_TYPE_CPDC_SGL_VECTOR;
	err = mpool_create(mpool_type, num_object_set,
			object_size, PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CPDC_DESC]);
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CPDC_DESC_VECTOR]);
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CPDC_SGL]);
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CPDC_SGL_VECTOR]);
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC]);
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC_VECTOR]);
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_SERVICE_CHAIN]);
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_SERVICE_CHAIN_ENTRY]);

	return PNSO_OK;

out:
	OSAL_LOG_ERROR("failed to allocate pool! mpool_type: %d err: %d",
			mpool_type, err);
	return err;
}

pnso_error_t
cpdc_init_accelerator(const struct cpdc_init_params *init_params,
		struct per_core_resource *pc_res)
{
	pnso_error_t err;

	OSAL_LOG_INFO("enter ...");

	OSAL_ASSERT(init_params);
	OSAL_ASSERT(pc_res);

	/* TODO-cpdc: use init params */

	err = init_mpools(pc_res);
	if (err)
		goto out_mpools;

	OSAL_LOG_INFO("exit!");
	return err;

out_mpools:
	deinit_mpools(pc_res);

	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

void
cpdc_deinit_accelerator(struct per_core_resource *pc_res)
{
	OSAL_LOG_INFO("enter ...");

	OSAL_ASSERT(pc_res);

	deinit_mpools(pc_res);

	OSAL_LOG_INFO("exit!");
}
