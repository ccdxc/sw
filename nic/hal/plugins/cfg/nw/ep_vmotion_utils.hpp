//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __EP_VMOTION_UTILS_HPP__
#define __EP_VMOTION_UTILS_HPP__

#include "nic/include/base.hpp"
#include "nic/hal/plugins/cfg/nw/ep_vmotion.hpp"
#include "gen/proto/vmotion.pb.h"

namespace hal {

using namespace vmotion;

bool ep_vmotion_valid_msg(VmotionMessage& msg, VmotionMessage::MsgCase msg_case);

} // namespace hal

#endif    // __EP_VMOTION_UTILS_HPP__
