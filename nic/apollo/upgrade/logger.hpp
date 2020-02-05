//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __UPGRADE_LOGGER_HPP__
#define __UPGRADE_LOGGER_HPP__

#include <sys/stat.h>
#include <nic/sdk/include/sdk/base.hpp>
#include "nic/sdk/lib/operd/logger.hpp"

namespace upg {

// operd logging
extern sdk::operd::logger_ptr g_upg_log;
#define UPG_LOG_NAME "upgradelog"

}    // namespace upg

using upg::g_upg_log;

#endif     // __UPGRADE_LOGGER_HPP__
