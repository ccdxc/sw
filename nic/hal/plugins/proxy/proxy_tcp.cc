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
#include "nic/hal/src/internal/internal.hpp"
#include "nic/hal/src/internal/proxyrcb.hpp"
#include "nic/include/tcp_common.h"
#include "nic/hal/plugins/cfg/nw/interface_api.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/hal/pd/tcp_msg_api.hpp"
//#include "nic/hal/pd/iris/hal_state_pd.hpp"
//#include "nic/hal/pd/iris/endpoint_pd.hpp"
#include "nic/hal/lkl/lklshim.hpp"
#include "nic/hal/lkl/lkl_api.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/hal/pd/cpupkt_api.hpp"
#include "nic/asm/cpu-p4plus/include/cpu-defines.h"

using proxy::ProxyGlobalCfgRequest;
using proxy::ProxyGlobalCfgRequestMsg;
using proxy::ProxyGlobalCfgResponseMsg;


namespace hal {
namespace proxy {

// Context for processing msgs from tcp rings
typedef struct tcp_msg_proc_ctxt_s {
    hal::pd::tcp_msg_batch_t batch;
    void            *arm_ctx;
    uint16_t        txpkt_cnt;
    uint8_t         qid;
//    txpkt_info_t    txpkts[MAX_QUEUED_PKTS];
} tcp_msg_proc_ctxt_t;

thread_local fte::ctx_t *gl_ctx;
thread_local tcp_msg_proc_ctxt_t tcp_ctx;
thread_local tcp_msg_proc_ctxt_t *gl_tcp_ctx;

#define SET_COMMON_IP_HDR(_buf, _i) \
    _buf[_i++] = 0x08; \
    _buf[_i++] = 0x00; \
    _buf[_i++] = 0x45; \
    _buf[_i++] = 0x09; \
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
    _buf[_i++] = 0x01; \
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

    HAL_TRACE_VERBOSE("tcp-proxy: header template eth={}", hex_dump((uint8_t*)eth, 18));
    HAL_TRACE_VERBOSE("tcp-proxy: header template ip={}", hex_dump((uint8_t*)ip, sizeof(ipv4_header_t)));
    HAL_TRACE_VERBOSE("tcp-proxy: header template tcp={}", hex_dump((uint8_t*)tcp, sizeof(tcp_header_t)));

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
    HAL_TRACE_VERBOSE("tcp-proxy: src_vlan_id: {}, htons(vlan_id): {:#x}", src_vlan_id, vlan_id);
    HAL_TRACE_VERBOSE("tcp-proxy: sport={}", hex_dump((uint8_t*)&sport, sizeof(sport)));
    HAL_TRACE_VERBOSE("tcp-proxy: dport={}", hex_dump((uint8_t*)&dport, sizeof(dport)));
    HAL_TRACE_VERBOSE("tcp-proxy: sip={}", hex_dump((uint8_t*)&sip, sizeof(sip)));
    HAL_TRACE_VERBOSE("tcp-proxy: dip={}", hex_dump((uint8_t*)&dip, sizeof(dip)));
    HAL_TRACE_VERBOSE("tcp-proxy: smac={}", hex_dump((uint8_t*)smac, sizeof(smac)));
    HAL_TRACE_VERBOSE("tcp-proxy: dmac={}", hex_dump((uint8_t*)dmac, sizeof(dmac)));

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

    HAL_TRACE_VERBOSE("Header template = {}", hex_dump((uint8_t*)buf, sizeof(buf)));
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

    HAL_TRACE_VERBOSE("tcp-proxy: header template eth={}", hex_dump((uint8_t*)eth, 18));
    HAL_TRACE_VERBOSE("tcp-proxy: header template ip6={}", hex_dump((uint8_t*)ip, sizeof(ipv6_header_t)));
    HAL_TRACE_VERBOSE("tcp-proxy: header template tcp={}", hex_dump((uint8_t*)tcp, sizeof(tcp_header_t)));

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
    HAL_TRACE_VERBOSE("tcp-proxy: src_vlan_id: {}, htons(vlan_id): {:#x}", src_vlan_id, vlan_id);
    HAL_TRACE_VERBOSE("tcp-proxy: sport={}", hex_dump((uint8_t*)&sport, sizeof(sport)));
    HAL_TRACE_VERBOSE("tcp-proxy: dport={}", hex_dump((uint8_t*)&dport, sizeof(dport)));
    HAL_TRACE_VERBOSE("tcp-proxy: sip={}", hex_dump((uint8_t*)&sip, sizeof(sip)));
    HAL_TRACE_VERBOSE("tcp-proxy: dip={}", hex_dump((uint8_t*)&dip, sizeof(dip)));
    HAL_TRACE_VERBOSE("tcp-proxy: smac={}", hex_dump((uint8_t*)smac, sizeof(smac)));
    HAL_TRACE_VERBOSE("tcp-proxy: dmac={}", hex_dump((uint8_t*)dmac, sizeof(dmac)));

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

