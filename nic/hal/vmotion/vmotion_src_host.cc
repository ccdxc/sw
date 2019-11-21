//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/vmotion/vmotion.hpp"
#include "nic/include/hal_mem.hpp"
#include "nic/hal/vmotion/vmotion_src_host.hpp"
#include "nic/hal/vmotion/vmotion_msg.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "gen/proto/vmotion.pb.h"

namespace hal {

using namespace vmotion_msg;

vmotion_src_host_fsm_def *
vmotion_src_host_fsm_def::factory(void)
{
    auto mem = HAL_CALLOC(HAL_MEM_ALLOC_VMOTION, sizeof(vmotion_src_host_fsm_def));
    if (!mem) {
        HAL_TRACE_ERR("OOM failed to memalloc for vmotion src_host_fsm");
        return NULL;
    }
    vmotion_src_host_fsm_def *fsm = new (mem) vmotion_src_host_fsm_def();

    return fsm;
}

vmotion_src_host_fsm_def::vmotion_src_host_fsm_def(void)
{
#define SM_FUNC(__func)       SM_BIND_NON_STATIC(vmotion_src_host_fsm_def, __func)
#define SM_FUNC_ARG_1(__func) SM_BIND_NON_STATIC_ARGS_1(vmotion_src_host_fsm_def, __func)
    FSM_SM_BEGIN((sm_def))
        FSM_STATE_BEGIN(STATE_SRC_HOST_INIT, 0, NULL, NULL)
            FSM_TRANSITION(EVT_SYNC_BEGIN, SM_FUNC(proc_sync_begin), STATE_SRC_HOST_SYNC)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_SRC_HOST_SYNC, 0, NULL, NULL)
            FSM_TRANSITION(EVT_SYNC_DONE, SM_FUNC(proc_evt_sync_done), STATE_SRC_HOST_SYNCED)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_SRC_HOST_SYNCED, 0, NULL, NULL)
            FSM_TRANSITION(EVT_TERM_SYNC_REQ, SM_FUNC(proc_term_sync_req), STATE_SRC_HOST_TERM_SYNC)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_SRC_HOST_TERM_SYNC, 0, NULL, NULL)
            FSM_TRANSITION(EVT_TERM_SYNC_DONE, SM_FUNC(proc_term_sync_done), STATE_SRC_HOST_TERM_SYNCED)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_SRC_HOST_TERM_SYNCED, 0, NULL, NULL)
            FSM_TRANSITION(EVT_TERM_SYNCED_ACK, SM_FUNC(proc_term_synced_ack), STATE_SRC_HOST_EP_MOVED)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_SRC_HOST_EP_MOVED, 0, NULL, NULL)
            FSM_TRANSITION(EVT_SRC_EP_MOVED_ACK, SM_FUNC(proc_ep_moved_ack), STATE_SRC_HOST_END)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_SRC_HOST_END, 0, SM_FUNC_ARG_1(state_src_host_end_entry), NULL)
        FSM_STATE_END
    FSM_SM_END

    set_state_machine(sm_def);
}

