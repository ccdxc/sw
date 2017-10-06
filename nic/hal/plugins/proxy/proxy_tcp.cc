#include "nic/hal/src/proxy.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "nic/hal/src/tcpcb.hpp"
#include "nic/include/tcp_common.h"

namespace hal {
namespace proxy {

static inline hal_ret_t
update_fwding_info(fte::ctx_t&ctx, proxy_flow_info_t* pfi)
{
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_FWDING_INFO};

    // update fwding info
    flowupd.fwding.lport = pfi->proxy->lport_id;
    flowupd.fwding.qid_en = true;
    flowupd.fwding.qtype = pfi->proxy->qtype;
    if (ctx.role() ==  hal::FLOW_ROLE_INITIATOR) {
        flowupd.fwding.qid = pfi->qid1;
    } else {
        flowupd.fwding.qid = pfi->qid2;
    }

    return ctx.update_flow(flowupd);
}

fte::pipeline_action_t
tcp_exec_cpu_lif(fte::ctx_t& ctx)
{
    hal_ret_t               ret = HAL_RET_OK;
    proxy_flow_info_t*      pfi = NULL;
    flow_key_t              flow_key = ctx.key();

    // Ignore direction. Always set it to 0
    flow_key.dir = 0;

    // get the flow info for the tcp proxy service 
    pfi = proxy_get_flow_info(types::PROXY_TYPE_TCP,
                              &flow_key);

    if(!pfi) {
        HAL_TRACE_DEBUG("tcp-proxy: not enabled for flow: {}", ctx.key());
        return fte::PIPELINE_CONTINUE;
    }

    // Update iflow
    ret = update_fwding_info(ctx, pfi);

    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END; 
    }

    return fte::PIPELINE_CONTINUE;
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

} // namespace hal
} // namespace net
