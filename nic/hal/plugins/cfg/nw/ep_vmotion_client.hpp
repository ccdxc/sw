//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __EP_VMOTION_CLIENT_HPP__
#define __EP_VMOTION_CLIENT_HPP__

#include "nic/include/base.hpp"
#include "nic/hal/plugins/cfg/nw/ep_vmotion.hpp"

namespace hal {

hal_ret_t ep_vmotion_client_start(vmotion_client_t *vm_client);
hal_ret_t ep_vmotion_client_process_init(vmotion_client_t *vm_client);
hal_ret_t ep_vmotion_client_initiate_to_server (vmotion_client_t *vm_client);
hal_ret_t ep_vmotion_client_initiate_done (VmotionMessage &msg,
                                           vmotion_client_t *vm_client);
hal_ret_t ep_vmotion_client_state_sync (VmotionMessage &msg,
                                        vmotion_client_t *vm_client);
hal_ret_t ep_vmotion_client_state_sync_end (VmotionMessage &msg,
                                            vmotion_client_t *vm_client);
hal_ret_t ep_vmotion_client_end_ack (VmotionMessage &msg,
                                     vmotion_client_t *vm_client);

} // namespace hal

#endif    // __EP_VMOTION_CLIENT_HPP__
