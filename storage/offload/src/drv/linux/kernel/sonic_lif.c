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

#include <linux/interrupt.h>

#include "sonic.h"
#include "sonic_bus.h"
#include "sonic_lif.h"
#include "sonic_debugfs.h"

/*static bool sonic_adminq_service(struct cq *cq, struct cq_info *cq_info,
				 void *cb_arg)
{
	struct admin_cpl *comp = cq_info->cq_desc;

	if (comp->color != cq->done_color)
		return false;

	sonic_q_service(cq->bound_q, cq_info, comp->cpl_index);

	return true;
}*/


static irqreturn_t sonic_isr(int irq, void *data)
{
	return IRQ_HANDLED;
}

int sonic_intr_alloc(struct lif *lif, struct intr *intr)
{
	struct sonic *sonic = lif->sonic;
	struct sonic_dev *idev = &sonic->idev;
	unsigned long index;

	index = find_first_zero_bit(sonic->intrs, sonic->nintrs);
	if (index == sonic->nintrs)
		return -ENOSPC;
	set_bit(index, sonic->intrs);

	return sonic_intr_init(idev, intr, index);
}

void sonic_intr_free(struct lif *lif, struct intr *intr)
{
	if (intr->index != INTR_INDEX_NOT_ASSIGNED)
		clear_bit(intr->index, lif->sonic->intrs);
}

int sonic_lifs_alloc(struct sonic *sonic)
{
	return 0;
}

void sonic_lifs_free(struct sonic *sonic)
{
}

static void sonic_lif_qcq_deinit(struct lif *lif, struct qcq *qcq)
{
}

static void sonic_lif_txqs_deinit(struct lif *lif)
{
}

void sonic_lifs_deinit(struct sonic *sonic)
{
}

static int sonic_request_irq(struct lif *lif, struct qcq *qcq)
{
	struct device *dev = lif->sonic->dev;
	struct intr *intr = &qcq->intr;
	struct queue *q = &qcq->q;

	snprintf(intr->name, sizeof(intr->name),
		 "%s-%s-%s", DRV_NAME, lif->name, q->name);
	return devm_request_irq(dev, intr->vector, sonic_isr,
				0, intr->name, NULL);
	return 0;
}

static int sonic_lif_adminq_init(struct lif *lif)
{
	struct sonic_dev *idev = &lif->sonic->idev;
	struct qcq *qcq = lif->adminqcq;
	struct queue *q = &qcq->q;
	struct adminq_init_cpl comp;
	int err;

	sonic_dev_cmd_adminq_init(idev, q, 0, lif->index, 0);
	err = sonic_dev_cmd_wait_check(idev, HZ * devcmd_timeout);
	if (err)
		return err;

	sonic_dev_cmd_comp(idev, &comp);
	q->qid = comp.qid;
	q->qtype = comp.qtype;
	q->db = sonic_db_map(idev, q);

	err = sonic_request_irq(lif, qcq);
	if (err) {
		return err;
	}

	qcq->flags |= QCQ_F_INITED;

	/* Enabling interrupts on adminq from here on... */
	sonic_intr_mask(&lif->adminqcq->intr, false);

	return sonic_debugfs_add_qcq(lif, qcq);
}

static int sonic_lif_txqs_init(struct lif *lif)
{
	return 0;
}

static int sonic_lif_rxqs_init(struct lif *lif)
{
	return 0;
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

	err = sonic_lif_txqs_init(lif);
	if (err)
		goto err_out_adminq_deinit;

	err = sonic_lif_rxqs_init(lif);
	if (err)
		goto err_out_txqs_deinit;

	lif->api_private = NULL;

	lif->flags |= LIF_F_INITED;

	return 0;

err_out_txqs_deinit:
	sonic_lif_txqs_deinit(lif);
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
		err = sonic_lif_init(lif);
		if (err)
			return err;
	}

	return 0;
}

int sonic_lif_register(struct lif *lif)
{
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
		if (lif->registered) {
			lif->registered = false;
		}
	}
}

int sonic_lifs_size(struct sonic *sonic)
{
	return 0;
}
