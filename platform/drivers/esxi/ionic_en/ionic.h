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
 * ionic.h --
 *
 * Definitions shared by ionic_en
 */

#ifndef _IONIC_H_
#define _IONIC_H_

#include <vmkapi.h>
#include "ionic_memory.h"
#include "ionic_locks.h"
#include "ionic_log.h"
#include "ionic_types.h"
#include "ionic_dev.h"
#include "ionic_pci.h"
#include "ionic_rx_filter.h"
#include "ionic_work_queue.h"
#include "ionic_utilities.h"
#include "ionic_time.h"
#include "ionic_dma.h"
#include "ionic_interrupt.h"
#include "ionic_logical_dev_register.h"
#include "ionic_lif.h"
#include "ionic_en_uplink.h"
#include "ionic_device_list.h"
#include "ionic_txrx.h"
#include "ionic_en_mq.h"
#include "ionic_api.h"
//#define ADMINQ
#define HAPS
#define IONIC_DRV_NAME        "ionic_en"
#define DRV_DESCRIPTION       "Pensando Ethernet NIC Driver"
#define DRV_VERSION           "0.2"
#define DRV_REL_DATE          "Jan-31-2019"

/* In bytes */
#define MEMPOOL_INIT_SIZE     (16 * 1024)
#define MEMPOOL_MAX_SIZE      (1024 * 1024 * 1024)
#define HEAP_INIT_SIZE        (16 * 1024)
#define HEAP_MAX_SIZE         (1024 * 1024 * 1024)

extern unsigned int ntxq_descs;
extern unsigned int nrxq_descs;
extern unsigned int ntxqs;
extern unsigned int nrxqs;
extern unsigned int devcmd_timeout;
extern struct ionic_driver ionic_driver;

#ifdef FAKE_ADMINQ
extern unsigned int use_AQ;
#endif

struct ionic_admin_ctx;

typedef enum ionic_bar {
        IONIC_BAR0              = 0x0,
        IONIC_BAR1              = 0x2,
} ionic_bar;

typedef struct ionic_bitmap {
        vmk_Lock                      lock;
        vmk_BitVector                 *bit_vector;
} ionic_bitmap;

struct ionic_en_device {
        struct ionic_dev              idev;
        vmk_Device                    vmk_device;
        vmk_Device                    uplink_vmk_dev;
        vmk_PCIDevice                 pci_device;
        vmk_PCIDeviceID               pci_device_id;
        vmk_PCIDeviceAddr             sbdf;
        vmk_PCIResource               pci_resources[IONIC_BARS_MAX];
        vmk_VA                        bars[IONIC_BARS_MAX];
        struct ionic_pci_device_entry dev_entry;
};

struct ionic {
//        struct pci_dev *pdev;
//        struct platform_device *pfdev;
//        struct device *dev;
//        vmk_PCIDevice *pdev;
//        vmk_Device *dev;

        struct ionic_en_device en_dev;

// Put it under ionic_en_device
//        struct ionic_dev idev;
        struct dentry *dentry;
	vmk_uint32 bar0_size;
        struct ionic_dev_bar bars[IONIC_BARS_MAX];
        unsigned int num_bars;
        union identity *ident;
        dma_addr_t ident_pa;
//        struct list_head lifs;
        //TODO: ADD LOCK TO PROTECT LIFS
        vmk_Lock lifs_lock;
        vmk_ListLinks lifs;
        vmk_Bool is_mgmt_nic;
        unsigned int neqs_per_lif;
        unsigned int ntxqs_per_lif;
        unsigned int nrxqs_per_lif;
        unsigned int nintrs;

        ionic_bitmap intrs;
//        DECLARE_BITMAP(intrs, INTR_CTRL_REGS_MAX);
#ifdef CONFIG_DEBUG_FS
#ifdef DEBUGFS_TEST_API
        void *scratch_bufs[NUM_SCRATCH_BUFS];
        dma_addr_t scratch_bufs_pa[NUM_SCRATCH_BUFS];
        struct debugfs_blob_wrapper scratch_bufs_blob[NUM_SCRATCH_BUFS];
#endif
#endif
#ifdef FAKE_ADMINQ
        spinlock_t cmd_lock;
 //       struct list_head cmd_list;
        vmk_ListLinks cmd_list;

        // TODO: MAKE SURE FREE IT
        struct ionic_work_queue *cmd_work_queue;
//     Need to implemnet the struct below
        struct ionic_work cmd_work;
#endif
};

struct ionic_driver {
        vmk_Name                      name;
        vmk_ModuleID                  module_id;
        vmk_HeapID                    heap_id;
        vmk_MemPool                   mem_pool;
        vmk_LockDomainID              lock_domain;
        vmk_Driver                    drv_handle;
        struct ionic_device_list      uplink_dev_list;        
};

struct ionic_en_priv_data {
        vmk_ModuleID                  module_id;
        vmk_HeapID                    heap_id;
        struct ionic                  ionic;
        struct ionic_en_uplink_handle uplink_handle;
        vmk_DMAEngine                 dma_engine_streaming;
        vmk_DMAEngine                 dma_engine_coherent;
        vmk_LockDomainID              lock_domain;
        vmk_MemPool                   mem_pool;
        vmk_IntrCookie                *intr_cookie_array;

        vmk_Bool                      is_lifs_size_compl;
};


VMK_ReturnStatus
ionic_pci_query(struct ionic_en_priv_data *priv_data);

VMK_ReturnStatus
ionic_pci_start(struct ionic_en_priv_data *priv_data);

void
ionic_pci_stop(struct ionic_en_priv_data *priv_data);

VMK_ReturnStatus
ionic_adminq_post_wait(struct lif *lif, struct ionic_admin_ctx *ctx);

int ionic_netpoll(int budget, ionic_cq_cb cb, void *cb_arg);

VMK_ReturnStatus
ionic_dev_cmd_wait_check(struct ionic_dev *idev, unsigned long max_wait);

#if 0
int ionic_adminq_check_err(struct lif *lif, struct ionic_admin_ctx *ctx);
int ionic_adminq_post_wait(struct lif *lif, struct ionic_admin_ctx *ctx);
int ionic_napi(struct napi_struct *napi, int budget, ionic_cq_cb cb,
               void *cb_arg);
int ionic_dev_cmd_wait_check(struct ionic_dev *idev, unsigned long max_wait);
int ionic_set_dma_mask(struct ionic *ionic);
int ionic_setup(struct ionic *ionic);
int ionic_identify(struct ionic *ionic);
void ionic_forget_identity(struct ionic *ionic);
int ionic_reset(struct ionic *ionic);
#endif

#endif /* End of _IONIC_H_ */