    HAL_TRACE_VERBOSE("Header template = {}", hex_dump((uint8_t*)buf, sizeof(buf)));
    spec.set_header_template(buf, i);
    spec.set_header_len(i);
    return ret;
}

hal_ret_t
proxy_tcp_cb_init_def_params(TcpCbSpec& spec)
{
    int window = 0;
    FILE *fp = fopen("/tcp.window", "r");
    if (fp != NULL) {
        fscanf(fp, "%d", &window);
        fclose(fp);
    }

    if (window <=0 || window > 65535) {
        window = 8000;
    }

    HAL_TRACE_VERBOSE("tcp rcv window = {}", window);

    // TODO : we need to get this from LKL/OFP
    spec.set_rcv_wnd(window);

    // Work around to set receive window scale. Remove this
    // once the fix to pass rcv_wscale from lkl is merged. 
    spec.set_rcv_wscale(4);
    spec.set_snd_wnd(8000);
    spec.set_rcv_mss(9216);
    spec.set_smss(9216);
    spec.set_snd_cwnd(8000);
    spec.set_snd_ssthresh(8000 * 10);
    spec.set_ato(TCP_ATO_USEC);
    spec.set_delay_ack(true);
    spec.set_ooo_queue(true);
    spec.set_sack_perm(true);
    /* timestamps disabled for DOL */
    spec.set_timestamps(false);
    spec.set_abc_l_var(2);
    // pred_flags
    //   header len = 8 (32 bytes with timestamp)
    //   flags = ACK
    //   window size = 8000
    spec.set_pred_flags(0x80101f40);
    return HAL_RET_OK;
}

hal_ret_t
tcp_create_cb(fte::ctx_t &ctx, qid_t qid, qid_t other_qid, uint16_t src_lif, uint16_t src_vlan_id,
              ether_header_t *eth, vlan_header_t* vlan,
              ipv4_header_t *ip, tcp_header_t *tcp,
              bool is_itor_dir, types::AppRedirType l7_proxy_type,
              types::ProxyType proxy_type)
{
    hal_ret_t       ret = HAL_RET_OK;
    TcpCbSpec       spec;
    TcpCbResponse   rsp;
    if(ctx.dif() == NULL || ctx.dif()->if_type == intf::IF_TYPE_TUNNEL) {
        HAL_TRACE_VERBOSE("Skipping TCPCB creation for TUNNEL interface");
        return HAL_RET_OK;
    }
    gl_ctx = &ctx;

    HAL_TRACE_VERBOSE("Create TCPCB for qid: {}", qid);
    spec.mutable_key_or_handle()->set_tcpcb_id(qid);
    spec.set_other_qid(other_qid);
    spec.set_source_lif(src_lif);
    spec.set_cpu_id(fte::fte_id());
    HAL_TRACE_VERBOSE("tcp-proxy: source lif: {}, cpu_id: {}", spec.source_lif(), spec.cpu_id());

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
    spec.set_proxy_type(proxy_type);
    ret = tcpcb_create(spec, &rsp);
    if(ret != HAL_RET_OK || rsp.api_status() != types::API_STATUS_OK) {
        HAL_TRACE_ERR("Failed to create TCP cb for id: {}, ret: {}, rsp: {}",
                        qid, ret, rsp.api_status());
        return ret;
    }

    HAL_TRACE_VERBOSE("Successfully created TCPCB for id: {}", qid);
    return ret;
}

hal_ret_t
tcp_create_cb_v6(fte::ctx_t &ctx, qid_t qid, qid_t other_qid, uint16_t src_lif, uint16_t src_vlan_id, ether_header_t *eth,
        vlan_header_t* vlan, ipv6_header_t *ip, tcp_header_t *tcp, bool is_itor_dir, types::AppRedirType l7_proxy_type,
        types::ProxyType proxy_type)
{
    hal_ret_t       ret = HAL_RET_OK;
    TcpCbSpec       spec;
    TcpCbResponse   rsp;
    if(ctx.dif() == NULL || ctx.dif()->if_type == intf::IF_TYPE_TUNNEL) {
        HAL_TRACE_VERBOSE("Skipping TCPCB creation for TUNNEL interface");
        return HAL_RET_OK;
    }
    gl_ctx = &ctx;

    HAL_TRACE_VERBOSE("Create TCPCB for qid: {}", qid);
    spec.mutable_key_or_handle()->set_tcpcb_id(qid);
    spec.set_other_qid(other_qid);
    spec.set_source_lif(src_lif);
    spec.set_cpu_id(fte::fte_id());

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
    spec.set_proxy_type(proxy_type);
    ret = tcpcb_create(spec, &rsp);
    if(ret != HAL_RET_OK || rsp.api_status() != types::API_STATUS_OK) {
        HAL_TRACE_ERR("Failed to create TCP cb for id: {}, ret: {}, rsp: {}",
                        qid, ret, rsp.api_status());
        return ret;
    }

    HAL_TRACE_VERBOSE("Successfully created TCPCB for id: {}", qid);
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
        HAL_TRACE_VERBOSE("tcp_update_cb not called from current thread");
        return;
    }
    kh.set_tcpcb_id(qid);
    get_req->set_allocated_key_or_handle(&kh);
    tcpcb_get(*get_req, &resp_msg);
    get_rsp = resp_msg.response(0);
    HAL_TRACE_VERBOSE("Get response: {}", get_rsp.api_status());
    spec->set_allocated_key_or_handle(&kh);
    HAL_TRACE_VERBOSE("tcp-proxy: tcpcb={}\n", tcpcb);
    HAL_TRACE_VERBOSE("tcp-proxy: tcpcb={}", hex_dump((uint8_t*)tcpcb, 2048));

