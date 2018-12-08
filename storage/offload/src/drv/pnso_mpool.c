/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include "osal.h"
#include "osal_rmem.h"
#include "pnso_mpool.h"

#ifdef NDEBUG
#define MPOOL_VALIDATE_OBJECT(p, o)
#else
#define MPOOL_VALIDATE_OBJECT(p, o)	validate_object(p, o)
#endif

#define MPOOL_MAGIC_VALID	0xffff012345670000
#define MPOOL_MAGIC_INVALID	0xabf0cdf0eff0abf0

#define PNSO_MAX(a, b) ((a) > (b) ? (a) : (b))

const char __attribute__ ((unused)) *mem_pool_types[] = {
	[MPOOL_TYPE_NONE] = "None (invalid)",
	[MPOOL_TYPE_CPDC_DESC] = "CPDC DESC",
	[MPOOL_TYPE_CPDC_DESC_VECTOR] = "CPDC DESC VECTOR",
	[MPOOL_TYPE_CPDC_SGL] = "CPDC SGL",
	[MPOOL_TYPE_CPDC_SGL_VECTOR] = "CPDC SGL VECTOR",
	[MPOOL_TYPE_CPDC_STATUS_DESC] = "CPDC STATUS DESC",
	[MPOOL_TYPE_CPDC_STATUS_DESC_VECTOR] = "CPDC STATUS DESC VECTOR",
	[MPOOL_TYPE_CRYPTO_DESC] = "CRYPTO DESC",
	[MPOOL_TYPE_CRYPTO_DESC_VECTOR] = "CRYPTO DESC VECTOR",
	[MPOOL_TYPE_CRYPTO_STATUS_DESC] = "CRYPTO STATUS DESC",
	[MPOOL_TYPE_CRYPTO_AOL] = "CRYPTO AOL",
	[MPOOL_TYPE_CRYPTO_AOL_VECTOR] = "CRYPTO AOL VECTOR",
	[MPOOL_TYPE_CHAIN_SGL_PDMA] = "CHAIN SGL PDMA",
	[MPOOL_TYPE_SERVICE_CHAIN] = "SERVICE CHAIN",
	[MPOOL_TYPE_SERVICE_CHAIN_ENTRY] = "SERVICE CHAIN ENTRY",
	[MPOOL_TYPE_BATCH_PAGE] = "BATCH PAGE",
	[MPOOL_TYPE_BATCH_INFO] = "BATCH INFO",
	[MPOOL_TYPE_RMEM_INTERM_CRYPTO_STATUS] = "RMEM CRYPTO STATUS DESC",
	[MPOOL_TYPE_RMEM_INTERM_CPDC_STATUS] = "RMEM CPDC STATUS DESC",
	[MPOOL_TYPE_RMEM_INTERM_BUF] = "RMEM INTERM BUF",
	[MPOOL_TYPE_MAX] = "Max (invalid)"
};

const char *
mpool_get_type_str(enum mem_pool_type mpool_type)
{
	if (mpool_type_is_valid(mpool_type))
		return mem_pool_types[mpool_type];

	return "unknown";
}

static bool __attribute__ ((unused))
is_pool_valid(struct mem_pool *mpool)
{
	return (mpool->mp_magic & MPOOL_MAGIC_VALID) ? true : false;
}

static inline void __attribute__ ((unused))
validate_object(struct mem_pool *mpool, void *object)
{
	uint32_t pool_size, obj_size;
	bool in_range;

	obj_size = mpool->mp_config.mpc_object_size +
		 mpool->mp_config.mpc_pad_size;
	pool_size = mpool->mp_config.mpc_pool_size;

	in_range = ((((char *) object - (char *) mpool->mp_objects) /
				obj_size) < pool_size);
	OSAL_ASSERT(in_range);
}

