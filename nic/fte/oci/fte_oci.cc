#include "nic/fte/fte.hpp"
#include "nic/fte/fte_ctx.hpp"
#include "nic/fte/fte_flow.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "lib/list/list.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/cpupkt_headers.hpp"
#include "nic/hal/pd/cpupkt_api.hpp"
#include "nic/asm/cpu-p4plus/include/cpu-defines.h"
#include "nic/hal/plugins/app_redir/app_redir_ctx.hpp"

namespace fte {
//------------------------------------------------------------------------------
// extract flow key from packet
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::extract_flow_key()
{
    return HAL_RET_ERR;
}

//------------------------------------------------------------------------------
// Lookup teannt/ep/if/l2seg from flow lookup key
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::lookup_flow_objs()
{
    return HAL_RET_OK;
}
//-----------------------------------------------------------------------------
// Initialize context from the existing session
//-----------------------------------------------------------------------------
void
ctx_t::init_ctxt_from_session(hal::session_t *sess)
{
}

//------------------------------------------------------------------------------
// Lookup existing session for the flow
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::lookup_session()
{
    return HAL_RET_ERR;
}

//------------------------------------------------------------------------------
// Creates new seesion for the pkt's flow
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::create_session()
{
    return HAL_RET_ERR;
}

//------------------------------------------------------------------------------
// Create/update session and flow table entries in hardware
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::update_flow_table()
{
    return HAL_RET_ERR;
}

//------------------------------------------------------------------------------
// Update the dest ep/if/l2seg on dnat change
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::update_for_dnat(hal::flow_role_t role, const header_rewrite_info_t& header)
{
    return  HAL_RET_ERR;
}

//------------------------------------------------------------------------------
// Update the src key  on snat change
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::update_for_snat(hal::flow_role_t role, const header_rewrite_info_t& header)
{
    return  HAL_RET_ERR;
}
//------------------------------------------------------------------------------
// Queues pkt for transmission on ASQ at the end of pipeline processing,
// after updating the flow table
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::queue_txpkt(uint8_t *pkt, size_t pkt_len,
                   hal::pd::cpu_to_p4plus_header_t *cpu_header,
                   hal::pd::p4plus_to_p4_header_t  *p4plus_header,
                   uint16_t dest_lif, uint8_t  qtype, uint32_t qid,
                   uint8_t  ring_number, types::WRingType wring_type,
                   post_xmit_cb_t cb)
{
    return HAL_RET_ERR;
}

//------------------------------------------------------------------------------
// Sends all the queued packets
//------------------------------------------------------------------------------
hal_ret_t
ctx_t::send_queued_pkts(hal::pd::cpupkt_ctxt_t* arm_ctx)
{
    return HAL_RET_ERR;
}

//------------------------------------------------------------------------------
// Add IPC based flow logging 
//------------------------------------------------------------------------------
void ctx_t::add_flow_logging(hal::flow_key_t key, hal_handle_t sess_hdl,
                          fte_flow_log_info_t *fwlog, hal::flow_direction_t direction) 
{

}

}

namespace hal {

const char *
flowkey2str (const flow_key_t& key)
{
    return  "";
}

}
