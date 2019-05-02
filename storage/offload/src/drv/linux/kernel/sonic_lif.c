/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include <linux/netdevice.h>
#include <linux/interrupt.h>
#include <linux/hardirq.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>

#ifdef __FreeBSD__
#include <sys/sysctl.h>
#define PNSO_INIT_STAT(s) SYSCTL_ADD_PROC(ctx, child, OID_AUTO, #s, \
		CTLTYPE_S64 | CTLFLAG_RD, lif, PNSO_STAT_ID(s), \
		sonic_sysctl_stat_handler, "Q", NULL);
#define PNSO_INIT_LIF_RESET(lif) SYSCTL_ADD_PROC(ctx, child, OID_AUTO, "reset", \
		CTLTYPE_U8 | CTLFLAG_RW, lif, lif->index, \
		sonic_sysctl_lif_reset_handler, "CU", "LIF reset control");
#include <linux/printk.h>
#endif
#include "pnso_stats.h"

#include "sonic.h"
#include "sonic_bus.h"
#include "sonic_dev.h"
#include "sonic_api_int.h"
#include "sonic_lif.h"
#include "sonic_interrupt.h"
#include "sonic_debugfs.h"
#include "osal_logger.h"
#include "osal_sys.h"
#include "osal_mem.h"
#include "osal_assert.h"

#include "pnso_init.h"

#define LIF_FOR_EACH_PC_RES(lif, idx, res)					\
	for (idx = 0; idx < lif->sonic->num_per_core_resources; idx++)		\
		if ((res = lif->res.pc_res[i]) != NULL)				\


#define LIF_FOR_EACH_SEQ_Q_BATCH_LIST(lif, list)				\
	for (list = &lif->seq_q_batch_ht[0];					\
	     list < &lif->seq_q_batch_ht[LIF_SEQ_Q_BATCH_HT_SZ];		\
	     list++)								\

#define LIF_RESET_CTL_CB(lif, state, err)					\
	if (lif->reset_ctl.cb)							\
	     (*lif->reset_ctl.cb)(lif->reset_ctl.cb_arg, state, err)		\

static int
sonic_lif_seq_q_batch_init(struct seq_queue_batch *batch,
			   void *cb_arg);
static int
sonic_lif_seq_q_init_complete(struct lif *lif);

static int
sonic_lif_seq_q_batch_control(struct seq_queue_batch *batch,
			      void *cb_arg);
static int
sonic_lif_reset(struct lif *lif);

static struct deferred_work *
sonic_lif_deferred_work_alloc_init(struct lif *lif, struct deferred_work *work)
{
	/* If work was given, assume it is static mark it for dont_free */
	if (work) {
		memset(work, 0, sizeof(*work));
		work->dont_free = true;
	} else {
		work = devm_kzalloc(lif->sonic->dev, sizeof(*work), GFP_ATOMIC);
		if (!work) {
			OSAL_LOG_ERROR("Failed deferred_work alloc");
			return NULL;
		}
	}
	INIT_LIST_HEAD(&work->list);
	return work;
}

static void
sonic_lif_deferred_work_free(struct lif *lif, struct deferred_work *work)
{
	if (work && !work->dont_free)
		devm_kfree(lif->sonic->dev, work);
}

static void sonic_lif_deferred_work(struct work_struct *work)
{
	struct lif *lif = container_of(work, struct lif, deferred.work);
	struct deferred *def = &lif->deferred;
	struct deferred_work *w = NULL;

	spin_lock_bh(&def->lock);
	if (!list_empty(&def->list)) {
		w = list_first_entry(&def->list, struct deferred_work, list);
		list_del_init(&w->list);
	}
	spin_unlock_bh(&def->lock);

	if (w) {
		switch (w->type) {
		case DW_TYPE_RESET:
			OSAL_LOG_DEBUG("Deferred RESET");
			break;
                default:
			break;
		};
		sonic_lif_deferred_work_free(lif, w);
		schedule_work(&def->work);
	}
}

static void
sonic_lif_deferred_enqueue(struct deferred *def,
			   struct deferred_work *work)
{
	/* Just to be safe, don't enqueue if already queued */
	spin_lock_bh(&def->lock);
	if (list_empty(&work->list))
		list_add_tail(&work->list, &def->list);
	spin_unlock_bh(&def->lock);
	schedule_work(&def->work);
}

static void __attribute__((unused))
sonic_lif_deferred_dequeue(struct deferred *def,
			   struct deferred_work *work)
{
	spin_lock_bh(&def->lock);
	if (!list_empty(&work->list))
		list_del_init(&work->list);
	spin_unlock_bh(&def->lock);
}

static void sonic_lif_deferred_cancel(struct lif *lif)
{
	struct deferred *def = &lif->deferred;
	struct deferred_work *w;
	struct deferred_work *w_next;

	cancel_work_sync(&def->work);
	spin_lock_bh(&def->lock);
	list_for_each_entry_safe(w, w_next, &def->list, list) {
		list_del_init(&w->list);
		sonic_lif_deferred_work_free(lif, w);
	}
	spin_unlock_bh(&def->lock);
}

static bool sonic_notifyq_service(struct cq *cq, struct cq_info *cq_info, void *cb_arg)
{
	union notifyq_cpl *cpl = cq_info->cq_desc;
	struct queue *q;
	struct lif *lif;

	q = cq->bound_q;
	lif = q->admin_info[0].cb_arg;

	/* Have we run out of new completions to process? */
	if (!(cpl->event.eid > lif->last_eid))
		return false;

	lif->last_eid = cpl->event.eid;
	switch (cpl->event.ecode) {
	case EVENT_OPCODE_RESET:
		OSAL_LOG_DEBUG("Notifyq EVENT_OPCODE_RESET eid=" PRIu64,
			    cpl->event.eid);
		OSAL_LOG_DEBUG("reset_code=%d state=%d",
			    cpl->reset.reset_code,
			    cpl->reset.state);
		lif->reset_ctl.work.type = DW_TYPE_RESET;
		sonic_lif_deferred_enqueue(&lif->deferred,
					   &lif->reset_ctl.work);
		break;
	case EVENT_OPCODE_HEARTBEAT:
		OSAL_LOG_DEBUG("Notifyq EVENT_OPCODE_HEARTBEAT eid=" PRIu64,
			    cpl->event.eid);
		break;
	case EVENT_OPCODE_LOG:
		OSAL_LOG_DEBUG("Notifyq EVENT_OPCODE_LOG eid=" PRIu64,
			    cpl->event.eid);
		print_hex_dump_debug("notifyq ", DUMP_PREFIX_OFFSET, 16, 1,
			       cpl->log.data, sizeof(cpl->log.data), true);
		break;
	default:
		OSAL_LOG_DEBUG("Notifyq bad event ecode=%d eid=" PRIu64,
			    cpl->event.ecode, cpl->event.eid);
		break;
	}

	return true;
}

static int sonic_notifyq_clean(struct lif *lif, int budget)
{
	struct cq *cq = &lif->notifyqcq->cq;
	int work_done;

	work_done = sonic_cq_service(cq, budget, sonic_notifyq_service, NULL);
	if (work_done)
		sonic_intr_return_credits(cq->bound_intr, work_done,
					  false, true);
	return work_done;
}

static bool sonic_adminq_service(struct cq *cq, struct cq_info *cq_info,
				 void *cb_arg)
{
	struct admin_cpl *comp = cq_info->cq_desc;

	if (comp->color != cq->done_color)
		return false;

	sonic_q_service(cq->bound_q, cq_info, comp->cpl_index);

	return true;
}

#ifndef __FreeBSD__
static int sonic_adminq_napi(struct napi_struct *napi, int budget)
{
	struct lif *lif = napi_to_cq(napi)->lif;
	int n_work = 0;
	int a_work = 0;

	if (likely(lif->notifyqcq && (lif->notifyqcq->flags & QCQ_F_INITED)))
		n_work = sonic_notifyq_clean(lif, budget);
	a_work = sonic_napi(napi, budget, sonic_adminq_service, NULL);

	return max(n_work, a_work);
}
#else
static void sonic_adminq_napi(struct napi_struct *napi)
{
	struct lif *lif = napi_to_cq(napi)->lif;
	int budget = NAPI_POLL_WEIGHT;
	int n_work = 0;
	int a_work = 0;

	if (likely(lif->notifyqcq && (lif->notifyqcq->flags & QCQ_F_INITED)))
		n_work = sonic_notifyq_clean(lif, budget);
	a_work = sonic_napi(napi, budget - n_work, sonic_adminq_service, NULL);

	if ((n_work + a_work) >= budget)
		napi_schedule(napi);
}
#endif

static irqreturn_t sonic_isr(int irq, void *data)
{
	struct napi_struct *napi = data;

	napi_schedule_irqoff(napi);

	return IRQ_HANDLED;
}

int sonic_intr_alloc(struct lif *lif, struct intr *intr)
{
	struct sonic *sonic = lif->sonic;
	struct sonic_dev *idev = &sonic->idev;
	unsigned long index;

	index = find_first_zero_bit(sonic->intrs, sonic->nintrs);
	if (index >= sonic->nintrs)
		return -ENOSPC;
	set_bit(index, sonic->intrs);

	return sonic_intr_init(idev, intr, index);
}

void sonic_intr_free(struct lif *lif, struct intr *intr)
{
	if (intr->index != INTR_INDEX_NOT_ASSIGNED)
		clear_bit(intr->index, lif->sonic->intrs);
}

static void *
sonic_desc_mem_zalloc(struct device *dev,
		      uint32_t *total_size,
		      uint32_t min_size,
		      dma_addr_t *base_pa)
{
	void *base_va = NULL;
	uint32_t alloc_size = *total_size;

	OSAL_ASSERT(alloc_size && min_size);
	while (alloc_size >= min_size) {
		base_va = dma_zalloc_coherent(dev, alloc_size,
					      base_pa, GFP_KERNEL);
		if (base_va) {
			OSAL_LOG_DEBUG("requested %u bytes allocated %u bytes",
				       *total_size, alloc_size); 
			break;
		}
		alloc_size >>= 1;
	}

	*total_size = alloc_size;
	return base_va;
}

