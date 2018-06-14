/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_DPOOL_H__
#define __PNSO_DPOOL_H__

/*
 * This file contain constants, declarations and functions to manage the
 * descriptor pools (dpool).
 *
 * There are few types of descriptors/buffers (dbufs) that are heavily used
 * in enabling the CPDC and XTS accelerator functionality.  The goal of dpool
 * is to prevent dynamic allocations and deallocations of dbufs.  dpool
 * attempts to maintain pools of such descriptors pre-allocated on per-core
 * basis, and to provide quick access to the dbufs as and when needed.
 *
 * During accelerators' startup and shutdown phase, dpools creation and
 * destruction takes place. dpool uses a stack data structure to keep track
 * of free-dbufs. Callers will have to obtain/pop one dbuf at a time from the
 * stack, and return/push upon its usage.
 *
 */

/* Different types of descriptors */
enum desc_pool_type {
	DPOOL_TYPE_NONE = 0,
	DPOOL_TYPE_CPDC,
	DPOOL_TYPE_CPDC_SGL,
	DPOOL_TYPE_CPDC_STATUS,
	DPOOL_TYPE_XTS,
	DPOOL_TYPE_AOL,
	DPOOL_STATE_MAX
};

/* TODO-dpool: revisit on align/pack, dp_destroy */
struct desc_pool_stack {
	uint32_t dps_num_dbufs;	/* total number of descriptors */
	uint32_t dps_top;	/* stack pointer */
	void **dps_descs;	/* array of pointers to dbufs */
} __attribute__ ((packed, aligned));

struct desc_pool_config {
	enum desc_pool_type dpc_type;	/* cpdc/xts/sgl pool */
	uint32_t dpc_num_dbufs;		/* total number of entries */
	uint32_t dpc_dbuf_size;		/* size of a descriptor */
} __attribute__ ((packed, aligned));

struct desc_pool {
	uint64_t dp_magic;		/* for debugging purposes */
	bool dp_destroy;		/* to prevent pool use on destroy */
	struct desc_pool_config	dp_config;	/* basic pool config */
	void *dp_dbufs;				/* array of descriptors */
	struct desc_pool_stack dp_dstack;	/* stack for dbufs */
} __attribute__ ((packed, aligned));

/**
 * dpool_create() - creates a descriptor pool.
 * @dpool_type:	[in]	specifies the type of dpool.
 * @num_dbufs:	[in]	specifies the maximum number of descriptors the pool
 *			to hold.
 * @out_dpool:	[out]	specifies the pointer to the newly created pool.
 *
 * This routine internally decides the size of the pool based on the requested
 * type and number of dbufs.  Also, this routine internally decides on the
 * alignment of dbuf memory.
 *
 * Return Value:
 *	PNSO_OK	- on success
 *	-EINVAL	- on invalid input parameters (ex: pool type, maximum
 *		number of dbufs, etc.)
 *	-EEXIST	- if the dpool was created already
 *	-ENOMEM	- on failing to allocate memory
 *
 */
pnso_error_t dpool_create(enum desc_pool_type dpool_type,
		uint32_t num_dbufs, struct desc_pool **out_dpool);

/**
 * dpool_destroy() - destroys the specified type of pool.
 * @dpool:	[in/out]	specifies the pointer to the dpool to be
 *				destroyed.  Upon successful destruction,
 *				this routine NULL-ifies the pointer.
 *
 * Return Value:
 *	None
 *
 */
void dpool_destroy(struct desc_pool **dpool);

/**
 * dpool_get_desc() - obtains a descriptor buffer.
 * @dpool:	[in]	specifies the pointer to the dpool.
 *
 * Return Value:
 *	- NULL if specified pool is NULL, or if all dbufs are in-use
 *	- a pointer to the dbuf memory
 *
 */
void *dpool_get_desc(struct desc_pool *dpool);

/**
 * dpool_put_desc() - releases a descriptor buffer back to the pool.
 * @dpool:	[in]	specifies the pointer to the dpool.
 * @dbuf:	[in]	specifies the pointer to the dbuf.
 *
 * Return Value:
 *	PNSO_OK	- on success
 *	-EINVAL	- on invalid input parameters
 *
 */
pnso_error_t dpool_put_desc(struct desc_pool *dpool, void *dbuf);

/**
 * dpool_pprint() - prints details of the descriptor pool.
 * @dpool:	[in]	specifies the pointer to the dpool.
 *
 * Return Value:
 *	None
 *
 */
void dpool_pprint(const struct desc_pool *dpool);

#endif /* __PNSO_DPOOL_H__ */
