//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __EP_VMOTION_SERVER_HPP__
#define __EP_VMOTION_SERVER_HPP__

#include "nic/include/base.hpp"
#include "nic/hal/plugins/cfg/nw/ep_vmotion.hpp"

namespace hal {

hal_ret_t ep_vmotion_server_msg(void);
hal_ret_t ep_vmotion_server_initiate(VmotionMessage& msg,
                                     vmotion_server_slave_t *vm_server);
hal_ret_t ep_vmotion_server_state_sync (VmotionMessage& msg, 
                                        vmotion_server_slave_t *vm_server);
hal_ret_t ep_vmotion_server_end (VmotionMessage& msg, 
                                 vmotion_server_slave_t *vm_server);

} // namespace hal

#endif    // __EP_VMOTION_SERVER_HPP__
