/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <netdb.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/param.h>

#include "src/lib/misc/include/maclib.h"
#include "src/lib/misc/include/bdf.h"
#include "src/sim/libsimlib/include/simserver.h"
#include "src/sim/libsimdev/src/dev_utils.h"
#include "src/sim/libsimdev/src/simdev_impl.h"
#include "src/sim/libsimdev/src/pcie_regs.h"
#include "src/sim/libsimdev/src/virtio_dev_regs.h"
#include "src/sim/libsimdev/src/capri_defines.h"

typedef struct devparams_s {
    int lif;
    int txq_type;
    int txq_count;
    int txq_qidbase;
    int rxq_type;
    int rxq_count;
    int rxq_qidbase;
    int intr_base;
    int intr_count;
    int qidbase[8];
    u_int64_t qstate_addr;
    mac_t mac;

    uint8_t     virtio_bar_count;
    pcie_bar_t  virtio_pcie_bars[VIRTIO_PCIE_BAR_MAX];
} devparams_t;

typedef struct qstate_app_virtiotx_s {
    uint16_t    pi_0;
    uint16_t    ci_0;
    uint16_t    pi_1;
    uint16_t    ci_1;

    uint64_t    features;
    uint64_t    tx_virtq_desc_addr;
    uint64_t    tx_virtq_avail_addr;
    uint64_t    tx_virtq_used_addr;
    uint32_t    tx_intr_assert_addr;
    uint16_t    tx_queue_size_mask;

    uint16_t    tx_virtq_avail_ci;
    uint16_t    tx_virtq_used_pi;
} __attribute__((packed)) qstate_app_virtiotx_t;

typedef struct qstate_app_virtiorx_s {
    uint16_t    pi_0;
    uint16_t    ci_0;
    uint64_t    features;
    uint64_t    rx_virtq_desc_addr;
    uint64_t    rx_virtq_avail_addr;
    uint64_t    rx_virtq_used_addr;
    uint32_t    rx_intr_assert_addr;
    uint16_t    rx_queue_size_mask;

    uint16_t    rx_virtq_avail_ci;
    uint16_t    rx_virtq_used_pi;
} __attribute__((packed)) qstate_app_virtiorx_t;

static int
virtio_lif(simdev_t *sd)
{
    devparams_t *dp = sd->priv;
    return dp->lif;
}

static int
virtio_intrb(simdev_t *sd)
{
    devparams_t *dp = sd->priv;
    return dp->intr_base;
}

static int
virtio_intrc(simdev_t *sd)
{
    devparams_t *dp = sd->priv;
    return dp->intr_count;
}

static u_int64_t
virtio_qstate_addr(simdev_t *sd, const int type, const int qid)
{
    devparams_t *dp = sd->priv;
    /* XXX assumes all qstate entries are 64B */
    return (dp->qstate_addr + ((dp->qidbase[type] + qid) * sizeof(qstate_t)));
}

static int
virtio_read_qstate(simdev_t *sd, const int type, const int qid, qstate_t *qs)
{
    const u_int64_t addr = virtio_qstate_addr(sd, type, qid);
    simdev_log("%s: Type: %d, QID: %d, Addr: 0x%lx\n",
            __FUNCTION__, type, qid, addr);
    return simdev_read_mem(addr, qs, sizeof(qstate_t));
}

static int
virtio_write_qstate(simdev_t *sd, const int type, const int qid, qstate_t *qs)
{
    const u_int64_t addr = virtio_qstate_addr(sd, type, qid);
    simdev_log("%s: Type: %d, QID: %d, Addr: 0x%lx\n",
            __FUNCTION__, type, qid, addr);
    return simdev_write_mem(addr, qs, sizeof(qstate_t));
}

static int
virtio_read_txqstate(simdev_t *sd, const int txqid, qstate_t *qs)
{
    devparams_t *dp = sd->priv;
    const int qid = dp->txq_qidbase + txqid;
    return virtio_read_qstate(sd, dp->txq_type, qid, qs);
}

static int
virtio_write_txqstate(simdev_t *sd, const int txqid, qstate_t *qs)
{
    devparams_t *dp = sd->priv;
    const int qid = dp->txq_qidbase + txqid;
    return virtio_write_qstate(sd, dp->txq_type, qid, qs);
}

static int
virtio_read_rxqstate(simdev_t *sd, const int rxqid, qstate_t *qs)
{
    devparams_t *dp = sd->priv;
    const int qid = dp->rxq_qidbase + rxqid;
    return virtio_read_qstate(sd, dp->rxq_type, qid, qs);
}

static int
virtio_write_rxqstate(simdev_t *sd, const int rxqid, qstate_t *qs)
{
    devparams_t *dp = sd->priv;
    const int qid = dp->rxq_qidbase + rxqid;
    return virtio_write_qstate(sd, dp->rxq_type, qid, qs);
}

static void virtio_txq_setup_queue(simdev_t *sd,
                uint16_t txqid,
                uint16_t queue_size,
                uint64_t virtq_desc_addr,
                uint64_t virtq_avail_addr,
                uint64_t virtq_used_addr)
{
    qstate_t                qs;
    qstate_app_virtiotx_t   *qsvirtiotx;
    devparams_t             *dp = sd->priv;

    if (virtio_read_qstate(sd, dp->txq_type, txqid, &qs) < 0) {
        simdev_error("%s: read_qstate %d failed\n", __FUNCTION__, txqid);
        return;
    }

    qsvirtiotx = (qstate_app_virtiotx_t*) &qs.app_data;

    qsvirtiotx->tx_virtq_desc_addr = virtq_desc_addr;
    qsvirtiotx->tx_virtq_avail_addr = virtq_avail_addr;
    qsvirtiotx->tx_virtq_used_addr = virtq_used_addr;
    /* TODO: Use log2(queue_size) */
    qsvirtiotx->tx_queue_size_mask = ((1 << 8) - 1) /* queue_size */;


    if (virtio_write_qstate(sd, dp->txq_type, txqid, &qs) < 0) {
        simdev_error("%s: write_qstate %d failed\n", __FUNCTION__, txqid);
        return;
    }

    simdev_log("%s: Setting up Virtq TXQ(%d): Queue Size: %d, Desc Addr: 0x%lx,"
            " Avail Addr: 0x%lx, Used Addr: 0x%lx\n",
            __FUNCTION__, txqid, queue_size,
            virtq_desc_addr, virtq_avail_addr, virtq_used_addr);

}

