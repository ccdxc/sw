/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
 */

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <endian.h>
#include <sstream>
#include <string>
#include <sys/time.h>

// Tell virtio_dev.hpp to emumerate definitions of all devcmds
#define VIRTIO_DEV_CMD_ENUMERATE  1

#include "nic/sdk/platform/misc/include/misc.h"
#include "nic/sdk/platform/intrutils/include/intrutils.h"
#include "nic/sdk/platform/pciemgr_if/include/pciemgr_if.hpp"
#include "nic/asm/eth/virtio_defines.h"
#include "nic/sdk/platform/fru/fru.hpp"
#include "nic/include/edmaq.h"

#include "logger.hpp"
#include "nicmgr_utils.hpp"
#include "virtio_dev.hpp"
#include "virtio_lif.hpp"
#include "pd_client.hpp"
#include "adminq.hpp"

using namespace std;

#define GBPS_TO_BYTES_PER_SEC(gbps)                     \
    ((uint64_t)(gbps) * (1000000000ULL / 8))

#define PCI_VENDOR_ID_PENSANDO          0x1dd8

// Amount of time to wait for sequencer queues to be quiesced
#define VIRTIO_DEV_SEQ_QUEUES_QUIESCE_TIME_US    5000000
#define VIRTIO_DEV_RING_OP_QUIESCE_TIME_US       1000000
#define VIRTIO_DEV_ALL_RINGS_MAX_QUIESCE_TIME_US (10 * VIRTIO_DEV_RING_OP_QUIESCE_TIME_US)

/*
 * rounded up log2
 */
static uint32_t
log_2(uint32_t x)
{
  uint32_t log = 0;

  while ((uint64_t)(1 << log) < (uint64_t)x) {
      log++;
  }
  return log;
}

static inline uint64_t
timestamp(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000 + tv.tv_usec);
}

static inline void
time_expiry_set(virtio_timestamp_t& ts,
                uint64_t expiry)
{
    ts.timestamp = timestamp();
    ts.expiry = expiry;
}

static inline bool
time_expiry_check(const virtio_timestamp_t& ts)
{
    return (ts.expiry == 0) ||
           ((timestamp() - ts.timestamp) > ts.expiry);
}

static const char              *lif_state_str_table[] = {
    VIRTIO_LIF_STATE_STR_TABLE
};

static const char              *lif_event_str_table[] = {
    VIRTIO_LIF_EVENT_STR_TABLE
};

static virtio_lif_state_event_t  lif_initial_ev_table[] = {
    {
        VIRTIO_LIF_EV_ANY,
        &VirtIOLif::virtio_lif_reject_action,
        VIRTIO_LIF_ST_SAME,
    },
    {
        VIRTIO_LIF_EV_CREATE,
        &VirtIOLif::virtio_lif_create_action,
        VIRTIO_LIF_ST_WAIT_HAL,
    },
    {
        VIRTIO_LIF_EV_DESTROY,
        &VirtIOLif::virtio_lif_null_action,
        VIRTIO_LIF_ST_SAME,
    },
    {
        VIRTIO_LIF_EV_NULL
    },
};

static virtio_lif_state_event_t  lif_wait_hal_ev_table[] = {
    {
        VIRTIO_LIF_EV_ANY,
        &VirtIOLif::virtio_lif_eagain_action,
        VIRTIO_LIF_ST_SAME,
    },
    {
        VIRTIO_LIF_EV_HAL_UP,
        &VirtIOLif::virtio_lif_hal_up_action,
        VIRTIO_LIF_ST_PRE_INIT,
    },
    {
        VIRTIO_LIF_EV_DESTROY,
        &VirtIOLif::virtio_lif_null_action,
        VIRTIO_LIF_ST_SAME,
    },
    {
        VIRTIO_LIF_EV_NULL
    },
};

