//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/vmotion/vmotion.hpp"
#include "nic/hal/vmotion/vmotion_dst_host.hpp"
#include "nic/hal/vmotion/vmotion_msg.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "gen/proto/vmotion.pb.h"
#include "nic/fte/utils/packet_utils.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {

using namespace vmotion_msg;

void
dst_host_end (vmotion_ep *vmn_ep)
{
    auto ep = vmn_ep->get_ep();

    HAL_TRACE_INFO("Dest Host end EP: {:p}, Hdl: {}, MigState:{} exited:{}", (void *)ep,
                   vmn_ep->get_ep_handle(), vmn_ep->get_migration_state(),
                   VMOTION_FLAG_IS_THREAD_EXITED(vmn_ep));

    if (VMOTION_FLAG_IS_THREAD_EXITED(vmn_ep)) {
        return;
    }

    if (ep) {
        // In Abort scenario, all these ep cleanup routines will be done in main thread itself
        if ((vmn_ep->get_migration_state() == MigrationState::SUCCESS) ||
            (vmn_ep->get_migration_state() == MigrationState::FAILED) ||
            (vmn_ep->get_migration_state() == MigrationState::TIMEOUT)) {
            // Remove EP Quiesce NACL entry
            if (VMOTION_FLAG_IS_EP_QUIESCE_ADDED(vmn_ep)) {
                ep_quiesce(ep, FALSE);
                VMOTION_FLAG_RESET_EP_QUIESCE_ADDED(vmn_ep);
            }

            if (VMOTION_FLAG_IS_INP_MAC_REMOVED(vmn_ep)) {
                endpoint_migration_inp_mac_vlan_pgm(ep, true);
                VMOTION_FLAG_RESET_INP_MAC_REMOVED(vmn_ep);
            }

            // Loop the sessions, and start aging timer
            endpoint_migration_session_age_reset(ep);
            // Send success/failure notification to Net Agent
            endpoint_migration_status_update(ep, vmn_ep->get_migration_state());
        }
        ep->vmotion_state = vmn_ep->get_migration_state();
    }

    VMOTION_FLAG_SET_THREAD_EXITED(vmn_ep);

    // Stats
    vmn_ep->get_vmotion()->incr_migration_state_stats(vmn_ep->get_migration_state());

    // Stop the watcher
    vmn_ep->get_event_thread()->stop();
}

