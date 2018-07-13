/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __KERNEL__
#include <assert.h>
#endif

#include "osal.h"
#include "pnso_dpool.h"

#include "pnso_cpdc.h"		/* Should dpool be made generic?? */

#define PNSO_MAX_NUM_DBUFS	512	/* Netapp's requirement is around 448 */
#define MIN_NUM_CPDC_DESC	PNSO_MAX_NUM_DBUFS
#define MIN_NUM_XTS_DESC	PNSO_MAX_NUM_DBUFS
#define MIN_NUM_SGL_DESC	PNSO_MAX_NUM_DBUFS

#define MAX_NUM_CPDC_DESC	512
#define MAX_NUM_XTS_DESC	MAX_NUM_CPDC_DESC
#define MAX_NUM_SGL_DESC	1024

#define PNSO_MEM_ALIGN_SPEC	64
#define PNSO_MEM_ALIGN_BUF	256
#define PNSO_MEM_ALIGN_PAGE	4096

#define DPOOL_MAGIC_INVALID	UINT64_C(0xabf0cdf0eff0abf0)
#define DPOOL_MAGIC		UINT64_C(0xffff012345670000)

struct xts_desc {
	uint8_t data[128];	/* replace with right fields */
};

static bool
__is_pool_type_valid(enum desc_pool_type pool_type)
{
	switch (pool_type) {
	case DPOOL_TYPE_CPDC:
	case DPOOL_TYPE_CPDC_SGL:
	case DPOOL_TYPE_CPDC_STATUS:
	case DPOOL_TYPE_XTS:
	case DPOOL_TYPE_AOL:
	case DPOOL_TYPE_CHAIN_ENTRY:
		return true;
	default:
		assert(0);
		return false;
	}

	return false;
}

static bool
__is_max_desc_valid(enum desc_pool_type pool_type, uint32_t num_dbufs)
{
	switch (pool_type) {
	case DPOOL_TYPE_CPDC:
	case DPOOL_TYPE_CPDC_STATUS:
		if (num_dbufs < MIN_NUM_CPDC_DESC ||
		    num_dbufs > MAX_NUM_CPDC_DESC)
			return false;
		break;
	case DPOOL_TYPE_CPDC_SGL:
		if (num_dbufs < MIN_NUM_SGL_DESC ||
		    num_dbufs > MAX_NUM_SGL_DESC)
			return false;
		break;
	case DPOOL_TYPE_XTS:
		if (num_dbufs < MIN_NUM_XTS_DESC ||
		    num_dbufs > MAX_NUM_XTS_DESC)
			return false;
		break;
	case DPOOL_TYPE_AOL:
	case DPOOL_TYPE_CHAIN_ENTRY:
	default:
		assert(0);
		return false;
	}

	return true;
}

static size_t
__get_pool_align_size(enum desc_pool_type pool_type)
{
	/*
	 * TODO: fix-up alignment/padding depending on pool_type,
	 * power of 2, etc.
	 *
	 */
	switch (pool_type) {
	case DPOOL_TYPE_CPDC:
		return sizeof(struct cpdc_desc);
	case DPOOL_TYPE_CPDC_SGL:
		return sizeof(struct cpdc_sgl);
	case DPOOL_TYPE_CPDC_STATUS:
		return sizeof(struct cpdc_status);
		// return PNSO_MEM_ALIGN_BUF;
	case DPOOL_TYPE_XTS:
		return sizeof(struct xts_desc);
	case DPOOL_TYPE_AOL:
	case DPOOL_TYPE_CHAIN_ENTRY:
	default:
		assert(0);
		return PNSO_MEM_ALIGN_PAGE;
	}

	return 0;
}

static size_t
__get_pool_size(enum desc_pool_type pool_type, uint32_t num_dbufs)
{
	switch (pool_type) {
	case DPOOL_TYPE_CPDC:
		return sizeof(struct cpdc_desc) * num_dbufs;
	case DPOOL_TYPE_CPDC_SGL:
		return sizeof(struct cpdc_sgl) * num_dbufs;
	case DPOOL_TYPE_CPDC_STATUS:
		return sizeof(struct cpdc_status) * num_dbufs;
		// return PNSO_MEM_ALIGN_BUF * num_dbufs;
	case DPOOL_TYPE_XTS:
		return sizeof(struct xts_desc) * num_dbufs;
	default:
		assert(0);
		return 0;
	}

	return 0;
}


