//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "nic/hal/plugins/cfg/nw/ep_vmotion_server.hpp"

namespace hal {

bool
ep_vmotion_valid_msg(VmotionMessage& msg, VmotionMessage::MsgCase msg_case) 
{
    if (msg.msg_case() == msg_case) {
        return true;
    }
    return false;
}


}
