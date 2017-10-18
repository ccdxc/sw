#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "nic/hal/src/proxy.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "nic/hal/src/tcpcb.hpp"
#include "nic/include/tcp_common.h"
#include "nic/include/interface_api.hpp"
#include "nic/hal/pd/common/pd_api.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/endpoint_pd.hpp"
#include "nic/hal/lkl/lkl_api.hpp"
#include "nic/p4/nw/include/defines.h"
#include "nic/asm/cpu-p4plus/include/cpu-defines.h"

namespace hal {
namespace proxy {

thread_local fte::ctx_t *gl_ctx;

#define SET_COMMON_IP_HDR(_buf, _i) \
    _buf[_i++] = 0x08; \
    _buf[_i++] = 0x00; \
    _buf[_i++] = 0x45; \
    _buf[_i++] = 0x08; \
    _buf[_i++] = 0x00; \
    _buf[_i++] = 0x28; \
    _buf[_i++] = 0x00; \
    _buf[_i++] = 0x01; \
    _buf[_i++] = 0x00; \
    _buf[_i++] = 0x00; \
    _buf[_i++] = 0x40; \
    _buf[_i++] = 0x06; \
    _buf[_i++] = 0xfa; \
    _buf[_i++] = 0x71;

// byte array to hex string for logging
std::string hex_dump(const uint8_t *buf, size_t sz)
{
    std::ostringstream result;

    for(size_t i = 0; i < sz; i+=8) {
        result << " 0x";
        for (size_t j = i ; j < sz && j < i+8; j++) {
            result << std::setw(2) << std::setfill('0') << std::hex << (int)buf[j];
        }
    }

    return result.str();
}


hal_ret_t
proxy_create_hdr_template(TcpCbSpec &spec,
                          ether_header_t *eth,
                          vlan_header_t* vlan,
                          ipv4_header_t *ip,
                          tcp_header_t *tcp,
                          bool is_itor_dir)
{
    hal_ret_t       ret = HAL_RET_OK;
    uint8_t         buf[64];
    uint8_t         i = 0;
    uint16_t        vlan_id;
    uint16_t        sport, dport;
    uint32_t        sip, dip;
    mac_addr_t      smac, dmac;

    HAL_TRACE_DEBUG("tcp-proxy: header template eth={}", hex_dump((uint8_t*)eth, 18));
    HAL_TRACE_DEBUG("tcp-proxy: header template ip={}", hex_dump((uint8_t*)ip, sizeof(ipv4_header_t)));
    HAL_TRACE_DEBUG("tcp-proxy: header template tcp={}", hex_dump((uint8_t*)tcp, sizeof(tcp_header_t)));

    if (is_itor_dir) {
        sport = tcp->sport;
        dport = tcp->dport;
        memcpy(&sip, &ip->saddr, sizeof(ip->saddr));
        memcpy(&dip, &ip->daddr, sizeof(ip->daddr));
        memcpy(&smac, eth->smac, ETH_ADDR_LEN);
        memcpy(&dmac, eth->dmac, ETH_ADDR_LEN);
        vlan_id = vlan->vlan_tag;
        //spec.set_source_lif(hal::pd::ep_pd_get_hw_lif_id(ctx.dep()));
    } else {
        sport = tcp->dport;
        dport = tcp->sport;
        memcpy(&sip, &ip->daddr, sizeof(ip->daddr));
        memcpy(&dip, &ip->saddr, sizeof(ip->saddr));
        memcpy(&smac, eth->dmac, ETH_ADDR_LEN);
        memcpy(&dmac, eth->smac, ETH_ADDR_LEN);
        vlan_id = vlan->vlan_tag;
        //spec.set_source_lif(hal::pd::ep_pd_get_hw_lif_id(ctx.sep()));
     }
    HAL_TRACE_DEBUG("tcp-proxy: sport={}", hex_dump((uint8_t*)&sport, sizeof(sport)));
    HAL_TRACE_DEBUG("tcp-proxy: dport={}", hex_dump((uint8_t*)&dport, sizeof(dport)));
    HAL_TRACE_DEBUG("tcp-proxy: sip={}", hex_dump((uint8_t*)&sip, sizeof(sip)));
    HAL_TRACE_DEBUG("tcp-proxy: dip={}", hex_dump((uint8_t*)&dip, sizeof(dip)));
    HAL_TRACE_DEBUG("tcp-proxy: smac={}", hex_dump((uint8_t*)smac, sizeof(smac)));
    HAL_TRACE_DEBUG("tcp-proxy: dmac={}", hex_dump((uint8_t*)dmac, sizeof(dmac)));
    HAL_TRACE_DEBUG("tcp-proxy: vlan={}", hex_dump((uint8_t*)&vlan_id, sizeof(vlan_id)));

    spec.set_source_port(ntohs(dport));
    spec.set_dest_port(ntohs(sport));
    memcpy(&buf[i], &smac, sizeof(smac));
    i += sizeof(smac);
    memcpy(&buf[i], &dmac, sizeof(dmac));
    i += sizeof(smac);
    buf[i++] = 0x81;
    buf[i++] = 0x00;
    memcpy(&buf[i], &vlan_id, sizeof(vlan_id));
    i += sizeof(vlan_id);
    SET_COMMON_IP_HDR(buf, i);
    memcpy(&buf[i], &dip, sizeof(dip));
    i += sizeof(dip);
    memcpy(&buf[i], &sip, sizeof(sip));
    i += sizeof(sip);

    HAL_ABORT(i < sizeof(buf));

    HAL_TRACE_DEBUG("Header template = {}", hex_dump((uint8_t*)buf, sizeof(buf)));
    spec.set_header_template(buf, i);
    spec.set_header_len(i);
    return ret;
}

hal_ret_t
tcp_create_cb(qid_t qid, uint16_t src_lif, ether_header_t *eth, vlan_header_t* vlan, ipv4_header_t *ip, tcp_header_t *tcp, bool is_itor_dir)
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
    spec.set_source_lif(src_lif);
    ret = proxy_create_hdr_template(spec, eth, vlan, ip, tcp, is_itor_dir);;
    ret = tcpcb_create(spec, &rsp);
    if(ret != HAL_RET_OK || rsp.api_status() != types::API_STATUS_OK) {
        HAL_TRACE_ERR("Failed to create TCP cb for id: {}, ret: {}, rsp: ",
                        qid, ret, rsp.api_status());
        return ret;
    }

