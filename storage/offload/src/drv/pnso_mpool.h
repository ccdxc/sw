/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_MPOOL_H__
#define __PNSO_MPOOL_H__

#include <linux/spinlock.h>

#include "osal_assert.h"
#include "osal_mem.h"
#include "sonic_api_int.h"

/*
 * This file contain constants, declarations and functions to manage the
 * fixed-size memory pools (mpool) for Pensando Storage Accelerators.
 *
 * There are few types of hardware descriptors, data structures, or buffers
 * (a.k.a. objects) heavily used in enabling the CPDC and CRYPTO accelerator
 * functionality.  The goal of mpool is to prevent dynamic allocations and
 * deallocations of objects in IO path.  mpool attempts to maintain pools
 * of such objects aligned to a default or requested size, pre-allocated
 * (on per-core basis as initiated by callers), and to provide quick access
 * to the objects as and when needed.
 *
 * During accelerators' startup and shutdown phase, pool creation and
 * destruction takes place. mpool uses a stack data structure to keep track
 * of free-objects. Callers will have to obtain/pop one object at a time from
 * the stack, and release/push upon its usage.
 *
 * Current implementation does not enable callers to obtain two or more
 * contiguous objects automatically.  In some scenarios, HW wants a group of
 * command/status descriptors in contiguous memory.  As mpool does not provide
 * this ability, however, mpool can be created such that one object encompasses
 * several such descriptors.  This gives the ablility to reference a vector/bulk
 * of such descriptors through object.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * TODO-mpool:
 *	- move PNSO_MAX to common/util
 *	- enhance: mpool stats, header/footer in objects as/if needed
 *	- use osal spin/lock
 *
 */
/* unit of following constants is bytes */
#define PNSO_MEM_ALIGN_NONE	0
#define PNSO_MEM_ALIGN_DESC	64	/* cpdc/sgl desc */
#define PNSO_MEM_ALIGN_BUF	256
#define PNSO_MEM_ALIGN_PAGE	4096

#define MPOOL_VEC_ELEM_SINGLE	1

/* Different types of objects */
enum mem_pool_type {
	MPOOL_TYPE_NONE = 0,
	MPOOL_TYPE_CPDC_DESC,
	MPOOL_TYPE_CPDC_DESC_VECTOR,
	MPOOL_TYPE_CPDC_DESC_PB_VECTOR,		/* for per-block hash/chksum */
	MPOOL_TYPE_CPDC_SGL,
	MPOOL_TYPE_CPDC_SGL_VECTOR,		/* 5 */
	MPOOL_TYPE_CPDC_STATUS_DESC,
	MPOOL_TYPE_CPDC_STATUS_DESC_VECTOR,
	MPOOL_TYPE_CRYPTO_DESC,
	MPOOL_TYPE_CRYPTO_DESC_VECTOR,
	MPOOL_TYPE_CRYPTO_STATUS_DESC,		/* 10 */
	MPOOL_TYPE_CRYPTO_AOL,
	MPOOL_TYPE_CRYPTO_AOL_VECTOR,
	MPOOL_TYPE_CHAIN_SGL_PDMA,
	MPOOL_TYPE_SERVICE_CHAIN,
	MPOOL_TYPE_SERVICE_CHAIN_ENTRY,		/* 15 */
	MPOOL_TYPE_BATCH_PAGE,
	MPOOL_TYPE_BATCH_INFO,
	/* All rmem types start from this range onward */
	MPOOL_TYPE_RMEM_START,
	MPOOL_TYPE_RMEM_INTERM_CRYPTO_STATUS = MPOOL_TYPE_RMEM_START,
	MPOOL_TYPE_RMEM_INTERM_CPDC_STATUS,
	MPOOL_TYPE_RMEM_INTERM_BUF,
	MPOOL_TYPE_MAX
};

struct mem_pool_stack {
	uint32_t mps_num_objects;	/* total number of objects */
	uint32_t mps_top;		/* stack pointer */
	spinlock_t mps_lock;		/* to serialize get/put in poll/async */
	void **mps_objects;		/* array of pointers to objects */
};

struct mem_pool_config {
	enum mem_pool_type mpc_type;	/* cpdc/crypto/sgl/etc pool */
	uint32_t mpc_num_allocs;	/* total number of object allocs */
	uint32_t mpc_num_objects;	/* total number of objects */
	uint32_t mpc_num_vec_elems;	/* total number of vector elements */
	uint32_t mpc_object_size;	/* size of an object */
	uint32_t mpc_vec_elem_size;	/* size of a vector element */
	uint32_t mpc_align_size;	/* object alignment size */
	uint32_t mpc_pad_size;		/* from object and align size */
	uint32_t mpc_pool_size;		/* total pool size */
	uint32_t mpc_page_size;		/* page size, if any */
};

struct mem_pool {
	uint64_t mp_magic;		/* for debugging purposes */
	struct mem_pool_config mp_config;	/* basic pool config */
	void *mp_objects;		/* array of objects */
	struct mem_pool_stack mp_stack;	/* stack for objects */
};

