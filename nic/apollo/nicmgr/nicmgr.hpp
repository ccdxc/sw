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
    /// \return always NULL
    static void *nicmgr_thread_start(void *ctxt);

private:
    /// \brief    constructor
    nicmgrapi() {}

    /// \brief    destructor
    ~nicmgrapi() {}

    /// \brief    port status handler callback
    /// \param[in]    ctxt callback context
    static void port_status_handler_(void *ctxt);

};

}    // namespace nicmgr

/// \@}

#endif    // __NICMGR_HPP__