    if (tcpcb) {
        uint32_t smss;
        uint32_t init_cwnd;
        uint32_t init_cwnd_segments;
        HAL_TRACE_VERBOSE("lkl rcv_nxt={}, snd_nxt={}, snd_una={}, rcv_tsval={}, ts_recent={} state={}",
                        hal::pd::lkl_get_tcpcb_rcv_nxt(tcpcb),
                        hal::pd::lkl_get_tcpcb_snd_nxt(tcpcb),
                        hal::pd::lkl_get_tcpcb_snd_una(tcpcb),
                        hal::pd::lkl_get_tcpcb_rcv_tsval(tcpcb),
                        hal::pd::lkl_get_tcpcb_ts_recent(tcpcb),
                        hal::pd::lkl_get_tcpcb_state(tcpcb));
        HAL_TRACE_VERBOSE("lkl snd_wscale={} rcv_wscale={}",
                        hal::pd::lkl_get_tcpcb_snd_wscale(tcpcb),
                        hal::pd::lkl_get_tcpcb_rcv_wscale(tcpcb));

        spec->set_rcv_nxt(hal::pd::lkl_get_tcpcb_rcv_nxt(tcpcb));
        spec->set_rcv_wup(hal::pd::lkl_get_tcpcb_rcv_nxt(tcpcb));
        spec->set_snd_nxt(hal::pd::lkl_get_tcpcb_snd_nxt(tcpcb));
        spec->set_snd_una(hal::pd::lkl_get_tcpcb_snd_una(tcpcb));
        spec->set_snd_wscale(hal::pd::lkl_get_tcpcb_snd_wscale(tcpcb));
        spec->set_snd_ssthresh(hal::pd::lkl_get_tcpcb_snd_ssthresh(tcpcb));
        spec->set_smss(hal::pd::lkl_get_tcpcb_smss(tcpcb));
        // Use rcv_mss same as smss for now
        spec->set_rcv_mss(hal::pd::lkl_get_tcpcb_smss(tcpcb));
        smss = hal::pd::lkl_get_tcpcb_smss(tcpcb);
        HAL_TRACE_VERBOSE("smss={}", smss);
        init_cwnd_segments = hal::pd::lkl_get_tcpcb_snd_cwnd(tcpcb);
        // RFC 6928
        // initial window = min (10*MSS, max (2*MSS, 14600))
        init_cwnd = std::min(init_cwnd_segments * smss,
                            std::max(2 * smss, init_cwnd_segments * 1460));

        // Use large initial window for now to do perf testing
        init_cwnd = 10000000;

        spec->set_snd_cwnd(init_cwnd);

        spec->set_initial_window(init_cwnd);
        HAL_TRACE_VERBOSE("lkl snd_cwnd={} snd_ssthresh={}",
                        init_cwnd,
                        hal::pd::lkl_get_tcpcb_snd_ssthresh(tcpcb));
    }
    spec->set_serq_base(get_rsp.mutable_spec()->serq_base());
    spec->set_debug_dol(get_rsp.mutable_spec()->debug_dol());
    spec->set_sesq_base(get_rsp.mutable_spec()->sesq_base());
    spec->set_sesq_pi(get_rsp.mutable_spec()->sesq_pi());
    spec->set_sesq_ci(get_rsp.mutable_spec()->sesq_ci());
    spec->set_snd_wnd(get_rsp.mutable_spec()->snd_wnd());
    spec->set_rcv_wnd(get_rsp.mutable_spec()->rcv_wnd());
    //spec->set_rcv_mss(get_rsp.mutable_spec()->rcv_mss());
    spec->set_rcv_wscale(get_rsp.mutable_spec()->rcv_wscale());
    spec->set_source_port(get_rsp.mutable_spec()->source_port());
    spec->set_dest_port(get_rsp.mutable_spec()->dest_port());
    spec->set_header_len(get_rsp.mutable_spec()->header_len());
    spec->set_l7_proxy_type(get_rsp.mutable_spec()->l7_proxy_type());
    spec->set_pred_flags(get_rsp.mutable_spec()->pred_flags());
    spec->set_other_qid(get_rsp.mutable_spec()->other_qid());
    spec->set_cpu_id(get_rsp.mutable_spec()->cpu_id());
    spec->set_delay_ack(get_rsp.mutable_spec()->delay_ack());
    spec->set_ato(get_rsp.mutable_spec()->ato());
    spec->set_ooo_queue(get_rsp.mutable_spec()->ooo_queue());
    spec->set_sack_perm(get_rsp.mutable_spec()->sack_perm());
    spec->set_timestamps(get_rsp.mutable_spec()->timestamps());
    spec->set_ts_recent(get_rsp.mutable_spec()->ts_recent());
    spec->set_rcv_tsval(get_rsp.mutable_spec()->ts_recent());
    spec->set_rtt_seq_tsoffset(get_rsp.mutable_spec()->rtt_seq_tsoffset());
    spec->set_rtt_time(get_rsp.mutable_spec()->rtt_time());
    spec->set_ts_learned(get_rsp.mutable_spec()->ts_learned());
    spec->set_ts_time(get_rsp.mutable_spec()->rtt_time());
    spec->set_ts_offset(get_rsp.mutable_spec()->rtt_seq_tsoffset());

