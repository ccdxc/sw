/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <endian.h>
#include <sstream>
#include <sys/time.h>

#include "nic/include/base.hpp"
#include "nic/sdk/platform/misc/include/misc.h"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "nic/sdk/asic/pd/pd.hpp"

#include "logger.hpp"
#include "ftl_dev.hpp"
#include "ftl_lif.hpp"
#include "pd_client.hpp"

/*
 * Amount of time to wait for scanner queues to be quiesced
 */
#define FTL_LIF_SCANNERS_QUIESCE_TIME_US            (5 * USEC_PER_SEC)

static uint64_t                 hw_coreclk_freq;
static double                   hw_ns_per_tick;

static inline uint64_t
hw_coreclk_ticks_to_time_ns(uint64_t ticks)
{
    return (uint64_t)(hw_ns_per_tick * (double)ticks);
}

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

static const char               *lif_state_str_table[] = {
    FTL_LIF_STATE_STR_TABLE
};

static const char               *lif_event_str_table[] = {
    FTL_LIF_EVENT_STR_TABLE
};

ftl_lif_state_event_t           FtlLif::lif_initial_ev_table[] = {
    {
        FTL_LIF_EV_ANY,
        &FtlLif::ftl_lif_reject_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_CREATE,
        &FtlLif::ftl_lif_create_action,
        FTL_LIF_ST_WAIT_HAL,
    },
    {
        FTL_LIF_EV_DESTROY,
        &FtlLif::ftl_lif_null_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_NULL
    },
};

ftl_lif_state_event_t           FtlLif::lif_wait_hal_ev_table[] = {
    {
        FTL_LIF_EV_ANY,
        &FtlLif::ftl_lif_eagain_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_HAL_UP,
        &FtlLif::ftl_lif_hal_up_action,
        FTL_LIF_ST_PRE_INIT,
    },
    {
        FTL_LIF_EV_DESTROY,
        &FtlLif::ftl_lif_null_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_NULL
    },
};

ftl_lif_state_event_t           FtlLif::lif_pre_init_ev_table[] = {
    {
        FTL_LIF_EV_ANY,
        &FtlLif::ftl_lif_reject_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_INIT,
        &FtlLif::ftl_lif_init_action,
        FTL_LIF_ST_POST_INIT,
    },
    {
        FTL_LIF_EV_IDENTIFY,
        &FtlLif::ftl_lif_identify_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_RESET,
        &FtlLif::ftl_lif_null_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_RESET_DESTROY,
        &FtlLif::ftl_lif_null_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_DESTROY,
        &FtlLif::ftl_lif_null_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_NULL
    },
};

ftl_lif_state_event_t           FtlLif::lif_post_init_ev_table[] = {
    {
        FTL_LIF_EV_ANY,
        &FtlLif::ftl_lif_reject_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_IDENTIFY,
        &FtlLif::ftl_lif_identify_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_SETATTR,
        &FtlLif::ftl_lif_setattr_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_GETATTR,
        &FtlLif::ftl_lif_getattr_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_ACCEL_AGING_CONTROL,
        &FtlLif::ftl_lif_accel_aging_ctl_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_RESET,
        &FtlLif::ftl_lif_reset_action,
        FTL_LIF_ST_QUEUES_RESET,
    },
    {
        FTL_LIF_EV_RESET_DESTROY,
        &FtlLif::ftl_lif_reset_action,
        FTL_LIF_ST_QUEUES_RESET,
    },
    {
        FTL_LIF_EV_DESTROY,
        &FtlLif::ftl_lif_destroy_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_NULL
    },
};

ftl_lif_state_event_t           FtlLif::lif_queues_reset_ev_table[] = {
    {
        FTL_LIF_EV_ANY,
        &FtlLif::ftl_lif_reject_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_GETATTR,
        &FtlLif::ftl_lif_getattr_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_RESET,
        &FtlLif::ftl_lif_scanners_quiesce_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_RESET_DESTROY,
        &FtlLif::ftl_lif_scanners_quiesce_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_DESTROY,
        &FtlLif::ftl_lif_destroy_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_SCANNERS_QUIESCE,
        &FtlLif::ftl_lif_scanners_quiesce_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_QUEUES_STOP_COMPLETE,
        &FtlLif::ftl_lif_null_action,
        FTL_LIF_ST_QUEUES_PRE_INIT,
    },
    {
        FTL_LIF_EV_NULL
    },
};

ftl_lif_state_event_t           FtlLif::lif_queues_pre_init_ev_table[] = {
    {
        FTL_LIF_EV_ANY,
        &FtlLif::ftl_lif_reject_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_RESET,
        &FtlLif::ftl_lif_null_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_RESET_DESTROY,
        &FtlLif::ftl_lif_null_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_DESTROY,
        &FtlLif::ftl_lif_destroy_action,
        FTL_LIF_ST_POST_INIT,
    },
    {
        FTL_LIF_EV_IDENTIFY,
        &FtlLif::ftl_lif_identify_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_SETATTR,
        &FtlLif::ftl_lif_setattr_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_GETATTR,
        &FtlLif::ftl_lif_getattr_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_ACCEL_AGING_CONTROL,
        &FtlLif::ftl_lif_accel_aging_ctl_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_POLLERS_INIT,
        &FtlLif::ftl_lif_pollers_init_action,
        FTL_LIF_ST_QUEUES_INIT_TRANSITION,
    },
    {
        FTL_LIF_EV_NULL
    },
};

ftl_lif_state_event_t           FtlLif::lif_queues_init_transition_ev_table[] = {
    {
        FTL_LIF_EV_ANY,
        &FtlLif::ftl_lif_reject_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_RESET,
        &FtlLif::ftl_lif_reset_action,
        FTL_LIF_ST_QUEUES_RESET,
    },
    {
        FTL_LIF_EV_SETATTR,
        &FtlLif::ftl_lif_setattr_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_GETATTR,
        &FtlLif::ftl_lif_getattr_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_ACCEL_AGING_CONTROL,
        &FtlLif::ftl_lif_accel_aging_ctl_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_RESET_DESTROY,
        &FtlLif::ftl_lif_reset_action,
        FTL_LIF_ST_QUEUES_RESET,
    },
    {
        FTL_LIF_EV_DESTROY,
        &FtlLif::ftl_lif_destroy_action,
        FTL_LIF_ST_POST_INIT,
    },
    {
        FTL_LIF_EV_SCANNERS_INIT,
        &FtlLif::ftl_lif_scanners_init_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_SCANNERS_START,
        &FtlLif::ftl_lif_scanners_start_action,
        FTL_LIF_ST_QUEUES_STARTED,
    },
    {
        FTL_LIF_EV_SCANNERS_START_SINGLE,
        &FtlLif::ftl_lif_null_no_log_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_SCANNERS_STOP,
        &FtlLif::ftl_lif_null_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_POLLERS_DEQ_BURST,
        &FtlLif::ftl_lif_pollers_deq_burst_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_POLLERS_FLUSH,
        &FtlLif::ftl_lif_pollers_flush_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_NULL
    },
};

ftl_lif_state_event_t           FtlLif::lif_queues_stopping_ev_table[] = {
    {
        FTL_LIF_EV_ANY,
        &FtlLif::ftl_lif_reject_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_RESET,
        &FtlLif::ftl_lif_reset_action,
        FTL_LIF_ST_QUEUES_RESET,
    },
    {
        FTL_LIF_EV_SETATTR,
        &FtlLif::ftl_lif_setattr_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_GETATTR,
        &FtlLif::ftl_lif_getattr_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_ACCEL_AGING_CONTROL,
        &FtlLif::ftl_lif_accel_aging_ctl_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_RESET_DESTROY,
        &FtlLif::ftl_lif_reset_action,
        FTL_LIF_ST_QUEUES_RESET,
    },
    {
        FTL_LIF_EV_DESTROY,
        &FtlLif::ftl_lif_destroy_action,
        FTL_LIF_ST_POST_INIT,
    },
    {
        FTL_LIF_EV_SCANNERS_STOP,
        &FtlLif::ftl_lif_scanners_quiesce_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_SCANNERS_QUIESCE,
        &FtlLif::ftl_lif_scanners_quiesce_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_QUEUES_STOP_COMPLETE,
        &FtlLif::ftl_lif_null_action,
        FTL_LIF_ST_QUEUES_INIT_TRANSITION,
    },
    {
        FTL_LIF_EV_SCANNERS_START_SINGLE,
        &FtlLif::ftl_lif_null_no_log_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_POLLERS_DEQ_BURST,
        &FtlLif::ftl_lif_pollers_deq_burst_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_POLLERS_FLUSH,
        &FtlLif::ftl_lif_pollers_flush_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_NULL
    },
};

ftl_lif_state_event_t           FtlLif::lif_queues_started_ev_table[] = {
    {
        FTL_LIF_EV_ANY,
        &FtlLif::ftl_lif_reject_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_RESET,
        &FtlLif::ftl_lif_reset_action,
        FTL_LIF_ST_QUEUES_RESET,
    },
    {
        FTL_LIF_EV_SETATTR,
        &FtlLif::ftl_lif_setattr_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_GETATTR,
        &FtlLif::ftl_lif_getattr_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_ACCEL_AGING_CONTROL,
        &FtlLif::ftl_lif_accel_aging_ctl_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_RESET_DESTROY,
        &FtlLif::ftl_lif_reset_action,
        FTL_LIF_ST_QUEUES_RESET,
    },
    {
        FTL_LIF_EV_DESTROY,
        &FtlLif::ftl_lif_destroy_action,
        FTL_LIF_ST_POST_INIT,
    },
    {
        FTL_LIF_EV_SCANNERS_START,
        &FtlLif::ftl_lif_null_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_SCANNERS_STOP,
        &FtlLif::ftl_lif_scanners_stop_action,
        FTL_LIF_ST_QUEUES_STOPPING,
    },
    {
        FTL_LIF_EV_SCANNERS_START_SINGLE,
        &FtlLif::ftl_lif_scanners_start_single_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_POLLERS_FLUSH,
        &FtlLif::ftl_lif_pollers_flush_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_POLLERS_DEQ_BURST,
        &FtlLif::ftl_lif_pollers_deq_burst_action,
        FTL_LIF_ST_SAME,
    },
    {
        FTL_LIF_EV_NULL
    },
};

static ftl_lif_state_event_t  *lif_fsm_table[FTL_LIF_ST_MAX] = {
    [FTL_LIF_ST_INITIAL]                = FtlLif::lif_initial_ev_table,
    [FTL_LIF_ST_WAIT_HAL]               = FtlLif::lif_wait_hal_ev_table,
    [FTL_LIF_ST_PRE_INIT]               = FtlLif::lif_pre_init_ev_table,
    [FTL_LIF_ST_POST_INIT]              = FtlLif::lif_post_init_ev_table,
    [FTL_LIF_ST_QUEUES_RESET]           = FtlLif::lif_queues_reset_ev_table,
    [FTL_LIF_ST_QUEUES_PRE_INIT]        = FtlLif::lif_queues_pre_init_ev_table,
    [FTL_LIF_ST_QUEUES_INIT_TRANSITION] = FtlLif::lif_queues_init_transition_ev_table,
    [FTL_LIF_ST_QUEUES_STOPPING]        = FtlLif::lif_queues_stopping_ev_table,
    [FTL_LIF_ST_QUEUES_STARTED]         = FtlLif::lif_queues_started_ev_table,
};