    HAL_TRACE_DEBUG("Successfully created TCPCB for id: {}", qid);
    return ret;
}


void
tcp_update_cb(void *tcpcb, uint32_t qid, uint16_t src_lif)
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

    spec->set_state(hal::pd::lkl_get_tcpcb_state(tcpcb));
    if (src_lif == 0) {
      src_lif = get_rsp.mutable_spec()->source_lif();
    }
    HAL_TRACE_DEBUG("Calling TCPCB Update with src_lif: {}", src_lif);
    spec->set_source_lif(src_lif);
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
tcp_trigger_ack_send(uint32_t qid, tcp_header_t *tcp)
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
    if (tcp != NULL) {
      
      spec->set_rcv_nxt(ntohl(tcp->seq)+1);
      HAL_TRACE_DEBUG("lkl_trigger_ack_send: rcv_nxt=0x{0:x}", ntohl(tcp->seq));
    } else {
      spec->set_rcv_nxt(get_rsp.mutable_spec()->rcv_nxt());
    }
    spec->set_snd_nxt(get_rsp.mutable_spec()->snd_nxt()+1);
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
    } else {
        flowupd.fwding.qid = pfi->qid2;
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

    // Check if exising session , then do nothing
    if(ctx.existing_session()) {
        HAL_TRACE_DEBUG("tcp-proxy: already enabled for flow: {}", ctx.key());
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

#if 1
    if (!ctx.protobuf_request()) {
        uint16_t shw_vlan_id, dhw_vlan_id;


        if (hal::pd::pd_l2seg_get_fromcpu_id(ctx.sl2seg(), &shw_vlan_id)) {
          HAL_TRACE_DEBUG("tcp-proxy: Got hw_vlan_id={} for sl2seg", shw_vlan_id);
        }

        if (hal::pd::pd_l2seg_get_fromcpu_id(ctx.dl2seg(), &dhw_vlan_id)) {
          HAL_TRACE_DEBUG("tcp-proxy: Got hw_vlan_id={} for dl2seg", dhw_vlan_id);
        }


        HAL_TRACE_DEBUG("LKL return {}",
                        hal::pd::lkl_handle_flow_miss_pkt(
                                                          hal::pd::lkl_alloc_skbuff(ctx.cpu_rxhdr(),
                                                                                    ctx.pkt(),
                                                                                    ctx.pkt_len(),
                                                                                    ctx.direction()),
                                                          ctx.direction(),
                                                          pfi->qid1, pfi->qid2,
                                                          ctx.cpu_rxhdr(),
                                                          dhw_vlan_id));
    }
#endif
    return fte::PIPELINE_CONTINUE;
}

#if 1
fte::pipeline_action_t
tcp_exec_tcp_lif(fte::ctx_t& ctx)
{
    HAL_TRACE_DEBUG("tcp_exec_tcp_lif: LKL return {}",
                     hal::pd::lkl_handle_flow_hit_pkt(
                                 hal::pd::lkl_alloc_skbuff(ctx.cpu_rxhdr(),
                                                           ctx.pkt(),
                                                           ctx.pkt_len(),
                                                           (hal::flow_direction_t)FLOW_DIR_FROM_UPLINK),//ctx.direction()),
                                 ctx.direction(),
                                 ctx.cpu_rxhdr()));

    return fte::PIPELINE_CONTINUE;
}
#else
fte::pipeline_action_t
tcp_exec_tcp_lif(fte::ctx_t& ctx)
{
    hal_ret_t ret = HAL_RET_OK;
    const fte::cpu_rxhdr_t* cpu_rxhdr = ctx.cpu_rxhdr();

    ret = tcp_trigger_ack_send(cpu_rxhdr->qid);
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, fte::PIPELINE_CONTINUE);

    return fte::PIPELINE_CONTINUE;
}
#endif

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
tcp_transmit_pkt(unsigned char* pkt, 
                 unsigned int len, 
                 bool is_connect_req, 
                 uint16_t dst_lif, 
                 uint16_t src_lif, 
        		 hal::flow_direction_t dir, 
                 uint16_t hw_vlan_id) 
{
    if (gl_ctx) {
        HAL_TRACE_DEBUG("tcp-proxy: txpkt dir={} src_lif={} hw_vlan_id={}", dir, src_lif, hw_vlan_id);
        if (true){//is_connect_req) {
            hal::pd::cpu_to_p4plus_header_t cpu_header;
            hal::pd::p4plus_to_p4_header_t  p4plus_header;

            p4plus_header.flags = 0;
            if (dir == FLOW_DIR_FROM_UPLINK) {
                if (is_connect_req) {
                    cpu_header.src_lif = hal::SERVICE_LIF_CPU;
                    cpu_header.hw_vlan_id = hw_vlan_id;
                    cpu_header.flags = CPU_TO_P4PLUS_FLAGS_UPD_VLAN;
                    p4plus_header.flags =  P4PLUS_TO_P4_FLAGS_LKP_INST;
                    
                } else {
                    cpu_header.src_lif = src_lif;
                    cpu_header.hw_vlan_id = 0;
                    cpu_header.flags = 0;
                }

            } else {
                if (is_connect_req) {
                    cpu_header.src_lif = src_lif;
                    cpu_header.hw_vlan_id = 0;
                    cpu_header.flags = 0;
                } else {
                    cpu_header.src_lif = hal::SERVICE_LIF_CPU;
                    cpu_header.hw_vlan_id = hw_vlan_id;
                    cpu_header.flags = CPU_TO_P4PLUS_FLAGS_UPD_VLAN;
                    p4plus_header.flags =  P4PLUS_TO_P4_FLAGS_LKP_INST;
                }
            }
            HAL_TRACE_DEBUG("tcp-proxy: txpkt cpu_header src_lif={} hw_vlan_id={} flags={}",
                            cpu_header.src_lif, cpu_header.hw_vlan_id, cpu_header.flags);

            gl_ctx->queue_txpkt(pkt, len, &cpu_header, &p4plus_header);
        } else {
            gl_ctx->queue_txpkt(pkt, len, NULL, NULL, dst_lif, CPU_ASQ_QTYPE, CPU_ASQ_QID, CPU_SCHED_RING_ASQ);
        }
    }
}


} // namespace hal
} // namespace net
