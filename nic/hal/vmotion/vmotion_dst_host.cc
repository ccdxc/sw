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
dst_host_end (vmotion_ep *vmn_ep, MigrationState migration_state)
{
    auto ep = vmn_ep->get_ep();

    HAL_TRACE_INFO("Dest Host end EP: {}", macaddr2str(ep->l2_key.mac_addr));
    // TODO - Check whether quiesce entry is there or not, if there delete it.
    // TODO - Clean up any other things to be cleaned up. e.g. timer

    // Send success/failure notification to Net Agent
    endpoint_migration_status_update (ep, migration_state);

    ep->vmotion_state = migration_state;

    // Stop the watcher
    vmn_ep->get_event_thread()->stop();
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
            FSM_TRANSITION(EVT_START_SYNC, SM_FUNC(process_start_sync), STATE_DST_HOST_SYNC_REQ)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_DST_HOST_SYNC_REQ, 0, NULL, NULL)
            FSM_TRANSITION(EVT_RARP_RCVD, SM_FUNC(process_rarp_req), STATE_DST_HOST_SYNC_REQ)
            FSM_TRANSITION(EVT_EP_MV_DONE_RCVD, SM_FUNC(process_ep_move_done), STATE_DST_HOST_SYNC_REQ)
            FSM_TRANSITION(EVT_SYNC, SM_FUNC(process_sync), STATE_DST_HOST_SYNCING)
            FSM_TRANSITION(EVT_SYNC_END, SM_FUNC(process_sync_end), STATE_DST_HOST_SYNCED)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_DST_HOST_SYNCING, 0, NULL, NULL)
            FSM_TRANSITION(EVT_RARP_RCVD, SM_FUNC(process_rarp_req), STATE_DST_HOST_SYNCING)
            FSM_TRANSITION(EVT_EP_MV_DONE_RCVD, SM_FUNC(process_ep_move_done), STATE_DST_HOST_SYNCING)
            FSM_TRANSITION(EVT_SYNC_END, SM_FUNC(process_sync_end), STATE_DST_HOST_SYNCED)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_DST_HOST_SYNCED, 0, SM_FUNC_ARG_1(state_dst_host_synced), NULL)
            FSM_TRANSITION(EVT_RARP_RCVD, SM_FUNC(process_send_term_sync_rarp), STATE_DST_HOST_SYNCED)
            FSM_TRANSITION(EVT_EP_MV_DONE_RCVD, SM_FUNC(process_send_term_sync_ep_move_done), STATE_DST_HOST_SYNCED)
            FSM_TRANSITION(EVT_DST_TERM_SYNC_REQ, SM_FUNC(process_send_term_req), STATE_DST_HOST_TERM_SYNC_START)
        FSM_STATE_END
        FSM_STATE_BEGIN(STATE_DST_HOST_TERM_SYNC_START, 0, NULL, NULL)
            FSM_TRANSITION(EVT_TERM_SYNC, SM_FUNC(process_term_sync), STATE_DST_HOST_TERM_SYNCING)
            FSM_TRANSITION(EVT_TERM_SYNC_END, SM_FUNC(process_term_sync_end), STATE_DST_HOST_TERM_SYNCED)
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

    HAL_TRACE_INFO("Dest Host Start Sync EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));

    msg_rsp.set_type(VMOTION_MSG_TYPE_INIT);
    msg_rsp.mutable_init()->set_mac_address(MAC_TO_UINT64(vmn_ep->get_ep()->l2_key.mac_addr));

    if (vmotion_send_msg(msg_rsp, vmn_ep->get_socket_fd()) != HAL_RET_OK) {
        HAL_TRACE_ERR("vmotion: unable to send sync req message");
        return false;
    }

    // Add EP Quiesce NACL entry
    ep_quiesce(vmn_ep->get_ep(), TRUE);

    return true;
}

