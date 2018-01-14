#include "nic/include/base.h"
#include "nic/include/fte_ctx.hpp"

class flow_t;
class phv_t;

hal::ep_t *temp_ep;
hal::vrf_t *temp_ten;
hal::ep_t *temp_dep;
hal_ret_t fte::ctx_t::init(cpu_rxhdr_t *cpu_rxhdr, uint8_t *pkt, size_t pkt_len,
                           flow_t iflow[], flow_t rflow[],
                           feature_state_t feature_state[], uint16_t num_features) {
    this->pkt_ = pkt;
    this->pkt_len_ = pkt_len;
    this->cpu_rxhdr_ = cpu_rxhdr;
    this->sep_ = temp_ep;
    this->dep_ = temp_dep;
    this->sep_handle_ = temp_ep->hal_handle;
    this->dep_handle_ = temp_dep ? temp_dep->hal_handle : HAL_HANDLE_INVALID;
    this->vrf_ = temp_ten;
    this->vlan_tag_valid_ = false;
    this->num_features_ = 100;
    this->feature_state_ = feature_state; //(feature_state_t *)HAL_CALLOC(hal::HAL_MEM_ALLOC_FTE, sizeof(feature_state_t));
    return HAL_RET_OK;
}

void fte_ctx_init(fte::ctx_t &ctx, hal::vrf_t *ten, hal::ep_t *ep,
        hal::ep_t *dep, fte::cpu_rxhdr_t *cpu_rxhdr,
        uint8_t *pkt, size_t pkt_len,
        fte::flow_t iflow[], fte::flow_t rflow[], fte::feature_state_t feature_state[])
{
    temp_ep = ep;
    temp_ten = ten;
    temp_dep = dep;
    ctx.init(cpu_rxhdr, pkt, pkt_len, iflow, rflow, feature_state, 0);
}

hal_ret_t fte::ctx_t::process()
{
    this->invoke_completion_handlers(false);
    return HAL_RET_OK;
}

void
fte::ctx_t::invoke_completion_handlers(bool fail)
{
    HAL_TRACE_DEBUG("fte: invoking completion handlers");
    for (int i = 0; i < num_features_; i++) {
        if (feature_state_[i].completion_handler != nullptr) {
            HAL_TRACE_DEBUG("fte: invoking completion handler {:p}",
                        (void*)(feature_state_[i].completion_handler));
            (*feature_state_[i].completion_handler)(*this, fail);
        }
    }
}

hal_ret_t fte::ctx_t::update_flow(const flow_update_t& flowupd,
                   const hal::flow_role_t role)
{
    return HAL_RET_OK;
}


hal_ret_t
mock_create_add_cb(hal::cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

hal_ret_t
mock_create_commit_cb (hal::cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

hal_ret_t
mock_create_abort_cb (hal::cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

hal_ret_t
mock_create_cleanup_cb (hal::cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}
