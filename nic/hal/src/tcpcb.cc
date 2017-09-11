#include <base.h>
#include <hal.hpp>
#include <hal_lock.hpp>
#include <hal_state.hpp>
#include <tcpcb.hpp>
#include <tcpcb_svc.hpp>
#include <tenant.hpp>
#include <pd_api.hpp>

namespace hal {
void *
tcpcb_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((tcpcb_t *)entry)->cb_id);
}

uint32_t
tcpcb_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(tcpcb_id_t)) % ht_size;
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
    return utils::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
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
    g_hal_state->tcpcb_hal_handle_ht()->insert(tcpcb,
                                               &tcpcb->hal_handle_ht_ctxt);
    g_hal_state->tcpcb_id_ht()->insert(tcpcb, &tcpcb->ht_ctxt);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a TCP CB create request
// TODO: if TCP CB exists, treat this as modify (tenant id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
tcpcb_create (TcpCbSpec& spec, TcpCbResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    tcpcb_t                *tcpcb;
    pd::pd_tcpcb_args_t    pd_tcpcb_args;

    // validate the request message
    ret = validate_tcpcb_create(spec, rsp);
    
    // instantiate TCP CB
    tcpcb = tcpcb_alloc_init();
    if (tcpcb == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }

    tcpcb->cb_id = spec.key_or_handle().tcpcb_id();
    tcpcb->rcv_nxt = spec.rcv_nxt();
    tcpcb->snd_nxt = spec.snd_nxt();
    tcpcb->snd_una = spec.snd_una();
    tcpcb->rcv_tsval = spec.rcv_tsval();
    tcpcb->ts_recent = spec.ts_recent();
    tcpcb->debug_dol = spec.debug_dol();
    tcpcb->snd_wnd = spec.snd_wnd();
    tcpcb->snd_cwnd = spec.snd_cwnd();
    tcpcb->rcv_mss = spec.rcv_mss();
    
    tcpcb->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming
    pd::pd_tcpcb_args_init(&pd_tcpcb_args);
    pd_tcpcb_args.tcpcb = tcpcb;
    ret = pd::pd_tcpcb_create(&pd_tcpcb_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD TCP CB create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto cleanup;
    }

    // add this L2 segment to our db
    ret = add_tcpcb_to_db(tcpcb);
    HAL_ASSERT(ret == HAL_RET_OK);

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
    pd::pd_tcpcb_args_t    pd_tcpcb_args;

    auto kh = spec.key_or_handle();

    tcpcb = find_tcpcb_by_id(kh.tcpcb_id());
    if (tcpcb == NULL) {
        rsp->set_api_status(types::API_STATUS_TCP_CB_NOT_FOUND);
        return HAL_RET_TCP_CB_NOT_FOUND;
    }
 
    pd::pd_tcpcb_args_init(&pd_tcpcb_args);
    HAL_TRACE_DEBUG("rcv_nxt: 0x{0:x}", spec.rcv_nxt());
    tcpcb->rcv_nxt = spec.rcv_nxt();
    tcpcb->snd_nxt = spec.snd_nxt();
    tcpcb->snd_una = spec.snd_una();
    tcpcb->rcv_tsval = spec.rcv_tsval();
    tcpcb->ts_recent = spec.ts_recent();
    tcpcb->debug_dol = spec.debug_dol();
    tcpcb->snd_wnd = spec.snd_wnd();
    tcpcb->snd_cwnd = spec.snd_cwnd();
    tcpcb->rcv_mss = spec.rcv_mss();
    pd_tcpcb_args.tcpcb = tcpcb;
    
    ret = pd::pd_tcpcb_update(&pd_tcpcb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD TCPCB: Update Failed, err: ", ret);
        rsp->set_api_status(types::API_STATUS_TCP_CB_NOT_FOUND);
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
tcpcb_get (TcpCbGetRequest& req, TcpCbGetResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK; 
    tcpcb_t                rtcpcb;
    tcpcb_t*               tcpcb;
    pd::pd_tcpcb_args_t    pd_tcpcb_args;

    auto kh = req.key_or_handle();

    tcpcb = find_tcpcb_by_id(kh.tcpcb_id());
    if (tcpcb == NULL) {
        rsp->set_api_status(types::API_STATUS_TCP_CB_NOT_FOUND);
        return HAL_RET_TCP_CB_NOT_FOUND;
    }
    
    tcpcb_init(&rtcpcb);
    rtcpcb.cb_id = tcpcb->cb_id;
    pd::pd_tcpcb_args_init(&pd_tcpcb_args);
    pd_tcpcb_args.tcpcb = &rtcpcb;
    
    ret = pd::pd_tcpcb_get(&pd_tcpcb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD TCPCB: Failed to get, err: {}", ret);
        rsp->set_api_status(types::API_STATUS_TCP_CB_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    HAL_TRACE_DEBUG("cb_id: 0x{0:x}, rcv_nxt: 0x{0:x}", rtcpcb.cb_id, rtcpcb.rcv_nxt);

    // fill config spec of this TCP CB 
    rsp->mutable_spec()->mutable_key_or_handle()->set_tcpcb_id(rtcpcb.cb_id);
    rsp->mutable_spec()->set_rcv_nxt(rtcpcb.rcv_nxt);
    rsp->mutable_spec()->set_debug_dol(rtcpcb.debug_dol);
    rsp->mutable_spec()->set_serq_base(rtcpcb.serq_base);
    rsp->mutable_spec()->set_sesq_base(rtcpcb.sesq_base);
    rsp->mutable_spec()->set_sesq_pi(rtcpcb.sesq_pi);
    rsp->mutable_spec()->set_sesq_ci(rtcpcb.sesq_ci);

    // fill operational state of this TCP CB
    rsp->mutable_status()->set_tcpcb_handle(tcpcb->hal_handle);

    // fill stats of this TCP CB
    rsp->mutable_stats()->set_bytes_rcvd(rtcpcb.bytes_rcvd);
    rsp->mutable_stats()->set_pkts_rcvd(rtcpcb.pkts_rcvd);
    rsp->mutable_stats()->set_pages_alloced(rtcpcb.pages_alloced);
    rsp->mutable_stats()->set_desc_alloced(rtcpcb.desc_alloced);
    rsp->mutable_stats()->set_debug_num_pkt_to_mem(rtcpcb.debug_num_pkt_to_mem);
    rsp->mutable_stats()->set_debug_num_phv_to_mem(rtcpcb.debug_num_phv_to_mem);
    rsp->mutable_stats()->set_debug_stage0_7_thread(rtcpcb.debug_stage0_7_thread);

    rsp->mutable_stats()->set_bytes_sent(rtcpcb.bytes_sent);
    rsp->mutable_stats()->set_pkts_sent(rtcpcb.pkts_sent);
    rsp->mutable_stats()->set_debug_num_phv_to_pkt(rtcpcb.debug_num_phv_to_pkt);
    rsp->mutable_stats()->set_debug_num_mem_to_pkt(rtcpcb.debug_num_mem_to_pkt);

    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a TCP CB delete request
//------------------------------------------------------------------------------
hal_ret_t
tcpcb_delete (tcpcb::TcpCbDeleteRequest& req, tcpcb::TcpCbDeleteResponseMsg *rsp)
{
    hal_ret_t              ret = HAL_RET_OK; 
    tcpcb_t*               tcpcb;
    pd::pd_tcpcb_args_t    pd_tcpcb_args;

    auto kh = req.key_or_handle();
    tcpcb = find_tcpcb_by_id(kh.tcpcb_id());
    if (tcpcb == NULL) {
        rsp->add_api_status(types::API_STATUS_OK);
        return HAL_RET_OK;
    }
 
    pd::pd_tcpcb_args_init(&pd_tcpcb_args);
    pd_tcpcb_args.tcpcb = tcpcb;
    
    ret = pd::pd_tcpcb_delete(&pd_tcpcb_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD TCPCB: delete Failed, err: {}", ret);
        rsp->add_api_status(types::API_STATUS_TCP_CB_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }
    
    // fill stats of this TCP CB
    rsp->add_api_status(types::API_STATUS_OK);
 
    return HAL_RET_OK;
}

}    // namespace hal