bool
vmotion_dst_host_fsm_def::process_rarp_req(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep      *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    HAL_TRACE_INFO("Dest Host EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));
    VMOTION_FLAG_SET_RARP_RCVD(vmn_ep);
    return true;
}

bool
vmotion_dst_host_fsm_def::process_ep_move_done(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep      *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    HAL_TRACE_INFO("Dest Host EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));
    VMOTION_FLAG_SET_EP_MOV_DONE_RCVD(vmn_ep);
    return true;
}

bool
vmotion_dst_host_fsm_def::process_sync(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep     *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    ep_t           *ep = vmn_ep->get_ep();
    l2seg_t        *l2seg = NULL;
    session_t      *session = NULL;
    SessionResponse rsp;
    hal::pd::pd_l2seg_get_flow_lkupid_args_t args;
    pd::pd_func_args_t                       pd_func_args = {0};

    if (!ep) {
        HAL_TRACE_ERR("Dest Host Sync EP is not found");
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

    for (auto i = 0; i < ((VmotionMessage *)data)->sync().sessions_size(); i++) {
        // Mark the session as syncing
        ((VmotionMessage *)data)->mutable_sync()->mutable_sessions(i)->\
                                  mutable_status()->set_session_syncing(1);
        ((VmotionMessage *)data)->mutable_sync()->mutable_sessions(i)->\
                                  mutable_status()->set_lookup_vrf(args.hwid);
        ((VmotionMessage *)data)->mutable_sync()->mutable_sessions(i)->\
                                  mutable_status()->set_l2seg_id(l2seg->seg_id);

        if ((((VmotionMessage *)data)->mutable_sync()->mutable_sessions(i)->\
                                       mutable_spec()->has_initiator_flow()) &&
            ((VmotionMessage *)data)->mutable_sync()->mutable_sessions(i)->\
                                      mutable_spec()->mutable_initiator_flow()->\
                                      mutable_flow_key()->has_l2_key())
        {
            HAL_TRACE_DEBUG("Dest Host Sync Spec L2Seg:{} EP L2Seg:{}",
                    (((VmotionMessage *)data)->mutable_sync()->mutable_sessions(i)->\
                    mutable_spec()->mutable_initiator_flow()->mutable_flow_key()->\
                    mutable_l2_key()->l2_segment_id()), ep->l2_key.l2_segid);

            ((VmotionMessage *)data)->mutable_sync()->mutable_sessions(i)->mutable_spec()->\
                mutable_initiator_flow()->mutable_flow_key()->mutable_l2_key()->\
                set_l2_segment_id(ep->l2_key.l2_segid);
        }

        auto spec   = ((VmotionMessage *)data)->sync().sessions(i).spec();
        auto status = ((VmotionMessage *)data)->sync().sessions(i).status();
        auto stats  = ((VmotionMessage *)data)->sync().sessions(i).stats();

        session = find_session_from_spec(spec, args.hwid);

        if (!session) {
            HAL_TRACE_INFO("Dest Host Sync Session not found");
            fte::session_create(spec, status, stats, &rsp);
        } else {
            HAL_TRACE_INFO("Dest Host Sync Session found");
        }
    }
    return true;
}

bool
vmotion_dst_host_fsm_def::process_sync_end(fsm_state_ctx ctx, fsm_event_data data)
{
    return true;
}

bool
vmotion_dst_host_fsm_def::process_send_term_req(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    VmotionMessage  msg_rsp;
    msg_rsp.set_type(VMOTION_MSG_TYPE_TERM_SYNC_REQ);

    HAL_TRACE_INFO("Dest Host Synced. EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));

    if (vmotion_send_msg(msg_rsp, vmn_ep->get_socket_fd()) != HAL_RET_OK) {
        HAL_TRACE_ERR("vmotion: unable to send sync req message");
        return false;
    }
    return true;
}

bool
vmotion_dst_host_fsm_def::process_send_term_sync_rarp(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);

    HAL_TRACE_INFO("Dest Host Synced. EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));

    VMOTION_FLAG_SET_RARP_RCVD(vmn_ep);
    if ((VMOTION_FLAG_IS_RARP_SET(vmn_ep) || VMOTION_FLAG_IS_EP_MOV_DONE_SET(vmn_ep))) {
        vmn_ep->throw_event(EVT_DST_TERM_SYNC_REQ, NULL);
    }
    return true;
}

bool
vmotion_dst_host_fsm_def::process_send_term_sync_ep_move_done(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);

    HAL_TRACE_INFO("Dest Host Synced. EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));

    VMOTION_FLAG_SET_EP_MOV_DONE_RCVD(vmn_ep);
    if ((VMOTION_FLAG_IS_RARP_SET(vmn_ep) || VMOTION_FLAG_IS_EP_MOV_DONE_SET(vmn_ep))) {
        vmn_ep->throw_event(EVT_DST_TERM_SYNC_REQ, NULL);
    }
    return true;
}

bool
vmotion_dst_host_fsm_def::process_term_sync(fsm_state_ctx ctx, fsm_event_data data)
{
    vmotion_ep     *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    ep_t           *ep = vmn_ep->get_ep();
    l2seg_t        *l2seg = NULL;
    session_t      *session = NULL;
    SessionResponse rsp;
    hal::pd::pd_l2seg_get_flow_lkupid_args_t args;
    pd::pd_func_args_t                       pd_func_args = {0};

    // Set the feature id to run update on
    uint64_t      bitmap = (uint64_t)(1 << fte::feature_id("pensando.io/network:fwding"));

    if (!ep) {
        HAL_TRACE_ERR("Dest Host term Sync EP is not found");
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

    for (auto i = 0; i < ((VmotionMessage *)data)->sync().sessions_size(); i++) {
        // Mark the session as syncing
        ((VmotionMessage *)data)->mutable_sync()->mutable_sessions(i)->\
                                  mutable_status()->set_session_syncing(1);
        ((VmotionMessage *)data)->mutable_sync()->mutable_sessions(i)->\
                                  mutable_status()->set_lookup_vrf(args.hwid);
        ((VmotionMessage *)data)->mutable_sync()->mutable_sessions(i)->\
                                  mutable_status()->set_l2seg_id(l2seg->seg_id);

        if ((((VmotionMessage *)data)->mutable_sync()->mutable_sessions(i)->\
                                       mutable_spec()->has_initiator_flow()) &&
            ((VmotionMessage *)data)->mutable_sync()->mutable_sessions(i)->\
                                      mutable_spec()->mutable_initiator_flow()->\
                                      mutable_flow_key()->has_l2_key())
        {
            HAL_TRACE_DEBUG("Dest Host Term Sync Spec L2Seg:{} EP L2Seg:{}",
                    (((VmotionMessage *)data)->mutable_sync()->mutable_sessions(i)->\
                    mutable_spec()->mutable_initiator_flow()->mutable_flow_key()->\
                    mutable_l2_key()->l2_segment_id()), ep->l2_key.l2_segid);

            ((VmotionMessage *)data)->mutable_sync()->mutable_sessions(i)->mutable_spec()->\
                mutable_initiator_flow()->mutable_flow_key()->mutable_l2_key()->\
                set_l2_segment_id(ep->l2_key.l2_segid);
        }

        auto spec   = ((VmotionMessage *)data)->sync().sessions(i).spec();
        auto status = ((VmotionMessage *)data)->sync().sessions(i).status();
        auto stats  = ((VmotionMessage *)data)->sync().sessions(i).stats();

        session = find_session_from_spec(spec, args.hwid);

        HAL_TRACE_INFO("Dest Host Term Sync - Session {:p}. Syncing:{}", (void *)session,
                        session ? session->syncing_session : 0);
        if (!session) {
            fte::session_create(spec, status, stats, &rsp);
        } else {
            // If its syncing session only, process term sync update
            if (session->syncing_session) {
                fte::session_update(spec, status, stats, &rsp, bitmap);
            }
        }
    }
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

    HAL_TRACE_INFO("Dest Host EP Moved EP: {}", macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr));

    // Remove EP Quiesce NACL entry
    ep_quiesce(vmn_ep->get_ep(), FALSE);

    // send rarp request packet out
    do_proxy_rarp_send(vmn_ep->get_ep());

    // Loop the sessions, and start aging timer
    endpoint_migration_session_age_reset(vmn_ep->get_ep());

    // Send EP MOVED ACK message to source host
    msg_rsp.set_type(VMOTION_MSG_TYPE_EP_MOVED_ACK);
    if (vmotion_send_msg(msg_rsp, vmn_ep->get_socket_fd()) != HAL_RET_OK) {
        HAL_TRACE_ERR("vmotion: unable to send ep moved ack message");
        return false;
    }

    return true;
}

void
vmotion_dst_host_fsm_def::state_dst_host_synced(fsm_state_ctx ctx)
{
    vmotion_ep *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);

    HAL_TRACE_INFO("State Host Synced EP: {} Flags: {}",
                   macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr), *vmn_ep->get_flags());

    if ((VMOTION_FLAG_IS_RARP_SET(vmn_ep) || VMOTION_FLAG_IS_EP_MOV_DONE_SET(vmn_ep))) {
        vmn_ep->throw_event(EVT_DST_TERM_SYNC_REQ, NULL);
    }
    return;
}

void
vmotion_dst_host_fsm_def::state_dst_host_end(fsm_state_ctx ctx)
{
    vmotion_ep *vmn_ep = reinterpret_cast<vmotion_ep *>(ctx);
    auto        ep = vmn_ep->get_ep();

    HAL_TRACE_INFO("State dest Host end EP: {}", macaddr2str(ep->l2_key.mac_addr));

    dst_host_end(vmn_ep, MigrationState::SUCCESS);
}

static void
dst_host_thread_rcv_sock_msg(sdk::event_thread::io_t *io, int sock_fd, int events)
{
    vmotion_ep        *vmn_ep = (vmotion_ep *)io->ctx;
    VmotionMessage     msg;
    uint32_t           event;
    hal_ret_t          ret;

    ret = vmotion_recv_msg(msg, sock_fd);

    if (ret == HAL_RET_CONN_CLOSED) {
        dst_host_end(vmn_ep, MigrationState::FAILED);
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
                    macaddr2str(vmn_ep->get_ep()->l2_key.mac_addr), *evt, *vmn_ep->get_flags());

    if (*evt == VMOTION_EVT_RARP_RCVD) {
        vmn_ep->process_event(EVT_RARP_RCVD, NULL);
    } else if (*evt == VMOTION_EVT_EP_MV_START) {
        vmn_ep->process_event(EVT_EP_MV_START_RCVD, NULL);
    } else if (*evt == VMOTION_EVT_EP_MV_DONE) {
        vmn_ep->process_event(EVT_EP_MV_DONE_RCVD, NULL);
    } else if (*evt == VMOTION_EVT_EP_MV_ABORT) {
        vmn_ep->process_event(EVT_EP_MV_ABORT_RCVD, NULL);
    }

    HAL_FREE(HAL_MEM_ALLOC_VMOTION, evt);
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
    addr.sin_port        = htons(50055); // TEMP
    addr.sin_addr.s_addr = htonl(get_old_homing_host_ip().addr.v4_addr);

    HAL_TRACE_INFO("connecting to old host. Addr:{}", addr.sin_addr.s_addr);

    // Connect to server socket
    if (connect(sock_fd_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        HAL_TRACE_ERR("vmotion: connection to old host failed. Addr:{} Error:{}",
                      addr.sin_addr.s_addr, strerror(errno));
        ret = HAL_RET_ERR;
        goto end;
    }

    process_event(EVT_START_SYNC, NULL);

    evt_io_.ctx = this;

    sdk::event_thread::io_init(&evt_io_, dst_host_thread_rcv_sock_msg, sock_fd_, EVENT_READ);
    sdk::event_thread::io_start(&evt_io_);

end:
    if (ret == HAL_RET_ERR) {
        dst_host_end(this, MigrationState::FAILED);
    }

    return HAL_RET_OK;
}

hal_ret_t
vmotion_ep::dst_host_exit(void)
{
    HAL_TRACE_DEBUG("Destination host thread exit");

    sdk::event_thread::event_thread::destroy(evt_thread_);

    vmotion_ptr_->release_thread_id(thread_id_);

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

    evt_thread_->start(this);

    return ret;
}

} // namespace hal
