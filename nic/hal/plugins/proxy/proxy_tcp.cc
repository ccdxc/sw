#include "nic/hal/src/proxy.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "nic/hal/src/tcpcb.hpp"
#include "nic/include/tcp_common.h"
#include "nic/include/interface_api.hpp"
#include "nic/hal/pd/common/pd_api.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/endpoint_pd.hpp"
#include "nic/hal/lkl/lkl_api.hpp"

namespace hal {
namespace proxy {

thread_local fte::ctx_t *gl_ctx;

#define SET_COMMON_IP_HDR(_buf, _i) \
    _buf[_i++] = 0x08; \
    _buf[_i++] = 0x00; \
    _buf[_i++] = 0x45; \
    _buf[_i++] = 0x08; \
    _buf[_i++] = 0x00; \
    _buf[_i++] = 0x7c; \
    _buf[_i++] = 0x00; \
    _buf[_i++] = 0x01; \
    _buf[_i++] = 0x00; \
    _buf[_i++] = 0x00; \
    _buf[_i++] = 0x40; \
    _buf[_i++] = 0x06; \
    _buf[_i++] = 0xfa; \
    _buf[_i++] = 0x71;

hal_ret_t
init_tcp_cb(fte::ctx_t &ctx, qid_t qid, TcpCbSpec &spec)
{
    hal_ret_t       ret = HAL_RET_OK;
    uint8_t         buf[64];
    uint8_t         i = 0;
    uint8_t         vlan_valid;
    uint16_t        vlan_id;

    spec.set_source_port(ctx.key().dport);
    spec.set_dest_port(ctx.key().sport);
    spec.set_source_lif(hal::pd::ep_pd_get_hw_lif_id(ctx.dep()));

    if_l2seg_get_encap(ctx.dif(), ctx.dl2seg(), &vlan_valid, &vlan_id);

    memcpy(&buf[i], ctx.sep()->l2_key.mac_addr, sizeof(ctx.sep()->l2_key.mac_addr));
    i += sizeof(ctx.sep()->l2_key.mac_addr);
    memcpy(&buf[i], ctx.dep()->l2_key.mac_addr, sizeof(ctx.dep()->l2_key.mac_addr));
    i += sizeof(ctx.dep()->l2_key.mac_addr);
    if (vlan_valid) {
        buf[i++] = 0x81;
        buf[i++] = 0x00;
        vlan_id = htons(vlan_id);
        memcpy(&buf[i], &vlan_id, sizeof(vlan_id));
        i += sizeof(vlan_id);
    }
    SET_COMMON_IP_HDR(buf, i);
    memcpy(buf, &ctx.key().dip.v4_addr, sizeof(ctx.key().dip.v4_addr));
    i += sizeof(ctx.key().dip.v4_addr);
    memcpy(buf, &ctx.key().sip.v4_addr, sizeof(ctx.key().sip.v4_addr));
    i += sizeof(ctx.key().sip.v4_addr);

    HAL_ABORT(i < sizeof(buf));

    spec.set_header_template(buf, i + 1);
    return ret;
}

#if 0
hal_ret_t
tcp_create_cb(qid_t qid)
{
    hal_ret_t       ret = HAL_RET_OK;
    TcpCbSpec       spec;
    TcpCbResponse   rsp;
    fte::ctx_t &ctx = *gl_ctx;
    if(ctx.dif() == NULL || ctx.dif()->if_type == intf::IF_TYPE_TUNNEL) {
        HAL_TRACE_DEBUG("Skipping TCPCB creation for TUNNEL interface");
        return HAL_RET_OK;
    }

    HAL_TRACE_DEBUG("Create TCPCB for qid: {}", qid);
    spec.mutable_key_or_handle()->set_tcpcb_id(qid);
    ret = init_tcp_cb(ctx, qid, spec);
    ret = tcpcb_create(spec, &rsp);
    if(ret != HAL_RET_OK || rsp.api_status() != types::API_STATUS_OK) {
        HAL_TRACE_ERR("Failed to create TCP cb for id: {}, ret: {}, rsp: ",
                        qid, ret, rsp.api_status());
        return ret;
    }
    HAL_TRACE_DEBUG("Successfully created TCPCB for id: {}", qid);
    return ret;
}
#endif

hal_ret_t
tcp_create_cb(fte::ctx_t &ctx, qid_t qid)
{
    hal_ret_t       ret = HAL_RET_OK;
    TcpCbSpec       spec;
    TcpCbResponse   rsp;

    if(ctx.dif() == NULL || ctx.dif()->if_type == intf::IF_TYPE_TUNNEL) {
        HAL_TRACE_DEBUG("Skipping TCPCB creation for TUNNEL interface");
        return HAL_RET_OK;
    }
    HAL_TRACE_DEBUG("Create TCPCB for qid: {}", qid);
    spec.mutable_key_or_handle()->set_tcpcb_id(qid);
    ret = init_tcp_cb(ctx, qid, spec);
    ret = tcpcb_create(spec, &rsp);
    if(ret != HAL_RET_OK || rsp.api_status() != types::API_STATUS_OK) {
        HAL_TRACE_ERR("Failed to create TCP cb for id: {}, ret: {}, rsp: ",
                        qid, ret, rsp.api_status());
        return ret;
    }
    return ret;
}

void
tcp_update_cb(void *tcpcb, uint32_t qid)
{
    hal_ret_t ret = HAL_RET_OK;
    TcpCbSpec *spec = new TcpCbSpec;
    TcpCbResponse rsp;
    TcpCbGetRequest *get_req = new TcpCbGetRequest;
    TcpCbGetResponse get_rsp;
    TcpCbKeyHandle kh;
    uint8_t data[64];

    if (!gl_ctx) {
        HAL_TRACE_DEBUG("tcp_update_cb not called from current thread");
        return;
    }
    kh.set_tcpcb_id(qid);
    get_req->set_allocated_key_or_handle(&kh);
    tcpcb_get(*get_req, &get_rsp);
    HAL_TRACE_DEBUG("Get response: {}", get_rsp.api_status());
    spec->set_allocated_key_or_handle(&kh);
    if (tcpcb) {
        HAL_TRACE_DEBUG("lkl rcv_nxt={}, snd_nxt={}, snd_una={}, rcv_tsval={}, ts_recent={}",
                        hal::pd::lkl_get_tcpcb_rcv_nxt(tcpcb),
                        hal::pd::lkl_get_tcpcb_snd_nxt(tcpcb),
                        hal::pd::lkl_get_tcpcb_snd_una(tcpcb),
                        hal::pd::lkl_get_tcpcb_rcv_tsval(tcpcb),
                        hal::pd::lkl_get_tcpcb_ts_recent(tcpcb));

        spec->set_rcv_nxt(hal::pd::lkl_get_tcpcb_rcv_nxt(tcpcb));
        spec->set_snd_nxt(hal::pd::lkl_get_tcpcb_snd_nxt(tcpcb));
        spec->set_snd_una(hal::pd::lkl_get_tcpcb_snd_una(tcpcb));
        spec->set_rcv_tsval(hal::pd::lkl_get_tcpcb_rcv_tsval(tcpcb));
        spec->set_ts_recent(hal::pd::lkl_get_tcpcb_ts_recent(tcpcb));
    }
    spec->set_serq_base(get_rsp.mutable_spec()->serq_base());
    spec->set_debug_dol(get_rsp.mutable_spec()->debug_dol());
    spec->set_sesq_base(get_rsp.mutable_spec()->sesq_base());
    spec->set_sesq_pi(get_rsp.mutable_spec()->sesq_pi());
    spec->set_sesq_ci(get_rsp.mutable_spec()->sesq_ci());
    spec->set_snd_wnd(get_rsp.mutable_spec()->snd_wnd());
    spec->set_snd_cwnd(get_rsp.mutable_spec()->snd_cwnd());
    spec->set_rcv_mss(get_rsp.mutable_spec()->rcv_mss());
    spec->set_source_port(get_rsp.mutable_spec()->source_port());
    spec->set_dest_port(get_rsp.mutable_spec()->dest_port());
    spec->set_header_len(get_rsp.mutable_spec()->header_len());

    memcpy(data,
           get_rsp.mutable_spec()->header_template().c_str(),
           std::max(get_rsp.mutable_spec()->header_template().size(), sizeof(data)));

    spec->set_header_template(data, sizeof(data));

    spec->set_state(TCP_ESTABLISHED);
    spec->set_source_lif(get_rsp.mutable_spec()->source_lif());
    spec->set_asesq_base(get_rsp.mutable_spec()->asesq_base());
    spec->set_asesq_pi(get_rsp.mutable_spec()->asesq_pi());
    spec->set_asesq_ci(get_rsp.mutable_spec()->asesq_ci());
    spec->set_debug_dol_tx(get_rsp.mutable_spec()->debug_dol_tx());
    spec->set_pending_ack_send(true);

    HAL_TRACE_DEBUG("Calling TCPCB Update with id: {}", qid);
    ret = tcpcb_update(*spec, &rsp);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("tcbcb update failed {}", ret);
        return;
    }
}

hal_ret_t
tcp_trigger_ack_send(uint32_t qid)
{
    hal_ret_t ret = HAL_RET_OK;
    TcpCbSpec *spec = new TcpCbSpec;
    TcpCbResponse rsp;
    TcpCbGetRequest *get_req = new TcpCbGetRequest;
    TcpCbGetResponse get_rsp;
    TcpCbKeyHandle kh;
    uint8_t data[64];

    kh.set_tcpcb_id(qid);
    get_req->set_allocated_key_or_handle(&kh);
    tcpcb_get(*get_req, &get_rsp);

    spec->set_allocated_key_or_handle(&kh);
    spec->set_rcv_nxt(get_rsp.mutable_spec()->rcv_nxt());
    spec->set_snd_nxt(get_rsp.mutable_spec()->snd_nxt());
    spec->set_snd_una(get_rsp.mutable_spec()->snd_una());
    spec->set_rcv_tsval(get_rsp.mutable_spec()->rcv_tsval());
    spec->set_ts_recent(get_rsp.mutable_spec()->ts_recent());
    spec->set_serq_base(get_rsp.mutable_spec()->serq_base());
    spec->set_debug_dol(get_rsp.mutable_spec()->debug_dol());
    spec->set_sesq_base(get_rsp.mutable_spec()->sesq_base());
    spec->set_sesq_pi(get_rsp.mutable_spec()->sesq_pi());
    spec->set_sesq_ci(get_rsp.mutable_spec()->sesq_ci());
    spec->set_snd_wnd(get_rsp.mutable_spec()->snd_wnd());
    spec->set_snd_cwnd(get_rsp.mutable_spec()->snd_cwnd());
    spec->set_rcv_mss(get_rsp.mutable_spec()->rcv_mss());
    spec->set_source_port(get_rsp.mutable_spec()->source_port());
    spec->set_dest_port(get_rsp.mutable_spec()->dest_port());
    spec->set_header_len(get_rsp.mutable_spec()->header_len());
    
    memcpy(data,
           get_rsp.mutable_spec()->header_template().c_str(), 
           std::max(get_rsp.mutable_spec()->header_template().size(), sizeof(data)));

    spec->set_header_template(data, sizeof(data));

    spec->set_state(TCP_ESTABLISHED);
    spec->set_source_lif(get_rsp.mutable_spec()->source_lif());
    spec->set_asesq_base(get_rsp.mutable_spec()->asesq_base());
    spec->set_asesq_pi(get_rsp.mutable_spec()->asesq_pi());
    spec->set_asesq_ci(get_rsp.mutable_spec()->asesq_ci());
    spec->set_debug_dol_tx(get_rsp.mutable_spec()->debug_dol_tx());
    spec->set_pending_ack_send(true);


    ret = tcpcb_update(*spec, &rsp);

    ret = hal::pd::cpupkt_program_send_ring_doorbell(SERVICE_LIF_TCP_PROXY,
                                                     0,
                                                     qid,
                                                     TCP_SCHED_RING_PENDING);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    return ret;
}

void tcp_ring_doorbell(uint32_t qid)
{
    hal_ret_t ret = HAL_RET_OK;
    ret = hal::pd::cpupkt_program_send_ring_doorbell(SERVICE_LIF_TCP_PROXY,
                                                     0,
                                                     qid,
                                                     TCP_SCHED_RING_PENDING);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("ring doorbell failed {}", ret);
    }
}