static void virtio_rxq_setup_queue(simdev_t *sd,
                uint16_t rxqid,
                uint16_t queue_size,
                uint64_t virtq_desc_addr,
                uint64_t virtq_avail_addr,
                uint64_t virtq_used_addr)
{
    qstate_t                qs;
    qstate_app_virtiorx_t   *qsvirtiorx;
    devparams_t             *dp = sd->priv;

    if (virtio_read_qstate(sd, dp->rxq_type, rxqid, &qs) < 0) {
        simdev_error("%s: read_qstate %d failed\n", __FUNCTION__, rxqid);
        return;
    }

    qsvirtiorx = (qstate_app_virtiorx_t*) &qs.app_data;

    qsvirtiorx->rx_virtq_desc_addr = virtq_desc_addr;
    qsvirtiorx->rx_virtq_avail_addr = virtq_avail_addr;
    qsvirtiorx->rx_virtq_used_addr = virtq_used_addr;
    /* TODO: Use log2(queue_size) */
    qsvirtiorx->rx_queue_size_mask = ((1 << 8) - 1) /* queue_size */;


    if (virtio_write_qstate(sd, dp->rxq_type, rxqid, &qs) < 0) {
        simdev_error("%s: write_qstate %d failed\n", __FUNCTION__, rxqid);
        return;
    }
    simdev_log("%s: Setting up Virtq RXQ(%d): Queue Size: %d, Desc Addr: 0x%lx,"
            " Avail Addr: 0x%lx, Used Addr: 0x%lx\n",
            __FUNCTION__, rxqid, queue_size,
            virtq_desc_addr, virtq_avail_addr, virtq_used_addr);

}

static void virtio_txq_setup_intr_assert_addr(simdev_t *sd,
                uint16_t txqid,
                uint32_t intr_assert_addr)
{
    qstate_t                qs;
    qstate_app_virtiotx_t   *qsvirtiotx;
    devparams_t             *dp = sd->priv;

    if (virtio_read_qstate(sd, dp->txq_type, txqid, &qs) < 0) {
        simdev_error("%s: read_qstate %d failed\n", __FUNCTION__, txqid);
        return;
    }

    qsvirtiotx = (qstate_app_virtiotx_t*) &qs.app_data;

    qsvirtiotx->tx_intr_assert_addr = intr_assert_addr;

    if (virtio_write_qstate(sd, dp->txq_type, txqid, &qs) < 0) {
        simdev_error("%s: write_qstate %d failed\n", __FUNCTION__, txqid);
        return;
    }

    simdev_log("%s: Setting up Virtq TXQ(%d): Interrupt Assert Addr: 0x%x\n",
            __FUNCTION__, txqid, intr_assert_addr);
}

static void virtio_rxq_setup_intr_assert_addr(simdev_t *sd,
                uint16_t rxqid,
                uint32_t intr_assert_addr)
{
    qstate_t                qs;
    qstate_app_virtiorx_t   *qsvirtiorx;
    devparams_t             *dp = sd->priv;

    if (virtio_read_qstate(sd, dp->rxq_type, rxqid, &qs) < 0) {
        simdev_error("%s: read_qstate %d failed\n", __FUNCTION__, rxqid);
        return;
    }

    qsvirtiorx = (qstate_app_virtiorx_t*) &qs.app_data;

    qsvirtiorx->rx_intr_assert_addr = intr_assert_addr;

    if (virtio_write_qstate(sd, dp->rxq_type, rxqid, &qs) < 0) {
        simdev_error("%s: write_qstate %d failed\n", __FUNCTION__, rxqid);
        return;
    }

    simdev_log("%s: Setting up Virtq RXQ(%d): Interrupt Assert Addr: 0x%x\n",
            __FUNCTION__, rxqid, intr_assert_addr);
}

static void virtio_txq_init(simdev_t *sd, uint16_t  virtio_txqid)
{
    qstate_t                qs;
    qstate_app_virtiotx_t   *qsvirtiotx;

    if (virtio_read_txqstate(sd, virtio_txqid, &qs) < 0) {
        simdev_error("%s: read_qstate %d failed\n", __FUNCTION__, virtio_txqid);
        return;
    }

    qs.host = 1;
    qs.total = 2;

    qsvirtiotx = (qstate_app_virtiotx_t*) &qs.app_data;

    qsvirtiotx->pi_0 = 0;
    qsvirtiotx->ci_0 = 0;
    qsvirtiotx->pi_1 = 0;
    qsvirtiotx->ci_1 = 0;

    qsvirtiotx->features = 0;
    qsvirtiotx->tx_virtq_desc_addr = 0;
    qsvirtiotx->tx_virtq_avail_addr = 0;
    qsvirtiotx->tx_virtq_used_addr = 0;
    qsvirtiotx->tx_queue_size_mask = 0;
    qsvirtiotx->tx_intr_assert_addr = 0;
    qsvirtiotx->tx_virtq_avail_ci = 0;
    qsvirtiotx->tx_virtq_used_pi = 0;

    if (virtio_write_txqstate(sd, virtio_txqid, &qs) < 0) {
        simdev_error("%s: write_txqstate %d failed\n",
                     __FUNCTION__, virtio_txqid);
        return;
    }

}

static void virtio_rxq_init(simdev_t *sd, uint16_t  virtio_rxqid)
{
    qstate_t                qs;
    qstate_app_virtiorx_t   *qsvirtiorx;

    if (virtio_read_rxqstate(sd, virtio_rxqid, &qs) < 0) {
        simdev_error("%s: read_qstate %d failed\n", __FUNCTION__, virtio_rxqid);
        return;
    }

    qsvirtiorx = (qstate_app_virtiorx_t*) &qs.app_data;

    qsvirtiorx->features = 0;
    qsvirtiorx->rx_virtq_desc_addr = 0;
    qsvirtiorx->rx_virtq_avail_addr = 0;
    qsvirtiorx->rx_virtq_used_addr = 0;
    qsvirtiorx->rx_queue_size_mask = 0;
    qsvirtiorx->rx_intr_assert_addr = 0;
    qsvirtiorx->rx_virtq_avail_ci = 0;
    qsvirtiorx->rx_virtq_used_pi = 0;

    if (virtio_write_rxqstate(sd, virtio_rxqid, &qs) < 0) {
        simdev_error("%s: write_rxqstate %d failed\n",
                     __FUNCTION__, virtio_rxqid);
        return;
    }

}

