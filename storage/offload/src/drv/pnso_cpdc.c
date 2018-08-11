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
#define PNSO_NUM_OBJECTS		512
#define PNSO_NUM_OBJECTS_IN_OBJECT	16

struct mem_pool *cpdc_sgl_mpool;

static void
destroy_mpools(struct per_core_resource *pc_res)
{
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_SGL_BULK]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC_BULK]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_DESC_BULK]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_SERVICE_CHAIN_ENTRY]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_SERVICE_CHAIN]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_SGL]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_DESC]);
}

pnso_error_t
cpdc_init_accelerator(const struct cpdc_init_params *init_params,
		struct per_core_resource *pc_res)
{
	pnso_error_t err;
	struct mem_pool *mpool;
	uint32_t num_objects, num_object_set, object_size, pad_size;

	OSAL_LOG_INFO("enter ...");

	OSAL_ASSERT(init_params);
	OSAL_ASSERT(pc_res);

	/* TODO-cpdc: use init params */

	num_objects = PNSO_NUM_OBJECTS;
	err = mpool_create(MPOOL_TYPE_CPDC_DESC, num_objects,
			sizeof(struct cpdc_desc), PNSO_MEM_ALIGN_DESC, &mpool);
	if (err) {
		OSAL_LOG_ERROR("failed to create CPDC descriptor pool err: %d",
				err);
		goto out;
	}
	pc_res->mpools[MPOOL_TYPE_CPDC_DESC] = mpool;

	err = mpool_create(MPOOL_TYPE_CPDC_SGL, num_objects,
			sizeof(struct cpdc_sgl), PNSO_MEM_ALIGN_DESC, &mpool);
	if (err) {
		OSAL_LOG_ERROR("failed to create CPDC sgl pool err: %d",
				err);
		goto out_free_cpdc;
	}
	pc_res->mpools[MPOOL_TYPE_CPDC_SGL] = mpool;

	err = mpool_create(MPOOL_TYPE_CPDC_STATUS_DESC, num_objects,
			sizeof(struct cpdc_status_desc), PNSO_MEM_ALIGN_DESC,
			&mpool);
	if (err) {
		OSAL_LOG_ERROR("failed to create CPDC status descriptor pool err: %d",
				err);
		goto out_free_cpdc_sgl;
	}
	pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC] = mpool;

	/*
	 * following pools are for special type of objects i.e. set of objects
	 * to be in contiguous memory
	 *
	 */
	num_object_set = PNSO_NUM_OBJECTS;
	num_objects = PNSO_NUM_OBJECTS_IN_OBJECT;
	pad_size = mpool_get_pad_size(sizeof(struct cpdc_desc),
			PNSO_MEM_ALIGN_DESC);
	object_size = (sizeof(struct cpdc_desc) + pad_size) * num_objects;

	err = mpool_create(MPOOL_TYPE_CPDC_DESC_BULK, num_object_set,
			object_size, PNSO_MEM_ALIGN_DESC, &mpool);
	if (err) {
		OSAL_LOG_ERROR("failed to create CPDC descriptor bulk pool err: %d",
				err);
		goto out_free_cpdc_status;
	}
	pc_res->mpools[MPOOL_TYPE_CPDC_DESC_BULK] = mpool;

	pad_size = mpool_get_pad_size(sizeof(struct cpdc_status_desc),
			PNSO_MEM_ALIGN_DESC);
	object_size = (sizeof(struct cpdc_status_desc) +
			pad_size) * num_objects;

	err = mpool_create(MPOOL_TYPE_CPDC_STATUS_DESC_BULK, num_object_set,
			object_size, PNSO_MEM_ALIGN_DESC, &mpool);
	if (err) {
		OSAL_LOG_ERROR("failed to create CPDC status descriptor bulk pool err: %d",
				err);
		goto out_free_cpdc_bulk;
	}
	pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC_BULK] = mpool;

	pad_size = mpool_get_pad_size(sizeof(struct cpdc_sgl),
			PNSO_MEM_ALIGN_DESC);
	object_size = (sizeof(struct cpdc_sgl) +
			pad_size) * num_objects;

	mpool_type = MPOOL_TYPE_CPDC_SGL_BULK;
	err = mpool_create(mpool_type, num_object_set,
			object_size, PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CPDC_DESC]);
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CPDC_DESC_BULK]);
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CPDC_SGL]);
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CPDC_SGL_BULK]);
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC]);
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC_BULK]);

	OSAL_LOG_INFO("exit!");
	return err;

out_free_cpdc_bulk:
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_DESC_BULK]);
out_free_cpdc_status:
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC]);
out_free_cpdc_sgl:
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_SGL]);
out_free_cpdc:
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_DESC]);
out:
	OSAL_LOG_ERROR("exit!");
	return err;
}

void
cpdc_deinit_accelerator(struct per_core_resource *pc_res)
{
	OSAL_LOG_INFO("enter ...");

	OSAL_ASSERT(pc_res);

	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC_BULK]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_DESC_BULK]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_SGL]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_DESC]);

	OSAL_LOG_INFO("exit!");
}
