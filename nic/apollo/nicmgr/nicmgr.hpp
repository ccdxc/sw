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

/// \defgroup PDS_NICMGR
/// @{

namespace nicmgr {

/// starting point for the periodic thread loop
void *nicmgr_thread_start(void *ctxt);

}    // namespace nicmgr

/// \@}

#endif    // __NICMGR_HPP__
