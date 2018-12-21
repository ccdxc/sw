/*
 * Copyright 2017-2018 Pensando Systems, Inc.  All rights reserved.
 *
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <linux/netdevice.h>
#include <linux/interrupt.h>
#include <linux/hardirq.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>

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
	return sonic_napi(napi, budget, sonic_adminq_service, NULL);
}
#else
static void sonic_adminq_napi(struct napi_struct *napi)
{
	int budget = NAPI_POLL_WEIGHT;
	int work_done;

	work_done = sonic_napi(napi, budget, sonic_adminq_service, NULL);

	if (work_done == budget)
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

void sonic_q_free(struct lif *lif, struct queue *q)
{
	if (q->info) {
		devm_kfree(lif->sonic->dev, q->info);
		q->info = NULL;
	}
}

int sonic_q_alloc(struct lif *lif, struct queue *q,
	unsigned int num_descs, unsigned int desc_size, bool do_alloc_descs)
{
	q->info = devm_kzalloc(lif->sonic->dev, sizeof(*q->info) * num_descs,
			       GFP_KERNEL);
	if (!q->info)
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
	}

	return 0;
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

	new->base = dma_zalloc_coherent(dev, total_size, &new->base_pa,
				 GFP_KERNEL);
	if (!new->base) {
		err = -ENOMEM;
		goto err_out_free_intr;
	}

	new->total_size = total_size;

	q_base = new->base;
	q_base_pa = new->base_pa;

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

static void sonic_qcq_free(struct lif *lif, struct qcq *qcq)
{
	if (!qcq)
		return;

	sonic_q_free(lif, &qcq->q);
	dma_free_coherent(lif->sonic->dev, qcq->total_size, qcq->base,
	qcq->base_pa);
	sonic_intr_free(lif, &qcq->intr);
}

static unsigned int sonic_pid_get(struct lif *lif, unsigned int page)
{
	unsigned int ndbpgs_per_lif = lif->sonic->ident->dev.db_pages_per_lif;

	BUG_ON(ndbpgs_per_lif < page + 1);

	return lif->index * ndbpgs_per_lif + page;
}

static int sonic_qcqs_alloc(struct lif *lif)
{
	unsigned int flags;
	unsigned int pid;
	int err = -ENOMEM;

	pid = sonic_pid_get(lif, 0);
	flags = QCQ_F_INTR;
	err = sonic_qcq_alloc(lif, 0, "admin", flags, 1 << 4,
			sizeof(struct admin_cmd),
			sizeof(struct admin_cpl),
			pid, &lif->adminqcq);
	return err;
}

static void sonic_qcqs_free(struct lif *lif)
{
	sonic_qcq_free(lif, lif->adminqcq);
}

static int sonic_lif_per_core_resources_alloc(struct lif *lif)
{
	int err = -ENOMEM;
	int i, j;
#ifndef __FreeBSD__
	struct device *dev = lif->sonic->dev;
#endif

	for (i = 0; i < lif->sonic->num_per_core_resources; i++) {
		lif->res.pc_res[i] = devm_kzalloc(dev,
				sizeof(*lif->res.pc_res[0]), GFP_KERNEL);
		if (!lif->res.pc_res[i])
			goto err_out_cleanup;
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

	snprintf(lif->name, sizeof(lif->name), "lif%u", index);

	spin_lock_init(&lif->adminq_lock);

#if 0
	spin_lock_init(&lif->deferred.lock);
	INIT_LIST_HEAD(&lif->deferred.list);
	INIT_WORK(&lif->deferred.work, sonic_lif_deferred_work);
#endif

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

void sonic_lifs_free(struct sonic *sonic)
{
	struct list_head *cur, *tmp;
	struct lif *lif;

	list_for_each_safe(cur, tmp, &sonic->lifs) {
		lif = list_entry(cur, struct lif, list);
		list_del(&lif->list);
		flush_scheduled_work();
		sonic_qcqs_free(lif);
		sonic_per_core_resources_free(lif);
	}
}

static void sonic_lif_qcq_deinit(struct lif *lif, struct qcq *qcq)
{
#ifndef __FreeBSD__
	struct device *dev = lif->sonic->dev;
#endif

	if (!(qcq->flags & QCQ_F_INITED))
		return;
	sonic_intr_mask(&qcq->intr, true);
	synchronize_irq(qcq->intr.vector);
	devm_free_irq(dev, qcq->intr.vector, &qcq->napi);
	netif_napi_del(&qcq->napi);
	qcq->flags &= ~QCQ_F_INITED;
}

static void sonic_cpdc_qs_deinit(struct per_core_resource *res)
{
	int i;

	/* Free status queues */
	for (i = 0; i < res->num_cpdc_status_qs; i++)
		sonic_q_free(res->lif, &res->cpdc_seq_status_qs[i]);

	/* Free submission queues */
	sonic_q_free(res->lif, &res->dc_seq_q);
	sonic_q_free(res->lif, &res->cp_seq_q);
}

