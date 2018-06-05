//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "nic/hal/src/internal/tcp_proxy_cb.hpp"
#include "nic/hal/src/internal/proxyrcb.hpp"
#include "nic/include/tcp_common.h"
#include "nic/include/interface_api.hpp"
#include "nic/hal/pd/pd_api.hpp"
//#include "nic/hal/pd/iris/hal_state_pd.hpp"
//#include "nic/hal/pd/iris/endpoint_pd.hpp"
#include "nic/hal/lkl/lklshim.hpp"
#include "nic/hal/lkl/lkl_api.hpp"
#include "nic/p4/iris/include/defines.h"
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

#define SET_COMMON_IP6_HDR(_buf, _i) \
    _buf[_i++] = 0x86; \
    _buf[_i++] = 0xdd; \
    _buf[_i++] = 0x60; \
    _buf[_i++] = 0x00; \
    _buf[_i++] = 0x00; \
    _buf[_i++] = 0x00; \
    _buf[_i++] = 0x00; \
    _buf[_i++] = 0x28; \
    _buf[_i++] = 0x06; \
    _buf[_i++] = 0x40;

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

static bool
is_local_ep(hal::if_t *intf) {
    return (intf != NULL && intf->if_type == intf::IF_TYPE_ENIC);
}

hal_ret_t
proxy_create_hdr_template(TcpCbSpec &spec,
                          ether_header_t *eth,
                          vlan_header_t* vlan,
                          ipv4_header_t *ip,
                          tcp_header_t *tcp,
                          bool is_itor_dir,
                          uint16_t src_vlan_id)
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
    } else {
        sport = tcp->dport;
        dport = tcp->sport;
        memcpy(&sip, &ip->daddr, sizeof(ip->daddr));
        memcpy(&dip, &ip->saddr, sizeof(ip->saddr));
        memcpy(&smac, eth->dmac, ETH_ADDR_LEN);
        memcpy(&dmac, eth->smac, ETH_ADDR_LEN);
    }

    vlan_id = htons(src_vlan_id);
    HAL_TRACE_DEBUG("tcp-proxy: src_vlan_id: {}, htons(vlan_id): {:#x}", src_vlan_id, vlan_id);
    HAL_TRACE_DEBUG("tcp-proxy: sport={}", hex_dump((uint8_t*)&sport, sizeof(sport)));
    HAL_TRACE_DEBUG("tcp-proxy: dport={}", hex_dump((uint8_t*)&dport, sizeof(dport)));
    HAL_TRACE_DEBUG("tcp-proxy: sip={}", hex_dump((uint8_t*)&sip, sizeof(sip)));
    HAL_TRACE_DEBUG("tcp-proxy: dip={}", hex_dump((uint8_t*)&dip, sizeof(dip)));
    HAL_TRACE_DEBUG("tcp-proxy: smac={}", hex_dump((uint8_t*)smac, sizeof(smac)));
    HAL_TRACE_DEBUG("tcp-proxy: dmac={}", hex_dump((uint8_t*)dmac, sizeof(dmac)));

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
proxy_create_v6_hdr_template(TcpCbSpec &spec,
                          ether_header_t *eth,
                          vlan_header_t* vlan,
                          ipv6_header_t *ip,
                          tcp_header_t *tcp,
                          bool is_itor_dir,
                          uint16_t src_vlan_id)
{
    hal_ret_t       ret = HAL_RET_OK;
    uint8_t         buf[64];
    uint8_t         i = 0;
    uint16_t        vlan_id;
    uint16_t        sport, dport;
    ipv6_addr_t     sip, dip;
    mac_addr_t      smac, dmac;

    HAL_TRACE_DEBUG("tcp-proxy: header template eth={}", hex_dump((uint8_t*)eth, 18));
    HAL_TRACE_DEBUG("tcp-proxy: header template ip6={}", hex_dump((uint8_t*)ip, sizeof(ipv6_header_t)));
    HAL_TRACE_DEBUG("tcp-proxy: header template tcp={}", hex_dump((uint8_t*)tcp, sizeof(tcp_header_t)));

    if (is_itor_dir) {
        sport = tcp->sport;
        dport = tcp->dport;
        memcpy(&sip.addr8, &ip->saddr, sizeof(ip->saddr));
        memcpy(&dip.addr8, &ip->daddr, sizeof(ip->daddr));
        memcpy(&smac, eth->smac, ETH_ADDR_LEN);
        memcpy(&dmac, eth->dmac, ETH_ADDR_LEN);
    } else {
        sport = tcp->dport;
        dport = tcp->sport;
        memcpy(&sip.addr8, &ip->daddr, sizeof(ip->daddr));
        memcpy(&dip.addr8, &ip->saddr, sizeof(ip->saddr));
        memcpy(&smac, eth->dmac, ETH_ADDR_LEN);
        memcpy(&dmac, eth->smac, ETH_ADDR_LEN);
    }

    vlan_id = htons(src_vlan_id);
    HAL_TRACE_DEBUG("tcp-proxy: src_vlan_id: {}, htons(vlan_id): {:#x}", src_vlan_id, vlan_id);
    HAL_TRACE_DEBUG("tcp-proxy: sport={}", hex_dump((uint8_t*)&sport, sizeof(sport)));
    HAL_TRACE_DEBUG("tcp-proxy: dport={}", hex_dump((uint8_t*)&dport, sizeof(dport)));
    HAL_TRACE_DEBUG("tcp-proxy: sip={}", hex_dump((uint8_t*)&sip, sizeof(sip)));
    HAL_TRACE_DEBUG("tcp-proxy: dip={}", hex_dump((uint8_t*)&dip, sizeof(dip)));
    HAL_TRACE_DEBUG("tcp-proxy: smac={}", hex_dump((uint8_t*)smac, sizeof(smac)));
    HAL_TRACE_DEBUG("tcp-proxy: dmac={}", hex_dump((uint8_t*)dmac, sizeof(dmac)));

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
    SET_COMMON_IP6_HDR(buf, i);
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
proxy_tcp_cb_init_def_params(TcpCbSpec& spec)
{
    spec.set_snd_wnd(8000);
    spec.set_snd_cwnd(8000);
    spec.set_rcv_mss(9216);
    // pred_flags
    //   header len = 8 (32 bytes with timestamp)
    //   flags = ACK
    //   window size = 8000
    spec.set_pred_flags(0x80101f40);
    return HAL_RET_OK;
}

hal_ret_t
tcp_create_cb(qid_t qid, qid_t other_qid, uint16_t src_lif, uint16_t src_vlan_id,
              ether_header_t *eth, vlan_header_t* vlan,
              ipv4_header_t *ip, tcp_header_t *tcp,
              bool is_itor_dir, types::AppRedirType l7_proxy_type)
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
    spec.set_other_qid(other_qid);
    spec.set_source_lif(src_lif);
    HAL_TRACE_DEBUG("tcp-proxy: source lif: {}", spec.source_lif());

    ret = proxy_tcp_cb_init_def_params(spec);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to initialize CB");
        return ret;
    }

    ret = proxy_create_hdr_template(spec, eth, vlan, ip, tcp, is_itor_dir, src_vlan_id);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to initialize header templates");
        return ret;
    }

    spec.set_l7_proxy_type(l7_proxy_type);
    ret = tcpcb_create(spec, &rsp);
    if(ret != HAL_RET_OK || rsp.api_status() != types::API_STATUS_OK) {
        HAL_TRACE_ERR("Failed to create TCP cb for id: {}, ret: {}, rsp: {}",
                        qid, ret, rsp.api_status());
        return ret;
    }

    HAL_TRACE_DEBUG("Successfully created TCPCB for id: {}", qid);
    return ret;
}

