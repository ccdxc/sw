/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "osal_rmem.h"
#include "pnso_api.h"

#include "pnso_mpool.h"
#include "pnso_batch.h"
#include "pnso_chain.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"

/*
 * TODO-cpdc:
 *	- stitch batch/init params for PNSO_NUM_OBJECTS during pool creation
 *	- rename/revisit 'objects in object'
 *	- move init/deinit of the following out of CPDC:
 *		MPOOL_TYPE_SERVICE_CHAIN*, MPOOL_TYPE_BATCH_*
 *	- size SGL -- MPOOL_TYPE_SGL needs 4 times DESC/DESC_VEC
 *
 */
#define PNSO_NUM_OBJECTS		128
#define PNSO_NUM_OBJECTS_IN_OBJECT	16

static void
deinit_mpools(struct per_core_resource *pc_res)
{
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_SGL_VECTOR]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC_VECTOR]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_DESC_VECTOR]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_BATCH_INFO]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_BATCH_PAGE]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_SERVICE_CHAIN_ENTRY]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_SERVICE_CHAIN]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_STATUS_DESC]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_SGL]);
	mpool_destroy(&pc_res->mpools[MPOOL_TYPE_CPDC_DESC]);
}

static pnso_error_t
init_mpools(struct pc_res_init_params *pc_init,
	    struct per_core_resource *pc_res)
{
	pnso_error_t err;
	uint32_t num_objects, num_object_set, object_size;
	enum mem_pool_type mpool_type;

	OSAL_ASSERT(pc_res);

	num_objects = pc_init->max_seq_sq_descs;
	mpool_type = MPOOL_TYPE_CPDC_DESC;
	err = mpool_create(mpool_type, num_objects, MPOOL_VEC_ELEM_SINGLE,
			sizeof(struct cpdc_desc), PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_CPDC_SGL;
	err = mpool_create(mpool_type, num_objects, MPOOL_VEC_ELEM_SINGLE,
			sizeof(struct cpdc_sgl), PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_CPDC_STATUS_DESC;
	err = mpool_create(mpool_type, num_objects, MPOOL_VEC_ELEM_SINGLE,
			sizeof(struct cpdc_status_desc), PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_SERVICE_CHAIN;
	err = mpool_create(mpool_type, num_objects, MPOOL_VEC_ELEM_SINGLE,
			sizeof(struct service_chain), PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_SERVICE_CHAIN_ENTRY;
	err = mpool_create(mpool_type, num_objects, MPOOL_VEC_ELEM_SINGLE,
			sizeof(struct chain_entry), PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_BATCH_PAGE;
	err = mpool_create(mpool_type, num_objects, MPOOL_VEC_ELEM_SINGLE,
			sizeof(struct batch_page), PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_BATCH_INFO;
	err = mpool_create(mpool_type, num_objects, MPOOL_VEC_ELEM_SINGLE,
			sizeof(struct batch_info), PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	/*
	 * following pools are for special type of objects
	 * i.e. set of objects to be in contiguous memory
	 *
	 */
	num_object_set = pc_init->max_seq_sq_descs;
	num_objects = PNSO_NUM_OBJECTS_IN_OBJECT;
	object_size = sizeof(struct cpdc_desc);

	mpool_type = MPOOL_TYPE_CPDC_DESC_VECTOR;
	err = mpool_create(mpool_type, num_object_set, num_objects,
			object_size, PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	object_size = sizeof(struct cpdc_status_desc);

	mpool_type = MPOOL_TYPE_CPDC_STATUS_DESC_VECTOR;
	err = mpool_create(mpool_type, num_object_set, num_objects,
			object_size, PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	object_size = sizeof(struct cpdc_sgl);

	mpool_type = MPOOL_TYPE_CPDC_SGL_VECTOR;
	err = mpool_create(mpool_type, num_object_set, num_objects,
			object_size, PNSO_MEM_ALIGN_DESC,
			&pc_res->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_RMEM_INTERM_CPDC_STATUS;
	err = mpool_create(mpool_type, num_objects, MPOOL_VEC_ELEM_SINGLE,
			sizeof(struct cpdc_status_desc), PNSO_MEM_ALIGN_DESC,
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
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_BATCH_PAGE]);
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_BATCH_INFO]);
	MPOOL_PPRINT(pc_res->mpools[MPOOL_TYPE_RMEM_INTERM_CPDC_STATUS]);

	return PNSO_OK;

out:
	OSAL_LOG_ERROR("failed to allocate pool! mpool_type: %d err: %d",
			mpool_type, err);
	return err;
}

pnso_error_t
cpdc_init_accelerator(struct pc_res_init_params *pc_init,
		struct per_core_resource *pc_res)
{
	pnso_error_t err;

	OSAL_LOG_DEBUG("enter ...");

	OSAL_ASSERT(pc_init);
	OSAL_ASSERT(pc_res);

	/* TODO-cpdc: use init params */

	err = init_mpools(pc_init, pc_res);
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

	deinit_mpools(pc_res);

	OSAL_LOG_DEBUG("exit!");
}
