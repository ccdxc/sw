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

/*
 * ionic_main.c --
 *
 * Implement native ionic_en driver
 */

#include "ionic.h"

struct ionic_driver ionic_driver;

unsigned int ntxq_descs = 4096;
unsigned int nrxq_descs = 4096;
unsigned int ntxqs = 32;
unsigned int nrxqs = 32;
unsigned int devcmd_timeout = 30;

VMK_MODPARAM(ntxq_descs,
             uint,
             "Descriptors per Tx queue, must be power of 2");
VMK_MODPARAM(nrxq_descs,
             uint,
             "Descriptors per Rx queue, must be power of 2");
VMK_MODPARAM(ntxqs,
             uint,
             "Hard set the number of Tx queues per LIF");
VMK_MODPARAM(nrxqs,
             uint,
             "Hard set the number of Rx queues per LIF");

VMK_MODPARAM(devcmd_timeout,
             uint,
             "Devcmd timeout in seconds (default 30 secs)");

#ifdef FAKE_ADMINQ
unsigned int use_AQ = 0;

VMK_MODPARAM(use_AQ,
             uint,
             "Set to non-0 to enable AQ testing, defaults to 1");
#endif




VMK_ReturnStatus
ionic_adminq_check_err(struct lif *lif,
                       struct ionic_admin_ctx *ctx,
                       vmk_Bool is_timeout)
{
//	struct net_device *netdev = lif->netdev;
	static struct cmds {
		unsigned int cmd;
		char *name;
	} cmds[] = {
		{ CMD_OPCODE_TXQ_INIT, "CMD_OPCODE_TXQ_INIT" },
		{ CMD_OPCODE_RXQ_INIT, "CMD_OPCODE_RXQ_INIT" },
		{ CMD_OPCODE_FEATURES, "CMD_OPCODE_FEATURES" },
		{ CMD_OPCODE_Q_ENABLE, "CMD_OPCODE_Q_ENABLE" },
		{ CMD_OPCODE_Q_DISABLE, "CMD_OPCODE_Q_DISABLE" },
		{ CMD_OPCODE_STATION_MAC_ADDR_GET,
			"CMD_OPCODE_STATION_MAC_ADDR_GET" },
		{ CMD_OPCODE_MTU_SET, "CMD_OPCODE_MTU_SET" },
		{ CMD_OPCODE_RX_MODE_SET, "CMD_OPCODE_RX_MODE_SET" },
		{ CMD_OPCODE_RX_FILTER_ADD, "CMD_OPCODE_RX_FILTER_ADD" },
		{ CMD_OPCODE_RX_FILTER_DEL, "CMD_OPCODE_RX_FILTER_DEL" },
		{ CMD_OPCODE_STATS_DUMP_START, "CMD_OPCODE_STATS_DUMP_START" },
		{ CMD_OPCODE_STATS_DUMP_STOP, "CMD_OPCODE_STATS_DUMP_STOP" },
		{ CMD_OPCODE_RSS_HASH_SET, "CMD_OPCODE_RSS_HASH_SET" },
		{ CMD_OPCODE_RSS_INDIR_SET, "CMD_OPCODE_RSS_INDIR_SET" },
	};
        int list_len = ARRAY_SIZE(cmds);
        struct cmds *cmd = cmds;
	char *name = "UNKNOWN";
        int i;

	if (ctx->comp.comp.status || is_timeout) {
                for (i = 0; i < list_len; i++) {
                        if (cmd[i].cmd == ctx->cmd.cmd.opcode) {
                                name = cmd[i].name;
                                break;
                        }
                }
		ionic_err("(%d) %s failed: %d %s\n", ctx->cmd.cmd.opcode,
			  name, ctx->comp.comp.status,
                          (is_timeout ? "(timeout)" : ""));
		return VMK_FAILURE;
	}

//	return 0;
	return VMK_OK;
}

VMK_ReturnStatus
ionic_adminq_post_wait(struct lif *lif, struct ionic_admin_ctx *ctx)
{
	VMK_ReturnStatus status;
        vmk_Bool is_timeout;

	status = ionic_api_adminq_post(lif, ctx);
	if (status != VMK_OK) {
		ionic_err("ionic_api_adminq_post() failed, status: %s",
			  vmk_StatusToString(status));
		return status;
	}
//		return err;

	is_timeout = ionic_wait_for_completion(&ctx->work,
                                               devcmd_timeout *
                                               VMK_MSEC_PER_SEC);
//	wait_for_completion(&ctx->work);

	return ionic_adminq_check_err(lif,
                                      ctx,
                                      is_timeout);
}