uint32_t
mpool_get_pad_size(uint32_t object_size, uint32_t align_size)
{
	uint32_t pad_size = 0;

	/* align_size of 0 means no alignment needed */
	if (align_size == PNSO_MEM_ALIGN_NONE)
		goto out;

	if (PNSO_MAX(object_size, align_size) == align_size) {
		pad_size = align_size - object_size;
		goto out;
	}

	if (object_size % align_size == 0)
		goto out;

	pad_size = (object_size + align_size -
			(object_size % align_size)) - object_size;

out:
	return pad_size;
}

static void *
mpool_create_mem_objects(struct mem_pool *mpool)
{
	void *p;

	mpool->mp_config.mpc_num_allocs = 0;
	mpool->mp_config.mpc_page_size = 0;
	if (mpool->mp_config.mpc_pool_size) {
		p = mpool->mp_config.mpc_align_size != PNSO_MEM_ALIGN_NONE ?
		    osal_aligned_alloc(mpool->mp_config.mpc_align_size,
				       mpool->mp_config.mpc_pool_size) :
		    osal_alloc(mpool->mp_config.mpc_pool_size);
		if (p) {
			mpool->mp_config.mpc_num_allocs = 1;
			return p;
		}
		OSAL_LOG_ERROR("failed to allocate mem objects for pool %s",
			       mpool_get_type_str(mpool->mp_config.mpc_type));
	}

	return NULL;
}

static void
mpool_destroy_mem_objects(struct mem_pool *mpool)
{
	if (mpool->mp_objects) {
		osal_free(mpool->mp_objects);
		mpool->mp_objects = NULL;
	}
}

static void *
mpool_create_rmem_objects(struct mem_pool *mpool)
{
	uint64_t first_rmem;
	uint64_t curr_rmem;
	uint64_t prev_rmem;
	uint32_t total_size;

	mpool->mp_config.mpc_num_allocs = 0;
	total_size = mpool->mp_config.mpc_pool_size;
	if (total_size == 0)
		return NULL;

	/*
	 * rmem alignment is always page_size so ensure it can
	 * subsume align_size.
	 */
	mpool->mp_config.mpc_page_size = osal_rmem_page_size_get();
	OSAL_ASSERT(is_power_of_2(mpool->mp_config.mpc_page_size));
	if ((mpool->mp_config.mpc_align_size != PNSO_MEM_ALIGN_NONE) &&
	    (!is_power_of_2(mpool->mp_config.mpc_align_size) ||
	     (mpool->mp_config.mpc_align_size >
	      mpool->mp_config.mpc_page_size))) {
		OSAL_LOG_ERROR("rmem invalid align_size %u in relation to page_size %u",
			       mpool->mp_config.mpc_align_size,
			       mpool->mp_config.mpc_page_size);
		return NULL;
	}

	/*
	 * rmem supports allocation of multiple pages, but it would do so in
	 * power of 2 count, e.g., an allocation of 20 pages will end up taking
	 * 32 pages. To avoid this, we loop and allocate each page individually.
	 */
	first_rmem = osal_rmem_addr_invalid_def();
	prev_rmem  = first_rmem;
	while (total_size) {
		curr_rmem = osal_rmem_alloc(mpool->mp_config.mpc_page_size);
		if (!osal_rmem_addr_valid(curr_rmem)) {
			OSAL_LOG_ERROR("pool %s failed after %u allocs remaining size %u",
				       mpool_get_type_str(
					       mpool->mp_config.mpc_type),
				       mpool->mp_config.mpc_num_allocs,
				       total_size);
			goto error;
		}

		mpool_void_ptr_check(curr_rmem);
		if (!osal_rmem_addr_valid(first_rmem))
			first_rmem = curr_rmem;

		if (osal_rmem_addr_valid(prev_rmem) &&
				((curr_rmem - mpool->mp_config.mpc_page_size) !=
				 prev_rmem)) {
			OSAL_LOG_ERROR("unexpected non-contiguous alloc curr_rmem 0x" PRIx64 " prev_rmem 0x" PRIx64,
					curr_rmem, prev_rmem);

			osal_rmem_free(curr_rmem,
					mpool->mp_config.mpc_page_size);
			goto error;
		}
		mpool->mp_config.mpc_num_allocs++;
		prev_rmem = curr_rmem;
		total_size = total_size > mpool->mp_config.mpc_page_size ?
			     total_size - mpool->mp_config.mpc_page_size : 0;
	}

	return (void *)first_rmem;
error:
	while (mpool->mp_config.mpc_num_allocs--) {
		osal_rmem_free(first_rmem, mpool->mp_config.mpc_page_size);
		first_rmem += mpool->mp_config.mpc_page_size;
	}
	return NULL;
}