hal_ret_t
tcp_create_cb_v6(qid_t qid, qid_t other_qid, uint16_t src_lif, uint16_t src_vlan_id, ether_header_t *eth, vlan_header_t* vlan, ipv6_header_t *ip, tcp_header_t *tcp, bool is_itor_dir, types::AppRedirType l7_proxy_type)
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
    spec.set_other_qid(other_qid);
    spec.set_source_lif(src_lif);

    ret = proxy_tcp_cb_init_def_params(spec);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to initialize CB");
        return ret;
    }

    ret = proxy_create_v6_hdr_template(spec, eth, vlan, ip, tcp, is_itor_dir, src_vlan_id);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to initialize header templates");
        return ret;
    }

    spec.set_l7_proxy_type(l7_proxy_type);
    ret = tcpcb_create(spec, &rsp);
    if(ret != HAL_RET_OK || rsp.api_status() != types::API_STATUS_OK) {
        HAL_TRACE_ERR("Failed to create TCP cb for id: {}, ret: {}, rsp: {}",
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
    TcpCbGetResponseMsg resp_msg;
    TcpCbKeyHandle kh;
    uint8_t data[64];

    if (!gl_ctx) {
        HAL_TRACE_DEBUG("tcp_update_cb not called from current thread");
        return;
    }
    kh.set_tcpcb_id(qid);
    get_req->set_allocated_key_or_handle(&kh);
    tcpcb_get(*get_req, &resp_msg);
    get_rsp = resp_msg.response(0);
    HAL_TRACE_DEBUG("Get response: {}", get_rsp.api_status());
    spec->set_allocated_key_or_handle(&kh);
    HAL_TRACE_DEBUG("tcp-proxy: tcpcb={}\n", tcpcb);
    HAL_TRACE_DEBUG("tcp-proxy: tcpcb={}", hex_dump((uint8_t*)tcpcb, 2048));

    if (tcpcb) {
        HAL_TRACE_DEBUG("lkl rcv_nxt={}, snd_nxt={}, snd_una={}, rcv_tsval={}, ts_recent={} state={}",
                        hal::pd::lkl_get_tcpcb_rcv_nxt(tcpcb),
                        hal::pd::lkl_get_tcpcb_snd_nxt(tcpcb),
                        hal::pd::lkl_get_tcpcb_snd_una(tcpcb),
                        hal::pd::lkl_get_tcpcb_rcv_tsval(tcpcb),
                        hal::pd::lkl_get_tcpcb_ts_recent(tcpcb),
                        hal::pd::lkl_get_tcpcb_state(tcpcb));

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
    spec->set_l7_proxy_type(get_rsp.mutable_spec()->l7_proxy_type());
    spec->set_pred_flags(get_rsp.mutable_spec()->pred_flags());
    spec->set_other_qid(get_rsp.mutable_spec()->other_qid());

    memcpy(data,
           get_rsp.mutable_spec()->header_template().c_str(),
           std::max(get_rsp.mutable_spec()->header_template().size(), sizeof(data)));

    spec->set_header_template(data, sizeof(data));

    spec->set_state(hal::pd::lkl_get_tcpcb_state(tcpcb));
    if(src_lif == 0)
        src_lif = get_rsp.mutable_spec()->source_lif();
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
    TcpCbGetResponseMsg resp_msg;
    TcpCbKeyHandle kh;
    uint8_t data[64];

    kh.set_tcpcb_id(qid);
    get_req->set_allocated_key_or_handle(&kh);
    tcpcb_get(*get_req, &resp_msg);
    get_rsp = resp_msg.response(0);

    spec->set_allocated_key_or_handle(&kh);
    if (tcp != NULL) {

      spec->set_rcv_nxt(ntohl(tcp->seq)+1);
      HAL_TRACE_DEBUG("lkl_trigger_ack_send: rcv_nxt={:#x}", ntohl(tcp->seq));
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
    spec->set_l7_proxy_type(get_rsp.mutable_spec()->l7_proxy_type());
    spec->set_pred_flags(get_rsp.mutable_spec()->pred_flags());

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

    hal::pd::pd_cpupkt_program_send_ring_doorbell_args_t args;
    pd::pd_func_args_t pd_func_args = {0};
    args.dest_lif = SERVICE_LIF_TCP_PROXY;
    args.qtype = 0;
    args.qid = qid;
    args.ring_number = TCP_SCHED_RING_PENDING_RX2TX;
    pd_func_args.pd_cpupkt_program_send_ring_doorbell = &args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_PGM_SEND_RING_DBELL, &pd_func_args);
#if 0
    ret = hal::pd::cpupkt_program_send_ring_doorbell(SERVICE_LIF_TCP_PROXY,
                                                     0,
                                                     qid,
                                                     TCP_SCHED_RING_PENDING_RX2TX);
#endif
    HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);

    return ret;
}

void tcp_ring_doorbell(uint32_t qid)
{
    hal_ret_t ret = HAL_RET_OK;
    hal::pd::pd_cpupkt_program_send_ring_doorbell_args_t args;
    pd::pd_func_args_t pd_func_args = {0};
    args.dest_lif = SERVICE_LIF_TCP_PROXY;
    args.qtype = 0;
    args.qid = qid;
    args.ring_number = TCP_SCHED_RING_PENDING_RX2TX;
    pd_func_args.pd_cpupkt_program_send_ring_doorbell = &args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_PGM_SEND_RING_DBELL, &pd_func_args);
#if 0
    ret = hal::pd::cpupkt_program_send_ring_doorbell(SERVICE_LIF_TCP_PROXY,
                                                     0,
                                                     qid,
                                                     TCP_SCHED_RING_PENDING_RX2TX);
#endif
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

    HAL_TRACE_DEBUG("tc-proxy: flow forwarding role: {} qid1: {} qid2: {}",
                    ctx.role(), pfi->qid1, pfi->qid2);
    HAL_TRACE_DEBUG("tc-proxy: updating lport = {} for sport = {} dport = {}",
                    flowupd.fwding.lport, ctx.key().sport, ctx.key().dport);
    return ctx.update_flow(flowupd);
}

