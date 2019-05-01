#ifndef __ACCEL_IF_HPP__
#define __ACCEL_IF_HPP__

#define ACCEL_DEV_PAGE_SIZE             4096
#define ACCEL_DEV_PAGE_MASK             (ACCEL_DEV_PAGE_SIZE - 1)

#define ACCEL_DEV_ADDR_ALIGN(addr, sz)  \
    (((addr) + ((uint64_t)(sz) - 1)) & ~((uint64_t)(sz) - 1))

#define ACCEL_DEV_PAGE_ALIGN(addr)      \
    ACCEL_DEV_ADDR_ALIGN(addr, ACCEL_DEV_PAGE_SIZE)

/* Supply these for accel_dev_if.h */
#define dma_addr_t uint64_t
#include "nic/include/accel_ring.h"
#include "nic/sdk/storage/storage_seq_common.h"

#include "nic/include/accel_dev_if.h"

#pragma pack(push, 1)

/**
 * Accelerator PF Devcmd Region
 */

typedef union dev_cmd {
    uint32_t                    words[16];
    admin_cmd_t                 cmd;
    nop_cmd_t                   nop;
    reset_cmd_t                 reset;
    identify_cmd_t              identify;
    lif_init_cmd_t              lif_init;
    lif_reset_cmd_t             lif_reset;
    hang_notify_cmd_t           hang_notify;
    adminq_init_cmd_t           adminq_init;
    seq_queue_init_cmd_t        seq_q_init;
    seq_queue_init_complete_cmd_t seq_q_init_complete;
    seq_queue_batch_init_cmd_t  seq_q_batch_init;
    seq_queue_control_cmd_t     q_control;
    seq_queue_batch_control_cmd_t q_batch_control;
} dev_cmd_t;

typedef union dev_cmd_cpl {
    uint32_t                    words[4];
    uint8_t                     status;
    admin_cpl_t                 cpl;
    nop_cpl_t                   nop;
    reset_cpl_t                 reset;
    identify_cpl_t              identify;
    lif_init_cpl_t              lif_init;
    lif_reset_cpl_t             lif_reset;
    hang_notify_cpl_t           hang_notify;
    adminq_init_cpl_t           adminq_init;
    seq_queue_init_cpl_t        seq_q_init;
    seq_queue_init_complete_cpl_t seq_q_init_complete;
    seq_queue_batch_init_cpl_t  seq_q_batch_init;
    seq_queue_control_cpl_t     q_control;
    seq_queue_batch_control_cpl_t q_batch_control;
} dev_cmd_cpl_t;
#pragma pack(pop)

typedef struct accel_dev_cmd_regs {
    uint32_t                    signature;
    uint32_t                    done;
    dev_cmd_t                   cmd;
    dev_cmd_cpl_t               cpl;
    uint8_t data[2048] __attribute__((aligned (2048)));
} accel_dev_cmd_regs_t;

static_assert(sizeof(accel_dev_cmd_regs_t) == ACCEL_DEV_PAGE_SIZE);
static_assert((offsetof(accel_dev_cmd_regs_t, cmd) % 4) == 0);
static_assert(sizeof(union dev_cmd) == 64);
static_assert((offsetof(accel_dev_cmd_regs_t, cpl) % 4) == 0);
static_assert(sizeof(union dev_cmd_cpl) == 16);
static_assert((offsetof(accel_dev_cmd_regs_t, data) % 4) == 0);

#endif //__ACCEL_IF_HPP__