static void
mpool_destroy_rmem_objects(struct mem_pool *mpool)
{
	uint64_t rmem;

	if (mpool->mp_objects) {
		OSAL_ASSERT(mpool->mp_config.mpc_num_allocs);
		rmem = (uint64_t)mpool->mp_objects;
		while (mpool->mp_config.mpc_num_allocs--) {
			osal_rmem_free(rmem, mpool->mp_config.mpc_page_size);
			rmem += mpool->mp_config.mpc_page_size;
		}
		mpool->mp_objects = NULL;
	}
}

pnso_error_t
mpool_create(enum mem_pool_type mpool_type, uint32_t num_objects,
		uint32_t num_vec_elems, uint32_t object_size,
		uint32_t align_size, struct mem_pool **out_mpool)
{
	pnso_error_t err = EINVAL;
	struct mem_pool *mpool = NULL;
	void **objects;
	char *obj;
	int i;

	if (!mpool_type_is_valid(mpool_type)) {
		OSAL_LOG_ERROR("invalid pool type specified. mpool_type: %d err: %d",
			       mpool_type, err);
		goto out;
	}

	if (num_vec_elems == 0) {
		OSAL_LOG_ERROR("num_vec_elems must be at least 1");
		goto out;
	}

	if (object_size == 0) {
		OSAL_LOG_ERROR("invalid object size specified. object_size: %d err: %d",
			       object_size, err);
		goto out;
	}

	if ((align_size != PNSO_MEM_ALIGN_NONE) && !is_power_of_2(align_size)) {
		OSAL_LOG_ERROR("invalid alignment size specified. align_size: %d err: %d",
			       align_size, err);
		goto out;
	}

	if (!out_mpool) {
		OSAL_LOG_ERROR("invalid pointer for pool specified. out_mpool: %p err: %d",
			       out_mpool, err);
		goto out;
	}

	/* allocate memory for pool, objects, and its stack */
	mpool = osal_alloc(sizeof(struct mem_pool));
	if (!mpool) {
		err = ENOMEM;
		OSAL_LOG_ERROR("failed to allocate memory for pool! mpool_type: %s num_objects: %d err: %d",
			       mpool_get_type_str(mpool_type),
			       num_objects, err);
		goto out;
	}
	memset(mpool, 0, sizeof(*mpool));
	mpool->mp_magic = MPOOL_MAGIC_VALID;
	mpool->mp_config.mpc_type = mpool_type;
	mpool->mp_config.mpc_num_objects = num_objects;
	mpool->mp_config.mpc_num_vec_elems = num_vec_elems;
	mpool->mp_config.mpc_align_size = align_size;

	/* compute pad and total pool size */
	mpool->mp_config.mpc_object_size = object_size;
	mpool->mp_config.mpc_vec_elem_size = object_size * num_vec_elems;
	mpool->mp_config.mpc_pad_size =
		mpool_get_pad_size(mpool->mp_config.mpc_vec_elem_size,
				align_size);
	mpool->mp_config.mpc_pool_size = ((mpool->mp_config.mpc_vec_elem_size +
				mpool->mp_config.mpc_pad_size) * num_objects);

	mpool->mp_objects = mpool_type_is_rmem(mpool_type) ?
			    mpool_create_rmem_objects(mpool) :
			    mpool_create_mem_objects(mpool);
	if (!mpool->mp_objects) {
		err = ENOMEM;
		goto out;
	}

	objects = osal_alloc(sizeof(void *) * num_objects);
	if (!objects) {
		err = ENOMEM;
		OSAL_LOG_ERROR("failed to allocate memory for stack objects! mpool_type: %d num_objects: %d err: %d",
			       mpool_type, num_objects, err);
		goto out;
	}

	mpool->mp_stack.mps_num_objects = num_objects;
	mpool->mp_stack.mps_objects = objects;
	spin_lock_init(&mpool->mp_stack.mps_lock);

	/* populate the stack to point the newly created objects */
	obj = (char *) mpool->mp_objects;
	for (i = 0; i < mpool->mp_config.mpc_num_objects; i++) {
		objects[i] = obj;
		OSAL_LOG_DEBUG("%30s[%d]: 0x"PRIx64" 0x"PRIx64" 0x"PRIx64" %u %u %u",
			       "mpool->mp_dstack.mps_objects", i,
			       (uint64_t) &objects[i], (uint64_t) objects[i],
			       (uint64_t) mpool_get_object_phy_addr(mpool_type,
				       objects[i]),
			       mpool->mp_config.mpc_vec_elem_size,
			       mpool->mp_config.mpc_pad_size, align_size);

		obj += (mpool->mp_config.mpc_vec_elem_size +
				mpool->mp_config.mpc_pad_size);
	}
	mpool->mp_stack.mps_top = mpool->mp_config.mpc_num_objects;

	*out_mpool = mpool;
	OSAL_LOG_INFO("pool allocated. mpool_type: %s num_objects: %d vec_elem_size: %d align_size: %d pad_size: %d mpool: 0x" PRIx64,
		      mpool_get_type_str(mpool_type), num_objects,
		      mpool->mp_config.mpc_vec_elem_size, align_size,
		      mpool->mp_config.mpc_pad_size, (uint64_t) mpool);

	err = PNSO_OK;
	return err;
out:
	OSAL_LOG_ERROR("failed to allocate pool!  mpool_type: %s num_objects: %d vec_elem_size: %d align_size: %d",
			mpool_get_type_str(mpool_type), num_objects,
			mpool->mp_config.mpc_vec_elem_size, align_size);
	if (mpool)
		mpool_destroy(&mpool);
	return err;
}

