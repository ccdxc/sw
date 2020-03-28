// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#ifndef __HAL_QOS_PD_HPP__
#define __HAL_QOS_PD_HPP__

#include "nic/include/base.hpp"
#include "nic/hal/plugins/cfg/aclqos/qos.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"


namespace hal {
namespace pd {

// Using a non-zero value for admic cos so that any uninitialized
// rings do not use this 
#define HAL_QOS_ADMIN_COS                   1

}   // namespace pd
}   // namespace hal

#endif    // __HAL_QOS_PD_HPP__
