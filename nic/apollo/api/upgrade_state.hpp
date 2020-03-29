//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines upgrade state for PDS
///
//----------------------------------------------------------------------------

#ifndef __API_UPGRADE_STATE_HPP__
#define __API_UPGRADE_STATE_HPP__

#include <list>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/lib/shmmgr/shmmgr.hpp"
#include "nic/apollo/upgrade/include/event_cb.hpp"
#include "nic/apollo/api/include/pds_upgrade.hpp"
#include "nic/apollo/api/include/pds_init.hpp"

namespace api {

/// \brief upgrade preserved
/// saved in shared memory to access after process restart.
/// as this is used across upgrades, all the modifications should be
/// done to the end
typedef struct __attribute__((packed)) upg_pstate_s {
    pds_memory_profile_t memory_profile; ///< running profile
    uint32_t service_lif_id;           ///< running service lif id
} upg_pstate_t;

/// \brief qstate info
typedef struct qstate_cfg_s {
    uint64_t addr;
    uint32_t size;
    uint32_t pgm_off;
} qstate_cfg_t;

/// \defgroup PDS_UPGRADE PDS Upgrade
/// \ingroup  UPGRADE_STATE
/// @{

/// \brief state maintained for upgrade
// singleton class
class upg_state {
public:
    /// \brief constructor
    upg_state() {}

    /// \brief destructor
    ~upg_state() {}

    /// \brief singleton factory method to instantiate the upgrade state
    /// \param[in] shm_create create/open the shared memory instance based on this
    /// \return NULL if there is a failure, pointer to state if success
    static upg_state *factory(bool shm_create = true);

    /// \brief destroy the upgrade state
    /// \param[in] state destroy the upgrade state
    static void destroy(upg_state *state);

    // shared memory manager. all the states are saved on this during upgrade
    shmmgr *shm_mgr(void) { return shm_mmgr_; }
    // compare the profile of the running with the new by saving it in shared memory
    void set_memory_profile(pds_memory_profile_t profile) {pstate_->memory_profile = profile; }
    pds_memory_profile_t memory_profile(void) { return pstate_->memory_profile; }
    // table engine configuration. will be extracted during pre-upgrade and will be
    // applied during the final stage of the upgrade.
    uint32_t tbl_eng_cfg(p4pd_pipeline_t pipe, p4_tbl_eng_cfg_t **cfg, uint32_t *max_cfgs);
    void incr_tbl_eng_cfg_count(p4pd_pipeline_t pipe, uint32_t ncfgs);
    // service lif id. will be saved in shared memory and compared during upgrade
    uint32_t service_lif_id(void) { return pstate_->service_lif_id; }
    // qstate config info. only pc_offset will be modified during upgrade
    // pc_offset will be exctracted during pre_upgrade and will be applied during
    // the final stage of the upgrade
    void set_qstate_cfg(uint64_t addr, uint32_t size, uint32_t pgm_off);
    std::list<qstate_cfg_t> &qstate_cfg(void) { return qstate_cfgs_; }
    // table engine rss configuration
    void tbl_eng_rss_cfg(p4_tbl_eng_cfg_t **cfg) { *cfg = &tbl_eng_cfg_rss_; }
    // last completed upgrade specification
    upg_stage_t last_stage(void) { return last_spec_.stage; }
    void set_spec(pds_upg_spec_t *spec) { last_spec_ = *spec; }
    // register event threads
    // don't change the push_front as pipeline event will be registered first
    // and it should be executed last
    void register_ev_thread(upg::upg_event_t &ev) { ev_threads_.push_front(ev); }
    std::list<upg::upg_event_t> &ev_threads(void) { return ev_threads_; }

private:
    /// last successfully completed spec request
    pds_upg_spec_t   last_spec_;
    /// shared memory manager
    shmmgr           *shm_mmgr_;
    /// preserved state
    upg_pstate_t     *pstate_;
    /// lif qstate mpu program offset map
    std::list<qstate_cfg_t> qstate_cfgs_;
    /// table engine configs. saved during upgrade init and applied during switch
    p4_tbl_eng_cfg_t tbl_eng_cfgs_[P4_PIPELINE_MAX][P4TBL_ID_MAX];
    /// number of valid entries in table engine config
    uint32_t tbl_eng_cfgs_count_[P4_PIPELINE_MAX];
    /// rss table engine config requires special handling in capri programming
    p4_tbl_eng_cfg_t tbl_eng_cfg_rss_;
    /// upgrade event callbacks registered by pds threads
    std::list<upg::upg_event_t> ev_threads_;
private:
    sdk_ret_t init_(bool create);
};

extern upg_state *g_upg_state;

/// @}

}    // namespace api
#endif    // __API_UPGRADE_STATE_HPP__