static simdev_t *current_sd;

static int virtio_reg_readonly_write_access(simdev_t *sd, pcie_bar_t *bar, pcie_reg_t *reg, uint64_t reg_val);
static int virtio_reg_common_read(simdev_t *sd, pcie_bar_t *bar, pcie_reg_t *reg, uint64_t *reg_val);
static int virtio_reg_common_write(simdev_t *sd, pcie_bar_t *bar, pcie_reg_t *reg, uint64_t reg_val);


static int virtio_reg_driver_features_write(simdev_t *sd, pcie_bar_t *bar, pcie_reg_t *reg, uint64_t reg_val);
static int virtio_reg_queue_address_write(simdev_t *sd, pcie_bar_t *bar, pcie_reg_t *reg, uint64_t reg_val);
static int virtio_reg_queue_notify_write(simdev_t *sd, pcie_bar_t *bar, pcie_reg_t *reg, uint64_t reg_val);
static int virtio_reg_device_status_write(simdev_t *sd, pcie_bar_t *bar, pcie_reg_t *reg, uint64_t reg_val);
static int virtio_reg_queue_msix_vector_write(simdev_t *sd, pcie_bar_t *bar, pcie_reg_t *reg, uint64_t reg_val);

#define VIRTIO_LEGACY_REG_OFFSET_INVALID(offset)            \
    {                                                       \
        .reg_name       = "invalid_reg",                    \
        .reg_size       = 0,                                \
        .reg_offset     = offset,                           \
        .reg_access_type= 0,                                \
        .initreg64      = 0,                                \
        .reg_read_cb    = NULL,                             \
        .reg_write_cb   = NULL,                             \
    }


pcie_reg_def_t virtio_legacy_regs_defs[VIRTIO_LEGACY_REG_OFFSET_MAX + 1] =  {
    /* Offset 0: Device Features  */
    {
        .reg_name       = "device_features",
        .reg_size       = VIRTIO_LEGACY_REG_SIZE_DEVICE_FEATURES,
        .reg_offset     = VIRTIO_LEGACY_REG_OFFSET_DEVICE_FEATURES,
        .reg_access_type= (PCIE_REG_ACCESS_TYPE_R),
        //.initreg32      = 0x00000021,
        .initreg32      = 0x00000001,
        .reg_read_cb    = virtio_reg_common_read,
        .reg_write_cb   = virtio_reg_readonly_write_access,
    },
    /* Offset 1: Invalid */
    VIRTIO_LEGACY_REG_OFFSET_INVALID(1),
    /* Offset 2: Invalid */
    VIRTIO_LEGACY_REG_OFFSET_INVALID(2),
    /* Offset 3: Invalid */
    VIRTIO_LEGACY_REG_OFFSET_INVALID(3),
    /* Offset 4: Driver Features */
    {
        .reg_name       = "driver_features",
        .reg_size       = VIRTIO_LEGACY_REG_SIZE_DRIVER_FEATURES,
        .reg_offset     = VIRTIO_LEGACY_REG_OFFSET_DRIVER_FEATURES,
        .reg_access_type= (PCIE_REG_ACCESS_TYPE_R | PCIE_REG_ACCESS_TYPE_W),
        .initreg32      = 0,
        .reg_read_cb    = virtio_reg_common_read,
        .reg_write_cb   = virtio_reg_driver_features_write,
    },
    /* Offset 5: Invalid */
    VIRTIO_LEGACY_REG_OFFSET_INVALID(5),
    /* Offset 6: Invalid */
    VIRTIO_LEGACY_REG_OFFSET_INVALID(6),
    /* Offset 7: Invalid */
    VIRTIO_LEGACY_REG_OFFSET_INVALID(7),
    /* Offset 8: Queue Address */
    {
        .reg_name       = "queue_address",
        .reg_size       = VIRTIO_LEGACY_REG_SIZE_QUEUE_ADDRESS,
        .reg_offset     = VIRTIO_LEGACY_REG_OFFSET_QUEUE_ADDRESS,
        .reg_access_type= (PCIE_REG_ACCESS_TYPE_R | 
                PCIE_REG_ACCESS_TYPE_W | PCIE_REG_ACCESS_TYPE_INDEXED),
        .initreg32      = 0,
        .reg_read_cb    = virtio_reg_common_read,
        .reg_write_cb   = virtio_reg_queue_address_write,
    },
    /* Offset 9: Invalid */
    VIRTIO_LEGACY_REG_OFFSET_INVALID(9),
    /* Offset 10: Invalid */
    VIRTIO_LEGACY_REG_OFFSET_INVALID(10),
    /* Offset 11: Invalid */
    VIRTIO_LEGACY_REG_OFFSET_INVALID(11),
    /* Offset 12: Queue Size */
    {
        .reg_name       = "queue_size",
        .reg_size       = VIRTIO_LEGACY_REG_SIZE_QUEUE_SIZE,
        .reg_offset     = VIRTIO_LEGACY_REG_OFFSET_QUEUE_SIZE,
        .reg_access_type= (PCIE_REG_ACCESS_TYPE_R |
                PCIE_REG_ACCESS_TYPE_INDEXED),
        .initreg16      = 256, /* VIRTIO_NET_RX_QUEUE_DEFAULT_SIZE from QEMU */
        .reg_read_cb    = virtio_reg_common_read,
        .reg_write_cb   = virtio_reg_readonly_write_access,
    },
    /* Offset 13: Invalid */
    VIRTIO_LEGACY_REG_OFFSET_INVALID(13),
    /* Offset 14: Queue Select */
    {
        .reg_name       = "queue_select",
        .reg_size       = VIRTIO_LEGACY_REG_SIZE_QUEUE_SELECT,
        .reg_offset     = VIRTIO_LEGACY_REG_OFFSET_QUEUE_SELECT,
        .reg_access_type= (PCIE_REG_ACCESS_TYPE_R | PCIE_REG_ACCESS_TYPE_W),
        .initreg16      = 0,
        .reg_read_cb    = virtio_reg_common_read,
        .reg_write_cb   = virtio_reg_common_write,
    },
    /* Offset 15: Invalid */
    VIRTIO_LEGACY_REG_OFFSET_INVALID(15),
    /* Offset 16: Queue Notify */
    {
        .reg_name       = "queue_notify",
        .reg_size       = VIRTIO_LEGACY_REG_SIZE_QUEUE_NOTIFY,
        .reg_offset     = VIRTIO_LEGACY_REG_OFFSET_QUEUE_NOTIFY,
        .reg_access_type= (PCIE_REG_ACCESS_TYPE_R | PCIE_REG_ACCESS_TYPE_W),
        .initreg16      = 0,
        .reg_read_cb    = virtio_reg_common_read,
        .reg_write_cb   = virtio_reg_queue_notify_write,
    },
    /* Offset 17: Invalid */
    VIRTIO_LEGACY_REG_OFFSET_INVALID(17),
    /* Offset 18: Device Status */
    {
        .reg_name       = "device_status",
        .reg_size       = VIRTIO_LEGACY_REG_SIZE_DEVICE_STATUS,
        .reg_offset     = VIRTIO_LEGACY_REG_OFFSET_DEVICE_STATUS,
        .reg_access_type= (PCIE_REG_ACCESS_TYPE_R | PCIE_REG_ACCESS_TYPE_W),
        .initreg8       = 0,
        .reg_read_cb    = virtio_reg_common_read,
        .reg_write_cb   = virtio_reg_device_status_write,
    },
    /* Offset 19: ISR Status */
    {
        .reg_name       = "isr_status",
        .reg_size       = VIRTIO_LEGACY_REG_SIZE_ISR_STATUS,
        .reg_offset     = VIRTIO_LEGACY_REG_OFFSET_ISR_STATUS,
        .reg_access_type= (PCIE_REG_ACCESS_TYPE_R),
        .initreg8       = 0,
        .reg_read_cb    = virtio_reg_common_read,
        .reg_write_cb   = virtio_reg_readonly_write_access,
    },
    /* Offset 20: Config MSIX Vector */
    {
        .reg_name       = "config_msix_vector",
        .reg_size       = VIRTIO_LEGACY_REG_SIZE_CONFIG_MSIX_VECTOR,
        .reg_offset     = VIRTIO_LEGACY_REG_OFFSET_CONFIG_MSIX_VECTOR,
        .reg_access_type= (PCIE_REG_ACCESS_TYPE_R | PCIE_REG_ACCESS_TYPE_W),
        .initreg16      = 0,
        .reg_read_cb    = virtio_reg_common_read,
        .reg_write_cb   = virtio_reg_common_write,
    },
    /* Offset 21: Invalid */
    VIRTIO_LEGACY_REG_OFFSET_INVALID(21),
    /* Offset 22: Queue MSIX Vector */
    {
        .reg_name       = "queue_msix_vector",
        .reg_size       = VIRTIO_LEGACY_REG_SIZE_QUEUE_MSIX_VECTOR,
        .reg_offset     = VIRTIO_LEGACY_REG_OFFSET_QUEUE_MSIX_VECTOR,
        .reg_access_type= (PCIE_REG_ACCESS_TYPE_R | 
                PCIE_REG_ACCESS_TYPE_W | PCIE_REG_ACCESS_TYPE_INDEXED),
        .initreg16      = 0,
        .reg_read_cb    = virtio_reg_common_read,
        .reg_write_cb   = virtio_reg_queue_msix_vector_write,
    },
};