static virtio_lif_state_event_t  lif_pre_init_ev_table[] = {
    {
        VIRTIO_LIF_EV_ANY,
        &VirtIOLif::virtio_lif_reject_action,
        VIRTIO_LIF_ST_SAME,
    },
    {
        VIRTIO_LIF_EV_INIT,
        &VirtIOLif::virtio_lif_init_action,
        VIRTIO_LIF_ST_POST_INIT,
    },
    {
        VIRTIO_LIF_EV_DESTROY,
        &VirtIOLif::virtio_lif_null_action,
        VIRTIO_LIF_ST_SAME,
    },
    {
        VIRTIO_LIF_EV_NULL
    },
};

static virtio_lif_state_event_t  lif_post_init_ev_table[] = {
    {
        VIRTIO_LIF_EV_ANY,
        &VirtIOLif::virtio_lif_reject_action,
        VIRTIO_LIF_ST_SAME,
    },
    {
        VIRTIO_LIF_EV_DESTROY,
        &VirtIOLif::virtio_lif_destroy_action,
        VIRTIO_LIF_ST_SAME,
    },
    {
        VIRTIO_LIF_EV_NULL
    },
};

static virtio_lif_state_event_t  *lif_fsm_table[VIRTIO_LIF_ST_MAX] = {
    [VIRTIO_LIF_ST_INITIAL]              = lif_initial_ev_table,
    [VIRTIO_LIF_ST_WAIT_HAL]             = lif_wait_hal_ev_table,
    [VIRTIO_LIF_ST_PRE_INIT]             = lif_pre_init_ev_table,
    [VIRTIO_LIF_ST_POST_INIT]            = lif_post_init_ev_table,
};

static virtio_lif_ordered_event_t lif_ordered_ev_table[VIRTIO_LIF_ST_MAX][VIRTIO_LIF_EV_MAX];


static void virtio_lif_state_machine_build(void);
static const char *lif_state_str(virtio_lif_state_t state);
static const char *lif_event_str(virtio_lif_event_t event);

#define VIRTIO_LIF_FSM_LOG()                                                     \
    NIC_LOG_DEBUG("{}: state {} event {}: ",                                    \
                  LifNameGet(),                                                 \
                  lif_state_str(fsm_ctx.enter_state), lif_event_str(event))

#if 0
static bool virtio_lif_fsm_verbose;

#define VIRTIO_LIF_FSM_VERBOSE_LOG()                                             \
    if (virtio_lif_fsm_verbose) VIRTIO_LIF_FSM_LOG()
#endif

#define VIRTIO_LIF_FSM_ERR_LOG()                                                 \
    NIC_LOG_ERR("{}: state {} invalid event {}: ",                              \
                LifNameGet(),                                                   \
                lif_state_str(fsm_ctx.enter_state), lif_event_str(event))

VirtIOLif::VirtIOLif(VirtIODev& virtio_dev,
                   virtio_lif_res_t& lif_res,
                   EV_P) :
    virtio_dev(virtio_dev),
    spec(virtio_dev.DevSpecGet()),
    pd(virtio_dev.PdClientGet()),
    dev_api(virtio_dev.DevApiGet())
{
    this->loop = loop;
    virtio_lif_state_machine_build();

    NIC_HEADER_TRACE("Adding VIRTIO LIF");

    memset(&hal_lif_info_, 0, sizeof(hal_lif_info_));
    hal_lif_info_.lif_id = lif_res.lif_id;
    hal_lif_info_.type = spec->lif_type;
    hal_lif_info_.pinned_uplink_port_num = spec->uplink_port_num;

    lif_name = spec->name + std::string("/lif") +
               std::to_string(hal_lif_info_.lif_id);
    strncpy0(hal_lif_info_.name, lif_name.c_str(), sizeof(hal_lif_info_.name));
    intr_base = lif_res.intr_base;

    memset(&fsm_ctx, 0, sizeof(fsm_ctx));
    fsm_ctx.state = VIRTIO_LIF_ST_INITIAL;
    virtio_lif_state_machine(VIRTIO_LIF_EV_CREATE);
}

