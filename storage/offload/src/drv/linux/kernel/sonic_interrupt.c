/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include <linux/interrupt.h>
#include <linux/hardirq.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/printk.h>

#include "osal_logger.h"
#include "osal_time.h"
#include "osal_sys.h"
#include "sonic.h"
#include "sonic_dev.h"
#include "sonic_lif.h"
#include "sonic_interrupt.h"
#include "sonic_api_int.h"

#define evid_to_db_pa(evl, id) (evl->db_base_pa + (sizeof(struct sonic_db_data) * (id)))
#define db_pa_to_evid(evl, addr) (((dma_addr_t)(addr) - evl->db_base_pa) / sizeof(struct sonic_db_data))
#define evid_to_db_va(evl, id) (struct sonic_db_data *)(evl->db_base + (sizeof(struct sonic_db_data) * (id)))
#define db_va_to_evid(evl, addr) (((void *)(addr) - evl->db_base) / sizeof(struct sonic_db_data))

static int sonic_get_evid(struct sonic_event_list *evl, u32 *evid)
{
	unsigned long irqflags;
	u32 id;

	if (!evl)
		return -EINVAL;

	spin_lock_irqsave(&evl->inuse_lock, irqflags);

	id = find_next_zero_bit(evl->inuse_evid_bmp, evl->size_ev_bmp,
			evl->next_evid);
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

static inline struct sonic_db_data *
sonic_intr_db_primed_usr_data_get(struct sonic_event_list *evl,
				  uint32_t id,
				  uint64_t *usr_data)
{
	struct sonic_db_data *db_data = evid_to_db_va(evl, id);
	*usr_data = db_data->primed == sonic_intr_get_fire_data32() ?
		    db_data->usr_data : 0;
	return db_data;
}

static inline void
sonic_intr_db_primed_usr_data_put(struct sonic_db_data *db_data)
{
	db_data->usr_data = 0;
}

static inline bool
sonic_intr_db_fired_chk(struct sonic_event_list *evl,
			uint32_t id,
			uint32_t *fired_val)
{
	struct sonic_db_data *db_data = evid_to_db_va(evl, id);
	*fired_val = db_data->fired;
	return db_data->fired == sonic_intr_get_fire_data32();
}

static inline void
sonic_intr_db_fired_clr(struct sonic_event_list *evl,
			uint32_t id)
{
	struct sonic_db_data *db_data = evid_to_db_va(evl, id);

	db_data->fired = 0;
	db_data->primed = 0;
}

static inline void
sonic_intr_ev_clr(struct sonic_event_list *evl,
		  uint32_t id)
{
	sonic_intr_db_fired_clr(evl, id);
	sonic_put_evid(evl, id);
}

#define SONIC_ISR_MAX_IDLE_COUNT 10000
#define SONIC_EV_WORK_POLLER_TIMEOUT (100 * OSAL_NSEC_PER_MSEC)
#define SONIC_EV_IDLE_WORK_JIFFIES 1

static int
sonic_poll_ev_list(struct sonic_event_list *evl, int budget,
		   struct sonic_work_data *work, int *used_count)
{
	struct sonic_db_data *db_data;
	uint32_t id, first_id, next_id, wrap_size;
	uint32_t loop_count = 0;
	uint32_t fired;
	uint64_t usr_data;
	int found = 0;
	int found_zero_data = 0;
	unsigned long irqflags;
	bool b_wrapped = false;

	spin_lock_irqsave(&evl->inuse_lock, irqflags);
	first_id = evl->next_used_evid;
	next_id = evl->next_used_evid;
	wrap_size = evl->size_ev_bmp;
	while (!found || loop_count < budget) {
		loop_count++;

		id = find_next_bit(evl->inuse_evid_bmp, wrap_size,
				   next_id);
		if (id >= wrap_size) {
			if (b_wrapped)
				break; /* deja vu */
			id = find_first_bit(evl->inuse_evid_bmp, first_id);
			if (id >= first_id)
				break;
			wrap_size = first_id;
			b_wrapped = true;
		}
		next_id = id + 1;
		db_data = sonic_intr_db_primed_usr_data_get(evl, id, &usr_data);
		if (usr_data && sonic_intr_db_fired_chk(evl, id, &fired)) {

			//OSAL_LOG_DEBUG("found ev id %d with data 0x%llx",
			//		id, (unsigned long long) *data);
			work->ev_data[found].evid = id;
			work->ev_data[found].data = usr_data;
			sonic_intr_db_primed_usr_data_put(db_data);
			if (!found_zero_data) {
				/* Start at next_id next time */
				evl->next_used_evid = next_id;
			}
			found++;
		} else {
			if (!found_zero_data) {
				/* Start at this id next time */
				evl->next_used_evid = id;
			}
			found_zero_data++;
		}
	}
	spin_unlock_irqrestore(&evl->inuse_lock, irqflags);
	*used_count = found + found_zero_data;

	work->ev_count = found;
	if (found) {
		work->found_work = true;
		evl->idle_count = 0;
	} else {
		if (evl->idle_count++ == SONIC_ISR_MAX_IDLE_COUNT)
			OSAL_LOG_CRITICAL("sonic_async_ev_isr stuck in idle loop!");
	}

	//if (!found || found > 2 || (found + found_zero_data) >= budget || loop_count > budget) {
	//	OSAL_LOG_WARN("TODO interesting: found %d, found_zero_data %d, loop_count %u",
	//		      found, found_zero_data, loop_count);
	//}

	return found;
}

static void sonic_ev_idle_handler(struct work_struct *work)
{
	struct sonic_event_list *evl = container_of(work, struct sonic_event_list, idle_work.work);
	struct sonic_work_data *real_swd = &evl->work_data;
	int npolled = 0;
	int used_count = 0;
	bool was_armed;

	/* Mask! */
	sonic_intr_mask(&evl->pc_res->intr, true);
	was_armed = xchg(&evl->armed, false);
	if (!was_armed) {
		/* ISR or work item are running */
		return;
	}

	if (work_pending(&real_swd->work)) {
		/* Let the real work item take care of it */
		return;
	}

	real_swd->found_work = false;
	npolled = sonic_poll_ev_list(evl, SONIC_ASYNC_BUDGET, real_swd,
				     &used_count);
	if (used_count) {
		if (npolled)
			evl->work_data.timestamp = 0;
		queue_work(evl->wq, &real_swd->work);
	} else {
		/* Unmask */
		xchg(&evl->armed, true);
		sonic_intr_mask(&evl->pc_res->intr, false);
	}
}

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
	int used_count = 0;
	int npolled = 0;

	OSAL_LOG_DEBUG("sonic_ev_work_handler enter (workid %u)...", work_id);

	cancel_delayed_work(&evl->idle_work);

	for (i = 0; i < swd->ev_count; i++) {
		evd = &swd->ev_data[i];
		if (!evd->data)
			continue;

		/* poll status */
		if (pnso_request_poller((void *) evd->data) != EBUSY) {
			evd->data = 0;
			sonic_intr_ev_clr(evl, evd->evid);
			complete_count++;
		} else {
			incomplete_count++;
		}
	}

	//if (complete_count == 0 || incomplete_count > 0 ||
	//    (complete_count + incomplete_count) == 0) {
	//	OSAL_LOG_WARN("TODO: interesting: sonic_ev_work_handler workid %u, %u complete, %u incomplete",
	//		      work_id, complete_count, incomplete_count);
	//}

	if (complete_count || !swd->timestamp)
		swd->timestamp = cur_ts;

	if (incomplete_count) {
		if ((cur_ts - swd->timestamp) > SONIC_EV_WORK_POLLER_TIMEOUT) {
			OSAL_LOG_WARN("timed out work item %u with %u events",
				      work_id, incomplete_count);
			for (i = 0; i < swd->ev_count; i++) {
				evd = &swd->ev_data[i];
				if (evd->data) {
					pnso_request_poller((void *)evd->data);
					evd->data = 0;
					sonic_intr_ev_clr(evl, evd->evid);
				}
			}
			complete_count += incomplete_count;
			incomplete_count = 0;
		} else {
			/* reenqueue */
			OSAL_LOG_DEBUG("work item %u reenqueued with %u events",
				       work_id, incomplete_count);
			if (!complete_count)
				osal_sched_yield();
			queue_work(evl->wq, &swd->work);
			goto done;
		}
	}

	/* Try to get more work */
	if (!swd->found_work) {
		/* No credits to return */
		npolled = sonic_poll_ev_list(evl, SONIC_ASYNC_BUDGET,
				&evl->work_data, &used_count);
		if (npolled) {
			swd->timestamp = 0; /* start fresh */
			queue_work(evl->wq, &swd->work);
		} else {
			OSAL_LOG_DEBUG("no work available, used_count %d",
				       used_count);

			/* Just unmask in hopes our isr kicks in */
			xchg(&evl->armed, true);
			sonic_intr_mask(&evl->pc_res->intr, false);
			if (used_count) {
				/* plan B */
				queue_delayed_work(evl->wq, &evl->idle_work,
						   SONIC_EV_IDLE_WORK_JIFFIES);
			}
		}
	} else {
		prev_ev_count = swd->ev_count;
		npolled = sonic_poll_ev_list(evl, SONIC_ASYNC_BUDGET,
				&evl->work_data, &used_count);
		if (npolled) {
			/* Return credits, but don't unmask */
			sonic_intr_return_credits(&evl->pc_res->intr,
					prev_ev_count, false, false);
			swd->timestamp = 0; /* start fresh */
			queue_work(evl->wq, &swd->work);
		} else {
			/* Return credits and unmask */
			xchg(&evl->armed, true);
			sonic_intr_return_credits(&evl->pc_res->intr,
					prev_ev_count, true, false);
			if (used_count) {
				/* plan B */
				queue_delayed_work(evl->wq, &evl->idle_work,
						   SONIC_EV_IDLE_WORK_JIFFIES);
			}
		}
	}

done:
	OSAL_LOG_DEBUG("... exit sonic_ev_work_handler workid %u, %u complete, %u incomplete, %u npolled",
		       work_id, complete_count, incomplete_count, npolled);
}