static inline hal_ret_t
update_fwding_info(fte::ctx_t&ctx, proxy_flow_info_t* pfi)
{
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_FWDING_INFO};

    // update fwding info
    flowupd.fwding.lport = pfi->proxy->meta->lif_info[0].lport_id;
    flowupd.fwding.qid_en = true;
    flowupd.fwding.qtype = pfi->proxy->meta->lif_info[0].qtype_info[0].qtype_val;
    if (ctx.role() ==  hal::FLOW_ROLE_INITIATOR) {
        flowupd.fwding.qid = pfi->qid1;
        tcp_create_cb(ctx, pfi->qid1);
    } else {
        flowupd.fwding.qid = pfi->qid2;
        tcp_create_cb(ctx, pfi->qid2);
    }

    return ctx.update_flow(flowupd);
}

fte::pipeline_action_t
tcp_exec_cpu_lif(fte::ctx_t& ctx)
{
    hal_ret_t               ret = HAL_RET_OK;
    gl_ctx = &ctx;
    proxy_flow_info_t*      pfi = NULL;
    flow_key_t              flow_key = ctx.key();

    // Ignore direction. Always set it to 0
    flow_key.dir = 0;

    // Check if TCP proxy is enabled for the flow
    if(!is_proxy_enabled_for_flow(types::PROXY_TYPE_TCP,
                                 flow_key)) {
        HAL_TRACE_DEBUG("tcp-proxy: not enabled for flow: {}", ctx.key());
        return fte::PIPELINE_CONTINUE;
    }

    // get the flow info for the tcp proxy service 
    pfi = proxy_get_flow_info(types::PROXY_TYPE_TCP,
                              &flow_key);

    if(!pfi) {
        // Allocate PFI for the flow
        ret = proxy_flow_enable(types::PROXY_TYPE_TCP, flow_key, true, NULL, NULL);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_DEBUG("tcp-proxy: failed to enable service for the flow: {}", ctx.key());
            ctx.set_feature_status(ret);
            return fte::PIPELINE_END;
        }
        pfi = proxy_get_flow_info(types::PROXY_TYPE_TCP, &flow_key);
        HAL_ASSERT_RETURN((NULL != pfi), fte::PIPELINE_CONTINUE);
    }

    // Update iflow
    ret = update_fwding_info(ctx, pfi);

    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END; 
    }


    if (ctx.role() == hal::FLOW_ROLE_RESPONDER) {
        HAL_TRACE_DEBUG("tcp-proxy: responder side. ignoring.");
        return fte::PIPELINE_CONTINUE;
    }

