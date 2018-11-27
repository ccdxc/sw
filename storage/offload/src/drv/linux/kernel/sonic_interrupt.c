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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
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
	uint32_t id, first_id, next_id;
	uint32_t loop_count = 0;
	uint64_t *data;
	struct sonic_work_data *work;
	int found = 0;

	first_id = evl->next_used_evid;
	next_id = first_id;
	while (found < budget && loop_count < MAX_PER_CORE_EVENTS) {
		id = find_next_bit(evl->inuse_evid_bmp, evl->size_ev_bmp, next_id);
		if (id >= evl->size_ev_bmp) {
			id = find_first_bit(evl->inuse_evid_bmp, first_id);
			if (id >= first_id)
				break;
		}
		next_id = id + 1;
		data = (uint64_t *) evid_to_db_va(evl, id);
		if (*data) {
			//OSAL_LOG_NOTICE("TODO: found ev id %d with data 0x%llx\n",
			//		id, (unsigned long long) *data);
			evl->next_used_evid = next_id;
			work = evid_to_work(evl, id);
			work->data = *data;
			*data = 0;
			queue_work(evl->wq, &work->work);
			found++;
		//} else {
		//	OSAL_LOG_NOTICE("TODO: found ev id %d without data\n", id);
		}
		loop_count++;
	}

	return found;
}

static void sonic_ev_work_handler(struct work_struct *work)
{
	struct sonic_work_data *swd = container_of(work, struct sonic_work_data, work);
	struct sonic_event_list *evl = swd->evl;
	unsigned long irqflags;
	bool completed = false;
	uint32_t evid = work_to_evid(evl, swd);

	OSAL_LOG_NOTICE("sonic_ev_work_handler enter (evid %u)...\n", evid);

	if (pnso_request_poller((void *) swd->data) == PNSO_OK) {
		/* Done, release it */
		spin_lock_irqsave(&evl->inuse_lock, irqflags);
		clear_bit(evid, evl->inuse_evid_bmp);
		swd->data = 0;
		swd->loop_count = 0;
		spin_unlock_irqrestore(&evl->inuse_lock, irqflags);
		completed = true;
	} else {
		/* TODO: improve timeout handling */
		if (swd->loop_count > 1000) {
			OSAL_LOG_WARN("dropping work item %u due to excessive loops\n", evid);
			spin_lock_irqsave(&evl->inuse_lock, irqflags);
			clear_bit(evid, evl->inuse_evid_bmp);
			swd->data = 0;
			swd->loop_count = 0;
			spin_unlock_irqrestore(&evl->inuse_lock, irqflags);
			completed = true;
		} else {
			/* Not done, reenqueue */
			swd->loop_count++;
			queue_work(evl->wq, &swd->work);
		}
	}

	if (completed) {
		//xchg(&evl->armed, true);
		sonic_intr_return_credits(&evl->pc_res->intr, 1,
					  true, false);
		//sonic_intr_mask(&evl->pc_res->intr, false);
		OSAL_LOG_NOTICE("... exit sonic_ev_work_handler evid %u, status success\n", evid);
	} else {
		OSAL_LOG_NOTICE("... exit sonic_ev_work_handler evid %u, status reenqueued\n", evid);
	}
}

irqreturn_t sonic_async_ev_isr(int irq, void *evlptr)
{
	struct sonic_event_list *evl = (struct sonic_event_list *) evlptr;
	int npolled;
	bool was_armed;

	was_armed = xchg(&evl->armed, false);

	//OSAL_LOG_NOTICE("sonic_async_ev_isr enter ...\n");

	if (unlikely(!evl->enable) || !was_armed) {
		//OSAL_LOG_WARN("... exit TODO: sonic_interrupt not armed\n");
		//sonic_intr_mask(&evl->pc_res->intr, false);
		return IRQ_HANDLED;
	}

	npolled = sonic_poll_ev_list(evl, SONIC_ASYNC_BUDGET);

	xchg(&evl->armed, true);

	//OSAL_LOG_NOTICE("... exit sonic_async_ev_isr, enqueued %d work items\n", npolled);

	return IRQ_HANDLED;
}