static inline pcie_bar_t *virtio_pcie_bar_get(simdev_t *sd, uint8_t bar)
{
    devparams_t     *dp = (devparams_t*) sd->priv;
    if (bar >= VIRTIO_PCIE_BAR_MAX)
        return NULL;
    return &dp->virtio_pcie_bars[bar];
}

static inline pcie_reg_t *virtio_pcie_reg_get(simdev_t *sd, pcie_bar_t *pcie_bar,
        uint16_t reg_offset, uint8_t reg_size)
{
    pcie_reg_t      *virtio_reg;

    if ((reg_offset < pcie_bar->reg_min_offset) ||
            (reg_offset > pcie_bar->reg_max_offset)) {
        simdev_error("Invalid (0x%hx) offset access for BAR%d",
                reg_offset, pcie_bar->bar_number);
        return NULL;
    }

    virtio_reg = &pcie_bar->reg_list[reg_offset];

    if (!virtio_reg->reg_def->reg_size) {
        simdev_error("Invalid (0x%hx) offset access for BAR%d",
                reg_offset, pcie_bar->bar_number);
        return NULL;
    }

    if (virtio_reg->reg_def->reg_size != reg_size) {
        simdev_error("Invalid register size (%d) access for BAR%d:Reg:0x%hx",
                reg_size, pcie_bar->bar_number, reg_offset);
        return NULL;
    }

    return virtio_reg;
}

static inline int virtio_pcie_reg_read(simdev_t *sd, uint8_t bar,
        uint16_t offset, uint8_t size, uint64_t *pval)
{
    pcie_bar_t      *pbar;
    pcie_reg_t      *preg;

    pbar = virtio_pcie_bar_get(sd, bar);
    if (!pbar) {
        simdev_error("%s: Invalid BAR (%d)\n", __FUNCTION__, bar);
        return -1;
    }

    preg = virtio_pcie_reg_get(sd, pbar, offset, size);
    if (!preg) {
        simdev_error("%s: BAR(%d): Invalid register (offset:%d, size:%d)\n",
                __FUNCTION__, bar, offset, size);
        return -1;
    }

    return preg->reg_def->reg_read_cb(sd, pbar, preg, pval);
}

static inline int virtio_pcie_reg_write(simdev_t *sd, uint8_t bar,
        uint16_t offset, uint8_t size, uint64_t val)
{
    pcie_bar_t      *pbar;
    pcie_reg_t      *preg;

    pbar = virtio_pcie_bar_get(sd, bar);
    if (!pbar) {
        simdev_error("%s: Invalid BAR (%d)\n", __FUNCTION__, bar);
        return -1;
    }

    preg = virtio_pcie_reg_get(sd, pbar, offset, size);
    if (!preg) {
        simdev_error("%s: BAR(%d): Invalid register (offset:%d, size:%d)\n",
                __FUNCTION__, bar, offset, size);
        return -1;
    }

    return preg->reg_def->reg_write_cb(sd, pbar, preg, val);
}

