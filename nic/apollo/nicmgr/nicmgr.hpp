//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// nicmgr external header file
///
//----------------------------------------------------------------------------

#ifndef __NICMGR_HPP__
#define __NICMGR_HPP__

#include "nic/sdk/platform/evutils/include/evutils.h"

/// \defgroup PDS_NICMGR
/// @{

class DeviceManager;

namespace nicmgr {

// TODO: why are we using class here ?
class nicmgrapi {
public:
    /// \brief    factory method to nicmgrapi instance
    /// \return    new instance of nicmgrapi or NULL, in case of error
    static nicmgrapi *factory(void);

    /// \brief    destroy nicmgrapi instance
    /// \param[in]    nicmgr nicmgrapi instance
    static void destroy(nicmgrapi *nicmgr);

    /// \brief    nicmgr thread's entry point
    /// \param[in]    ctxt thread start callback context
    static void nicmgr_thread_init(void *ctxt);

    /// \brief    nicmgr thread's cleanup point
    /// \param[in]    ctxt object to be destroyed
    static void nicmgr_thread_exit(void *ctxt=NULL);

    /// \brief    event callback
    /// \param[in]    msg  event message pointer
    /// \param[in]    ctxt callback context
    static void nicmgr_event_handler(void *msg, void *ctxt);

    /// \brief    DeviceManager interface
    static DeviceManager *devmgr_if(void);

private:
    /// \brief    constructor
    nicmgrapi() {}

    /// \brief    destructor
    ~nicmgrapi() {}

    /// \brief    HAL UP event handler
    /// \param[in]    msg the message of the event
    /// \param[in]    ctxt callback context
    static void hal_up_event_handler_(sdk::ipc::ipc_msg_ptr msg,
                                      const void *ctxt);

    /// \brief    port handler callback
    /// \param[in]    msg the message of the event
    /// \param[in]    ctxt callback context
    static void port_event_handler_(sdk::ipc::ipc_msg_ptr msg,
                                    const void *ctxt);

    /// \brief    transceiver event handler
    /// \param[in]    msg the message of the event
    /// \param[in]    ctxt callback context
    static void xcvr_event_handler_(sdk::ipc::ipc_msg_ptr msg,
                                    const void *ctxt);
};

}    // namespace nicmgr

/// \@}

#endif    // __NICMGR_HPP__