static void
sonic_desc_mem_free(struct device *dev,
		    size_t size,
		    void *base_va,
		    dma_addr_t base_pa)
{
	dma_free_coherent(dev, size, base_va, base_pa);
}

static inline uint32_t
sonic_seq_q_batch_hash(const struct queue *q)
{
	return jhash_3words(q->qgroup, q->num_descs, q->desc_size, 0);
}

static inline uint32_t
sonic_seq_q_batch_hash_idx(uint32_t hash)
{
	return hash & (LIF_SEQ_Q_BATCH_HT_SZ - 1);
}

static bool
sonic_seq_q_batch_key_eq(const struct seq_queue_batch *batch,
			 const struct queue *q)
{
	return (batch->qgroup == q->qgroup) &&
	       (batch->per_q_num_descs == q->num_descs) &&
	       (batch->per_q_desc_size == q->desc_size);
}

/*
 * Freebsd does not support linux/hashtable.h so we roll our own.
 */
static void
sonic_seq_q_batch_ht_init(struct lif *lif)
{
	struct hlist_head *list;

	LIF_FOR_EACH_SEQ_Q_BATCH_LIST(lif, list) {
		INIT_HLIST_HEAD(list);
        }
}

static void
sonic_seq_q_batch_ht_add_head(struct lif *lif,
			      struct seq_queue_batch *batch,
			      uint32_t hash)
{
	hlist_add_head(&batch->node,
		       &lif->seq_q_batch_ht[sonic_seq_q_batch_hash_idx(hash)]);
}

static void
sonic_seq_q_batch_ht_del( struct seq_queue_batch *batch)
{
	hlist_del_init(&batch->node);
}

static int
sonic_seq_q_batch_ht_for_each(struct lif *lif,
			      uint32_t hash,
			      seq_q_batch_ht_cb cb,
			      void *cb_arg)
{
	struct seq_queue_batch *batch;
	int err = 0;

	hlist_for_each_entry(batch, 
		&lif->seq_q_batch_ht[sonic_seq_q_batch_hash_idx(hash)], node) {

		err = cb(batch, cb_arg);
		if (err)
			break;
	}

	return err;
}

static int
sonic_seq_q_batch_ht_for_each_safe(struct lif *lif,
				   uint32_t hash,
				   seq_q_batch_ht_cb cb,
				   void *cb_arg)
{
	struct seq_queue_batch *batch;
	struct hlist_node *next;
	int err = 0;

	hlist_for_each_entry_safe(batch, next,
		&lif->seq_q_batch_ht[sonic_seq_q_batch_hash_idx(hash)], node) {

		err = cb(batch, cb_arg);
		if (err)
			break;
	}

	return err;
}

static int
sonic_seq_q_batch_ht_for_all(struct lif *lif,
			     seq_q_batch_ht_cb cb,
			     void *cb_arg)
{
	struct hlist_head *list;
	uint32_t hash = 0;
	int err = 0;

	LIF_FOR_EACH_SEQ_Q_BATCH_LIST(lif, list) {
		err = sonic_seq_q_batch_ht_for_each(lif, hash, cb, cb_arg);
		if (err)
			break;
		hash++;
	}

	return err;
}

static int
sonic_seq_q_batch_ht_for_all_safe(struct lif *lif,
				  seq_q_batch_ht_cb cb,
				  void *cb_arg)
{
	struct hlist_head *list;
	uint32_t hash = 0;
	int err = 0;

	LIF_FOR_EACH_SEQ_Q_BATCH_LIST(lif, list) {
		err = sonic_seq_q_batch_ht_for_each_safe(lif, hash, cb, cb_arg);
		if (err)
			break;
		hash++;
	}

	return err;
}

static uint32_t
sonic_seq_q_batch_multiple(const struct queue *q)
{
        const struct per_core_resource *res = q->pc_res;
	uint32_t multiple = q->lif->sonic->num_per_core_resources;

	switch (q->qgroup) {

        case STORAGE_SEQ_QGROUP_CPDC:
		multiple *= MAX_PER_CORE_CPDC_QUEUES;
		break;

	case STORAGE_SEQ_QGROUP_CRYPTO:
		multiple *= MAX_PER_CORE_CRYPTO_QUEUES;
		break;

	case STORAGE_SEQ_QGROUP_CPDC_STATUS:
		multiple *= res->num_cpdc_status_qs;
		break;

	case STORAGE_SEQ_QGROUP_CRYPTO_STATUS:
		multiple *= res->num_crypto_status_qs;
		break;

	default:
		multiple = 0;
		break;
	}

	OSAL_ASSERT(multiple);
	return multiple;
}

static struct seq_queue_batch *
sonic_seq_q_batch_alloc(struct queue *q)
{
	struct lif *lif = q->lif;
	struct device *dev = lif->sonic->dev;
	struct seq_queue_batch *batch;
	uint32_t hash;

	batch = devm_kzalloc(dev, sizeof(*batch), GFP_KERNEL);
	if (!batch)
		goto error;

	batch->lif = lif;
	batch->base_qid = q->qid;
	batch->curr_qid = q->qid;
	batch->qgroup = q->qgroup;
	batch->per_q_num_descs = q->num_descs;
	batch->per_q_desc_size = q->desc_size;
	batch->base_alloc_size =
		ALIGN(q->total_size * sonic_seq_q_batch_multiple(q), PAGE_SIZE);
	batch->base_va = sonic_desc_mem_zalloc(dev, &batch->base_alloc_size,
					       ALIGN(q->total_size, PAGE_SIZE),
					       &batch->base_pa);
	if (!batch->base_va)
		goto error;

	hash = sonic_seq_q_batch_hash(q);
	sonic_seq_q_batch_ht_add_head(lif, batch, hash);
	lif->curr_seq_q_batch = batch;
	lif->num_seq_q_batches++;
	return batch;

error:
	if (batch)
		devm_kfree(dev, batch);
	return NULL;
}

static int
sonic_seq_q_batch_free(struct seq_queue_batch *batch,
		       void *cb_arg)
{
	struct lif *lif = cb_arg;
	struct device *dev = lif->sonic->dev;

	if (batch) {
		sonic_seq_q_batch_ht_del(batch);
		sonic_desc_mem_free(dev, batch->base_alloc_size,
				    batch->base_va, batch->base_pa);
		devm_kfree(dev, batch);
	}

	return 0;
}

struct seq_q_key_find_arg
{
	const struct queue *q;
	struct seq_queue_batch *ret_batch;
};

static int
sonic_seq_q_batch_key_find(struct seq_queue_batch *batch,
			   void *cb_arg)
{
	struct seq_q_key_find_arg *arg = cb_arg;

        arg->ret_batch = NULL;
	if (sonic_seq_q_batch_key_eq(batch, arg->q)) {
		arg->ret_batch = batch;
		return SONIC_SEQ_Q_BATCH_KEY_EXIST;
	}

        return 0;
}

static struct seq_queue_batch *
sonic_seq_q_batch_find(struct queue *q)
{
	struct lif *lif = q->lif;
	struct seq_queue_batch *batch;
	struct seq_q_key_find_arg find_arg;
	uint32_t hash;
	int err;

	batch = lif->curr_seq_q_batch;
	if (!batch ||
	    !sonic_seq_q_batch_key_eq(batch, q)) {

		find_arg.q = q;
		find_arg.ret_batch = NULL;
		batch = NULL;
		hash = sonic_seq_q_batch_hash(q);
		err = sonic_seq_q_batch_ht_for_each(lif, hash,
				    sonic_seq_q_batch_key_find, &find_arg);
		if (err == SONIC_SEQ_Q_BATCH_KEY_EXIST) {
			batch = find_arg.ret_batch;
			lif->curr_seq_q_batch = batch;
		}
	}
	if (!batch)
		batch = sonic_seq_q_batch_alloc(q);

	return batch;
}

static int
sonic_seq_q_alloc_from_batch(struct queue *q)
{
	struct seq_queue_batch *batch;
	int err = -ENOMEM;

	batch = sonic_seq_q_batch_find(q);
	while (batch) {
		OSAL_ASSERT(q->total_size <= batch->base_alloc_size);
		OSAL_ASSERT(batch->curr_alloc_size <= batch->base_alloc_size);
		if (q->total_size <= (batch->base_alloc_size - batch->curr_alloc_size)) {

			/*
			 * qid is expected to be contiguous for batching to work
			 */
			OSAL_ASSERT((q->qid - batch->curr_qid) <= 1);
			OSAL_ASSERT((batch->pid == 0) || (batch->pid == q->pid));
			batch->pid = q->pid;
			batch->curr_qid = q->qid;
			batch->num_queues++;
			q->base = batch->base_va + batch->curr_alloc_size;
			q->base_pa = batch->base_pa + batch->curr_alloc_size;
			batch->curr_alloc_size += q->total_size;
			err = 0;
			break;
		}
		batch = sonic_seq_q_batch_alloc(q);
	}

	return err;
}

void sonic_q_free(struct lif *lif, struct queue *q)
{
	if (q->admin_info) {
		devm_kfree(lif->sonic->dev, q->admin_info);
		q->admin_info = NULL;
	}
	if (q->base) {
		dma_free_coherent(lif->sonic->dev, q->total_size, q->base,
				  q->base_pa);
		q->base = NULL;
	}
}