int virtio_pcie_bar0_init(pcie_bar_t *bar0, uint16_t virtq_count)
{
    int             reg_offset;
    int             reg_idx;
    pcie_reg_t      *reg;

    bar0->bar_number = 0;
    bar0->bar_type = PCIE_BAR_TYPE_IO;
    bar0->reg_min_offset = VIRTIO_LEGACY_REG_OFFSET_MIN;
    bar0->reg_max_offset = VIRTIO_LEGACY_REG_OFFSET_MAX;
    bar0->reg_list = calloc((VIRTIO_LEGACY_REG_OFFSET_MAX + 1), sizeof(pcie_reg_t));

    for (reg_offset = VIRTIO_LEGACY_REG_OFFSET_MIN; reg_offset <= VIRTIO_LEGACY_REG_OFFSET_MAX; reg_offset++) {
        reg = &bar0->reg_list[reg_offset];
        reg->reg_def = &virtio_legacy_regs_defs[reg_offset];
        if (reg->reg_def->reg_size) {
            if (reg->reg_def->reg_access_type &
                    PCIE_REG_ACCESS_TYPE_INDEXED) {
                reg->reg_array = calloc(virtq_count, sizeof(uint64_t));
                reg->reg_idx_count = virtq_count;
                for (reg_idx = 0; reg_idx < virtq_count; reg_idx++) {
                    switch (reg->reg_def->reg_size) {
                        case 1:
                            reg->reg_array[reg_idx] = reg->reg_def->initreg8;
                            break;
                        case 2:
                            reg->reg_array[reg_idx] = reg->reg_def->initreg16;
                            break;
                        case 4:
                            reg->reg_array[reg_idx] = reg->reg_def->initreg32;
                            break;
                        case 8:
                            reg->reg_array[reg_idx]= reg->reg_def->initreg64;
                            break;
                        default:
                            break;
                    }
                }
            }
            else {
                switch (reg->reg_def->reg_size) {
                    case 1:
                        reg->reg8 = reg->reg_def->initreg8;
                        break;
                    case 2:
                        reg->reg16 = reg->reg_def->initreg16;
                        break;
                    case 4:
                        reg->reg32 = reg->reg_def->initreg32;
                        break;
                    case 8:
                        reg->reg64= reg->reg_def->initreg64;
                        break;
                    default:
                        break;
                }
            }
        }
    }
    return 0;
}

int virtio_pcie_bar0_uninit(pcie_bar_t *bar0)
{
    int             reg_offset;
    pcie_reg_t      *reg;

    for (reg_offset = VIRTIO_LEGACY_REG_OFFSET_MIN; reg_offset <= VIRTIO_LEGACY_REG_OFFSET_MAX; reg_offset++) {
        reg = &bar0->reg_list[reg_offset];
        if ((reg->reg_def->reg_size) && (reg->reg_def->reg_access_type &
                    PCIE_REG_ACCESS_TYPE_INDEXED)) {
            if (reg->reg_array)
                free(reg->reg_array);
        }
    }
    free(bar0->reg_list);
    return 0;
}

/* Common helper callbacks */

static int virtio_reg_readonly_write_access(simdev_t *sd, pcie_bar_t *bar, pcie_reg_t *reg, uint64_t reg_val)
{
    simdev_error("Write attempt to read-only register (%s)", reg->reg_def->reg_name);
    return -1;
}

static inline uint16_t virtio_reg_index_get(simdev_t *sd, pcie_bar_t *bar)
{
    pcie_reg_t      *preg = 
        &bar->reg_list[VIRTIO_LEGACY_REG_OFFSET_QUEUE_SELECT];
    
    return preg->reg16;
}

static int virtio_reg_common_read(simdev_t *sd, pcie_bar_t *bar, pcie_reg_t *reg, uint64_t *reg_val)
{
    uint16_t                reg_idx;

    if (reg->reg_def->reg_access_type & PCIE_REG_ACCESS_TYPE_INDEXED) {
        reg_idx = virtio_reg_index_get(sd, bar);

        *reg_val = reg->reg_array[reg_idx];
        simdev_log("%s: reg(%s) index(%d) returning: 0x%lx\n",
                __FUNCTION__, reg->reg_def->reg_name, reg_idx, *reg_val);
    }
    else {

        switch(reg->reg_def->reg_size) {
            case 1:
                *((uint8_t*)reg_val) = reg->reg8;
                simdev_log("%s: reg(%s) returning: 0x%hhx\n",
                        __FUNCTION__, reg->reg_def->reg_name, reg->reg8);
                break;
            case 2:
                *((uint16_t*)reg_val) = reg->reg16;
                simdev_log("%s: reg(%s) returning: 0x%hx\n",
                        __FUNCTION__, reg->reg_def->reg_name, reg->reg16);
                break;
            case 4:
                *((uint32_t*)reg_val) = reg->reg32;
                simdev_log("%s: reg(%s) returning: 0x%x\n",
                        __FUNCTION__, reg->reg_def->reg_name, reg->reg32);
                break;
            case 8:
                *((uint64_t*)reg_val) = reg->reg64;
                simdev_log("%s: reg(%s) returning: 0x%lx\n",
                        __FUNCTION__, reg->reg_def->reg_name, reg->reg64);
                break;
            default:
                simdev_error("%s: Invalid register size! (%d)\n", __FUNCTION__, reg->reg_def->reg_size);
                return -1;

        }
    }
    return 0;
}

/* For local use */
static inline void virtio_reg_read(simdev_t *sd, pcie_bar_t *pcie_bar, uint16_t reg_offset, uint64_t *reg_val)
{
    pcie_reg_t      *preg;
    int             ret = 0;
   
    assert((reg_offset >= pcie_bar->reg_min_offset) &&
            (reg_offset <= pcie_bar->reg_max_offset));

    preg = &pcie_bar->reg_list[reg_offset];
    ret = virtio_reg_common_read(sd, pcie_bar, preg, reg_val);
    if (ret)
        *reg_val = 0xffffffffffffffffULL;
}