static void
vmotion_dst_host_timeout_cb (void *timer, uint32_t timer_id, void *ctxt)
{
    vmotion_ep *vmn_ep = (vmotion_ep *) ctxt;
    vmotion_thread_evt_t *evt =
        (vmotion_thread_evt_t *)HAL_CALLOC(HAL_MEM_ALLOC_VMOTION, sizeof(vmotion_thread_evt_t));

    *evt = VMOTION_EVT_TIMEOUT;

    HAL_TRACE_INFO("vMotion dst host timeout");

    vmn_ep->set_expiry_timer(NULL);

    sdk::event_thread::message_send(vmn_ep->get_thread_id(), (void *)evt);
}

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
            FSM_TRANSITION(EVT_START_SYNC, SM_FUNC(process_start_sync), STATE_DST_HOST_SYNCING)
            FSM_TRANSITION(EVT_EP_MV_DONE_RCVD, SM_FUNC(process_ep_move_done), STATE_DST_HOST_TERM_SYNC_START)
            FSM_TRANSITION(EVT_EP_MV_ABORT_RCVD, SM_FUNC(process_ep_move_abort), STATE_DST_HOST_END)
            FSM_TRANSITION(EVT_VMOTION_TIMEOUT, SM_FUNC(process_vmotion_timeout), STATE_DST_HOST_END)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_DST_HOST_SYNCING, 0, NULL, NULL)
            FSM_TRANSITION(EVT_RARP_RCVD, SM_FUNC(process_rarp_req), STATE_DST_HOST_TERM_SYNC_START)
            FSM_TRANSITION(EVT_EP_MV_DONE_RCVD, SM_FUNC(process_ep_move_done), STATE_DST_HOST_TERM_SYNC_START)
            FSM_TRANSITION(EVT_EP_MV_ABORT_RCVD, SM_FUNC(process_ep_move_abort), STATE_DST_HOST_END)
            FSM_TRANSITION(EVT_VMOTION_TIMEOUT, SM_FUNC(process_vmotion_timeout), STATE_DST_HOST_END)
            FSM_TRANSITION(EVT_SYNC, SM_FUNC(process_sync), STATE_DST_HOST_SYNCING)
            FSM_TRANSITION(EVT_SYNC_END, SM_FUNC(process_sync_end), STATE_DST_HOST_SYNCED)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_DST_HOST_SYNCED, 0, NULL, NULL)
            FSM_TRANSITION(EVT_RARP_RCVD, SM_FUNC(process_rarp_req), STATE_DST_HOST_TERM_SYNC_START)
            FSM_TRANSITION(EVT_EP_MV_DONE_RCVD, SM_FUNC(process_ep_move_done), STATE_DST_HOST_TERM_SYNC_START)
            FSM_TRANSITION(EVT_EP_MV_ABORT_RCVD, SM_FUNC(process_ep_move_abort), STATE_DST_HOST_END)
            FSM_TRANSITION(EVT_VMOTION_TIMEOUT, SM_FUNC(process_vmotion_timeout), STATE_DST_HOST_END)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_DST_HOST_TERM_SYNC_START, 0, NULL, NULL)
            FSM_TRANSITION(EVT_SYNC, SM_FUNC(process_sync), STATE_DST_HOST_TERM_SYNC_START)
            FSM_TRANSITION(EVT_TERM_SYNC, SM_FUNC(process_term_sync), STATE_DST_HOST_TERM_SYNCING)
            FSM_TRANSITION(EVT_TERM_SYNC_END, SM_FUNC(process_term_sync_end), STATE_DST_HOST_TERM_SYNCED)
            FSM_TRANSITION(EVT_VMOTION_TIMEOUT, SM_FUNC(process_vmotion_timeout), STATE_DST_HOST_END)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_DST_HOST_TERM_SYNCING, 0, NULL, NULL)
            FSM_TRANSITION(EVT_TERM_SYNC, SM_FUNC(process_term_sync), STATE_DST_HOST_TERM_SYNCING)
            FSM_TRANSITION(EVT_TERM_SYNC_END, SM_FUNC(process_term_sync_end), STATE_DST_HOST_TERM_SYNCED)
            FSM_TRANSITION(EVT_VMOTION_TIMEOUT, SM_FUNC(process_vmotion_timeout), STATE_DST_HOST_END)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_DST_HOST_TERM_SYNCED, 0, NULL, NULL)
            FSM_TRANSITION(EVT_DST_EP_MOVED, SM_FUNC(process_ep_moved), STATE_DST_HOST_END)
            FSM_TRANSITION(EVT_VMOTION_TIMEOUT, SM_FUNC(process_vmotion_timeout), STATE_DST_HOST_END)
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
    ep_t            *ep = vmn_ep->get_ep();
    VmotionMessage   msg_rsp;

    HAL_TRACE_INFO("Dest Host Start Sync EP: {} ptr:{:p}", vmn_ep->get_ep_handle(), (void *)ep);

    if (!ep) {
        return false;
    }

    msg_rsp.set_type(VMOTION_MSG_TYPE_INIT);
    msg_rsp.mutable_init()->set_mac_address(MAC_TO_UINT64(ep->l2_key.mac_addr));

    if (vmotion_send_msg(msg_rsp, vmn_ep->get_ssl()) != HAL_RET_OK) {
        HAL_TRACE_ERR("vmotion: unable to send sync req message");
        return false;
    }

    // Add EP Quiesce NACL entry
    ep_quiesce(ep, TRUE);
    VMOTION_FLAG_SET_EP_QUIESCE_ADDED(vmn_ep);

    return true;
}

bool
vmotion_dst_host_fsm_def::process_rarp_req(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep      *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    VmotionMessage  msg_rsp;

    HAL_TRACE_INFO("Dest Host EP: {}", vmn_ep->get_ep_handle());

    VMOTION_FLAG_SET_RARP_RCVD(vmn_ep);

    msg_rsp.set_type(VMOTION_MSG_TYPE_TERM_SYNC_REQ);

    if (vmotion_send_msg(msg_rsp, vmn_ep->get_ssl()) != HAL_RET_OK) {
        HAL_TRACE_ERR("vmotion: unable to send sync req message");
        return false;
    }
    return true;
}

bool
vmotion_dst_host_fsm_def::process_ep_move_done(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep      *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    VmotionMessage  msg_rsp;

    HAL_TRACE_INFO("Dest Host EP: {}", vmn_ep->get_ep_handle());

    VMOTION_FLAG_SET_EP_MOV_DONE_RCVD(vmn_ep);

    msg_rsp.set_type(VMOTION_MSG_TYPE_TERM_SYNC_REQ);

    if (vmotion_send_msg(msg_rsp, vmn_ep->get_ssl()) != HAL_RET_OK) {
        HAL_TRACE_ERR("vmotion: unable to send sync req message");
        return false;
    }
    return true;
}

