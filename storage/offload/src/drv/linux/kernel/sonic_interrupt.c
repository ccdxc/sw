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
#include <linux/hardirq.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/printk.h>

#include "osal_logger.h"
#include "osal_time.h"
#include "sonic_dev.h"
#include "sonic_lif.h"
#include "sonic_interrupt.h"
#include "sonic_api_int.h"

#define evid_to_db_pa(evl, id) (evl->db_base_pa + (sizeof(uint64_t) * (id)))
#define db_pa_to_evid(evl, addr) (((dma_addr_t)(addr) - evl->db_base_pa) / sizeof(uint64_t))
#define evid_to_db_va(evl, id) (evl->db_base + (sizeof(uint64_t) * (id)))
#define db_va_to_evid(evl, addr) (((void*)(addr) - evl->db_base) / sizeof(uint64_t))

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

static int sonic_poll_ev_list(struct sonic_event_list *evl, int budget, struct sonic_work_data *work)
{
	uint32_t id, first_id, next_id;
	uint32_t loop_count = 0;
	uint64_t *data;
	int found = 0;
	int found_zero_data = 0;

	first_id = evl->next_used_evid;
	next_id = first_id;
	while (loop_count < budget) {
		loop_count++;
		id = find_next_bit(evl->inuse_evid_bmp, evl->size_ev_bmp, next_id);
		if (id >= evl->size_ev_bmp) {
			id = find_first_bit(evl->inuse_evid_bmp, first_id);
			if (id >= first_id)
				break;
		}
		next_id = id + 1;
		data = (uint64_t *) evid_to_db_va(evl, id);
		if (*data) {
			//OSAL_LOG_DEBUG("found ev id %d with data 0x%llx\n",
			//		id, (unsigned long long) *data);
			if (!found_zero_data)
				evl->next_used_evid = next_id;
			work->ev_data[found].evid = id;
			work->ev_data[found].data = *data;
			*data = 0;
			found++;
		} else {
			/* Expect this event to trigger soon */
			if (!found_zero_data) {
				evl->next_used_evid = id;
			}

			found_zero_data++;
		}
	}

	if (found) {
		work->timestamp = 0;
		work->ev_count = found;
		queue_work(evl->wq, &work->work);
	}

	//if (!found || found > 2 || (found + found_zero_data) >= budget || loop_count > budget) {
	//	OSAL_LOG_WARN("TODO interesting: found %d, found_zero_data %d, loop_count %u\n",
	//		      found, found_zero_data, loop_count);
	//}

	return found;
}

#define SONIC_EV_WORK_TIMEOUT (10 * OSAL_NSEC_PER_MSEC)

static void sonic_ev_work_handler(struct work_struct *work)
{
	struct sonic_work_data *swd = container_of(work, struct sonic_work_data, work);
	struct sonic_event_list *evl = swd->evl;
	struct sonic_event_data *evd;
	int work_id = evl->pc_res->idx;
	uint64_t cur_ts = osal_get_clock_nsec();
	uint32_t complete_count = 0;
	uint32_t incomplete_count = 0;
	uint32_t prev_ev_count;
	uint32_t i;
	int npolled = 0;

	OSAL_LOG_DEBUG("sonic_ev_work_handler enter (workid %u)...\n", work_id);

	for (i = 0; i < swd->ev_count; i++) {
		evd = &swd->ev_data[i];
		if (!evd->data)
			continue;
		if (pnso_request_poller((void *) evd->data) == PNSO_OK) {
			/* Done, release evid */
			evd->data = 0;
			sonic_put_evid(evl, evd->evid);
			complete_count++;
		} else {
			incomplete_count++;
		}
	}

	//if (complete_count == 0 || incomplete_count > 0 ||
	//    (complete_count + incomplete_count) == 0) {
	//	OSAL_LOG_WARN("TODO: interesting: sonic_ev_work_handler workid %u, %u complete, %u incomplete\n",
	//		      work_id, complete_count, incomplete_count);
	//}

	if (complete_count || !swd->timestamp)
		swd->timestamp = cur_ts;

	if (incomplete_count) {
		if ((swd->timestamp - cur_ts) > SONIC_EV_WORK_TIMEOUT) {
			OSAL_LOG_WARN("timed out work item %u with %u events\n",
				      work_id, incomplete_count);
			for (i = 0; i < swd->ev_count; i++) {
				evd = &swd->ev_data[i];
				if (evd->data)
					clear_bit(evd->evid, evl->inuse_evid_bmp);
			}
			complete_count += incomplete_count;
			incomplete_count = 0;
		} else {
			/* reenqueue */
			OSAL_LOG_DEBUG("work item %u reenqueued with %u events\n",
				       work_id, incomplete_count);
			queue_work(evl->wq, &swd->work);
			goto done;
		}
	}

	/* Try to get more work */
	prev_ev_count = swd->ev_count;
	npolled = sonic_poll_ev_list(evl, SONIC_ASYNC_BUDGET, &evl->work_data);
	if (npolled) {
		/* Return credits, but don't unmask */
		sonic_intr_return_credits(&evl->pc_res->intr, prev_ev_count,
					  false, false);
	} else {
		/* Return credits and unmask */
		xchg(&evl->armed, true);
		sonic_intr_return_credits(&evl->pc_res->intr, prev_ev_count,
					  true, false);
	}

done:
	OSAL_LOG_DEBUG("... exit sonic_ev_work_handler workid %u, %u complete, %u incomplete, %u npolled\n",
		       work_id, complete_count, incomplete_count, npolled);
}

