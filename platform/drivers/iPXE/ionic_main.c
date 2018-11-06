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

#include "ionic.h"

unsigned int ntxq_descs = 1024;
unsigned int nrxq_descs = 1024;

/* Find last set bit; fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32 */
static int fls(int x)
{
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}

static inline u32 ilog2(u32 mem)
{
	return (fls(mem) - 1);
}

static int ionic_dev_cmd_wait(struct ionic_dev *idev, unsigned long max_wait)
{
	unsigned long wait;
	int done;

	// Wait for dev cmd to complete...but no more than max_wait
	for (wait = 0; wait < max_wait; wait++) {
		done = ionic_dev_cmd_done(idev);
		if (done)
			return 0;

		// Delay
		mdelay(1000);
	}

	DBG("%s :: timeout happened\n", __FUNCTION__);
	return -ETIMEDOUT;
}

static int ionic_dev_cmd_check_error(struct ionic_dev *idev)
{
	u8 status;

	status = ionic_dev_cmd_status(idev);
	switch (status) {
	case 0:
		return 0;
	}

	return -EIO;
}

int ionic_dev_cmd_wait_check(struct ionic_dev *idev, unsigned long max_wait)
{
	int err;

	err = ionic_dev_cmd_wait(idev, max_wait);
	if (err) {
		return err;
	}

	return ionic_dev_cmd_check_error(idev);
}

u8 ionic_dev_cmd_status(struct ionic_dev *idev)
{
	return readb(&idev->dev_cmd->comp.status);
}

bool ionic_dev_cmd_done(struct ionic_dev *idev)
{
	return readl(&idev->dev_cmd->done) & DEV_CMD_DONE;
}

void ionic_dev_cmd_go(struct ionic_dev *idev, union dev_cmd *cmd)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(cmd->words); i++) {
		writel(cmd->words[i], &idev->dev_cmd->cmd.words[i]);
	}

	writel(0, &idev->dev_cmd->done);
	writel(1, &idev->dev_cmd_db->v);
}