int sonic_q_alloc(struct lif *lif, struct queue *q,
	unsigned int num_descs, unsigned int desc_size, bool do_alloc_descs)
{
	q->admin_info = devm_kzalloc(lif->sonic->dev, sizeof(*q->admin_info) * num_descs,
			       GFP_KERNEL);
	if (!q->admin_info)
		return -ENOMEM;

	if (do_alloc_descs) {
		unsigned int total_size =
			ALIGN(num_descs * desc_size, PAGE_SIZE);

		q->base = dma_zalloc_coherent(lif->sonic->dev, total_size,
					     &q->base_pa, GFP_KERNEL);
		if (!q->base) {
			sonic_q_free(lif, q);
			return -ENOMEM;
		}
		q->total_size = total_size;
	}

	return 0;
}

static void sonic_link_qcq_interrupts(struct qcq *src_qcq, struct qcq *n_qcq)
{
	n_qcq->intr.vector = src_qcq->intr.vector;
	n_qcq->intr.ctrl = src_qcq->intr.ctrl;
}

static int sonic_qcq_alloc(struct lif *lif, unsigned int index,
	 const char *base, unsigned int flags,
	 unsigned int num_descs, unsigned int desc_size,
	 unsigned int cq_desc_size,
	 unsigned int pid, struct qcq **qcq)
{
	struct sonic_dev *idev = &lif->sonic->idev;
	struct device *dev = lif->sonic->dev;
	struct qcq *new;
	unsigned int q_size = num_descs * desc_size;
	unsigned int cq_size = num_descs * cq_desc_size;
	unsigned int total_size = ALIGN(q_size, PAGE_SIZE) +
		ALIGN(cq_size, PAGE_SIZE);
	void *q_base, *cq_base;
	dma_addr_t q_base_pa, cq_base_pa;
	int err;

	*qcq = NULL;

	new = devm_kzalloc(dev, sizeof(*new), GFP_KERNEL);
	if (!new)
		return -ENOMEM;

	new->flags = flags;

	err = sonic_q_alloc(lif, &new->q, num_descs, desc_size, false);
	if (err)
		return err;

	err = sonic_q_init(lif, idev, &new->q, index, base, num_descs,
			   desc_size, pid);
	if (err)
		return err;

	if (flags & QCQ_F_INTR) {
		err = sonic_intr_alloc(lif, &new->intr);
		if (err)
			return err;
		err = sonic_bus_get_irq(lif->sonic, new->intr.index);
		if (err < 0)
			goto err_out_free_intr;
		new->intr.vector = err;
		sonic_intr_mask_on_assertion(&new->intr);
	} else {
		new->intr.index = INTR_INDEX_NOT_ASSIGNED;
	}

	new->cq.info = devm_kzalloc(dev, sizeof(*new->cq.info) * num_descs,
			GFP_KERNEL);
	if (!new->cq.info)
		return -ENOMEM;

	err = sonic_cq_init(lif, &new->cq, &new->intr,
		num_descs, cq_desc_size);
	if (err)
		goto err_out_free_intr;

	new->q.base = dma_zalloc_coherent(dev, total_size, &new->q.base_pa,
					  GFP_KERNEL);
	if (!new->q.base) {
		err = -ENOMEM;
		goto err_out_free_intr;
	}

	new->q.total_size = total_size;

	q_base = new->q.base;
	q_base_pa = new->q.base_pa;

	cq_base = (void *)ALIGN((uintptr_t)q_base + q_size, PAGE_SIZE);
	cq_base_pa = ALIGN(q_base_pa + q_size, PAGE_SIZE);

	sonic_q_map(&new->q, q_base, q_base_pa);
	sonic_cq_map(&new->cq, cq_base, cq_base_pa);
	sonic_cq_bind(&new->cq, &new->q);

	*qcq = new;

	return 0;

err_out_free_intr:
	sonic_intr_free(lif, &new->intr);

	return err;
}

static int sonic_qcq_reinit(struct qcq *qcq)
{
	int err;

	err = sonic_q_reinit(&qcq->q);
	if (!err)
		err = sonic_cq_reinit(&qcq->cq);

	return err;
}

static void sonic_qcq_free(struct lif *lif, struct qcq *qcq)
{
	if (!qcq)
		return;

	sonic_q_free(lif, &qcq->q);
	sonic_intr_free(lif, &qcq->intr);
	devm_kfree(lif->sonic->dev, qcq->cq.info);
	devm_kfree(lif->sonic->dev, qcq);
}

static unsigned int sonic_pid_get(struct lif *lif, unsigned int page)
{
	unsigned int ndbpgs_per_lif = lif->sonic->ident->dev.db_pages_per_lif;

	BUG_ON(ndbpgs_per_lif < page + 1);

	return lif->index * ndbpgs_per_lif + page;
}

static int sonic_qcqs_alloc(struct lif *lif)
{
	unsigned int pid;
	int err = -ENOMEM;

	pid = sonic_pid_get(lif, 0);
	err = sonic_qcq_alloc(lif, 0, "admin", QCQ_F_INTR, ADMINQ_LENGTH,
			sizeof(struct admin_cmd),
			sizeof(struct admin_cpl),
			pid, &lif->adminqcq);
	if (err)
		goto err_out;

	err = sonic_qcq_alloc(lif, 0, "notifyq", QCQ_F_NOTIFYQ, NOTIFYQ_LENGTH,
			sizeof(struct notifyq_cmd),
			sizeof(union notifyq_cpl),
			pid, &lif->notifyqcq);
	if (err)
		goto err_out_free_adminqcq;

	/* Let the notifyq ride on the adminq interrupt */
	sonic_link_qcq_interrupts(lif->adminqcq, lif->notifyqcq);
	return 0;

err_out_free_adminqcq:
	sonic_qcq_free(lif, lif->adminqcq);
	lif->adminqcq = NULL;
err_out:
	return err;
}

static int sonic_qcqs_reinit(struct lif *lif)
{
	int err;

	err = sonic_qcq_reinit(lif->adminqcq);
	if (!err)
		err = sonic_qcq_reinit(lif->notifyqcq);
	return err;
}

static void sonic_qcqs_free(struct lif *lif)
{
	sonic_qcq_free(lif, lif->notifyqcq);
	lif->notifyqcq = NULL;
	sonic_qcq_free(lif, lif->adminqcq);
	lif->adminqcq = NULL;
}

static int
sonic_lif_per_core_seq_q_count(struct lif *lif)
{
	int q_count = lif->sonic->ident->dev.seq_queues_per_lif;
	return q_count / lif->sonic->num_per_core_resources;
}

static int sonic_lif_per_core_resources_alloc(struct lif *lif)
{
	int err = -ENOMEM;
	int i, j;
#ifndef __FreeBSD__
	struct device *dev = lif->sonic->dev;
#endif
        struct per_core_resource *res;

	for (i = 0; i < lif->sonic->num_per_core_resources; i++) {
		res = devm_kzalloc(dev,	sizeof(*res), GFP_KERNEL);
		if (!res)
			goto err_out_cleanup;
		lif->res.pc_res[i] = res;
		res->idx = i;
		res->core_id = -1;
                spin_lock_init(&res->seq_cpdc_statusq_lock);
                spin_lock_init(&res->seq_crypto_statusq_lock);
	}

	return 0;

err_out_cleanup:
	for (j = 0; j < i; j++)
		devm_kfree(dev, lif->res.pc_res[j]);

	return err;
}

static int sonic_lif_alloc(struct sonic *sonic, unsigned int index)
{
	struct lif *lif;
	int err;

	lif = devm_kzalloc(sonic->dev, sizeof(*lif), GFP_KERNEL);
	if (!lif)
		return -ENOMEM;
#ifndef __FreeBSD__
	init_dummy_netdev(&lif->dummy_netdev);
#else
	//INIT_LIST_HEAD(&lif->dummy_netdev.napi_list);
#endif
	lif->sonic = sonic;
	lif->index = index;
	lif->lif_id = sonic_get_lif_id(sonic, index);
	atomic_set(&lif->reset_ctl.state, RESET_CTL_ST_IDLE);
	sonic_lif_deferred_work_alloc_init(lif, &lif->reset_ctl.work);

	snprintf(lif->name, sizeof(lif->name), "lif%u", index);

	spin_lock_init(&lif->adminq_lock);
	spin_lock_init(&lif->deferred.lock);
	INIT_LIST_HEAD(&lif->deferred.list);
	INIT_WORK(&lif->deferred.work, sonic_lif_deferred_work);

	sonic_seq_q_batch_ht_init(lif);

	err = sonic_qcqs_alloc(lif);
	if (err)
		return err;

	err = sonic_lif_per_core_resources_alloc(lif);
	if (err)
		goto err_out_free_qcqs;

	list_add_tail(&lif->list, &sonic->lifs);
	return 0;

err_out_free_qcqs:
	sonic_qcqs_free(lif);
	return err;
}

int sonic_lifs_alloc(struct sonic *sonic)
{
	unsigned int i;
	int err;

	INIT_LIST_HEAD(&sonic->lifs);

	for (i = 0; i < sonic->ident->dev.num_lifs; i++) {
		err = sonic_lif_alloc(sonic, i);
		if (err)
			return err;
	}

	return 0;
}

static int sonic_lif_reset(struct lif *lif)
{
	struct sonic_dev *idev = &lif->sonic->idev;

	sonic_dev_cmd_lif_reset(idev, lif->index);
	lif->flags &= ~LIF_F_INITED;
	return sonic_dev_cmd_wait_check(idev, devcmd_timeout);
}

static int sonic_lif_hang_notify(struct lif *lif)
{
	struct sonic_dev *idev = &lif->sonic->idev;

	sonic_dev_cmd_hang_notify(idev, lif->lif_id);
	return sonic_dev_cmd_wait_check(idev, devcmd_timeout);
}

void sonic_lif_reset_ctl_register(struct lif *lif,
				 reset_ctl_cb cb,
				 void *cb_arg)
{
	lif->reset_ctl.cb = cb;
	lif->reset_ctl.cb_arg = cb_arg;
}