static void sonic_crypto_qs_deinit(struct per_core_resource *res)
{
	int i;

	/* Free status queues */
	for (i = 0; i < res->num_crypto_status_qs; i++)
		sonic_q_free(res->lif, &res->crypto_seq_status_qs[i]);

	/* Free submission queues */
	sonic_q_free(res->lif, &res->crypto_dec_seq_q);
	sonic_q_free(res->lif, &res->crypto_enc_seq_q);
}

static void sonic_ev_intr_deinit(struct per_core_resource *res)
{
	if (!res->evl)
		return;

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
	int i;

	pnso_deinit();
	for (i = 0; i < lif->sonic->num_per_core_resources; i++) {
		sonic_lif_per_core_resource_deinit(lif->res.pc_res[i]);
	}
}

static void sonic_lif_deinit(struct lif *lif)
{
	sonic_lif_qcq_deinit(lif, lif->adminqcq);
	sonic_lif_per_core_resources_deinit(lif);
}

/*
 * Global variable to track the lif
 * Unlike network device driver, offload driver does not get lif handle
 * as part of api. Tracking lif via global handle is a bit ugly but will make
 * upper level interfaces less clumsy
 */
static struct lif *sonic_glif;

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
	err = sonic_dev_cmd_wait_check(idev, HZ * devcmd_timeout);
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

static int sonic_cpdc_q_init(struct per_core_resource *res, struct queue *q,
		unsigned int qpos, int qgroup, uint32_t num_descs,
		uint16_t desc_size)
{
	int err;
	unsigned int pid;

	pid = sonic_pid_get(res->lif, 0);

	err = sonic_q_alloc(res->lif, q, num_descs, desc_size, true);
	if (err)
		return err;

	err = sonic_q_init(res->lif, &res->lif->sonic->idev, q,
			   res->lif->seq_q_index++, "cpdc",
			   num_descs, desc_size, pid);
	if (err) {
		sonic_q_free(res->lif, q);
		return err;
	}
	q->pc_res = res;
	q->qpos = qpos;
	q->qtype = STORAGE_SEQ_QTYPE_SQ;
	q->qgroup = qgroup;
	sonic_q_map(q, q->base, q->base_pa);

	return err;
}

static int get_seq_q_desc_count(uint32_t status_q_count,
				struct per_core_resource *res,
				uint32_t ring_id, uint32_t *desc_count)
{
	int err = -EINVAL;
	struct accel_ring *ring;

	if (ring_id >= ACCEL_RING_ID_MAX)
		goto done;

	ring = &res->lif->sonic->ident->dev.accel_ring_tbl[ring_id];

	OSAL_LOG_DEBUG("get_seq_q_desc_count: hw ring %u: size=%u, desc_size=%u.\n",
		ring_id, ring->ring_size, ring->ring_desc_size);

	/*
	 * Reserve a fixed number of descriptors per sequencer queue and rely
	 * on separate queue accounting to prevent overflow.
	 */
	*desc_count = min(ring->ring_size, (uint32_t)MAX_PER_QUEUE_SQ_ENTRIES);
	if (*desc_count == 0) {
		OSAL_LOG_ERROR("No descs available for hw ring %d, ring_size=%u.\n",
			ring_id, ring->ring_size);
		goto done;
	}

	OSAL_LOG_INFO("get_seq_q_desc_count: hw ring %u: q_entries_per_core=%u.\n",
		 ring_id, *desc_count);

	err = 0;

done:
	return err;
}

