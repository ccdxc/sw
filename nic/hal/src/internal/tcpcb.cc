//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/src/internal/tcp_proxy_cb.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/tls/tls_api.hpp"
#include "nic/include/tcp_common.h"

namespace hal {

using hal::tls::proxy_tls_bypass_mode;

void *
tcpcb_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((tcpcb_t *)entry)->cb_id);
}

uint32_t
tcpcb_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(tcpcb_id_t)) % ht_size;
}

bool
tcpcb_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(tcpcb_id_t *)key1 == *(tcpcb_id_t *)key2) {
        return true;
    }
    return false;
}

void *
tcpcb_get_handle_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((tcpcb_t *)entry)->hal_handle);
}

uint32_t
tcpcb_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
tcpcb_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// validate an incoming TCPCB create request
// TODO:
// 1. check if TCPCB exists already
//------------------------------------------------------------------------------
static hal_ret_t
validate_tcpcb_create (TcpCbSpec& spec, TcpCbResponse *rsp)
{
    // must have key-handle set
    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_TCP_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    // must have key in the key-handle
    if (spec.key_or_handle().key_or_handle_case() !=
            TcpCbKeyHandle::kTcpcbId) {
        rsp->set_api_status(types::API_STATUS_TCP_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert this TCP CB in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_tcpcb_to_db (tcpcb_t *tcpcb)
{
    g_hal_state->tcpcb_id_ht()->insert(tcpcb, &tcpcb->ht_ctxt);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a TCP CB create request
// TODO: if TCP CB exists, treat this as modify (vrf id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
tcpcb_create (TcpCbSpec& spec, TcpCbResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    tcpcb_t                *tcpcb;
    pd::pd_tcpcb_create_args_t    pd_tcpcb_args;
    pd::pd_func_args_t          pd_func_args = {0};

    // validate the request message
    ret = validate_tcpcb_create(spec, rsp);

    // instantiate TCP CB
    tcpcb = tcpcb_alloc_init();
    if (tcpcb == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }

    tcpcb->cb_id = spec.key_or_handle().tcpcb_id();
    tcpcb->other_qid = spec.other_qid();
    tcpcb->rcv_nxt = spec.rcv_nxt();
    tcpcb->snd_nxt = spec.snd_nxt();
    tcpcb->snd_una = spec.snd_una();
    tcpcb->rcv_tsval = spec.rcv_tsval();
    tcpcb->ts_recent = spec.ts_recent();
    if (hal::tls::proxy_tls_bypass_mode) {
        tcpcb->bypass_tls = true;
        tcpcb->debug_dol = spec.debug_dol() | TCP_DDOL_BYPASS_BARCO;
        tcpcb->debug_dol_tx = spec.debug_dol_tx() | TCP_TX_DDOL_BYPASS_BARCO;
    } else {
        tcpcb->bypass_tls = false;
        tcpcb->debug_dol = spec.debug_dol();
        tcpcb->debug_dol_tx = spec.debug_dol_tx();
    }
    tcpcb->snd_wnd = spec.snd_wnd();
    tcpcb->snd_cwnd = spec.snd_cwnd();
    tcpcb->initial_window = spec.initial_window();
    tcpcb->snd_ssthresh = spec.snd_ssthresh();
    tcpcb->snd_cwnd_cnt = spec.snd_cwnd_cnt();
    tcpcb->rcv_mss = spec.rcv_mss();
    tcpcb->smss = spec.smss();
    tcpcb->source_port = spec.source_port();
    tcpcb->dest_port = spec.dest_port();
    tcpcb->header_len = spec.header_len();
    memcpy(tcpcb->header_template, spec.header_template().c_str(),
            std::min(sizeof(tcpcb->header_template), spec.header_template().size()));
    tcpcb->state = spec.state();
    tcpcb->source_lif = spec.source_lif();
    tcpcb->l7_proxy_type = spec.l7_proxy_type();
    tcpcb->serq_pi = spec.serq_pi();
    tcpcb->serq_ci = spec.serq_ci();
    tcpcb->pred_flags = spec.pred_flags();
    tcpcb->rto = spec.rto();
    tcpcb->rto_backoff = spec.rto_backoff();
    tcpcb->cpu_id = spec.cpu_id();
    tcpcb->rcv_wnd = spec.rcv_wnd();
    tcpcb->snd_wscale = spec.snd_wscale();
    tcpcb->rcv_wscale = spec.rcv_wscale();
    tcpcb->delay_ack = spec.delay_ack();
    tcpcb->ato = spec.ato();
    tcpcb->abc_l_var = spec.abc_l_var();

    tcpcb->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming
    pd::pd_tcpcb_create_args_init(&pd_tcpcb_args);
    pd_tcpcb_args.tcpcb = tcpcb;
    pd_func_args.pd_tcpcb_create = &pd_tcpcb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_TCPCB_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD TCP CB create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // add this L2 segment to our db
    ret = add_tcpcb_to_db(tcpcb);
    HAL_ASSERT(ret == HAL_RET_OK);

    HAL_TRACE_DEBUG("Added TCPCB to DB  with id: {}", tcpcb->cb_id);
    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_tcpcb_status()->set_tcpcb_handle(tcpcb->hal_handle);
    return HAL_RET_OK;

cleanup:
    tcpcb_free(tcpcb);
    return ret;
}

//------------------------------------------------------------------------------
// process a TCP CB update request
//------------------------------------------------------------------------------
hal_ret_t
tcpcb_update (TcpCbSpec& spec, TcpCbResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    tcpcb_t*               tcpcb;
    pd::pd_tcpcb_update_args_t    pd_tcpcb_args;
    pd::pd_func_args_t          pd_func_args = {0};

    auto kh = spec.key_or_handle();

    tcpcb = find_tcpcb_by_id(kh.tcpcb_id());
    if (tcpcb == NULL) {
        HAL_TRACE_DEBUG("tcpcb_update cb not found: {}", kh.tcpcb_id());
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_TCP_CB_NOT_FOUND;
    }

    pd::pd_tcpcb_update_args_init(&pd_tcpcb_args);
    HAL_TRACE_DEBUG("rcv_nxt: {:#x}", spec.rcv_nxt());
    tcpcb->rcv_nxt = spec.rcv_nxt();
    tcpcb->snd_nxt = spec.snd_nxt();
    tcpcb->snd_una = spec.snd_una();
    tcpcb->rcv_tsval = spec.rcv_tsval();
    tcpcb->ts_recent = spec.ts_recent();
    if (hal::tls::proxy_tls_bypass_mode) {
        tcpcb->bypass_tls = true;
        tcpcb->debug_dol = spec.debug_dol() | TCP_DDOL_BYPASS_BARCO;
        tcpcb->debug_dol_tx = spec.debug_dol_tx() | TCP_TX_DDOL_BYPASS_BARCO;
    } else {
        tcpcb->bypass_tls = false;
        tcpcb->debug_dol = spec.debug_dol();
        tcpcb->debug_dol_tx = spec.debug_dol_tx();
    }
    tcpcb->debug_dol = spec.debug_dol();
    tcpcb->debug_dol_tx = spec.debug_dol_tx();
    tcpcb->snd_wnd = spec.snd_wnd();
    tcpcb->snd_cwnd = spec.snd_cwnd();
    tcpcb->initial_window = spec.initial_window();
    tcpcb->snd_ssthresh = spec.snd_ssthresh();
    tcpcb->snd_cwnd_cnt = spec.snd_cwnd_cnt();
    tcpcb->rcv_mss = spec.rcv_mss();
    tcpcb->smss = spec.smss();
    tcpcb->source_port = spec.source_port();
    tcpcb->dest_port = spec.dest_port();
    tcpcb->state = spec.state();
    tcpcb->source_lif = spec.source_lif();
    tcpcb->header_len = spec.header_len();
    tcpcb->l7_proxy_type = spec.l7_proxy_type();
    tcpcb->serq_pi = spec.serq_pi();
    tcpcb->serq_ci = spec.serq_ci();
    tcpcb->pred_flags = spec.pred_flags();
    tcpcb->rto = spec.rto();
    tcpcb->rto_backoff = spec.rto_backoff();
    tcpcb->cpu_id = spec.cpu_id();
    tcpcb->rcv_wnd = spec.rcv_wnd();
    tcpcb->snd_wscale = spec.snd_wscale();
    tcpcb->rcv_wscale = spec.rcv_wscale();
    tcpcb->delay_ack = spec.delay_ack();
    tcpcb->ato = spec.ato();
    tcpcb->abc_l_var = spec.abc_l_var();
    memcpy(tcpcb->header_template, spec.header_template().c_str(),
            std::max(sizeof(tcpcb->header_template), spec.header_template().size()));
    pd_tcpcb_args.tcpcb = tcpcb;

    pd_func_args.pd_tcpcb_update = &pd_tcpcb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_TCPCB_UPDATE, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD TCPCB: Update Failed, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill stats of this TCP CB
    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

hal_ret_t
tcp_proxy_session_fill_rsp(hal::proxy_flow_info_t *proxy_flow_info,
                           tcp_proxy::TcpProxySessionGetResponse *rsp)
{
    rsp->mutable_tcpproxy_flow()->set_sport(proxy_flow_info->flow_key.sport);
    rsp->mutable_tcpproxy_flow()->set_dport(proxy_flow_info->flow_key.dport);

    rsp->mutable_tcpproxy_flow()->set_flow_type(proxy_flow_info->flow_key.flow_type);

    if(proxy_flow_info->flow_key.flow_type == FLOW_TYPE_V4) {
        rsp->mutable_tcpproxy_flow()->mutable_src_ip()->set_ip_af(types::IP_AF_INET);
        rsp->mutable_tcpproxy_flow()->mutable_src_ip()->set_v4_addr(
                proxy_flow_info->flow_key.sip.v4_addr);
        rsp->mutable_tcpproxy_flow()->mutable_dst_ip()->set_ip_af(types::IP_AF_INET);
        rsp->mutable_tcpproxy_flow()->mutable_dst_ip()->set_v4_addr(
                proxy_flow_info->flow_key.dip.v4_addr);
    } else {
        rsp->mutable_tcpproxy_flow()->mutable_src_ip()->set_ip_af(types::IP_AF_INET6);
        rsp->mutable_tcpproxy_flow()->mutable_src_ip()->set_v6_addr(
                &proxy_flow_info->flow_key.sip.v6_addr, IP6_ADDR8_LEN);
        rsp->mutable_tcpproxy_flow()->mutable_dst_ip()->set_ip_af(types::IP_AF_INET6);
        rsp->mutable_tcpproxy_flow()->mutable_dst_ip()->set_v6_addr(
                &proxy_flow_info->flow_key.dip.v6_addr, IP6_ADDR8_LEN);
    }

    rsp->mutable_tcpproxy_flow()->set_qid1(proxy_flow_info->qid1);
    rsp->mutable_tcpproxy_flow()->set_qid2(proxy_flow_info->qid2);

    return HAL_RET_OK;
}

static bool
session_matches_filter(hal::proxy_flow_info_t *proxy_flow_info,
                       tcp_proxy::TcpProxySessionFilter *filter)
{
    hal_ret_t ret;

    if(filter->match_all()) {
        return true;
    } else {
        flow_key_t *flow_key = &proxy_flow_info->flow_key;

        if((filter->src_port()) && (filter->src_port() != flow_key->sport)) {
            return false;
        }

        if((filter->dst_port()) && (filter->dst_port() != flow_key->dport)) {
            return false;
        }

        if(filter->has_src_ip()) {
            ip_addr_t ip_addr;
            ip_addr_t check_addr;

            ret = ip_addr_spec_to_ip_addr(&ip_addr, filter->src_ip());
            if(ret != HAL_RET_OK) {
                return false;
            }
            check_addr.af = IP_AF_IPV4;
            memcpy(&check_addr.addr, &flow_key->sip, sizeof(check_addr.addr));
 
            if (filter->src_ip().v4_addr() && (!ip_addr_check_equal(&check_addr, &ip_addr))) {
                return false;
            }
        }

        if(filter->has_dst_ip()) {
            ip_addr_t ip_addr;
            ip_addr_t check_addr;

            ret = ip_addr_spec_to_ip_addr(&ip_addr, filter->dst_ip());
            if(ret != HAL_RET_OK) {
                return false;
            }
            check_addr.af = IP_AF_IPV4;
            memcpy(&check_addr.addr, &flow_key->dip, sizeof(check_addr.addr));
 
            if (filter->dst_ip().v4_addr() && (!ip_addr_check_equal(&check_addr, &ip_addr))) {
                return false;
            }
        }

        return true;
    }
}

hal_ret_t
tcp_proxy_session_get (tcp_proxy::TcpProxySessionGetRequest& req,
                       tcp_proxy::TcpProxySessionGetResponseMsg *rsp)
{
    struct session_filter_t {
        tcp_proxy::TcpProxySessionFilter *filter;
        tcp_proxy::TcpProxySessionGetResponseMsg *response;
    } ctx = {0};

    auto walk_func = [](void *entry, void *ctx) {
        hal::proxy_flow_info_t  *proxy_flow_info = (proxy_flow_info_t *)entry;
        tcp_proxy::TcpProxySessionFilter *filter =
                      ((session_filter_t *)ctx)->filter;
        tcp_proxy::TcpProxySessionGetResponseMsg *response =
                      ((session_filter_t *)ctx)->response;

        if (session_matches_filter(proxy_flow_info, filter)) {
            tcp_proxy_session_fill_rsp(proxy_flow_info, response->add_response());
        }
        return false;
    };
 
    types::ProxyType proxy_type = types::PROXY_TYPE_TCP;
    proxy_t *proxy = (proxy_t *)g_hal_state->proxy_type_ht()->lookup(&proxy_type);
    if(!proxy) {
        HAL_TRACE_ERR("No TCP sessions found!");
        return HAL_RET_HW_FAIL;
    }
 
    ctx.filter = req.mutable_session_filter();
    ctx.response = rsp;
    sdk::sdk_ret_t ret = proxy->flow_ht_->walk_safe(walk_func, &ctx);

    return hal_sdk_ret_to_hal_ret(ret);
}

//------------------------------------------------------------------------------
// process a TCP CB get request
//------------------------------------------------------------------------------
hal_ret_t
tcpcb_get (TcpCbGetRequest& req, TcpCbGetResponseMsg *resp)
{
    hal_ret_t              ret = HAL_RET_OK;
    tcpcb_t                rtcpcb;
    tcpcb_t*               tcpcb;
    pd::pd_tcpcb_get_args_t    pd_tcpcb_args;
    pd::pd_func_args_t          pd_func_args = {0};
    TcpCbGetResponse *rsp = resp->add_response();

    auto kh = req.key_or_handle();

    tcpcb = find_tcpcb_by_id(kh.tcpcb_id());
    if (tcpcb == NULL) {
        HAL_TRACE_ERR("TCPCB get: Failed to find cb with id {}", kh.tcpcb_id());
	    rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_TCP_CB_NOT_FOUND;
    }

    tcpcb_init(&rtcpcb);
    rtcpcb.cb_id = tcpcb->cb_id;
    pd::pd_tcpcb_get_args_init(&pd_tcpcb_args);
    pd_tcpcb_args.tcpcb = &rtcpcb;

    pd_func_args.pd_tcpcb_get = &pd_tcpcb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_TCPCB_GET, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD TCPCB: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    HAL_TRACE_DEBUG("cb_id: {:#x}, rcv_nxt: {:#x}", rtcpcb.cb_id, rtcpcb.rcv_nxt);

    // fill config spec of this TCP CB
    rsp->mutable_spec()->mutable_key_or_handle()->set_tcpcb_id(rtcpcb.cb_id);
    rsp->mutable_spec()->set_rcv_nxt(rtcpcb.rcv_nxt);
    rsp->mutable_spec()->set_rx_ts(rtcpcb.rx_ts);
    rsp->mutable_spec()->set_snd_nxt(rtcpcb.snd_nxt);
    rsp->mutable_spec()->set_snd_recover(rtcpcb.snd_recover);
    rsp->mutable_spec()->set_snd_una(rtcpcb.snd_una);
    rsp->mutable_spec()->set_rcv_tsval(rtcpcb.rcv_tsval);
    rsp->mutable_spec()->set_ts_recent(rtcpcb.ts_recent);
    rsp->mutable_spec()->set_serq_base(rtcpcb.serq_base);
    rsp->mutable_spec()->set_debug_dol(rtcpcb.debug_dol);
    rsp->mutable_spec()->set_sesq_pi(rtcpcb.sesq_pi);
    rsp->mutable_spec()->set_sesq_ci(rtcpcb.sesq_ci);
    rsp->mutable_spec()->set_sesq_base(rtcpcb.sesq_base);
    rsp->mutable_spec()->set_asesq_pi(rtcpcb.asesq_pi);
    rsp->mutable_spec()->set_asesq_ci(rtcpcb.asesq_ci);
    rsp->mutable_spec()->set_asesq_base(rtcpcb.asesq_base);
    rsp->mutable_spec()->set_snd_wnd(rtcpcb.snd_wnd);
    rsp->mutable_spec()->set_snd_cwnd(rtcpcb.snd_cwnd);
    rsp->mutable_spec()->set_initial_window(rtcpcb.initial_window);
    rsp->mutable_spec()->set_snd_ssthresh(rtcpcb.snd_ssthresh);
    rsp->mutable_spec()->set_snd_cwnd_cnt(rtcpcb.snd_cwnd_cnt);
    rsp->mutable_spec()->set_rcv_mss(rtcpcb.rcv_mss);
    rsp->mutable_spec()->set_smss(rtcpcb.smss);
    rsp->mutable_spec()->set_source_port(rtcpcb.source_port);
    rsp->mutable_spec()->set_dest_port(rtcpcb.dest_port);
    rsp->mutable_spec()->set_header_len(rtcpcb.header_len);
    rsp->mutable_spec()->set_header_template(rtcpcb.header_template,
                                             sizeof(rtcpcb.header_template));
    rsp->mutable_spec()->set_sesq_retx_ci(rtcpcb.sesq_retx_ci);
    rsp->mutable_spec()->set_sesq_tx_ci(rtcpcb.sesq_tx_ci);
    rsp->mutable_spec()->set_retx_snd_una(rtcpcb.retx_snd_una);
    rsp->mutable_spec()->set_rto(rtcpcb.rto);

    rsp->mutable_spec()->set_state(rtcpcb.state);
    rsp->mutable_spec()->set_source_lif(rtcpcb.source_lif);
    rsp->mutable_spec()->set_debug_dol_tx(rtcpcb.debug_dol_tx);
    rsp->mutable_spec()->set_l7_proxy_type(rtcpcb.l7_proxy_type);
    rsp->mutable_spec()->set_serq_pi(rtcpcb.serq_pi);
    rsp->mutable_spec()->set_serq_ci(rtcpcb.serq_ci);
    rsp->mutable_spec()->set_pred_flags(rtcpcb.pred_flags);
    rsp->mutable_spec()->set_debug_dol_tblsetaddr(rtcpcb.debug_dol_tblsetaddr);
    rsp->mutable_spec()->set_packets_out(rtcpcb.packets_out);
    rsp->mutable_spec()->set_rto_backoff(rtcpcb.rto_backoff);
    rsp->mutable_spec()->set_cpu_id(rtcpcb.cpu_id);
    rsp->mutable_spec()->set_rcv_wnd(rtcpcb.rcv_wnd);
    rsp->mutable_spec()->set_snd_wscale(rtcpcb.snd_wscale);
    rsp->mutable_spec()->set_rcv_wscale(rtcpcb.rcv_wscale);
    rsp->mutable_spec()->set_delay_ack(rtcpcb.delay_ack);
    rsp->mutable_spec()->set_ato(rtcpcb.ato);
    rsp->mutable_spec()->set_abc_l_var(rtcpcb.abc_l_var);

    // fill operational state of this TCP CB
    rsp->mutable_status()->set_tcpcb_handle(tcpcb->hal_handle);

    // fill stats of this TCP CB
    rsp->mutable_stats()->set_bytes_rcvd(rtcpcb.bytes_rcvd);
    rsp->mutable_stats()->set_pkts_rcvd(rtcpcb.pkts_rcvd);
    rsp->mutable_stats()->set_pages_alloced(rtcpcb.pages_alloced);
    rsp->mutable_stats()->set_desc_alloced(rtcpcb.desc_alloced);
    rsp->mutable_stats()->set_debug_num_pkt_to_mem(rtcpcb.debug_num_pkt_to_mem);
    rsp->mutable_stats()->set_debug_num_phv_to_mem(rtcpcb.debug_num_phv_to_mem);
    rsp->mutable_stats()->set_bytes_acked(rtcpcb.bytes_acked);
    rsp->mutable_stats()->set_slow_path_cnt(rtcpcb.slow_path_cnt);
    rsp->mutable_stats()->set_serq_full_cnt(rtcpcb.serq_full_cnt);
    rsp->mutable_stats()->set_ooo_cnt(rtcpcb.ooo_cnt);

    rsp->mutable_stats()->set_debug_atomic_delta(rtcpcb.debug_atomic_delta);
    rsp->mutable_stats()->set_debug_atomic0_incr1247(rtcpcb.debug_atomic0_incr1247);
    rsp->mutable_stats()->set_debug_atomic1_incr247(rtcpcb.debug_atomic1_incr247);
    rsp->mutable_stats()->set_debug_atomic2_incr47(rtcpcb.debug_atomic2_incr47);
    rsp->mutable_stats()->set_debug_atomic3_incr47(rtcpcb.debug_atomic3_incr47);
    rsp->mutable_stats()->set_debug_atomic4_incr7(rtcpcb.debug_atomic4_incr7);
    rsp->mutable_stats()->set_debug_atomic5_incr7(rtcpcb.debug_atomic5_incr7);
    rsp->mutable_stats()->set_debug_atomic6_incr7(rtcpcb.debug_atomic6_incr7);

    rsp->mutable_stats()->set_bytes_sent(rtcpcb.bytes_sent);
    rsp->mutable_stats()->set_pkts_sent(rtcpcb.pkts_sent);
    rsp->mutable_stats()->set_debug_num_phv_to_pkt(rtcpcb.debug_num_phv_to_pkt);
    rsp->mutable_stats()->set_debug_num_mem_to_pkt(rtcpcb.debug_num_mem_to_pkt);
    rsp->mutable_stats()->set_sesq_pi(rtcpcb.sesq_pi);
    rsp->mutable_stats()->set_sesq_ci(rtcpcb.sesq_ci);
    rsp->mutable_stats()->set_sesq_retx_ci(rtcpcb.sesq_retx_ci);
    rsp->mutable_stats()->set_asesq_retx_ci(rtcpcb.asesq_retx_ci);
    rsp->mutable_stats()->set_send_ack_pi(rtcpcb.send_ack_pi);
    rsp->mutable_stats()->set_send_ack_ci(rtcpcb.send_ack_ci);
    rsp->mutable_stats()->set_fast_timer_pi(rtcpcb.fast_timer_pi);
    rsp->mutable_stats()->set_fast_timer_ci(rtcpcb.fast_timer_ci);
    rsp->mutable_stats()->set_del_ack_pi(rtcpcb.del_ack_pi);
    rsp->mutable_stats()->set_del_ack_ci(rtcpcb.del_ack_ci);
    rsp->mutable_stats()->set_asesq_pi(rtcpcb.asesq_pi);
    rsp->mutable_stats()->set_asesq_ci(rtcpcb.asesq_ci);
    rsp->mutable_stats()->set_pending_tx_pi(rtcpcb.pending_tx_pi);
    rsp->mutable_stats()->set_pending_tx_ci(rtcpcb.pending_tx_ci);
    rsp->mutable_stats()->set_fast_retrans_pi(rtcpcb.fast_retrans_pi);
    rsp->mutable_stats()->set_fast_retrans_ci(rtcpcb.fast_retrans_ci);
    rsp->mutable_stats()->set_clean_retx_pi(rtcpcb.clean_retx_pi);
    rsp->mutable_stats()->set_clean_retx_ci(rtcpcb.clean_retx_ci);
    rsp->mutable_stats()->set_packets_out(rtcpcb.packets_out);
    rsp->mutable_stats()->set_sesq_tx_ci(rtcpcb.sesq_tx_ci);
    rsp->mutable_stats()->set_tx_ring_pi(rtcpcb.tx_ring_pi);
    rsp->mutable_stats()->set_stretch_ack_cnt(rtcpcb.stretch_ack_cnt);
    rsp->mutable_stats()->set_rto_deadline(rtcpcb.rto_deadline);
    rsp->mutable_stats()->set_ato_deadline(rtcpcb.ato_deadline);
    rsp->mutable_stats()->set_idle_deadline(rtcpcb.idle_deadline);
    rsp->mutable_stats()->set_cc_flags(rtcpcb.cc_flags);

    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a TCP CB delete request
//------------------------------------------------------------------------------
hal_ret_t
tcpcb_delete (internal::TcpCbDeleteRequest& req, internal::TcpCbDeleteResponseMsg *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    tcpcb_t*               tcpcb;
    pd::pd_tcpcb_delete_args_t    pd_tcpcb_args;
    pd::pd_func_args_t          pd_func_args = {0};

    auto kh = req.key_or_handle();
    tcpcb = find_tcpcb_by_id(kh.tcpcb_id());
    if (tcpcb == NULL) {
        rsp->add_api_status(types::API_STATUS_OK);
        return HAL_RET_OK;
    }

    pd::pd_tcpcb_delete_args_init(&pd_tcpcb_args);
    pd_tcpcb_args.tcpcb = tcpcb;

    pd_func_args.pd_tcpcb_delete = &pd_tcpcb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_TCPCB_DELETE, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD TCPCB: delete Failed, err: {}", ret);
        rsp->add_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill stats of this TCP CB
    rsp->add_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

hal_ret_t
tcp_proxy_global_stats_get(tcp_proxy::TcpProxyGlobalStatsGetRequest& req,
                           tcp_proxy::TcpProxyGlobalStatsGetResponseMsg* rsp)
{
    hal_ret_t ret = HAL_RET_OK;
    pd::pd_func_args_t pd_func_args = {};
    pd::pd_tcp_global_stats_get_args_t pd_tcp_global_stats_get_args = {};

    pd_func_args.pd_tcp_global_stats_get = &pd_tcp_global_stats_get_args;

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_TCP_GLOBAL_STATS_GET, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD TCP GLOBAL STATS : get Failed, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    rsp->mutable_global_stats()->set_rnmdr_full(
                  pd_tcp_global_stats_get_args.rnmdr_full);
    rsp->mutable_global_stats()->set_invalid_sesq_descr(
                  pd_tcp_global_stats_get_args.invalid_sesq_descr);
    rsp->mutable_global_stats()->set_invalid_retx_sesq_descr(
                  pd_tcp_global_stats_get_args.invalid_retx_sesq_descr);
    rsp->mutable_global_stats()->set_stretch_ack(
                  pd_tcp_global_stats_get_args.stretch_ack);
    rsp->mutable_global_stats()->set_retx_nop_schedule(
                  pd_tcp_global_stats_get_args.retx_nop_schedule);
    rsp->mutable_global_stats()->set_gc_full(
                  pd_tcp_global_stats_get_args.gc_full);
    rsp->mutable_global_stats()->set_tls_gc_full(
                  pd_tcp_global_stats_get_args.tls_gc_full);
    rsp->set_api_status(types::API_STATUS_OK);

    return HAL_RET_OK;
}

}    // namespace hal