static bool
tcp_is_proxy_enabled_for_flow(const flow_key_t &flow_key)
{
    return is_proxy_enabled_for_flow(types::PROXY_TYPE_TCP, flow_key) ||
           is_proxy_enabled_for_flow(types::PROXY_TYPE_APP_REDIR_PROXY_TCP, flow_key) ||
           is_proxy_enabled_for_flow(types::PROXY_TYPE_APP_REDIR_PROXY_TCP_SPAN, flow_key);
    return true;
}

static proxy_flow_info_t*
tcp_proxy_get_flow_info(const flow_key_t& flow_key)
{
    proxy_flow_info_t*      pfi;

    pfi = proxy_get_flow_info(types::PROXY_TYPE_TCP, &flow_key);
    if (!pfi) {
        pfi = proxy_get_flow_info(types::PROXY_TYPE_APP_REDIR_PROXY_TCP, &flow_key);
    }
    if (!pfi) {
        pfi = proxy_get_flow_info(types::PROXY_TYPE_APP_REDIR_PROXY_TCP_SPAN, &flow_key);
    }

    return pfi;
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

    // no need to enable proxy if both src and dest are local
    if(is_local_ep(ctx.sif()) && is_local_ep(ctx.dif())) {
        HAL_TRACE_DEBUG("tcp-proxy: skipping proxy when both src/dest are local");
        return fte::PIPELINE_CONTINUE;
    }

    // Check if TCP proxy is enabled for the flow
    if(!tcp_is_proxy_enabled_for_flow(flow_key)) {
        HAL_TRACE_DEBUG("tcp-proxy: not enabled for flow: {}", ctx.key());
        return fte::PIPELINE_CONTINUE;
    }

    // Check if exising session , then do nothing
    if (ctx.existing_session()) {
        ctx.set_drop();
        HAL_TRACE_DEBUG("tcp-proxy: already enabled for flow: {}", ctx.key());
        ctx.set_feature_status(HAL_RET_EXISTING_PROXY_SESSION);
        return fte::PIPELINE_END;
    }

    // get the flow info for the tcp proxy service
    pfi = tcp_proxy_get_flow_info(flow_key);

    if(!pfi) {
        // Allocate PFI for the flow
        ret = proxy_flow_enable(types::PROXY_TYPE_TCP, flow_key, true, NULL, NULL);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_DEBUG("tcp-proxy: failed to enable service for the flow: {}", ctx.key());
            ctx.set_feature_status(ret);
            return fte::PIPELINE_END;
        }
        pfi = tcp_proxy_get_flow_info(flow_key);
        HAL_ASSERT_RETURN((NULL != pfi), fte::PIPELINE_CONTINUE);
    }

    // Update iflow
    ret = update_fwding_info(ctx, pfi);

    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END;
    }

    return fte::PIPELINE_CONTINUE;
}