#if 0
    if (!ctx.protobuf_request()) {
        HAL_TRACE_DEBUG("LKL return {}",
                        hal::pd::lkl_handle_flow_miss_pkt(
                                 hal::pd::lkl_alloc_skbuff(ctx.cpu_rxhdr(),
                                                           ctx.pkt(),
                                                           ctx.pkt_len(),
                                                           ctx.direction()),
                                 ctx.direction(),
                                 pfi->qid1, pfi->qid2));
    }
#endif
    return fte::PIPELINE_CONTINUE;
}

#if 0
fte::pipeline_action_t
tcp_exec_tcp_lif(fte::ctx_t& ctx)
{
    HAL_TRACE_DEBUG("tcp_exec_tcp_lif: LKL return {}",
                     hal::pd::lkl_handle_flow_hit_pkt(
                                 hal::pd::lkl_alloc_skbuff(ctx.cpu_rxhdr(),
                                                           ctx.pkt(),
                                                           ctx.pkt_len(),
                                                           ctx.direction()),
                                 ctx.direction(),
                                 ctx.cpu_rxhdr()));

    return fte::PIPELINE_CONTINUE;
}
#endif
fte::pipeline_action_t
tcp_exec_tcp_lif(fte::ctx_t& ctx)
{
    hal_ret_t ret = HAL_RET_OK;
    const fte::cpu_rxhdr_t* cpu_rxhdr = ctx.cpu_rxhdr();

    ret = tcp_trigger_ack_send(cpu_rxhdr->qid);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, fte::PIPELINE_CONTINUE);

    return fte::PIPELINE_CONTINUE;
}

fte::pipeline_action_t
tcp_exec(fte::ctx_t& ctx)
{
    const fte::cpu_rxhdr_t* cpu_rxhdr = ctx.cpu_rxhdr();

    if (cpu_rxhdr && (cpu_rxhdr->lif == hal::SERVICE_LIF_TCP_PROXY)) {
      return tcp_exec_tcp_lif(ctx);
    } else {
      return tcp_exec_cpu_lif(ctx);
    }

    return fte::PIPELINE_CONTINUE;
}

void
tcp_transmit_pkt(unsigned char* pkt, unsigned int len, bool is_connect_req, uint16_t dst_lif) {
    if (gl_ctx) {
        if (true){//is_connect_req) {
            gl_ctx->queue_txpkt(pkt, len);
        } else {
            gl_ctx->queue_txpkt(pkt, len, NULL, NULL, dst_lif, CPU_ASQ_QTYPE, CPU_ASQ_QID, CPU_SCHED_RING_ASQ);
        }
    }
}

} // namespace hal
} // namespace net
