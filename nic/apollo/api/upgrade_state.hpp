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

#include "include/sdk/base.hpp"
#include "nic/sdk/lib/shmmgr/shmmgr.hpp"
#include "nic/apollo/api/include/pds_upgrade.hpp"
#include "nic/apollo/api/include/pds_init.hpp"

namespace upg {

/// \brief upgrade preserved
/// saved in shared memory to access after process restart.
/// as this is used across upgrades, all the modifications should be
/// done to the end
typedef struct __attribute__((packed)) upg_pstate_s {
    pds_scale_profile_t scale_profile; ///< running profile
} upg_pstate_t;

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

    /// \brief singleton factory method to get the upgrade state
    /// \return NULL if it is not allocated, pointer to state otherwise
    static upg_state *get_instance(void);
    shmmgr *shm_mgr(void) { return shm_mmgr_; }
    void set_scale_profile(pds_scale_profile_t profile) {pstate_->scale_profile = profile; }
    pds_scale_profile_t scale_profile(void) { return pstate_->scale_profile; }
private:
    static upg_state *upg_state_;      ///< singleton upgrate state
    pds_upg_spec_t   spec_;            ///< ongoing upgrade spec request
    shmmgr           *shm_mmgr_;       ///< shared memory manager
    upg_pstate_t     *pstate_;         ///< preserved state
private:
    sdk_ret_t init_(bool create);
};

/// @}

}    // namespace upg
#endif    // __API_UPGRADE_STATE_HPP__