static hal_ret_t
tcp_get_flow_encap_for_h2n_flow(const fte::ctx_t &ctx,
                                const proxy_flow_info_t& pfi,
                                hal::lklshim_flow_encap_t& flow_encap)
{
    uint16_t hw_vlan_id = 0;
    hal::pd::pd_l2seg_get_fromcpu_vlanid_args_t args = {0};
    pd::pd_func_args_t pd_func_args = {0};

    HAL_TRACE_DEBUG("tcp-proxy: Host -> Uplink connection");

    // Get CPU hw vlan-id
    args.l2seg = ctx.dl2seg();
    args.vid = &hw_vlan_id;

    pd_func_args.pd_l2seg_get_fromcpu_vlanid = &args;
    if (hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID,
                             &pd_func_args) != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get CPU VLAN-id");
    } else {
        HAL_TRACE_DEBUG("tcp-proxy: Got hw_vlan_id={} for dl2seg", hw_vlan_id);
    }

    flow_encap.i_src_lif = hal::SERVICE_LIF_CPU;
    flow_encap.i_src_vlan_id = hw_vlan_id;
    flow_encap.r_src_lif = ctx.cpu_rxhdr()->src_lif;
    flow_encap.r_src_vlan_id = ctx.sif()->encap_vlan;
    flow_encap.encrypt_qid = pfi.qid1;
    flow_encap.decrypt_qid = pfi.qid2;
    flow_encap.is_server_ctxt = false;
    return HAL_RET_OK;
}

