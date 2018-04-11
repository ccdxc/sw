/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_DPOOL_H__
#define __PNSO_DPOOL_H__

enum desc_pool_type {
	DPOOL_TYPE_NONE = 0,
	DPOOL_TYPE_CPDC,
	DPOOL_TYPE_CPDC_SGL,
	DPOOL_TYPE_CPDC_STATUS_NOSHA,
	DPOOL_TYPE_CPDC_STATUS_SHA256,
	DPOOL_TYPE_CPDC_STATUS_SHA512,
	DPOOL_TYPE_XTS,
	DPOOL_STATE_MAX
};

struct desc_pool_stack {
	uint32_t	dps_num_dbufs;	/* total number of descriptors */
	uint32_t	dps_top;	/* stack pointer */
	void		**dps_descs;	/* array of pointers to dbufs */
} __attribute__ ((packed, aligned));

struct desc_pool_config {
	enum desc_pool_type	dpc_type;	/* cpdc/xts/sgl pool */
	uint32_t		dpc_num_dbufs;	/* total number of entries */
	uint32_t		dpc_dbuf_size;	/* size of a descriptor */
} __attribute__ ((packed, aligned));

struct desc_pool {
	uint64_t		dp_magic;
	bool			dp_destroy;

	struct desc_pool_config	dp_config;

	void			*dp_dbufs;	/* array of descriptors */
	struct desc_pool_stack	dp_dstack;	/* stack for dbufs */
} __attribute__ ((packed, aligned));

pnso_error_t dpool_create(enum desc_pool_type dpool_type,
		uint32_t num_dbufs, struct desc_pool **out_dpool);

void dpool_destroy(struct desc_pool **dpool);

void *dpool_get_desc(struct desc_pool *dpool);

pnso_error_t dpool_put_desc(struct desc_pool *dpool, void *dbuf);

void dpool_pprint(struct desc_pool *dpool);

#endif /* __PNSO_DPOOL_H__ */
