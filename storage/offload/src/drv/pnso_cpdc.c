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
 *	- move init/deinit of the following out of CPDC:
 *		MPOOL_TYPE_SERVICE_CHAIN*, MPOOL_TYPE_BATCH_*
 *	- do not assume max input buffer as 64K and block size as 4K
 *
 */
#define PNSO_NUM_OBJECTS_IN_OBJECT	16
#define PNSO_NUM_BYPASS_OBJECTS		2	/* per desc for non-batch */
#define PNSO_MAX_NUM_PB_PER_REQUEST	16 /* max # of per-block per request */

static void
deinit_mpools(struct per_core_resource *pcr)
{
	mpool_destroy(&pcr->mpools[
			MPOOL_TYPE_RMEM_INTERM_CPDC_STATUS_DESC_VECTOR]);
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_CPDC_SGL_VECTOR]);
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_CPDC_STATUS_DESC_VECTOR]);
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_CPDC_DESC_BO_PB_VECTOR]);
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_CPDC_DESC_BO_VECTOR]);
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_CPDC_DESC_PB_VECTOR]);
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_CPDC_DESC_VECTOR]);
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_RMEM_INTERM_CPDC_STATUS_DESC]);
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_BATCH_INFO]);
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_BATCH_PAGE]);
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_SERVICE_CHAIN_ENTRY]);
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_SERVICE_CHAIN]);
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_CPDC_STATUS_DESC]);
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_CPDC_SGL]);
	mpool_destroy(&pcr->mpools[MPOOL_TYPE_CPDC_DESC]);
}

static pnso_error_t
init_mpools(struct pc_res_init_params *pc_init, struct per_core_resource *pcr)
{
	pnso_error_t err;
	uint32_t num_objects;
	enum mem_pool_type mpool_type;

	num_objects = pc_init->max_seq_sq_descs;

	mpool_type = MPOOL_TYPE_CPDC_DESC;
	err = mpool_create(mpool_type, num_objects, MPOOL_VEC_ELEM_SINGLE,
			sizeof(struct cpdc_desc), PNSO_MEM_ALIGN_DESC,
			&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_CPDC_SGL;
	err = mpool_create(mpool_type, num_objects * MAX_CPDC_SGLS_PER_REQ,
			MPOOL_VEC_ELEM_SINGLE,
			sizeof(struct cpdc_sgl), PNSO_MEM_ALIGN_DESC,
			&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_CPDC_STATUS_DESC;
	err = mpool_create(mpool_type, num_objects, MPOOL_VEC_ELEM_SINGLE,
			sizeof(struct cpdc_status_desc), PNSO_MEM_ALIGN_DESC,
			&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_SERVICE_CHAIN;
	err = mpool_create(mpool_type, num_objects, MPOOL_VEC_ELEM_SINGLE,
			sizeof(struct service_chain), PNSO_MEM_ALIGN_DESC,
			&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_SERVICE_CHAIN_ENTRY;
	err = mpool_create(mpool_type, num_objects, MPOOL_VEC_ELEM_SINGLE,
			sizeof(struct chain_entry), PNSO_MEM_ALIGN_DESC,
			&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_BATCH_PAGE;
	err = mpool_create(mpool_type, num_objects, MPOOL_VEC_ELEM_SINGLE,
			sizeof(struct batch_page), PNSO_MEM_ALIGN_DESC,
			&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_BATCH_INFO;
	err = mpool_create(mpool_type, num_objects, MPOOL_VEC_ELEM_SINGLE,
			sizeof(struct batch_info), PNSO_MEM_ALIGN_DESC,
			&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_RMEM_INTERM_CPDC_STATUS_DESC;
	err = mpool_create(mpool_type, num_objects, MPOOL_VEC_ELEM_SINGLE,
			sizeof(struct cpdc_status_desc), PNSO_MEM_ALIGN_DESC,
			&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	/*
	 * following pools are for special type of objects
	 * i.e. set of objects to be in contiguous memory
	 *
	 */
	mpool_type = MPOOL_TYPE_CPDC_DESC_VECTOR;
	err = mpool_create(mpool_type, num_objects * MAX_CPDC_DESC_VEC_PER_REQ,
			PNSO_NUM_OBJECTS_IN_OBJECT,
			sizeof(struct cpdc_desc), PNSO_MEM_ALIGN_DESC,
			&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_CPDC_DESC_PB_VECTOR;
	err = mpool_create(mpool_type, MAX_NUM_PAGES,
			PNSO_MAX_NUM_PB_PER_REQUEST * MAX_PAGE_ENTRIES,
			sizeof(struct cpdc_desc), PNSO_MEM_ALIGN_DESC,
			&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_CPDC_DESC_BO_VECTOR;
	err = mpool_create(mpool_type, num_objects,
			PNSO_NUM_BYPASS_OBJECTS,
			sizeof(struct cpdc_desc), PNSO_MEM_ALIGN_DESC,
			&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_CPDC_DESC_BO_PB_VECTOR;
	err = mpool_create(mpool_type, num_objects,
			PNSO_NUM_BYPASS_OBJECTS * PNSO_NUM_OBJECTS_IN_OBJECT,
			sizeof(struct cpdc_desc), PNSO_MEM_ALIGN_DESC,
			&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_CPDC_STATUS_DESC_VECTOR;
	err = mpool_create(mpool_type, num_objects, PNSO_NUM_OBJECTS_IN_OBJECT,
			sizeof(struct cpdc_status_desc), PNSO_MEM_ALIGN_DESC,
			&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_CPDC_SGL_VECTOR;
	err = mpool_create(mpool_type, num_objects * MAX_CPDC_SGL_VEC_PER_REQ,
			PNSO_NUM_OBJECTS_IN_OBJECT,
			sizeof(struct cpdc_sgl), PNSO_MEM_ALIGN_DESC,
			&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	mpool_type = MPOOL_TYPE_RMEM_INTERM_CPDC_STATUS_DESC_VECTOR;
	err = mpool_create(mpool_type, num_objects, PNSO_NUM_OBJECTS_IN_OBJECT,
			sizeof(struct cpdc_status_desc), PNSO_MEM_ALIGN_DESC,
			&pcr->mpools[mpool_type]);
	if (err)
		goto out;

	MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CPDC_DESC]);
	MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CPDC_DESC_VECTOR]);
	MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CPDC_DESC_PB_VECTOR]);
	MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CPDC_DESC_BO_VECTOR]);
	MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CPDC_DESC_BO_PB_VECTOR]);
	MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CPDC_SGL]);
	MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CPDC_SGL_VECTOR]);
	MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CPDC_STATUS_DESC]);
	MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_CPDC_STATUS_DESC_VECTOR]);
	MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_SERVICE_CHAIN]);
	MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_SERVICE_CHAIN_ENTRY]);
	MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_BATCH_PAGE]);
	MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_BATCH_INFO]);
	MPOOL_PPRINT(pcr->mpools[MPOOL_TYPE_RMEM_INTERM_CPDC_STATUS_DESC]);

	return PNSO_OK;

out:
	OSAL_LOG_ERROR("failed to allocate pool! mpool_type: %d num_objects: %d err: %d",
			mpool_type, num_objects, err);
	return err;
}

pnso_error_t
cpdc_init_accelerator(struct pc_res_init_params *pc_init,
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
cpdc_deinit_accelerator(struct per_core_resource *pcr)
{
	OSAL_LOG_DEBUG("enter ...");

	deinit_mpools(pcr);

	OSAL_LOG_DEBUG("exit!");
}