    memcpy(data,
           get_rsp.mutable_spec()->header_template().c_str(),
           std::max(get_rsp.mutable_spec()->header_template().size(), sizeof(data)));

    spec->set_header_template(data, sizeof(data));

    spec->set_state(hal::pd::lkl_get_tcpcb_state(tcpcb));
    if(src_lif == 0)
        src_lif = get_rsp.mutable_spec()->source_lif();
    HAL_TRACE_VERBOSE("Calling TCPCB Update with src_lif: {}", src_lif);
    spec->set_source_lif(src_lif);
    spec->set_asesq_base(get_rsp.mutable_spec()->asesq_base());
    spec->set_asesq_pi(get_rsp.mutable_spec()->asesq_pi());
    spec->set_asesq_ci(get_rsp.mutable_spec()->asesq_ci());
    spec->set_debug_dol_tx(get_rsp.mutable_spec()->debug_dol_tx());

    HAL_TRACE_VERBOSE("Calling TCPCB Update with id: {}", qid);
    ret = tcpcb_update(*spec, &rsp);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_VERBOSE("tcbcb update failed {}", ret);
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
      HAL_TRACE_VERBOSE("lkl_trigger_ack_send: rcv_nxt={:#x}", ntohl(tcp->seq));
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
    spec->set_rcv_wnd(get_rsp.mutable_spec()->rcv_wnd());
    spec->set_snd_cwnd(get_rsp.mutable_spec()->snd_cwnd());
    spec->set_rcv_mss(get_rsp.mutable_spec()->rcv_mss());
    spec->set_smss(get_rsp.mutable_spec()->smss());
    spec->set_source_port(get_rsp.mutable_spec()->source_port());
    spec->set_dest_port(get_rsp.mutable_spec()->dest_port());
    spec->set_header_len(get_rsp.mutable_spec()->header_len());
    spec->set_l7_proxy_type(get_rsp.mutable_spec()->l7_proxy_type());
    spec->set_pred_flags(get_rsp.mutable_spec()->pred_flags());
    spec->set_cpu_id(get_rsp.mutable_spec()->cpu_id());

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

    ret = tcpcb_update(*spec, &rsp);

    hal::pd::pd_cpupkt_program_send_ring_doorbell_args_t args;
    pd::pd_func_args_t pd_func_args = {0};
    args.dest_lif = SERVICE_LIF_TCP_PROXY;
    args.qtype = 0;
    args.qid = qid;
    args.ring_number = TCP_SCHED_RING_SEND_ACK;
    args.flags = DB_IDX_UPD_PIDX_INC | DB_SCHED_UPD_SET;
    pd_func_args.pd_cpupkt_program_send_ring_doorbell = &args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_PGM_SEND_RING_DBELL, &pd_func_args);
#if 0
    ret = hal::pd::cpupkt_program_send_ring_doorbell(SERVICE_LIF_TCP_PROXY,
                                                     0,
                                                     qid,
                                                     TCP_SCHED_RING_SEND_ACK);
#endif
    SDK_ASSERT_RETURN(ret == HAL_RET_OK, ret);

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
    args.ring_number = TCP_SCHED_RING_SEND_ACK;
    args.flags = DB_IDX_UPD_PIDX_INC | DB_SCHED_UPD_SET;
    pd_func_args.pd_cpupkt_program_send_ring_doorbell = &args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_PGM_SEND_RING_DBELL, &pd_func_args);
#if 0
    ret = hal::pd::cpupkt_program_send_ring_doorbell(SERVICE_LIF_TCP_PROXY,
                                                     0,
                                                     qid,
                                                     TCP_SCHED_RING_SEND_ACK);
