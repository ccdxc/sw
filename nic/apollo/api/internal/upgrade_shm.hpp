//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines upgrade shared memory states for PDS
///
//----------------------------------------------------------------------------

#ifndef __API_UPGRADE_SHM_HPP__
#define __API_UPGRADE_SHM_HPP__

#include <list>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/sdk/lib/shmmgr/shmmgr.hpp"
#include "nic/apollo/api/include/pds_upgrade.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/apollo/api/internal/upgrade_ev.hpp"
#include "nic/apollo/api/internal/upgrade_ctxt.hpp"

namespace api {

class upg_shm {
public:
    /// \brief factory method to initialize
    /// \param[in] create create the shared memory instance if it true,
    ///            open the existing if it is false
    static upg_shm *factory(bool create);

    /// \brief destroy the upgrade shm including the backend
    static void destroy(upg_shm *);

    /// \brief returns the shared memory manager instance
    shmmgr *shm_mgr(void) { return shm_mmgr_; }

    void set_api_upg_ctx(upg_ctxt *ctxt) { api_upg_ctx_ = ctxt; }
    void set_nicmgr_upg_ctx(upg_ctxt *ctxt) { nicmgr_upg_ctx_ = ctxt; }
    upg_ctxt *api_upg_ctx(void) { return api_upg_ctx_; }
    upg_ctxt *nicmgr_upg_ctx(void) { return nicmgr_upg_ctx_; }

private:
    shmmgr              *shm_mmgr_;       ///< shared memory manager
    upg_ctxt            *api_upg_ctx_;    ///< api upgrade obj context
    upg_ctxt            *nicmgr_upg_ctx_; ///< nicmgr upgrade obj context

private:
    sdk_ret_t init_(bool create);
};

}    // namespace api
#endif    // __API_UPGRADE_SHM_HPP__
