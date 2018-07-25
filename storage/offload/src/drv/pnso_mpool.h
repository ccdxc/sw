/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_MPOOL_H__
#define __PNSO_MPOOL_H__

/*
 * This file contain constants, declarations and functions to manage the
 * fixed-size memory pools (mpool) for Pensando Storage Accelerators.
 *
 * There are few types of hardware descriptors, data structures, or buffers
 * (a.k.a. objects) heavily used in enabling the CPDC and XTS accelerator
 * functionality.  The goal of mpool is to prevent dynamic allocations and
 * deallocations of objects in IO path.  mpool attempts to maintain pools
 * of such objects aligned to a default or requested size, pre-allocated
 * (on per-core basis as initiated by callers), and to provide quick access
 * to the objects as and when needed.
 *
 * During accelerators' startup and shutdown phase, pool creation and
 * destruction takes place. mpool uses a stack data structure to keep track
 * of free-objects. Callers will have to obtain/pop one object at a time from
 * the stack, and release/push upon its usage.  Current implementation does
 * not enable callers to obtain two or more contiguous objects automatically.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * TODO-mpool:
 *	- fix aribitrary min/max: PNSO_XXX_NUM_POOL_OBJECTS
 *	- move default align sizes, min/max objects to global.h for caller(s)
 *	to specify
 *	- move PNSO_MAX to common/util
 *	- enhance: mpool stats, header/footer in objects as/if needed
 *
 */
#define PNSO_MIN_NUM_POOL_OBJECTS	32
#define PNSO_MIN_NUM_CPDC_DESC		PNSO_MIN_NUM_POOL_OBJECTS
#define PNSO_MIN_NUM_CPDC_SGL_DESC	PNSO_MIN_NUM_POOL_OBJECTS
#define PNSO_MIN_NUM_CPDC_STATUS_DESC	PNSO_MIN_NUM_POOL_OBJECTS
#define PNSO_MIN_NUM_XTS_DESC		PNSO_MIN_NUM_POOL_OBJECTS

#define PNSO_MAX_NUM_POOL_OBJECTS	512
#define PNSO_MAX_NUM_CPDC_DESC		PNSO_MAX_NUM_POOL_OBJECTS
#define PNSO_MAX_NUM_CPDC_SGL_DESC	(PNSO_MAX_NUM_POOL_OBJECTS * 2)
#define PNSO_MAX_NUM_CPDC_STATUS_DESC	PNSO_MAX_NUM_POOL_OBJECTS
#define PNSO_MAX_NUM_XTS_DESC		PNSO_MAX_NUM_POOL_OBJECTS
#define PNSO_MAX_NUM_AOL		(PNSO_MAX_NUM_POOL_OBJECTS * 2)

#define PNSO_NUM_OBJECTS_WITHIN_OBJECT	16

/* unit of following constants is bytes */
#define PNSO_MEM_ALIGN_DESC	64	/* cpdc/sgl/aol/xts desc */
#define PNSO_MEM_ALIGN_BUF	256
#define PNSO_MEM_ALIGN_PAGE	4096

/*
 * TODO-mpool:
 *
 * Rename
 *	MPOOL_TYPE_CPDC => MPOOL_TYPE_CPDC_DESC
 *	MPOOL_TYPE_CPDC_STATUS ==> MPOOL_TYPE_CPDC_STATUS_DESC
 *	MPOOL_TYPE_XTS => MPOOL_TYPE_XTS_DESC
 *	MPOOL_TYPE_AOL => MPOOL_TYPE_XTS_AOL
 * Fix
 *	their strings
 */
/* Different types of objects */
enum mem_pool_type {
	MPOOL_TYPE_NONE = 0,
	MPOOL_TYPE_CPDC,
	MPOOL_TYPE_CPDC_SGL,
	MPOOL_TYPE_CPDC_STATUS,
	MPOOL_TYPE_XTS,
	MPOOL_TYPE_AOL,
	MPOOL_TYPE_CHAIN_ENTRY,
	MPOOL_TYPE_MAX
};

struct mem_pool_stack {
	uint32_t mps_num_objects;	/* total number of objects */
	uint32_t mps_top;		/* stack pointer */
	void **mps_objects;		/* array of pointers to objects */
};

struct mem_pool_config {
	enum mem_pool_type mpc_type;	/* cpdc/xts/sgl/etc pool */
	uint32_t mpc_num_objects;	/* total number of objects */
	uint32_t mpc_object_size;	/* size of an object */
	uint32_t mpc_align_size;	/* object alignment size */
	uint32_t mpc_pad_size;		/* from object and align size */
	uint32_t mpc_pool_size;		/* total pool size */
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
 * @object_size:	[in]	specifies the size of an object in bytes.
 * @align_size:		[in]	specifies the alignment size of an object.
 * @out_mpool:		[out]	specifies the pointer to the newly created pool.
 *
 * This routine internally decides the size of the pool based on the requested
 * number of objects and its alignment size.
 *
 * Return Value:
 *	PNSO_OK	- on success
 *	-EINVAL	- on invalid input parameters (ex: mpool type, maximum
 *		number of objects, etc.)
 *	-EEXIST	- if the mpool was created already
 *	-ENOMEM	- on failing to allocate memory
 *
 */
pnso_error_t mpool_create(enum mem_pool_type mpool_type,
		uint32_t num_objects, uint32_t object_size,
		uint32_t align_size, struct mem_pool **out_mpool);

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
 * mpool_put_object() - releases an object back to the pool.
 * @mpool:	[in]	specifies the pointer to the mpool.
 * @object:	[in]	specifies the pointer to the object.
 *
 * Return Value:
 *	PNSO_OK	- on success
 *	-EINVAL	- on invalid input parameters
 *
 */
pnso_error_t mpool_put_object(struct mem_pool *mpool, void *object);

/**
 * mpool_pprint() - prints details of the pool.
 * @mpool:	[in]	specifies the pointer to the mpool.
 *
 * Return Value:
 *	None
 *
 */
void mpool_pprint(const struct mem_pool *mpool);

#ifdef __cplusplus
}
#endif

#endif /* __PNSO_MPOOL_H__ */
