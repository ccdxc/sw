#include "nic/hal/src/proxy.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "nic/hal/pd/cpupkt_api.hpp"
#include "nic/include/tcp_common.h"
#include "nic/hal/tls/tls_api.hpp"
#include "nic/hal/src/tlscb.hpp"

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

    // Give the data to SSL/TLS library
    ret = hal::tls::tls_api_data_receive(cpu_rxhdr->qid, data, datalen);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("tls-proxy: failed to process tls packet: {}", ret);
    }

    // get tlscb
    get_req.mutable_key_or_handle()->set_tlscb_id(cpu_rxhdr->qid);
    ret = tlscb_get(get_req, &get_resp);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("tls-proxy: tlscb for id not found: {}", cpu_rxhdr->qid);
	    return fte::PIPELINE_CONTINUE;
    }

    HAL_TRACE_DEBUG("tls-proxy: received debug_dol: {:#x}", get_resp.spec().debug_dol());
    if(get_resp.spec().debug_dol() & TLS_DDOL_ARM_LOOP_CTRL_PKTS) {
        HAL_TRACE_DEBUG("tls-proxy: debug_dol to loop ctrl pkacet set. Transmit the packet");
        if (asesq_ctx == NULL)   {
            asesq_ctx = hal::pd::cpupkt_ctxt_alloc_init();
            HAL_ASSERT_RETURN(asesq_ctx != NULL, fte::PIPELINE_CONTINUE);
        }
        
        ret = hal::pd::cpupkt_register_tx_queue(asesq_ctx,
                                                types::WRING_TYPE_ASESQ,
                                                cpu_rxhdr->qid);
        HAL_ASSERT_RETURN(ret == HAL_RET_OK, fte::PIPELINE_CONTINUE);

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
    }
	return fte::PIPELINE_CONTINUE;
}

} // namespace hal
} // namespace net
