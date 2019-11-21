//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/vmotion/vmotion.hpp"
#include "nic/hal/vmotion/vmotion_dst_host.hpp"
#include "nic/hal/vmotion/vmotion_msg.hpp"
#include "gen/proto/vmotion.pb.h"

namespace hal {

using namespace vmotion_msg;

vmotion_dst_host_fsm_def *
vmotion_dst_host_fsm_def::factory(void)
{
    auto mem = HAL_CALLOC(HAL_MEM_ALLOC_VMOTION, sizeof(vmotion_dst_host_fsm_def));
    if (!mem) {
        HAL_TRACE_ERR("OOM failed to memalloc for vmotion dst_host_fsm");
        return NULL;
    }
    vmotion_dst_host_fsm_def *fsm = new (mem) vmotion_dst_host_fsm_def();

    return fsm;
}

vmotion_dst_host_fsm_def::vmotion_dst_host_fsm_def(void)
{
#define SM_FUNC(__func) SM_BIND_NON_STATIC(vmotion_dst_host_fsm_def, __func)
#define SM_FUNC_ARG_1(__func) SM_BIND_NON_STATIC_ARGS_1(vmotion_dst_host_fsm_def, __func)
    FSM_SM_BEGIN((sm_def))
        FSM_STATE_BEGIN(STATE_DST_HOST_INIT, 0, NULL, NULL)
            FSM_TRANSITION(EVT_START_SYNC, SM_FUNC(process_start_sync), STATE_DST_HOST_SYNC_REQ)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_DST_HOST_SYNC_REQ, 0, NULL, NULL)
            FSM_TRANSITION(EVT_SYNC, SM_FUNC(process_sync), STATE_DST_HOST_SYNCING)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_DST_HOST_SYNCING, 0, NULL, NULL)
            FSM_TRANSITION(EVT_SYNC_END, SM_FUNC(process_sync_end), STATE_DST_HOST_SYNCED)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_DST_HOST_SYNCED, 0, NULL, NULL)
            FSM_TRANSITION(EVT_TERM_SYNC, SM_FUNC(process_term_sync), STATE_DST_HOST_TERM_SYNCING)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_DST_HOST_TERM_SYNCING, 0, NULL, NULL)
            FSM_TRANSITION(EVT_TERM_SYNC_END, SM_FUNC(process_term_sync_end), STATE_DST_HOST_TERM_SYNCED)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_DST_HOST_TERM_SYNCED, 0, NULL, NULL)
            FSM_TRANSITION(EVT_DST_EP_MOVED, SM_FUNC(process_ep_moved), STATE_DST_HOST_END)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_DST_HOST_END, 0, SM_FUNC_ARG_1(state_dst_host_end), NULL)
        FSM_STATE_END
    FSM_SM_END

    set_state_machine(sm_def);
}