bool
vmotion_dst_host_fsm_def::process_ep_move_abort(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep      *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    HAL_TRACE_INFO("Dest Host EP: {}", vmn_ep->get_ep_handle());
    vmn_ep->set_migration_state(MigrationState::ABORTED);
    return true;
}

bool
vmotion_dst_host_fsm_def::process_vmotion_timeout(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep      *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    HAL_TRACE_INFO("Dest Host EP: {}", vmn_ep->get_ep_handle());
    vmn_ep->set_migration_state(MigrationState::TIMEOUT);
    return true;
}

bool
vmotion_dst_host_fsm_def::process_sync(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep     *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    ep_t           *ep = vmn_ep->get_ep();
    l2seg_t        *l2seg = NULL;
    hal::pd::pd_l2seg_get_flow_lkupid_args_t args;
    pd::pd_func_args_t                       pd_func_args = {0};
    fte::fte_session_args_t sess_args;

    if (!ep) {
        HAL_TRACE_ERR("Dest Host Sync EP is not found");
        dst_host_end(vmn_ep);
        return false;
    }

    l2seg = l2seg_lookup_by_handle(ep->l2seg_handle);
    if (!l2seg) {
        HAL_TRACE_ERR("Dest Host Sync EP: {} L2SegHdl:{} L2seg not found",
                      macaddr2str(ep->l2_key.mac_addr), ep->l2seg_handle);
        return false;
    }

    args.l2seg = l2seg;
    pd_func_args.pd_l2seg_get_flow_lkupid = &args;
    hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FLOW_LKPID, &pd_func_args);

    HAL_TRACE_DEBUG("Dest Host Sync EP: {} L2SegHdl:{} VrfHdl:{} L2SegVrf:{} lkpVrf:{}",
                    macaddr2str(ep->l2_key.mac_addr), ep->l2seg_handle, ep->vrf_handle,
                    l2seg->vrf_handle, args.hwid);

    sess_args.l2seg_id   = l2seg->seg_id;
    sess_args.vrf_handle = args.hwid;
    sess_args.sync_msg   =  ((VmotionMessage *)data)->sync();

    fte::sync_session(sess_args);

    return true;
}

bool
vmotion_dst_host_fsm_def::process_sync_end(fsm_state_ctx ctx, fsm_event_data data)
{
    return true;
}

bool
vmotion_dst_host_fsm_def::process_term_sync(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep     *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    ep_t           *ep = vmn_ep->get_ep();
    l2seg_t        *l2seg = NULL;
    fte::fte_session_args_t sess_args;
    hal::pd::pd_l2seg_get_flow_lkupid_args_t args;
    pd::pd_func_args_t                       pd_func_args = {0};

    if (!ep) {
        HAL_TRACE_ERR("Dest Host term Sync EP is not found");
        dst_host_end(vmn_ep);
        return false;
    }

    l2seg = l2seg_lookup_by_handle(ep->l2seg_handle);
    if (!l2seg) {
        HAL_TRACE_ERR("Dest Host Term Sync EP: {} L2SegHdl:{} L2seg not found",
                      macaddr2str(ep->l2_key.mac_addr), ep->l2seg_handle);
        return false;
    }

    args.l2seg = l2seg;
    pd_func_args.pd_l2seg_get_flow_lkupid = &args;
    hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FLOW_LKPID, &pd_func_args);

    HAL_TRACE_DEBUG("Dest Host Term Sync EP: {} L2SegHdl:{} VrfHdl:{} L2SegVrf:{} lkpVrf:{}",
                    macaddr2str(ep->l2_key.mac_addr), ep->l2seg_handle, ep->vrf_handle,
                    l2seg->vrf_handle, args.hwid);

    sess_args.l2seg_id   = l2seg->seg_id;
    sess_args.vrf_handle = args.hwid;
    sess_args.sync_msg   =  ((VmotionMessage *)data)->sync();

    fte::sync_session(sess_args);

    return true;
}

bool
vmotion_dst_host_fsm_def::process_term_sync_end(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep      *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    VmotionMessage  msg_rsp;
    ep_t            *ep = vmn_ep->get_ep();

    HAL_TRACE_INFO("Dest Host Term Sync End EP: {} ptr:{:p}", vmn_ep->get_ep_handle(), (void *)ep);

    if (!ep) {
        return false;
    }

    endpoint_migration_normalization_cfg(ep, true);

    msg_rsp.set_type(VMOTION_MSG_TYPE_TERM_SYNC_ACK);

    if (vmotion_send_msg(msg_rsp, vmn_ep->get_ssl()) != HAL_RET_OK) {
        HAL_TRACE_ERR("vmotion: unable to send sync req message");
        return false;
    }
    return true;
}

