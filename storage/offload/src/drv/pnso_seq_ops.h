/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_SEQ_OPS_H__
#define __PNSO_SEQ_OPS_H__

#include "osal.h"
#include "pnso_chain.h"

#ifdef __cplusplus
extern "C" {
#endif

struct sequencer_ops {
	void * (*setup_desc)(struct service_info *svc_info,
			const void *src_desc, size_t desc_size);

	void (*ring_db)(const struct service_info *svc_info);
};

extern const struct sequencer_ops model_seq_ops;
extern const struct sequencer_ops hw_seq_ops;

#ifdef __cplusplus
}
#endif

#endif /* __PNSO_SEQ_OPS_H__ */
