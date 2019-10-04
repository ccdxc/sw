//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "nic/hal/plugins/cfg/nw/ep_vmotion_server.hpp"
#include "nic/hal/plugins/cfg/nw/ep_vmotion_utils.hpp"
#include "nic/hal/plugins/cfg/nw/endpoint.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "gen/proto/vmotion.pb.h"

namespace hal {

using namespace vmotion;

hal_ret_t
ep_vmotion_server_msg (void)
{
    hal_ret_t               ret = HAL_RET_OK;
    thread                  *curr_thread = sdk::lib::thread::current_thread();
    vmotion_server_slave_t  *vm_server = (vmotion_server_slave_t *)curr_thread->data();
    VmotionMessage          msg;

    vmotion_recv_msg(msg, vm_server->slave_sd);
    HAL_TRACE_DEBUG("server slave recvd msg: {}",
                    VmotionMessageType_Name(msg.type()));
    switch(msg.type()) {
        case VMOTION_MSG_TYPE_INITIATE:
            ep_vmotion_server_initiate(msg, vm_server);
            break;
        case VMOTION_MSG_TYPE_STATE_SYNC_START:
            ep_vmotion_server_state_sync(msg, vm_server);
            break;
        case VMOTION_MSG_TYPE_END:
            ep_vmotion_server_end(msg, vm_server);
            break;
        default:
            HAL_TRACE_ERR("unexpected msg type: {}",
                          VmotionMessageType_Name(msg.type()));
            // ep_vmotion_client_terminate();
    }

    return ret;
}

hal_ret_t
ep_vmotion_server_initiate (VmotionMessage& msg, 
                            vmotion_server_slave_t *vm_server)
{
    hal_ret_t                   ret = HAL_RET_OK;
    ep_vmotion_server_ctxt_t    *vm_server_ctxt = NULL;
    VmotionInitiate             init_msg;
    VmotionMessage              msg_rsp;
    // ep_t                        *ep = NULL;

    vm_server_ctxt = (ep_vmotion_server_ctxt_t *)
        HAL_CALLOC(HAL_MEM_ALLOC_VMOTION_SERVER_SLAVE_CTXT, 
                   sizeof(ep_vmotion_server_ctxt_t));
    vm_server->ctxt = vm_server_ctxt;

   if (!ep_vmotion_valid_msg(msg, VmotionMessage::kInit)) {
        HAL_TRACE_ERR("unexpected msg: {}", msg.msg_case());
        ret = HAL_RET_ERR;
        goto end;
   }

    // get ep handle from mac
    init_msg = msg.init();

    MAC_UINT64_TO_ADDR(vm_server_ctxt->mac, init_msg.mac_address());
    HAL_TRACE_DEBUG("vmotion init for mac: {}", macaddr2str(vm_server_ctxt->mac));

    // ep = find_ep_by_mac(vm_server_ctxt->mac);

    // TODO: Change flows with SEP as EP. The direction for them would be ENIC.
    //       change them to Uplink

    // send INIT done response to client
    msg_rsp.set_type(VMOTION_MSG_TYPE_INITIATE_DONE);
    ret = vmotion_send_msg(msg_rsp, vm_server->slave_sd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to send initiate done to client");
        goto end;
    }

end:
    return ret;
}

hal_ret_t
ep_vmotion_server_state_sync (VmotionMessage& msg, 
                              vmotion_server_slave_t *vm_server)
{
    hal_ret_t       ret = HAL_RET_OK;
    VmotionMessage  msg_rsp;

    // TODO: start syncing state

    // send sync state done
    msg_rsp.set_type(VMOTION_MSG_TYPE_STATE_SYNC_END);
    ret = vmotion_send_msg(msg_rsp, vm_server->slave_sd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to send initiate done to client");
        goto end;
    }

end:
    return ret;
}

hal_ret_t
ep_vmotion_server_end (VmotionMessage& msg, 
                       vmotion_server_slave_t *vm_server)
{
    hal_ret_t                   ret = HAL_RET_OK;
    VmotionMessage              msg_rsp;

    // TODO: start syncing state

    // send sync state done
    msg_rsp.set_type(VMOTION_MSG_TYPE_END_ACK);
    ret = vmotion_send_msg(msg_rsp, vm_server->slave_sd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to send initiate done to client");
        goto end;
    }

end:
    return ret;
}


} // namespace hal