static ftl_lif_ordered_event_t lif_ordered_ev_table[FTL_LIF_ST_MAX][FTL_LIF_EV_MAX];

static bool ftl_lif_fsm_verbose;

static void ftl_lif_state_machine_build(void);
static void poller_cb_activate(const mem_access_t *access);
static void poller_cb_deactivate(const mem_access_t *access);
static void scanner_session_cb_activate(const mem_access_t *access);
static void scanner_session_cb_deactivate(const mem_access_t *access);
static const char *lif_state_str(ftl_lif_state_t state);
static const char *lif_event_str(ftl_lif_event_t event);

/*
 * devcmd opcodes to state machine events
 */
typedef std::map<uint32_t,ftl_lif_event_t> opcode2event_map_t;

static const opcode2event_map_t opcode2event_map = {
    {FTL_DEVCMD_OPCODE_NOP,                    FTL_LIF_EV_NULL},
    {FTL_DEVCMD_OPCODE_LIF_IDENTIFY,           FTL_LIF_EV_IDENTIFY},
    {FTL_DEVCMD_OPCODE_LIF_INIT,               FTL_LIF_EV_INIT},
    {FTL_DEVCMD_OPCODE_LIF_SETATTR,            FTL_LIF_EV_SETATTR},
    {FTL_DEVCMD_OPCODE_LIF_GETATTR,            FTL_LIF_EV_GETATTR},
    {FTL_DEVCMD_OPCODE_LIF_RESET,              FTL_LIF_EV_RESET},
    {FTL_DEVCMD_OPCODE_POLLERS_INIT,           FTL_LIF_EV_POLLERS_INIT},
    {FTL_DEVCMD_OPCODE_POLLERS_DEQ_BURST,      FTL_LIF_EV_POLLERS_DEQ_BURST},
    {FTL_DEVCMD_OPCODE_POLLERS_FLUSH,          FTL_LIF_EV_POLLERS_FLUSH},
    {FTL_DEVCMD_OPCODE_SCANNERS_INIT,          FTL_LIF_EV_SCANNERS_INIT},
    {FTL_DEVCMD_OPCODE_SCANNERS_START,         FTL_LIF_EV_SCANNERS_START},
    {FTL_DEVCMD_OPCODE_SCANNERS_START_SINGLE,  FTL_LIF_EV_SCANNERS_START_SINGLE},
    {FTL_DEVCMD_OPCODE_SCANNERS_STOP,          FTL_LIF_EV_SCANNERS_STOP},
    {FTL_DEVCMD_OPCODE_ACCEL_AGING_CONTROL,    FTL_LIF_EV_ACCEL_AGING_CONTROL},
};

/*
 * Devapi availability check
 */
#define FTL_LIF_DEVAPI_CHECK(devcmd_status, ret_val)                            \
    if (!dev_api) {                                                             \
        NIC_LOG_ERR("{}: Uninitialized devapi", LifNameGet());                  \
        if (devcmd_status) devcmd_ctx.status = devcmd_status;                   \
        return ret_val;                                                         \
    }

#define FTL_LIF_DEVAPI_CHECK_VOID(devcmd_status)                                \
    if (!dev_api) {                                                             \
        NIC_LOG_ERR("{}: Uninitialized devapi", LifNameGet());                  \
        if (devcmd_status) devcmd_ctx.status = devcmd_status;                   \
        return;                                                                 \
    }

#define FTL_LIF_FSM_LOG()                                                       \
    NIC_LOG_DEBUG("{}: state {} event {}: ",                                    \
                  LifNameGet(),                                                 \
                  lif_state_str(fsm_ctx.enter_state), lif_event_str(event))

#define FTL_LIF_FSM_VERBOSE_LOG()                                               \
    if (ftl_lif_fsm_verbose) FTL_LIF_FSM_LOG()

#define FTL_LIF_FSM_ERR_LOG()                                                   \
    NIC_LOG_ERR("{}: state {} invalid event {}: ",                              \
                LifNameGet(),                                                   \
                lif_state_str(fsm_ctx.enter_state), lif_event_str(event))


FtlLif::FtlLif(FtlDev& ftl_dev,
               ftl_lif_res_t& lif_res,
               EV_P) :
    ftl_dev(ftl_dev),
    spec(ftl_dev.DevSpecGet()),
    session_scanners_ctl(*this, FTL_QTYPE_SCANNER_SESSION,
                         spec->session_hw_scanners),
    conntrack_scanners_ctl(*this, FTL_QTYPE_SCANNER_CONNTRACK,
                           spec->conntrack_hw_scanners),
    pollers_ctl(*this, FTL_QTYPE_POLLER, spec->sw_pollers),
    pd(ftl_dev.PdClientGet()),
    dev_api(ftl_dev.DevApiGet()),
    normal_age_access_(*this),
    accel_age_access_(*this),
    EV_A(EV_A)
{
    uint64_t    cmb_age_tmo_addr;
    uint32_t    cmb_age_tmo_size;
    ftl_lif_devcmd_ctx_t    devcmd_ctx;

    ftl_lif_state_machine_build();

    memset(&hal_lif_info_, 0, sizeof(hal_lif_info_));
    hal_lif_info_.lif_id = lif_res.lif_id;
    lif_name = spec->name + std::string("/lif") +
               std::to_string(hal_lif_info_.lif_id);
    strncpy0(hal_lif_info_.name, lif_name.c_str(), sizeof(hal_lif_info_.name));

    /*
     * If multiple LIFs are required in the future, make an
     * allocator for allocating PAL memory for age timeout CBs below.
     */
    cmb_age_tmo_addr = pd->mp_->start_addr(FTL_DEV_AGE_TIMEOUTS_HBM_HANDLE);
    cmb_age_tmo_size = pd->mp_->size(FTL_DEV_AGE_TIMEOUTS_HBM_HANDLE);
    NIC_LOG_DEBUG("{}: cmb_age_tmo_addr: {:#x} cmb_age_tmo_size: {}",
                  LifNameGet(), cmb_age_tmo_addr, cmb_age_tmo_size);
    /*
     * Need 2 CBs, for normal and accelerated age timeouts.
     */
    if ((cmb_age_tmo_addr == INVALID_MEM_ADDRESS) ||
        (cmb_age_tmo_size < (2 * sizeof(age_tmo_cb_t)))) {

        NIC_LOG_ERR("{}: HBM memory error for {}",
                    LifNameGet(), FTL_DEV_AGE_TIMEOUTS_HBM_HANDLE);
        throw;
    }

    normal_age_access_.reset(cmb_age_tmo_addr, sizeof(age_tmo_cb_t));
    accel_age_access_.reset(cmb_age_tmo_addr + sizeof(age_tmo_cb_t),
                            sizeof(age_tmo_cb_t));
    age_tmo_cb_init(&normal_age_tmo_cb, normal_age_access(), true);
    age_tmo_cb_init(&accel_age_tmo_cb, accel_age_access(), false);

    ftl_lif_state_machine(FTL_LIF_EV_CREATE, devcmd_ctx);
}

FtlLif::~FtlLif()
{
    ftl_lif_devcmd_ctx_t    devcmd_ctx;

    devcmd_ctx.status = FTL_RC_EAGAIN;
    while (devcmd_ctx.status == FTL_RC_EAGAIN) {
        ftl_lif_state_machine(FTL_LIF_EV_DESTROY, devcmd_ctx);
    }
}

void
FtlLif::SetHalClient(devapi *dapi)
{
    dev_api = dapi;
}

void
FtlLif::HalEventHandler(bool status)
{
    ftl_lif_devcmd_ctx_t    devcmd_ctx;

    if (status) {
        ftl_lif_state_machine(FTL_LIF_EV_HAL_UP, devcmd_ctx);
    }
}

ftl_status_code_t
FtlLif::CmdHandler(ftl_devcmd_t *req,
                   void *req_data,
                   ftl_devcmd_cpl_t *rsp,
                   void *rsp_data)
{
    ftl_lif_event_t         event;
    ftl_lif_devcmd_ctx_t    devcmd_ctx;
    bool                    log_cpl = false;

    devcmd_ctx.req = req;
    devcmd_ctx.req_data = req_data;
    devcmd_ctx.rsp = rsp;
    devcmd_ctx.rsp_data = rsp_data;
    devcmd_ctx.status = FTL_RC_SUCCESS;

    /*
     * Short cut for the most frequent "datapath" operations
     */
    switch (req->cmd.opcode) {

    case FTL_DEVCMD_OPCODE_POLLERS_DEQ_BURST:
        ftl_lif_state_machine(FTL_LIF_EV_POLLERS_DEQ_BURST, devcmd_ctx);
        break;

    case FTL_DEVCMD_OPCODE_SCANNERS_START_SINGLE:
        ftl_lif_state_machine(FTL_LIF_EV_SCANNERS_START_SINGLE, devcmd_ctx);
        break;

    case FTL_DEVCMD_OPCODE_ACCEL_AGING_CONTROL:
        ftl_lif_state_machine(FTL_LIF_EV_ACCEL_AGING_CONTROL, devcmd_ctx);
        break;

    default:
        NIC_LOG_DEBUG("{}: Handling cmd: {}", LifNameGet(),
                      ftl_dev_opcode_str(req->cmd.opcode));
        log_cpl = true;

        auto iter = opcode2event_map.find(req->cmd.opcode);
        if (iter != opcode2event_map.end()) {
            event = iter->second;
            if (event != FTL_LIF_EV_NULL) {
                ftl_lif_state_machine(event, devcmd_ctx);
            }
        } else {
            NIC_LOG_ERR("{}: Unknown Opcode {}", LifNameGet(), req->cmd.opcode);
            devcmd_ctx.status = FTL_RC_EOPCODE;
        }
        break;
    }

    rsp->status = devcmd_ctx.status;
    if (log_cpl) {
        NIC_LOG_DEBUG("{}: Done cmd: {}, status: {}", LifNameGet(),
                      ftl_dev_opcode_str(req->cmd.opcode), devcmd_ctx.status);
    }
    return devcmd_ctx.status;
}

ftl_status_code_t
FtlLif::reset(bool destroy)
{
    ftl_lif_devcmd_ctx_t    devcmd_ctx;
    lif_reset_cmd_t         devcmd_reset = {0};

    devcmd_ctx.req = (ftl_devcmd_t *)&devcmd_reset;
    devcmd_reset.quiesce_check = true;
    ftl_lif_state_machine(destroy ? FTL_LIF_EV_RESET_DESTROY :
                                    FTL_LIF_EV_RESET, devcmd_ctx);
    return devcmd_ctx.status;
}

/*
 * LIF State Machine Actions
 */
ftl_lif_event_t
FtlLif::ftl_lif_null_action(ftl_lif_event_t event,
                            ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    FTL_LIF_FSM_LOG();
    devcmd_ctx.status = FTL_RC_SUCCESS;
    return FTL_LIF_EV_NULL;
}

ftl_lif_event_t
FtlLif::ftl_lif_null_no_log_action(ftl_lif_event_t event,
                                   ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    devcmd_ctx.status = FTL_RC_SUCCESS;
    return FTL_LIF_EV_NULL;
}

ftl_lif_event_t
FtlLif::ftl_lif_eagain_action(ftl_lif_event_t event,
                              ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    FTL_LIF_FSM_LOG();
    devcmd_ctx.status = FTL_RC_EAGAIN;
    return FTL_LIF_EV_NULL;
}

