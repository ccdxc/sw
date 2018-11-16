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

#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/mman.h>
#include <linux/module.h>
#include <linux/printk.h>

#include "osal_logger.h"
#include "sonic_dev.h"
#include "sonic_lif.h"
#include "sonic_interrupt.h"

#define evid_to_db_pa(evl, id) (evl->db_base_pa + (sizeof(uint64_t) * (id)))
#define db_pa_to_evid(evl, addr) (((dma_addr_t)(addr) - evl->db_base_pa) / sizeof(uint64_t))
#define evid_to_db_va(evl, id) (evl->db_base + (sizeof(uint64_t) * (id)))
#define db_va_to_evid(evl, addr) (((void*)(addr) - evl->db_base) / sizeof(uint64_t))
#define evid_to_work(evl, id) (&evl->work_data[(id)])
#define work_to_evid(evl, addr) (((void*)(addr) - (void*)evl->work_data) / sizeof(struct sonic_work_data))

static int sonic_get_evid(struct sonic_event_list *evl, u32 *evid)
{
	unsigned long irqflags;
	u32 id;

	if (!evl)
		return -EINVAL;

	spin_lock_irqsave(&evl->inuse_lock, irqflags);

	id = find_next_zero_bit(evl->inuse_evid_bmp, evl->size_ev_bmp, evl->next_evid);
	if (id < evl->size_ev_bmp)
		goto found;

	id = find_first_zero_bit(evl->inuse_evid_bmp, evl->next_evid);
	if (id < evl->next_evid)
		goto found;

	spin_unlock_irqrestore(&evl->inuse_lock, irqflags);

	/* not found */
	return -ENOMEM;

found:
	set_bit(id, evl->inuse_evid_bmp);
	evl->next_evid = id + 1;

	spin_unlock_irqrestore(&evl->inuse_lock, irqflags);

	*evid = id;

	return 0;
}

static void sonic_put_evid(struct sonic_event_list *evl, u32 evid)
{
	unsigned long irqflags;

	spin_lock_irqsave(&evl->inuse_lock, irqflags);
	clear_bit(evid, evl->inuse_evid_bmp);
	spin_unlock_irqrestore(&evl->inuse_lock, irqflags);
}

int sonic_poll_ev_list(struct sonic_event_list *evl, int budget)
{
	uint32_t id;
	struct sonic_work_data *work;
	int found = 0;

	while (found < budget) {
		id = find_next_bit(evl->inuse_evid_bmp, evl->size_ev_bmp, evl->next_used_evid);
		if (id >= evl->size_ev_bmp)
			break;
		evl->next_used_evid = id + 1;
		work = evid_to_work(evl, id);
		queue_work(evl->wq, &work->work);
		found++;
	}
	
	while (found < budget) {
		id = find_first_bit(evl->inuse_evid_bmp, evl->next_used_evid);
		if (id >= evl->next_used_evid)
			break;
		evl->next_used_evid = id + 1;
		work = evid_to_work(evl, id);
		queue_work(evl->wq, &work->work);
		found++;
	}

	return found;
}

irqreturn_t sonic_async_ev_isr(int irq, void *evlptr)
{
	struct sonic_event_list *evl = (struct sonic_event_list *) evlptr;
	unsigned long irqflags;
	int npolled;
	bool was_armed;

	was_armed = xchg(&evl->armed, false);

	if (unlikely(!evl->enable) || !was_armed)
		return IRQ_HANDLED;

	spin_lock_irqsave(&evl->inuse_lock, irqflags);

	npolled = sonic_poll_ev_list(evl, SONIC_ASYNC_BUDGET);

	spin_unlock_irqrestore(&evl->inuse_lock, irqflags);

	if (!npolled)
		xchg(&evl->armed, true);

	OSAL_LOG_NOTICE("sonic_interrupt isr enqueued %d work items", npolled);

	return IRQ_HANDLED;
}

static void work_handler(struct work_struct *work)
{
	struct sonic_work_data *swd = (struct sonic_work_data *) work;
	struct sonic_event_list *evl = swd->evl;
	unsigned long irqflags;
	bool completed = false;

	spin_lock_irqsave(&evl->inuse_lock, irqflags);

	/* TODO */
	if (sonic_pnso_async_poll(swd->data)) {
		/* Done, release it */
		clear_bit(work_to_evid(evl, swd), evl->inuse_evid_bmp);
		swd->data = 0;
		swd->loop_count = 0;
		completed = true;
	} else {
		/* Not done, reenqueue */
		swd->loop_count++;
		queue_work(evl->wq, &swd->work);
	}

	spin_unlock_irqrestore(&evl->inuse_lock, irqflags);
	if (completed) {
		xchg(&evl->armed, true);
		sonic_intr_return_credits(&evl->pc_res->intr, 1,
					  false, false);
		OSAL_LOG_NOTICE("sonic_interrupt handled a work item, success!");
	} else {
		OSAL_LOG_NOTICE("sonic_interrupt handled a work item, reenqueued");
	}
}

