//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __EP_VMOTION_HPP__
#define __EP_VMOTION_HPP__

#include "gen/proto/vmotion.pb.h"
#include "nic/include/base.hpp"
#include "nic/hal/vmotion/vmotion.hpp"
#include "nic/sdk/include/sdk/eth.hpp"

namespace hal {

using vmotion::VmotionMessage;

typedef struct ep_vmotion_client_ctxt_s {
    hal_handle_t ep_handle;
    string vmotion_serv_ip;
    uint32_t vmotion_serv_port;
} ep_vmotion_client_ctxt_t;

typedef struct ep_vmotion_server_ctxt_s {
    mac_addr_t mac;

    // dervived info
    hal_handle_t ep_handle;
} ep_vmotion_server_ctxt_t;

hal_ret_t ep_vmotion_client(vmotion_client_t *vm_client);
hal_ret_t ep_vmotion_server(int server_sd);
hal_ret_t ep_vmotion_get_server_info(void *ctxt, string *serv_ip, 
                                     uint32_t *serv_port);
bool ep_vmotion_is_platform_type_sim(void);
hal_ret_t vmotion_send_msg(VmotionMessage& msg, int sd);
hal_ret_t vmotion_recv_msg(VmotionMessage& msg, int sd);
void ep_vmotion_client_msg(struct ev_loop *loop, struct ev_io *watcher,
                           int revents);


} // namespace hal

#endif    // __EP_VMOTION_HPP__

