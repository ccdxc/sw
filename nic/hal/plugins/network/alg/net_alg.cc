#include "nic/hal/plugins/network/net_plugin.hpp"
#include "nic/hal/plugins/network/alg/alg_tftp.hpp"
#include "nic/hal/plugins/network/alg/alg_rpc.hpp"
#include "nic/hal/plugins/network/alg/alg_utils.hpp"
#include "nic/hal/plugins/firewall/firewall.hpp"

namespace hal {
namespace net {

static bool
IsAlgExistingFlow(fte::alg_entry_t *alg_entry)
{
    HAL_TRACE_DEBUG("ALG Exec alg_proto_state: {}", 
                     (alg_entry != NULL)?alg_entry->alg_proto_state:fte::ALG_PROTO_STATE_NONE);

    return ((alg_entry != NULL) && 
            (alg_entry->alg_proto_state != fte::ALG_PROTO_STATE_NONE));
}

fte::pipeline_action_t
alg_exec(fte::ctx_t& ctx)
{
    hal_ret_t          ret = HAL_RET_OK;
    fte::alg_entry_t  *alg_entry = NULL;
    hal::firewall::firewall_info_t *firewall_info =
        (hal::firewall::firewall_info_t*)ctx.feature_state(hal::firewall::FTE_FEATURE_FIREWALL);

    alg_entry = (fte::alg_entry_t *)ctx.alg_entry();
    if (ctx.protobuf_request()) {
        return fte::PIPELINE_CONTINUE;
    }
   
    HAL_TRACE_DEBUG("ALG Proto: {}", firewall_info->alg_proto);
    if (firewall_info->alg_proto != nwsec::APP_SVC_NONE) {

        if (ctx.role() == hal::FLOW_ROLE_INITIATOR) 
            alg_entry = alloc_and_init_alg_entry(ctx);

        switch(firewall_info->alg_proto) {
            case nwsec::APP_SVC_TFTP:
                ret = process_tftp_first_packet(ctx);
                break;

            case nwsec::APP_SVC_SUN_RPC:
                if (alg_entry->alg_proto_state == fte::ALG_PROTO_STATE_SUNRPC_DATA) {
                    ret = process_sunrpc_data_flow(ctx);
                } else {
                    ret = process_sunrpc_control_flow(ctx); 
                }
                break;
  
            case nwsec::APP_SVC_MSFT_RPC:
                if (alg_entry->alg_proto_state == fte::ALG_PROTO_STATE_MSRPC_DATA) {
                    ret = process_msrpc_data_flow(ctx);
                } else {
                    ret = process_msrpc_control_flow(ctx);
                }
                break;
 
            case nwsec::APP_SVC_FTP:
                break;

            case nwsec::APP_SVC_DNS:
                break;

            default:
                break;
        };
    } else if (IsAlgExistingFlow(alg_entry)) {
        // Todo -- move this to callbacks from the alg_entry 
        // avoid switch-case
        switch(alg_entry->alg_proto_state) {
            case fte::ALG_PROTO_STATE_TFTP_RRQ:
            case fte::ALG_PROTO_STATE_TFTP_WRQ:
                ret = process_tftp(ctx);
                break;

            case fte::ALG_PROTO_STATE_SUNRPC_INIT:
            case fte::ALG_PROTO_STATE_RPC_GETPORT:
            case fte::ALG_PROTO_STATE_RPC_DUMP:
                ret = parse_sunrpc_control_flow(ctx);
                break;

            case fte::ALG_PROTO_STATE_MSRPC_INIT:
            case fte::ALG_PROTO_STATE_MSRPC_EPM:
            case fte::ALG_PROTO_STATE_MSRPC_BIND:
            case fte::ALG_PROTO_STATE_MSRPC_BOUND:
                ret = parse_msrpc_control_flow(ctx);
                break;

            default:
                break;
        };
    }

    ctx.set_feature_status(ret);
    
    return fte::PIPELINE_CONTINUE;
}

} // namespace net
} // namespace hal
