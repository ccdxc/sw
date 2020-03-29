//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines event handlers during process upgrade
///
//----------------------------------------------------------------------------

#ifndef __API_UPGRADE_EV_HPP__
#define __API_UPGRADE_EV_HPP__


namespace api {
typedef uint32_t upg_thread_id_t;

}   // namespace api

#include "nic/apollo/upgrade/include/ev.hpp"
#include "nic/apollo/api/internal/upgrade_ev_graceful.hpp"
#include "nic/apollo/api/internal/upgrade_ev_hitless.hpp"

#endif   // __API_UPGRADE_EV_HPP__