#endif
    if (ret != HAL_RET_OK) {
        HAL_TRACE_VERBOSE("ring doorbell failed {}", ret);
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

    HAL_TRACE_VERBOSE("tc-proxy: flow forwarding role: {} qid1: {} qid2: {}",
                    ctx.role(), pfi->qid1, pfi->qid2);
    HAL_TRACE_VERBOSE("tc-proxy: updating lport = {} for sport = {} dport = {}",
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

    pfi = proxy_get_flow_info(types::PROXY_TYPE_TCP, flow_key);
    if (!pfi) {
        pfi = proxy_get_flow_info(types::PROXY_TYPE_APP_REDIR_PROXY_TCP, flow_key);
    }
    if (!pfi) {
        pfi = proxy_get_flow_info(types::PROXY_TYPE_APP_REDIR_PROXY_TCP_SPAN, flow_key);
    }

    return pfi;
}

static const hal::ipv4_rule_t *
tcp_proxy_lookup_rules(vrf_id_t vrf_id, fte::ctx_t &ctx)
{
    const hal::ipv4_rule_t *rule = NULL;
    const acl::acl_ctx_t *acl_ctx = NULL;
    hal::ipv4_tuple acl_key = {};
    char ctx_name[ACL_NAMESIZE];

    if (ctx.get_key().proto != types::IPPROTO_TCP) {
        return NULL;
    }

    HAL_TRACE_VERBOSE("vrf_id {} key : {}", vrf_id, ctx.key());

    acl_key.proto = ctx.get_key().proto;
    acl_key.ip_src = ctx.get_key().sip.v4_addr;
    acl_key.ip_dst = ctx.get_key().dip.v4_addr;
    acl_key.port_src = ctx.get_key().sport;
    acl_key.port_dst = ctx.get_key().dport;

    tcp_proxy_acl_ctx_name(ctx_name, vrf_id);
    acl_ctx = acl::acl_get(ctx_name);
    if (acl_ctx == NULL) {
        HAL_TRACE_VERBOSE("Could not find acl for {}", ctx_name);
        return NULL;
    }

    if (acl_classify(acl_ctx, (const uint8_t *)&acl_key,
                (const acl_rule_t **)&rule, 0x01) != HAL_RET_OK) {
        return NULL;
    }

    return rule;
}

static bool
tcp_is_proxy_policy_enabled_for_flow(fte::ctx_t &ctx)
{
    const hal::ipv4_rule_t *rule = NULL;
    tcp_proxy_cfg_rule_t *rule_cfg;
    acl::ref_t *rc;

    rule = tcp_proxy_lookup_rules(ctx.key().dvrf_id, ctx);
    if (!rule) {
        //HAL_TRACE_VERBOSE("TCP Proxy rule lookup failed for vrf {}", ctx.key().dvrf_id);
        return false;
    }

    rc = get_rule_data((acl_rule_t *) rule);
    rule_cfg = RULE_MATCH_USER_DATA(rc, tcp_proxy_cfg_rule_t, ref_count);
    if (rule_cfg->action.tcp_proxy_action ==
            tcp_proxy::TcpProxyActionType::TCP_PROXY_ACTION_TYPE_ENABLE) {
        return true;
    }

    return false;
}

hal_ret_t
tcp_proxy_type_action(fte::ctx_t &ctx, proxy_flow_info_t *pfi)
{
    const hal::ipv4_rule_t *rule = NULL;
    tcp_proxy_cfg_rule_t *rule_cfg;
    acl::ref_t *rc;

    rule = tcp_proxy_lookup_rules(ctx.key().dvrf_id, ctx);
    if (!rule) {
        //HAL_TRACE_VERBOSE("TCP Proxy rule lookup failed for vrf {}", ctx.key().dvrf_id);
        return HAL_RET_ERR;
    }

    rc = get_rule_data((acl_rule_t *) rule);
    rule_cfg = RULE_MATCH_USER_DATA(rc, tcp_proxy_cfg_rule_t, ref_count);

    pfi->proxy_type = rule_cfg->action.proxy_type;

    if (rule_cfg->action.proxy_type  == types::PROXY_TYPE_TCP) {
        /* FIXME: Unfortunately bypass TLS is currently a global knob and not
         * per flow
         */
        ProxyGlobalCfgRequest       req;
        ProxyGlobalCfgResponseMsg   rsp_msg;

        req.set_proxy_type(types::PROXY_TYPE_TLS);
        req.set_bypass_mode(1);

        if (HAL_RET_OK != proxy_globalcfg_set(req, &rsp_msg)) {
            HAL_TRACE_ERR("Failed to enable TLSL bypass for PROXY_TYPE_TCP: err {}",
                    rsp_msg.api_status(0));
            return HAL_RET_ERR;
        }
    }
    else if (rule_cfg->action.proxy_type  == types::PROXY_TYPE_TLS) {
        return tls_proxy_cfg_rule_action(&rule_cfg->action.u.tls_cfg, pfi);
    }

    return HAL_RET_OK;
}


fte::pipeline_action_t
tcp_exec_cpu_lif(fte::ctx_t& ctx)
{
    hal_ret_t               ret = HAL_RET_OK;
    proxy_flow_info_t*      pfi = NULL;
    flow_key_t              flow_key = ctx.key();

    // Ignore direction. Always set it to 0
    flow_key.dir = 0;

    // no need to enable proxy if both src and dest are local
    if(is_local_ep(ctx.sif()) && is_local_ep(ctx.dif())) {
        HAL_TRACE_VERBOSE("tcp-proxy: skipping proxy when both src/dest are local");
        return fte::PIPELINE_CONTINUE;
    }

    // Check if TCP proxy is enabled for the flow
    if(!tcp_is_proxy_enabled_for_flow(flow_key) &&
            !tcp_is_proxy_policy_enabled_for_flow(ctx)) {
        HAL_TRACE_VERBOSE("tcp-proxy: not enabled");
        return fte::PIPELINE_CONTINUE;
    }

    // Check if exising session , then do nothing
    if (ctx.existing_session()) {
        ctx.set_drop();
        HAL_TRACE_VERBOSE("tcp-proxy: already enabled for flow: {}", ctx.key());
        ctx.set_feature_status(HAL_RET_EXISTING_PROXY_SESSION);
        return fte::PIPELINE_END;
    }

    // get the flow info for the tcp proxy service
    pfi = tcp_proxy_get_flow_info(flow_key);

    if(!pfi) {
        // Allocate PFI for the flow
        ret = proxy_flow_enable(types::PROXY_TYPE_TCP, flow_key, true, NULL, NULL);
        if(ret != HAL_RET_OK) {
            HAL_TRACE_VERBOSE("tcp-proxy: failed to enable service for the flow: {}", ctx.key());
            ctx.set_feature_status(ret);
            return fte::PIPELINE_END;
        }
        pfi = tcp_proxy_get_flow_info(flow_key);
        SDK_ASSERT_RETURN((NULL != pfi), fte::PIPELINE_CONTINUE);
        // Proxy type specific configuration
        ret = tcp_proxy_type_action(ctx, pfi);
        if (ret != HAL_RET_OK) {
            ctx.set_feature_status(ret);
            return fte::PIPELINE_END;
        }
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

    HAL_TRACE_VERBOSE("tcp-proxy: Host -> Uplink connection");

    // Get CPU hw vlan-id
    args.l2seg = ctx.dl2seg();
    args.vid = &hw_vlan_id;

    pd_func_args.pd_l2seg_get_fromcpu_vlanid = &args;
    if (hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID,
                             &pd_func_args) != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get CPU VLAN-id");
    } else {
        HAL_TRACE_VERBOSE("tcp-proxy: Got hw_vlan_id={} for dl2seg", hw_vlan_id);
    }

    flow_encap.i_src_lif = HAL_LIF_CPU;
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

    HAL_TRACE_VERBOSE("tcp-proxy: Uplink -> host connection");

    // Get CPU hw vlan-id
    args.l2seg = ctx.sl2seg();
    args.vid = &hw_vlan_id;
    pd_func_args.pd_l2seg_get_fromcpu_vlanid = &args;
    if (hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID,
                             &pd_func_args) != HAL_RET_OK) {
        HAL_TRACE_ERR("tcp-proxy: Failed to get CPU VLAN-id");
    } else {
        HAL_TRACE_VERBOSE("tcp-proxy: Got hw_vlan_id={} for sl2seg", hw_vlan_id);
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
    HAL_TRACE_VERBOSE("tcp-proxy: Found lif id: {}, hw_lif-id: {}",
                    lif->lif_id, hw_lif_id);

    flow_encap.i_src_lif = hw_lif_id;
    flow_encap.i_src_vlan_id = ctx.dif()->encap_vlan;
    flow_encap.r_src_lif = HAL_LIF_CPU;
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

    HAL_TRACE_VERBOSE("tcp-proxy: Uplink -> Uplink connection");

    // Get CPU hw vlan-id
    args.l2seg = ctx.sl2seg();
    args.vid = &shw_vlan_id;
    pd_func_args.pd_l2seg_get_fromcpu_vlanid = &args;
    if (hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID,
                             &pd_func_args) != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get CPU VLAN-id");
    } else {
        HAL_TRACE_VERBOSE("tcp-proxy: Got hw_vlan_id={} for sl2seg", shw_vlan_id);
    }

    args.l2seg = ctx.dl2seg();
    args.vid = &dhw_vlan_id;
    pd_func_args.pd_l2seg_get_fromcpu_vlanid = &args;
    if (hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FRCPU_VLANID,
                             &pd_func_args) != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get CPU VLAN-id");
    } else {
        HAL_TRACE_VERBOSE("tcp-proxy: Got hw_vlan_id={} for dl2seg", dhw_vlan_id);
    }

    flow_encap.i_src_lif = HAL_LIF_CPU;
    flow_encap.i_src_vlan_id = dhw_vlan_id;
    flow_encap.r_src_lif = HAL_LIF_CPU;
    flow_encap.r_src_vlan_id = shw_vlan_id;
    flow_encap.encrypt_qid = pfi.qid1;
    flow_encap.decrypt_qid = pfi.qid2;
    flow_encap.is_server_ctxt = false;

    if (pfi.proxy_type == types::PROXY_TYPE_TLS) {
        if (pfi.u.tlsproxy.tls_proxy_side == tcp_proxy::TLS_PROXY_SIDE_SERVER) {
            flow_encap.is_server_ctxt = true;
            flow_encap.encrypt_qid = pfi.qid2;
            flow_encap.decrypt_qid = pfi.qid1;
        }
    }

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
        HAL_TRACE_VERBOSE("tcp-proxy: both src and dest local. proxy shouldn't be enabled");
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
    bool handled;

    // Ignore direction for lookup. Always set it to 0
    if (ctx.role() == hal::FLOW_ROLE_RESPONDER) {
        HAL_TRACE_VERBOSE("{}: responder side. ignoring.", __FUNCTION__);
        return fte::PIPELINE_CONTINUE;
    }

    if (ctx.protobuf_request()) {
        HAL_TRACE_VERBOSE("protobuf request. ignoring.");
        return fte::PIPELINE_CONTINUE;
    }

    // Ignore direction for lookup. Always set it to 0
    flow_key.dir = 0;

    // get the flow info for the tcp proxy service
    pfi = tcp_proxy_get_flow_info(flow_key);
    if(!pfi) {
        HAL_TRACE_VERBOSE("proxy flow info not found for the flow. ignoring");
        return fte::PIPELINE_CONTINUE;
    }

    ret = tcp_get_flow_encap(ctx, *pfi, flow_encap);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to get flow encap params");
        return fte::PIPELINE_CONTINUE;
    }

    handled = hal::pd::lkl_handle_flow_miss_pkt(ctx, hal::pd::lkl_alloc_skbuff(ctx.cpu_rxhdr(),
                                                                                ctx.pkt(),
                                                                                ctx.pkt_len(),
                                                                                ctx.direction()),
                                                      ctx.direction(),
                                                      pfi->qid1, pfi->qid2,
                                                      ctx.cpu_rxhdr(),
                                                      pfi,
                                                      &flow_encap);
    HAL_TRACE_VERBOSE("LKL return {}", handled);
    return fte::PIPELINE_CONTINUE;
}