bool
vmotion_dst_host_fsm_def::process_start_sync(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep      *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    VmotionMessage   msg_rsp;
    //VmotionInitiate  init_msg;

    HAL_TRACE_INFO("Dest Host End EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));

    msg_rsp.set_type(VMOTION_MSG_TYPE_INIT);
#if 0 // TEMP
    init_msg.set_mac_address(MAC_TO_UINT64(vmn_ep->get_ep()->l2_key.mac_addr));
    msg_rsp.set_allocated_init(&init_msg);
#endif

    if (vmotion_send_msg(msg_rsp, vmn_ep->get_socket_fd()) != HAL_RET_OK) {
        HAL_TRACE_ERR("vmotion: unable to send sync req message");
        return false;
    }

    // Add EP Quiesce NACL entry
    ep_quiesce(vmn_ep->get_ep(), TRUE);

    return true;
}

bool
vmotion_dst_host_fsm_def::process_sync(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);

    HAL_TRACE_INFO("Dest Host Sync EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));

    // TODO
    return true;
}

bool
vmotion_dst_host_fsm_def::process_sync_end(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    VmotionMessage  msg_rsp;

    HAL_TRACE_INFO("Dest Host Sync End. EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));

    msg_rsp.set_type(VMOTION_MSG_TYPE_TERM_SYNC_REQ);

    if (vmotion_send_msg(msg_rsp, vmn_ep->get_socket_fd()) != HAL_RET_OK) {
        HAL_TRACE_ERR("vmotion: unable to send sync req message");
        return false;
    }

    return true;
}

bool
vmotion_dst_host_fsm_def::process_term_sync(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);

    HAL_TRACE_INFO("Dest Host Term Sync EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));

    // TODO
    return true;
}

bool
vmotion_dst_host_fsm_def::process_term_sync_end(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    VmotionMessage  msg_rsp;

    HAL_TRACE_INFO("Dest Host Term Sync End EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));

    msg_rsp.set_type(VMOTION_MSG_TYPE_TERM_SYNC_ACK);

    if (vmotion_send_msg(msg_rsp, vmn_ep->get_socket_fd()) != HAL_RET_OK) {
        HAL_TRACE_ERR("vmotion: unable to send sync req message");
        return false;
    }
    return true;
}

bool
vmotion_dst_host_fsm_def::process_ep_moved(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    VmotionMessage  msg_rsp;

    HAL_TRACE_INFO("Dest Host EP Moved EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));

    // Remove EP Quiesce NACL entry
    ep_quiesce(vmn_ep->get_ep(), FALSE);

    msg_rsp.set_type(VMOTION_MSG_TYPE_EP_MOVED_ACK);

    if (vmotion_send_msg(msg_rsp, vmn_ep->get_socket_fd()) != HAL_RET_OK) {
        HAL_TRACE_ERR("vmotion: unable to send ep moved ack message");
        return false;
    }
    return true;
}

void
vmotion_dst_host_fsm_def::state_dst_host_end(fsm_state_ctx ctx)
{
    vmotion_ep *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);

    HAL_TRACE_INFO("Dest Host end EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));

    // Stop the watcher
    vmn_ep->get_event_thread()->stop();
}

static void
dst_host_thread_rcv_sock_msg(sdk::event_thread::io_t *io, int sock_fd, int events)
{
    vmotion_ep        *vmn_ep = (vmotion_ep *)io->ctx;
    VmotionMessage     msg;
    uint32_t           event;

    vmotion_recv_msg(msg, sock_fd);

    switch(msg.type()) {
    case VMOTION_MSG_TYPE_SYNC:
        event = EVT_SYNC;
        break;
    case VMOTION_MSG_TYPE_SYNC_END:
        event = EVT_SYNC_END;
        break;
    case VMOTION_MSG_TYPE_TERM_SYNC:
        event = EVT_TERM_SYNC;
        break;
    case VMOTION_MSG_TYPE_TERM_SYNC_END:
        event = EVT_TERM_SYNC_END;
        break;
    case VMOTION_MSG_TYPE_EP_MOVED:
        event = EVT_DST_EP_MOVED;
        break;
    default:
        HAL_TRACE_ERR("unexpected msg type: {}", VmotionMessageType_Name(msg.type()));
        return;
    }
    vmn_ep->process_event(event, NULL);
}

static void
dst_host_thread_init (void *ctx)
{
    vmotion_ep     *vmn_ep = (vmotion_ep *)ctx;

    vmn_ep->dst_host_init();
}

static void
dst_host_thread_exit (void *ctx)
{
    vmotion_ep     *vmn_ep = (vmotion_ep *)ctx;

    vmn_ep->dst_host_exit();
}

static void
dst_host_thread_rcv_event (void *message, void *ctx)
{
}

hal_ret_t
vmotion_ep::dst_host_init()
{
    struct sockaddr_in  addr;

    // Create client socket
    if ((sock_fd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        HAL_TRACE_ERR("socket creation failed in destination host");
        return HAL_RET_ERR;
    }

    bzero(&addr, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(50055); // TEMP
    //inet_pton(AF_INET, ep->get_old_host_ip().c_str(), &addr.sin_addr.s_addr);
    addr.sin_addr.s_addr =  htonl(INADDR_ANY);  // TODO

    HAL_TRACE_INFO("connecting to old host. IP:{} Addr:{}",
                   old_host_ip_.c_str(), addr.sin_addr.s_addr);

    // Connect to server socket
    if (connect(sock_fd_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        HAL_TRACE_ERR("vmotion: connection to old host failed. IP:{} Addr:{} Error:{}",
                      old_host_ip_.c_str(), addr.sin_addr.s_addr, strerror(errno));
        return HAL_RET_ERR;
    }

    process_event(EVT_START_SYNC, NULL);

    evt_io_.ctx = this;

    sdk::event_thread::io_init(&evt_io_, dst_host_thread_rcv_sock_msg, sock_fd_, EVENT_READ);
    sdk::event_thread::io_start(&evt_io_);

    return HAL_RET_OK;
}

hal_ret_t
vmotion_ep::dst_host_exit(void)
{
    HAL_TRACE_DEBUG("Destination host thread exit");

    close(sock_fd_);

    vmotion_ptr_->release_thread_id(thread_id_);

    return HAL_RET_OK;
}

hal_ret_t
vmotion_ep::spawn_dst_host_thread(void)
{
    char      thread_name[SDK_MAX_THREAD_NAME_LEN];
    hal_ret_t ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("spawning destination host thread");

    ret = get_vmotion()->alloc_thread_id(&thread_id_);
    if (ret != HAL_RET_OK) {
        return ret;
    }
    snprintf(thread_name, sizeof(thread_name), "vmn-dst-%d", thread_id_);

    evt_thread_ =
        sdk::event_thread::event_thread::factory(thread_name, thread_id_,
                                                 sdk::lib::THREAD_ROLE_CONTROL,
                                                 0x0,    // use all control cores
                                                 dst_host_thread_init,  // Thread Init Fn
                                                 dst_host_thread_exit,  // Thread Exit Fn
                                                 dst_host_thread_rcv_event,  // Thread Event CB
                                                 sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
                                                 sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
                                                 NULL);
    if (!evt_thread_) {
        HAL_TRACE_ERR("vmotion dst host thread create failure");
        return HAL_RET_ERR;
    }

    evt_thread_->start(this);

    return ret;
}

} // namespace hal