bool
vmotion_src_host_fsm_def::proc_sync_begin(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep     *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    VmotionMessage  msg_rsp;

    HAL_TRACE_INFO("Sync Begin EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));
 
    msg_rsp.set_type(VMOTION_MSG_TYPE_SYNC);

    if (vmotion_send_msg(msg_rsp, vmn_ep->get_socket_fd()) != HAL_RET_OK) {
        HAL_TRACE_ERR("vmotion unable to send sync message");
        return false;
    }

    vmn_ep->throw_event(EVT_SYNC_DONE, NULL);

    return true;
}

bool
vmotion_src_host_fsm_def::proc_evt_sync_done(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    VmotionMessage  msg_rsp;

    HAL_TRACE_INFO("Sync Done EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));

    msg_rsp.set_type(VMOTION_MSG_TYPE_SYNC_END);

    if (vmotion_send_msg(msg_rsp, vmn_ep->get_socket_fd()) != HAL_RET_OK) {
        HAL_TRACE_ERR("vmotion unable to send sync end message");
        return false;
    }

    return true;
}

bool
vmotion_src_host_fsm_def::proc_term_sync_req(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    VmotionMessage  msg_rsp;

    HAL_TRACE_INFO("Term Sync Req EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));

    // Add EP Quiesce NACL entry
    ep_quiesce(vmn_ep->get_ep(), TRUE);

    msg_rsp.set_type(VMOTION_MSG_TYPE_TERM_SYNC);

    if (vmotion_send_msg(msg_rsp, vmn_ep->get_socket_fd()) != HAL_RET_OK) {
        HAL_TRACE_ERR("vmotion unable to send term sync message");
        return false;
    }

    vmn_ep->throw_event(EVT_TERM_SYNC_DONE, NULL);

    return true;
}

bool
vmotion_src_host_fsm_def::proc_term_sync_done(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    VmotionMessage  msg_rsp;

    HAL_TRACE_INFO("Term Sync Done EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));

    msg_rsp.set_type(VMOTION_MSG_TYPE_TERM_SYNC_END);

    if (vmotion_send_msg(msg_rsp, vmn_ep->get_socket_fd()) != HAL_RET_OK) {
        HAL_TRACE_ERR("vmotion unable to send term sync message");
        return false;
    }
    return true;
}

bool
vmotion_src_host_fsm_def::proc_term_synced_ack(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    VmotionMessage  msg_rsp;

    HAL_TRACE_INFO("Term Sync Ack EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));

    msg_rsp.set_type(VMOTION_MSG_TYPE_EP_MOVED);

    if (vmotion_send_msg(msg_rsp, vmn_ep->get_socket_fd()) != HAL_RET_OK) {
        HAL_TRACE_ERR("vmotion unable to send term sync end message");
        return false;
    }

    return true;
}

bool
vmotion_src_host_fsm_def::proc_ep_moved_ack(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);

    HAL_TRACE_INFO("Ep Moved Ack EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));

    // Stop the watcher
    vmn_ep->get_event_thread()->stop();
    return true;
}

void
vmotion_src_host_fsm_def::state_src_host_end_entry(fsm_state_ctx ctx)
{
    vmotion_ep *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);

    HAL_TRACE_INFO("Source Host End EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));

    // Remove EP Quiesce NACL entry
    ep_quiesce(vmn_ep->get_ep(), FALSE);

    // Stop the watcher
    vmn_ep->get_event_thread()->stop();
}

vmotion_ep *
src_host_proc_init_msg(vmotion *vmn, VmotionMessage msg, vmotion_thread_ctx_t *thread_ctx)
{
    VmotionInitiate    init_msg = msg.init();
#if 1 // TEMP
    mac_addr_t mac = {0x0, 0x0, 0x0, 0x0, 0xAB, 0xCD};
#else
    mac_addr_t         mac;
    MAC_UINT64_TO_ADDR(mac, init_msg.mac_address());
#endif
    auto ep = find_ep_by_mac(mac);
    if (!ep) {
        HAL_TRACE_ERR("EP Not exists in Init Msg. MAC: {}", init_msg.mac_address());
        return NULL;
    }

    auto vmn_ep = vmn->get_vmotion_ep(ep);
    if (vmn_ep) {
        HAL_TRACE_ERR("unexpected. EP Already exists MAC: {}", init_msg.mac_address());
        return vmn_ep;
    }

    vmn_ep = vmn->create_vmotion_ep(ep, VMOTION_TYPE_MIGRATE_OUT);
    if (!vmn_ep) {
        HAL_TRACE_ERR("EP Creation failed in Source Host. MAC: {}", init_msg.mac_address());
        return NULL;
    }

    HAL_TRACE_INFO("Source Host Init EP: {} FD: {}", macaddr2str(ep->l2_key.mac_addr),
                   thread_ctx->fd);

    vmn_ep->set_socket_fd(thread_ctx->fd);
    vmn_ep->set_event_thread(thread_ctx->th);

    vmn_ep->process_event(EVT_SYNC_BEGIN, NULL);
    return vmn_ep;
}

static void
src_host_thread_rcv_sock_msg (sdk::event_thread::io_t *io, int sock_fd, int events)
{
    vmotion_thread_ctx_t *thread_ctx = (vmotion_thread_ctx_t *)io->ctx;
    vmotion_ep           *vmn_ep = thread_ctx->vmn_ep;
    vmotion_ep           *new_ep = NULL;
    VmotionMessage        msg;

    vmotion_recv_msg(msg, thread_ctx->fd);

    HAL_TRACE_DEBUG("source host thread recvd sock msg: {}", VmotionMessageType_Name(msg.type()));

    switch(msg.type()) {
    case VMOTION_MSG_TYPE_INIT:
        if (vmn_ep) {
            HAL_TRACE_ERR("unexpected rcv msg. EP Already exists MAC");
            return;
        }
        new_ep = src_host_proc_init_msg(g_hal_state->get_vmotion(), msg, thread_ctx);

        if (!new_ep)
            return;
        thread_ctx->vmn_ep = new_ep;
        break;

    case VMOTION_MSG_TYPE_TERM_SYNC_REQ:
        if (!vmn_ep) {
            HAL_TRACE_ERR("unexpected msg type: {} EP Not created.",
                    VmotionMessageType_Name(msg.type()));
            return;
        }
        vmn_ep->process_event(EVT_TERM_SYNC_REQ, NULL);
        break;
    case VMOTION_MSG_TYPE_TERM_SYNC_ACK:
        if (!vmn_ep) {
            HAL_TRACE_ERR("unexpected msg type: {} EP Not created.",
                    VmotionMessageType_Name(msg.type()));
            return;
        }
        vmn_ep->process_event(EVT_TERM_SYNCED_ACK, NULL);
        break;
    case VMOTION_MSG_TYPE_EP_MOVED_ACK:
        if (!vmn_ep) {
            HAL_TRACE_ERR("unexpected msg type: {} EP Not created.",
                    VmotionMessageType_Name(msg.type()));
            return;
        }
        vmn_ep->process_event(EVT_SRC_EP_MOVED_ACK, NULL);
        break;
    default:
        HAL_TRACE_ERR("unexpected msg type: {}", VmotionMessageType_Name(msg.type()));
    }
}

static void
src_host_thread_init (void *ctxt)
{
    vmotion_thread_ctx_t *thread_ctx = (vmotion_thread_ctx_t *)ctxt;

    thread_ctx->io.ctx = ctxt;

    // initialize and start a watcher to accept messages from the remote host
    sdk::event_thread::io_init(&thread_ctx->io, src_host_thread_rcv_sock_msg,
                               thread_ctx->fd, EVENT_READ);
    sdk::event_thread::io_start(&thread_ctx->io);
}

static void
src_host_thread_exit (void *ctxt)
{
    vmotion_thread_ctx_t *thread_ctx = (vmotion_thread_ctx_t *)ctxt;

    HAL_TRACE_DEBUG("Source host thread exit");

    sdk::event_thread::event_thread::destroy(thread_ctx->th);

    g_hal_state->get_vmotion()->release_thread_id(thread_ctx->tid);

    close(thread_ctx->fd);

    hal::delay_delete_to_slab(HAL_SLAB_VMOTION_THREAD_CTX, thread_ctx);
}

static void
src_host_thread_rcv_event (void *message, void *ctx)
{
}

hal_ret_t
vmotion::spawn_src_host_thread(int sock_fd)
{
    hal_ret_t               ret = HAL_RET_OK;
    struct sockaddr_in      host_addr;
    socklen_t               host_len = sizeof(host_addr);
    uint32_t                tid;
    char                    thread_name[SDK_MAX_THREAD_NAME_LEN];

    vmotion_thread_ctx_t *thread_ctx =
        (vmotion_thread_ctx_t *)g_hal_state->vmotion_thread_ctx_slab()->alloc();

    if (!thread_ctx) {
        HAL_TRACE_ERR("OOM Fail allocate memory for vmotion_thread_ctx");
        return HAL_RET_ERR;
    }

    // accept the host connect request
    thread_ctx->fd = accept(sock_fd, (struct sockaddr *)&host_addr, &host_len);
    if (thread_ctx->fd < 0) {
        HAL_TRACE_ERR("accept fail in source host");
        return HAL_RET_ERR;
    }

    ret = alloc_thread_id(&tid);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("vmotion thread id allocation fail in source host");
        return ret;
    }

    HAL_TRACE_INFO("source host thread active. host: {}:{} thread_id: {}",
                   inet_ntoa(host_addr.sin_addr), (int) ntohs(host_addr.sin_port), tid);

    snprintf(thread_name, sizeof(thread_name), "vmn-src-%d", tid);

    thread_ctx->tid = tid;
    thread_ctx->th  =
        sdk::event_thread::event_thread::factory(thread_name, tid, sdk::lib::THREAD_ROLE_CONTROL,
                                                 0x0,    // use all control cores
                                                 src_host_thread_init,  // Thread Init Fn
                                                 src_host_thread_exit,  // Thread Exit Fn
                                                 src_host_thread_rcv_event,  // Thread Event CB
                                                 sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
                                                 sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
                                                 NULL);
    if (!thread_ctx->th) {
        HAL_TRACE_ERR("vmotion src host thread create failure");
        return HAL_RET_ERR;
    }
    // Start the thread
    thread_ctx->th->start(thread_ctx);

    return ret;
}

} // namespace hal