static hal_ret_t
tcp_get_flow_encap_for_n2h_flow(const fte::ctx_t &ctx,
                                const proxy_flow_info_t& pfi,
                                hal::lklshim_flow_encap_t& flow_encap)
{
    hal_ret_t ret = HAL_RET_OK;
    uint16_t hw_vlan_id = 0, hw_lif_id = 0;
    hal::pd::pd_l2seg_get_fromcpu_vlanid_args_t args = {0};
    pd::pd_func_args_t pd_func_args = {0};
    hal::lif_t *lif = NULL;
    hal::pd::pd_lif_get_args_t hwlif_args = {0};

    HAL_TRACE_DEBUG("tcp-proxy: Uplink -> host connection");

    // Get CPU hw vlan-id
    args.l2seg = ctx.sl2seg();
    args.vid = &hw_vlan_id;
    pd_func_args.pd_l2seg_get_fromcpu_vlanid = &args;
    if (hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID,
                             &pd_func_args) != HAL_RET_OK) {
        HAL_TRACE_ERR("tcp-proxy: Failed to get CPU VLAN-id");
    } else {
        HAL_TRACE_DEBUG("tcp-proxy: Got hw_vlan_id={} for sl2seg", hw_vlan_id);
    }

    // find lif
    lif = find_lif_by_handle(ctx.dif()->lif_handle);
    if(!lif) {
        HAL_TRACE_ERR("Failed to find for the handle: {}", ctx.dif()->lif_handle);
        return HAL_RET_LIF_NOT_FOUND;
    }

    hwlif_args.lif = lif;
    pd_func_args.pd_lif_get = &hwlif_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_LIF_GET, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get hw_lif_id for lif {}", lif->lif_id);
        return ret;
    }
    hw_lif_id = hwlif_args.hw_lif_id;
    HAL_TRACE_DEBUG("tcp-proxy: Found lif id: {}, hw_lif-id: {}",
                    lif->lif_id, hw_lif_id);

    flow_encap.i_src_lif = hw_lif_id;
    flow_encap.i_src_vlan_id = ctx.dif()->encap_vlan;
    flow_encap.r_src_lif = hal::SERVICE_LIF_CPU;
    flow_encap.r_src_vlan_id = hw_vlan_id;
    flow_encap.encrypt_qid = pfi.qid2;
    flow_encap.decrypt_qid = pfi.qid1;
    flow_encap.is_server_ctxt = true;

    return HAL_RET_OK;
}

