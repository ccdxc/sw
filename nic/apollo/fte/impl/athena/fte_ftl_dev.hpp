//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
///----------------------------------------------------------------------------
///
/// \file
/// FTE Flow Table Library device handling
///
///----------------------------------------------------------------------------

#ifndef __FTE_FTL_DEV_HPP__
#define __FTE_FTL_DEV_HPP__

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <endian.h>
#include <sstream>
#include <sys/time.h>

#include <rte_spinlock.h>
#include <rte_atomic.h>

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/sdk/lib/ipc/ipc.hpp"

#include "nic/apollo/nicmgr/nicmgr.hpp"
#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "platform/src/lib/nicmgr/include/ftl_dev.hpp"
#include "platform/src/lib/nicmgr/include/ftl_lif.hpp"

/*
 * Max devcmd retry timeout
 */
#define FTE_FTL_DEVCMD_RETRY_TMO_US         (60 * USEC_PER_SEC)

namespace fte_ftl_dev {

sdk_ret_t init(unsigned int lcore_id);
sdk_ret_t scanners_start(void);
sdk_ret_t scanners_stop(bool quiesce_check);
sdk_ret_t scanners_start_single(enum ftl_qtype qtype,
                                uint32_t qid);
sdk_ret_t pollers_flush(void);
sdk_ret_t normal_timeouts_set(const pds_flow_age_timeouts_t *age_tmo);
sdk_ret_t accel_timeouts_set(const pds_flow_age_timeouts_t *age_tmo);
sdk_ret_t accel_aging_control(bool enable_sense);

bool lif_init_done(void);

class devcmd_t;

/**
 * Queues control class
 */
class lif_queues_ctl_t {
public:
    lif_queues_ctl_t(enum ftl_qtype qtype,
                     uint32_t qcount,
                     uint32_t qdepth);
    ~lif_queues_ctl_t();

    sdk_ret_t init(devcmd_t *owner_devcmd = nullptr);
    sdk_ret_t start(devcmd_t *owner_devcmd = nullptr);
    sdk_ret_t stop( bool quiesce_check,
                    devcmd_t *owner_devcmd = nullptr);
    sdk_ret_t start_single(uint32_t qid);
    sdk_ret_t flush(void);

    void lock(uint32_t qid);
    void unlock(uint32_t qid);
    void lock_all(void);
    void unlock_all(void);

private:
    enum ftl_qtype          qtype;
    uint32_t                qcount;
    uint32_t                qdepth;
    rte_spinlock_t          *spinlocks;

    sdk_ret_t pollers_init(devcmd_t *devcmd);
    sdk_ret_t scanners_init(devcmd_t *devcmd);
};

/**
 * Devcmd interface wrapper
 * 
 * A note about devcmd locking: the FTL device in nicmgr implements a 
 * state machine to maintain operation correctness.
 * 
 * Consider one example: Thread A has, for some reason, invoked a
 * scanners-stop-all command. Some time after, poller thread B
 * executes a single-queue scanner reschedule. The nicmgr state machine
 * will correctly turn the reschedule into a no-op. But for all this
 * to work, proper state machine locking would be required. However,
 * locking inside nicmgr would be coarse and inefficient. Instead, this
 * will be left to the responsibilty of the caller, where it might have
 * better control with regards to more fine grained locking.
 */
typedef void (*spinlock_enter_t)(void *user_arg,
                                 uint32_t user_idx);
typedef void (*spinlock_leave_t)(void *user_arg,
                                 uint32_t user_idx);
class devcmd_t {
public:
    devcmd_t(FtlDev *ftl_dev,
             spinlock_enter_t spinlock_enter = nullptr,
             spinlock_leave_t spinlock_leave = nullptr,
             void *user_arg = nullptr,
             uint32_t user_idx = 0);
    devcmd_t(FtlLif *ftl_lif,
             spinlock_enter_t spinlock_enter = nullptr,
             spinlock_leave_t spinlock_leave = nullptr,
             void *user_arg = nullptr,
             uint32_t user_idx = 0);

    void req_clr(void) { req_ = {0}; }
    void rsp_clr(void) { rsp_ = {0}; }

    ftl_devcmd_t& req(void) { return req_; }
    ftl_devcmd_cpl_t& rsp(void) { return rsp_; }

    sdk_ret_t owner_pre_lock(void);
    sdk_ret_t owner_pre_unlock(void);
    sdk_ret_t submit(void *req_data = nullptr,
                     void *rsp_data = nullptr);
    sdk_ret_t submit_with_retry(void *req_data = nullptr,
                                void *rsp_data = nullptr);

private:
    FtlDev                      *ftl_dev;
    FtlLif                      *ftl_lif;
    spinlock_enter_t            spinlock_enter;
    spinlock_leave_t            spinlock_leave;
    void                        *user_arg;
    uint32_t                    user_idx;
    bool                        pre_locked;
    ftl_devcmd_t                req_;
    ftl_devcmd_cpl_t            rsp_;

    sdk_ret_t cmd_handler(void *req_data,
                          void *rsp_data);
};

} // namespace fte_ftl_dev

#endif // __FTE_FTL_DEV_HPP__