void sonic_lif_reset_ctl_start(struct lif *lif)
{
	int old_state;
	int err;

	old_state = atomic_cmpxchg(&lif->reset_ctl.state,
				   RESET_CTL_ST_IDLE, RESET_CTL_ST_PRE_RESET);
	if (old_state == RESET_CTL_ST_IDLE) {
		OSAL_LOG_DEBUG("%s: lif_id %u", __FUNCTION__, lif->lif_id);
		err = sonic_lif_hang_notify(lif);
		LIF_RESET_CTL_CB(lif, RESET_CTL_ST_PRE_RESET, err);
	}
}

void sonic_lif_reset_ctl_reset(struct lif *lif)
{
	int old_state;
	int err;

	old_state = atomic_cmpxchg(&lif->reset_ctl.state,
				   RESET_CTL_ST_PRE_RESET, RESET_CTL_ST_RESET);
	if (old_state == RESET_CTL_ST_PRE_RESET) {
		OSAL_LOG_DEBUG("%s: lif_id %u", __FUNCTION__, lif->lif_id);
		err = sonic_lif_reset(lif);
		LIF_RESET_CTL_CB(lif, RESET_CTL_ST_RESET, err);
	}
}

void sonic_lif_reset_ctl_reinit(struct lif *lif)
{
	int old_state;
	int err;

	old_state = atomic_cmpxchg(&lif->reset_ctl.state,
				   RESET_CTL_ST_RESET, RESET_CTL_ST_REINIT);
	if (old_state == RESET_CTL_ST_RESET) {
		OSAL_LOG_DEBUG("%s: lif_id %u", __FUNCTION__, lif->lif_id);
		err = sonic_lif_reinit(lif);
		LIF_RESET_CTL_CB(lif, RESET_CTL_ST_REINIT, err);
	}
}

void sonic_lif_reset_ctl_end(struct lif *lif)
{
	int old_state;

	old_state = atomic_cmpxchg(&lif->reset_ctl.state,
				   RESET_CTL_ST_REINIT, RESET_CTL_ST_IDLE);
	if (old_state == RESET_CTL_ST_REINIT) {
		OSAL_LOG_DEBUG("%s: lif_id %u", __FUNCTION__, lif->lif_id);
		LIF_RESET_CTL_CB(lif, RESET_CTL_ST_IDLE, 0);
	}
}

bool sonic_lif_reset_ctl_pending(struct lif *lif)
{
	return atomic_read(&lif->reset_ctl.state) != RESET_CTL_ST_IDLE;
}

static void sonic_per_core_resources_free(struct lif *lif)
{
#ifndef __FreeBSD__
	struct device *dev = lif->sonic->dev;
#endif
	int i;

	for (i = 0; i < SONIC_MAX_CORES; i++) {
		if (lif->res.pc_res[i]) {
			devm_kfree(dev, lif->res.pc_res[i]);
			lif->res.pc_res[i] = NULL;
		}
	}
}

#ifdef __FreeBSD__

static int
sonic_sysctl_stat_handler(SYSCTL_HANDLER_ARGS)
{
	struct lif* lif = oidp->oid_arg1;
	uint32_t stat_id = oidp->oid_arg2;
	int64_t stat_val = 0;
	uint32_t i;
	struct per_core_resource *res;

	LIF_FOR_EACH_PC_RES(lif, i, res) {
		stat_val += PNSO_STAT_READ_BY_ID(&res->api_stats, stat_id);
	}

	return sysctl_handle_64(oidp, &stat_val, 0, req);
}

static int
sonic_sysctl_lif_reset_handler(SYSCTL_HANDLER_ARGS)
{
	struct lif *lif = oidp->oid_arg1;
	struct device *dev = lif->sonic->dev;
	int err;
	int8_t reset_sense = lif->reset_ctl.sense;

	err = sysctl_handle_8(oidp, &reset_sense, 0, req);
	if (err) {
		dev_err(dev, "Failed to set sonic systcl reset\n");
		return err;	}

	OSAL_LOG_DEBUG("sonic systcl reset %d", reset_sense);
	if (reset_sense == lif->reset_ctl.sense)
		return 0;

	lif->reset_ctl.sense = reset_sense;
	if (reset_sense)
		sonic_lif_reset_ctl_start(lif);
	return 0;
}

static void sonic_sysctl_deinit(struct lif *lif)
{
	/* Nothing to do, for now */
}

/*
 * Simple reset_ctl test function, overridable by application such as pnso.
 */
static void sonic_sysctl_reset_test(void *cb_arg,
				    enum reset_ctl_state ctl_state,
				    int err)
{
	struct lif *lif = cb_arg;

	if (!err) {
		OSAL_LOG_DEBUG("reset ctl_state %d succeeded", ctl_state);
	}

	switch (ctl_state) {
	case RESET_CTL_ST_PRE_RESET:
		sonic_lif_reset_ctl_reset(lif);
		break;
	case RESET_CTL_ST_RESET:
		sonic_lif_reset_ctl_reinit(lif);
		break;
	case RESET_CTL_ST_REINIT:
		sonic_lif_reset_ctl_end(lif);
		break;
	default:
		break;
	}
}

static int sonic_sysctl_init(struct lif *lif)
{
	struct device *dev = lif->sonic->dev;
	struct sysctl_ctx_list *ctx;
	struct sysctl_oid *tree;
	struct sysctl_oid_list *child;

	ctx = device_get_sysctl_ctx(dev->bsddev);
	tree = device_get_sysctl_tree(dev->bsddev);
	if (!ctx || !tree) {
		dev_err(dev, "Cannot find sonic systcl root device node\n");
		return EPERM;
	}
	child = SYSCTL_CHILDREN(tree);

	PNSO_INIT_STATS;
	PNSO_INIT_LIF_RESET(lif);

	/* Register a transient reset_ctl test function */
	sonic_lif_reset_ctl_register(lif, sonic_sysctl_reset_test, lif);

	return 0;
}
#endif

static void sonic_lif_free(struct lif *lif)
{
	sonic_lif_reset(lif);

	flush_scheduled_work();
	sonic_lif_deferred_cancel(lif);
	sonic_qcqs_free(lif);
	sonic_per_core_resources_free(lif);
	list_del(&lif->list);
	devm_kfree(lif->sonic->dev, lif);
}

void sonic_lifs_free(struct sonic *sonic)
{
	struct list_head *cur, *tmp;
	struct lif *lif;

	list_for_each_safe(cur, tmp, &sonic->lifs) {
		lif = list_entry(cur, struct lif, list);
		sonic_lif_free(lif);
	}
}

static void sonic_lif_qcq_deinit(struct lif *lif, struct qcq *qcq)
{
#ifndef __FreeBSD__
	struct device *dev = lif->sonic->dev;
#endif
	if (!qcq)
		return;

	sonic_debugfs_del_qcq(qcq);

	if (!(qcq->flags & QCQ_F_INITED))
		return;

	if (qcq->intr.index != INTR_INDEX_NOT_ASSIGNED) {
		sonic_intr_mask(&qcq->intr, true);
		synchronize_irq(qcq->intr.vector);
		devm_free_irq(dev, qcq->intr.vector, &qcq->napi);
		netif_napi_del(&qcq->napi);
	}
	qcq->flags &= ~QCQ_F_INITED;
}

static void sonic_cpdc_qs_deinit(struct per_core_resource *res)
{

	/* CPDC queues are freed through their corresponding batches */
}

static void sonic_crypto_qs_deinit(struct per_core_resource *res)
{
	/* Crypto queues are freed through their corresponding batches */
}

static void sonic_ev_intr_deinit(struct per_core_resource *res)
{
	if (!res->evl)
		return;

	sonic_disable_ev_list(res);
	if (res->intr.index != INTR_INDEX_NOT_ASSIGNED) {
		sonic_intr_mask(&res->intr, true);
		synchronize_irq(res->intr.vector);
		devm_free_irq(res->lif->sonic->dev, res->intr.vector, res->evl);
		res->intr.index = INTR_INDEX_NOT_ASSIGNED;
	}
	sonic_destroy_ev_list(res);
}

static void sonic_lif_per_core_resource_deinit(struct per_core_resource *res)
{
	if (!res)
		return;

	sonic_ev_intr_deinit(res);
	sonic_cpdc_qs_deinit(res);
	sonic_crypto_qs_deinit(res);
}

static void sonic_lif_per_core_resources_deinit(struct lif *lif)
{
	struct per_core_resource *res;
	int i;

	pnso_deinit();
	LIF_FOR_EACH_PC_RES(lif, i, res) {
		sonic_lif_per_core_resource_deinit(res);
	}
}

static void sonic_lif_deinit(struct lif *lif)
{
	napi_disable(&lif->adminqcq->napi);
	sonic_lif_qcq_deinit(lif, lif->notifyqcq);
	sonic_lif_qcq_deinit(lif, lif->adminqcq);
#ifdef __FreeBSD__
	sonic_sysctl_deinit(lif);
#endif
	sonic_lif_per_core_resources_deinit(lif);
	sonic_seq_q_batch_ht_for_all_safe(lif, sonic_seq_q_batch_free, lif);
}

/*
 * Global variable to track the lif
 * Unlike network device driver, offload driver does not get lif handle
 * as part of api. Tracking lif via global handle is a bit ugly but will make
 * upper level interfaces less clumsy
 */
struct lif *sonic_glif;

void sonic_lifs_deinit(struct sonic *sonic)
{
	struct list_head *cur;
	struct lif *lif;

	list_for_each(cur, &sonic->lifs) {
		lif = list_entry(cur, struct lif, list);
		sonic_lif_deinit(lif);
	}
	sonic_glif = NULL;
}

