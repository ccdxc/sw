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
 *	- rename/revisit 'objects in object'
 *
 */
#define PNSO_NUM_OBJECTS		8
#define PNSO_NUM_OBJECTS_IN_OBJECT	16

uint64_t pad_buffer;

static pnso_error_t
alloc_pad_buffer(void)
{
	pnso_error_t err = EINVAL;

	pad_buffer = osal_rmem_aligned_alloc(PNSO_MEM_ALIGN_PAGE,
			PNSO_MEM_ALIGN_PAGE);
	if (pad_buffer <= 0) {
		OSAL_LOG_ERROR("failed to allocate static pad buffer! err: %d",
				err);
		goto out;
	}
	/* TODO-chain: include rmem calloc/set fix that is in-flight */
	// memset((void *) pad_buffer, 0, PNSO_MEM_ALIGN_PAGE);

	err = PNSO_OK;
	OSAL_LOG_ERROR("aligned pad buffer allocated and initialized! pad_buffer: 0x%llx",
			pad_buffer);
out:
	return err;
}

static inline void
free_pad_buffer(uint64_t pad_buffer)
{
	osal_rmem_free(pad_buffer, PNSO_MEM_ALIGN_PAGE);
}

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

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(init_params);
	OSAL_ASSERT(pc_res);

	/* TODO-cpdc: use init params */

	err = init_mpools(pc_res);
	if (err)
		goto out_mpools;

	err = alloc_pad_buffer();
	if (err)
		goto out_mpools;

	OSAL_LOG_DEBUG("exit!");
	return err;

out_mpools:
	deinit_mpools(pc_res);

	OSAL_LOG_ERROR("exit! err: %d", err);
	return err;
}

void
cpdc_deinit_accelerator(struct per_core_resource *pc_res)
{
	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(pc_res);

	free_pad_buffer(pad_buffer);

	deinit_mpools(pc_res);

	OSAL_LOG_DEBUG("exit!");
}