ftl_lif_event_t
FtlLif::ftl_lif_reject_action(ftl_lif_event_t event,
                              ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    FTL_LIF_FSM_ERR_LOG();
    devcmd_ctx.status = FTL_RC_EPERM;
    return FTL_LIF_EV_NULL;
}

ftl_lif_event_t
FtlLif::ftl_lif_create_action(ftl_lif_event_t event,
                              ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    FTL_LIF_FSM_LOG();
    memset(pd_qinfo, 0, sizeof(pd_qinfo));

    pd_qinfo[FTL_QTYPE_SCANNER_SESSION] = {
        .type_num = FTL_QTYPE_SCANNER_SESSION,
        .size = HW_CB_MULTIPLE(SCANNER_SESSION_CB_TABLE_BYTES_SHFT),
        .entries = log_2(spec->session_hw_scanners),
    };

    pd_qinfo[FTL_QTYPE_SCANNER_CONNTRACK] = {
        .type_num = FTL_QTYPE_SCANNER_CONNTRACK,
        .size = HW_CB_MULTIPLE(SCANNER_SESSION_CB_TABLE_BYTES_SHFT),
        .entries = log_2(spec->conntrack_hw_scanners),
    };

    pd_qinfo[FTL_QTYPE_POLLER] = {
        .type_num = FTL_QTYPE_POLLER,
        .size = HW_CB_MULTIPLE(POLLER_CB_TABLE_BYTES_SHFT),
        .entries = log_2(spec->sw_pollers),
    };

    hal_lif_info_.type = sdk::platform::LIF_TYPE_SERVICE;
    hal_lif_info_.lif_state = sdk::types::LIF_STATE_UP;
    hal_lif_info_.tx_sched_table_offset = INVALID_INDEXER_INDEX;
    memcpy(hal_lif_info_.queue_info, pd_qinfo, sizeof(hal_lif_info_.queue_info));
    return FTL_LIF_EV_NULL;
}

ftl_lif_event_t
FtlLif::ftl_lif_destroy_action(ftl_lif_event_t event,
                               ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    FTL_LIF_FSM_LOG();
    devcmd_ctx.status = FTL_RC_SUCCESS;
    return FTL_LIF_EV_RESET_DESTROY;
}

ftl_lif_event_t
FtlLif::ftl_lif_hal_up_action(ftl_lif_event_t event,
                              ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    FTL_LIF_FSM_LOG();
    cosA = 1;
    cosB = 0;
    ctl_cosA = 1;

    hw_coreclk_freq = sdk::asic::pd::asicpd_get_coreclk_freq(pd->platform_);
    NIC_LOG_DEBUG("{}: hw_coreclk_freq {}", LifNameGet(), hw_coreclk_freq);
    hw_ns_per_tick = hw_coreclk_freq ?
                     (double)1E9 / (double)hw_coreclk_freq : 0;

    FTL_LIF_DEVAPI_CHECK(FTL_RC_ERROR, FTL_LIF_EV_NULL);
    dev_api->qos_get_txtc_cos("INTERNAL_TX_PROXY_DROP", 1, &cosB);
    if ((int)cosB < 0) {
        NIC_LOG_ERR("{}: Failed to get cosB for group {}, uplink {}",
                    LifNameGet(), "INTERNAL_TX_PROXY_DROP", 1);
        cosB = 0;
        devcmd_ctx.status = FTL_RC_ERROR;
    }
    dev_api->qos_get_txtc_cos("CONTROL", 1, &ctl_cosB);
    if ((int)ctl_cosB < 0) {
        NIC_LOG_ERR("{}: Failed to get cosB for group {}, uplink {}",
                    LifNameGet(), "CONTROL", 1);
        ctl_cosB = 0;
        devcmd_ctx.status = FTL_RC_ERROR;
    }

    NIC_LOG_DEBUG("{}: cosA: {} cosB: {} ctl_cosA: {} ctl_cosB: {}",
                  LifNameGet(), cosA, cosB, ctl_cosA, ctl_cosB);
    return FTL_LIF_EV_NULL;
}


ftl_lif_event_t
FtlLif::ftl_lif_setattr_action(ftl_lif_event_t event,
                               ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    lif_setattr_cmd_t  *cmd = &devcmd_ctx.req->lif_setattr;

    FTL_LIF_FSM_LOG();
    switch (cmd->attr) {

    case FTL_LIF_ATTR_NAME:
        FTL_LIF_DEVAPI_CHECK(FTL_RC_ERROR, FTL_LIF_EV_NULL);

        /*
         * Note: this->lif_name must remains fixed as it was used
         * for log tracing purposes. Only the HAL lif name should change.
         */
        strncpy0(hal_lif_info_.name, cmd->name, sizeof(hal_lif_info_.name));
        dev_api->lif_upd_name(LifIdGet(), hal_lif_info_.name);
        NIC_LOG_DEBUG("{}: HAL name changed to {}",
                      LifNameGet(), hal_lif_info_.name);
        break;

    case FTL_LIF_ATTR_NORMAL_AGE_TMO:
        age_tmo_cb_set("normal", &normal_age_tmo_cb,
                       normal_age_access(), &cmd->age_tmo);
        break;

    case FTL_LIF_ATTR_ACCEL_AGE_TMO:
        age_tmo_cb_set("accelerated", &accel_age_tmo_cb,
                       accel_age_access(), &cmd->age_tmo);
        break;

    case FTL_LIF_ATTR_METRICS:

        /*
         * Setting of metrics not supported; only Get allowed
         */
        devcmd_ctx.status = FTL_RC_EPERM;
        break;

    case FTL_LIF_ATTR_FORCE_SESSION_EXPIRED_TS:
        force_session_expired_ts_set(&normal_age_tmo_cb,
                              normal_age_access(), cmd->force_expired_ts);
        force_session_expired_ts_set(&accel_age_tmo_cb,
                              accel_age_access(), cmd->force_expired_ts);
        break;

    case FTL_LIF_ATTR_FORCE_CONNTRACK_EXPIRED_TS:
        force_conntrack_expired_ts_set(&normal_age_tmo_cb,
                              normal_age_access(), cmd->force_expired_ts);
        force_conntrack_expired_ts_set(&accel_age_tmo_cb,
                              accel_age_access(), cmd->force_expired_ts);
        break;

    default:
        NIC_LOG_ERR("{}: unknown ATTR {}", LifNameGet(), cmd->attr);
        devcmd_ctx.status = FTL_RC_EINVAL;
        break;
    }

    return FTL_LIF_EV_NULL;
}

ftl_lif_event_t
FtlLif::ftl_lif_getattr_action(ftl_lif_event_t event,
                               ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    lif_getattr_cmd_t  *cmd = &devcmd_ctx.req->lif_getattr;
    lif_getattr_cpl_t  *cpl = &devcmd_ctx.rsp->lif_getattr;

    FTL_LIF_FSM_LOG();

    switch (cmd->attr) {

    case FTL_LIF_ATTR_NAME:

        /*
         * Name is too big to return in cpl so use rsp_data if available
         */
        if (devcmd_ctx.rsp_data) {
            strncpy0((char *)devcmd_ctx.rsp_data, hal_lif_info_.name,
                     FTL_DEV_IFNAMSIZ);
        }
        break;

    case FTL_LIF_ATTR_NORMAL_AGE_TMO:
        if (devcmd_ctx.rsp_data) {
            age_tmo_cb_get((lif_attr_age_tmo_t *)devcmd_ctx.rsp_data,
                           &normal_age_tmo_cb);
        }
        break;

    case FTL_LIF_ATTR_ACCEL_AGE_TMO:
        if (devcmd_ctx.rsp_data) {
            age_tmo_cb_get((lif_attr_age_tmo_t *)devcmd_ctx.rsp_data,
                           &accel_age_tmo_cb);
        }
        break;

    case FTL_LIF_ATTR_METRICS:

        if (devcmd_ctx.rsp_data) {
            lif_attr_metrics_t  *metrics =
                     (lif_attr_metrics_t *)devcmd_ctx.rsp_data;
            switch (cmd->qtype) {
            case FTL_QTYPE_SCANNER_SESSION:
                devcmd_ctx.status = session_scanners_ctl.metrics_get(metrics);
                break;

            case FTL_QTYPE_SCANNER_CONNTRACK:
                devcmd_ctx.status = conntrack_scanners_ctl.metrics_get(metrics);
                break;

            case FTL_QTYPE_POLLER:
                devcmd_ctx.status = pollers_ctl.metrics_get(metrics);
                break;

            default:
                NIC_LOG_ERR("{}: Unsupported qtype {}", LifNameGet(), cmd->qtype);
                devcmd_ctx.status = FTL_RC_EQTYPE;
                break;
            }
        }
        break;

    case FTL_LIF_ATTR_FORCE_SESSION_EXPIRED_TS:
        cpl->force_expired_ts = normal_age_tmo_cb.force_session_expired_ts;
        break;

    case FTL_LIF_ATTR_FORCE_CONNTRACK_EXPIRED_TS:
        cpl->force_expired_ts = normal_age_tmo_cb.force_conntrack_expired_ts;
        break;

    default:
        NIC_LOG_ERR("{}: unknown ATTR {}", LifNameGet(), cmd->attr);
        devcmd_ctx.status = FTL_RC_EINVAL;
        break;
    }

    return FTL_LIF_EV_NULL;
}

ftl_lif_event_t
FtlLif::ftl_lif_identify_action(ftl_lif_event_t event,
                                ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    lif_identify_cmd_t  *cmd = &devcmd_ctx.req->lif_identify;
    lif_identity_t      *rsp = (lif_identity_t *)devcmd_ctx.rsp_data;
    lif_identify_cpl_t  *cpl = &devcmd_ctx.rsp->lif_identify;

    FTL_LIF_FSM_LOG();

    if (cmd->type >= FTL_LIF_TYPE_MAX) {
        NIC_LOG_ERR("{}: bad lif type {}", LifNameGet(), cmd->type);
        devcmd_ctx.status = FTL_RC_EINVAL;
        return FTL_LIF_EV_NULL;
    }
    if (cmd->ver != IDENTITY_VERSION_1) {
        NIC_LOG_ERR("{}: unsupported version {}", LifNameGet(), cmd->ver);
        devcmd_ctx.status = FTL_RC_EVERSION;
        return FTL_LIF_EV_NULL;
    }

    memset(&rsp->base, 0, sizeof(rsp->base));
    rsp->base.version = IDENTITY_VERSION_1;
    rsp->base.hw_index = LifIdGet();
    rsp->base.qident[FTL_QTYPE_SCANNER_SESSION].qcount =
                                       session_scanners_ctl.qcount();
    rsp->base.qident[FTL_QTYPE_SCANNER_SESSION].burst_sz =
                                       spec->session_burst_size;
    rsp->base.qident[FTL_QTYPE_SCANNER_SESSION].burst_resched_time_us =
                                       spec->session_burst_resched_time_us;
    rsp->base.qident[FTL_QTYPE_SCANNER_CONNTRACK].qcount =
                                       conntrack_scanners_ctl.qcount();
    rsp->base.qident[FTL_QTYPE_SCANNER_CONNTRACK].burst_sz =
                                       spec->conntrack_burst_size;
    rsp->base.qident[FTL_QTYPE_SCANNER_CONNTRACK].burst_resched_time_us =
                                       spec->conntrack_burst_resched_time_us;
    rsp->base.qident[FTL_QTYPE_POLLER].qcount = pollers_ctl.qcount();
    rsp->base.qident[FTL_QTYPE_POLLER].qdepth = spec->sw_poller_qdepth;

    cpl->ver = IDENTITY_VERSION_1;
    return FTL_LIF_EV_NULL;
}

