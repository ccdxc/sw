//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/include/tcp_common.h"
#include "nic/hal/tls/tls_api.hpp"
#include "nic/hal/src/internal/tls_proxy_cb.hpp"
#include "nic/hal/src/internal/internal.hpp"
#include "nic/sdk/asic/pd/pd.hpp"

#define TLS_DDOL_ARM_LOOP_CTRL_PKTS     16   /* Loopback control packets in ARM */

using namespace sdk::asic::pd;

namespace hal {
namespace proxy {

hal::pd::cpupkt_ctxt_t* asesq_ctx = NULL;

fte::pipeline_action_t
tls_exec(fte::ctx_t& ctx)
{
    hal_ret_t ret;
    const fte::cpu_rxhdr_t* cpu_rxhdr = ctx.cpu_rxhdr();
    uint8_t *data = ctx.pkt();
    uint32_t datalen = ctx.pkt_len();
    TlsCbGetRequest     get_req;
    TlsCbGetResponse    get_resp;
    TlsCbGetResponseMsg resp_msg;
    hal::pd::pd_func_args_t pd_func_args = {0};

    // Give the data to SSL/TLS library
    ret = hal::tls::tls_api_data_receive(cpu_rxhdr->qid, data, datalen);
    if(ret == HAL_RET_ASYNC) {
        HAL_TRACE_DEBUG("Async operation in progress. Skipping");
        return fte::PIPELINE_CONTINUE;
    } else if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tls-proxy: failed to process tls packet: {}", ret);
    }

    // get tlscb
    get_req.mutable_key_or_handle()->set_tlscb_id(cpu_rxhdr->qid);
    ret = tlscb_get(get_req, &resp_msg);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("tls-proxy: tlscb for id not found: {}", cpu_rxhdr->qid);
	    return fte::PIPELINE_CONTINUE;
    }
    get_resp = resp_msg.response(0);

    HAL_TRACE_DEBUG("tls-proxy: received debug_dol: {:#x}", get_resp.spec().debug_dol());
    if(get_resp.spec().debug_dol() & TLS_DDOL_ARM_LOOP_CTRL_PKTS) {
        HAL_TRACE_DEBUG("tls-proxy: debug_dol to loop ctrl pkacet set. Transmit the packet");
        if (asesq_ctx == NULL)   {
            hal::pd::pd_cpupkt_ctxt_alloc_init_args_t args;
            pd_func_args.pd_cpupkt_ctxt_alloc_init = &args;
            hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_ALLOC_INIT, &pd_func_args);
            asesq_ctx = args.ctxt;
            // asesq_ctx = hal::pd::cpupkt_ctxt_alloc_init();
            SDK_ASSERT_RETURN(asesq_ctx != NULL, fte::PIPELINE_CONTINUE);
        }

        hal::pd::pd_cpupkt_register_tx_queue_args_t t_args;
        t_args.ctxt = asesq_ctx;
        t_args.type = types::WRING_TYPE_ASESQ;
        t_args.queue_id = cpu_rxhdr->qid;
        pd_func_args.pd_cpupkt_register_tx_queue = &t_args;
        ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_REG_TXQ, &pd_func_args);
#if 0
        ret = hal::pd::cpupkt_register_tx_queue(asesq_ctx,
                                                types::WRING_TYPE_ASESQ,
                                                cpu_rxhdr->qid);
#endif
        SDK_ASSERT_RETURN(ret == HAL_RET_OK, fte::PIPELINE_CONTINUE);

        hal::pd::pd_cpupkt_send_args_t s_args;
        s_args.ctxt = asesq_ctx;
        s_args.type = types::WRING_TYPE_ASESQ;
        s_args.queue_id = cpu_rxhdr->qid;
        s_args.cpu_header = NULL;
        s_args.p4_header = NULL;
        s_args.data = data;
        s_args.data_len = datalen;
        s_args.dest_lif = SERVICE_LIF_TCP_PROXY;
        s_args.qtype = 0;
        s_args.qid = cpu_rxhdr->qid;
        s_args.ring_number = TCP_SCHED_RING_ASESQ;
        pd_func_args.pd_cpupkt_send = &s_args;
        hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_SEND, &pd_func_args);
#if 0
        hal::pd::cpupkt_send(asesq_ctx,
                            types::WRING_TYPE_ASESQ,
                            cpu_rxhdr->qid,
                            NULL,
                            NULL,
                            data,
                            datalen,
                            SERVICE_LIF_TCP_PROXY,
                            0,
                            cpu_rxhdr->qid,
                            TCP_SCHED_RING_ASESQ);
#endif
    }
	return fte::PIPELINE_CONTINUE;
}

hal_ret_t
tls_poll_asym_pend_req_q(void)
{
    sdk_ret_t      sdk_ret;
    hal_ret_t      ret = HAL_RET_OK;
    uint32_t       batch_size = 1;
    uint32_t       qid_count = 0;
    uint32_t       qid[batch_size] = {0};

    sdk_ret = asicpd_barco_asym_poll_pend_req(batch_size, &qid_count, qid);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to poll barco pending queue: {}", ret);
        return ret;
    }
    for(uint32_t i = 0; i < qid_count; i++) {
        HAL_TRACE_DEBUG("Received status for qid {}", qid[i]);
        ret = hal::tls::tls_api_process_hw_oper_done(qid[i]);
        if(ret != HAL_RET_OK && ret != HAL_RET_ASYNC) {
            HAL_TRACE_ERR("Failed to process hw operation completion: {}", ret);
            return ret;
        }
    }
    return HAL_RET_OK;
}

hal_ret_t
tls_proxy_cfg_rule_action(tcp_proxy_tls_cfg_t *tls_cfg, proxy_flow_info_t *pfi)
{

    pfi->u.tlsproxy.cert_id = tls_cfg->cert_id;
    HAL_TRACE_DEBUG("Received ciphers: {}", tls_cfg->ciphers);
    if(tls_cfg->ciphers.length() > 0) {
        pfi->u.tlsproxy.ciphers = tls_cfg->ciphers;
    }
    pfi->u.tlsproxy.tls_proxy_side = tls_cfg->tls_proxy_side;
    pfi->u.tlsproxy.key_type = tls_cfg->asym_key_type;
    switch(tls_cfg->asym_key_type) {
    case types::CRYPTO_ASYM_KEY_TYPE_ECDSA:
        pfi->u.tlsproxy.u.ecdsa_keys.sign_key_id =
            tls_cfg->u.ecdsa_key.sign_key_idx;
        break;
    case types::CRYPTO_ASYM_KEY_TYPE_RSA:
        pfi->u.tlsproxy.u.rsa_keys.sign_key_id =
            tls_cfg->u.rsa_key.sign_key_idx;
        pfi->u.tlsproxy.u.rsa_keys.decrypt_key_id =
            tls_cfg->u.rsa_key.decrypt_key_idx;
        break;
    default:
        HAL_TRACE_ERR("Unknown key type: {}", tls_cfg->asym_key_type);
    }
    pfi->u.tlsproxy.is_valid = true;

    HAL_TRACE_DEBUG("TLS proxy config for qid: {}, cert: {}",
                    pfi->qid1,
                    pfi->u.tlsproxy.cert_id);

    return HAL_RET_OK;
}

} // namespace hal
} // namespace net