void
tcp_exec_tcp_lif_alt(hal::pd::tcp_msg_info_t& msg_info)
{
    const hal::pd::p4_to_p4plus_cpu_pkt_t* rxhdr = msg_info.u.pkt.cpu_rxhdr;
    hal::flow_direction_t dir = hal::lklshim_get_flow_hit_pkt_direction(rxhdr->qid);

    if (msg_info.u.pkt.pkt_len == 0) {
        HAL_TRACE_DEBUG("tcp_exec_tcp_lif: LKL return {}",
                        hal::pd::lkl_handle_flow_hit_hdr(
                                                         hal::pd::lkl_alloc_skbuff(rxhdr,
                                                                                   msg_info.u.pkt.pkt,
                                                                                   msg_info.u.pkt.pkt_len,
                                                                                   dir),
                                                         dir,
                                                         rxhdr));
    } else {
        HAL_TRACE_DEBUG("tcp_exec_tcp_lif: LKL return {}",
                        hal::pd::lkl_handle_flow_hit_pkt(
                                                         hal::pd::lkl_alloc_skbuff(rxhdr,
                                                                                   msg_info.u.pkt.pkt,
                                                                                   msg_info.u.pkt.pkt_len,
                                                                                   dir),
                                                         dir,
                                                         rxhdr));
    }
    return;
}