irqreturn_t sonic_async_ev_isr(int irq, void *evlptr)
{
	struct sonic_event_list *evl = (struct sonic_event_list *) evlptr;
	int npolled = 0;
	int used_count = 0;
	bool was_armed;

	was_armed = xchg(&evl->armed, false);

	//OSAL_LOG_DEBUG("sonic_async_ev_isr enter ...");

	if (unlikely(!evl->enable) || !was_armed) {
		//OSAL_LOG_DEBUG("... exit sonic_async_ev_isr, not armed");
		//sonic_intr_mask(&evl->pc_res->intr, false);
		return IRQ_HANDLED;
	}

	evl->work_data.found_work = false;
	npolled = sonic_poll_ev_list(evl, SONIC_ASYNC_BUDGET, &evl->work_data,
			&used_count);
	if (used_count) {
		evl->work_data.timestamp = 0;
		queue_work(evl->wq, &evl->work_data.work);
	} else {
		//OSAL_LOG_DEBUG("... no work available in sonic_async_ev_isr");

		/* spurious interrupt case */
		xchg(&evl->armed, true);
		sonic_intr_mask(&evl->pc_res->intr, false);
	}

	//OSAL_LOG_DEBUG("... exit sonic_async_ev_isr, enqueued %d work items", npolled);

	return IRQ_HANDLED;
}

