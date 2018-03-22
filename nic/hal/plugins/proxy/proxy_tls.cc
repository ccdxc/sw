#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/include/tcp_common.h"
#include "nic/hal/tls/tls_api.hpp"
#include "nic/hal/src/internal/tls_proxy_cb.hpp"

#define TLS_DDOL_ARM_LOOP_CTRL_PKTS     16   /* Loopback control packets in ARM */

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

    // Give the data to SSL/TLS library
    ret = hal::tls::tls_api_data_receive(cpu_rxhdr->qid, data, datalen);
    if(ret != HAL_RET_OK) {
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
            hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_ALLOC_INIT, (void *)&args);
            asesq_ctx = args.ctxt;
            // asesq_ctx = hal::pd::cpupkt_ctxt_alloc_init();
            HAL_ASSERT_RETURN(asesq_ctx != NULL, fte::PIPELINE_CONTINUE);
        }
        
        hal::pd::pd_cpupkt_register_tx_queue_args_t t_args;
        t_args.ctxt = asesq_ctx;
        t_args.type = types::WRING_TYPE_ASESQ;
        t_args.queue_id = cpu_rxhdr->qid;
        ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_REG_TXQ, (void *)&t_args);
#if 0
        ret = hal::pd::cpupkt_register_tx_queue(asesq_ctx,
                                                types::WRING_TYPE_ASESQ,
                                                cpu_rxhdr->qid);
#endif
        HAL_ASSERT_RETURN(ret == HAL_RET_OK, fte::PIPELINE_CONTINUE);

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
        hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_CPU_SEND, (void *)&s_args);
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

} // namespace hal
} // namespace net