uint64_t sonic_intr_get_db_addr(struct per_core_resource *pc_res)
{
	struct sonic_event_list *evl = pc_res->evl;
	uint32_t evid;

	if (!evl || !evl->db_base) {
		OSAL_LOG_WARN("Invalid evl\n");
		return 0;
	}

	if (sonic_get_evid(evl, &evid) != 0) {
		OSAL_LOG_WARN("Cannot find free evid\n");
		return 0;
	}

	OSAL_LOG_WARN("Successfully allocated evid %u\n", evid);

	return sonic_hostpa_to_devpa((uint64_t) evid_to_db_pa(evl, evid));
}

void sonic_intr_put_db_addr(struct per_core_resource *pc_res, uint64_t addr)
{
	struct sonic_event_list *evl = pc_res->evl;
	uint32_t evid;

	if (!evl || !evl->db_base)
		return;

	evid = db_pa_to_evid(evl, sonic_devpa_to_hostpa(addr));
	if (evid < evl->size_ev_bmp)
		sonic_put_evid(evl, evid);
}

void sonic_destroy_ev_list(struct per_core_resource *pc_res)
{
	struct sonic_event_list *evl = pc_res->evl;
	struct device *dev = pc_res->lif->sonic->dev;

	if (!evl)
		return;

	evl->enable = false;

	if (evl->wq) {
		flush_workqueue(evl->wq);
		destroy_workqueue(evl->wq);
		evl->wq = NULL;
	}

	if (evl->work_data) {
		devm_kfree(dev, evl->work_data);
		evl->work_data = NULL;
	}

	if (evl->db_base) {
		dma_free_coherent(pc_res->lif->sonic->dev,
				  evl->db_total_size,
				  evl->db_base, evl->db_base_pa);
		evl->db_base = 0;
	}

	devm_kfree(dev, evl);
	pc_res->evl = NULL;
}

int sonic_create_ev_list(struct per_core_resource *pc_res, uint32_t ev_count)
{
	struct sonic_event_list *evl = NULL;
	struct device *dev = pc_res->lif->sonic->dev;
	uint32_t total_size;
	uint32_t i;
	int rc = 0;

	if (ev_count > MAX_PER_CORE_EVENTS) {
		OSAL_LOG_WARN("Truncating event count from %u to %u\n",
			      ev_count, (uint32_t) MAX_PER_CORE_EVENTS);
		ev_count = MAX_PER_CORE_EVENTS;
	}

	evl = devm_kzalloc(dev, sizeof(*evl), GFP_KERNEL);
	if (!evl) {
		OSAL_LOG_ERROR("Failed to alloc %u bytes for evl\n",
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
	evl->db_base = dma_alloc_coherent(dev,
					  evl->db_total_size,
					  &evl->db_base_pa, GFP_KERNEL);
	if (!evl->db_base) {
		OSAL_LOG_ERROR("Failed to dma_alloc %u bytes for db\n",
			       evl->db_total_size);
		rc = -ENOMEM;
		goto err_evl;
	}
	memset(evl->db_base, 0, evl->db_total_size);

	/* Setup workqueue entries */
	total_size = sizeof(struct sonic_work_data) * ev_count;
	evl->work_data = devm_kzalloc(dev, total_size, GFP_KERNEL);
	if (!evl->work_data) {
		OSAL_LOG_ERROR("Failed to alloc %u bytes for work_data\n",
			       total_size);
		rc = -ENOMEM;
		goto err_evl;
	}
	for (i = 0; i < ev_count; i++) {
		INIT_WORK(&evl->work_data[i].work, sonic_ev_work_handler);
		evl->work_data[i].evl = evl;
	}

	/* Create workqueue */
	//evl->wq = create_singlethread_workqueue("sonic_async");
	evl->wq = create_workqueue("sonic_async");
	if (!evl->wq) {
		OSAL_LOG_ERROR("Failed to create_singlethread_workqueue\n");
		rc = -ENOMEM;
		goto err_evl;
	}

	OSAL_LOG_NOTICE("Successfully created event list\n");
	evl->enable = true;
	return 0;

err_evl:
	OSAL_LOG_ERROR("Failed to create event list\n");
	sonic_destroy_ev_list(pc_res);
	return rc;
}