/**
 * mpool_create() - creates a memory pool.
 * @mpool_type:		[in]	specifies the type of mpool.
 * @num_objects:	[in]	specifies the maximum number of objects the
 *				pool to hold.
 * @num_vec_elems:	[in]	specifies the maximum number of vector elements
 *				comprising the object.
 * @object_size:	[in]	specifies the size of an object in bytes.
 * @align_size:		[in]	specifies the alignment size of an object.
 * @out_mpool:		[out]	specifies the pointer to the newly created pool.
 *
 * This routine internally decides the size of the pool based on the requested
 * number of objects and its alignment size.
 *
 * Return Value:
 *	PNSO_OK	- on success
 *	EINVAL	- on invalid input parameters (ex: mpool type, maximum
 *		number of objects, etc.)
 *	EEXIST	- if the mpool was created already
 *	ENOMEM	- on failing to allocate memory
 *
 */
pnso_error_t mpool_create(enum mem_pool_type mpool_type,
		uint32_t num_objects, uint32_t num_vec_elems,
		uint32_t object_size, uint32_t align_size,
		struct mem_pool **out_mpool);

/**
 * mpool_destroy() - destroys the specified type of pool.
 * @mpool:	[in/out]	specifies the pointer to the mpool to be
 *				destroyed.  Upon successful destruction,
 *				this routine NULL-ifies the pointer.
 *
 * Return Value:
 *	None
 *
 */
void mpool_destroy(struct mem_pool **mpool);

/**
 * mpool_get_object() - obtains a pointer to an object within the pool.
 * @mpool:	[in]	specifies the pointer to the mpool.
 *
 * Return Value:
 *	- NULL if specified pool is NULL, or if all objects are in-use
 *	- a pointer to the object
 *
 */
void *mpool_get_object(struct mem_pool *mpool);

/**
 * mpool_put_object() - releases an object back to the pool.
 * @mpool:	[in]	specifies the pointer to the mpool.
 * @object:	[in]	specifies the pointer to the object.
 *
 * Return Value:
 *	None
 *
 */
void mpool_put_object(struct mem_pool *mpool, void *object);

/**
 * mpool_get_pad_size() - returns the number of padding-bytes for the specified
 * object and alignment size.
 * object and align size.
 * @object_size:	[in]	specifies size of the object in bytes.
 * @align_size:		[in]	specifies the alignment size.
 *
 * Return Value:
 *	- number of padding-bytes.
 *
 */
uint32_t mpool_get_pad_size(uint32_t object_size, uint32_t align_size);

/**
 * mpool_pprint() - prints details of the pool.
 * @mpool:	[in]	specifies the pointer to the mpool.
 *
 * Return Value:
 *	None
 *
 */
void mpool_pprint(const struct mem_pool *mpool);

#ifdef NDEBUG
#define MPOOL_PPRINT(mpool)
#else
#define MPOOL_PPRINT(mpool) mpool_pprint(mpool)
#endif

const char *mpool_get_type_str(enum mem_pool_type mpool_type);

static inline bool
mpool_type_is_valid(enum mem_pool_type mpool_type)
{
	return (mpool_type > MPOOL_TYPE_NONE) && (mpool_type < MPOOL_TYPE_MAX);
}

static inline bool
mpool_type_is_rmem(enum mem_pool_type mpool_type)
{
	return mpool_type >= MPOOL_TYPE_RMEM_START;
}

static inline uint64_t
mpool_get_object_phy_addr(enum mem_pool_type mpool_type,
			  void *object)
{
	if (mpool_type_is_rmem(mpool_type))
		return (uint64_t)object;
	else
		return sonic_virt_to_phy(object);
}

static inline void *
mpool_get_object_alloc_addr(enum mem_pool_type mpool_type,
			    uint64_t object_pa)
{
	if (mpool_type_is_rmem(mpool_type))
		return (void *)object_pa;
	else
		return sonic_phy_to_virt(object_pa);
}

static inline uint32_t
mpool_get_object_size(const struct mem_pool *mpool)
{
	return mpool->mp_config.mpc_vec_elem_size;
}

static inline void
mpool_clear_object(const struct mem_pool *mpool,
		   void *object)
{
	if (mpool_type_is_rmem(mpool->mp_config.mpc_type))
		sonic_rmem_set((uint64_t)object, 0,
				mpool_get_object_size(mpool));
	else
		memset(object, 0, mpool_get_object_size(mpool));
}

static inline uint32_t
mpool_get_object_num_vec_elems(const struct mem_pool *mpool)
{
	return mpool->mp_config.mpc_num_vec_elems;
}

static inline uint32_t
mpool_get_object_page_size(const struct mem_pool *mpool)
{
	return mpool->mp_config.mpc_page_size;
}

static inline uint32_t
mpool_get_object_pad_size(const struct mem_pool *mpool)
{
	return mpool->mp_config.mpc_pad_size;
}

static inline uint32_t
mpool_get_object_num_allocs(const struct mem_pool *mpool)
{
	return mpool->mp_config.mpc_num_allocs;
}

/*
 * Ensure there's no loss of significant bits converting
 * from uint64_t to void *.
 */
static inline void
mpool_void_ptr_check(uint64_t v)
{
	void *p = (void *)v;

	OSAL_ASSERT((sizeof(void *) >= sizeof(uint64_t)) ||
		    ((uint64_t)p == v));
}

#ifdef __cplusplus
}
#endif

#endif /* __PNSO_MPOOL_H__ */