ftl_lif_event_t
FtlLif::ftl_lif_init_action(ftl_lif_event_t event,
                            ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    FTL_LIF_FSM_LOG();

    fsm_ctx.reset = false;
    fsm_ctx.reset_destroy = false;
    FTL_LIF_DEVAPI_CHECK(FTL_RC_ERROR, FTL_LIF_EV_NULL);
    if (dev_api->lif_create(&hal_lif_info_) != SDK_RET_OK) {
        NIC_LOG_ERR("{}: Failed to create LIF", LifNameGet());
        devcmd_ctx.status = FTL_RC_ERROR;
    }

    pd->program_qstate((struct queue_info*)hal_lif_info_.queue_info,
                       &hal_lif_info_, 0x0);
    NIC_LOG_INFO("{}: created", LifNameGet());
    return FTL_LIF_EV_NULL;
}

ftl_lif_event_t
FtlLif::ftl_lif_reset_action(ftl_lif_event_t event,
                             ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    lif_reset_cmd_t     *cmd = &devcmd_ctx.req->lif_reset;

    FTL_LIF_FSM_LOG();
    fsm_ctx.reset = true;
    fsm_ctx.reset_destroy = (event == FTL_LIF_EV_RESET_DESTROY) ||
                            (event == FTL_LIF_EV_DESTROY);
    /*
     * Special handling for the case where lif_reset is
     * called before scanners are initialized.
     */
    if (session_scanners_ctl.empty_qstate_access() ||
        conntrack_scanners_ctl.empty_qstate_access()) {
        return FTL_LIF_EV_QUEUES_STOP_COMPLETE;
    }
    session_scanners_ctl.stop();
    conntrack_scanners_ctl.stop();

    if (cmd->quiesce_check) {
        fsm_ctx.ts.time_expiry_set(FTL_LIF_SCANNERS_QUIESCE_TIME_US);
        devcmd_ctx.status = FTL_RC_EAGAIN;
        return FTL_LIF_EV_SCANNERS_QUIESCE;
    }

    return FTL_LIF_EV_QUEUES_STOP_COMPLETE;
}

ftl_lif_event_t
FtlLif::ftl_lif_pollers_init_action(ftl_lif_event_t event,
                                    ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    pollers_init_cmd_t          *cmd = &devcmd_ctx.req->pollers_init;
    pollers_init_cpl_t          *cpl = &devcmd_ctx.rsp->pollers_init;

    FTL_LIF_FSM_LOG();
    devcmd_ctx.status = pollers_ctl.init(cmd);

    cpl->qtype = cmd->qtype;
    return FTL_LIF_EV_NULL;
}

ftl_lif_event_t
FtlLif::ftl_lif_pollers_flush_action(ftl_lif_event_t event,
                                     ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    pollers_flush_cmd_t     *cmd = &devcmd_ctx.req->pollers_flush;
    pollers_flush_cpl_t     *cpl = &devcmd_ctx.rsp->pollers_flush;

    FTL_LIF_FSM_LOG();

    /*
     * SW pollers flush is implemented with the stop() method which
     * has immediate completion.
     */
    devcmd_ctx.status = pollers_ctl.stop();

    cpl->qtype = cmd->qtype;
    return FTL_LIF_EV_NULL;
}

ftl_lif_event_t
FtlLif::ftl_lif_pollers_deq_burst_action(ftl_lif_event_t event,
                                         ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    pollers_deq_burst_cmd_t *cmd = &devcmd_ctx.req->pollers_deq_burst;
    pollers_deq_burst_cpl_t *cpl = &devcmd_ctx.rsp->pollers_deq_burst;
    uint32_t                burst_count = cmd->burst_count;

    // Don't log as this function is called very frequently
    //FTL_LIF_FSM_LOG();

    devcmd_ctx.status =
        pollers_ctl.dequeue_burst(cmd->index, &burst_count,
                                  (uint8_t *)devcmd_ctx.rsp_data, cmd->buf_sz);
    cpl->qtype = cmd->qtype;
    cpl->read_count = burst_count;
    return FTL_LIF_EV_NULL;
}

ftl_lif_event_t
FtlLif::ftl_lif_scanners_init_action(ftl_lif_event_t event,
                                     ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    scanners_init_cmd_t         *cmd = &devcmd_ctx.req->scanners_init;
    scanners_init_cpl_t         *cpl = &devcmd_ctx.rsp->scanners_init;

    FTL_LIF_FSM_LOG();
    switch (cmd->qtype) {

    case FTL_QTYPE_SCANNER_SESSION:
        devcmd_ctx.status = session_scanners_ctl.init(cmd);
        break;

    case FTL_QTYPE_SCANNER_CONNTRACK:
        devcmd_ctx.status = conntrack_scanners_ctl.init(cmd);
        break;

    default:
        NIC_LOG_ERR("{}: Unsupported qtype {}", LifNameGet(), cmd->qtype);
        devcmd_ctx.status = FTL_RC_EQTYPE;
        break;
    }

    cpl->qtype = cmd->qtype;
    return FTL_LIF_EV_NULL;
}

ftl_lif_event_t
FtlLif::ftl_lif_scanners_start_action(ftl_lif_event_t event,
                                      ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    ftl_status_code_t           session_status;
    ftl_status_code_t           conntrack_status;

    FTL_LIF_FSM_LOG();
    session_status = session_scanners_ctl.start();
    conntrack_status = conntrack_scanners_ctl.start();

    devcmd_ctx.status = session_status;
    if (devcmd_ctx.status == FTL_RC_SUCCESS) {
        devcmd_ctx.status = conntrack_status;
    }
    return FTL_LIF_EV_NULL;
}

ftl_lif_event_t
FtlLif::ftl_lif_scanners_stop_action(ftl_lif_event_t event,
                                     ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    scanners_stop_cmd_t         *cmd = &devcmd_ctx.req->scanners_stop;
    ftl_status_code_t           session_status;
    ftl_status_code_t           conntrack_status;

    FTL_LIF_FSM_LOG();
    session_status = session_scanners_ctl.stop();
    conntrack_status = conntrack_scanners_ctl.stop();

    devcmd_ctx.status = session_status;
    if (devcmd_ctx.status == FTL_RC_SUCCESS) {
        devcmd_ctx.status = conntrack_status;
    }

    if (cmd->quiesce_check) {
        fsm_ctx.ts.time_expiry_set(FTL_LIF_SCANNERS_QUIESCE_TIME_US);
        devcmd_ctx.status = FTL_RC_EAGAIN;
        return FTL_LIF_EV_SCANNERS_QUIESCE;
    }

    return FTL_LIF_EV_QUEUES_STOP_COMPLETE;
}

ftl_lif_event_t
FtlLif::ftl_lif_scanners_quiesce_action(ftl_lif_event_t event,
                                        ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    bool    quiesce_complete = false;

    FTL_LIF_FSM_VERBOSE_LOG();

    devcmd_ctx.status = FTL_RC_EAGAIN;
    if (session_scanners_ctl.quiesce() && conntrack_scanners_ctl.quiesce()) {
        quiesce_complete = true;
    }

    if (!quiesce_complete && fsm_ctx.ts.time_expiry_check()) {
        NIC_LOG_DEBUG("{}: scanners quiesce timed out", LifNameGet());
        NIC_LOG_DEBUG("    last session qid quiesced: {} qid_high: {}",
                      session_scanners_ctl.quiesce_qid(),
                      session_scanners_ctl.qid_high());
        NIC_LOG_DEBUG("    last conntrack qid quiesced: {} qid_high: {}",
                      conntrack_scanners_ctl.quiesce_qid(),
                      conntrack_scanners_ctl.qid_high());
        quiesce_complete = true;
    }

    if (quiesce_complete) {
        session_scanners_ctl.quiesce_idle();
        conntrack_scanners_ctl.quiesce_idle();
        return FTL_LIF_EV_QUEUES_STOP_COMPLETE;
    }

    return FTL_LIF_EV_NULL;
}

ftl_lif_event_t
FtlLif::ftl_lif_scanners_start_single_action(ftl_lif_event_t event,
                                             ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    scanners_start_single_cmd_t *cmd = &devcmd_ctx.req->scanners_start_single;
    scanners_start_single_cpl_t *cpl = &devcmd_ctx.rsp->scanners_start_single;

    // Don't log as this function may be called very frequently
    //FTL_LIF_FSM_LOG();
    switch (cmd->qtype) {

    case FTL_QTYPE_SCANNER_SESSION:
        devcmd_ctx.status = session_scanners_ctl.sched_start_single(cmd->index);
        break;

    case FTL_QTYPE_SCANNER_CONNTRACK:
        devcmd_ctx.status = conntrack_scanners_ctl.sched_start_single(cmd->index);
        break;

    case FTL_QTYPE_POLLER:
        devcmd_ctx.status = pollers_ctl.sched_start_single(cmd->index);
        break;

    default:
        NIC_LOG_ERR("{}: Unsupported qtype {}", LifNameGet(), cmd->qtype);
        devcmd_ctx.status = FTL_RC_EQTYPE;
        break;
    }

    cpl->qtype = cmd->qtype;
    return FTL_LIF_EV_NULL;
}

ftl_lif_event_t
FtlLif::ftl_lif_accel_aging_ctl_action(ftl_lif_event_t event,
                                       ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    accel_aging_ctl_cmd_t   *cmd = &devcmd_ctx.req->accel_aging_ctl;

    // Don't log as this function might be called frequently
    //FTL_LIF_FSM_LOG();

    devcmd_ctx.status = cmd->enable_sense ?
                        accel_age_tmo_cb_select() : normal_age_tmo_cb_select();
    return FTL_LIF_EV_NULL;
}

/*
 * Event-Action state machine execute
 */
void
FtlLif::ftl_lif_state_machine(ftl_lif_event_t event,
                              ftl_lif_devcmd_ctx_t& devcmd_ctx)
{
    ftl_lif_ordered_event_t     *ordered_event;
    ftl_lif_action_t            action;

    while (event != FTL_LIF_EV_NULL) {

        if ((fsm_ctx.state < FTL_LIF_ST_MAX) &&
            (event < FTL_LIF_EV_MAX)) {

            ordered_event = &lif_ordered_ev_table[fsm_ctx.state][event];
            fsm_ctx.enter_state = fsm_ctx.state;
            if (ordered_event->next_state != FTL_LIF_ST_SAME) {
                fsm_ctx.state = ordered_event->next_state;
            }
            action = ordered_event->action;
            if (!action) {
                NIC_LOG_ERR("Null action for state {} event {}",
                            lif_state_str(fsm_ctx.enter_state),
                            lif_event_str(event));
                throw;
            }
            event = (this->*action)(event, devcmd_ctx);

        } else {
            NIC_LOG_ERR("Unknown state {} or event {}",
                        fsm_ctx.state, event);
            throw;
        }
    }
}