struct rarp_proxy_request_ctx_t {
    ep_t         *ep;
    hal_ret_t     ret;
};

static void
send_proxy_rarp_request (void *data)
{
    rarp_proxy_request_ctx_t  *rarp_ctx = reinterpret_cast<rarp_proxy_request_ctx_t*>(data);
    ep_t*                      ep       = rarp_ctx->ep;
    fte::utils::arp_pkt_data_t pkt_data = { 0 };
    ip_addr_t                  src_ip_addr = { 0 };

    pkt_data.ep          = ep;
    pkt_data.src_mac     = &(ep->l2_key.mac_addr);
    pkt_data.src_ip_addr = &src_ip_addr;  // Source IP address of RARP pkt will be zero

    rarp_ctx->ret    = fte::utils::hal_inject_rarp_request_pkt(&pkt_data);

    HAL_TRACE_DEBUG("Send Rarp req EP:{} Ret:{}", macaddr2str(ep->l2_key.mac_addr), rarp_ctx->ret);

    HAL_FREE(hal::HAL_MEM_ALLOC_FTE, rarp_ctx);
}

static hal_ret_t
do_proxy_rarp_send (ep_t *ep)
{
    struct rarp_proxy_request_ctx_t *fn_ctx;
    if (ep == nullptr) {
        return HAL_RET_EP_NOT_FOUND;
    }

    fn_ctx = (struct rarp_proxy_request_ctx_t*)HAL_MALLOC(hal::HAL_MEM_ALLOC_FTE,
                                                          sizeof(struct rarp_proxy_request_ctx_t));
    fn_ctx->ep = ep;
    fte::fte_execute(0, send_proxy_rarp_request, fn_ctx);

    return HAL_RET_OK;
}

bool
vmotion_dst_host_fsm_def::process_ep_moved(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep      *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    VmotionMessage  msg_rsp;
    ep_t            *ep = vmn_ep->get_ep();

    HAL_TRACE_INFO("Dest Host EP Moved EP: {} ptr:{:p}", vmn_ep->get_ep_handle(), (void *)ep);

    if (!ep) {
        return false;
    }

    // Remove EP Quiesce NACL entry
    if (VMOTION_FLAG_IS_EP_QUIESCE_ADDED(vmn_ep)) {
        ep_quiesce(ep, FALSE);
        VMOTION_FLAG_RESET_EP_QUIESCE_ADDED(vmn_ep);
    }

    // send rarp request packet out
    do_proxy_rarp_send(ep);

    // Send EP MOVED ACK message to source host
    msg_rsp.set_type(VMOTION_MSG_TYPE_EP_MOVED_ACK);
    if (vmotion_send_msg(msg_rsp, vmn_ep->get_ssl()) != HAL_RET_OK) {
        HAL_TRACE_ERR("vmotion: unable to send ep moved ack message");
        return false;
    }

    vmn_ep->set_migration_state(MigrationState::SUCCESS);

    return true;
}

void
vmotion_dst_host_fsm_def::state_dst_host_end(fsm_state_ctx ctx)
{
    dst_host_end(reinterpret_cast<vmotion_ep *>(ctx));
}

static void
dst_host_thread_rcv_sock_msg(sdk::event_thread::io_t *io, int sock_fd, int events)
{
    vmotion_ep        *vmn_ep = (vmotion_ep *)io->ctx;
    VmotionMessage     msg;
    uint32_t           event;
    hal_ret_t          ret;

    ret = vmotion_recv_msg(msg, vmn_ep->get_ssl());

    if (ret == HAL_RET_CONN_CLOSED) {
        vmn_ep->set_migration_state(MigrationState::FAILED);
        dst_host_end(vmn_ep);
        return;
    }

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
    vmn_ep->process_event(event, (fsm_event_data)&msg);
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

    vmn_ep->get_vmotion()->delete_vmotion_ep(vmn_ep);
}