VirtIOLif::~VirtIOLif()
{
    /*
     * Host driver would have already performed graceful reset-destroy, in
     * which case, the following FSM event would result in very quick work,
     * i.e., no delay.
     */
    fsm_ctx.devcmd.status = VIRTIO_RC_EAGAIN;
    while (fsm_ctx.devcmd.status == VIRTIO_RC_EAGAIN) {
        virtio_lif_state_machine(VIRTIO_LIF_EV_DESTROY);
    }
}

void
VirtIOLif::SetHalClient(devapi *dapi)
{
    dev_api = dapi;
}

void
VirtIOLif::HalEventHandler(bool status)
{
    if (status) {
        virtio_lif_state_machine(VIRTIO_LIF_EV_HAL_UP);
    }
}

virtio_status_code_t
VirtIOLif::Enable(virtio_dev_cmd_regs_t *regs_p)
{
    fsm_ctx.devcmd.req = regs_p;
    virtio_lif_state_machine(VIRTIO_LIF_EV_INIT);
    return fsm_ctx.devcmd.status;
}

virtio_status_code_t
VirtIOLif::Disable(virtio_dev_cmd_regs_t *regs_p)
{
    fsm_ctx.devcmd.status = VIRTIO_RC_SUCCESS;
    return fsm_ctx.devcmd.status;
}

uint64_t
VirtIOLif::GetQstateAddr(int qtype, int qid)
{
    return pd->lm_->get_lif_qstate_addr(LifIdGet(), qtype, qid);
}

void
VirtIOLif::ResetRxQstate(int rx_qid)
{
    uint64_t addr = GetQstateAddr(VIRTIO_QTYPE_RX, rx_qid);
    virtio_qstate_rx_t rxq = { 0 };

    // disable promiscuous stuff
    dev_api->lif_upd_rx_mode(hal_lif_info_.lif_id, false, false, false);

    // clear rx queue state
    WRITE_MEM(addr, (uint8_t *)&rxq, sizeof(rxq), 0);

    PAL_barrier();
    p4plus_invalidate_cache(addr, sizeof(rxq), P4PLUS_CACHE_INVALIDATE_RXDMA);
}

void
VirtIOLif::SetupRxQstate(int rx_qid,
                         uint64_t features,
                         uint64_t desc_addr,
                         uint64_t avail_addr,
                         uint64_t used_addr,
                         uint16_t intr,
                         uint16_t size)
{
    uint64_t addr = GetQstateAddr(VIRTIO_QTYPE_RX, rx_qid);
    virtio_qstate_rx_t rxq = { 0 };

    rxq.qs.pc_offset = pc_rx;
    rxq.qs.cosA = cosA;
    rxq.qs.cosB = cosB;
    rxq.qs.host = 1;
    rxq.qs.total = VIRTIO_RX_RINGS;
    rxq.features = features;
    rxq.rx_virtq_desc_addr = desc_addr | VIRTIO_PHYS_ADDR_HOST_LIF_SET(1, LifIdGet());
    rxq.rx_virtq_avail_addr = avail_addr | VIRTIO_PHYS_ADDR_HOST_LIF_SET(1, LifIdGet());
    rxq.rx_virtq_used_addr = used_addr | VIRTIO_PHYS_ADDR_HOST_LIF_SET(1, LifIdGet());
    rxq.rx_intr_assert_addr = intr_assert_addr(intr_base + intr);
    rxq.rx_queue_size_mask = size - 1;

    NIC_LOG_DEBUG("{}: setup RXQ{} cb {:#x} features {:#x} desc_addr {:#x} avail_addr {:#x} used_addr {:#x} intr_addr {} size_mask {}",
                  LifNameGet(), rx_qid, addr,
                  rxq.features,
                  rxq.rx_virtq_desc_addr,
                  rxq.rx_virtq_avail_addr,
                  rxq.rx_virtq_used_addr,
                  rxq.rx_intr_assert_addr,
                  rxq.rx_queue_size_mask);

    WRITE_MEM(addr, (uint8_t *)&rxq, sizeof(rxq), 0);

    PAL_barrier();
    p4plus_invalidate_cache(addr, sizeof(rxq), P4PLUS_CACHE_INVALIDATE_RXDMA);

    // enable promoscuous mode (and broadcast, allmulti)
    dev_api->lif_upd_rx_mode(hal_lif_info_.lif_id, true, true, true);
}