static int sonic_request_irq(struct lif *lif, struct qcq *qcq)
{
#ifndef __FreeBSD__
	struct device *dev = lif->sonic->dev;
#endif
	struct intr *intr = &qcq->intr;
#ifndef __FreeBSD__
	struct queue *q = &qcq->q;
#endif
	struct napi_struct *napi = &qcq->napi;

#ifndef __FreeBSD__
	snprintf(intr->name, sizeof(intr->name),
		 "%s-%s-%s", DRV_NAME, lif->name, q->name);
#else
	/*
	 * BSD will prefix by device name, also limited by space.
	 */
	snprintf(intr->name, sizeof(intr->name),
		 "%s", "a");//q->name);
#endif
	return devm_request_irq(dev, intr->vector, sonic_isr,
				0, intr->name, napi);
}

static int sonic_lif_adminq_init(struct lif *lif)
{
	struct sonic_dev *idev = &lif->sonic->idev;
	struct qcq *qcq = lif->adminqcq;
	struct queue *q = &qcq->q;
	struct napi_struct *napi = &qcq->napi;
	struct adminq_init_cpl comp;
	int err;

	sonic_dev_cmd_adminq_init(idev, q, 0, lif->index, 0);
	err = sonic_dev_cmd_wait_check(idev, devcmd_timeout);
	if (err)
		return err;

	sonic_dev_cmd_comp(idev, &comp);
	q->pc_res = NULL;
	q->qid = comp.qid;
	q->qtype = comp.qtype;
	q->qgroup = (storage_seq_qgroup_t)STORAGE_SEQ_QTYPE_ADMIN;
	q->db = sonic_db_map(idev, q);

	netif_napi_add(&lif->dummy_netdev, napi, sonic_adminq_napi,
		       NAPI_POLL_WEIGHT);

	err = sonic_request_irq(lif, qcq);
	if (err) {
		netif_napi_del(napi);
		return err;
	}

	qcq->flags |= QCQ_F_INITED;

	/* Enabling interrupts on adminq from here on... */
	napi_enable(napi);
	sonic_intr_mask(&lif->adminqcq->intr, false);

	return sonic_debugfs_add_qcq(lif, qcq);
}

static int sonic_lif_adminq_reinit(struct lif *lif)
{
	struct sonic_dev *idev = &lif->sonic->idev;
	struct qcq *qcq = lif->adminqcq;
	struct queue *q = &qcq->q;
	int err;

	sonic_dev_cmd_adminq_init(idev, q, 0, lif->index, 0);
	err = sonic_dev_cmd_wait_check(idev, devcmd_timeout);
	if (err)
		return err;

	qcq->flags |= QCQ_F_INITED;

	sonic_intr_mask(&lif->adminqcq->intr, false);

	return 0;
}

static int sonic_lif_cmd_notifyq_init(struct lif *lif)
{
	struct qcq *qcq = lif->notifyqcq;
	struct queue *q = &qcq->q;
	int err;

	struct sonic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.notifyq_init.opcode = CMD_OPCODE_NOTIFYQ_INIT,
		.cmd.notifyq_init.index = q->qid,
		.cmd.notifyq_init.pid = q->pid,
		.cmd.notifyq_init.intr_index = lif->adminqcq->intr.index,
		.cmd.notifyq_init.lif_index = lif->index,
		.cmd.notifyq_init.ring_size = ilog2(q->num_descs),
		.cmd.notifyq_init.ring_base = q->base_pa,
		.cmd.notifyq_init.notify_size = 0,
		.cmd.notifyq_init.notify_base = 0,
	};

	err = sonic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	q->qid = ctx.comp.notifyq_init.qid;
	q->qtype = ctx.comp.notifyq_init.qtype;
        return 0;
}

static int sonic_lif_notifyq_init(struct lif *lif)
{
	struct qcq *qcq = lif->notifyqcq;
	struct queue *q = &qcq->q;
	int err;

	err = sonic_lif_cmd_notifyq_init(lif);
	if (err)
		return err;

	q->db = NULL;

	/* preset the callback info */
	q->admin_info[0].cb_arg = lif;

	qcq->flags |= QCQ_F_INITED;

	return sonic_debugfs_add_qcq(lif, qcq);
}

static int sonic_lif_notifyq_reinit(struct lif *lif)
{
	struct qcq *qcq = lif->notifyqcq;
	int err;

	err = sonic_lif_cmd_notifyq_init(lif);
	if (err)
		return err;

	qcq->flags |= QCQ_F_INITED;
	return 0;
}

static int
sonic_seq_q_init(struct per_core_resource *res,
		 struct queue *q,
		 const char *name,
		 unsigned int qpos,
		 storage_seq_qgroup_t qgroup,
		 uint16_t desc_size)
{
	unsigned int ring_size;
	int err = -EINVAL;

	if (desc_size == 0 || !is_power_of_2(q->num_descs))
		goto done;

	ring_size = ilog2(q->num_descs);
	if (ring_size < 1 || ring_size > 16)
		goto done;

	q->pc_res = res;
	q->lif = res->lif;
	q->idev = &res->lif->sonic->idev;
	q->pid = sonic_pid_get(res->lif, 0);
	q->qid = res->lif->seq_q_index++;
	q->qpos = qpos;
	q->qtype = STORAGE_SEQ_QTYPE_SQ;
	q->qgroup = qgroup;
	q->desc_size = desc_size;
	q->total_size = q->num_descs * desc_size;
	q->pindex = 0;
	q->db = sonic_db_map(q->idev, q);
	osal_atomic_init(&q->descs_inuse, 0);

	err = sonic_seq_q_alloc_from_batch(q);
	if (err)
		goto done;

	snprintf(q->name, sizeof(q->name), "%s%u", name, q->qid);
	OSAL_LOG_INFO("sonic_seq_q_init q: " PRIx64 " %s",
			(u64)q, q->name);
	OSAL_LOG_INFO("seq_q->qtype %d", q->qtype);
	OSAL_LOG_INFO("seq_q->db " PRIx64, (u64)q->db);
done:
	return err;
}

static int
sonic_seq_q_reinit(struct queue *q)
{
	q->pindex = 0;
	osal_atomic_init(&q->descs_inuse, 0);
	return 0;
}

static int
get_seq_q_desc_count(struct queue *q,
		     uint32_t ring_id)
{
	struct sonic_accel_ring *ring;
	int err = -EINVAL;

	ring = sonic_get_accel_ring(ring_id);
	if (!ring)
		goto done;

	OSAL_LOG_DEBUG("get_seq_q_desc_count: hw ring %s: size=%u, desc_size=%u",
		       ring->name, ring->accel_ring.ring_size,
                       ring->accel_ring.ring_desc_size);

	/*
	 * Reserve a fixed number of descriptors per sequencer queue and rely
	 * on separate queue accounting to prevent overflow.
	 */
	q->num_descs = min(ring->accel_ring.ring_size,
	 		   (uint32_t)MAX_PER_QUEUE_SQ_ENTRIES);
	if (q->num_descs == 0) {
		OSAL_LOG_ERROR("No descs available for hw ring %s, ring_size=%u",
			       ring->name, ring->accel_ring.ring_size);
		goto done;
	}

	OSAL_LOG_INFO("get_seq_q_desc_count: hw ring %s: num_descs=%u",
		      ring->name, q->num_descs);
	err = 0;
done:
	return err;
}

static int
get_seq_statusq_desc_count(struct queue *q,
			   storage_seq_qgroup_t qgroup)
{
	q->num_descs = MAX_PER_QUEUE_STATUS_ENTRIES;
	return 0;
}

static int
sonic_cpdc_statusq_count_init(struct per_core_resource *res,
			      int q_count)
{
	uint32_t status_q_count;

	if (q_count < 4)
		return -EINVAL;

	status_q_count = q_count - 2;
	if (status_q_count > MAX_PER_CORE_CPDC_SEQ_STATUS_QUEUES)
		status_q_count = MAX_PER_CORE_CPDC_SEQ_STATUS_QUEUES;
	res->num_cpdc_status_qs = status_q_count;
	return 0;
}

static int
sonic_cpdc_qs_init(struct per_core_resource *res)
{
	int err;

	/* CP queue init */
	err = get_seq_q_desc_count(&res->cp_seq_q, ACCEL_RING_CP);
	if (err)
		goto done;
	err = sonic_seq_q_init(res, &res->cp_seq_q, "cp",
			       0, STORAGE_SEQ_QGROUP_CPDC,
			       SONIC_SEQ_Q_DESC_SIZE);
	if (err) {
		OSAL_LOG_ERROR("sonic_seq_q_init failed for CP, err=%d", err);
		goto done;
	}

	/* DC queue init */
	err = get_seq_q_desc_count(&res->dc_seq_q, ACCEL_RING_DC);
	if (err)
		goto done;
	err = sonic_seq_q_init(res, &res->dc_seq_q, "dc",
			       0, STORAGE_SEQ_QGROUP_CPDC,
			       SONIC_SEQ_Q_DESC_SIZE);
	if (err) {
		OSAL_LOG_ERROR("sonic_seq_q_init failed for DC, err=%d", err);
		goto done;
	}
done:
	return err;
}

static int
sonic_cpdc_qs_reinit(struct per_core_resource *res)
{
	int err;

	/* CP queue init */
	err = sonic_seq_q_reinit(&res->cp_seq_q);
	if (err) {
		OSAL_LOG_ERROR("sonic_seq_q_reinit failed for CP, err=%d", err);
		goto done;
	}

	/* DC queue init */
	err = sonic_seq_q_reinit(&res->dc_seq_q);
	if (err) {
		OSAL_LOG_ERROR("sonic_seq_q_reinit failed for DC, err=%d", err);
		goto done;
	}
done:
	return err;
}

