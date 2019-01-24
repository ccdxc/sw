#include "nic/include/base.hpp"                                                                                                                                                                                                                                        
#include "nic/hal/hal.hpp"                                                                                                                                                                                                                                             
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/iris/include/hal_state.hpp"                                                                                                                                                                                                                                   
#include "nic/hal/plugins/cfg/tcp_proxy/tcp_proxy.hpp"                                                                                                                                                                                                                 
#include "nic/hal/plugins/cfg/nw/vrf.hpp"                                                                                                                                                                                                                              
#include "nic/include/pd_api.hpp"                                                                                                                                                                                                                                      
//#include "nic/hal/tls/tls_api.hpp"                                                                                                                                                                                                                                     
#include "nic/include/tcp_common.h"                                                                                                                                                                                                                                    
                                                                                                                                                                                                                                                                       
namespace hal {

//using hal::tls::proxy_tls_bypass_mode;

void *
tcp_proxy_cb_get_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((tcp_proxy_cb_t *)entry)->cb_id);
}

uint32_t
tcp_proxy_cb_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(tcp_proxy_cb_id_t)) % ht_size;
}

bool
tcp_proxy_cb_compare_key_func (void *key1, void *key2)
{
    SDK_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(tcp_proxy_cb_id_t *)key1 == *(tcp_proxy_cb_id_t *)key2) {
        return true;
    }
    return false;
}

void *
tcp_proxy_cb_get_handle_key_func (void *entry)
{
    SDK_ASSERT(entry != NULL);
    return (void *)&(((tcp_proxy_cb_t *)entry)->hal_handle);
}

