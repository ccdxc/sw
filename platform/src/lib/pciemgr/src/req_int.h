/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __REQ_INT_H__
#define __REQ_INT_H__

/*
 * Common interface for tgt_req_notify_int and tgt_req_indirect_int.
 */

void
req_int_set(const u_int64_t reg, const u_int64_t addr, const u_int32_t data);

void
req_int_get(const u_int64_t reg, u_int64_t *addrp, u_int32_t *datap);

#endif /* __REQ_INT_H__ */