static int
sonic_cpdc_statusqs_init(struct per_core_resource *res)
{
	int err = 0;
	int i;

	for (i = 0; i < res->num_cpdc_status_qs; i++) {
		err = get_seq_statusq_desc_count(&res->cpdc_seq_status_qs[i],
					STORAGE_SEQ_QGROUP_CPDC_STATUS);
		if (err)
			break;
                err = sonic_seq_q_init(res, &res->cpdc_seq_status_qs[i],
                                       "cpdc_status",
                                       i, STORAGE_SEQ_QGROUP_CPDC_STATUS,
                                       SONIC_SEQ_STATUS_Q_DESC_SIZE);
		if (err) {
			OSAL_LOG_ERROR("sonic_seq_q_init failed for CPDC "
				       "status queue %d, err=%d", i, err);
			break;
		}
	}

	return err;
}

static int
sonic_cpdc_statusqs_reinit(struct per_core_resource *res)
{
	int err = 0;
	int i;

	for (i = 0; i < res->num_cpdc_status_qs; i++) {
                err = sonic_seq_q_reinit(&res->cpdc_seq_status_qs[i]);
		if (err) {
			OSAL_LOG_ERROR("sonic_seq_q_reinit failed for CPDC "
				       "status queue %d, err=%d", i, err);
			break;
		}
	}

	return err;
}

static int
sonic_crypto_statusq_count_init(struct per_core_resource *res,
				int q_count)
{
	uint32_t status_q_count;

	if (q_count < 4)
		return -EINVAL;

	status_q_count = q_count - 2;
	if (status_q_count > MAX_PER_CORE_CRYPTO_SEQ_STATUS_QUEUES)
		status_q_count = MAX_PER_CORE_CRYPTO_SEQ_STATUS_QUEUES;
	res->num_crypto_status_qs = status_q_count;
	return 0;
}

static int
sonic_crypto_qs_init(struct per_core_resource *res)
{
	int err;

	/* Encryption queue init */
	err = get_seq_q_desc_count(&res->crypto_enc_seq_q, ACCEL_RING_XTS0);
	if (err)
		goto done;
	err = sonic_seq_q_init(res, &res->crypto_enc_seq_q, "crypto_enc",
			       0, STORAGE_SEQ_QGROUP_CRYPTO,
			       SONIC_SEQ_Q_DESC_SIZE);
	if (err) {
		OSAL_LOG_ERROR("sonic_seq_q_init failed for crypto_enc, "
			       "err=%d", err);
		goto done;
	}

	/* Decryption queue init */
	err = get_seq_q_desc_count(&res->crypto_dec_seq_q, ACCEL_RING_XTS1);
	if (err)
		goto done;
	err = sonic_seq_q_init(res, &res->crypto_dec_seq_q, "crypto_dec",
			       0, STORAGE_SEQ_QGROUP_CRYPTO,
			       SONIC_SEQ_Q_DESC_SIZE);
	if (err) {
		OSAL_LOG_ERROR("sonic_seq_q_init failed for crypto_dec, "
			       "err=%d", err);
		goto done;
	}
done:
	return err;
}

static int
sonic_crypto_qs_reinit(struct per_core_resource *res)
{
	int err;

	err = sonic_seq_q_reinit(&res->crypto_enc_seq_q);
	if (err) {
		OSAL_LOG_ERROR("sonic_seq_q_reinit failed for crypto_enc, "
			       "err=%d", err);
		goto done;
	}

	err = sonic_seq_q_reinit(&res->crypto_dec_seq_q);
	if (err) {
		OSAL_LOG_ERROR("sonic_seq_q_reinit failed for crypto_dec, "
			       "err=%d", err);
		goto done;
	}
done:
	return err;
}

static int
sonic_crypto_statusqs_init(struct per_core_resource *res)
{
	int err = 0;
	int i;

	for (i = 0; i < res->num_crypto_status_qs; i++) {
		err = get_seq_statusq_desc_count(&res->crypto_seq_status_qs[i],
					STORAGE_SEQ_QGROUP_CRYPTO_STATUS);
		if (err)
			break;
                err = sonic_seq_q_init(res, &res->crypto_seq_status_qs[i],
                                       "crypto_status",
                                       i, STORAGE_SEQ_QGROUP_CRYPTO_STATUS,
                                       SONIC_SEQ_STATUS_Q_DESC_SIZE);
		if (err) {
			OSAL_LOG_ERROR("sonic_seq_q_init failed for crypto "
				       "status queue %d, err=%d", i, err);
			break;
		}
	}

	return err;
}

static int
sonic_crypto_statusqs_reinit(struct per_core_resource *res)
{
	int err = 0;
	int i;

	for (i = 0; i < res->num_crypto_status_qs; i++) {
                err = sonic_seq_q_reinit(&res->crypto_seq_status_qs[i]);
		if (err) {
			OSAL_LOG_ERROR("sonic_seq_q_reinit failed for crypto "
				       "status queue %d, err=%d", i, err);
			break;
		}
	}

	return err;
}

static int per_core_resource_request_irq(struct per_core_resource *res)
{
#ifndef __FreeBSD__
	struct device *dev = res->lif->sonic->dev;
#endif
	struct intr *intr = &res->intr;

#ifndef __FreeBSD__
	snprintf(intr->name, sizeof(intr->name),
		 "%s-%s-%d", DRV_NAME, res->lif->name, res->idx);
#else
	/*
	 * BSD will prefix by device name, also limited by space.
	 */
	snprintf(intr->name, sizeof(intr->name),
		 "%d", res->idx);//q->name);
#endif
	return devm_request_irq(dev, intr->vector, sonic_async_ev_isr,
				0, intr->name, res->evl);
}

static int sonic_ev_intr_init(struct per_core_resource *res, int ev_count)
{
	int err;

	err = sonic_create_ev_list(res, ev_count);
	if (err) {
		OSAL_LOG_ERROR("Failed to create ev_list");
		goto done;
	}
	err = sonic_intr_alloc(res->lif, &res->intr);
	if (err) {
		OSAL_LOG_ERROR("Failed to alloc interrupt");
		res->intr.index = INTR_INDEX_NOT_ASSIGNED;
		goto done;
	}
	err = sonic_bus_get_irq(res->lif->sonic, res->intr.index);
	if (err < 0) {
		OSAL_LOG_ERROR("Failed to get irq");
		res->intr.index = INTR_INDEX_NOT_ASSIGNED;
		goto done;
	}
	res->intr.vector = err;
	err = 0;
	sonic_intr_mask(&res->intr, true);
	sonic_intr_mask_on_assertion(&res->intr);
	sonic_intr_coal_set(&res->intr, 0);
	sonic_intr_return_credits(&res->intr, 0, false, false);
	err = per_core_resource_request_irq(res);
	if (err) {
		OSAL_LOG_ERROR("Failed to request irq");
		goto done;
	}

done:
	return err;
}

static int
sonic_lif_per_core_resources_init(struct lif *lif)
{
	struct per_core_resource *res;
	int err = 0;
	int i;
	int pc_q_count = sonic_lif_per_core_seq_q_count(lif);

	OSAL_LOG_INFO("Init per-core-resources, %u seq_queues_per_lif, "
		 "%u num_per_core_resources, %u pc_q_count",
		 lif->sonic->ident->dev.seq_queues_per_lif,
		 lif->sonic->num_per_core_resources, pc_q_count);

	spin_lock_init(&lif->res.lock);
	for (i = 0; i < OSAL_MAX_CORES; i++)
		lif->res.core_to_res_map[i] = -1;

	/*
	 * Allocate per-core queues of the same qgroup horizontally
	 * across cores to ensure they get contiguous queue IDs.
	 */
	LIF_FOR_EACH_PC_RES(lif, i, res) {
                res->lif = lif;
		err = sonic_cpdc_statusq_count_init(res, pc_q_count / 2);
		if (err)
			goto done;
		err = sonic_cpdc_qs_init(res);
		if (err)
			goto done;
	}

	LIF_FOR_EACH_PC_RES(lif, i, res) {
		err = sonic_crypto_statusq_count_init(res, pc_q_count / 2);
		if (err)
			goto done;
		err = sonic_crypto_qs_init(res);
		if (err)
			goto done;
	}

	/*
	 * Now do the same horizontal queue allocations for status queues
	 */
	LIF_FOR_EACH_PC_RES(lif, i, res) {
		err = sonic_cpdc_statusqs_init(res);
		if (err)
			goto done;
	}

	LIF_FOR_EACH_PC_RES(lif, i, res) {
		err = sonic_crypto_statusqs_init(res);
		if (err)
			goto done;
	}
done:
	return err;
}

static int
sonic_lif_per_core_resources_reinit(struct lif *lif)
{
	struct per_core_resource *res;
	int err = 0;
	int i;
	int pc_q_count = sonic_lif_per_core_seq_q_count(lif);

	OSAL_LOG_DEBUG("Reinit per-core-resources, %u seq_queues_per_lif, "
		 "%u num_per_core_resources, %u pc_q_count",
		 lif->sonic->ident->dev.seq_queues_per_lif,
		 lif->sonic->num_per_core_resources, pc_q_count);

	LIF_FOR_EACH_PC_RES(lif, i, res) {
		err = sonic_cpdc_qs_reinit(res);
		if (err)
			goto done;

		err = sonic_crypto_qs_reinit(res);
		if (err)
			goto done;

		err = sonic_cpdc_statusqs_reinit(res);
		if (err)
			goto done;

		err = sonic_crypto_statusqs_reinit(res);
		if (err)
			goto done;
	}
done:
	return err;
}

static int
sonic_lif_per_core_resources_post_init(struct lif *lif)
{
	struct per_core_resource *res;
	int pc_q_count = sonic_lif_per_core_seq_q_count(lif);
	enum cmd_opcode opcode;
	int err = 0;
	int i;

	OSAL_LOG_DEBUG("num_seq_q_batches %u", lif->num_seq_q_batches);
	err = sonic_seq_q_batch_ht_for_all(lif, sonic_lif_seq_q_batch_init, NULL);
	if (err)
		goto done;

	opcode = CMD_OPCODE_SEQ_QUEUE_BATCH_ENABLE;
	err = sonic_seq_q_batch_ht_for_all(lif, sonic_lif_seq_q_batch_control,
					   &opcode);
	if (err)
		goto done;

	LIF_FOR_EACH_PC_RES(lif, i, res) {
		err = sonic_ev_intr_init(res, pc_q_count);
		if (err)
			goto done;

		res->initialized = true;
		sonic_intr_mask(&res->intr, false);
	}
done:
	return err;
}