uint64_t sonic_intr_get_db_addr(struct per_core_resource *pc_res,
		uint64_t usr_data)
{
	struct sonic_event_list *evl = pc_res->evl;
	struct sonic_db_data *db_data;
	uint32_t evid;

	if (!evl || !evl->db_base) {
		OSAL_LOG_WARN("Invalid evl");
		return 0;
	}

	if (sonic_get_evid(evl, &evid) != 0) {
		OSAL_LOG_WARN("Cannot find free evid");
		return 0;
	}

	OSAL_LOG_DEBUG("Successfully allocated evid %u", evid);

	db_data = evid_to_db_va(evl, evid);
	db_data->usr_data = usr_data;
	db_data->primed = sonic_intr_get_fire_data32();
	return sonic_hostpa_to_devpa((uint64_t) evid_to_db_pa(evl, evid) +
				     offsetof(struct sonic_db_data, fired));
}

void sonic_intr_put_db_addr(struct per_core_resource *pc_res, uint64_t addr)
{
	struct sonic_event_list *evl = pc_res->evl;
	uint32_t evid;

	if (!evl || !evl->db_base)
		return;

	addr -= offsetof(struct sonic_db_data, fired);
	evid = db_pa_to_evid(evl, sonic_devpa_to_hostpa(addr));
	if (evid < evl->size_ev_bmp) {
		sonic_intr_ev_clr(evl, evid);
	}
}