#define SONIC_ISR_MAX_IDLE_COUNT 1000

irqreturn_t sonic_async_ev_isr(int irq, void *evlptr)
{
	struct sonic_event_list *evl = (struct sonic_event_list *) evlptr;
	int npolled = 0;
	bool was_armed;

	was_armed = xchg(&evl->armed, false);

	//OSAL_LOG_DEBUG("sonic_async_ev_isr enter ...\n");

	if (unlikely(!evl->enable) || !was_armed) {
		//OSAL_LOG_DEBUG("... exit sonic_async_ev_isr, not armed\n");
		//sonic_intr_mask(&evl->pc_res->intr, false);
		return IRQ_HANDLED;
	}

	npolled = sonic_poll_ev_list(evl, SONIC_ASYNC_BUDGET, &evl->work_data);

	if (!npolled) {
		if (evl->idle_count++ == SONIC_ISR_MAX_IDLE_COUNT)
			OSAL_LOG_CRITICAL("sonic_async_ev_isr stuck in idle loop!\n");
		xchg(&evl->armed, true);
		sonic_intr_mask(&evl->pc_res->intr, false);
	} else {
		//if (evl->idle_count)
		//	OSAL_LOG_WARN("isr idle count was %d\n", evl->idle_count);
		evl->idle_count = 0;
	}

	//OSAL_LOG_DEBUG("... exit sonic_async_ev_isr, enqueued %d work items\n", npolled);

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

	OSAL_LOG_DEBUG("Successfully allocated evid %u\n", evid);

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

	if (!evl)
		return;

	evl->enable = false;

	if (evl->wq) {
		flush_workqueue(evl->wq);
		destroy_workqueue(evl->wq);
		evl->wq = NULL;
	}

	if (evl->db_base) {
		dma_free_coherent(pc_res->lif->sonic->dev,
				  evl->db_total_size,
				  evl->db_base, evl->db_base_pa);
		evl->db_base = 0;
	}

	devm_kfree(pc_res->lif->sonic->dev, evl);
	pc_res->evl = NULL;
}

int sonic_create_ev_list(struct per_core_resource *pc_res, uint32_t ev_count)
{
	struct sonic_event_list *evl = NULL;
	struct device *dev = pc_res->lif->sonic->dev;
	int rc = 0;

	if (ev_count > MAX_PER_CORE_EVENTS) {
		OSAL_LOG_INFO("Truncating event count from %u to %u\n",
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
	evl->db_base = dma_zalloc_coherent(dev,
					  evl->db_total_size,
					  &evl->db_base_pa, GFP_KERNEL);
	if (!evl->db_base) {
		OSAL_LOG_ERROR("Failed to dma_alloc %u bytes for db\n",
			       evl->db_total_size);
		rc = -ENOMEM;
		goto err_evl;
	}
	memset(evl->db_base, 0, evl->db_total_size);

	/* Setup workqueue entry */
	INIT_WORK(&evl->work_data.work, sonic_ev_work_handler);
	evl->work_data.evl = evl;

	/* Create workqueue */
	sprintf(evl->name, "sonic_async%u", pc_res->idx);
	//evl->wq = create_workqueue(evl->name);
	//evl->wq = create_singlethread_workqueue(evl->name);
	evl->wq = osal_create_workqueue_fast(evl->name, SONIC_ASYNC_BUDGET);
	if (!evl->wq) {
		OSAL_LOG_ERROR("Failed to create_workqueue\n");
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