static uint32_t
__get_dbuf_size(enum desc_pool_type pool_type)
{
	switch (pool_type) {
	case DPOOL_TYPE_CPDC:
		return sizeof(struct cpdc_desc);
	case DPOOL_TYPE_CPDC_SGL:
		return sizeof(struct cpdc_sgl);
	case DPOOL_TYPE_CPDC_STATUS:
		return sizeof(struct cpdc_status);
		// return PNSO_MEM_ALIGN_BUF;
	case DPOOL_TYPE_XTS:
		return sizeof(struct xts_desc);
	default:
		assert(0);
		return 0;
	}

	return 0;
}

pnso_error_t
dpool_create(enum desc_pool_type pool_type,
	     uint32_t num_dbufs, struct desc_pool **out_pool)
{
	pnso_error_t err;
	struct desc_pool *dpool = NULL;
	size_t pool_align;
	size_t pool_size;
	uint32_t dbuf_size;
	void **descs;
	void *dbuf;
	int i;

	if (!__is_pool_type_valid(pool_type)) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid pool type specified. pool_type: %d err: %d",
			       pool_type, err);
		return err;
	}

	if (!__is_max_desc_valid(pool_type, num_dbufs)) {
		err = EINVAL;
		OSAL_LOG_ERROR("invalid max desc requested. num_dbufs: %d err: %d",
			       num_dbufs, err);
		return err;
	}

	/* allocate memory for pool, dbufs, and dstack */
	dpool = osal_alloc(sizeof(struct desc_pool));
	if (!dpool) {
		err = ENOMEM;
		OSAL_LOG_ERROR("failed to allocate memory for pool! pool_type: %d num_dbufs: %d err: %d",
			       pool_type, num_dbufs, err);
		return err;
	}

	pool_align = __get_pool_align_size(pool_type);
	pool_size = __get_pool_size(pool_type, num_dbufs);

	dpool->dp_dbufs = osal_aligned_alloc(pool_align, pool_size);
	if (!dpool->dp_dbufs) {
		err = ENOMEM;
		OSAL_LOG_ERROR("failed to allocate memory for dbufs! pool_type: %d num_dbufs: %d err: %d",
			       pool_type, num_dbufs, err);
		goto out_free_pool;
	}

	descs = osal_alloc(sizeof(void *) * num_dbufs);
	if (!descs) {
		err = ENOMEM;
		OSAL_LOG_ERROR("failed to allocated memory for descs! pool_type: %d num_dbufs: %d err: %d",
			       pool_type, num_dbufs, err);
		goto out_free_dbufs;
	}
	dbuf_size = __get_dbuf_size(pool_type);

	/* populate the pool */
	dpool->dp_magic = DPOOL_MAGIC;
	dpool->dp_destroy = false;

	dpool->dp_config.dpc_type = pool_type;
	dpool->dp_config.dpc_num_dbufs = num_dbufs;
	dpool->dp_config.dpc_dbuf_size = dbuf_size;

	dpool->dp_dstack.dps_num_dbufs = num_dbufs;
	dpool->dp_dstack.dps_top = 0;
	dpool->dp_dstack.dps_descs = descs;

	/* populate the descriptor stack to point to dbufs */
	dbuf = dpool->dp_dbufs;
	for (i = 0; i < dpool->dp_config.dpc_num_dbufs; i++) {
		descs[i] = dbuf;
		// OSAL_LOG_INFO("%-30s: %p %p", "dbuf", dbuf, descs[i]);
		OSAL_LOG_INFO("%30s[%d]: %p %p",
			       "dpool->dp_dstack.dps_descs", i,
			       &descs[i], descs[i]);
		dbuf += dbuf_size;
	}
	dpool->dp_dstack.dps_top = dpool->dp_config.dpc_num_dbufs;

	*out_pool = dpool;
	OSAL_LOG_INFO("pool allocated. dpc_type: %d dpc_num_dbufs: %d dpool: %p",
		      pool_type, num_dbufs, dpool);

	return PNSO_OK;