static int virtio_reg_common_write(simdev_t *sd, pcie_bar_t *bar, pcie_reg_t *reg, uint64_t reg_val)
{
    uint16_t                reg_idx;

    if (reg->reg_def->reg_access_type & PCIE_REG_ACCESS_TYPE_INDEXED) {
        reg_idx = virtio_reg_index_get(sd, bar);

        reg->reg_array[reg_idx] = reg_val;
        simdev_log("%s: reg(%s) index(%d) writing : 0x%lx\n",
                __FUNCTION__, reg->reg_def->reg_name,
                reg_idx, reg->reg_array[reg_idx]);
    }
    else {
        switch(reg->reg_def->reg_size) {
            case 1:
                reg->reg8 = (uint8_t) reg_val;
                simdev_log("%s: reg(%s) writing : 0x%hhx\n",
                        __FUNCTION__, reg->reg_def->reg_name, reg->reg8);
                break;
            case 2:
                reg->reg16 = (uint16_t) reg_val;
                simdev_log("%s: reg(%s) writing : 0x%hx\n",
                        __FUNCTION__, reg->reg_def->reg_name, reg->reg16);
                break;
            case 4:
                reg->reg32 = (uint32_t) reg_val;
                simdev_log("%s: reg(%s) writing : 0x%x\n",
                        __FUNCTION__, reg->reg_def->reg_name, reg->reg32);
                break;
            case 8:
                reg->reg64 = (uint64_t) reg_val;
                simdev_log("%s: reg(%s) writing : 0x%lx\n",
                        __FUNCTION__, reg->reg_def->reg_name, reg->reg64);
                break;
            default:
                simdev_error("%s: Invalid register size! (%d)\n", __FUNCTION__, reg->reg_def->reg_size);
                return -1;

        }
    }
    return 0;
}

static inline uint8_t virtio_qid_is_rxq(simdev_t *sd, uint16_t vqid)
{
    /* TODO: Use context in sd when we do support control vq */
    if ((vqid & 0x0001)) {
        return 0;
    }
    return 1;
}

static inline uint8_t virtio_qid_is_txq(simdev_t *sd, uint16_t vqid)
{
    /* TODO: Use context in sd when we do support control vq */
    return (!virtio_qid_is_rxq(sd, vqid));
}

/* Mapping between virtio queue ID and TX/RX queue ID */
static inline uint16_t virtio_qid_to_rxqid(simdev_t *sd, uint16_t vqid)
{
    return vqid/2;
}

static inline uint16_t virtio_qid_to_txqid(simdev_t *sd, uint16_t vqid)
{
    return (vqid - 1)/2;
}


/* Driver Features callbacks */

static int virtio_reg_driver_features_write(simdev_t *sd, pcie_bar_t *bar, pcie_reg_t *reg, uint64_t reg_val)
{
    uint32_t        driver_features = 0;
    uint32_t        device_features = 0;
    uint64_t        tmp;

    /* TODO: Validate that the driver features are a subset 
     * of the device advertised features
     */

    virtio_reg_read(sd, bar, VIRTIO_LEGACY_REG_OFFSET_DEVICE_FEATURES, &tmp);
    device_features = (uint32_t) tmp;
    driver_features = (uint32_t) reg_val;

    if (driver_features & (~device_features)) {
        simdev_error("%s:Invalid feature set from the driver: (0x%x), supported (0x%x)\n",
                __FUNCTION__, driver_features, device_features);
        /* Force driver_features to 0 for now */
        reg_val = 0;
    }

    reg->reg32 = (uint32_t) reg_val;
    simdev_log("%s: reg(%s) writing : 0x%x\n",
            __FUNCTION__, reg->reg_def->reg_name, reg->reg32);
    return 0;
}

static inline void virtio_resolve_ring_addresses(uint64_t queue_address,
        uint16_t queue_size, uint64_t *virtq_desc_addr,
        uint64_t *virtq_avail_addr, uint64_t *virtq_used_addr)
{
    *virtq_desc_addr = queue_address;
    *virtq_avail_addr = *virtq_desc_addr + (queue_size * sizeof(virtq_desc_t));
    *virtq_used_addr = VIRTIO_ALIGN_UP(*virtq_avail_addr +
            offsetof(virtq_avail_t, ring[queue_size]), VIRTIO_PCI_VRING_ALIGN);
}

static int virtio_reg_queue_address_write(simdev_t *sd, pcie_bar_t *bar, pcie_reg_t *reg, uint64_t reg_val)
{
    uint64_t        queue_address = 0;
    uint64_t        queue_size = 0;
    uint16_t        reg_idx;
    uint64_t        virtq_desc_addr = 0;
    uint64_t        virtq_avail_addr = 0;
    uint64_t        virtq_used_addr = 0;

    reg_idx = virtio_reg_index_get(sd, bar);

    /* For legacy mode, the queue_size is readonly and hence should already be setup at initialization */
    virtio_reg_read(sd, bar, VIRTIO_LEGACY_REG_OFFSET_QUEUE_SIZE, &queue_size);
    
    queue_address = reg_val; /* PFN */
    queue_address <<= 12;    /* Physical address */

    virtio_resolve_ring_addresses(queue_address, queue_size,
            &virtq_desc_addr, &virtq_avail_addr, &virtq_used_addr);

    if (virtio_qid_is_rxq(sd, reg_idx)) {
        virtio_rxq_setup_queue(sd,
                virtio_qid_to_rxqid(sd, reg_idx),
                (uint16_t)queue_size,
                virtq_desc_addr,
                virtq_avail_addr,
                virtq_used_addr);
    }
    else {
        virtio_txq_setup_queue(sd,
                virtio_qid_to_txqid(sd, reg_idx),
                (uint16_t)queue_size,
                virtq_desc_addr,
                virtq_avail_addr,
                virtq_used_addr);
    }

    reg->reg_array[reg_idx] = reg_val;
    simdev_log("%s: reg(%s) index(%d) writing : 0x%lx\n",
            __FUNCTION__, reg->reg_def->reg_name,
            reg_idx, reg->reg_array[reg_idx]);
    return 0;
}


static inline void virtio_post_doorbell(simdev_t *sd, uint16_t lif,
        uint8_t type, uint16_t txqid, uint8_t ring)
{

    uint64_t            host_base = db_host_addr(lif);
    uint64_t            db_addr = 0, db_data = 0;

    db_addr = capri_doorbell_address(CAPRI_DB_ADDR_PID_CHECK_NONE,
            CAPRI_DB_ADDR_IDX_PIDX_INCR,
            CAPRI_DB_ADDR_SCHED_UPD_EVAL,
            lif, type);

    db_data = capri_doorbell_data(0, txqid, ring, 0);

    simdev_doorbell((host_base + db_addr), db_data);
    simdev_log("%s: Posting doorbell: Addr: 0x%lx, Data: 0x%lx\n",
            __FUNCTION__, (host_base + db_addr), db_data);
}