void
FtlLif::age_tmo_cb_init(age_tmo_cb_t *age_tmo_cb,
                        const mem_access_t& access,
                        bool cb_select)
{
    memset(age_tmo_cb, 0, sizeof(*age_tmo_cb));

    /*
     * Timeout values are stored in big endian to make it
     * convenient for MPU code to load them with bit truncation.
     */
    age_tmo_cb->tcp_syn_tmo = htonl(SCANNER_TCP_SYN_TMO_DFLT);
    age_tmo_cb->tcp_est_tmo = htonl(SCANNER_TCP_EST_TMO_DFLT);
    age_tmo_cb->tcp_fin_tmo = htonl(SCANNER_TCP_FIN_TMO_DFLT);
    age_tmo_cb->tcp_timewait_tmo = htonl(SCANNER_TCP_TIMEWAIT_TMO_DFLT);
    age_tmo_cb->tcp_rst_tmo = htonl(SCANNER_TCP_RST_TMO_DFLT);
    age_tmo_cb->udp_tmo = htonl(SCANNER_UDP_TMO_DFLT);
    age_tmo_cb->udp_est_tmo = htonl(SCANNER_UDP_EST_TMO_DFLT);
    age_tmo_cb->icmp_tmo = htonl(SCANNER_ICMP_TMO_DFLT);
    age_tmo_cb->others_tmo = htonl(SCANNER_OTHERS_TMO_DFLT);
    age_tmo_cb->session_tmo = htonl(SCANNER_SESSION_TMO_DFLT);

    age_tmo_cb->cb_activate = SCANNER_AGE_TMO_CB_ACTIVATE;
    age_tmo_cb->cb_select = cb_select;
    access.small_write(0, (uint8_t *)age_tmo_cb, sizeof(*age_tmo_cb));
}

void
FtlLif::age_tmo_cb_set(const char *which,
                       age_tmo_cb_t *age_tmo_cb,
                       const mem_access_t& access,
                       const lif_attr_age_tmo_t *attr_age_tmo)
{
    lif_attr_age_tmo_t  scratch_tmo;

    /*
     * Age values change in CB requires the following order:
     *   deactivate, change values, activate
     * in order to ensure MPU does not pick up any partially filled values.
     */
    age_tmo_cb->cb_activate = (age_tmo_cb_activate_t)~SCANNER_AGE_TMO_CB_ACTIVATE;
    access.small_write(offsetof(age_tmo_cb_t, cb_activate),
                       (uint8_t *)&age_tmo_cb->cb_activate,
                       sizeof(age_tmo_cb->cb_activate));
    /*
     * Timeout values are stored in big endian to make it
     * convenient for MPU code to load them with bit truncation.
     */
    age_tmo_cb->tcp_syn_tmo = htonl(std::min(attr_age_tmo->tcp_syn_tmo,
                                         (uint32_t)SCANNER_TCP_SYN_TMO_MAX));
    age_tmo_cb->tcp_est_tmo = htonl(std::min(attr_age_tmo->tcp_est_tmo,
                                         (uint32_t)SCANNER_TCP_EST_TMO_MAX));
    age_tmo_cb->tcp_fin_tmo = htonl(std::min(attr_age_tmo->tcp_fin_tmo,
                                         (uint32_t)SCANNER_TCP_FIN_TMO_MAX));
    age_tmo_cb->tcp_timewait_tmo = htonl(std::min(attr_age_tmo->tcp_timewait_tmo,
                                         (uint32_t)SCANNER_TCP_TIMEWAIT_TMO_MAX));
    age_tmo_cb->tcp_rst_tmo = htonl(std::min(attr_age_tmo->tcp_rst_tmo,
                                         (uint32_t)SCANNER_TCP_RST_TMO_MAX));
    age_tmo_cb->udp_tmo = htonl(std::min(attr_age_tmo->udp_tmo,
                                         (uint32_t)SCANNER_UDP_TMO_MAX));
    age_tmo_cb->udp_est_tmo = htonl(std::min(attr_age_tmo->udp_est_tmo,
                                         (uint32_t)SCANNER_UDP_EST_TMO_MAX));
    age_tmo_cb->icmp_tmo = htonl(std::min(attr_age_tmo->icmp_tmo,
                                         (uint32_t)SCANNER_ICMP_TMO_MAX));
    age_tmo_cb->others_tmo = htonl(std::min(attr_age_tmo->others_tmo,
                                         (uint32_t)SCANNER_OTHERS_TMO_MAX));
    age_tmo_cb->session_tmo = htonl(std::min(attr_age_tmo->session_tmo,
                                         (uint32_t)SCANNER_SESSION_TMO_MAX));
    NIC_LOG_DEBUG("{}: {} inactivity timeout values change",
                  LifNameGet(), which);
    age_tmo_cb_get(&scratch_tmo, age_tmo_cb);

    access.small_write(0, (uint8_t *)age_tmo_cb, sizeof(*age_tmo_cb));

    age_tmo_cb->cb_activate = SCANNER_AGE_TMO_CB_ACTIVATE;
    access.small_write(offsetof(age_tmo_cb_t, cb_activate),
                       (uint8_t *)&age_tmo_cb->cb_activate,
                       sizeof(age_tmo_cb->cb_activate));
}

void
FtlLif::age_tmo_cb_get(lif_attr_age_tmo_t *attr_age_tmo,
                       const age_tmo_cb_t *age_tmo_cb)
{
    attr_age_tmo->tcp_syn_tmo = ntohl(age_tmo_cb->tcp_syn_tmo);
    attr_age_tmo->tcp_est_tmo = ntohl(age_tmo_cb->tcp_est_tmo);
    attr_age_tmo->tcp_fin_tmo = ntohl(age_tmo_cb->tcp_fin_tmo);
    attr_age_tmo->tcp_timewait_tmo = ntohl(age_tmo_cb->tcp_timewait_tmo);
    attr_age_tmo->tcp_rst_tmo = ntohl(age_tmo_cb->tcp_rst_tmo);
    attr_age_tmo->udp_tmo = ntohl(age_tmo_cb->udp_tmo);
    attr_age_tmo->udp_est_tmo = ntohl(age_tmo_cb->udp_est_tmo);
    attr_age_tmo->icmp_tmo = ntohl(age_tmo_cb->icmp_tmo);
    attr_age_tmo->others_tmo = ntohl(age_tmo_cb->others_tmo);
    attr_age_tmo->session_tmo = ntohl(age_tmo_cb->session_tmo);

    NIC_LOG_DEBUG("    tcp_syn_tmo {} tcp_est_tmo {} tcp_fin_tmo {} "
                  "tcp_timewait_tmo {} tcp_rst_tmo {}",
                  attr_age_tmo->tcp_syn_tmo, attr_age_tmo->tcp_est_tmo,
                  attr_age_tmo->tcp_fin_tmo, attr_age_tmo->tcp_timewait_tmo,
                  attr_age_tmo->tcp_rst_tmo);
    NIC_LOG_DEBUG("    udp_tmo {} udp_est_tmo {} icmp_tmo {} others_tmo {} "
                  " session_tmo {}", attr_age_tmo->udp_tmo,
                  attr_age_tmo->udp_est_tmo, attr_age_tmo->icmp_tmo,
                  attr_age_tmo->others_tmo, attr_age_tmo->session_tmo);
}

void
FtlLif::force_session_expired_ts_set(age_tmo_cb_t *age_tmo_cb,
                                     const mem_access_t& access,
                                     uint8_t force_expired_ts)
{
    /*
     * Timestamp set/get only used for SIM debugging purposes so
     * it's fine to dynamically update it.
     */
    age_tmo_cb->force_session_expired_ts = force_expired_ts;
    access.small_write(offsetof(age_tmo_cb_t, force_session_expired_ts),
                       (uint8_t *)&age_tmo_cb->force_session_expired_ts,
                       sizeof(age_tmo_cb->force_session_expired_ts));
}

void
FtlLif::force_conntrack_expired_ts_set(age_tmo_cb_t *age_tmo_cb,
                                       const mem_access_t& access,
                                       uint8_t force_expired_ts)
{
    /*
     * Timestamp set/get only used for SIM debugging purposes so
     * it's fine to dynamically update it.
     */
    age_tmo_cb->force_conntrack_expired_ts = force_expired_ts;
    access.small_write(offsetof(age_tmo_cb_t, force_conntrack_expired_ts),
                       (uint8_t *)&age_tmo_cb->force_conntrack_expired_ts,
                       sizeof(age_tmo_cb->force_conntrack_expired_ts));
}

ftl_status_code_t
FtlLif::normal_age_tmo_cb_select(void)
{
    if (!normal_age_tmo_cb.cb_select) {

        /*
         * Must always enable the newly selected CB before disabling the other.
         */
        normal_age_tmo_cb.cb_select = true;
        normal_age_access().small_write(offsetof(age_tmo_cb_t, cb_select),
                                  (uint8_t *)&normal_age_tmo_cb.cb_select,
                                  sizeof(normal_age_tmo_cb.cb_select));

        accel_age_tmo_cb.cb_select = false;
        accel_age_access().small_write(offsetof(age_tmo_cb_t, cb_select),
                                 (uint8_t *)&accel_age_tmo_cb.cb_select,
                                 sizeof(accel_age_tmo_cb.cb_select));
    }

    return FTL_RC_SUCCESS;
}

ftl_status_code_t
FtlLif::accel_age_tmo_cb_select(void)
{
    if (!accel_age_tmo_cb.cb_select) {

        /*
         * Must always enable the newly selected CB before disabling the other.
         */
        accel_age_tmo_cb.cb_select = true;
        accel_age_access().small_write(offsetof(age_tmo_cb_t, cb_select),
                                 (uint8_t *)&accel_age_tmo_cb.cb_select,
                                 sizeof(accel_age_tmo_cb.cb_select));

        normal_age_tmo_cb.cb_select = false;
        normal_age_access().small_write(offsetof(age_tmo_cb_t, cb_select),
                                  (uint8_t *)&normal_age_tmo_cb.cb_select,
                                  sizeof(normal_age_tmo_cb.cb_select));
    }

    return FTL_RC_SUCCESS;
}

/*
 * One-time state machine initialization for efficient direct indexing.
 */
static void
ftl_lif_state_machine_build(void)
{
    ftl_lif_state_event_t       **fsm_entry;
    ftl_lif_state_event_t       *state_event;
    ftl_lif_state_event_t       *any_event;
    ftl_lif_ordered_event_t     *ordered_event;
    uint32_t                    state;

    static bool lif_ordered_event_table_built;
    if (lif_ordered_event_table_built) {
        return;
    }
    lif_ordered_event_table_built = true;

    for (fsm_entry = &lif_fsm_table[0], state = 0;
         fsm_entry < &lif_fsm_table[FTL_LIF_ST_MAX];
         fsm_entry++, state++) {

        state_event = *fsm_entry;
        if (state_event) {
            any_event = nullptr;
            while (state_event->event != FTL_LIF_EV_NULL) {
                if (state_event->event < FTL_LIF_EV_MAX) {
                    ordered_event = &lif_ordered_ev_table[state]
                                                         [state_event->event];

                    ordered_event->action = state_event->action;
                    ordered_event->next_state = state_event->next_state;

                    if (state_event->event == FTL_LIF_EV_ANY) {
                        any_event = state_event;
                    }

                } else {
                    NIC_LOG_ERR("Unknown event {} for state {}", state_event->event,
                                lif_state_str((ftl_lif_state_t)state));
                    throw;
                }
                state_event++;
            }

            if (!any_event) {
                NIC_LOG_ERR("Missing 'any' event for state {}",
                            lif_state_str((ftl_lif_state_t)state));
                throw;
            }

            for (ordered_event = &lif_ordered_ev_table[state][0];
                 ordered_event < &lif_ordered_ev_table[state][FTL_LIF_EV_MAX];
                 ordered_event++) {

                if (!ordered_event->action) {
                    ordered_event->action  = any_event->action;
                    ordered_event->next_state = any_event->next_state;
                }
            }
        }
    }
}

