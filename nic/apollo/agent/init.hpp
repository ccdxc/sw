//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __PDS_AGENT_INIT_HPP__
#define __PDS_AGENT_INIT_HPP__

#include <string>
#include "nic/sdk/include/sdk/base.hpp"

namespace core {

sdk_ret_t agent_init(std::string cfg_file, std::string memory_profile,
                     std::string device_profile, std::string pipeline);

}    // namespace core

#endif    // __PDS_AGENT_INIT_HPP__