void ionic_dev_cmd_reset(struct ionic_dev *idev)
{
	union dev_cmd cmd = {
		.reset.opcode = CMD_OPCODE_RESET,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_identify(struct ionic_dev *idev, u16 ver, dma_addr_t addr)
{
	union dev_cmd cmd = {
		.identify.opcode = CMD_OPCODE_IDENTIFY,
		.identify.ver = ver,
		.identify.addr = addr,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

char *ionic_dev_asic_name(u8 asic_type)
{
	switch (asic_type) {
	case ASIC_TYPE_CAPRI:
		return "Capri";
	default:
		return "Unknown";
	}
}

void ionic_dev_cmd_lif_init(struct ionic_dev *idev, u32 index)
{
	union dev_cmd cmd = {
		.lif_init.opcode = CMD_OPCODE_LIF_INIT,
		.lif_init.index = index,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_comp(struct ionic_dev *idev, void *mem)
{
	union dev_cmd_comp *comp = mem;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(comp->words); i++)
		comp->words[i] = readl(&idev->dev_cmd->comp.words[i]);
}

void ionic_dev_cmd_adminq_init(struct ionic_dev *idev, struct queue *adminq,
							   unsigned int lif_index)
{
	union dev_cmd cmd = {
		.adminq_init.opcode = CMD_OPCODE_ADMINQ_INIT,
		.adminq_init.index = adminq->index,
		.adminq_init.pid = adminq->pid,
		//.adminq_init.intr_index = intr_index,
		.adminq_init.lif_index = lif_index,
		.adminq_init.ring_size = ilog2(adminq->num_descs),
		.adminq_init.ring_base = adminq->base_pa,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_txq_init(struct ionic_dev *idev, struct queue *txq,
							unsigned int cos)
{
	union dev_cmd cmd;
	cmd.txq_init.opcode = CMD_OPCODE_TXQ_INIT;
	cmd.txq_init.I = false;
	cmd.txq_init.E = false;
	cmd.txq_init.pid = (u16)txq->pid;
	//cmd.txq_init.intr_index = (u16)cq->bound_intr->index;
	cmd.txq_init.type = TXQ_TYPE_ETHERNET;
	cmd.txq_init.index = txq->index;
	cmd.txq_init.cos = cos;
	cmd.txq_init.ring_base = txq->base_pa;
	cmd.txq_init.ring_size = (u8)ilog2(txq->num_descs);

	DBG("txq_init.pid %d\n", cmd.txq_init.pid);
	DBG("txq_init.index %d\n", cmd.txq_init.index);
	DBG("txq_init.ring_base %llx\n", cmd.txq_init.ring_base);
	DBG("txq_init.ring_size %d\n", cmd.txq_init.ring_size);
	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_rxq_init(struct ionic_dev *idev, struct queue *rxq)
{
	union dev_cmd cmd;
	cmd.rxq_init.opcode = CMD_OPCODE_RXQ_INIT;
	cmd.rxq_init.I = false;
	cmd.rxq_init.E = false;
	cmd.rxq_init.pid = (u16)rxq->pid;
	//cmd.rxq_init.intr_index = (u16)cq->bound_intr->index;
	cmd.rxq_init.type = RXQ_TYPE_ETHERNET;
	cmd.rxq_init.index = rxq->index;
	cmd.rxq_init.ring_base = rxq->base_pa;
	cmd.rxq_init.ring_size = (u8)ilog2(rxq->num_descs);

	DBG("rxq_init.pid %d\n", cmd.rxq_init.pid);
	DBG("rxq_init.index %d\n", cmd.rxq_init.index);
	DBG("rxq_init.ring_base %llx\n", cmd.rxq_init.ring_base);
	DBG("rxq_init.ring_size %d\n", cmd.rxq_init.ring_size);
	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_dev_cmd_station_get(struct ionic_dev *idev)
{
	union dev_cmd cmd = {
		.station_mac_addr_get.opcode = CMD_OPCODE_STATION_MAC_ADDR_GET,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_setmode_adminq_post(struct lif *lif, struct ionic_admin_ctx *ctx)
{
	struct ionic_dev *idev = &lif->ionic->idev;
	union dev_cmd cmd = {
		.rx_mode_set.opcode = ctx->cmd.rx_mode_set.opcode,
		.rx_mode_set.rx_mode = ctx->cmd.rx_mode_set.rx_mode,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_enable_adminq_post(struct lif *lif, struct ionic_admin_ctx *ctx)
{
	struct ionic_dev *idev = &lif->ionic->idev;
	union dev_cmd cmd = {
		.q_enable.opcode = ctx->cmd.q_enable.opcode,
		.q_enable.qid = ctx->cmd.q_enable.qid,
		.q_enable.qtype = ctx->cmd.q_enable.qtype,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_disable_adminq_post(struct lif *lif, struct ionic_admin_ctx *ctx)
{
	struct ionic_dev *idev = &lif->ionic->idev;
	union dev_cmd cmd = {
		.q_disable.opcode = ctx->cmd.q_disable.opcode,
		.q_disable.qid = ctx->cmd.q_disable.qid,
		.q_disable.qtype = ctx->cmd.q_disable.qtype,
	};

	ionic_dev_cmd_go(idev, &cmd);
}

void ionic_rx_filter_adminq_post(struct lif *lif, struct ionic_admin_ctx *ctx)
{
	struct ionic_dev *idev = &lif->ionic->idev;
	union dev_cmd cmd = {
		.rx_filter_add.opcode = ctx->cmd.rx_filter_add.opcode,
		.rx_filter_add.match = ctx->cmd.rx_filter_add.match,
	};

	memcpy(cmd.rx_filter_add.mac.addr, ctx->cmd.rx_filter_add.mac.addr, ETH_ALEN);

	ionic_dev_cmd_go(idev, &cmd);
}
/**
 * Probe helper functions.
 * */
int ionic_dev_setup(struct ionic_dev *idev, struct ionic_device_bar bars[],
					unsigned int num_bars)
{
	struct ionic_device_bar *bar = &bars[0];
	u32 sig;

	// BAR0 resources
	if (num_bars < 1 || bar->len != BAR0_SIZE) {
		DBG("%s :: cannot setup BAR0\n", __FUNCTION__);
		return -EFAULT;
	}

	idev->dev_cmd = bar->virtaddr + BAR0_DEV_CMD_REGS_OFFSET;
	idev->dev_cmd_db = bar->virtaddr + BAR0_DEV_CMD_DB_OFFSET;
	idev->intr_status = bar->virtaddr + BAR0_INTR_STATUS_OFFSET;
	idev->intr_ctrl = bar->virtaddr + BAR0_INTR_CTRL_OFFSET;

	sig = readl(&idev->dev_cmd->signature);
	if (sig != DEV_CMD_SIGNATURE) {
		DBG("%s :: unable to read dev_cmd_signature %x\n", __FUNCTION__, sig);
		return -EFAULT;
	}

	// BAR1 resources
	bar++;
	if (num_bars < 2) {
		DBG("%s :: num bars is less than 2\n", __FUNCTION__);
		return -EFAULT;
	}

	idev->db_pages = bar->virtaddr;
	idev->phy_db_pages = bar->bus_addr;

	// BAR2 resources
	bar++;
	if (num_bars < 3) {
		idev->phy_hbm_pages = 0;
		idev->hbm_npages = 0;
		idev->hbm_inuse = NULL;
		return 0;
	}
	return 0;
}

/**
 * Initialize the ionic and the setup the command registers. 
**/
int ionic_setup(struct ionic *ionic)
{
	int err;

	err = ionic_dev_setup(&ionic->idev, ionic->bars, ionic->num_bars);
	if (err) {
		return err;
	}

	return 0;
}

/**
 * Identify the Ionic device.
 **/
int ionic_identify(struct ionic *ionic)
{
	struct ionic_dev *idev = &ionic->idev;
	union identity *ident;
	dma_addr_t ident_pa;
	int err;

	ident = malloc_dma(sizeof(*ident), IDENTITY_ALIGN);
	if (!ident) {
		return -ENOMEM;
	}

	memset(ident, 0, sizeof(*ident));

	ident_pa = virt_to_phys(ident);
	ident->drv.os_type = OS_TYPE_IPXE;
	ident->drv.os_dist = 0;
	strncpy(ident->drv.driver_ver_str, DRV_VERSION,
			sizeof(ident->drv.driver_ver_str) - 1);

	ionic_dev_cmd_identify(idev, IDENTITY_VERSION_1, ident_pa);

	err = ionic_dev_cmd_wait_check(idev, devcmd_timeout);
	if (err)
		goto err_free_identify;

	ionic->ident = ident;
	ionic->ident_pa = ident_pa;

	return 0;

err_free_identify:
	free_dma(ident, sizeof(*ident));
	return err;
}

/**
 * Initialize the q
 * */
int ionic_q_init(struct lif *lif, struct ionic_dev *idev, struct queue *q,
				 unsigned int index, const char *base, unsigned int num_descs,
				 size_t desc_size, size_t sg_desc_size, unsigned int pid)
{
	struct desc_info *cur;
	unsigned int ring_size;
	unsigned int i;

	if (desc_size == 0 || !is_power_of_2(num_descs)) {
		return -EINVAL;
	}

	ring_size = ilog2(num_descs);
	if (ring_size < 2 || ring_size > 16) {
		return -EINVAL;
	}

	q->lif = lif;
	q->idev = idev;
	q->index = index;
	q->num_descs = num_descs;
	q->desc_size = desc_size;
	q->sg_desc_size = sg_desc_size;
	q->head = q->tail = q->info;
	q->pid = pid;

	snprintf(q->name, sizeof(q->name), "%s%u", base, index);

	cur = q->info;

	for (i = 0; i < num_descs; i++) {
		if (i + 1 == num_descs) {
			cur->next = q->info;
		}
		else {
			cur->next = cur + 1;
		}
		cur->index = i;
		cur->left = num_descs - i;
		cur++;
	}

	return 0;
}

/**
 * Initialize the cq
 * */
int ionic_cq_init(struct lif *lif, struct cq *cq,
				  unsigned int num_descs, size_t desc_size)
{
	struct cq_info *cur;
	unsigned int ring_size;
	unsigned int i;

	if (desc_size == 0 || !is_power_of_2(num_descs)) {
		return -EINVAL;
	}

	ring_size = ilog2(num_descs);
	if (ring_size < 2 || ring_size > 16) {
		return -EINVAL;
	}

	cq->lif = lif;
	cq->num_descs = num_descs;
	cq->desc_size = desc_size;
	cq->tail = cq->info;
	cq->done_color = 1;

	cur = cq->info;

	for (i = 0; i < num_descs; i++) {
		if (i + 1 == num_descs) {
			cur->next = cq->info;
			cur->last = true;
		}
		else {
			cur->next = cur + 1;
		}
		cur->index = i;
		cur++;
	}

	return 0;
}

/**
 * Map the q with the descriptors
 * */
void ionic_q_map(struct queue *q, void *base, dma_addr_t base_pa)
{
	struct desc_info *cur;
	unsigned int i;

	q->base = base;
	q->base_pa = base_pa;

	for (i = 0, cur = q->info; i < q->num_descs; i++, cur++) {
		cur->desc = base + (i * q->desc_size);
	}
}

/**
 * Map the cq with the descriptors
 * */
void ionic_cq_map(struct cq *cq, void *base, dma_addr_t base_pa)
{
	struct cq_info *cur;
	unsigned int i;

	cq->base = base;
	cq->base_pa = base_pa;

	for (i = 0, cur = cq->info; i < cq->num_descs; i++, cur++) {
		cur->cq_desc = base + (i * cq->desc_size);
	}
}

/**
 * Map the q sg.
 * */
void ionic_q_sg_map(struct queue *q, void *base, dma_addr_t base_pa)
{
	struct desc_info *cur;
	unsigned int i;

	q->sg_base = base;
	q->sg_base_pa = base_pa;

	for (i = 0, cur = q->info; i < q->num_descs; i++, cur++) {
		cur->sg_desc = base + (i * q->sg_desc_size);
	}
}

/**
 * Bind the q with cq.
 * */
void ionic_cq_bind(struct cq *cq, struct queue *q)
{
	// TODO support many:1 bindings using qid as index into bound_q array
	cq->bound_q = q;
}

/**
 * Allocating and initializing the qcq.
 * */
static int ionic_qcq_alloc(struct lif *lif, unsigned int index,
						   const char *base, unsigned int flags,
						   unsigned int num_descs, unsigned int desc_size,
						   unsigned int cq_desc_size,
						   unsigned int sg_desc_size,
						   unsigned int pid, struct qcq **qcq)
{
	struct ionic_dev *idev = &lif->ionic->idev;
	struct qcq *new;

	unsigned int q_size = num_descs * desc_size;
	unsigned int cq_size = num_descs * cq_desc_size;
	unsigned int sg_size = num_descs * sg_desc_size;
	unsigned int total_size = ALIGN(q_size, PAGE_SIZE) +
							  ALIGN(cq_size, PAGE_SIZE) +
							  ALIGN(sg_size, PAGE_SIZE);
	void *q_base, *cq_base, *sg_base;
	dma_addr_t q_base_pa, cq_base_pa, sg_base_pa;
	int err;

	*qcq = NULL;

	total_size = ALIGN(q_size, PAGE_SIZE) + ALIGN(cq_size, PAGE_SIZE);
	if (flags & QCQ_F_SG)
		total_size += ALIGN(sg_size, PAGE_SIZE);

	new = zalloc(sizeof(*new));
	if (!new)
		return -ENOMEM;

	new->flags = flags;
	new->q.info = zalloc(sizeof(*new->q.info) * num_descs);
	if (!new->q.info) {
		err = -ENOMEM;
		goto err_free_new_qcq;
	}

	err = ionic_q_init(lif, idev, &new->q, index, base, num_descs,
					   desc_size, sg_desc_size, pid);
	if (err) {
		err = -ENOMEM;
		goto err_free_q_info;
	}

	new->cq.info = zalloc(sizeof(*new->cq.info) * num_descs);
	if (!new->cq.info) {
		err = -ENOMEM;
		goto err_free_q_info;
	}

	err = ionic_cq_init(lif, &new->cq, num_descs, cq_desc_size);
	if (err) {
		err = -ENOMEM;
		goto err_free_cq_info;
	}

	new->base = malloc_dma(total_size, PAGE_SIZE);
	if (!new->base) {
		err = -ENOMEM;
		goto err_free_cq_info;
	}

	new->total_size = total_size;
	new->base_pa = virt_to_phys(new->base);
	q_base = new->base;
	q_base_pa = new->base_pa;

	cq_base = (void *)ALIGN((uintptr_t)q_base + q_size, PAGE_SIZE);
	cq_base_pa = ALIGN(q_base_pa + q_size, PAGE_SIZE);

	if (flags & QCQ_F_SG) {
		sg_base = (void *)ALIGN((uintptr_t)cq_base + cq_size,
								PAGE_SIZE);
		sg_base_pa = ALIGN(cq_base_pa + cq_size, PAGE_SIZE);
		ionic_q_sg_map(&new->q, sg_base, sg_base_pa);
	}

	ionic_q_map(&new->q, q_base, q_base_pa);
	ionic_cq_map(&new->cq, cq_base, cq_base_pa);
	ionic_cq_bind(&new->cq, &new->q);

	*qcq = new;

	return 0;

err_free_cq_info:
	free(new->cq.info);
err_free_q_info:
	free(new->q.info);
err_free_new_qcq:
	free(new);
	return err;
}

/**
 * deallocate the qcq
 * */
void ionic_qcq_dealloc(struct qcq *qcq)
{
	free_dma(qcq->base, qcq->total_size);
	free(qcq->cq.info);
	free(qcq->q.info);
	free(qcq);
}

/**
 * Allocate the adminq, txq and rxq.
 * */
static int ionic_qcqs_alloc(struct lif *lif)
{
	unsigned int flags;
	unsigned int pid;
	int err = -ENOMEM;

	pid = 0;
	flags = QCQ_F_INTR;
	err = ionic_qcq_alloc(lif, 0, "admin", flags, 1 << 4,
						  sizeof(struct admin_cmd),
						  sizeof(struct admin_comp),
						  0, pid, &lif->adminqcq);
	if (err)
		goto err_free_adminqcq;

	flags = QCQ_F_TX_STATS | QCQ_F_INTR | QCQ_F_SG;
	err = ionic_qcq_alloc(lif, 0, "tx", flags, ntxq_descs,
						  sizeof(struct txq_desc),
						  sizeof(struct txq_comp),
						  sizeof(struct txq_sg_desc),
						  pid, &lif->txqcqs);
	if (err)
		goto err_free_txqcq;

	flags = QCQ_F_RX_STATS | QCQ_F_INTR;
	err = ionic_qcq_alloc(lif, 0, "rx", flags, nrxq_descs,
						  sizeof(struct rxq_desc),
						  sizeof(struct rxq_comp),
						  0, pid, &lif->rxqcqs);
	if (err)
		goto err_free_rxqcq;

	return 0;

err_free_rxqcq:
	ionic_qcq_dealloc(lif->txqcqs);
err_free_txqcq:
	ionic_qcq_dealloc(lif->adminqcq);
err_free_adminqcq:
	return err;
}

/**
 * Allocate for the lif and call the qcqs alloc
 * */
int ionic_lif_alloc(struct ionic *ionic, unsigned int index)
{
	struct lif *lif;
	int err = -ENOMEM;

	// allocate memory for lif
	lif = zalloc(sizeof(*lif));
	if (!lif) {
		DBG("%s :: could not allocate for lif\n", __FUNCTION__);
		return err;
	}

	snprintf(lif->name, sizeof(lif->name), "lif");
	lif->ionic = ionic;
	lif->index = index;

	// allocate the qcqs
	err = ionic_qcqs_alloc(lif);
	if (err)
		goto err_freelif;

	ionic->ionic_lif = lif;
	return 0;

err_freelif:
	free(lif);
	return err;
}

/**
 * Get the doorbell from the mapped pages.
 * */
struct doorbell __iomem *ionic_db_map(struct ionic_dev *idev, struct queue *q)
{
	struct doorbell __iomem *db;

	db = (void *)idev->db_pages + (q->pid * PAGE_SIZE);
	db += q->qtype;

	return db;
}

/**
 * Initialize the Adminq
 * */
static int ionic_lif_adminq_init(struct lif *lif)
{
	struct ionic_dev *idev = &lif->ionic->idev;
	struct qcq *qcq = lif->adminqcq;
	struct queue *q = &qcq->q;
	struct adminq_init_comp comp;
	int err;

	ionic_dev_cmd_adminq_init(idev, q, lif->index);
	err = ionic_dev_cmd_wait_check(idev, devcmd_timeout);
	if (err) {
		DBG("%s :: lif adminq initiation failed\n", __FUNCTION__);
		return err;
	}

	ionic_dev_cmd_comp(idev, &comp);
	q->qid = comp.qid;
	q->qtype = comp.qtype;
	q->db = ionic_db_map(idev, q);
	qcq->flags |= QCQ_F_INITED;

	return 0;
}

/**
 * Initialize the TXQ.
 * */
static int ionic_lif_txq_init(struct lif *lif, struct qcq *qcq)
{
	struct queue *q = &qcq->q;
	struct ionic_dev *idev = &lif->ionic->idev;
	struct txq_init_comp comp;
	int err;

	ionic_dev_cmd_txq_init(idev, q, DEFAULT_COS);
	err = ionic_dev_cmd_wait_check(idev, devcmd_timeout);
	if (err) {
		DBG("txq init dev_cmd failed err\n");
		return err;
	}
	ionic_dev_cmd_comp(idev, &comp);

	q->qid = comp.qid;
	q->qtype = comp.qtype;
	q->db = ionic_db_map(idev, q);
	qcq->flags |= QCQ_F_INITED;

	return 0;
}

/**
 * Initialize the RXQ.
 * */
static int ionic_lif_rxq_init(struct lif *lif, struct qcq *qcq)
{
	struct queue *q = &qcq->q;
	struct ionic_dev *idev = &lif->ionic->idev;
	struct rxq_init_comp comp;
	int err;

	ionic_dev_cmd_rxq_init(idev, q);
	err = ionic_dev_cmd_wait_check(idev, devcmd_timeout);
	if (err) {
		DBG("rxq init dev_cmd failed err\n");
		return err;
	}
	ionic_dev_cmd_comp(idev, &comp);

	q->qid = comp.qid;
	q->qtype = comp.qtype;
	q->db = ionic_db_map(idev, q);
	qcq->flags |= QCQ_F_INITED;

	return 0;
}

/**
 * Sets the mac address using the adminq_post
 * TODO:: remove adminq
 * */
static void ionic_lif_add_rx_filter(struct lif *lif, const u8 *addr)
{
	struct ionic_admin_ctx ctx = {
		.cmd.rx_filter_add = {
			.opcode = CMD_OPCODE_RX_FILTER_ADD,
			.match = RX_FILTER_MATCH_MAC,
		},
	};

	memcpy(ctx.cmd.rx_filter_add.mac.addr, addr, ETH_ALEN);
	// call the devcmd.
	ionic_rx_filter_adminq_post(lif, &ctx);
}

/**
 * Gets the mac address using a dev command and 
 * sets the rx_filter for that mac address.
 * */
static int ionic_lif_station_mac_addr(struct lif *lif, struct net_device *netdev)
{
	struct ionic_dev *idev = &lif->ionic->idev;
	struct station_mac_addr_get_comp comp;
	int err;

	ionic_dev_cmd_station_get(idev);
	err = ionic_dev_cmd_wait_check(idev, devcmd_timeout);
	if (err) {
		DBG("%s ::station mac devcmd failed\n", __FUNCTION__);
		return err;
	}
	ionic_dev_cmd_comp(idev, &comp);
	memcpy(netdev->hw_addr, comp.addr, ETH_ALEN);

	//adding the rx filter for the mac
	ionic_lif_add_rx_filter(lif, comp.addr);

	DBG("Station MAC Success %02x:%02x:%02x:%02x:%02x:%02x\n",
		comp.addr[0],
		comp.addr[1],
		comp.addr[2],
		comp.addr[3],
		comp.addr[4],
		comp.addr[5]);

	return 0;
}

/**
 * Initialize the lif
 * 1. Initialize all the queues.
 * 2. Get the MAC address.
 * */
int ionic_lif_init(struct net_device *netdev)
{
	struct ionic *ionic = netdev->priv;
	struct lif *lif = ionic->ionic_lif;
	struct ionic_dev *idev = &lif->ionic->idev;
	int err;

	ionic_dev_cmd_lif_init(idev, lif->index);
	err = ionic_dev_cmd_wait_check(idev, devcmd_timeout);
	if (err) {
		DBG("%s:: lif initiation failed\n", __FUNCTION__);
		return err;
	}

	err = ionic_lif_adminq_init(lif);
	if (err) {
		DBG("%s :: adminq initiation failed\n", __FUNCTION__);
		return err;
	}

	err = ionic_lif_txq_init(lif, lif->txqcqs);
	if (err) {
		DBG("%s ::failed to initialize the txq\n", __FUNCTION__);
		return err;
	}

	err = ionic_lif_rxq_init(lif, lif->rxqcqs);
	if (err) {
		DBG("%s ::failed to initialize the rxq\n", __FUNCTION__);
		return err;
	}

	err = ionic_lif_station_mac_addr(lif, netdev);
	if (err) {
		DBG("%s ::lif station mac addr failed\n", __FUNCTION__);
		return err;
	}
	netdev->max_pkt_len = IONIC_MAX_MTU;
	return 0;
}

/**
 * Sets the rx mode for the lif, using adminq_post
 * */
int ionic_lif_rx_mode(struct lif *lif, unsigned int rx_mode)
{
	struct ionic_admin_ctx ctx = {
		.cmd.rx_mode_set = {
			.opcode = CMD_OPCODE_RX_MODE_SET,
			.rx_mode = rx_mode,
		},
	};
	int err;
	ionic_setmode_adminq_post(lif, &ctx);
	err = ionic_dev_cmd_wait_check(&lif->ionic->idev, devcmd_timeout);
	if (err) {
		DBG("%s :: setting mode failed\n", __FUNCTION__);
		return err;
	}
	return 0;
}

/**
 * Enables the QCQ, using adminq_post
 * */
int ionic_qcq_enable(struct qcq *qcq)
{
	struct queue *q = &qcq->q;
	struct lif *lif = q->lif;
	int err;
	struct ionic_admin_ctx ctx = {
		.cmd.q_enable = {
			.opcode = CMD_OPCODE_Q_ENABLE,
			.qid = q->qid,
			.qtype = q->qtype,
		},
	};

	ionic_enable_adminq_post(lif, &ctx);
	err = ionic_dev_cmd_wait_check(&lif->ionic->idev, devcmd_timeout);
	if (err) {
		DBG("%s :: enable queue failed\n", __FUNCTION__);
		return err;
	}
	return 0;
}

/**
 * Disables the QCQ, using adminq_post
 * */
int ionic_qcq_disable(struct qcq *qcq)
{
	struct queue *q = &qcq->q;
	struct lif *lif = q->lif;
	int err;
	struct ionic_admin_ctx ctx = {
		.cmd.q_disable = {
			.opcode = CMD_OPCODE_Q_DISABLE,
			.qid = q->qid,
			.qtype = q->qtype,
		},
	};

	ionic_disable_adminq_post(lif, &ctx);
	err = ionic_dev_cmd_wait_check(&lif->ionic->idev, devcmd_timeout);
	if (err) {
		DBG("%s :: disable queue failed\n", __FUNCTION__);
		return err;
	}
	return 0;
}

/**
 * Cleans up the rxq of the remaining IOB
 * */
void ionic_rx_flush(struct lif *lif)
{
	unsigned int i;

	for (i = 0; i < NRXQ_DESC; i++) {
		if (lif->rx_iobuf[i])
			free_iob(lif->rx_iobuf[i]);
		lif->rx_iobuf[i] = NULL;
	}
}

/**
 * Returns the available space in the q.
 * */
unsigned int ionic_q_space_avail(struct queue *q)
{
	unsigned int avail = q->tail->index;

	if (q->head->index >= avail)
		avail += q->head->left - 1;
	else
		avail -= q->head->index + 1;

	return avail;
}

/**
 * Checks if the q has space in it.
 * */
bool ionic_q_has_space(struct queue *q, unsigned int want)
{
	return ionic_q_space_avail(q) >= want;
}

/**
 * Allocates and assigns IOB for the rxq.
 * */
void ionic_rx_fill(struct net_device *netdev, int length)
{
	struct ionic *ionic = netdev->priv;
	struct queue *rxq = &ionic->ionic_lif->rxqcqs->q;
	struct rxq_desc *desc;
	struct io_buffer *iobuf;
	unsigned int i;

	for (i = ionic_q_space_avail(rxq); i; i--) {

		// Allocate I/O buffer
		iobuf = alloc_iob(length);
		if (!iobuf) {
			DBG("%s :: failed to allocate buffer for index %d \n", __FUNCTION__, ionic_q_space_avail(rxq));
			return;
		}
		desc = rxq->head->desc;
		desc->addr = virt_to_bus(iobuf->data);
		desc->len = length;
		desc->opcode = RXQ_DESC_OPCODE_SIMPLE;
		rxq->lif->rx_iobuf[rxq->head->index] = iobuf;
		rxq->head = rxq->head->next;

		// only ring doorbell every stride.
		if (((rxq->head->index + 1) & RX_RING_DOORBELL_STRIDE) == 0) {
			struct doorbell db = {
				.qid_lo = rxq->qid,
				.qid_hi = rxq->qid >> 8,
				.ring = 0,
				.p_index = rxq->head->index,
			};
			writeq(*(u64 *)&db, rxq->db);
		}
	}
}

/**
 * Poll for received packets
 *
 * @v netdev				Network device
 */
void ionic_poll_rx(struct net_device *netdev)
{
	struct ionic *ionic = netdev->priv;

	struct queue *rxq = &ionic->ionic_lif->rxqcqs->q;
	struct cq *rxcq = &ionic->ionic_lif->rxqcqs->cq;
	struct rxq_comp *comp = rxcq->info->cq_desc;
	struct rxq_desc *desc;
	struct io_buffer *iobuf;
	size_t len;

	while (comp->color == rxcq->done_color) {

		// update the cq_info and check if the last descriptor.
		if (rxcq->info->last) {
			rxcq->done_color = !rxcq->done_color;
		}
		rxcq->info = rxcq->info->next;

		// Populate I/O buffer
		desc = rxq->tail->desc;
		iobuf = ionic->ionic_lif->rx_iobuf[rxq->tail->index];
		ionic->ionic_lif->rx_iobuf[rxq->tail->index] = NULL;
		len = desc->len;
		iob_put(iobuf, len);

		netdev_rx(netdev, iobuf);

		// update the q tail index;
		rxq->tail = rxq->tail->next;

		// update the comp struct
		comp = rxcq->info->cq_desc;
	}
}

/**
 * Poll for transmitted packets
 *
 * @v netdev				Network device
 */
void ionic_poll_tx(struct net_device *netdev)
{
	struct ionic *ionic = netdev->priv;
	struct queue *txq = &ionic->ionic_lif->txqcqs->q;
	struct cq *txcq = &ionic->ionic_lif->txqcqs->cq;
	struct txq_comp *comp = txcq->info->cq_desc;

	while (comp->color == txcq->done_color) {

		// update the cq_info and check if the last descriptor.
		if (txcq->info->last) {
			txcq->done_color = !txcq->done_color;
		}
		txcq->info = txcq->info->next;

		// update the q tail index;
		txq->tail = txq->tail->next;

		// update the comp struct
		comp = txcq->info->cq_desc;

		// Complete transmit
		netdev_tx_complete_next(netdev);
	}
}