/*
 * Queues control class
 */
ftl_lif_queues_ctl_t::ftl_lif_queues_ctl_t(FtlLif& lif,
                                           enum ftl_qtype qtype,
                                           uint32_t qcount) :
    lif(lif),
    qtype_(qtype),
    db_pndx_inc(lif),
    db_shed_clr(lif),
    wrings_base_addr(0),
    wring_single_sz(0),
    slot_data_sz(0),
    qcount_(qcount),
    qdepth(0),
    qdepth_mask(0),
    qid_high_(0),
    quiescing(false)
{
}

ftl_lif_queues_ctl_t::~ftl_lif_queues_ctl_t()
{
}

ftl_status_code_t
ftl_lif_queues_ctl_t::init(const scanners_init_cmd_t *cmd)
{
    scanner_init_single_cmd_t   single_cmd = {0};
    uint32_t                    curr_scan_table_sz;
    ftl_status_code_t           status;

    NIC_LOG_DEBUG("{}: qtype {} qcount {} cos_override {} cos {}",
                  lif.LifNameGet(), cmd->qtype, cmd->qcount,
                  cmd->cos_override, cmd->cos);
    NIC_LOG_DEBUG("    scan_addr_base {:#x} scan_table_sz {} scan_id_base {} "
                  "scan_burst_sz {} scan_resched_time {}",
                  cmd->scan_addr_base, cmd->scan_table_sz, cmd->scan_id_base,
                  cmd->scan_burst_sz, cmd->scan_resched_time);
    NIC_LOG_DEBUG("    poller_lif {} poller_qcount {} poller_qdepth {} "
                  "poller_qtype {}", cmd->poller_lif,
                  cmd->poller_qcount, cmd->poller_qdepth,
                  cmd->poller_qtype);
    qstate_access.clear();
    wring_access.clear();
    qid_high_ = 0;

    single_cmd.cos = cmd->cos;
    single_cmd.cos_override = cmd->cos_override;
    single_cmd.qtype = cmd->qtype;
    single_cmd.lif_index = cmd->lif_index;
    single_cmd.pid = cmd->pid;
    single_cmd.scan_addr_base = cmd->scan_addr_base;
    single_cmd.scan_id_base = cmd->scan_id_base;
    single_cmd.scan_burst_sz = cmd->scan_burst_sz;
    single_cmd.scan_resched_time = cmd->scan_resched_time;

    single_cmd.poller_lif = cmd->poller_lif;
    single_cmd.poller_qtype = cmd->poller_qtype;

    /*
     * poller queue depth must be a power of 2
     */
    assert(is_power_of_2(cmd->poller_qdepth));
    single_cmd.poller_qdepth_shft = log_2(cmd->poller_qdepth);

    /*
     * Partition out the entire table space to
     * the available number of queues
     */
    assert(cmd->qcount && cmd->poller_qcount);
    single_cmd.scan_range_sz = std::max((uint32_t)1,
                                        cmd->scan_table_sz / cmd->qcount);
    curr_scan_table_sz = cmd->scan_table_sz;

    single_cmd.index = 0;
    single_cmd.poller_qid = 0;
    while ((single_cmd.index < cmd->qcount) && curr_scan_table_sz) {
        single_cmd.scan_range_sz = std::min(curr_scan_table_sz,
                                            single_cmd.scan_range_sz);
        status = scanner_init_single(&single_cmd);
        if (status != FTL_RC_SUCCESS) {
            return status;
        }

        curr_scan_table_sz -= single_cmd.scan_range_sz;
        single_cmd.scan_id_base += single_cmd.scan_range_sz;

        single_cmd.index++;
        single_cmd.poller_qid++;
        if (single_cmd.poller_qid >= cmd->poller_qcount) {
            single_cmd.poller_qid = 0;
        }
    }

    NIC_LOG_DEBUG("{}: qtype {} qid_high {}", lif.LifNameGet(),
                  cmd->qtype, qid_high_);
    return FTL_RC_SUCCESS;
}

ftl_status_code_t
ftl_lif_queues_ctl_t::init(const pollers_init_cmd_t *cmd)
{
    poller_init_single_cmd_t    single_cmd = {0};
    ftl_status_code_t           status;

    NIC_LOG_DEBUG("{}: qtype {} qcount {} qdepth {} wrings_base_addr {:#x} "
                  "wrings_total_sz {}", lif.LifNameGet(), cmd->qtype,
                  cmd->qcount, cmd->qdepth, cmd->wrings_base_addr,
                  cmd->wrings_total_sz);
    qstate_access.clear();
    wring_access.clear();
    qid_high_ = 0;
    wrings_base_addr = cmd->wrings_base_addr;
    slot_data_sz = POLLER_SLOT_DATA_BYTES;
    qdepth = cmd->qdepth;

    single_cmd.lif_index = cmd->lif_index;
    single_cmd.pid = cmd->pid;

    /*
     * poller queue depth must be a power of 2
     */
    assert(is_power_of_2(cmd->qdepth));
    single_cmd.qdepth_shft = log_2(cmd->qdepth);
    qdepth_mask = (1 << single_cmd.qdepth_shft) - 1;

    /*
     * Partition out the work rings space to
     * the available number of queues
     */
    assert(cmd->qcount);
    wring_single_sz = cmd->qdepth * POLLER_SLOT_DATA_BYTES;
    if ((cmd->qcount * wring_single_sz) > cmd->wrings_total_sz) {
        NIC_LOG_ERR("{}: wrings_total_sz {} too small for qcount {}",
                    lif.LifNameGet(), cmd->wrings_total_sz, cmd->qcount);
        return FTL_RC_ENOSPC;
    }

    single_cmd.wring_base_addr = cmd->wrings_base_addr;
    single_cmd.wring_sz = wring_single_sz;
    single_cmd.index = 0;
    while (single_cmd.index < cmd->qcount) {
        status = poller_init_single(&single_cmd);
        if (status != FTL_RC_SUCCESS) {
            return status;
        }

        single_cmd.wring_base_addr += wring_single_sz;
        single_cmd.index++;
    }

    NIC_LOG_DEBUG("{}: qtype {} qid_high {}", lif.LifNameGet(),
                  cmd->qtype, qid_high_);
    return FTL_RC_SUCCESS;
}

ftl_status_code_t
ftl_lif_queues_ctl_t::start(void)
{
    void                (*cb_activate)(const mem_access_t *access);
    const mem_access_t  *qstate_access;

    switch (qtype()) {

    case FTL_QTYPE_SCANNER_SESSION:
    case FTL_QTYPE_SCANNER_CONNTRACK:
        cb_activate = &scanner_session_cb_activate;
        break;

    case FTL_QTYPE_POLLER:
        cb_activate = &poller_cb_activate;
        break;

    default:
        cb_activate = nullptr;
        break;
    }

    if (cb_activate && qcount_) {

        for (uint32_t qid = 0; qid <= qid_high_; qid++) {
            qstate_access = qid_qstate_access(qid);
            if (!qstate_access) {
                return FTL_RC_EFAULT;
            }

            (*cb_activate)(qstate_access);
            sched_start_single(qid);
        }
    }

    return FTL_RC_SUCCESS;
}

ftl_status_code_t
ftl_lif_queues_ctl_t::sched_start_single(uint32_t qid)
{
    uint32_t        db_data;

    if (qid >= qcount()) {
        NIC_LOG_ERR("{}:qid {} exceeds qcount {}",
                    lif.LifNameGet(), qid, qcount());
        return FTL_RC_EQID;
    }

    switch (qtype()) {

    case FTL_QTYPE_SCANNER_SESSION:
    case FTL_QTYPE_SCANNER_CONNTRACK:

        /*
         * Doorbell update with a pndx increment
         */
        db_data = FTL_LIF_DBDATA32_SET(qid, 0);
        db_pndx_inc.write32(db_data);
        break;

    default:

        /*
         * Software queues don't use scheduler
         */
        break;
    }

    return FTL_RC_SUCCESS;
}

ftl_status_code_t
ftl_lif_queues_ctl_t::stop(void)
{
    void                (*cb_deactivate)(const mem_access_t *access);
    const mem_access_t  *qstate_access;
    ftl_status_code_t   status = FTL_RC_SUCCESS;

    switch (qtype()) {

    case FTL_QTYPE_SCANNER_SESSION:
    case FTL_QTYPE_SCANNER_CONNTRACK:
        cb_deactivate = &scanner_session_cb_deactivate;
        break;

    case FTL_QTYPE_POLLER:
        cb_deactivate = &poller_cb_deactivate;
        break;

    default:
        cb_deactivate = nullptr;
        break;
    }

    if (cb_deactivate && qcount_) {

        for (uint32_t qid = 0; qid <= qid_high_; qid++) {
            qstate_access = qid_qstate_access(qid);
            if (!qstate_access) {

                /*
                 * continue to try and stop as many queues as possible
                 */
                status = FTL_RC_EFAULT;
                continue;
            }

            (*cb_deactivate)(qstate_access);
            sched_stop_single(qid);
        }
    }

    return status;
}

ftl_status_code_t
ftl_lif_queues_ctl_t::sched_stop_single(uint32_t qid)
{
    uint32_t        db_data;

    if (qid >= qcount()) {
        NIC_LOG_ERR("{}:qid {} exceeds qcount {}",
                    lif.LifNameGet(), qid, qcount());
        return FTL_RC_EQID;
    }

    switch (qtype()) {

    case FTL_QTYPE_SCANNER_SESSION:
    case FTL_QTYPE_SCANNER_CONNTRACK:

        /*
         * Doorbell update clear
         */
        db_data = FTL_LIF_DBDATA32_SET(qid, 0);
        db_shed_clr.write32(db_data);
        break;

    default:

        /*
         * Software queues don't use scheduler
         */
        break;
    }

    return FTL_RC_SUCCESS;
}

ftl_status_code_t
ftl_lif_queues_ctl_t::dequeue_burst(uint32_t qid,
                                    uint32_t *burst_count,
                                    uint8_t *buf,
                                    uint32_t buf_sz)
{
    const mem_access_t  *qstate_access;
    const mem_access_t  *wring_access;
    uint32_t            slot_offset;
    qstate_1ring_cb_t   qstate_1ring_cb;
    uint32_t            avail_count;
    uint32_t            read_count;
    uint32_t            total_read_sz;
    uint32_t            max_read_sz;
    uint32_t            read_sz;
    uint32_t            cndx;
    uint32_t            pndx;

    qstate_access = qid_qstate_access(qid);
    wring_access = qid_wring_access(qid);
    if (!qstate_access || !wring_access) {
        return FTL_RC_EQID;
    }

    if (qdepth && slot_data_sz) {
        qstate_access->small_read(offsetof(qstate_1ring_cb_t, p_ndx0),
                                  (uint8_t *)&qstate_1ring_cb.p_ndx0,
                                  sizeof(qstate_1ring_cb.p_ndx0) +
                                  sizeof(qstate_1ring_cb.c_ndx0));
        cndx = qstate_1ring_cb.c_ndx0 & qdepth_mask;
        pndx = qstate_1ring_cb.p_ndx0 & qdepth_mask;
        if (cndx == pndx) {
            *burst_count = 0;
            return FTL_RC_SUCCESS;
        }

        avail_count = pndx > cndx ?
                      pndx - cndx : (pndx + qdepth) - cndx;
        read_count = std::min(avail_count, *burst_count);
        total_read_sz = read_count * slot_data_sz;
        if (!buf || (buf_sz < total_read_sz)) {
            NIC_LOG_ERR("{}: total_read_sz {} exceeds buf_sz {}",
                        lif.LifNameGet(), total_read_sz, buf_sz);
            return FTL_RC_EINVAL;
        }

        /*
         * Handle ring wrap during read
         */
        slot_offset = cndx * slot_data_sz;
        max_read_sz = (qdepth - cndx) * slot_data_sz;
        read_sz = std::min(total_read_sz, max_read_sz);
        wring_access->large_read(slot_offset, buf, read_sz);

        total_read_sz -= read_sz;
        buf += read_sz;
        if (total_read_sz) {

            /*
             * Wrap once to start of ring
             */
            wring_access->large_read(0, buf, total_read_sz);
        }

        qstate_1ring_cb.c_ndx0 = (cndx + read_count) & qdepth_mask;
        qstate_access->small_write(offsetof(qstate_1ring_cb_t, c_ndx0),
                                   (uint8_t *)&qstate_1ring_cb.c_ndx0,
                                   sizeof(qstate_1ring_cb.c_ndx0));
        qstate_access->cache_invalidate();
        *burst_count = read_count;
        return FTL_RC_SUCCESS;
    }

    return FTL_RC_EIO;
}