static int sonic_cpdc_qs_init(struct per_core_resource *res,
			      int q_count)
{
	int err;
	int i;
	uint32_t status_q_count;
	uint32_t desc_count;

	if (q_count < 4)
		return -EINVAL;

	/* Calculate status_q_count */
	status_q_count = q_count - 2;
	if (status_q_count > MAX_PER_CORE_CPDC_SEQ_STATUS_QUEUES)
		status_q_count = MAX_PER_CORE_CPDC_SEQ_STATUS_QUEUES;
	res->num_cpdc_status_qs = status_q_count;

	/* CP queue init */
	err = get_seq_q_desc_count(status_q_count, res,
			ACCEL_RING_CP, &desc_count);
	if (err)
		goto done;
	err = sonic_cpdc_q_init(res, &res->cp_seq_q, 0, STORAGE_SEQ_QGROUP_CPDC,
				desc_count, SONIC_SEQ_Q_DESC_SIZE);
	if (err) {
		OSAL_LOG_ERROR("sonic_cpdc_q_init failed for CP, err=%d\n", err);
		goto done;
	}

	/* DC queue init */
	err = get_seq_q_desc_count(status_q_count, res,
			ACCEL_RING_DC, &desc_count);
	if (err)
		goto done;
	err = sonic_cpdc_q_init(res, &res->dc_seq_q, 0, STORAGE_SEQ_QGROUP_CPDC,
				desc_count, SONIC_SEQ_Q_DESC_SIZE);
	if (err) {
		OSAL_LOG_ERROR("sonic_cpdc_q_init failed for DC, err=%d\n", err);
		goto done;
	}

	/* Status queues init */
	for (i = 0; i < status_q_count; i++) {
		err = sonic_cpdc_q_init(res, &res->cpdc_seq_status_qs[i], i,
					STORAGE_SEQ_QGROUP_CPDC_STATUS,
					MAX_PER_QUEUE_STATUS_ENTRIES,
					SONIC_SEQ_STATUS_Q_DESC_SIZE);
		if (err) {
			OSAL_LOG_ERROR("sonic_cpdc_q_init failed for CPDC status queue %d, err=%d\n",
				i, err);
			goto done;
		}
	}
done:
	return err;
}

static int sonic_crypto_q_init(struct per_core_resource *res,
			       struct queue *q, unsigned int qpos, int qgroup,
			       uint32_t num_descs, uint16_t desc_size)
{
	int err;
	unsigned int pid;

	pid = sonic_pid_get(res->lif, 0);

	err = sonic_q_alloc(res->lif, q, num_descs, desc_size, true);
	if (err)
		return err;
	err = sonic_q_init(res->lif, &res->lif->sonic->idev, q,
			   res->lif->seq_q_index++, "crypto",
			   num_descs, desc_size, pid);
	if (err) {
		sonic_q_free(res->lif, q);
		return err;
	}
	q->pc_res = res;
	q->qpos = qpos;
	q->qtype = STORAGE_SEQ_QTYPE_SQ;
	q->qgroup = qgroup;
	sonic_q_map(q, q->base, q->base_pa);

	return err;
}