void
mpool_destroy(struct mem_pool **mpoolp)
{
	struct mem_pool *mpool;

	if (!mpoolp || !*mpoolp)
		return;

	mpool = *mpoolp;

	OSAL_LOG_INFO("pool deallocated. mpc_type: %s mpc_num_objects: %d mpool: 0x" PRIx64,
		      mpool_get_type_str(mpool->mp_config.mpc_type),
		      mpool->mp_config.mpc_num_objects, (uint64_t) mpool);

	/* TODO-mpool: for graceful exit, ensure stack top is back to full */
	mpool->mp_magic = MPOOL_MAGIC_INVALID;

	if (mpool_type_is_rmem(mpool->mp_config.mpc_type))
		mpool_destroy_rmem_objects(mpool);
	else
		mpool_destroy_mem_objects(mpool);
	osal_free(mpool->mp_stack.mps_objects);
	osal_free(mpool);

	*mpoolp = NULL;
}

void *
mpool_get_object(struct mem_pool *mpool)
{
	struct mem_pool_stack *mem_stack;
	void *object = NULL;

	if (!mpool)
		return NULL;

	if (!is_pool_valid(mpool))
		return NULL;

	mem_stack = &mpool->mp_stack;

	spin_lock(&mem_stack->mps_lock);
	if (mem_stack->mps_top > 0) {
		object = mem_stack->mps_objects[--(mem_stack->mps_top)];
		// MPOOL_VALIDATE_OBJECT(mpool, object);
	}
	spin_unlock(&mem_stack->mps_lock);

	return object;
}

