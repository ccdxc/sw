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

struct mem_pool *cpdc_mpool;
struct mem_pool *cpdc_sgl_mpool;
struct mem_pool *cpdc_status_mpool;

/*
 * In some scenarios, HW wants a group of command/status descriptors in
 * contiguous memory.  mpool does not provide this ability, however, mpool can
 * be created such that one object encompasses several such descriptors.  This
 * gives the ablility to reference a bulk of such descriptors through object.
 *
 */
struct mem_pool *cpdc_bulk_mpool;
struct mem_pool *cpdc_status_bulk_mpool;

pnso_error_t
cpdc_start_accelerator(const struct cpdc_init_params *init_params)
{
	pnso_error_t err;
	uint32_t num_objects, object_size, pad_size;

	OSAL_LOG_INFO("enter ...");

	/* TODO-cpdc: use init params */

	err = mpool_create(MPOOL_TYPE_CPDC, PNSO_MAX_NUM_CPDC_DESC,
			sizeof(struct cpdc_desc), PNSO_MEM_ALIGN_DESC,
			&cpdc_mpool);
	if (err) {
		OSAL_LOG_ERROR("failed to create CPDC descriptor pool err: %d",
				err);
		goto out;
	}

	err = mpool_create(MPOOL_TYPE_CPDC_SGL, PNSO_MAX_NUM_CPDC_SGL_DESC,
			sizeof(struct cpdc_sgl), PNSO_MEM_ALIGN_DESC,
			&cpdc_sgl_mpool);
	if (err) {
		OSAL_LOG_ERROR("failed to create CPDC sgl pool err: %d",
				err);
		goto out_free_cpdc;
	}

	err = mpool_create(MPOOL_TYPE_CPDC_STATUS,
			PNSO_MAX_NUM_CPDC_STATUS_DESC,
			sizeof(struct cpdc_status_desc), PNSO_MEM_ALIGN_DESC,
			&cpdc_status_mpool);
	if (err) {
		OSAL_LOG_ERROR("failed to create CPDC status descriptor pool err: %d",
				err);
		goto out_free_cpdc_sgl;
	}

	/*
	 * following pools are for special type of objects i.e. set of objects
	 * to be in contiguous memory
	 *
	 */
	num_objects = PNSO_NUM_OBJECTS_WITHIN_OBJECT;
	pad_size = mpool_get_pad_size(sizeof(struct cpdc_desc),
			PNSO_MEM_ALIGN_DESC);
	object_size = (sizeof(struct cpdc_desc) + pad_size) * num_objects;

	err = mpool_create(MPOOL_TYPE_CPDC, PNSO_MAX_NUM_CPDC_DESC,
			object_size, PNSO_MEM_ALIGN_DESC, &cpdc_bulk_mpool);
	if (err) {
		OSAL_LOG_ERROR("failed to create CPDC bulk descriptor pool err: %d",
				err);
		goto out_free_cpdc_status;
	}

	pad_size = mpool_get_pad_size(sizeof(struct cpdc_status_desc),
			PNSO_MEM_ALIGN_DESC);
	object_size = (sizeof(struct cpdc_status_desc) +
			pad_size) * num_objects;

	err = mpool_create(MPOOL_TYPE_CPDC_STATUS,
			PNSO_MAX_NUM_CPDC_STATUS_DESC, object_size,
			PNSO_MEM_ALIGN_DESC, &cpdc_status_bulk_mpool);
	if (err) {
		OSAL_LOG_ERROR("failed to create CPDC bulk status pool err: %d",
				err);
		goto out_free_cpdc_bulk;
	}

	mpool_pprint(cpdc_mpool);
	mpool_pprint(cpdc_sgl_mpool);
	mpool_pprint(cpdc_status_mpool);

	mpool_pprint(cpdc_bulk_mpool);
	mpool_pprint(cpdc_status_bulk_mpool);

	OSAL_LOG_INFO("exit!");
	return err;

out_free_cpdc_bulk:
	mpool_destroy(&cpdc_bulk_mpool);
out_free_cpdc_status:
	mpool_destroy(&cpdc_status_mpool);
out_free_cpdc_sgl:
	mpool_destroy(&cpdc_sgl_mpool);
out_free_cpdc:
	mpool_destroy(&cpdc_mpool);
out:
	OSAL_LOG_ERROR("exit!");
	return err;
}

void
cpdc_stop_accelerator(void)
{
	OSAL_LOG_INFO("enter ...");

	mpool_destroy(&cpdc_status_bulk_mpool);
	mpool_destroy(&cpdc_bulk_mpool);

	mpool_destroy(&cpdc_status_mpool);
	mpool_destroy(&cpdc_sgl_mpool);
	mpool_destroy(&cpdc_mpool);

	OSAL_LOG_INFO("exit!");
}