static int virtio_reg_queue_notify_write(simdev_t *sd, pcie_bar_t *bar, pcie_reg_t *reg, uint64_t reg_val)
{
    uint16_t            vqid = 0;
    uint16_t            txqid = 0;
    devparams_t         *dp = sd->priv;

    vqid = (uint16_t) reg_val;
    /* TODO: Validate virtq */
    if (virtio_qid_is_rxq(sd, vqid)) {
        /* Do nothing for now */
    }
    else {
        txqid = virtio_qid_to_txqid(sd, vqid);
        /* Post doorbell */
        //simdev_log("%s: Posting dbell: LIF:%d, Type:%d, QID: %d\n", 
        //        __FUNCTION__, virtio_lif(sd), dp->txq_type, txqid);
        virtio_post_doorbell(sd, virtio_lif(sd), dp->txq_type, txqid, VIRTIO_TX_TYPE_RING_HOST);
    }

    reg->reg16 = (uint16_t) reg_val;
    simdev_log("%s: reg(%s) writing : 0x%hx\n",
            __FUNCTION__, reg->reg_def->reg_name, reg->reg16);

    return 0;
}

static int virtio_reg_device_status_write(simdev_t *sd, pcie_bar_t *bar, pcie_reg_t *reg, uint64_t reg_val)
{

    if (reg_val == 0) {
        /* Device reset */
        virtio_txq_init(sd, 0);
        virtio_rxq_init(sd, 0);
    }

    reg->reg8 = (uint8_t) reg_val;
    simdev_log("%s: reg(%s) writing : 0x%hhx\n",
            __FUNCTION__, reg->reg_def->reg_name, reg->reg8);

    return 0;
}

static int virtio_reg_queue_msix_vector_write(simdev_t *sd, pcie_bar_t *bar, pcie_reg_t *reg, uint64_t reg_val)
{
    uint64_t        queue_msix_vector = 0x0fff;
    uint32_t        ia_addr = 0;
    uint16_t        reg_idx;
    devparams_t     *dp = sd->priv;

    queue_msix_vector = reg_val;
    /* TODO: validate MSIX vector */

    ia_addr = intr_assert_addr(dp->intr_base + queue_msix_vector);

    reg_idx = virtio_reg_index_get(sd, bar);

    if (virtio_qid_is_rxq(sd, reg_idx)) {
        virtio_rxq_setup_intr_assert_addr(sd, 
                virtio_qid_to_rxqid(sd, reg_idx),
                ia_addr);
    }
    else {
        virtio_txq_setup_intr_assert_addr(sd, 
                virtio_qid_to_txqid(sd, reg_idx),
                ia_addr);
    }

    reg->reg_array[reg_idx] = reg_val;
    simdev_log("%s: reg(%s) index(%d) writing : 0x%lx\n",
            __FUNCTION__, reg->reg_def->reg_name,
            reg_idx, reg->reg_array[reg_idx]);
    return 0;
}

static int
dev_lif(simdev_t *sd)
{
    devparams_t *dp = sd->priv;
    return dp->lif;
}

static int
dev_intrb(simdev_t *sd)
{
    devparams_t *dp = sd->priv;
    return dp->intr_base;
}

static int
bar0_rd(simdev_t *sd, int bar, u_int64_t offset, u_int8_t size, u_int64_t *valp)
{
    uint64_t    val;

    if (virtio_pcie_reg_read(sd, (uint8_t) bar, (uint16_t) offset, size, &val) < 0) {
        return -1;
    }
    *valp = val;
    return 0;
}

static int
bar0_wr(simdev_t *sd, int bar, u_int64_t offset, u_int8_t size, u_int64_t val)
{
    if (virtio_pcie_reg_write(sd, (uint8_t) bar, (uint16_t) offset, size, val) < 0) {
        return -1;
    }
    return 0;
}

static int
bar1_rd(int bar, u_int64_t offset, u_int8_t size, u_int64_t *valp)
{
    int r = 0;

    if (offset < 0x1000) {
        const int intrb = dev_intrb(current_sd);
        r = msixtbl_rd(intrb, offset, size, valp);
    } else {
        const int lif = dev_lif(current_sd);
        r = msixpba_rd(lif, offset - 0x1000, size, valp);
    }
    return r;
}

static int
bar1_wr(int bar, u_int64_t offset, u_int8_t size, u_int64_t val)
{
    int r = 0;

    if (offset < 0x1000) {
        const int intrb = dev_intrb(current_sd);
        r = msixtbl_wr(intrb, offset, size, val);
    } else {
        const int lif = dev_lif(current_sd);
        r = msixpba_wr(lif, offset - 0x1000, size, val);
    }
    return r;
}

/*
 * ================================================================
 * process messages
 * ----------------------------------------------------------------
 */

static int
virtio_memrd(simdev_t *sd, simmsg_t *m, u_int64_t *valp)
{
    const u_int16_t bdf  = m->u.read.bdf;
    const u_int8_t  bar  = m->u.read.bar;
    const u_int64_t addr = m->u.read.addr;
    const u_int8_t  size = m->u.read.size;
    int r;

    current_sd = sd;

    switch (bar) {
    case 1:
        r = bar1_rd(bar, addr, size, valp);
        break;
    default:
        simdev_error("memrd: unhandled bar %d\n", bar);
        r = -1;
        break;
    }

    if (r < 0) {
        sims_readres(sd->fd, bdf, bar, addr, size, 0, EFAULT);
        return -1;
    }

    sims_readres(sd->fd, bdf, bar, addr, size, *valp, 0);
    return 0;
}

static void
virtio_memwr(simdev_t *sd, simmsg_t *m)
{
    const u_int16_t bdf  = m->u.write.bdf;
    const u_int8_t  bar  = m->u.write.bar;
    const u_int64_t addr = m->u.write.addr;
    const u_int8_t  size = m->u.write.size;
    const u_int64_t val  = m->u.write.val;

    current_sd = sd;

    switch (bar) {
    case 1:
        bar1_wr(bar, addr, size, val);
        break;
    default:
        simdev_error("memwr: unhandled bar %d\n", bar);
        break;
    }
    sims_writeres(sd->fd, bdf, bar, addr, size, 0);
}

static int
virtio_iord(simdev_t *sd, simmsg_t *m, u_int64_t *valp)
{
    const u_int16_t bdf  = m->u.read.bdf;
    const u_int8_t  bar  = m->u.read.bar;
    const u_int64_t addr = m->u.read.addr;
    const u_int8_t  size = m->u.read.size;
    int r;

    current_sd = sd;

    switch (bar) {
    case 0:
        r = bar0_rd(sd, bar, addr, size, valp);
        break;
    default:
        simdev_error("iord: unhandled bar %d\n", bar);
        r = -1;
        break;
    }

    if (r < 0) {
        sims_readres(sd->fd, bdf, bar, addr, size, 0, EFAULT);
        return -1;
    }

    sims_readres(sd->fd, bdf, bar, addr, size, *valp, 0);
    return 0;
}