fte::pipeline_action_t
tcp_exec_tcp_lif(fte::ctx_t& ctx)
{
    const hal::pd::p4_to_p4plus_cpu_pkt_t* rxhdr = ctx.cpu_rxhdr();
    hal::flow_direction_t dir = hal::lklshim_get_flow_hit_pkt_direction(rxhdr->qid);
    bool handled;
    if (ctx.pkt_len() == 0) {
        handled = hal::pd::lkl_handle_flow_hit_hdr(hal::pd::lkl_alloc_skbuff(rxhdr,
                                                                           ctx.pkt(),
                                                                           ctx.pkt_len(),
                                                                           dir),
                                                                        dir,
                                                                        rxhdr);
        HAL_TRACE_VERBOSE("tcp_exec_tcp_lif: LKL return {}", handled);
    } else {
        handled = hal::pd::lkl_handle_flow_hit_pkt(hal::pd::lkl_alloc_skbuff(rxhdr,
                                                                            ctx.pkt(),
                                                                            ctx.pkt_len(),
                                                                            dir),
                                                                        dir,
                                                                        rxhdr);
        HAL_TRACE_VERBOSE("tcp_exec_tcp_lif: LKL return {}", handled);
    }

    return fte::PIPELINE_CONTINUE;
}

fte::pipeline_action_t
tcp_exec(fte::ctx_t& ctx)
{
    const fte::cpu_rxhdr_t* cpu_rxhdr = ctx.cpu_rxhdr();

    if (cpu_rxhdr && (cpu_rxhdr->lif == SERVICE_LIF_TCP_PROXY)) {
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
    if (!gl_ctx->tcp_proxy_pipeline()) {
        if (gl_ctx) {
            HAL_TRACE_DEBUG("tcp-proxy: txpkt src_lif={} src_vlan_id={}", src_lif, src_vlan_id);

            hal::pd::cpu_to_p4plus_header_t cpu_header = {0};
            hal::pd::p4plus_to_p4_header_t  p4plus_header = {0};
            cpu_header.src_lif = src_lif;
            cpu_header.hw_vlan_id = src_vlan_id;
            cpu_header.flags = CPU_TO_P4PLUS_FLAGS_UPD_VLAN;
            p4plus_header.lkp_inst = 1;
            p4plus_header.compute_l4_csum = 1;
            p4plus_header.compute_ip_csum = 1;

            gl_ctx->queue_txpkt(pkt, len, &cpu_header, &p4plus_header);
        } else {
            HAL_TRACE_DEBUG("tcp-proxy: gl_ctx is NULL");
        }
    } else {

        hal::pd::cpu_to_p4plus_header_t cpu_header = {0};
        hal::pd::p4plus_to_p4_header_t  p4plus_header = {0};
        hal::pd::pd_cpupkt_send_args_t args;
        hal::pd::pd_func_args_t pd_func_args = {0};
        hal_ret_t ret;

        if (gl_tcp_ctx == NULL || gl_tcp_ctx->arm_ctx == NULL) {
            HAL_TRACE_ERR("gl_tcp_ctx not set !");
            HAL_ABORT(0);
        }

        cpu_header.src_lif = src_lif;
        cpu_header.hw_vlan_id = src_vlan_id;
        cpu_header.flags = CPU_TO_P4PLUS_FLAGS_UPD_VLAN;
        p4plus_header.lkp_inst = 1;
        p4plus_header.compute_l4_csum = 1;
        p4plus_header.compute_ip_csum = 1;
        p4plus_header.p4plus_app_id = P4PLUS_APPTYPE_CPU;

        args.ctxt = (hal::pd::cpupkt_ctxt_t *)gl_tcp_ctx->arm_ctx;
        args.type = types::WRING_TYPE_ASQ;
        args.queue_id = gl_tcp_ctx->qid;
        args.cpu_header = &cpu_header;
        args.p4_header = &p4plus_header;
        args.data = pkt;
        args.data_len = len;
        args.dest_lif = HAL_LIF_CPU;
        args.qtype = CPU_ASQ_QTYPE;
        args.qid = gl_tcp_ctx->qid;
        args.ring_number = CPU_SCHED_RING_ASQ;
        HAL_TRACE_DEBUG("txpkt lkp_inst={} src_lif={} vlan={} "
                        "dest_lifq lif={} qtype={} qid={} ring={} wring={} pkt={:p} len={}",
                        p4plus_header.lkp_inst,
                        cpu_header.src_lif,
                        cpu_header.hw_vlan_id,
                        args.dest_lif, args.qtype, args.qid, args.ring_number, args.type,
                        args.data, args.data_len);
        pd_func_args.pd_cpupkt_send = &args;
        ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_SEND, &pd_func_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("fte: failed to transmit pkt, ret={}", ret);
        }
        gl_tcp_ctx->txpkt_cnt++;
    }
}

