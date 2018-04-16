/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "pnso_global.h"
#include "pnso_logger.h"
#include "pnso_dpool.h"

#include "pnso_cpdc.h"

/* TODO: depends on qdepth, so revisit upon Netapp review */
#define MIN_NUM_CPDC_DESC	PNSO_UT_MAX_DBUFS
#define MIN_NUM_XTS_DESC	MIN_NUM_CPDC_DESC
#define MIN_NUM_SGL_DESC	MIN_NUM_CPDC_DESC

#define MAX_NUM_CPDC_DESC	512
#define MAX_NUM_XTS_DESC	MAX_NUM_CPDC_DESC
#define MAX_NUM_SGL_DESC	1024

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
	case DPOOL_TYPE_XTS:
	case DPOOL_TYPE_CPDC_SGL:
	case DPOOL_TYPE_CPDC_STATUS_NOSHA:
	case DPOOL_TYPE_CPDC_STATUS_SHA256:
	case DPOOL_TYPE_CPDC_STATUS_SHA512:
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
	case DPOOL_TYPE_CPDC_STATUS_NOSHA:
	case DPOOL_TYPE_CPDC_STATUS_SHA256:
	case DPOOL_TYPE_CPDC_STATUS_SHA512:
		if (num_dbufs < MIN_NUM_CPDC_DESC ||
		    num_dbufs > MAX_NUM_CPDC_DESC)
			return false;
		break;
	case DPOOL_TYPE_XTS:
		if (num_dbufs < MIN_NUM_XTS_DESC ||
		    num_dbufs > MAX_NUM_XTS_DESC)
			return false;
		break;
	case DPOOL_TYPE_CPDC_SGL:
		if (num_dbufs < MIN_NUM_SGL_DESC ||
		    num_dbufs > MAX_NUM_SGL_DESC)
			return false;
		break;
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
	case DPOOL_TYPE_CPDC_STATUS_NOSHA:
		return sizeof(struct cpdc_status_nosha);
	case DPOOL_TYPE_CPDC_STATUS_SHA256:
		return sizeof(struct cpdc_status_sha256);
	case DPOOL_TYPE_CPDC_STATUS_SHA512:
		return sizeof(struct cpdc_status_sha512);
		// return PNSO_MEM_ALIGN_BUF;
	case DPOOL_TYPE_XTS:
		return sizeof(struct xts_desc);
	case DPOOL_TYPE_CPDC_SGL:
		return sizeof(struct cpdc_sgl);
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
	case DPOOL_TYPE_CPDC_STATUS_NOSHA:
		return sizeof(struct cpdc_status_nosha) * num_dbufs;
	case DPOOL_TYPE_CPDC_STATUS_SHA256:
		return sizeof(struct cpdc_status_sha256) * num_dbufs;
	case DPOOL_TYPE_CPDC_STATUS_SHA512:
		return sizeof(struct cpdc_status_sha512) * num_dbufs;
		// return PNSO_MEM_ALIGN_BUF * num_dbufs;
	case DPOOL_TYPE_XTS:
		return sizeof(struct xts_desc) * num_dbufs;
	case DPOOL_TYPE_CPDC_SGL:
		return sizeof(struct cpdc_sgl) * num_dbufs;
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
	case DPOOL_TYPE_CPDC_STATUS_NOSHA:
		return sizeof(struct cpdc_status_nosha);
	case DPOOL_TYPE_CPDC_STATUS_SHA256:
		return sizeof(struct cpdc_status_sha256);
	case DPOOL_TYPE_CPDC_STATUS_SHA512:
		return sizeof(struct cpdc_status_sha512);
		// return PNSO_MEM_ALIGN_BUF;
	case DPOOL_TYPE_XTS:
		return sizeof(struct xts_desc);
	case DPOOL_TYPE_CPDC_SGL:
		return sizeof(struct cpdc_sgl);
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
		PNSO_LOG_ERROR(err,
			       "invalid pool type specified. pool_type: %d",
			       pool_type);
		return err;
	}

	if (!__is_max_desc_valid(pool_type, num_dbufs)) {
		err = EINVAL;
		PNSO_LOG_ERROR(err, "invalid max desc requested. num_dbufs: %d",
			       num_dbufs);
		return err;
	}

	/* allocate memory for pool, dbufs, and dstack */
	dpool = pnso_malloc(sizeof(struct desc_pool));
	if (!dpool) {
		err = ENOMEM;
		PNSO_LOG_ERROR(err,
			       "failed to allocate memory for pool! pool_type: %d num_dbufs: %d",
			       pool_type, num_dbufs);
		return err;
	}

	pool_align = __get_pool_align_size(pool_type);
	pool_size = __get_pool_size(pool_type, num_dbufs);

	err = pnso_memalign(pool_align, pool_size, (void **) &dpool->dp_dbufs);
	if (err) {
		PNSO_LOG_ERROR(err,
			       "failed to allocate memory for dbufs! pool_type: %d num_dbufs: %d",
			       pool_type, num_dbufs);
		goto out_free_pool;
	}

	descs = pnso_malloc(sizeof(void *) * num_dbufs);
	if (!descs) {
		err = ENOMEM;
		PNSO_LOG_ERROR(err,
			       "failed to allocated memory for descs! pool_type: %d num_dbufs: %d",
			       pool_type, num_dbufs);
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
		// PNSO_LOG_INFO(PNSO_OK, "%-30s: %p %p", "dbuf", dbuf, descs[i]);
		PNSO_LOG_INFO(PNSO_OK, "%30s[%d]: %p %p",
			       "dpool->dp_dstack.dps_descs", i,
			       &descs[i], descs[i]);
		dbuf += dbuf_size;
	}
	dpool->dp_dstack.dps_top = dpool->dp_config.dpc_num_dbufs;

	*out_pool = dpool;
	PNSO_LOG_INFO(PNSO_OK,
		      "pool allocated. dpc_type: %d dpc_num_dbufs: %d dpool: %p",
		      pool_type, num_dbufs, dpool);

	return err;