static void
virtio_iowr(simdev_t *sd, simmsg_t *m)
{
    const u_int16_t bdf  = m->u.write.bdf;
    const u_int8_t  bar  = m->u.write.bar;
    const u_int64_t addr = m->u.write.addr;
    const u_int8_t  size = m->u.write.size;
    const u_int64_t val  = m->u.write.val;

    current_sd = sd;

    switch (bar) {
    case 0:
        bar0_wr(sd, bar, addr, size, val);
        break;
    default:
        simdev_error("iowr: unhandled bar %d\n", bar);
        break;
    }
    sims_writeres(sd->fd, bdf, bar, addr, size, 0);
}

static void
virtio_init_lif(simdev_t *sd)
{
    /* anything to do for lif? */
}

static void
virtio_init_intr_pba_cfg(simdev_t *sd)
{
    const u_int32_t lif = virtio_lif(sd);
    const u_int32_t intrb = virtio_intrb(sd);
    const u_int32_t intrc = virtio_intrc(sd);

    intr_pba_cfg(lif, intrb, intrc);
}

static void
virtio_init_intr_fwcfg(simdev_t *sd)
{
    const int lif = virtio_lif(sd);
    const u_int32_t intrb = virtio_intrb(sd);
    const u_int32_t intrc = virtio_intrc(sd);
    u_int32_t intr;

    for (intr = intrb; intr < intrb + intrc; intr++) {
        intr_fwcfg_msi(intr, lif, 0);
    }
}

static void
virtio_init_intr_pba(simdev_t *sd)
{
    const u_int32_t intrb = virtio_intrb(sd);
    const u_int32_t intrc = virtio_intrc(sd);
    u_int32_t intr;

    for (intr = intrb; intr < intrb + intrc; intr++) {
        intr_pba_clear(intr);
    }
}

static void
virtio_init_intr_drvcfg(simdev_t *sd)
{
    const u_int32_t intrb = virtio_intrb(sd);
    const u_int32_t intrc = virtio_intrc(sd);
    u_int32_t intr;

    for (intr = intrb; intr < intrb + intrc; intr++) {
        intr_drvcfg_unmasked(intr);
    }
}

static void
virtio_init_intr_msixcfg(simdev_t *sd)
{
    const u_int32_t intrb = virtio_intrb(sd);
    const u_int32_t intrc = virtio_intrc(sd);
    u_int32_t intr;

    for (intr = intrb; intr < intrb + intrc; intr++) {
        intr_msixcfg(intr, 0, 0, 0);
    }
}

static void
virtio_init_intrs(simdev_t *sd)
{
    virtio_init_intr_pba_cfg(sd);
    virtio_init_intr_fwcfg(sd);
    virtio_init_intr_pba(sd);
    virtio_init_intr_drvcfg(sd);
    virtio_init_intr_msixcfg(sd);
}

static void
virtio_init_device(simdev_t *sd)
{
    virtio_init_lif(sd);
    virtio_init_intrs(sd);
}

static int
virtio_init(simdev_t *sd, const char *devparams)
{
    devparams_t *dp;
    char pbuf[80];
    pcie_bar_t  *pbar;

    if (devparam_str(devparams, "help", NULL, 0) == 0) {
        simdev_error("virtio params:\n"
                     "    lif=<lif>\n"
                     "    txq_type=<txq_type>\n"
                     "    txq_count=<txq_count>\n"
                     "    txq_qidbase=<txq_qidbase>\n"
                     "    rxq_type=<rxq_type>\n"
                     "    rxq_count=<rxq_count>\n"
                     "    rxq_qidbase=<rxq_qidbase>\n"
                     "    intr_base=<intr_base>\n"
                     "    intr_count=<intr_count>\n");
        return -1;
    }

    dp = calloc(1, sizeof(devparams_t));
    if (dp == NULL) {
        simdev_error("devparams alloc failed: no mem\n");
        return -1;
    }
    sd->priv = dp;

#define GET_PARAM(P, TYP) \
    devparam_##TYP(devparams, #P, &dp->P)

    GET_PARAM(lif, int);
    GET_PARAM(txq_type, int);
    GET_PARAM(txq_count, int);
    GET_PARAM(txq_qidbase, int);
    GET_PARAM(rxq_type, int);
    GET_PARAM(rxq_count, int);
    GET_PARAM(rxq_qidbase, int);
    GET_PARAM(intr_base, int);
    GET_PARAM(intr_count, int);
    GET_PARAM(qstate_addr, u64);
    GET_PARAM(mac, mac);

    /*
     * qidbase=0:1:2:3:4:5:6:7
     */
    if (devparam_str(devparams, "qidbase", pbuf, sizeof(pbuf)) == 0) {
        char *p, *q, *sp;
        int i;

        q = pbuf;
        for (i = 0; i < 8 && (p = strtok_r(q, ":", &sp)) != NULL; i++) {
            dp->qidbase[i] = strtoul(p, NULL, 0);
            if (q != NULL) q = NULL;
        }
    }

    /* Virtio specific per device/LIF initialization */
    dp->virtio_bar_count = 1;
    /* BAR 0 - IO Setup */
    pbar = &dp->virtio_pcie_bars[0];
    if (virtio_pcie_bar0_init(pbar, 2) == -1) {
        simdev_error("Failed to initialize Virtio BAR0 for device\n");
    }

    virtio_init_device(sd);

    return 0;
}

static void
virtio_free(simdev_t *sd)
{
    devparams_t     *dp = (devparams_t*) sd->priv;
    pcie_bar_t  *pbar;

    pbar = &dp->virtio_pcie_bars[0];
    if (virtio_pcie_bar0_uninit(pbar) == -1) {
        simdev_error("Failed to uninitialize Virtio BAR0 for device\n");
    }
    free(dp);
    sd->priv = NULL;
}

dev_ops_t virtio_ops = {
    .init  = virtio_init,
    .free  = virtio_free,
    .cfgrd = generic_cfgrd,
    .cfgwr = generic_cfgwr,
    .memrd = virtio_memrd,
    .memwr = virtio_memwr,
    .iord  = virtio_iord,
    .iowr  = virtio_iowr,
};