void
mpool_put_object(struct mem_pool *mpool, void *object)
{
	struct mem_pool_stack *mem_stack;

	if (!mpool || !object)
		return;

	if (!is_pool_valid(mpool))
		return;

	MPOOL_VALIDATE_OBJECT(mpool, object);

	mem_stack = &mpool->mp_stack;
	spin_lock(&mem_stack->mps_lock);
	if (mem_stack->mps_top >= mem_stack->mps_num_objects) {
		spin_unlock(&mem_stack->mps_lock);
		OSAL_LOG_ERROR("cannot return object to pool! object: 0x" PRIx64 "  type: %s",
				(uint64_t) object,
				mpool_get_type_str(mpool->mp_config.mpc_type));
		OSAL_ASSERT(0);
		return;
	}

	mem_stack->mps_objects[mem_stack->mps_top] = object;
	mem_stack->mps_top++;
	spin_unlock(&mem_stack->mps_lock);
}

void __attribute__ ((unused))
mpool_pprint(const struct mem_pool *mpool)
{
	int i;
	void **objects;

	if (!mpool)
		return;

	OSAL_LOG_DEBUG("%-30s: 0x" PRIx64, "mpool", (uint64_t) mpool);
	// OSAL_LOG_DEBUG("%-30s: " PRIx64, "mpool->mp_magic", mpool->mp_magic);

	OSAL_LOG_DEBUG("%-30s: %u:%s", "mpool->mp_config.mpc_type",
			mpool->mp_config.mpc_type,
			mpool_get_type_str(mpool->mp_config.mpc_type));
	OSAL_LOG_DEBUG("%-30s: %u", "mpool->mp_config.mpc_num_objects",
			mpool->mp_config.mpc_num_objects);
	OSAL_LOG_DEBUG("%-30s: %u", "mpool->mp_config.mpc_num_vec_elems",
			mpool->mp_config.mpc_num_vec_elems);
	OSAL_LOG_DEBUG("%-30s: %u", "mpool->mp_config.mpc_object_size",
			mpool->mp_config.mpc_object_size);
	OSAL_LOG_DEBUG("%-30s: %u", "mpool->mp_config.mpc_vec_elem_size",
			mpool->mp_config.mpc_vec_elem_size);
	OSAL_LOG_DEBUG("%-30s: %u", "mpool->mp_config.mpc_align_size",
			mpool->mp_config.mpc_align_size);
	OSAL_LOG_DEBUG("%-30s: %u", "mpool->mp_config.mpc_pad_size",
			mpool->mp_config.mpc_pad_size);
	OSAL_LOG_DEBUG("%-30s: %u", "mpool->mp_config.mpc_pool_size",
			mpool->mp_config.mpc_pool_size);
	OSAL_LOG_DEBUG("%-30s: %u", "mpool->mp_config.mpc_num_allocs",
			mpool->mp_config.mpc_num_allocs);
	OSAL_LOG_DEBUG("%-30s: %u", "mpool->mp_config.mpc_page_size",
			mpool->mp_config.mpc_page_size);

	OSAL_LOG_DEBUG("%-30s: 0x" PRIx64, "mpool->mp_objects",
			(uint64_t) mpool->mp_objects);

	OSAL_LOG_DEBUG("%-30s: %d", "mpool->mp_stack.mps_num_objects",
			mpool->mp_stack.mps_num_objects);
	OSAL_LOG_DEBUG("%-30s: %d", "mpool->mp_stack.mps_top",
			mpool->mp_stack.mps_top);
	OSAL_LOG_DEBUG("%-30s: 0x" PRIx64, "mpool->mp_stack.mps_objects",
			(uint64_t) mpool->mp_stack.mps_objects);

	objects = mpool->mp_stack.mps_objects;
	for (i = 0; i < mpool->mp_config.mpc_num_objects; i++) {
		OSAL_LOG_DEBUG("%30s[%d]: 0x" PRIx64 " 0x" PRIx64 "",
				"mpool->mp_stack.mps_objects", i,
				(uint64_t) &objects[i], (uint64_t) objects[i]);
	}
}