static int
sonic_lif_per_core_resources_post_reinit(struct lif *lif)
{
	enum cmd_opcode opcode = CMD_OPCODE_SEQ_QUEUE_BATCH_ENABLE;
	int err = 0;

	OSAL_LOG_DEBUG("num_seq_q_batches %u", lif->num_seq_q_batches);
	err = sonic_seq_q_batch_ht_for_all(lif, sonic_lif_seq_q_batch_init, NULL);
	if (!err)
		err = sonic_seq_q_batch_ht_for_all(lif,
				sonic_lif_seq_q_batch_control, &opcode);
	return err;
}

static int sonic_lif_init(struct lif *lif)
{
	struct sonic_dev *idev = &lif->sonic->idev;
	int err;

	err = sonic_debugfs_add_lif(lif);
	if (err)
		return err;

	sonic_dev_cmd_lif_init(idev, lif->index);
	err = sonic_dev_cmd_wait_check(idev, devcmd_timeout);
	if (err)
		return err;

	sonic_dev_cmd_lif_reset(idev, lif->index);
	err = sonic_dev_cmd_wait_check(idev, devcmd_timeout);
	if (err)
		return err;

	err = sonic_lif_adminq_init(lif);
	if (err)
		goto err_out_adminq_deinit;

	err = sonic_lif_notifyq_init(lif);
	if (err)
		goto err_out_adminq_deinit;

	err = sonic_lif_per_core_resources_init(lif);
	if (err)
		goto err_out_notifyq_deinit;

#ifdef __FreeBSD__
	err = sonic_sysctl_init(lif);
	if (err)
		goto err_out_per_core_res_deinit;
#endif

	err = sonic_lif_per_core_resources_post_init(lif);
	if (err)
		goto err_out_per_core_res_deinit;
	err = sonic_lif_seq_q_init_complete(lif);
	if (err && (err != SONIC_DEVCMD_UNKNOWN))
		goto err_out_per_core_res_deinit;

	lif->api_private = NULL;

	lif->flags |= LIF_F_INITED;
	OSAL_LOG_WARN("sonic lif init successful");

	return 0;

err_out_per_core_res_deinit:
	sonic_lif_per_core_resources_deinit(lif);

err_out_notifyq_deinit:
	sonic_lif_qcq_deinit(lif, lif->notifyqcq);

err_out_adminq_deinit:
	sonic_lif_qcq_deinit(lif, lif->adminqcq);
	sonic_lif_reset(lif);
	return err;
}

int sonic_lifs_init(struct sonic *sonic)
{
	struct list_head *cur;
	struct lif *lif;
	int err;

	list_for_each(cur, &sonic->lifs) {
		lif = list_entry(cur, struct lif, list);
		sonic_glif = lif;
		err = sonic_lif_init(lif);
		if (err) {
			sonic_glif = NULL;
			return err;
		}
	}

	return 0;
}

int sonic_lif_reinit(struct lif *lif)
{
	int err;

	err = sonic_qcqs_reinit(lif);
	if (err)
		goto err_out;

	err = sonic_lif_adminq_reinit(lif);
	if (err)
		goto err_out;

	err = sonic_lif_notifyq_reinit(lif);
	if (err)
		goto err_out;

	err = sonic_lif_per_core_resources_reinit(lif);
	if (err)
		goto err_out;

	err = sonic_lif_per_core_resources_post_reinit(lif);
	if (err)
		goto err_out;

	err = sonic_lif_seq_q_init_complete(lif);
	if (err)
		goto err_out;

	sonic_accel_rings_reinit(&lif->sonic->idev);

	lif->flags |= LIF_F_INITED;
	OSAL_LOG_WARN("sonic lif reinit successful");

	return 0;

err_out:
	return err;
}

static int sonic_lif_register(struct lif *lif)
{
	/* noop for offload device */
	lif->registered = true;
	return 0;
}

int sonic_lifs_register(struct sonic *sonic)
{
	struct list_head *cur;
	struct lif *lif;
	int err;

	list_for_each(cur, &sonic->lifs) {
		lif = list_entry(cur, struct lif, list);
		err = sonic_lif_register(lif);
		if (err)
			return err;
	}

	return 0;
}

void sonic_lifs_unregister(struct sonic *sonic)
{
	struct list_head *cur;
	struct lif *lif;

	list_for_each(cur, &sonic->lifs) {
		lif = list_entry(cur, struct lif, list);
		if (lif->registered)
			lif->registered = false;
	}
}

int sonic_lifs_size(struct sonic *sonic)
{
	int err;
	identity_t *ident = sonic->ident;
	unsigned int nintrs, dev_nintrs = ident->dev.num_intrs;

	sonic->num_per_core_resources = core_count; /* module param */
	nintrs = core_count + 1; /* 1 for adminq */
	if (nintrs > dev_nintrs)
		return -ENOSPC;

	err = sonic_bus_alloc_irq_vectors(sonic, nintrs);
	if (err < 0)
		return err;
	sonic->nintrs = nintrs;
	return sonic_debugfs_add_sizes(sonic);
}

static int assign_per_core_res_id(struct lif *lif, int core_id)
{
	int err = -ENOSPC;
	unsigned long free_res_id = -1;

	spin_lock(&lif->res.lock);
	if (lif->res.core_to_res_map[core_id] >= 0) {
		/* another thread already did this */
		spin_unlock(&lif->res.lock);
		OSAL_LOG_INFO("already assigned per core res_id %d for core_id %d",
			      lif->res.core_to_res_map[core_id], core_id);
		return 0;
	}
	free_res_id = find_first_zero_bit(lif->res.pc_res_bmp,
			lif->sonic->num_per_core_resources);
	if (free_res_id >= lif->sonic->num_per_core_resources) {
		spin_unlock(&lif->res.lock);
		OSAL_LOG_ERROR("Per core resource exhausted for core_id %d",
				core_id);
		return err;
	}
	set_bit(free_res_id, lif->res.pc_res_bmp);
	OSAL_ASSERT(lif->res.pc_res[free_res_id]->core_id == -1);
	lif->res.pc_res[free_res_id]->core_id = core_id;
	lif->res.core_to_res_map[core_id] = free_res_id;
	spin_unlock(&lif->res.lock);
	OSAL_LOG_DEBUG("assign per core res_id %lu for core_id %d",
		       free_res_id, core_id);
	return 0;
}

struct per_core_resource *
sonic_get_per_core_res(struct lif *lif)
{
	int err = -ENOSPC;
	int pc_res_idx = -1;
	int core_id;

	core_id = osal_get_coreid();
	OSAL_ASSERT(core_id >= 0 && core_id < OSAL_MAX_CORES);
	pc_res_idx = lif->res.core_to_res_map[core_id];
	if (pc_res_idx < 0) {
		err = assign_per_core_res_id(lif, core_id);
		if (err != 0) {
			OSAL_LOG_ERROR("assign_per_core_res_id failed with error %d",
					err);
			return NULL;
		}
		pc_res_idx = lif->res.core_to_res_map[core_id];
	} else {
		OSAL_ASSERT(lif->res.pc_res[pc_res_idx]->core_id == core_id);
	}

	return lif->res.pc_res[pc_res_idx];
}

uint32_t
sonic_get_num_per_core_res(struct lif *lif)
{
	return lif->sonic->num_per_core_resources;
}

struct per_core_resource *
sonic_get_per_core_res_by_res_id(struct lif *lif, uint32_t res_id)
{
	return res_id < lif->sonic->num_per_core_resources ?
	       lif->res.pc_res[res_id] : NULL;
}

int
sonic_get_per_core_seq_sq(struct per_core_resource *pc_res,
			      enum sonic_queue_type sonic_qtype,
			      struct queue **q)
{
	int err = -EPERM;

	*q = NULL;
	if (pc_res == NULL)
		return err;
	switch (sonic_qtype) {
	case SONIC_QTYPE_CP_SQ:
		*q = &pc_res->cp_seq_q;
		break;
	case SONIC_QTYPE_DC_SQ:
		*q = &pc_res->dc_seq_q;
		break;
	case SONIC_QTYPE_CRYPTO_ENC_SQ:
		*q = &pc_res->crypto_enc_seq_q;
		break;
	case SONIC_QTYPE_CRYPTO_DEC_SQ:
		*q = &pc_res->crypto_dec_seq_q;
		break;
	default:
		return err;
		break;
	}
	return 0;
}

int sonic_get_seq_sq(struct lif *lif, enum sonic_queue_type sonic_qtype,
		struct queue **q)
{
	struct per_core_resource *pc_res = NULL;

	pc_res = sonic_get_per_core_res(lif);
	return sonic_get_per_core_seq_sq(pc_res, sonic_qtype, q);
}

static void
sonic_pprint_bmp(struct per_core_resource *pcr, unsigned long *bmp, int max,
		 spinlock_t *lock, const char *name)
{
	int count = 0;
	unsigned long id;

	spin_lock(lock);
	id = find_first_bit(bmp, max);
	if (id < max) {
		do {
			count++;
			id = find_next_bit(bmp, max, id+1);
		} while (id < max);
	}
	spin_unlock(lock);

	OSAL_LOG_NOTICE("SONIC bmp %s has %d non-zero entries",
			name, count);
}

void
sonic_pprint_seq_bmps(struct per_core_resource *pcr)
{
	sonic_pprint_bmp(pcr, pcr->cpdc_seq_status_qs_bmp, pcr->num_cpdc_status_qs,
			 &pcr->seq_cpdc_statusq_lock, "CPDC STATUS");
	sonic_pprint_bmp(pcr, pcr->crypto_seq_status_qs_bmp, pcr->num_crypto_status_qs,
			 &pcr->seq_crypto_statusq_lock, "CRYPTO STATUS");
}

