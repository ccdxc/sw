/*
 * Copyright (c) 2018 Pensando Systems, Inc.  All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef SONIC_INTERRUPT_H
#define SONIC_INTERRUPT_H

#include <linux/device.h>
#include <linux/workqueue.h>
#include "sonic_dev.h"

#define MAX_PER_CORE_EVENTS MAX_PER_QUEUE_SQ_ENTRIES
#define SONIC_ASYNC_BUDGET 10

enum sonic_intr_bits {
	/* mask (and mask-on-assert) values */
	SONIC_INTR_MASK_CLEAR		= 0u, /* unmask interrupts */
	SONIC_INTR_MASK_SET		= 1u, /* mask interrupts */
};

struct sonic_event_list;

struct sonic_event_data {
	int evid;
	uint64_t data;
};


struct sonic_db_data {
	uint64_t usr_data;
	uint32_t fired;		/* 'fired' must immediately */
	uint32_t primed;	/* precede 'primed' */
};

struct sonic_work_data {
	struct work_struct work;
	struct sonic_event_list *evl;
	struct sonic_event_data ev_data[SONIC_ASYNC_BUDGET];
	uint64_t timestamp;
	uint32_t ev_count;
};

struct sonic_event_list {
	struct per_core_resource *pc_res;
	bool			enable;
	bool			armed;
	int			idle_count;

	int			irq;
	char			name[32];

	struct workqueue_struct *wq;
	struct sonic_work_data work_data;

	spinlock_t inuse_lock;
	int next_evid;
	int next_used_evid;
	int size_ev_bmp;
	DECLARE_BITMAP(inuse_evid_bmp, MAX_PER_CORE_EVENTS);
	uint32_t db_total_size;
	void *db_base;
	dma_addr_t db_base_pa;
};

int sonic_create_ev_list(struct per_core_resource *pc_res, uint32_t ev_count);
void sonic_destroy_ev_list(struct per_core_resource *pc_res);
irqreturn_t sonic_async_ev_isr(int irq, void *evlptr);

#endif /* SONIC_INTERRUPT_H */
