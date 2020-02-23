// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// common definitions for core upgrade data structure for external uses
///
//----------------------------------------------------------------------------

#ifndef __PDS_UPGMGR_HPP__
#define __PDS_UPGMGR_HPP__

#include "nic/sdk/include/sdk/base.hpp"

namespace upg {
    /// \brief Core upgrade stages
    /// \remark
    /// upgrade orchestration will happen to start from the compatibility_check stage
    /// (STAGE_ID_COMPAT_CHECK) to the exit stage (STAGE_ID_EXIT). Internal stage change
    /// will happen base on predefined logic from a json file.
    typedef enum stage_id_t {
        STAGE_ID_COMPAT_CHECK = 0,
        STAGE_ID_START = 1,
        STAGE_ID_PREPARE = 2,
        STAGE_ID_BACKUP = 3,
        STAGE_ID_SWITCHOVER = 4,
        STAGE_ID_VERIFY = 5,
        STAGE_ID_FINISH = 6,
        STAGE_ID_ABORT = 7,
        STAGE_ID_ROLLBACK = 8,
        STAGE_ID_CRITICAL = 9,
        STAGE_ID_EXIT = 10
    } stage_id_t ;

    /// \brief function call back type
    /// \remark
    /// Type of the registered callback , whether to call the address
    /// before executing the stage or after executing the stage.
    typedef enum stage_callback_t {
        PRE_STAGE  = 0,
        POST_STAGE = 1
    } stage_callback_t;

    /// \brief        Exported API to register application function callback
    /// \param[in]    cb_type       Enum to differentiate pre or post callback
    /// \param[in]    cb_stage      Enum to identify stage
    /// \param[in]    callback      callback function
    /// \return       status to callback registration
    extern sdk_ret_t pds_register_callback( stage_callback_t cb_type,
                                            stage_id_t cb_stage,
                                            void *(*callback)(void*), void* arg)
        __attribute__ ((visibility ("default") ));

    /// \brief          Exported API for initiating pds upgrade
    /// \param[in]      void
    /// \return         pds upgrade status
    extern sdk_ret_t pds_do_switchover(void) __attribute__ ((visibility
                                                          ("default") ));
}
#endif    //__PDS_UPGMGR_HPP___