bool
ftl_lif_queues_ctl_t::quiesce(void)
{
    const mem_access_t  *qstate_access;
    qstate_1ring_cb_t   qstate_1ring_cb;

    if (!quiescing) {
        quiescing = true;
        quiesce_qid_ = 0;
    }

    switch (qtype()) {

    case FTL_QTYPE_SCANNER_SESSION:
    case FTL_QTYPE_SCANNER_CONNTRACK:

        for (; qid_high_ && (quiesce_qid_ <= qid_high_); quiesce_qid_++) {
            qstate_access = qid_qstate_access(quiesce_qid_);
            if (!qstate_access) {
                continue;
            }
            qstate_access->small_read(offsetof(qstate_1ring_cb_t, p_ndx0),
                                      (uint8_t *)&qstate_1ring_cb.p_ndx0,
                                      sizeof(qstate_1ring_cb.p_ndx0) +
                                      sizeof(qstate_1ring_cb.c_ndx0));
            /*
             * As part of deactivate, MPU would set c_ndx = p_ndx
             */
            if (qstate_1ring_cb.c_ndx0 != qstate_1ring_cb.p_ndx0) {
                return false;
            }
        }
        break;

    default:
        break;
    }

    return true;
}

void
ftl_lif_queues_ctl_t::quiesce_idle(void)
{
    quiescing = false;
}

ftl_status_code_t
ftl_lif_queues_ctl_t::metrics_get(lif_attr_metrics_t *metrics)
{
    const mem_access_t  *qstate_access;
    uint64_t            min_elapsed_ticks;
    uint64_t            max_elapsed_ticks;
    uint64_t            total_min_elapsed_ticks;
    uint64_t            total_max_elapsed_ticks;
    uint32_t            curr_range_sz;
    uint32_t            avg_count;
    ftl_status_code_t   status = FTL_RC_SUCCESS;

    memset(metrics, 0, sizeof(*metrics));
    if (qcount_) {
        min_elapsed_ticks = (uint64_t)~0;
        max_elapsed_ticks = 0;
        total_min_elapsed_ticks = 0;
        total_max_elapsed_ticks = 0;
        curr_range_sz = 0;
        avg_count = 0;

        for (uint32_t qid = 0; qid <= qid_high_; qid++) {
            qstate_access = qid_qstate_access(qid);
            if (!qstate_access) {

                /*
                 * continue to try with as many queues as possible
                 */
                status = FTL_RC_EFAULT;
                continue;
            }

            switch (qtype()) {

            case FTL_QTYPE_SCANNER_SESSION:
            case FTL_QTYPE_SCANNER_CONNTRACK: {
                scanner_session_qstate_t scanner_qstate;

                qstate_access->small_read(0, (uint8_t *)&scanner_qstate,
                                          sizeof(scanner_qstate));
                metrics->scanners.total_expired_entries  +=
                         scanner_qstate.metrics0.expired_entries;
                metrics->scanners.total_scan_invocations +=
                         scanner_qstate.metrics0.scan_invocations;
                metrics->scanners.total_cb_cfg_discards  +=
                         scanner_qstate.metrics0.cb_cfg_discards;
                /*
                 * Only include in calcs for averages if queues have the same
                 * range size (all queues of the same qtype do except maybe
                 * the last queue).
                 */
                if (((qid == 0) || 
                     (curr_range_sz == scanner_qstate.fsm.scan_range_sz)) &&

                    /*
                     * qstate min_range_elapsed_ticks had been initialized to ~0
                     * so skip the entry if it were never updated.
                     */
                    (scanner_qstate.metrics0.min_range_elapsed_ticks != (uint64_t)~0)) {

                    avg_count++;
                    min_elapsed_ticks = std::min(min_elapsed_ticks,
                                        scanner_qstate.metrics0.min_range_elapsed_ticks);
                    max_elapsed_ticks = std::max(max_elapsed_ticks,
                                        scanner_qstate.metrics0.max_range_elapsed_ticks);
                    total_min_elapsed_ticks +=
                          scanner_qstate.metrics0.min_range_elapsed_ticks;
                    total_max_elapsed_ticks +=
                          scanner_qstate.metrics0.max_range_elapsed_ticks;
                }
                curr_range_sz = scanner_qstate.fsm.scan_range_sz;
                break;
            }

            case FTL_QTYPE_POLLER: {
                poller_qstate_t poller_qstate;

                qstate_access->small_read(0, (uint8_t *)&poller_qstate,
                                          sizeof(poller_qstate));
                metrics->pollers.total_num_qposts += poller_qstate.num_qposts;
                metrics->pollers.total_num_qfulls += poller_qstate.num_qfulls;
                break;
            }

            default:
                return FTL_RC_SUCCESS;
            }
        }

        if (avg_count) {
            if (min_elapsed_ticks != (uint64_t)~0) {
                metrics->scanners.min_range_elapsed_ns =
                         hw_coreclk_ticks_to_time_ns(min_elapsed_ticks);
            }
            metrics->scanners.max_range_elapsed_ns =
                     hw_coreclk_ticks_to_time_ns(max_elapsed_ticks);
            metrics->scanners.avg_min_range_elapsed_ns =
                     hw_coreclk_ticks_to_time_ns(total_min_elapsed_ticks / avg_count);
            metrics->scanners.avg_max_range_elapsed_ns =
                     hw_coreclk_ticks_to_time_ns(total_max_elapsed_ticks / avg_count);
        }
    }

    return status;
}


ftl_status_code_t
ftl_lif_queues_ctl_t::scanner_init_single(const scanner_init_single_cmd_t *cmd)
{
    int64_t                 qstate_addr;
    int64_t                 poller_qstate_addr;
    uint32_t                qid = cmd->index;
    scanner_session_qstate_t qstate = {0};
    mem_access_t            qs_access(lif);
    ftl_status_code_t       status;
    uint8_t                 pc_offset;

    if (qid >= qcount()) {
        NIC_LOG_ERR("{}: qid {} exceeds max {}", lif.LifNameGet(),
                    qid, qcount());
        return FTL_RC_EQID;
    }

    qstate_addr = qid_qstate_addr(qid);
    if (qstate_addr < 0) {
        return FTL_RC_ERROR;
    }

    /*
     * Init doorbell accesses
     */
    db_pndx_inc.reset(qtype(),
                      ASIC_DB_ADDR_UPD_FILL(ASIC_DB_UPD_SCHED_COSB,
                                            ASIC_DB_UPD_INDEX_INCR_PINDEX,
                                            false));
    db_shed_clr.reset(qtype(),
                      ASIC_DB_ADDR_UPD_FILL(ASIC_DB_UPD_SCHED_COSA,
                                            ASIC_DB_UPD_INDEX_UPDATE_NONE,
                                            false));
    /*
     * Scanner queues init/start/stop are carried out from only one
     * single thread at a time. However, because PAL lib is not
     * thread safe, mmap is req is still required for qstate access.
     */
    qs_access.reset(qstate_addr, sizeof(scanner_session_qstate_t));

    qid_high_ = std::max(qid_high_, qid);
    status = pgm_pc_offset_get("scanner_session_stage0", &pc_offset);
    if (status != FTL_RC_SUCCESS) {
        return status;
    }
    qstate.cb.qstate_1ring.pc_offset = pc_offset;
    qstate.cb.qstate_1ring.eval_last = 1 << 0;
    qstate.cb.qstate_1ring.cosB = cmd->cos_override ? cmd->cos : lif.cosB;
    qstate.cb.qstate_1ring.host_wrings = 0;
    qstate.cb.qstate_1ring.total_wrings = 1;
    qstate.cb.qstate_1ring.pid = cmd->pid;

    qstate.cb.normal_tmo_cb_addr = lif.normal_age_access().pa();
    qstate.cb.accel_tmo_cb_addr = lif.accel_age_access().pa();
    qstate.cb.scan_resched_ticks =
           time_us_to_txs_sched_ticks(cmd->scan_resched_time,
                                      &qstate.cb.resched_uses_slow_timer);
    /*
     * burst size may be zero but range size must be > 0
     */
    assert(cmd->scan_range_sz);
    qstate.fsm.scan_range_sz = cmd->scan_range_sz;
    if (cmd->scan_burst_sz) {

        /*
         * round up burst size to next power of 2
         */
        qstate.fsm.scan_burst_sz = cmd->scan_burst_sz;
        qstate.fsm.scan_burst_sz_shft = log_2(cmd->scan_burst_sz);
    }
    qstate.fsm.fsm_state = SCANNER_STATE_INITIAL;
    qstate.fsm.scan_id_base = cmd->scan_id_base;
    qstate.fsm.scan_id_next = cmd->scan_id_base;
    qstate.fsm.scan_addr_base = cmd->scan_addr_base;

    qstate.summarize.poller_qdepth_shft = cmd->poller_qdepth_shft;
    poller_qstate_addr = lif.pd->lm_->get_lif_qstate_addr(cmd->poller_lif,
                                      cmd->poller_qtype, cmd->poller_qid);
    if (poller_qstate_addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for poller "
                    "lif {} qtype {} qid {}", lif.LifNameGet(),
                    cmd->poller_lif, cmd->poller_qtype, cmd->poller_qid);
        return FTL_RC_ERROR;
    }
    qstate.summarize.poller_qstate_addr = poller_qstate_addr;
    qstate.metrics0.min_range_elapsed_ticks = ~qstate.metrics0.min_range_elapsed_ticks;
    qs_access.small_write(0, (uint8_t *)&qstate, sizeof(qstate));
    qs_access.cache_invalidate();

    qstate_access.push_back(std::move(qs_access));
    return FTL_RC_SUCCESS;
}