uint32_t
tcp_proxy_cb_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
tcp_proxy_cb_compare_handle_key_func (void *key1, void *key2)
{
    SDK_ASSERT((key1 != NULL) && (key2 != NULL));
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
validate_tcp_proxy_cb_create (TcpProxyCbSpec& spec, TcpProxyCbResponse *rsp)
{
    // must have key-handle set
    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_TCP_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    // must have key in the key-handle
    if (spec.key_or_handle().key_or_handle_case() !=
            TcpProxyCbKeyHandle::kTcpcbId) {
        rsp->set_api_status(types::API_STATUS_TCP_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert this TCP CB in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_tcp_proxy_cb_to_db (tcp_proxy_cb_t *tcp_proxy_cb)
{
    g_hal_state->tcpcb_id_ht()->insert(tcp_proxy_cb, &tcp_proxy_cb->ht_ctxt);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a TCP CB create request
// TODO: if TCP CB exists, treat this as modify (vrf id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
tcp_proxy_cb_create (TcpProxyCbSpec& spec, TcpProxyCbResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    tcp_proxy_cb_t                *tcp_proxy_cb;
    pd::pd_tcp_proxy_cb_create_args_t    pd_tcp_proxy_cb_args;
    pd::pd_func_args_t          pd_func_args = {0};

    // validate the request message
    ret = validate_tcp_proxy_cb_create(spec, rsp);

    // instantiate TCP CB
    tcp_proxy_cb = tcp_proxy_cb_alloc_init();
    if (tcp_proxy_cb == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }

    tcp_proxy_cb->cb_id = spec.key_or_handle().tcpcb_id();
    tcp_proxy_cb->other_qid = spec.other_qid();
    tcp_proxy_cb->rcv_nxt = spec.rcv_nxt();
    tcp_proxy_cb->snd_nxt = spec.snd_nxt();
    tcp_proxy_cb->snd_una = spec.snd_una();
    tcp_proxy_cb->rcv_tsval = spec.rcv_tsval();
    tcp_proxy_cb->ts_recent = spec.ts_recent();

#if 0
    if (hal::tls::proxy_tls_bypass_mode) {
        tcp_proxy_cb->debug_dol = spec.debug_dol() | TCP_DDOL_BYPASS_BARCO;
        tcp_proxy_cb->debug_dol_tx = spec.debug_dol_tx() | TCP_TX_DDOL_BYPASS_BARCO;
    } else {
        tcp_proxy_cb->debug_dol = spec.debug_dol();
        tcp_proxy_cb->debug_dol_tx = spec.debug_dol_tx();
    }
#endif

    tcp_proxy_cb->snd_wnd = spec.snd_wnd();
    tcp_proxy_cb->snd_cwnd = spec.snd_cwnd();
    tcp_proxy_cb->snd_cwnd_cnt = spec.snd_cwnd_cnt();
    tcp_proxy_cb->rcv_mss = spec.rcv_mss();
    tcp_proxy_cb->source_port = spec.source_port();
    tcp_proxy_cb->dest_port = spec.dest_port();
    tcp_proxy_cb->header_len = spec.header_len();
    memcpy(tcp_proxy_cb->header_template, spec.header_template().c_str(),
            std::min(sizeof(tcp_proxy_cb->header_template), spec.header_template().size()));
    tcp_proxy_cb->pending_ack_send = spec.pending_ack_send();
    tcp_proxy_cb->state = spec.state();
    tcp_proxy_cb->source_lif = spec.source_lif();
    tcp_proxy_cb->l7_proxy_type = spec.l7_proxy_type();
    tcp_proxy_cb->serq_pi = spec.serq_pi();
    tcp_proxy_cb->serq_ci = spec.serq_ci();
    tcp_proxy_cb->pred_flags = spec.pred_flags();
    tcp_proxy_cb->rto = spec.rto();
    tcp_proxy_cb->rto_backoff = spec.rto_backoff();
    tcp_proxy_cb->cpu_id = spec.cpu_id();

    tcp_proxy_cb->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming
    pd::pd_tcp_proxy_cb_create_args_init(&pd_tcp_proxy_cb_args);
    pd_tcp_proxy_cb_args.tcp_proxy_cb = tcp_proxy_cb;
    pd_func_args.pd_tcp_proxy_cb_create = &pd_tcp_proxy_cb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_TCPCB_CREATE, &pd_func_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD TCP CB create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // add this L2 segment to our db
    ret = add_tcp_proxy_cb_to_db(tcp_proxy_cb);
    SDK_ASSERT(ret == HAL_RET_OK);

    HAL_TRACE_DEBUG("Added TCPCB to DB  with id: {}", tcp_proxy_cb->cb_id);
    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_tcpcb_status()->set_tcpcb_handle(tcp_proxy_cb->hal_handle);
    return HAL_RET_OK;

cleanup:
    tcp_proxy_cb_free(tcp_proxy_cb);
    return ret;
}

//------------------------------------------------------------------------------
// process a TCP CB update request
//------------------------------------------------------------------------------
hal_ret_t
tcp_proxy_cb_update (TcpProxyCbSpec& spec, TcpProxyCbResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    tcp_proxy_cb_t*               tcp_proxy_cb;
    pd::pd_tcp_proxy_cb_update_args_t    pd_tcp_proxy_cb_args;
    pd::pd_func_args_t          pd_func_args = {0};

    auto kh = spec.key_or_handle();

    tcp_proxy_cb = find_tcp_proxy_cb_by_id(kh.tcpcb_id());
    if (tcp_proxy_cb == NULL) {
        HAL_TRACE_DEBUG("tcp_proxy_cb_update cb not found: {}", kh.tcpcb_id());
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_TCP_CB_NOT_FOUND;
    }

    pd::pd_tcp_proxy_cb_update_args_init(&pd_tcp_proxy_cb_args);
    HAL_TRACE_DEBUG("rcv_nxt: {:#x}", spec.rcv_nxt());
    tcp_proxy_cb->rcv_nxt = spec.rcv_nxt();
    tcp_proxy_cb->snd_nxt = spec.snd_nxt();
    tcp_proxy_cb->snd_una = spec.snd_una();
    tcp_proxy_cb->rcv_tsval = spec.rcv_tsval();
    tcp_proxy_cb->ts_recent = spec.ts_recent();
#if 0
    if (hal::tls::proxy_tls_bypass_mode) {
        tcp_proxy_cb->debug_dol = spec.debug_dol() | TCP_DDOL_BYPASS_BARCO;
        tcp_proxy_cb->debug_dol_tx = spec.debug_dol_tx() | TCP_TX_DDOL_BYPASS_BARCO;
    } else {
        tcp_proxy_cb->debug_dol = spec.debug_dol();
        tcp_proxy_cb->debug_dol_tx = spec.debug_dol_tx();
    }
#endif
    tcp_proxy_cb->debug_dol = spec.debug_dol();
    tcp_proxy_cb->debug_dol_tx = spec.debug_dol_tx();
    tcp_proxy_cb->snd_wnd = spec.snd_wnd();
    tcp_proxy_cb->snd_cwnd = spec.snd_cwnd();
    tcp_proxy_cb->snd_cwnd_cnt = spec.snd_cwnd_cnt();
    tcp_proxy_cb->rcv_mss = spec.rcv_mss();
    tcp_proxy_cb->source_port = spec.source_port();
    tcp_proxy_cb->dest_port = spec.dest_port();
    tcp_proxy_cb->state = spec.state();
    tcp_proxy_cb->source_lif = spec.source_lif();
    tcp_proxy_cb->pending_ack_send = spec.pending_ack_send();
    tcp_proxy_cb->header_len = spec.header_len();
    tcp_proxy_cb->l7_proxy_type = spec.l7_proxy_type();
    tcp_proxy_cb->serq_pi = spec.serq_pi();
    tcp_proxy_cb->serq_ci = spec.serq_ci();
    tcp_proxy_cb->pred_flags = spec.pred_flags();
    tcp_proxy_cb->rto = spec.rto();
    tcp_proxy_cb->rto_backoff = spec.rto_backoff();
    tcp_proxy_cb->cpu_id = spec.cpu_id();
    memcpy(tcp_proxy_cb->header_template, spec.header_template().c_str(),
            std::max(sizeof(tcp_proxy_cb->header_template), spec.header_template().size()));
    pd_tcp_proxy_cb_args.tcp_proxy_cb = tcp_proxy_cb;

    pd_func_args.pd_tcp_proxy_cb_update = &pd_tcp_proxy_cb_args;
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

//------------------------------------------------------------------------------
// process a TCP CB get request
//------------------------------------------------------------------------------
hal_ret_t
tcp_proxy_cb_get (TcpProxyCbGetRequest& req, TcpProxyCbGetResponseMsg *resp)
{
    hal_ret_t              ret = HAL_RET_OK;
    tcp_proxy_cb_t                rtcp_proxy_cb;
    tcp_proxy_cb_t*               tcp_proxy_cb;
    pd::pd_tcp_proxy_cb_get_args_t    pd_tcp_proxy_cb_args;
    pd::pd_func_args_t          pd_func_args = {0};
    TcpProxyCbGetResponse *rsp = resp->add_response();

    auto kh = req.key_or_handle();

    tcp_proxy_cb = find_tcp_proxy_cb_by_id(kh.tcpcb_id());
    if (tcp_proxy_cb == NULL) {
        HAL_TRACE_ERR("TCPCB get: Failed to find cb with id {}", kh.tcpcb_id());
            rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_TCP_CB_NOT_FOUND;
    }

    tcp_proxy_cb_init(&rtcp_proxy_cb);
    rtcp_proxy_cb.cb_id = tcp_proxy_cb->cb_id;
    pd::pd_tcp_proxy_cb_get_args_init(&pd_tcp_proxy_cb_args);
    pd_tcp_proxy_cb_args.tcp_proxy_cb = &rtcp_proxy_cb;

    pd_func_args.pd_tcp_proxy_cb_get = &pd_tcp_proxy_cb_args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_TCPCB_GET, &pd_func_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD TCPCB: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    HAL_TRACE_DEBUG("cb_id: {:#x}, rcv_nxt: {:#x}", rtcp_proxy_cb.cb_id, rtcp_proxy_cb.rcv_nxt);

    // fill config spec of this TCP CB
    rsp->mutable_spec()->mutable_key_or_handle()->set_tcpcb_id(rtcp_proxy_cb.cb_id);
    rsp->mutable_spec()->set_rcv_nxt(rtcp_proxy_cb.rcv_nxt);
    rsp->mutable_spec()->set_rx_ts(rtcp_proxy_cb.rx_ts);
    rsp->mutable_spec()->set_snd_nxt(rtcp_proxy_cb.snd_nxt);
    rsp->mutable_spec()->set_snd_una(rtcp_proxy_cb.snd_una);
    rsp->mutable_spec()->set_rcv_tsval(rtcp_proxy_cb.rcv_tsval);
    rsp->mutable_spec()->set_ts_recent(rtcp_proxy_cb.ts_recent);
    rsp->mutable_spec()->set_serq_base(rtcp_proxy_cb.serq_base);
    rsp->mutable_spec()->set_debug_dol(rtcp_proxy_cb.debug_dol);
    rsp->mutable_spec()->set_sesq_pi(rtcp_proxy_cb.sesq_pi);
    rsp->mutable_spec()->set_sesq_ci(rtcp_proxy_cb.sesq_ci);
    rsp->mutable_spec()->set_sesq_base(rtcp_proxy_cb.sesq_base);
    rsp->mutable_spec()->set_asesq_pi(rtcp_proxy_cb.asesq_pi);
    rsp->mutable_spec()->set_asesq_ci(rtcp_proxy_cb.asesq_ci);
    rsp->mutable_spec()->set_asesq_base(rtcp_proxy_cb.asesq_base);
    rsp->mutable_spec()->set_snd_wnd(rtcp_proxy_cb.snd_wnd);
    rsp->mutable_spec()->set_snd_cwnd(rtcp_proxy_cb.snd_cwnd);
    rsp->mutable_spec()->set_snd_cwnd_cnt(rtcp_proxy_cb.snd_cwnd_cnt);
    rsp->mutable_spec()->set_rcv_mss(rtcp_proxy_cb.rcv_mss);
    rsp->mutable_spec()->set_source_port(rtcp_proxy_cb.source_port);
    rsp->mutable_spec()->set_dest_port(rtcp_proxy_cb.dest_port);
    rsp->mutable_spec()->set_header_len(rtcp_proxy_cb.header_len);
    rsp->mutable_spec()->set_header_template(rtcp_proxy_cb.header_template,
                                             sizeof(rtcp_proxy_cb.header_template));
    rsp->mutable_spec()->set_sesq_retx_ci(rtcp_proxy_cb.sesq_retx_ci);
    rsp->mutable_spec()->set_retx_snd_una(rtcp_proxy_cb.retx_snd_una);
    rsp->mutable_spec()->set_rto(rtcp_proxy_cb.rto);

    rsp->mutable_spec()->set_state(rtcp_proxy_cb.state);
    rsp->mutable_spec()->set_source_lif(rtcp_proxy_cb.source_lif);
    rsp->mutable_spec()->set_debug_dol_tx(rtcp_proxy_cb.debug_dol_tx);
    rsp->mutable_spec()->set_pending_ack_send(rtcp_proxy_cb.pending_ack_send);
    rsp->mutable_spec()->set_l7_proxy_type(rtcp_proxy_cb.l7_proxy_type);
    rsp->mutable_spec()->set_serq_pi(rtcp_proxy_cb.serq_pi);
    rsp->mutable_spec()->set_serq_ci(rtcp_proxy_cb.serq_ci);
    rsp->mutable_spec()->set_pred_flags(rtcp_proxy_cb.pred_flags);
    rsp->mutable_spec()->set_debug_dol_tblsetaddr(rtcp_proxy_cb.debug_dol_tblsetaddr);
    rsp->mutable_spec()->set_packets_out(rtcp_proxy_cb.packets_out);
    rsp->mutable_spec()->set_rto_pi(rtcp_proxy_cb.rto_pi);
    rsp->mutable_spec()->set_retx_timer_ci(rtcp_proxy_cb.retx_timer_ci);
    rsp->mutable_spec()->set_rto_backoff(rtcp_proxy_cb.rto_backoff);
    rsp->mutable_spec()->set_cpu_id(rtcp_proxy_cb.cpu_id);

    // fill operational state of this TCP CB
    rsp->mutable_status()->set_tcpcb_handle(tcp_proxy_cb->hal_handle);

    // fill stats of this TCP CB
    rsp->mutable_stats()->set_bytes_rcvd(rtcp_proxy_cb.bytes_rcvd);
    rsp->mutable_stats()->set_pkts_rcvd(rtcp_proxy_cb.pkts_rcvd);
    rsp->mutable_stats()->set_pages_alloced(rtcp_proxy_cb.pages_alloced);
    rsp->mutable_stats()->set_desc_alloced(rtcp_proxy_cb.desc_alloced);
    rsp->mutable_stats()->set_debug_num_pkt_to_mem(rtcp_proxy_cb.debug_num_pkt_to_mem);
    rsp->mutable_stats()->set_debug_num_phv_to_mem(rtcp_proxy_cb.debug_num_phv_to_mem);
    rsp->mutable_stats()->set_bytes_acked(rtcp_proxy_cb.bytes_acked);
    rsp->mutable_stats()->set_slow_path_cnt(rtcp_proxy_cb.slow_path_cnt);
    rsp->mutable_stats()->set_serq_full_cnt(rtcp_proxy_cb.serq_full_cnt);
    rsp->mutable_stats()->set_ooo_cnt(rtcp_proxy_cb.ooo_cnt);

    rsp->mutable_stats()->set_debug_atomic_delta(rtcp_proxy_cb.debug_atomic_delta);
    rsp->mutable_stats()->set_debug_atomic0_incr1247(rtcp_proxy_cb.debug_atomic0_incr1247);
    rsp->mutable_stats()->set_debug_atomic1_incr247(rtcp_proxy_cb.debug_atomic1_incr247);
    rsp->mutable_stats()->set_debug_atomic2_incr47(rtcp_proxy_cb.debug_atomic2_incr47);
    rsp->mutable_stats()->set_debug_atomic3_incr47(rtcp_proxy_cb.debug_atomic3_incr47);
    rsp->mutable_stats()->set_debug_atomic4_incr7(rtcp_proxy_cb.debug_atomic4_incr7);
    rsp->mutable_stats()->set_debug_atomic5_incr7(rtcp_proxy_cb.debug_atomic5_incr7);
    rsp->mutable_stats()->set_debug_atomic6_incr7(rtcp_proxy_cb.debug_atomic6_incr7);

    rsp->mutable_stats()->set_bytes_sent(rtcp_proxy_cb.bytes_sent);
    rsp->mutable_stats()->set_pkts_sent(rtcp_proxy_cb.pkts_sent);
    rsp->mutable_stats()->set_debug_num_phv_to_pkt(rtcp_proxy_cb.debug_num_phv_to_pkt);
    rsp->mutable_stats()->set_debug_num_mem_to_pkt(rtcp_proxy_cb.debug_num_mem_to_pkt);

    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a TCP CB delete request
//------------------------------------------------------------------------------
hal_ret_t
tcp_proxy_cb_delete (tcp_proxy::TcpProxyCbDeleteRequest& req, tcp_proxy::TcpProxyCbDeleteResponseMsg *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    tcp_proxy_cb_t*               tcp_proxy_cb;
    pd::pd_tcp_proxy_cb_delete_args_t    pd_tcp_proxy_cb_args;
    pd::pd_func_args_t          pd_func_args = {0};

    auto kh = req.key_or_handle();
    tcp_proxy_cb = find_tcp_proxy_cb_by_id(kh.tcpcb_id());
    if (tcp_proxy_cb == NULL) {
        rsp->add_api_status(types::API_STATUS_OK);
        return HAL_RET_OK;
    }

    pd::pd_tcp_proxy_cb_delete_args_init(&pd_tcp_proxy_cb_args);
    pd_tcp_proxy_cb_args.tcp_proxy_cb = tcp_proxy_cb;

    pd_func_args.pd_tcp_proxy_cb_delete = &pd_tcp_proxy_cb_args;
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

} //namespace hal