static hal_ret_t
tcp_get_flow_encap_for_n2n_flow(const fte::ctx_t &ctx,
                                const proxy_flow_info_t& pfi,
                                hal::lklshim_flow_encap_t& flow_encap)
{
    uint16_t shw_vlan_id = 0, dhw_vlan_id = 0;
    hal::pd::pd_l2seg_get_fromcpu_vlanid_args_t args = {0};
    pd::pd_func_args_t pd_func_args = {0};

    HAL_TRACE_DEBUG("tcp-proxy: Uplink -> Uplink connection");

    // Get CPU hw vlan-id
    args.l2seg = ctx.sl2seg();
    args.vid = &shw_vlan_id;
    pd_func_args.pd_l2seg_get_fromcpu_vlanid = &args;
    if (hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID,
                             &pd_func_args) != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get CPU VLAN-id");
    } else {
        HAL_TRACE_DEBUG("tcp-proxy: Got hw_vlan_id={} for sl2seg", shw_vlan_id);
    }

    args.l2seg = ctx.dl2seg();
    args.vid = &dhw_vlan_id;
    pd_func_args.pd_l2seg_get_fromcpu_vlanid = &args;
    if (hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID,
                             &pd_func_args) != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get CPU VLAN-id");
    } else {
        HAL_TRACE_DEBUG("tcp-proxy: Got hw_vlan_id={} for dl2seg", dhw_vlan_id);
    }

    flow_encap.i_src_lif = hal::SERVICE_LIF_CPU;
    flow_encap.i_src_vlan_id = dhw_vlan_id;
    flow_encap.r_src_lif = hal::SERVICE_LIF_CPU;
    flow_encap.r_src_vlan_id = shw_vlan_id;
    flow_encap.encrypt_qid = pfi.qid1;
    flow_encap.decrypt_qid = pfi.qid2;
    flow_encap.is_server_ctxt = false;

    return HAL_RET_OK;
}

static hal_ret_t
tcp_get_flow_encap(const fte::ctx_t& ctx,
                   const proxy_flow_info_t& pfi,
                   hal::lklshim_flow_encap_t& flow_encap)
{
    bool is_local_src = is_local_ep(ctx.sif());
    bool is_local_dest = is_local_ep(ctx.dif());

    if(is_local_src && is_local_dest) {
        // Host -> Host
        HAL_TRACE_DEBUG("tcp-proxy: both src and dest local. proxy shouldn't be enabled");
        return HAL_RET_INVALID_ARG;
    } else if (is_local_src && !is_local_dest) {
        return tcp_get_flow_encap_for_h2n_flow(ctx, pfi, flow_encap);
    } else if (!is_local_src && is_local_dest) {
        return tcp_get_flow_encap_for_n2h_flow(ctx, pfi, flow_encap);
    } else {
        return tcp_get_flow_encap_for_n2n_flow(ctx, pfi, flow_encap);
    }
    return HAL_RET_OK;
}