void sonic_destroy_ev_list(struct per_core_resource *pc_res)
{
	struct sonic_event_list *evl = pc_res->evl;

	if (!evl)
		return;

	evl->enable = false;

	if (evl->wq) {
		cancel_delayed_work(&evl->idle_work);
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
		OSAL_LOG_INFO("Truncating event count from %u to %u",
			      ev_count, (uint32_t) MAX_PER_CORE_EVENTS);
		ev_count = MAX_PER_CORE_EVENTS;
	}

	evl = devm_kzalloc(dev, sizeof(*evl), GFP_KERNEL);
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

	evl->db_total_size = sizeof(struct sonic_db_data) * ev_count;
	evl->db_base = dma_zalloc_coherent(dev,
					  evl->db_total_size,
					  &evl->db_base_pa, GFP_KERNEL);
	if (!evl->db_base) {
		OSAL_LOG_ERROR("Failed to dma_alloc %u bytes for db",
			       evl->db_total_size);
		rc = -ENOMEM;
		goto err_evl;
	}
	memset(evl->db_base, 0, evl->db_total_size);

	/* Setup workqueue entry */
	INIT_DELAYED_WORK(&evl->idle_work, sonic_ev_idle_handler);
	INIT_WORK(&evl->work_data.work, sonic_ev_work_handler);
	evl->work_data.evl = evl;

	/* Create workqueue */
	sprintf(evl->name, "sonic_async%u", pc_res->idx);
	//evl->wq = create_workqueue(evl->name);
	//evl->wq = create_singlethread_workqueue(evl->name);
	evl->wq = osal_create_workqueue_fast(evl->name, SONIC_ASYNC_BUDGET);
	if (!evl->wq) {
		OSAL_LOG_ERROR("Failed to create_workqueue");
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

void sonic_pprint_ev_list(struct sonic_event_list *evl)
{
	if (!evl)
		return;
	OSAL_LOG_NOTICE("EVL %s: irq %d, enabled %u, armed %u",
		      evl->name, evl->irq, evl->enable, evl->armed);
	OSAL_LOG_NOTICE("  idle_count %d, next_evid %d, next_used_evid %d",
		      evl->idle_count, evl->next_evid, evl->next_used_evid);
	OSAL_LOG_NOTICE("  size_bmp %d, bmp = %llx %llx %llx %llx %llx %llx %llx %llx",
		      evl->size_ev_bmp,
		      (unsigned long long) evl->inuse_evid_bmp[0],
		      (unsigned long long) evl->inuse_evid_bmp[1],
		      (unsigned long long) evl->inuse_evid_bmp[2],
		      (unsigned long long) evl->inuse_evid_bmp[3],
		      (unsigned long long) evl->inuse_evid_bmp[4],
		      (unsigned long long) evl->inuse_evid_bmp[5],
		      (unsigned long long) evl->inuse_evid_bmp[6],
		      (unsigned long long) evl->inuse_evid_bmp[7]);
	OSAL_LOG_NOTICE("  db_total_size %u, db_base 0x%llx, db_base_pa 0x%llx",
		      evl->db_total_size,
		      (unsigned long long) evl->db_base,
		      (unsigned long long) evl->db_base_pa);
	OSAL_LOG_NOTICE("  idle work: pending %u",
		      delayed_work_pending(&evl->idle_work));
	OSAL_LOG_NOTICE("  work: pending %u, found %u, ev_count %u, timestamp %llu",
		      work_pending(&evl->work_data.work),
		      evl->work_data.found_work, evl->work_data.ev_count,
		      (unsigned long long) evl->work_data.timestamp);
}

void sonic_pprint_pcr_ev_list(struct per_core_resource *pcr)
{
	if (pcr)
		sonic_pprint_ev_list(pcr->evl);
}
