//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __DESCRIPTOR_AOL_HPP__
#define __DESCRIPTOR_AOL_HPP__


#include "nic/include/base.h"
#include "nic/include/pd.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/gen/proto/hal/descriptor_aol.pb.h"

using descraol::DescrAolSpec;
using descraol::DescrAolRequest;
using descraol::DescrAolRequestMsg;
using descraol::DescrAolResponseMsg;

namespace hal {
hal_ret_t descraol_get(descraol::DescrAolRequest& request,
                       descraol::DescrAolResponseMsg *response);
}
#endif /* __DESCRIPTOR_AOL_HPP__ */