void
VirtIOLif::ResetTxQstate(int tx_qid)
{
    uint64_t addr = GetQstateAddr(VIRTIO_QTYPE_TX, tx_qid);
    virtio_qstate_tx_t txq = { 0 };

    WRITE_MEM(addr, (uint8_t *)&txq, sizeof(txq), 0);

    PAL_barrier();
    p4plus_invalidate_cache(addr, sizeof(txq), P4PLUS_CACHE_INVALIDATE_TXDMA);
}

void
VirtIOLif::SetupTxQstate(int tx_qid,
                         uint64_t features,
                         uint64_t desc_addr,
                         uint64_t avail_addr,
                         uint64_t used_addr,
                         uint16_t intr,
                         uint16_t size)
{
    uint64_t addr = GetQstateAddr(VIRTIO_QTYPE_TX, tx_qid);
    virtio_qstate_tx_t txq = { 0 };

    txq.qs.pc_offset = pc_tx;
    txq.qs.cosA = cosA;
    txq.qs.cosB = cosB;
    txq.qs.host = 1;
    txq.qs.total = VIRTIO_TX_RINGS;
    txq.features = features;
    txq.tx_virtq_desc_addr = desc_addr | VIRTIO_PHYS_ADDR_HOST_LIF_SET(1, LifIdGet());
    txq.tx_virtq_avail_addr = avail_addr | VIRTIO_PHYS_ADDR_HOST_LIF_SET(1, LifIdGet());
    txq.tx_virtq_used_addr = used_addr | VIRTIO_PHYS_ADDR_HOST_LIF_SET(1, LifIdGet());
    txq.tx_intr_assert_addr = intr_assert_addr(intr_base + intr);
    txq.tx_queue_size_mask = size - 1;

    NIC_LOG_DEBUG("{}: setup TXQ{} cb {:#x} features {:#x} desc_addr {:#x} avail_addr {:#x} used_addr {:#x} intr_addr {} size_mask {}",
                  LifNameGet(), tx_qid, addr,
                  txq.features,
                  txq.tx_virtq_desc_addr,
                  txq.tx_virtq_avail_addr,
                  txq.tx_virtq_used_addr,
                  txq.tx_intr_assert_addr,
                  txq.tx_queue_size_mask);

    WRITE_MEM(addr, (uint8_t *)&txq, sizeof(txq), 0);

    PAL_barrier();
    p4plus_invalidate_cache(addr, sizeof(txq), P4PLUS_CACHE_INVALIDATE_TXDMA);
}

void
VirtIOLif::NotifyTxQueue(int tx_qid)
{
    uint64_t addr = VIRTIO_LIF_LOCAL_DBADDR_SET(LifIdGet(), VIRTIO_QTYPE_TX);
    uint64_t data = VIRTIO_LIF_LOCAL_DBDATA_SET(tx_qid, 0, 0);

    NIC_LOG_DEBUG("{}: TXQ{} doorbell addr {:#x} data {:#x}",
                  LifNameGet(), tx_qid, addr, data);

    PAL_barrier();
    WRITE_DB64(addr, data);
}

/*
 * LIF State Machine Actions
 */
virtio_lif_event_t
VirtIOLif::virtio_lif_null_action(virtio_lif_event_t event)
{
    VIRTIO_LIF_FSM_LOG();
    fsm_ctx.devcmd.status = VIRTIO_RC_SUCCESS;
    return VIRTIO_LIF_EV_NULL;
}