static int sonic_crypto_qs_init(struct per_core_resource *res,
				int q_count)
{
	int err;
	int i;
	uint32_t status_q_count;
	uint32_t desc_count;

	if (q_count < 4)
		return -EINVAL;

	/* Calculate status_q_count */
	status_q_count = q_count - 2;
	if (status_q_count > MAX_PER_CORE_CRYPTO_SEQ_STATUS_QUEUES)
		status_q_count = MAX_PER_CORE_CRYPTO_SEQ_STATUS_QUEUES;
	res->num_crypto_status_qs = status_q_count;

	/* Encryption queue init */
	err = get_seq_q_desc_count(status_q_count, res,
			ACCEL_RING_XTS0, &desc_count);
	if (err)
		goto done;
	err = sonic_crypto_q_init(res, &res->crypto_enc_seq_q, 0,
				  STORAGE_SEQ_QGROUP_CRYPTO,
				  desc_count, SONIC_SEQ_Q_DESC_SIZE);
	if (err)
		goto done;

	/* Decryption queue init */
	err = get_seq_q_desc_count(status_q_count, res,
			ACCEL_RING_XTS1, &desc_count);
	if (err)
		goto done;
	err = sonic_crypto_q_init(res, &res->crypto_dec_seq_q, 0,
				  STORAGE_SEQ_QGROUP_CRYPTO,
				  desc_count, SONIC_SEQ_Q_DESC_SIZE);
	if (err)
		goto done;

	/* Status queues init */
	for (i = 0; i < status_q_count; i++) {
		err = sonic_crypto_q_init(res, &res->crypto_seq_status_qs[i], i,
					  STORAGE_SEQ_QGROUP_CRYPTO_STATUS,
					  MAX_PER_QUEUE_STATUS_ENTRIES,
					  SONIC_SEQ_STATUS_Q_DESC_SIZE);
		if (err)
			goto done;
	}

done:
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

static int sonic_lif_per_core_resource_init(struct lif *lif,
					    struct per_core_resource *res,
					    int seq_q_count)
{
	int err;

	/* Initialize local driver structures */
	res->lif = lif;
	res->core_id = -1; /* determined later */
	spin_lock_init(&res->seq_statusq_lock);

	err = sonic_cpdc_qs_init(res, seq_q_count / 2);
	if (err)
		goto done;

	err = sonic_crypto_qs_init(res, seq_q_count / 2);
	if (err)
		goto done;

	/* Send commands to initialize/enable hw queues. TODO: move to pnso_init */
	err = sonic_lif_cpdc_seq_qs_init(res);
	if (err)
		goto done;

	err = sonic_lif_crypto_seq_qs_init(res);
	if (err)
		goto done;

#if 0
	err = sonic_lif_cpdc_seq_qs_control(res, CMD_OPCODE_SEQ_QUEUE_ENABLE);
	if (err)
		goto done;

	err = sonic_lif_crypto_seq_qs_control(res, CMD_OPCODE_SEQ_QUEUE_ENABLE);
	if (err)
		goto done;
#endif

	err = sonic_ev_intr_init(res, seq_q_count);
	if (err)
		goto done;

	OSAL_LOG_NOTICE("Successully initialized per_core_resource");

	res->initialized = true;
	sonic_intr_mask(&res->intr, false);
done:
	return err;
}

static int
sonic_lif_per_core_resources_init(struct lif *lif)
{
	int err = 0;
	int i;
	int q_count = lif->sonic->ident->dev.seq_queues_per_lif;

	OSAL_LOG_INFO("Init per-core-resources, %u seq_queues_per_lif, %u num_per_core_resources.\n",
		 lif->sonic->ident->dev.seq_queues_per_lif,
		 lif->sonic->num_per_core_resources);

	for (i = 0; i < OSAL_MAX_CORES; i++)
		lif->res.core_to_res_map[i] = -1;

	for (i = 0; i < lif->sonic->num_per_core_resources; i++) {
		if (lif->res.pc_res[i]) {
			lif->res.pc_res[i]->idx = i;
			err = sonic_lif_per_core_resource_init(lif,
				lif->res.pc_res[i],
				q_count / lif->sonic->num_per_core_resources);
			if (err)
				break;
		}
	}

	spin_lock_init(&lif->res.lock);

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
	err = sonic_dev_cmd_wait_check(idev, HZ * devcmd_timeout);
	if (err)
		return err;

	err = sonic_lif_adminq_init(lif);
	if (err)
		goto err_out_adminq_deinit;

	err = sonic_lif_per_core_resources_init(lif);
	if (err)
		goto err_out_per_core_res_deinit;

	lif->api_private = NULL;

	lif->flags |= LIF_F_INITED;
	OSAL_LOG_WARN("sonic lif init successful\n");

	return 0;

err_out_per_core_res_deinit:
	sonic_lif_per_core_resources_deinit(lif);

err_out_adminq_deinit:
	sonic_lif_qcq_deinit(lif, lif->adminqcq);

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
		OSAL_LOG_INFO("already assigned per core res_id %d for core_id %d\n",
			      lif->res.core_to_res_map[core_id], core_id);
		return 0;
	}
	free_res_id = find_first_zero_bit(lif->res.pc_res_bmp,
			lif->sonic->num_per_core_resources);
	if (free_res_id >= lif->sonic->num_per_core_resources) {
		spin_unlock(&lif->res.lock);
		OSAL_LOG_ERROR("Per core resource exhausted for core_id %d\n",
				core_id);
		return err;
	}
	set_bit(free_res_id, lif->res.pc_res_bmp);
	OSAL_ASSERT(lif->res.pc_res[free_res_id]->core_id == -1);
	lif->res.pc_res[free_res_id]->core_id = core_id;
	lif->res.core_to_res_map[core_id] = free_res_id;
	spin_unlock(&lif->res.lock);
	OSAL_LOG_DEBUG("assign per core res_id %lu for core_id %d\n",
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
			OSAL_LOG_ERROR("assign_per_core_res_id failed with error %d\n",
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
sonic_pprint_bmp(struct per_core_resource *pcr, unsigned long *bmp, int max, const char *name)
{
	int count = 0;
	unsigned long id;

	spin_lock(&pcr->seq_statusq_lock);
	id = find_first_bit(bmp, max);
	if (id < max) {
		do {
			count++;
			id = find_next_bit(bmp, max, id+1);
		} while (id < max);
	}
	spin_unlock(&pcr->seq_statusq_lock);

	OSAL_LOG_NOTICE("SONIC bmp %s has %d non-zero entries\n",
			name, count);
}

void
sonic_pprint_seq_bmps(struct per_core_resource *pcr)
{
	sonic_pprint_bmp(pcr, pcr->cpdc_seq_status_qs_bmp, pcr->num_cpdc_status_qs, "CPDC STATUS");
	sonic_pprint_bmp(pcr, pcr->crypto_seq_status_qs_bmp, pcr->num_crypto_status_qs, "CRYPTO STATUS");
}

int
sonic_get_seq_statusq(struct lif *lif, enum sonic_queue_type sonic_qtype,
		struct queue **q)
{
	int err = -EPERM;
	unsigned long free_qid = -1;
	unsigned long *bmp;
	struct per_core_resource *pc_res = NULL;
	int max = 0;

	*q = NULL;

	pc_res = sonic_get_per_core_res(lif);
	if (pc_res == NULL)
		return err;

	//TODO - Change MAX_PER_CORE_CPDC_SEQ_STATUS_QUEUES to actual value
	switch (sonic_qtype) {
	case SONIC_QTYPE_CPDC_STATUS:
		bmp = pc_res->cpdc_seq_status_qs_bmp;
		max = pc_res->num_cpdc_status_qs;
		break;
	case SONIC_QTYPE_CRYPTO_STATUS:
		bmp = pc_res->crypto_seq_status_qs_bmp;
		max = pc_res->num_crypto_status_qs;
		break;
	default:
		return err;
		break;
	}

	spin_lock(&pc_res->seq_statusq_lock);
	free_qid = find_first_zero_bit(bmp, max);
	if (free_qid >= max) {
		spin_unlock(&pc_res->seq_statusq_lock);
		return err;
	}
	set_bit(free_qid, bmp);
	spin_unlock(&pc_res->seq_statusq_lock);

	switch (sonic_qtype) {
	case SONIC_QTYPE_CPDC_STATUS:
		*q = &pc_res->cpdc_seq_status_qs[free_qid];
		break;
	case SONIC_QTYPE_CRYPTO_STATUS:
		*q = &pc_res->crypto_seq_status_qs[free_qid];
		break;
	default:
		return err;
		break;
	}

	OSAL_ASSERT((*q)->qpos == free_qid);
	return 0;
}

void
sonic_put_seq_statusq(struct queue *q)
{
	struct per_core_resource *pc_res;
	unsigned long *bmp;
	int max = 0;

	if (q) {
		pc_res = q->pc_res;
		switch (q->qgroup) {
		case STORAGE_SEQ_QGROUP_CPDC_STATUS:
			bmp = pc_res->cpdc_seq_status_qs_bmp;
			max = pc_res->num_cpdc_status_qs;
			break;
		case STORAGE_SEQ_QGROUP_CRYPTO_STATUS:
			bmp = pc_res->crypto_seq_status_qs_bmp;
			max = pc_res->num_crypto_status_qs;
			break;
		default:
			return;
			break;
		}

		spin_lock(&pc_res->seq_statusq_lock);
		clear_bit(q->qpos, bmp);
		spin_unlock(&pc_res->seq_statusq_lock);
	}
}

struct lif *sonic_get_lif(void)
{
	return sonic_glif;
}

struct sonic_dev *sonic_get_idev(void)
{
	return sonic_glif && sonic_glif->sonic ?
	       &sonic_glif->sonic->idev : NULL;
}

static int sonic_lif_seq_q_init(struct queue *q)
{
	struct lif *lif = q->lif;
	struct sonic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.seq_queue_init = {
			.opcode = CMD_OPCODE_SEQ_QUEUE_INIT,
			.index = q->index,
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

	OSAL_LOG_INFO("seq_q_init.pid %d\n", ctx.cmd.seq_queue_init.pid);
	OSAL_LOG_INFO("seq_q_init.index %d\n", ctx.cmd.seq_queue_init.index);
	OSAL_LOG_INFO("seq_q_init.wring_base 0x" PRIx64 "\n",
		   ctx.cmd.seq_queue_init.wring_base);
	OSAL_LOG_INFO("seq_q_init.wring_size %d\n",
		   ctx.cmd.seq_queue_init.wring_size);

	err = sonic_adminq_post_wait(lif, &ctx);
	if (err)
		return err;

	q->qid = ctx.comp.seq_queue_init.qid;
	q->qtype = ctx.comp.seq_queue_init.qtype;
	q->db = sonic_db_map(q->idev, q);

	if (err)
		return err;

	//q->flags |= QCQ_F_INITED;

	OSAL_LOG_INFO("seq_q->qid %d\n", q->qid);
	OSAL_LOG_INFO("seq_q->qtype %d\n", q->qtype);
	OSAL_LOG_INFO("seq_q->db " PRIx64 "\n", (u64) q->db);
	// err = sonic_debugfs_add_q(lif, q);
	return err;
}

static void sonic_lif_seq_q_deinit(struct queue *q)
{
	/* TODO */
}

int sonic_lif_cpdc_seq_qs_init(struct per_core_resource *res)
{
	unsigned int i = 0;
	int err;

	/* Initalize cpdc queues */
	err = sonic_lif_seq_q_init(&res->cp_seq_q);
	if (err)
		goto done;
	err = sonic_lif_seq_q_init(&res->dc_seq_q);
	if (err)
		goto err_out_dc;
	for (i = 0; i < res->num_cpdc_status_qs; i++) {
		err = sonic_lif_seq_q_init(&res->cpdc_seq_status_qs[i]);
		if (err)
			goto err_out_cpdc_status;
	}
done:
	return err;

err_out_cpdc_status:
	for (; i > 0; i--)
		sonic_lif_seq_q_deinit(&res->cpdc_seq_status_qs[i-1]);
	sonic_lif_seq_q_deinit(&res->dc_seq_q);
err_out_dc:
	sonic_lif_seq_q_deinit(&res->cp_seq_q);

	return err;
}

int sonic_lif_crypto_seq_qs_init(struct per_core_resource *res)
{
	unsigned int i = 0;
	int err;

	/* Initalize crypto queues */
	err = sonic_lif_seq_q_init(&res->crypto_enc_seq_q);
	if (err)
		goto done;
	err = sonic_lif_seq_q_init(&res->crypto_dec_seq_q);
	if (err)
		goto err_out_crypto_dec;
	for (i = 0; i < res->num_crypto_status_qs; i++) {
		err = sonic_lif_seq_q_init(&res->crypto_seq_status_qs[i]);
		if (err)
			goto err_out_crypto_status;
	}

done:
	return err;

err_out_crypto_status:
	for (; i > 0; i--)
		sonic_lif_seq_q_deinit(&res->crypto_seq_status_qs[i-1]);
	sonic_lif_seq_q_deinit(&res->crypto_dec_seq_q);
err_out_crypto_dec:
	sonic_lif_seq_q_deinit(&res->crypto_enc_seq_q);

	return err;
}

static int
sonic_lif_seq_q_control(struct queue *q, uint16_t opcode)
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

	BUG_ON(opcode != CMD_OPCODE_SEQ_QUEUE_ENABLE &&
	       opcode != CMD_OPCODE_SEQ_QUEUE_DISABLE);

	OSAL_LOG_INFO("seq_q_control.qid %d\n",
			ctx.cmd.seq_queue_control.qid);
	OSAL_LOG_INFO("seq_q_control.opcode %d\n",
			ctx.cmd.seq_queue_control.opcode);

	err = sonic_adminq_post_wait(lif, &ctx);
	if (err) {
		OSAL_LOG_ERROR("seq_q_control failed\n");
		return err;
	}

	OSAL_LOG_INFO("seq_q_control successful\n");

	return err;
}

int
sonic_lif_cpdc_seq_qs_control(struct per_core_resource *res, uint16_t opcode)
{
	unsigned int i = 0;
	int err;

	/* Control cpdc queues */
	err = sonic_lif_seq_q_control(&res->cp_seq_q, opcode);
	if (err)
		goto done;
	err = sonic_lif_seq_q_control(&res->dc_seq_q, opcode);
	if (err)
		goto done;
	for (i = 0; i < res->num_cpdc_status_qs; i++) {
		err = sonic_lif_seq_q_control(&res->cpdc_seq_status_qs[i],
				opcode);
		if (err)
			goto done;
	}
done:
	return err;
}

int
sonic_lif_crypto_seq_qs_control(struct per_core_resource *res, uint16_t opcode)
{
	unsigned int i = 0;
	int err;

	/* Control crypto queues */
	err = sonic_lif_seq_q_control(&res->crypto_enc_seq_q, opcode);
	if (err)
		goto done;
	err = sonic_lif_seq_q_control(&res->crypto_dec_seq_q, opcode);
	if (err)
		goto done;
	for (i = 0; i < res->num_crypto_status_qs; i++) {
		err = sonic_lif_seq_q_control(&res->crypto_seq_status_qs[i],
				opcode);
		if (err)
			goto done;
	}

done:
	return err;
}

#if 0
static int
sonic_lif_hang_notify(struct lif *lif)
{
	struct sonic_admin_ctx ctx = {
		.work = COMPLETION_INITIALIZER_ONSTACK(ctx.work),
		.cmd.hang_notify = {
			.opcode = CMD_OPCODE_HANG_NOTIFY,
		},
	};
	int err;

	OSAL_LOG_INFO("hang_notify query\n");

	err = sonic_adminq_post_wait(lif, &ctx);
	if (err) {
		OSAL_LOG_INFO("hang_notify query failed\n");
		return err;
	}

	OSAL_LOG_INFO("hang_notify query successful, status %u\n",
		 ctx.comp.hang_notify.status);
	err = ctx.comp.hang_notify.status ? -EFAULT : 0; /* TODO */

	return err;
}
#endif
