// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
///  Implement pds upgrade
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/upgrade/core/fsm.hpp"

namespace upg {

/// \brief        Exported API to register application function callback
/// \param[in]    cb_type       Enum to differentiate pre or post callback
/// \param[in]    cb_stage      Enum to identify stage
/// \param[in]    callback      callback function
/// \return       status to callback registration

// TODO:params ??
sdk_ret_t
pds_register_callback (stage_callback_t cb_type,
                                        stage_id_t cb_stage,
                                        void *(*callback)(void*),
                                        void *arg) {
    return register_callback(cb_type, cb_stage, callback, arg);
}

/// \brief          Exported API for initiating pds upgrade
/// \param[in]      void
/// \return         pds upgrade status

// TODO:params ??
sdk_ret_t
pds_do_upgrade (void) {
    return do_upgrade();
}
}