virtio_lif_event_t
VirtIOLif::virtio_lif_eagain_action(virtio_lif_event_t event)
{
    VIRTIO_LIF_FSM_LOG();
    fsm_ctx.devcmd.status = VIRTIO_RC_EAGAIN;
    return VIRTIO_LIF_EV_NULL;
}

virtio_lif_event_t
VirtIOLif::virtio_lif_reject_action(virtio_lif_event_t event)
{
    VIRTIO_LIF_FSM_ERR_LOG();
    fsm_ctx.devcmd.status = VIRTIO_RC_EPERM;
    return VIRTIO_LIF_EV_NULL;
}

virtio_lif_event_t
VirtIOLif::virtio_lif_create_action(virtio_lif_event_t event)
{
    VIRTIO_LIF_FSM_LOG();
    memset(qinfo, 0, sizeof(qinfo));

    qinfo[VIRTIO_QTYPE_RX] = {
        .type_num = VIRTIO_QTYPE_RX,
        .size = HW_CB_MULTIPLE(VIRTIO_RX_CB_SIZE_SHIFT),
        .entries = log_2(spec->txrx_count),
    };

    qinfo[VIRTIO_QTYPE_TX] = {
        .type_num = VIRTIO_QTYPE_TX,
        .size = HW_CB_MULTIPLE(VIRTIO_TX_CB_SIZE_SHIFT),
        .entries = log_2(spec->txrx_count),
    };

    memcpy(hal_lif_info_.queue_info, qinfo, sizeof(hal_lif_info_.queue_info));

    return VIRTIO_LIF_EV_NULL;
}

virtio_lif_event_t
VirtIOLif::virtio_lif_destroy_action(virtio_lif_event_t event)
{
    VIRTIO_LIF_FSM_LOG();
    fsm_ctx.devcmd.status = VIRTIO_RC_SUCCESS;
    return VIRTIO_LIF_EV_NULL;
}

virtio_lif_event_t
VirtIOLif::virtio_lif_hal_up_action(virtio_lif_event_t event)
{
    VIRTIO_LIF_FSM_LOG();

    // Resolve class of service configuration
    cosA = 0;
    cosB = 0;
    admin_cosA = 1;
    admin_cosB = 1;

    // Resolve the data path programs PC
    if (pd->get_pc_offset("rxdma_stage0.bin", "virtio_rx_stage0", &pc_rx, NULL) < 0) {
        NIC_LOG_ERR("{}: Failed to get PC offset of program: {} label: {}", LifNameGet(), "rxdma_stage0.bin", "virtio_rx_stage0");
        fsm_ctx.devcmd.status = VIRTIO_RC_ERROR;
    }
    if (pd->get_pc_offset("txdma_stage0.bin", "virtio_tx_stage0", &pc_tx, NULL) < 0) {
        NIC_LOG_ERR("{}: Failed to get PC offset of program: {} label: {}", LifNameGet(), "txdma_stage0.bin", "virtio_tx_stage0");
        fsm_ctx.devcmd.status = VIRTIO_RC_ERROR;
    }
    NIC_LOG_DEBUG("{}: PC {} {} {} {}", LifNameGet(), "virtio_rx_stage0", pc_rx, "virtio_tx_stage0", pc_tx);

    // Proceed with LIF INIT right away
    return VIRTIO_LIF_EV_INIT;
}