out_free_dbufs:
	osal_free(dpool->dp_dbufs);
out_free_pool:
	osal_free(dpool);
// out: TODO
	return err;
}

void
dpool_destroy(struct desc_pool **poolp)
{
	struct desc_pool *dpool;

	if (!poolp || !*poolp)
		return;

	dpool = *poolp;

	OSAL_LOG_INFO("pool deallocated. dpc_type: %d dpc_num_dbufs: %d dpool: %p",
		      dpool->dp_config.dpc_type, dpool->dp_config.dpc_num_dbufs,
		      dpool);

	dpool->dp_destroy = true;
	dpool->dp_magic = DPOOL_MAGIC_INVALID;

	/* TODO: ensure stack top is full */

	osal_free(dpool->dp_dstack.dps_descs);
	osal_free(dpool->dp_dbufs);
	osal_free(dpool);

	*poolp = NULL;
}

void *
dpool_get_desc(struct desc_pool *dpool)
{
	struct desc_pool_stack *dstack;
	void *dbuf = NULL;

	if (!dpool)
		return NULL;

	dstack = &dpool->dp_dstack;
	assert(dstack);

	if (dstack->dps_top > 0)
		dbuf = dstack->dps_descs[--(dstack->dps_top)];

	return dbuf;
}

pnso_error_t
dpool_put_desc(struct desc_pool *dpool, void *dbuf)
{
	pnso_error_t err = ENOTEMPTY;
	struct desc_pool_stack *dstack;

	if (!dpool || !dbuf)
		return -EINVAL;

	dstack = &dpool->dp_dstack;
	assert(dstack);

	if (dstack->dps_top < dstack->dps_num_dbufs) {
		dstack->dps_descs[dstack->dps_top] = dbuf;
		dstack->dps_top++;
		err = PNSO_OK;
	}

	return err;
}

void
dpool_pprint(const struct desc_pool *dpool)
{
	int i;
	void **descs;

	/* TODO: disable invocation of this function in release mode */

	if (!dpool)
		return;

	OSAL_LOG_INFO("%-30s: %p", "dpool", dpool);
	OSAL_LOG_INFO("%-30s: %jx", "dpool->dp_magic", dpool->dp_magic);

	OSAL_LOG_INFO("%-30s: %ju", "dpool->dp_config.dpc_type",
			dpool->dp_config.dpc_type);
	OSAL_LOG_INFO("%-30s: %ju", "dpool->dp_config.dpc_num_dbufs",
			dpool->dp_config.dpc_num_dbufs);
	OSAL_LOG_INFO("%-30s: %ju", "dpool->dp_config.dpc_dbuf_size",
			dpool->dp_config.dpc_dbuf_size);

	OSAL_LOG_INFO("%-30s: %d", "dpool->dp_destroy",
			dpool->dp_destroy);

	OSAL_LOG_INFO("%--30s: %p", "dpool->dp_dbufs",
			dpool->dp_dbufs);

	OSAL_LOG_INFO("%-30s: %d", "dpool->dp_dstack.dps_num_dbufs",
			dpool->dp_dstack.dps_num_dbufs);
	OSAL_LOG_INFO("%-30s: %d", "dpool->dp_dstack.dps_top",
			dpool->dp_dstack.dps_top);
	OSAL_LOG_INFO("%-30s: %p", "dpool->dp_dstack.dps_descs",
			dpool->dp_dstack.dps_descs);

	descs = dpool->dp_dstack.dps_descs;
	for (i = 0; i < dpool->dp_config.dpc_num_dbufs; i++) {
		OSAL_LOG_DEBUG("%30s[%d]: %p %p",
				"dpool->dp_dstack.dps_descs", i,
				&descs[i], descs[i]);
	}
}