fte::pipeline_action_t
tcp_exec_trigger_connection(fte::ctx_t& ctx)
{
    hal_ret_t               ret = HAL_RET_OK;
    proxy_flow_info_t*      pfi = NULL;
    flow_key_t              flow_key = ctx.key();
    hal::lklshim_flow_encap_t flow_encap = {0};

    // Ignore direction for lookup. Always set it to 0
    if (ctx.role() == hal::FLOW_ROLE_RESPONDER) {
        HAL_TRACE_DEBUG("{}: responder side. ignoring.", __FUNCTION__);
        return fte::PIPELINE_CONTINUE;
    }

    if (ctx.protobuf_request()) {
        HAL_TRACE_DEBUG("protobuf request. ignoring.");
        return fte::PIPELINE_CONTINUE;
    }

    // Ignore direction for lookup. Always set it to 0
    flow_key.dir = 0;

    // get the flow info for the tcp proxy service
    pfi = tcp_proxy_get_flow_info(flow_key);
    if(!pfi) {
        HAL_TRACE_DEBUG("proxy flow info not found for the flow. ignoring");
        return fte::PIPELINE_CONTINUE;
    }

    ret = tcp_get_flow_encap(ctx, *pfi, flow_encap);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get flow encap params");
        return fte::PIPELINE_CONTINUE;
    }

    HAL_TRACE_DEBUG("LKL return {}",
                    hal::pd::lkl_handle_flow_miss_pkt(hal::pd::lkl_alloc_skbuff(ctx.cpu_rxhdr(),
                                                                                ctx.pkt(),
                                                                                ctx.pkt_len(),
                                                                                ctx.direction()),
                                                      ctx.direction(),
                                                      pfi->qid1, pfi->qid2,
                                                      ctx.cpu_rxhdr(),
                                                      pfi,
                                                      &flow_encap));
    return fte::PIPELINE_CONTINUE;
}

fte::pipeline_action_t
tcp_exec_tcp_lif(fte::ctx_t& ctx)
{
    gl_ctx = &ctx;
    const hal::pd::p4_to_p4plus_cpu_pkt_t* rxhdr = ctx.cpu_rxhdr();
    hal::flow_direction_t dir = hal::lklshim_get_flow_hit_pkt_direction(rxhdr->qid);

    if (ctx.pkt_len() == 0) {
        HAL_TRACE_DEBUG("tcp_exec_tcp_lif: LKL return {}",
                        hal::pd::lkl_handle_flow_hit_hdr(
                                                         hal::pd::lkl_alloc_skbuff(rxhdr,
                                                                                   ctx.pkt(),
                                                                                   ctx.pkt_len(),
                                                                                   dir),
                                                         dir,
                                                         rxhdr));
    } else {
        HAL_TRACE_DEBUG("tcp_exec_tcp_lif: LKL return {}",
                        hal::pd::lkl_handle_flow_hit_pkt(
                                                         hal::pd::lkl_alloc_skbuff(rxhdr,
                                                                                   ctx.pkt(),
                                                                                   ctx.pkt_len(),
                                                                                   dir),
                                                         dir,
                                                         rxhdr));
    }

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
tcp_transmit_pkt(unsigned char* pkt,
                 unsigned int len,
                 uint16_t src_lif,
                 uint16_t src_vlan_id)
{
    if (gl_ctx) {
        HAL_TRACE_DEBUG("tcp-proxy: txpkt src_lif={} src_vlan_id={}", src_lif, src_vlan_id);

        hal::pd::cpu_to_p4plus_header_t cpu_header = {0};
        hal::pd::p4plus_to_p4_header_t  p4plus_header = {0};
        cpu_header.src_lif = src_lif;
        cpu_header.hw_vlan_id = src_vlan_id;
        cpu_header.flags = CPU_TO_P4PLUS_FLAGS_UPD_VLAN;
        p4plus_header.flags =  P4PLUS_TO_P4_FLAGS_LKP_INST;

        gl_ctx->queue_txpkt(pkt, len, &cpu_header, &p4plus_header);
    } else {
        HAL_TRACE_DEBUG("tcp-proxy: gl_ctx is NULL");
    }
}


} // namespace hal
} // namespace net