virtio_lif_event_t
VirtIOLif::virtio_lif_init_action(virtio_lif_event_t event)
{
    VIRTIO_LIF_FSM_LOG();

    if (dev_api->lif_create(&hal_lif_info_) != SDK_RET_OK) {
        NIC_LOG_ERR("{}: Failed to create LIF", LifNameGet());
        fsm_ctx.devcmd.status = VIRTIO_RC_ERROR;
    }
    if (dev_api->lif_init(&hal_lif_info_) != SDK_RET_OK) {
        NIC_LOG_ERR("{}: Failed to init LIF", LifNameGet());
        fsm_ctx.devcmd.status = VIRTIO_RC_ERROR;
    }

    // program the queue state
    pd->program_qstate((struct queue_info*)hal_lif_info_.queue_info,
                       &hal_lif_info_, 0x0);

    return VIRTIO_LIF_EV_NULL;
}

void
VirtIOLif::virtio_lif_state_machine(virtio_lif_event_t event)
{
    virtio_lif_ordered_event_t   *ordered_event;
    virtio_lif_action_t          action;

    while (event != VIRTIO_LIF_EV_NULL) {

        if ((fsm_ctx.state < VIRTIO_LIF_ST_MAX) &&
            (event < VIRTIO_LIF_EV_MAX)) {

            ordered_event = &lif_ordered_ev_table[fsm_ctx.state][event];
            fsm_ctx.enter_state = fsm_ctx.state;
            if (ordered_event->next_state != VIRTIO_LIF_ST_SAME) {
                fsm_ctx.state = ordered_event->next_state;
            }
            action = ordered_event->action;
            if (!action) {
                NIC_LOG_ERR("Null action for state {} event {}",
                            lif_state_str(fsm_ctx.enter_state),
                            lif_event_str(event));
                throw;
            }
            event = (this->*action)(event);

        } else {
            NIC_LOG_ERR("Unknown state {} or event {}",
                        fsm_ctx.state, event);
            throw;
        }
    }
}

static void
virtio_lif_state_machine_build(void)
{
    virtio_lif_state_event_t    **fsm_entry;
    virtio_lif_state_event_t    *state_event;
    virtio_lif_state_event_t    *any_event;
    virtio_lif_ordered_event_t  *ordered_event;
    uint32_t                   state;

    static bool lif_ordered_event_table_built;
    if (lif_ordered_event_table_built) {
        return;
    }
    lif_ordered_event_table_built = true;

    for (fsm_entry = &lif_fsm_table[0], state = 0;
         fsm_entry < &lif_fsm_table[VIRTIO_LIF_ST_MAX];
         fsm_entry++, state++) {

        state_event = *fsm_entry;
        if (state_event) {
            any_event = nullptr;
            while (state_event->event != VIRTIO_LIF_EV_NULL) {
                if (state_event->event < VIRTIO_LIF_EV_MAX) {
                    ordered_event = &lif_ordered_ev_table[state]
                                                         [state_event->event];

                    ordered_event->action = state_event->action;
                    ordered_event->next_state = state_event->next_state;

                    if (state_event->event == VIRTIO_LIF_EV_ANY) {
                        any_event = state_event;
                    }

                } else {
                    NIC_LOG_ERR("Unknown event {} for state {}", state_event->event,
                                lif_state_str((virtio_lif_state_t)state));
                    throw;
                }
                state_event++;
            }

            if (!any_event) {
                NIC_LOG_ERR("Missing 'any' event for state {}",
                            lif_state_str((virtio_lif_state_t)state));
                throw;
            }

            for (ordered_event = &lif_ordered_ev_table[state][0];
                 ordered_event < &lif_ordered_ev_table[state][VIRTIO_LIF_EV_MAX];
                 ordered_event++) {

                if (!ordered_event->action) {
                    ordered_event->action  = any_event->action;
                    ordered_event->next_state = any_event->next_state;
                }
            }
        }
    }
}

static const char *
lif_state_str(virtio_lif_state_t state)
{
    if (state < VIRTIO_LIF_ST_MAX) {
        return lif_state_str_table[state];
    }
    return "unknown_state";
}

static const char *
lif_event_str(virtio_lif_event_t event)
{
    if (event < VIRTIO_LIF_EV_MAX) {
        return lif_event_str_table[event];
    }
    return "unknown_event";
}