ftl_status_code_t
ftl_lif_queues_ctl_t::poller_init_single(const poller_init_single_cmd_t *cmd)
{
    int64_t                 qstate_addr;
    uint32_t                qid = cmd->index;
    mem_access_t            qs_access(lif);
    mem_access_t            wr_access(lif);
    poller_qstate_t         qstate = {0};

    if (qid >= qcount()) {
        NIC_LOG_ERR("{}: qid {} exceeds max {}", lif.LifNameGet(),
                    qid, qcount());
        return FTL_RC_EQID;
    }

    qstate_addr = qid_qstate_addr(qid);
    if (qstate_addr < 0) {
        return FTL_RC_ERROR;
    }

    /*
     * Poller queues will be polled from multiple threads so mmap
     * is required for qstate access (since PAL lib is not thread safe)
     */
    qs_access.reset(qstate_addr, sizeof(poller_qstate_t));

    qid_high_ = std::max(qid_high_, qid);
    qstate.qstate_1ring.host_wrings = 0;
    qstate.qstate_1ring.total_wrings = 1;
    qstate.qstate_1ring.pid = cmd->pid;
    qstate.qdepth_shft = cmd->qdepth_shft;
    qstate.wring_base_addr = cmd->wring_base_addr;
    qs_access.small_write(0, (uint8_t *)&qstate, sizeof(qstate));
    qs_access.cache_invalidate();

    qstate_access.push_back(std::move(qs_access));

    /*
     * Similarly, mmap the wring as required
     */
    wr_access.reset(qstate.wring_base_addr, cmd->wring_sz);
    wring_access.push_back(std::move(wr_access));
    return FTL_RC_SUCCESS;
}

int64_t
ftl_lif_queues_ctl_t::qid_qstate_addr(uint32_t qid)
{
    int64_t     qstate_addr;

    qstate_addr = qid < qcount() ?
                  lif.pd->lm_->get_lif_qstate_addr(lif.LifIdGet(), qtype(), qid) :
                  -1;
    if (qstate_addr < 0) {
        NIC_LOG_ERR("{}: Failed to get qstate address for qtype {} qid {}",
                    lif.LifNameGet(), qtype(), qid);
    }

    return qstate_addr;
}

const mem_access_t *
ftl_lif_queues_ctl_t::qid_qstate_access(uint32_t qid)
{
    const mem_access_t  *access;

    access = qid < (uint32_t)qstate_access.size() ?
             &qstate_access.at(qid) : nullptr;
    if (!access) {
        NIC_LOG_ERR("{}: Failed to get qstate access for qtype {} qid {}",
                    lif.LifNameGet(), qtype(), qid);
    }

    return access;
}

const mem_access_t *
ftl_lif_queues_ctl_t::qid_wring_access(uint32_t qid)
{
    const mem_access_t  *access;

    access = qid < (uint32_t)wring_access.size() ?
             &wring_access.at(qid) : nullptr;
    if (!access) {
        NIC_LOG_ERR("{}: Failed to get wring access for qtype {} qid {}",
                    lif.LifNameGet(), qtype(), qid);
    }

    return access;
}

ftl_status_code_t
ftl_lif_queues_ctl_t::pgm_pc_offset_get(const char *pc_jump_label,
                                        uint8_t *pc_offset)
{
    if (lif.pd->get_pc_offset("txdma_stage0.bin", pc_jump_label,
                              pc_offset, NULL) < 0) {
        NIC_LOG_ERR("{}: Failed to get PC offset of jump label: {}",
                    lif.LifNameGet(), pc_jump_label);
        return FTL_RC_ERROR;
    }
    return FTL_RC_SUCCESS;
}

/*
 * Miscelaneous utility functions
 */
static void
poller_cb_activate(const mem_access_t *access)
{
    /*
     * SW queues don't use cb_activate
     */
}

static void
poller_cb_deactivate(const mem_access_t *access)
{
    qstate_1ring_cb_t   qstate_1ring_cb;

    /*
     * SW queues don't have the benefit of MPU setting CI=PI on
     * deactivate so must be done inline here.
     */
    access->small_read(offsetof(qstate_1ring_cb_t, p_ndx0),
                       (uint8_t *)&qstate_1ring_cb.p_ndx0,
                       sizeof(qstate_1ring_cb.p_ndx0) +
                       sizeof(qstate_1ring_cb.c_ndx0));
    qstate_1ring_cb.c_ndx0 = qstate_1ring_cb.p_ndx0;
    access->small_write(offsetof(qstate_1ring_cb_t, c_ndx0),
                        (uint8_t *)&qstate_1ring_cb.c_ndx0,
                        sizeof(qstate_1ring_cb.c_ndx0));
    access->cache_invalidate();
}

static void
scanner_session_cb_activate(const mem_access_t *access)
{
    scanner_session_cb_activate_t   activate = SCANNER_SESSION_CB_ACTIVATE;

    /*
     * Activate the CB sections in this order: summarize, fsm, cb
     */
    access->small_write(offsetof(scanner_session_qstate_t, summarize) +
                        offsetof(scanner_session_summarize_t, cb_activate),
                        (uint8_t *)&activate, sizeof(activate));
    access->small_write(offsetof(scanner_session_qstate_t, fsm) +
                        offsetof(scanner_session_fsm_t, cb_activate),
                        (uint8_t *)&activate, sizeof(activate));
    access->small_write(offsetof(scanner_session_qstate_t, cb) +
                        offsetof(scanner_session_cb_t, cb_activate),
                        (uint8_t *)&activate, sizeof(activate));
    access->cache_invalidate();
}

static void
scanner_session_cb_deactivate(const mem_access_t *access)
{
    scanner_session_cb_activate_t   deactivate =
            (scanner_session_cb_activate_t)~SCANNER_SESSION_CB_ACTIVATE;

    /*
     * Deactivate the CB sections in this order: cb, fsm, summarize
     */
    access->small_write(offsetof(scanner_session_qstate_t, cb) +
                        offsetof(scanner_session_cb_t, cb_activate),
                        (uint8_t *)&deactivate, sizeof(deactivate));
    access->small_write(offsetof(scanner_session_qstate_t, fsm) +
                        offsetof(scanner_session_fsm_t, cb_activate),
                        (uint8_t *)&deactivate, sizeof(deactivate));
    access->small_write(offsetof(scanner_session_qstate_t, summarize) +
                        offsetof(scanner_session_summarize_t, cb_activate),
                    (uint8_t *)&deactivate, sizeof(deactivate));
    access->cache_invalidate();
}

/*
 * Memory access wrapper
 */
mem_access_t::~mem_access_t()
{
    if (vaddr) {
        sdk::lib::pal_mem_unmap((void *)vaddr);
        vaddr = nullptr;
    }
}

void
mem_access_t::reset(int64_t new_paddr,
                    uint32_t new_sz,
                    bool mmap_requested)
{
    if (vaddr) {
        sdk::lib::pal_mem_unmap((void *)vaddr);
        vaddr = nullptr;
    }
    paddr = new_paddr;
    total_sz = new_sz;

    if (platform_is_hw(lif.pd->platform_) && mmap_requested) {
        vaddr = (volatile uint8_t *)sdk::lib::pal_mem_map(paddr, total_sz);
        if (!vaddr) {
            NIC_LOG_ERR("{}: memory map error addr {:#x} total_sz {}",
                        lif.LifNameGet(), paddr, total_sz);
            throw;
        }
    }
}

void
mem_access_t::small_read(uint32_t offset,
                         uint8_t *buf,
                         uint32_t read_sz) const
{
    if ((offset + read_sz) > total_sz) {
        NIC_LOG_ERR("{}: offset {} + read_sz {} > total_sz {}",
                    lif.LifNameGet(), offset, read_sz, total_sz);
        throw;
    }

    if (vaddr) {
        memcpy(buf, (void *)(vaddr + offset), read_sz);
    } else {
        sdk::asic::asic_mem_read(paddr + offset, buf, read_sz);
    }
}

void
mem_access_t::small_write(uint32_t offset,
                          const uint8_t *buf,
                          uint32_t write_sz) const
{
    if ((offset + write_sz) > total_sz) {
        NIC_LOG_ERR("{}: offset {} + write_sz {} > total_sz {}",
                    lif.LifNameGet(), offset, write_sz, total_sz);
        throw;
    }

    if (vaddr) {
        memcpy((void *)(vaddr + offset), buf, write_sz);
    } else {
        sdk::asic::asic_mem_write(paddr + offset, (uint8_t *)buf, write_sz);
    }
}

/*
 * Large read/write, breaking into multiple reads or writes as necessary.
 */
void
mem_access_t::large_read(uint32_t offset,
                         uint8_t *buf,
                         uint32_t read_sz) const
{
    uint32_t    curr_sz;

    /*
     * With vaddr, no need to do any breakup
     */
    if (vaddr) {
        small_read(offset, buf, read_sz);
    } else {
        while (read_sz) {
            curr_sz = std::min(read_sz, (uint32_t)FTL_DEV_HBM_RW_LARGE_BYTES_MAX);
            small_read(offset, buf, curr_sz);
            offset += curr_sz;
            buf += curr_sz;
            read_sz -= curr_sz;
        }
    }
}

void
mem_access_t::large_write(uint32_t offset,
                          const uint8_t *buf,
                          uint32_t write_sz) const
{
    uint32_t    curr_sz;

    /*
     * With vaddr, no need to do any breakup
     */
    if (vaddr) {
        small_write(offset, buf, write_sz);
    } else {
        while (write_sz) {
            curr_sz = std::min(write_sz, (uint32_t)FTL_DEV_HBM_RW_LARGE_BYTES_MAX);
            small_write(offset, buf, curr_sz);
            offset += curr_sz;
            buf += curr_sz;
            write_sz -= curr_sz;
        }
    }
}

void
mem_access_t::cache_invalidate(uint32_t offset,
                               uint32_t sz) const
{
    if (!sz) {
        sz = offset < total_sz ?
             total_sz - offset : 0;
    }
    if ((offset + sz) > total_sz) {
        NIC_LOG_ERR("{}: offset {} + sz {} > total_sz {}",
                    lif.LifNameGet(), offset, sz, total_sz);
        throw;
    }

    /*
     * Note that p4plus_invalidate_cache() also uses vaddr to
     * access the required invalidate register.
     */
    PAL_barrier();
    sdk::asic::pd::asicpd_p4plus_invalidate_cache(paddr + offset, sz, P4PLUS_CACHE_INVALIDATE_TXDMA);
}

/*
 * Doorbell access wrapper
 */
void
db_access_t::reset(enum ftl_qtype qtype,
                   uint32_t upd)
{
    uint64_t    paddr;

    db_addr = {0};
    db_addr.lif_id = lif.LifIdGet();
    db_addr.q_type = (uint8_t)qtype;
    db_addr.upd = upd;

    /*
     * Use 32-bit doorbells from ARM to benefit from Capri atomic 32-bit
     * register writes. Note that this works as long as PID check isn't
     * used and number of rings is 1 for the LIF. Also, number of queues
     * must be 64K or less.
     */
    paddr = sdk::asic::pd::asic_localdb32_addr(db_addr.lif_id,
                                               db_addr.q_type,
                                               db_addr.upd);
    db_access.reset(paddr, sizeof(uint32_t));
}

void
db_access_t::write32(uint32_t data)
{
    volatile uint32_t *db = (volatile uint32_t *)db_access.va();

    PAL_barrier();
    if (db) {
        *db = data;
    } else {
        sdk::asic::pd::asic_ring_db(&db_addr, data);
    }
}

/*
 * FSM state/event value to string conversion
 */
static const char *
lif_state_str(ftl_lif_state_t state)
{
    if (state < FTL_LIF_ST_MAX) {
        return lif_state_str_table[state];
    }
    return "unknown_state";
}

static const char *
lif_event_str(ftl_lif_event_t event)
{
    if (event < FTL_LIF_EV_MAX) {
        return lif_event_str_table[event];
    }
    return "unknown_event";
}

