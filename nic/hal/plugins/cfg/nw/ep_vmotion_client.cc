//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#include "nic/hal/plugins/cfg/nw/ep_vmotion_client.hpp"
#include "nic/hal/plugins/cfg/nw/endpoint.hpp"
#include "gen/proto/vmotion.pb.h"
#include "ev.h"

namespace hal {

using namespace vmotion;

hal_ret_t 
ep_vmotion_client_start (vmotion_client_t *vm_client)
{
    hal_ret_t       ret = HAL_RET_OK;
    struct ev_loop  *loop = ev_loop_new(EVBACKEND_POLL | EVBACKEND_SELECT | 
                                        EVFLAG_NOENV);
    struct ev_io    w_read;

    HAL_TRACE_DEBUG("starting vmotion for client");

    ret = ep_vmotion_client_initiate_to_server(vm_client);

    ret = ep_vmotion_client_process_init(vm_client);

    // push the clean up function

    // start event loop to listen to server responses
    ev_io_init(&w_read, ep_vmotion_client_msg, vm_client->client_sd, EV_READ);
    ev_io_start(loop, &w_read);

    ev_run(loop, 0);

    return ret;
} 

void
ep_vmotion_client_msg (struct ev_loop *loop, struct ev_io *watcher,
                       int revents)
{
    hal_ret_t           ret = HAL_RET_OK;
    int                 client_sd = watcher->fd;
    VmotionMessage      msg;
    thread              *curr_thread = sdk::lib::thread::current_thread();
    vmotion_client_t    *vm_client = (vmotion_client_t *)curr_thread->data();

    vmotion_recv_msg(msg, client_sd);
    HAL_TRACE_DEBUG("client recvd msg: {}", 
                    VmotionMessageType_Name(msg.type()));
    switch(msg.type()) {
        case VMOTION_MSG_TYPE_INITIATE_DONE:
            ep_vmotion_client_initiate_done(msg, vm_client);
            break;
        case VMOTION_MSG_TYPE_STATE_SYNC:
            ep_vmotion_client_state_sync(msg, vm_client);
            break;
        case VMOTION_MSG_TYPE_STATE_SYNC_END:
            ep_vmotion_client_state_sync_end(msg, vm_client);
            break;
        case VMOTION_MSG_TYPE_END_ACK:
            ep_vmotion_client_end_ack(msg, vm_client);
            break;
        default:
            HAL_TRACE_ERR("unexpected msg type: {}", 
                          VmotionMessageType_Name(msg.type()));
            // ep_vmotion_client_terminate();
    }

    return;
}

hal_ret_t
ep_vmotion_client_initiate_to_server (vmotion_client_t *vm_client)
{
    hal_ret_t                   ret = HAL_RET_OK;
    VmotionMessage              msg;
    VmotionInitiate             *init_msg;
    ep_vmotion_client_ctxt_t    *ctxt = NULL;
    ep_t                        *ep = NULL;

    ctxt = (ep_vmotion_client_ctxt_t *)vm_client->ctxt; 
    ep = find_ep_by_handle(ctxt->ep_handle);
    // commenting for testing
#if 0
    if (!ep) {
        HAL_TRACE_ERR("no ep with handle: {}", ctxt->ep_handle);
        return HAL_RET_ERR;
    }
#endif


    msg.set_type(VMOTION_MSG_TYPE_INITIATE);
    init_msg = msg.mutable_init();
    init_msg->set_mac_address(ep ? MAC_TO_UINT64(ep->l2_key.mac_addr) : 0);
    ret = vmotion_send_msg(msg, vm_client->client_sd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to send initiate msg to server");
        return ret;
    }
}

hal_ret_t
ep_vmotion_client_process_init(vmotion_client_t *vm_client)
{
    hal_ret_t ret = HAL_RET_OK;

    // TODO: Change flows with SEP == EP, to direction enic. 
    HAL_TRACE_DEBUG("Change flows to enic direction");

    return ret;
}

hal_ret_t
ep_vmotion_client_initiate_done (VmotionMessage &msg, 
                                 vmotion_client_t *vm_client)
{
    hal_ret_t       ret = HAL_RET_OK;
    VmotionMessage  msg_rsp;


    // send state sync start
    msg_rsp.set_type(VMOTION_MSG_TYPE_STATE_SYNC_START);
    ret = vmotion_send_msg(msg_rsp, vm_client->client_sd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to send sync start msg to server");
        goto end;
    }

end:
    return ret;
}

hal_ret_t
ep_vmotion_client_state_sync (VmotionMessage &msg,
                              vmotion_client_t *vm_client)
{
    hal_ret_t       ret = HAL_RET_OK;

    // TODO: create sessions for the synced state

    return ret;
}

hal_ret_t
ep_vmotion_client_state_sync_end (VmotionMessage &msg,
                                 vmotion_client_t *vm_client)
{
    hal_ret_t       ret = HAL_RET_OK;
    VmotionMessage  msg_rsp;

    // TODO: Not sure what we can do here.


    // send state sync start
    msg_rsp.set_type(VMOTION_MSG_TYPE_END);
    ret = vmotion_send_msg(msg_rsp, vm_client->client_sd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to send vmotion end msg to server");
        goto end;
    }

end:
    return ret;
}

hal_ret_t
ep_vmotion_client_end_ack (VmotionMessage &msg,
                           vmotion_client_t *vm_client)
{
    hal_ret_t       ret = HAL_RET_OK;

    // TODO: 

    return ret;
}

}