int ionic_netpoll(int budget, ionic_cq_cb cb, 
                  void *cb_arg)
{
        struct qcq *qcq = (struct qcq *) cb_arg;
        struct cq *cq = &qcq->cq;
        unsigned int work_done;

        work_done = ionic_cq_service(cq, budget, cb, cb_arg);

        if (work_done > 0)
                ionic_intr_return_credits(cq->bound_intr,
                                          work_done,
                                          VMK_FALSE,
                                          VMK_TRUE);

        if (work_done < budget) {
                ionic_intr_mask(cq->bound_intr, VMK_FALSE);
        }

        return work_done;
}


static VMK_ReturnStatus
ionic_dev_cmd_check_error(struct ionic_dev *idev)
{
        VMK_ReturnStatus status;

        status = ionic_dev_cmd_status(idev);
        switch (status) {
        case 0:
                return VMK_OK;
        }

        return VMK_FAILURE;
}

// TODO: We don't need to convert max_wait at the first place, check its caller
static VMK_ReturnStatus
ionic_dev_cmd_wait(struct ionic_dev *idev, unsigned long max_wait)
{
        VMK_ReturnStatus status;
        unsigned long time;
//        signed long wait;
        int done;

//        WARN_ON(in_interrupt());

        /* Wait for dev cmd to complete...but no more than max_wait
         */

//        time = jiffies + max_wait;
        time = vmk_GetTimerCycles() + max_wait;
        do {

                done = ionic_dev_cmd_done(idev);
#ifdef HAPS
                if (done) {
                        ionic_dbg("DEVCMD %d done took %ld secs (%ld jiffies)\n",
                                  idev->dev_cmd->cmd.cmd.opcode,
                                  (vmk_GetTimerCycles() + max_wait - time) / HZ,
                                  vmk_GetTimerCycles() + max_wait - time);
                }
#endif
                if (done) {
                        return VMK_OK;
                }

                status = vmk_WorldSleep(100);
                if (status != VMK_OK) {
                        ionic_err("vmk_WorldSleep() returns: %s",
                                  vmk_StatusToString(status));
                        return status;
                }

        } while (IONIC_TIME_AFTER(time, vmk_GetTimerCycles()));

#ifdef HAPS
        ionic_err("DEVCMD timeout after %ld secs\n", max_wait / HZ);
#endif
        return VMK_TIMEOUT;
}