void *sonic_intr_get_db_addr(struct per_core_resource *pc_res)
{
	struct sonic_event_list *evl = pc_res->evl;
	uint32_t evid;

	if (!evl || !evl->db_base)
		return NULL;

	if (sonic_get_evid(evl, &evid) != 0)
		return NULL;

	return (void *) evid_to_db_pa(evl, evid);
}

void sonic_intr_put_db_addr(struct per_core_resource *pc_res, void *addr)
{
	struct sonic_event_list *evl = pc_res->evl;
	uint32_t evid;

	if (!evl || !evl->db_base)
		return;

	evid = db_pa_to_evid(evl, addr);
	if (evid < evl->size_ev_bmp)
		sonic_put_evid(evl, evid);
}

void sonic_destroy_ev_list(struct per_core_resource *pc_res)
{
	struct sonic_event_list *evl = pc_res->evl;

	if (!evl)
		return;

	evl->enable = false;

	if (evl->wq) {
		flush_workqueue(evl->wq);
		destroy_workqueue(evl->wq);
		evl->wq = NULL;
	}

	if (evl->work_data) {
		kfree(evl->work_data);
		evl->work_data = NULL;
	}

	if (evl->db_base) {
		dma_free_coherent(pc_res->lif->sonic->dev,
				  evl->db_total_size,
				  evl->db_base, evl->db_base_pa);
		evl->db_base = 0;
	}

	kfree(evl);
	pc_res->evl = NULL;
}

int sonic_create_ev_list(struct per_core_resource *pc_res, uint32_t ev_count)
{
	struct sonic_event_list *evl = NULL;
	uint32_t total_size;
	uint32_t i;
	int rc = 0;

	if (ev_count > MAX_PER_CORE_EVENTS) {
		OSAL_LOG_WARN("Truncating event count from %u to %u",
			      ev_count, (uint32_t) MAX_PER_CORE_EVENTS);
		ev_count = MAX_PER_CORE_EVENTS;
	}

	evl = kzalloc(sizeof(*evl), GFP_KERNEL);
	if (!evl) {
		OSAL_LOG_ERROR("Failed to alloc %u bytes for evl",
			       (uint32_t) sizeof(*evl));
		rc = -ENOMEM;
		goto err_evl;
	}

	spin_lock_init(&evl->inuse_lock);
	pc_res->evl = evl;
	evl->pc_res = pc_res;
	evl->size_ev_bmp = ev_count;
	evl->armed = true;
	evl->enable = false;

	evl->db_total_size = sizeof(uint64_t) * ev_count;
	evl->db_base = dma_alloc_coherent(pc_res->lif->sonic->dev,
					  evl->db_total_size,
					  &evl->db_base_pa, GFP_KERNEL);
	if (!evl->db_base) {
		OSAL_LOG_ERROR("Failed to dma_alloc %u bytes for db",
			       evl->db_total_size);
		rc = -ENOMEM;
		goto err_evl;
	}

	/* Setup workqueue entries */
	total_size = sizeof(struct sonic_work_data) * ev_count;
	evl->work_data = kzalloc(total_size, GFP_KERNEL);
	if (!evl->work_data) {
		OSAL_LOG_ERROR("Failed to alloc %u bytes for work_data",
			       total_size);
		rc = -ENOMEM;
		goto err_evl;
	}
	for (i = 0; i < ev_count; i++) {
		INIT_WORK(&evl->work_data[i].work, work_handler);
		evl->work_data[i].evl = evl;
	}

	/* Create workqueue */
	evl->wq = create_singlethread_workqueue("sonic_async");
	if (!evl->wq) {
		OSAL_LOG_ERROR("Failed to create_singlethread_workqueue");
		rc = -ENOMEM;
		goto err_evl;
	}
	
	OSAL_LOG_NOTICE("Successfully created event list");
	evl->enable = true;
	return 0;

err_evl:
	OSAL_LOG_ERROR("Failed to create event list");
	sonic_destroy_ev_list(pc_res);
	return rc;
}

