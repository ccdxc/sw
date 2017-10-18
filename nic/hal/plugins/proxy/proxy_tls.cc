#include "nic/hal/src/proxy.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "nic/hal/pd/common/cpupkt_api.hpp"
#include "nic/include/tcp_common.h"
#include "nic/hal/tls/tls_api.hpp"

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
    
    // Give the data to SSL/TLS library
    ret = hal::tls::tls_api_data_receive(cpu_rxhdr->qid, data, datalen);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to process tls packet: {}", ret);
    }
    if (asesq_ctx == NULL)   {
        asesq_ctx = hal::pd::cpupkt_ctxt_alloc_init();
        HAL_ASSERT_RETURN(asesq_ctx != NULL, fte::PIPELINE_CONTINUE);
    }
    ret = hal::pd::cpupkt_register_tx_queue(asesq_ctx, types::WRING_TYPE_ASESQ, cpu_rxhdr->qid);
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
	return fte::PIPELINE_CONTINUE;
}

} // namespace hal
} // namespace net