VMK_ReturnStatus
ionic_dev_cmd_wait_check(struct ionic_dev *idev, unsigned long max_wait)
{
        VMK_ReturnStatus status;
//        int err;

        status = ionic_dev_cmd_wait(idev, max_wait);
        if (status != VMK_OK) {
                ionic_err("ionic_dev_cmd_wait() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        return ionic_dev_cmd_check_error(idev);
}



#ifdef FAKE_ADMINQ
#define XXX_DEVCMD_HALF_PAGE 0x800

// XXX temp func to get side-band data from 2nd half page of dev_cmd reg space.
static VMK_ReturnStatus
SBD_get(struct ionic_dev *idev, void *dst, size_t len)
{
        u32 __iomem *page32 = (void __iomem *)idev->dev_cmd;
        u32 *dst32 = dst;
        unsigned int i, count;

        // check pointer and size alignment
        if ((unsigned long)dst & 0x3 || len & 0x3)
                return VMK_FAILURE;

        // check length fits in 2nd half of page
        if (len > XXX_DEVCMD_HALF_PAGE)
                return VMK_FAILURE;

        page32 += XXX_DEVCMD_HALF_PAGE / sizeof(*page32);
        count = len / sizeof(*page32);

        for (i = 0; i < count; ++i)
//                dst32[i] = ioread32(&page32[i]);
                dst32[i] = ionic_readl_raw((vmk_VA)&page32[i]);
        return VMK_OK;
}

// XXX temp func to put side-band data into 2nd half page of dev_cmd reg space.
static VMK_ReturnStatus
SBD_put(struct ionic_dev *idev, void *src, size_t len)
{
        u32 __iomem *page32 = (void __iomem *)idev->dev_cmd;
        u32 *src32 = src;
        unsigned int i, count;

        // check pointer and size alignment
        if ((unsigned long)src & 0x3 || len & 0x3)
                return VMK_FAILURE;

        // check length fits in 2nd half of page
        if (len > XXX_DEVCMD_HALF_PAGE)
                return VMK_FAILURE;

        page32 += XXX_DEVCMD_HALF_PAGE / sizeof(*page32);
        count = len / sizeof(*page32);

        for (i = 0; i < count; ++i)
                ionic_writel_raw(src32[i],
                                 (vmk_VA)&page32[i]);
//                iowrite32(src32[i], &page32[i]);

        return VMK_OK;
}


//static void ionic_dev_cmd_work(struct work_struct *work)
static void
ionic_dev_cmd_work(vmk_AddrCookie data)
{
        VMK_ReturnStatus status;
        struct ionic_work *work = data.ptr;
        struct ionic *ionic = IONIC_CONTAINER_OF(work,
                                                 struct ionic,
                                                 cmd_work);
        struct ionic_admin_ctx *ctx;
//        unsigned long irqflags;
//        int err = 0;

//        spin_lock_irqsave(&ionic->cmd_lock, irqflags);
        vmk_SpinlockLock(ionic->cmd_lock);
        if (vmk_ListIsEmpty(&ionic->cmd_list)) {
//                spin_unlock_irqrestore(&ionic->cmd_lock, irqflags);
                vmk_SpinlockUnlock(ionic->cmd_lock);
                return;
        }

//        ctx = list_first_entry(&ionic->cmd_list,
//                               struct ionic_admin_ctx, list);
        ctx = VMK_LIST_ENTRY(vmk_ListFirst(&ionic->cmd_list),
                             struct ionic_admin_ctx,
                             list);
        vmk_ListRemove(&ctx->list);
//        spin_unlock_irqrestore(&ionic->cmd_lock, irqflags);
        vmk_SpinlockUnlock(ionic->cmd_lock);

//        dev_dbg(ionic->dev, "post admin dev command:\n");
        ionic_dbg("post admin dev command:\n");
//        print_hex_dump_debug("cmd ", DUMP_PREFIX_OFFSET, 16, 1,
//                             &ctx->cmd, sizeof(ctx->cmd), true);

        if (ctx->side_data) {
//                dynamic_hex_dump("data ", DUMP_PREFIX_OFFSET, 16, 1,
//                                 ctx->side_data, ctx->side_data_len, true);

                status = SBD_put(&ionic->en_dev.idev,
                                 ctx->side_data,
                                 ctx->side_data_len);
                if (status != VMK_OK)
                        goto err_out;
        }

        ionic_dev_cmd_go(&ionic->en_dev.idev, (void *)&ctx->cmd);

        status = ionic_dev_cmd_wait_check(&ionic->en_dev.idev,
                                          HZ * devcmd_timeout);
        if (status != VMK_OK) {
                ionic_err("ionic_dev_cmd_wait_check() failed, status: %s",
                          vmk_StatusToString(status));
                goto err_out;
        }

        ionic_dev_cmd_comp(&ionic->en_dev.idev, &ctx->comp);

        if (ctx->side_data) {
                status = SBD_get(&ionic->en_dev.idev, ctx->side_data, ctx->side_data_len);
                if (status != VMK_OK)
                        goto err_out;
        }

//        dev_dbg(ionic->dev, "comp admin dev command:\n");
        ionic_dbg("comp admin dev command:\n");
//        print_hex_dump_debug("comp ", DUMP_PREFIX_OFFSET, 16, 1,
//                             &ctx->comp, sizeof(ctx->comp), true);

err_out:
        if (IONIC_WARN_ON(status))
                vmk_Memset(&ctx->comp, 0xAB, sizeof(ctx->comp));

        //complete_all(&ctx->work);
        ionic_complete(&ctx->work);

        //schedule_work(&ionic->cmd_work);
        status = ionic_work_queue_submit(ionic->cmd_work_queue,
                                         &ionic->cmd_work,
                                         0);
        if (status != VMK_OK) {
                ionic_err("ionic_work_queue_submit() failed, status: %s",
                          vmk_StatusToString(status));
        }
}
#endif





static VMK_ReturnStatus
ionic_setup(struct ionic *ionic)
{
        VMK_ReturnStatus status;

	ionic_info("ionic_setup() called");

        status = ionic_dev_setup(&ionic->en_dev.idev,
                                 ionic->bars,
                                 ionic->num_bars);
        if (status != VMK_OK) {
		ionic_err("ionic_dev_setup() failed, status: %s",
			  vmk_StatusToString(status));
                return status;
        }

        status = ionic_spinlock_create("ionic->lifs_lock",
                                       ionic_driver.module_id,
                                       ionic_driver.heap_id,
                                       ionic_driver.lock_domain,
                                       VMK_SPINLOCK,
                                       IONIC_LOCK_RANK_NORMAL,
                                       &ionic->lifs_lock);
        if (status != VMK_OK) {
                ionic_err("ionic_spinlock_create() failed, status: %s",
                          vmk_StatusToString(status));
                goto lifs_lock_err;
        }                

#ifdef FAKE_ADMINQ
//        spin_lock_init(&ionic->cmd_lock);
//        INIT_LIST_HEAD(&ionic->cmd_list);
//        INIT_WORK(&ionic->cmd_work, ionic_dev_cmd_work);
        status = ionic_spinlock_create("ionic->cmd_lock",
                                       ionic_driver.module_id,
                                       ionic_driver.heap_id,
                                       ionic_driver.lock_domain,
                                       VMK_SPINLOCK,
                                       IONIC_LOCK_RANK_NORMAL,
                                       &ionic->cmd_lock);
        if (status != VMK_OK) {
                ionic_err("ionic_spinlock_create() failed, status: %s",
                          vmk_StatusToString(status));
                goto cmd_lock_err;
        }                

        vmk_ListInit(&ionic->cmd_list);

        ionic->cmd_work_queue = ionic_work_queue_create(ionic_driver.heap_id,
                                                        ionic_driver.module_id,
                                                        "cmd_work_queue");
        if (ionic->cmd_work_queue == NULL) {
                ionic_err("ionic_work_queue_create() failed, status: %s",
                          vmk_StatusToString(status));
                status = VMK_NO_MEMORY;
                goto wq_create_err;
        }

        ionic_work_init(&ionic->cmd_work,
                        ionic_dev_cmd_work,
                        &ionic->cmd_work);

        return status;

wq_create_err:
        ionic_spinlock_destroy(ionic->cmd_lock);

cmd_lock_err:
        ionic_spinlock_destroy(ionic->lifs_lock);

#else
        return status;
#endif

lifs_lock_err:
        ionic_dev_clean(ionic);

        return status;
}


static inline void
ionic_clean(struct ionic *ionic)
{
#ifndef ADMINQ
        ionic_work_queue_destroy(ionic->cmd_work_queue);
        ionic_spinlock_destroy(ionic->cmd_lock);
#endif
        ionic_spinlock_destroy(ionic->lifs_lock);
        ionic_dev_clean(ionic);
}



VMK_ReturnStatus
ionic_reset(struct ionic *ionic)
{
        struct ionic_dev *idev = &ionic->en_dev.idev;

	ionic_info("ionic_reset() called");

        ionic_dev_cmd_reset(idev);

        return ionic_dev_cmd_wait_check(idev,
                                        HZ * devcmd_timeout);
}


VMK_ReturnStatus
ionic_identify(struct ionic *ionic)
{
//        struct device *dev = ionic->dev;
        VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;
        struct ionic_dev *idev = &ionic->en_dev.idev;
        union identity *ident;
        //dma_addr_t ident_pa;
        vmk_IOA ident_pa;
//        int err;
#ifdef HAPS
        unsigned int i;
#endif

/*
        ident = devm_kzalloc(dev, sizeof(*ident), GFP_KERNEL | GFP_DMA);
        if (!ident)
                return -ENOMEM;
        ident_pa = dma_map_single(dev, ident, sizeof(*ident),
                                  DMA_BIDIRECTIONAL);
        if (dma_mapping_error(dev, ident_pa))
                return -EIO;
*/
	ionic_info("ionic_identify() called");

        priv_data = IONIC_CONTAINER_OF(ionic,
                                       struct ionic_en_priv_data,
                                       ionic);

        ident = ionic_dma_zalloc_align(ionic_driver.heap_id,
                                       priv_data->dma_engine_streaming,
                                       sizeof(union identity),
                                       VMK_PAGE_SIZE,
                                       &ident_pa);
        if (VMK_UNLIKELY(ident == NULL)) {
                ionic_err("ionic_dma_alloc_align() failed, status: NO MEMORY");
                return VMK_NO_MEMORY;
        }

        ident->drv.os_type = OS_TYPE_ESXI;
        ident->drv.os_dist = 0;

#if 0
        //TODO: we need to go back here later to finalize this part
        vmk_Strncpy(ident->drv.os_dist_str, utsname()->release,
                    sizeof(ident->drv.os_dist_str) - 1);

        ident->drv.kernel_ver = LINUX_VERSION_CODE;
        vmk_Strncpy(ident->drv.kernel_ver_str, utsname()->version,
                    sizeof(ident->drv.kernel_ver_str) - 1);
        vmk_Strncpy(ident->drv.driver_ver_str, DRV_VERSION,
                    sizeof(ident->drv.driver_ver_str) - 1);
#endif 

#ifdef HAPS
        for (i = 0; i < 512; i++)
//                iowrite32(idev->ident->words[i], &ident->words[i]);
                ionic_writel_raw(idev->ident->words[i],
                                 (vmk_VA)&ident->words[i]);
#endif

        ionic_dev_cmd_identify(idev, IDENTITY_VERSION_1, ident_pa);

        status = ionic_dev_cmd_wait_check(idev, HZ * devcmd_timeout);
        if (status != VMK_OK) {
                ionic_err("ionic_dev_cmd_wait_check() failed, status: %s",
                          vmk_StatusToString(status));
                goto dev_cmd_wait_err;
        };

        vmk_WorldSleep(1000);

#ifdef HAPS
        for (i = 0; i < 512; i++)
//                ident->words[i] = ioread32(&idev->ident->words[i]);
                ident->words[i] = ionic_readl_raw((vmk_VA)&idev->ident->words[i]);
#endif

        ionic->ident = ident;
        ionic->ident_pa = ident_pa;

//        err = ionic_debugfs_add_ident(ionic);
//        if (err)
//                goto err_out_unmap;
	ionic_info("ionic_identify() completed successfully!");

        return status;

dev_cmd_wait_err:
        ionic_dma_free(ionic_driver.heap_id,
                       priv_data->dma_engine_streaming,
                       sizeof(union identity),
                       ident,
                       ident_pa);              

//err_out_unmap:
//        dma_unmap_single(dev, ident_pa, sizeof(*ident), DMA_BIDIRECTIONAL);
        return status;
}


/*
 *****************************************************************************
 *
 * ionic_en_get_sbdf
 *
 *     Get PCI device address 
 *
 *  Parameters:
 *     data       - IN (pointer to ionic_en_priv_data)
 *
 *  Results:
 *     vmk_PCIDeviceAddr accosiated with priv_data
 *
 *  Side effects:
 *     None
 *
 *****************************************************************************
 */

static
vmk_PCIDeviceAddr ionic_en_get_sbdf(vmk_AddrCookie data)
{
        struct ionic_en_priv_data *priv_data = (struct ionic_en_priv_data *) data.ptr;

        VMK_ASSERT(priv_data);

        return priv_data->ionic.en_dev.sbdf;
}


/*
 ******************************************************************************
 *
 * ionic_en_attach --
 *
 *     Driver callback that attaches ionic device to the driver
 *
 *  Parameters:
 *     device - IN (vmk_Device handle that generated by vmkernel device layer)
 *
 *  Results:
 *     Return VMK_OK on success
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

static VMK_ReturnStatus
ionic_en_attach(vmk_Device device)                                // IN
{
        VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;

	ionic_info("ionic_en_attach() called");

        priv_data = ionic_heap_zalign(ionic_driver.heap_id,
                                      sizeof(struct ionic_en_priv_data),
                                      VMK_L1_CACHELINE_SIZE);
        if (VMK_UNLIKELY(!priv_data)) {
                ionic_err("ionic_heap_zalign() failed, status: VMK_NO_MEMORY");
                return VMK_NO_MEMORY;
        }

        priv_data->ionic.en_dev.vmk_device     = device;
        priv_data->module_id                   = ionic_driver.module_id;
        priv_data->heap_id                     = ionic_driver.heap_id;
        priv_data->lock_domain                 = ionic_driver.lock_domain;
        priv_data->mem_pool                    = ionic_driver.mem_pool;

        status = vmk_DeviceSetAttachedDriverData(device, priv_data);
        if (status != VMK_OK) {
                ionic_err("vmk_DeviceSetAttachedDriverData() failed, status: %s",
                          vmk_StatusToString(status));
                goto set_attached_err;
        }

        status = ionic_pci_query(priv_data);
        if (status != VMK_OK) {
                ionic_err("ionic_pci_query() failed, status: %s",
                          vmk_StatusToString(status));
                goto set_attached_err;
        }

        status = ionic_pci_start(priv_data);
        if (status != VMK_OK) {
                ionic_err("ionic_pci_start() failed, status: %s",
                          vmk_StatusToString(status));
                goto set_attached_err;
        }

        status = ionic_dma_engine_create("dma_engine_streaming",
                                         ionic_driver.module_id,
                                         device,
                                         VMK_FALSE, /* Not coherent */
                                         IONIC_DMA_MAX_SEGMENT,
                                         0,
                                         &priv_data->dma_engine_streaming);
        if (status != VMK_OK) {
                ionic_err("ionic_dma_engine_create() failed, status: %s",
                          vmk_StatusToString(status));
                goto dma_streaming_err;;
        }

        status = ionic_dma_engine_create("dma_engine_coherent",
                                         ionic_driver.module_id,
                                         device,
                                         VMK_TRUE, /* coherent */
                                         IONIC_DMA_MAX_SEGMENT,
                                         0,
                                         &priv_data->dma_engine_coherent);
        if (status != VMK_OK) {
                ionic_err("ionic_dma_engine_create() failed, status: %s",
                          vmk_StatusToString(status));
                goto dma_coherent_err;
        }
        
        status = ionic_setup(&priv_data->ionic);
        if (status != VMK_OK) {
                ionic_err("ionic_setup() failed, status: %s",
                          vmk_StatusToString(status));
                goto setup_err;
        }

        status = ionic_reset(&priv_data->ionic);
        if (status != VMK_OK) {
                ionic_err("ionic_reset() failed, status: %s",
                          vmk_StatusToString(status));
                goto reset_err;
        }

	status = ionic_identify(&priv_data->ionic);
        if (status != VMK_OK) {
                ionic_err("ionic_identify() failed, status: %s",
                          vmk_StatusToString(status));
        } else {
	        ionic_info("ASIC: %s rev: 0x%X serial num: %s fw version: %s\n",
		           ionic_dev_asic_name(priv_data->ionic.ident->dev.asic_type),
        	           priv_data->ionic.ident->dev.asic_rev,
	                   priv_data->ionic.ident->dev.serial_num,
		           priv_data->ionic.ident->dev.fw_version);
        	ionic_info("ionic_en_attach() completed successfully!");
        }

        return status;

reset_err:
        ionic_clean(&priv_data->ionic);

setup_err:
        ionic_dma_engine_destroy(priv_data->dma_engine_coherent);

dma_coherent_err:
        ionic_dma_engine_destroy(priv_data->dma_engine_streaming);

dma_streaming_err:
        ionic_pci_stop(priv_data);

set_attached_err:
        ionic_heap_free(ionic_driver.heap_id, priv_data);

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_en_scan --
 *
 *     Driver callback that scan ionic device and create logical devices
 *
 *  Parameters:
 *     device - IN (vmk_Device handle)
 *
 *  Results:
 *     Return VMK_OK on success
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

static VMK_ReturnStatus
ionic_en_scan(vmk_Device device)                                  // IN
{
        VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;

	ionic_info("ionic_en_scan() called");

        status = vmk_DeviceGetAttachedDriverData(device, 
                                                 (vmk_AddrCookie *) &priv_data);
        if (status != VMK_OK) {
                ionic_err("vmk_DeviceGetAttachedDriverData() failed, "
                          "status: %s", vmk_StatusToString(status));
                return status;
        }

        status = ionic_lifs_size(&priv_data->ionic);
	if (status != VMK_OK) {
		ionic_err("ionic_lifs_size() failed, status: %s",
			  vmk_StatusToString(status));
                return status;
        }
        priv_data->is_lifs_size_compl = VMK_TRUE;

        status = ionic_en_uplink_init(priv_data);
        if (status != VMK_OK) {
                ionic_err("ionic_en_uplink_init() failed, status: %s",
                          vmk_StatusToString(status));
                goto uplink_init_err;
        }

        status = ionic_lifs_alloc(&priv_data->ionic);
        if (status != VMK_OK) {
                ionic_err("ionic_lifs_alloc() failed, status: %s",
                          vmk_StatusToString(status));
                goto lifs_alloc_err;
        }

        status = ionic_lifs_init(&priv_data->ionic);
        if (status != VMK_OK) {
                ionic_err("ionic_lifs_init() failed, status: %s",
                          vmk_StatusToString(status));
                goto lifs_init_err;
        }

        status = ionic_logical_dev_register(ionic_driver.drv_handle,
                                            device,
                                            priv_data,
                                            &priv_data->uplink_handle.uplink_reg_data,
                                            &priv_data->ionic.en_dev.uplink_vmk_dev);
        if (status != VMK_OK) {
                ionic_err("ionic_logical_dev_register() failed, status: %s",
                          vmk_StatusToString(status));
                goto logical_dev_err;
        }

        return status;

logical_dev_err:
        ionic_lifs_deinit(&priv_data->ionic);

lifs_init_err:
        ionic_lifs_free(&priv_data->ionic);

lifs_alloc_err:
        ionic_en_uplink_cleanup(priv_data); 

uplink_init_err:
        ionic_lifs_size_undo(priv_data);

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_en_detach --
 *
 *     Driver callback that detach ionic device from the driver
 *
 *  Parameters:
 *     device - IN (vmk_Device handle)
 *
 *  Results:
 *     Return VMK_OK on success
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

static VMK_ReturnStatus
ionic_en_detach(vmk_Device device)                                // IN
{
	VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;

	ionic_info("ionic_en_detach() called");

	status = vmk_DeviceGetAttachedDriverData(device,
						 (vmk_AddrCookie *) &priv_data);
	if (status != VMK_OK) {
		ionic_err("vmk_DeviceGetAttachedDriverData() failed, status: %s",
			  vmk_StatusToString(status));
                return status;
	}

        ionic_dma_free(ionic_driver.heap_id,
                       priv_data->dma_engine_streaming,
                       sizeof(union identity),
                       priv_data->ionic.ident,
                       priv_data->ionic.ident_pa);              

        ionic_dma_engine_destroy(priv_data->dma_engine_streaming);
        ionic_dma_engine_destroy(priv_data->dma_engine_coherent);

	ionic_clean(&priv_data->ionic);
        if (priv_data->is_lifs_size_compl) {
                ionic_en_uplink_cleanup(priv_data);
        }
	ionic_pci_stop(priv_data);
	ionic_heap_free(ionic_driver.heap_id, priv_data);

	return status;
}


/*
 ******************************************************************************
 *
 * ionic_en_quiesce --
 *
 *     Driver callback that quiesces ionic device
 *
 *  Parameters:
 *     device - IN (vmk_Device handle)
 *
 *  Results:
 *     Return VMK_OK on success
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

static VMK_ReturnStatus
ionic_en_quiesce(vmk_Device device)                               // IN
{
        VMK_ReturnStatus status;
        struct ionic_en_priv_data *priv_data;

	ionic_info("ionic_en_quiesce() called");

        status = vmk_DeviceGetAttachedDriverData(device, 
                                                 (vmk_AddrCookie *) &priv_data);
        if (status != VMK_OK) {
                ionic_err("vmk_DeviceGetAttachedDriverData() failed, "
                          "status: %s", vmk_StatusToString(status));
                return status;
        }

        if (priv_data->is_lifs_size_compl) {
                ionic_lifs_deinit(&priv_data->ionic);
                ionic_lifs_free(&priv_data->ionic);
                ionic_lifs_size_undo(priv_data);
        }

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_en_start --
 *
 *     Driver callback that starts ionic device
 *
 *  Parameters:
 *     device - IN (vmk_Device handle)
 *
 *  Results:
 *     Return VMK_OK on success
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

static VMK_ReturnStatus
ionic_en_start(vmk_Device device)                                 // IN
{
	ionic_info("ionic_en_start() called");
	return VMK_OK;
}


/*
 ******************************************************************************
 *
 * ionic_en_forget --
 *
 *     Driver callback that forgets ionic device
 *
 *  Parameters:
 *     device - IN (vmk_Device handle)
 *
 *  Results:
 *     Return VMK_OK on success
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

static void
ionic_en_forget(vmk_Device device)                                // IN
{
	ionic_info("ionic_en_forget() called");
}


/* Driver callbacks that required by device layer */
static vmk_DriverOps ionic_driver_ops = {
        .attachDevice 	= ionic_en_attach,
        .scanDevice   	= ionic_en_scan,
	.detachDevice   = ionic_en_detach,
	.quiesceDevice  = ionic_en_quiesce,
	.startDevice	= ionic_en_start,
	.forgetDevice	= ionic_en_forget,
};


/*
 ******************************************************************************
 *
 * init_module --
 *
 *     Driver starting point
 *
 *  Parameters:
 *     None
 *
 *  Results:
 *     Initialize resources for this driver
 *     Return 0 when completes successfully
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

int
init_module(void)
{
        VMK_ReturnStatus status;
        vmk_DriverProps drv_props;

        ionic_info("%s: init module...", IONIC_DRV_NAME);

        status = vmk_NameInitialize(&ionic_driver.name,
                                    IONIC_DRV_NAME);
        if (VMK_UNLIKELY(status != VMK_OK)) {
                ionic_err("%s, vmk_NameInitialize() failed, status: %s",
                          IONIC_DRV_NAME, vmk_StatusToString(status));
                return status;
        }

        ionic_driver.module_id = vmk_ModuleCurrentID;

        /*TODO: We need to add a function to validate module parameters */


        /* Create a memory pool */
        status = ionic_mem_pool_init(IONIC_DRV_NAME,
                                     ionic_driver.module_id,
                                     VMK_MEMPOOL_INVALID,
                                     VMK_MEM_POOL_LEAF,
                                     MEMPOOL_INIT_SIZE,
                                     MEMPOOL_MAX_SIZE,
                                     &ionic_driver.mem_pool);
        if (status != VMK_OK) {
                ionic_err("%s: ionic_mem_pool_init failed, status: %s",
                          IONIC_DRV_NAME, vmk_StatusToString(status));
                return status;
        }

        /* Create heap that back the created memory pool */
        status = ionic_heap_init(IONIC_DRV_NAME,
                                 ionic_driver.module_id,
                                 ionic_driver.mem_pool,
                                 VMK_HEAP_TYPE_MEMPOOL,
                                 HEAP_INIT_SIZE,
                                 HEAP_MAX_SIZE,
                                 VMK_MEM_PHYS_CONTIGUOUS,
                                 &ionic_driver.heap_id);
        if (status != VMK_OK) {
                ionic_err("%s: ionic_HeapInit() failed, status: %s",
                          IONIC_DRV_NAME, vmk_StatusToString(status));
                goto heap_init_err;
        }

        vmk_ModuleSetHeapID(ionic_driver.module_id, ionic_driver.heap_id);

        /* Create a lock domain */
        status = vmk_LockDomainCreate(ionic_driver.module_id,
                                      ionic_driver.heap_id,
                                      &ionic_driver.name,
                                      &ionic_driver.lock_domain);
        if (status != VMK_OK) {
                ionic_err("%s: vmk_LockDomainCreate() failed, status: %s",
                          IONIC_DRV_NAME, vmk_StatusToString(status));
                goto lock_domain_err;
        }

        /* Initialize driver properties that required by device layer */
        vmk_Memset(&drv_props, 0, sizeof(vmk_DriverProps));

        status = vmk_NameInitialize(&drv_props.name, IONIC_DRV_NAME);
        if (VMK_UNLIKELY(status != VMK_OK)) {
                ionic_err("%s: vmk_NameInitialize() failed, status: %s",
                          IONIC_DRV_NAME, vmk_StatusToString(status));
                goto name_init_err;
        }

        drv_props.moduleID = ionic_driver.module_id;
        drv_props.ops      = &ionic_driver_ops;

        status = vmk_DriverRegister(&drv_props, &ionic_driver.drv_handle);
        if (status != VMK_OK) {
                ionic_err("%s: vmk_DriverRegister() failed, status: %s",
                          IONIC_DRV_NAME, vmk_StatusToString(status));
                goto name_init_err;
        }

        status = ionic_device_list_init(ionic_driver.module_id,
                                        ionic_driver.heap_id,
                                        ionic_en_get_sbdf,
                                        &ionic_driver.uplink_dev_list);
        if (status != VMK_OK) {
                ionic_err("ionic_device_list_init() failed, status: %s",
                          vmk_StatusToString(status));
                goto name_init_err;
        }

        return status;

name_init_err:
   vmk_LockDomainDestroy(ionic_driver.lock_domain);

lock_domain_err:
   ionic_heap_destroy(ionic_driver.heap_id);

heap_init_err:
   ionic_mem_pool_destroy(ionic_driver.mem_pool);

   return status;
}


/*
 ******************************************************************************
 *
 * cleanup_module --
 *
 *     Driver exit point
 *
 *  Parameters:
 *     None
 *
 *  Results:
 *     Destroy all driver data structures
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

void
cleanup_module()
{
        ionic_device_list_destroy(&ionic_driver.uplink_dev_list);
	vmk_DriverUnregister(ionic_driver.drv_handle);
	vmk_LockDomainDestroy(ionic_driver.lock_domain);	
	ionic_heap_destroy(ionic_driver.heap_id);
	ionic_mem_pool_destroy(ionic_driver.mem_pool);
}