static void
dst_host_thread_rcv_event (void *message, void *ctx)
{
    vmotion_ep            *vmn_ep = (vmotion_ep *)ctx;
    vmotion_thread_evt_t  *evt = (vmotion_thread_evt_t *) message;

    HAL_TRACE_DEBUG("vMotion dst host event thread. EP: {} Event:{} Flags: {}",
                    vmn_ep->get_ep_handle(), *evt, *vmn_ep->get_flags());

    if (*evt == VMOTION_EVT_RARP_RCVD) {
        vmn_ep->process_event(EVT_RARP_RCVD, NULL);
    } else if (*evt == VMOTION_EVT_EP_MV_START) {
        vmn_ep->process_event(EVT_EP_MV_START_RCVD, NULL);
    } else if (*evt == VMOTION_EVT_EP_MV_DONE) {
        vmn_ep->process_event(EVT_EP_MV_DONE_RCVD, NULL);
    } else if (*evt == VMOTION_EVT_EP_MV_ABORT) {
        vmn_ep->process_event(EVT_EP_MV_ABORT_RCVD, NULL);
    } else if (*evt == VMOTION_EVT_TIMEOUT) {
        vmn_ep->process_event(EVT_VMOTION_TIMEOUT, NULL);
    }

    HAL_FREE(HAL_MEM_ALLOC_VMOTION, evt);
}

static hal_ret_t
dst_host_connect_to_old_host(int sock, struct sockaddr_in *addr)
{
    struct timeval tv; 
    int            flags = fcntl(sock, F_GETFL, 0);
    int            ret = 0;
    fd_set         wset;

    // Set the socket to non-blocking mode
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    ret = connect(sock, (struct sockaddr *)addr, sizeof(struct sockaddr));

    if (ret == 0) {
        // Connected successfully
        // Reset the NONBLOCK flag. We use TCP recv() in blocking way (using MSG_WAITALL)
        flags &= ~O_NONBLOCK;
        fcntl(sock, F_SETFL, flags);

        HAL_TRACE_DEBUG("connect success");
        return HAL_RET_OK;
    }

    if (errno == EINPROGRESS) {
        FD_ZERO(&wset);
        FD_SET(sock, &wset);
        tv.tv_sec  = VMOTION_CONNECT_RETRY_TIME;
        tv.tv_usec = 0;

        // Add to the select interval to check socket becomes writable
        if (select(sock + 1, NULL, &wset, NULL, &tv) > 0) {
            flags &= ~O_NONBLOCK;
            fcntl(sock, F_SETFL, flags);

            HAL_TRACE_DEBUG("connect success");
            return HAL_RET_OK;
        }
    }
    HAL_TRACE_ERR("vmotion: connection to old host failed. Addr:{} Error:{}",
                  addr->sin_addr.s_addr, strerror(errno));
    return HAL_RET_ERR;
}

hal_ret_t
vmotion_ep::dst_host_init()
{
    struct sockaddr_in  addr;
    hal_ret_t           ret = HAL_RET_OK;

    // Create client socket
    if ((sock_fd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        HAL_TRACE_ERR("socket creation failed in destination host");
        ret = HAL_RET_ERR;
        goto end;
    }

    bzero(&addr, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(get_vmotion()->get_vmotion_port());
    addr.sin_addr.s_addr = htonl(get_old_homing_host_ip().addr.v4_addr);

    HAL_TRACE_INFO("connecting to old host. Addr:{} Port:{}",
                   addr.sin_addr.s_addr, get_vmotion()->get_vmotion_port());

    // Connect to server socket
    if (dst_host_connect_to_old_host(sock_fd_, &addr) != HAL_RET_OK) {
        ret = HAL_RET_ERR;
        goto end;
    }

    tls_connection_ = get_vmotion()->get_tls_context()->init_ssl_connection(sock_fd_, FALSE);
    if (!tls_connection_) {
        ret = HAL_RET_ERR;
        goto end;
    }

    process_event(EVT_START_SYNC, NULL);

    evt_io_.ctx = this;

    sdk::event_thread::io_init(&evt_io_, dst_host_thread_rcv_sock_msg, sock_fd_, EVENT_READ);
    sdk::event_thread::io_start(&evt_io_);

end:
    if (ret == HAL_RET_ERR) {
        this->set_migration_state(MigrationState::FAILED);
        dst_host_end(this);
    }
    return ret;
}

hal_ret_t
vmotion_ep::dst_host_exit(void)
{
    HAL_TRACE_DEBUG("Destination host thread exit");

    vmotion::delay_delete_thread(evt_thread_);

    if (expiry_timer_) {
        sdk::lib::timer_delete(expiry_timer_);
    }

    if (tls_connection_) {
        TLSConnection::destroy(tls_connection_);
    }

    close(sock_fd_);

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

    // Start the timer for vMotion clean if its timed out 
    expiry_timer_ = sdk::lib::timer_schedule(HAL_TIMER_ID_VMOTION_TIMEOUT, VMOTION_TIMEOUT,
                                             reinterpret_cast<void *>(this),
                                             vmotion_dst_host_timeout_cb, false);

    evt_thread_->start(this);

    return ret;
}


} // namespace hal