out_free_dbufs:
	pnso_free(dpool->dp_dbufs);
out_free_pool:
	pnso_free(dpool);
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

	PNSO_LOG_INFO(PNSO_OK,
		      "pool deallocated. dpc_type: %d dpc_num_dbufs: %d dpool: %p",
		      dpool->dp_config.dpc_type, dpool->dp_config.dpc_num_dbufs,
		      dpool);

	dpool->dp_destroy = true;
	dpool->dp_magic = DPOOL_MAGIC_INVALID;

	/* TODO: ensure stack top is full */

	pnso_free(dpool->dp_dstack.dps_descs);
	pnso_free(dpool->dp_dbufs);
	pnso_free(dpool);

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
dpool_pprint(struct desc_pool *dpool)
{
	int i;
	void **descs;

	/* TODO: disable invocation of this function in release mode */

	if (!dpool)
		return;

	PNSO_LOG_INFO(PNSO_OK, "%-30s: %p", "dpool", dpool);
	PNSO_LOG_INFO(PNSO_OK, "%-30s: %jx", "dpool->dp_magic",
		      dpool->dp_magic);

	PNSO_LOG_INFO(PNSO_OK, "%-30s: %ju", "dpool->dp_config.dpc_type",
		      dpool->dp_config.dpc_type);
	PNSO_LOG_INFO(PNSO_OK, "%-30s: %ju", "dpool->dp_config.dpc_num_dbufs",
		      dpool->dp_config.dpc_num_dbufs);
	PNSO_LOG_INFO(PNSO_OK, "%-30s: %ju", "dpool->dp_config.dpc_dbuf_size",
		      dpool->dp_config.dpc_dbuf_size);

	PNSO_LOG_INFO(PNSO_OK, "%-30s: %d", "dpool->dp_destroy",
		      dpool->dp_destroy);

	PNSO_LOG_INFO(PNSO_OK, "%--30s: %p", "dpool->dp_dbufs",
		      dpool->dp_dbufs);

	PNSO_LOG_INFO(PNSO_OK, "%-30s: %d", "dpool->dp_dstack.dps_num_dbufs",
		      dpool->dp_dstack.dps_num_dbufs);
	PNSO_LOG_INFO(PNSO_OK, "%-30s: %d", "dpool->dp_dstack.dps_top",
		      dpool->dp_dstack.dps_top);
	PNSO_LOG_INFO(PNSO_OK, "%-30s: %p", "dpool->dp_dstack.dps_descs",
		      dpool->dp_dstack.dps_descs);

	descs = dpool->dp_dstack.dps_descs;
	for (i = 0; i < dpool->dp_config.dpc_num_dbufs; i++) {
		PNSO_LOG_DEBUG(PNSO_OK, "%30s[%d]: %p %p",
			       "dpool->dp_dstack.dps_descs", i,
			       &descs[i], descs[i]);
	}
}
