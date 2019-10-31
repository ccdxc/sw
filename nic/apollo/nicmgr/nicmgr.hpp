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

private:
    /// \brief    constructor
    nicmgrapi() {}

    /// \brief    destructor
    ~nicmgrapi() {}

    /// \brief    port handler callback
    /// \param[in]    event pointer to the event
    /// \param[in]    event_len length of the event data
    /// \param[in]    ctxt callback context
    static void port_event_handler_(void *event, size_t event_len, void *ctxt);

    /// \brief    transceiver event handler
    /// \param[in]    event pointer to the event
    /// \param[in]    event_len length of the event data
    /// \param[in]    ctxt callback context
    static void xcvr_event_handler_(void *data, size_t data_len, void *ctxt);
};

}    // namespace nicmgr

/// \@}

#endif    // __NICMGR_HPP__