void *
tcp_rings_ctxt_init(uint8_t qid, void *arm_ctx)
{
    hal_ret_t ret;
    hal::pd::pd_tcp_rings_ctxt_init_args_t args;
    hal::pd::pd_tcp_rings_register_args_t reg_args;
    hal::pd::pd_func_args_t pd_func_args = {0};

    /*
     *  Init the global thread local ctx for tcp processing
     *  Assumption: One to one mapping between tcp ring ctxt and calling thread
     */
    gl_tcp_ctx = &tcp_ctx;
    // Tcp processing ctx requires arm ctx to send pkts. Only Rx tcp pkt is done under tcp ctx.
    gl_tcp_ctx->arm_ctx = arm_ctx;
    gl_tcp_ctx->qid = qid;

    pd_func_args.pd_tcp_rings_ctxt_init = &args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_TCP_RINGS_CTXT_INIT, &pd_func_args);
    if (ret != HAL_RET_OK)
        HAL_TRACE_DEBUG("Init failed {}", ret);                     \
    SDK_ASSERT(ret == HAL_RET_OK);

    reg_args.ctxt = args.ctxt;
    reg_args.type = types::WRING_TYPE_TCP_ACTL_Q;
    reg_args.queue_id = qid;
    pd_func_args.pd_tcp_rings_register = &reg_args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_TCP_RINGS_REGISTER, &pd_func_args);
    if (ret != HAL_RET_OK)
        HAL_TRACE_DEBUG("Register failed {}", ret);                     \
    SDK_ASSERT(ret == HAL_RET_OK);

    return args.ctxt;
}

hal_ret_t
tcp_rings_poll(void *ctxt)
{
    hal_ret_t ret = HAL_RET_OK;
    uint32_t msg_cnt;
    pd::pd_func_args_t pd_func_args = {0};
    hal::pd::tcp_msg_batch_t *batch = &gl_tcp_ctx->batch;

    hal::pd::pd_tcp_rings_poll_args_t args = {0};
    args.ctxt = ctxt;
    args.msg_batch = batch;
    pd_func_args.pd_tcp_rings_poll = &args;
    hal::pd::tcp_msg_info_t *msg_info;
    uint8_t *pkt;
    bool copied_pkt;

    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_TCP_RINGS_POLL, &pd_func_args);
    if(ret != HAL_RET_OK) {
        if (ret != HAL_RET_RETRY) {
            HAL_TRACE_ERR("Failed to poll tcp rings: {}", ret);
        }
        return ret;
    }

    msg_cnt = batch->msg_cnt;
    if (msg_cnt) {
        HAL_TRACE_DEBUG("msg_cnt={}", msg_cnt);
    }

    for(uint32_t i = 0; i < msg_cnt; i++) {
        msg_info = &batch->msg_info[i];
        if (msg_info->msg_type == TCP_ACTL_MSG_TYPE_PKT) {
            pkt = msg_info->u.pkt.pkt;
            copied_pkt = msg_info->u.pkt.copied_pkt;
            tcp_exec_tcp_lif_alt(*msg_info);

            // TODO: Revisit. Who frees the descr/copied-to slab in case on txpkt_cnt non zero
            if (gl_tcp_ctx->txpkt_cnt == 0) {
                if (copied_pkt) {
                    hal::free_to_slab(hal::HAL_SLAB_CPU_PKT, (pkt-sizeof(hal::pd::p4_to_p4plus_cpu_pkt_t)));
                } else {

                    /*
                     * Hand-over the packet to cpupkt library, to free any resources allocated
                     * from the data-path for this packet.
                     */
                    hal::pd::pd_cpupkt_free_pkt_resources_args_t args;
                    hal::pd::pd_func_args_t pd_func_args = {0};
                    args.ctxt = NULL; // Unused field
                    args.pkt = (pkt - sizeof(hal::pd::p4_to_p4plus_cpu_pkt_t));
                    pd_func_args.pd_cpupkt_free_pkt_resources = &args;

                    hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_FREE_PKT_RES, &pd_func_args);
                }
            }
        } else {
            // Only pkt type msg supported as of now.
            HAL_TRACE_ERR("Unexpected msg type {}", msg_info->msg_type);
            HAL_ABORT(0);
        }
        // Reset txpkt_cnt after each tcp rxpkt processing
        gl_tcp_ctx->txpkt_cnt = 0;
    }

    // Reset msg_cnt per polled batch processing, ready for next batch processing
    batch->msg_cnt = 0;

    return ret;
}

} // namespace hal
} // namespace net