void sonic_pprint_ev_lists(void)
{
	struct lif *lif = sonic_get_lif();
	struct per_core_resource *res;
	uint32_t i;

	if (!lif)
		return;

	LIF_FOR_EACH_PC_RES(lif, i, res) {
		sonic_pprint_ev_list(res->evl);
	}
}
//EXPORT_SYMBOL(sonic_pprint_ev_lists);

/* Return -1 if not found */
static inline int
sonic_find_next_zero_id(unsigned long *bmp, unsigned int max, spinlock_t *lock,
			int *next_free_id)
{
	int free_id;

	spin_lock(lock);

	free_id = find_next_zero_bit(bmp, max, *next_free_id);
	if (free_id < max)
		goto found;

	free_id = find_first_zero_bit(bmp, *next_free_id);
	if (free_id < *next_free_id)
		goto found;

	spin_unlock(lock);
	return -1;

found:
	set_bit(free_id, bmp);
	*next_free_id = free_id + 1;
	spin_unlock(lock);
	return free_id;
}

int
sonic_get_seq_statusq(struct lif *lif, enum sonic_queue_type sonic_qtype,
		struct queue **q)
{
	int err = -EAGAIN;
	int free_qid = -1;
	struct per_core_resource *pc_res = NULL;

	*q = NULL;

	pc_res = sonic_get_per_core_res(lif);
	if (pc_res == NULL)
		return err;

	//TODO - Change MAX_PER_CORE_CPDC_SEQ_STATUS_QUEUES to actual value
	switch (sonic_qtype) {
	case SONIC_QTYPE_CPDC_STATUS:
		free_qid = sonic_find_next_zero_id(pc_res->cpdc_seq_status_qs_bmp,
						   pc_res->num_cpdc_status_qs,
						   &pc_res->seq_cpdc_statusq_lock,
						   &pc_res->next_cpdc_statusq_id);
		if (free_qid >= 0) {
			*q = &pc_res->cpdc_seq_status_qs[free_qid];
		}
		break;
	case SONIC_QTYPE_CRYPTO_STATUS:
		free_qid = sonic_find_next_zero_id(pc_res->crypto_seq_status_qs_bmp,
						   pc_res->num_crypto_status_qs,
						   &pc_res->seq_crypto_statusq_lock,
						   &pc_res->next_crypto_statusq_id);
		if (free_qid >= 0) {
			*q = &pc_res->crypto_seq_status_qs[free_qid];
		}
		break;
	default:
		err = -EPERM;
		break;
	}

	if (free_qid < 0)
		return err;

	OSAL_ASSERT((*q)->qpos == free_qid);
	return 0;
}

void
sonic_put_seq_statusq(struct queue *q)
{
	struct per_core_resource *pc_res;
	spinlock_t *lock;
	unsigned long *bmp;
	unsigned int max = 0;

	if (q) {
		pc_res = q->pc_res;
		switch (q->qgroup) {
		case STORAGE_SEQ_QGROUP_CPDC_STATUS:
			bmp = pc_res->cpdc_seq_status_qs_bmp;
			max = pc_res->num_cpdc_status_qs;
			lock = &pc_res->seq_cpdc_statusq_lock;
			break;
		case STORAGE_SEQ_QGROUP_CRYPTO_STATUS:
			bmp = pc_res->crypto_seq_status_qs_bmp;
			max = pc_res->num_crypto_status_qs;
			lock = &pc_res->seq_crypto_statusq_lock;
			break;
		default:
			return;
			break;
		}

		spin_lock(lock);
		clear_bit(q->qpos, bmp);
		spin_unlock(lock);
	}
}

static int __attribute__((unused))
sonic_lif_seq_q_legacy_init(struct queue *q)
{
	struct lif *lif = q->lif;
	struct sonic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.seq_queue_init = {
			.opcode = CMD_OPCODE_SEQ_QUEUE_INIT,
			.index = q->qid,
			.pid = q->pid,
			.qgroup = q->qgroup,
			.cos = 0,
			.enable = true, /* TODO: false after adminq speedup */
			.total_wrings = 1,
			.host_wrings = 1,
			.entry_size = ilog2(q->desc_size),
			.wring_size = ilog2(q->num_descs),
			.wring_base = (dma_addr_t)
				sonic_hostpa_to_devpa((uint64_t) q->base_pa),
			.core_id = q->pc_res->core_id,
		},
	};
	int err;

	OSAL_LOG_INFO("seq_q_init.pid %d", ctx.cmd.seq_queue_init.pid);
	OSAL_LOG_INFO("seq_q_init.index %d", ctx.cmd.seq_queue_init.index);
	OSAL_LOG_INFO("seq_q_init.wring_base 0x" PRIx64,
		   ctx.cmd.seq_queue_init.wring_base);
	OSAL_LOG_INFO("seq_q_init.wring_size %d",
		   ctx.cmd.seq_queue_init.wring_size);

	err = sonic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	q->qid = ctx.comp.seq_queue_init.qid;
	q->qtype = ctx.comp.seq_queue_init.qtype;
	q->db = sonic_db_map(q->idev, q);

	OSAL_LOG_INFO("seq_q->qid %d", q->qid);
	OSAL_LOG_INFO("seq_q->qtype %d", q->qtype);
	OSAL_LOG_INFO("seq_q->db " PRIx64, (u64) q->db);

	return err;
}

static int __attribute__((unused))
sonic_lif_seq_q_legacy_control(struct queue *q, uint16_t opcode)
{
	struct lif *lif = q->lif;
	struct sonic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.seq_queue_control = {
			.opcode = opcode,
			.qid = q->qid,
			.qtype = STORAGE_SEQ_QTYPE_SQ,
		},
	};
	int err;

	OSAL_ASSERT((opcode == CMD_OPCODE_SEQ_QUEUE_BATCH_ENABLE) ||
		    (opcode == CMD_OPCODE_SEQ_QUEUE_BATCH_DISABLE));

	OSAL_LOG_INFO("seq_q_control.qid %d",
			ctx.cmd.seq_queue_control.qid);
	OSAL_LOG_INFO("seq_q_control.opcode %d",
			ctx.cmd.seq_queue_control.opcode);

	err = sonic_adminq_post_wait(lif, &ctx);
	if (err) {
		OSAL_LOG_ERROR("seq_q_control failed");
		return err;
	}

	OSAL_LOG_INFO("seq_q_control successful");

	return err;
}

static int
sonic_lif_seq_q_batch_control(struct seq_queue_batch *batch,
			      void *cb_arg)
{
	enum cmd_opcode opcode = *((enum cmd_opcode *)cb_arg);
	struct sonic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.seq_queue_batch_control = {
			.opcode = opcode,
			.qid = batch->base_qid,
			.qtype = STORAGE_SEQ_QTYPE_SQ,
			.num_queues = batch->num_queues,
		},
	};
	int err;

	OSAL_ASSERT((opcode == CMD_OPCODE_SEQ_QUEUE_BATCH_ENABLE) ||
		    (opcode == CMD_OPCODE_SEQ_QUEUE_BATCH_DISABLE));

	OSAL_LOG_INFO("seq_q_batch_control.qid %d",
			ctx.cmd.seq_queue_batch_control.qid);
	OSAL_LOG_INFO("seq_q_batch_control.num_queues %d",
			ctx.cmd.seq_queue_batch_control.num_queues);
	OSAL_LOG_INFO("seq_q_control.opcode %d",
			ctx.cmd.seq_queue_batch_control.opcode);

	err = sonic_adminq_post_wait(batch->lif, &ctx);
	if (err) {
		OSAL_LOG_ERROR("seq_q_batch_control failed");
		return err;
	}

	OSAL_LOG_INFO("seq_q_batch_control successful");

	return err;
}

static int
sonic_lif_seq_q_batch_init(struct seq_queue_batch *batch,
			   void *cb_arg)
{
	struct sonic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.seq_queue_batch_init = {
			.opcode = CMD_OPCODE_SEQ_QUEUE_BATCH_INIT,
			.index = batch->base_qid,
			.num_queues = batch->num_queues,
			.pid = batch->pid,
			.qgroup = batch->qgroup,
			.cos = 0,
			.enable = false,
			.total_wrings = 1,
			.host_wrings = 1,
			.entry_size = ilog2(batch->per_q_desc_size),
			.wring_size = ilog2(batch->per_q_num_descs),
			.wring_base = (dma_addr_t)
				sonic_hostpa_to_devpa((uint64_t) batch->base_pa),
		},
	};
	int err;

	OSAL_LOG_INFO("seq_q_init.pid %d", ctx.cmd.seq_queue_batch_init.pid);
	OSAL_LOG_INFO("seq_q_init.index %d", ctx.cmd.seq_queue_batch_init.index);
	OSAL_LOG_INFO("seq_q_init.num_queues %d",
		      ctx.cmd.seq_queue_batch_init.num_queues);
	OSAL_LOG_INFO("seq_q_init.qgroup %d",
		      ctx.cmd.seq_queue_batch_init.qgroup);
	OSAL_LOG_INFO("seq_q_init.wring_base 0x" PRIx64,
		      ctx.cmd.seq_queue_batch_init.wring_base);
	OSAL_LOG_INFO("seq_q_init.wring_size %d",
		      ctx.cmd.seq_queue_batch_init.wring_size);

	err = sonic_adminq_post_wait(batch->lif, &ctx);
	if (err)
		return err;

	return err;
}

static int
sonic_lif_seq_q_init_complete(struct lif *lif)
{
	struct sonic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.seq_queue_init_complete = {
			.opcode = CMD_OPCODE_SEQ_QUEUE_INIT_COMPLETE,
		},
	};
	int err;

	err = sonic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	return err;
}

