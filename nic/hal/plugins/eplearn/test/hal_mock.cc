#include "nic/include/base.h"
#include "nic/include/fte_ctx.hpp"

class flow_t;
class phv_t;

hal::ep_t *temp_ep;
hal::vrf_t *temp_ten;
hal::ep_t *temp_dep;
hal_ret_t fte::ctx_t::init(cpu_rxhdr_t *cpu_rxhdr, uint8_t *pkt, size_t pkt_len,
                           flow_t iflow[], flow_t rflow[],
                           uint8_t *feature_state, size_t feature_state_size) {
    this->pkt_ = pkt;
    this->pkt_len_ = pkt_len;
    this->cpu_rxhdr_ = cpu_rxhdr;
    this->sep_ = temp_ep;
    this->dep_ = temp_dep;
    this->vrf_ = temp_ten;
    this->vlan_tag_valid_ = false;
    return HAL_RET_OK;
}

void fte_ctx_init(fte::ctx_t &ctx, hal::vrf_t *ten, hal::ep_t *ep,
        hal::ep_t *dep, fte::cpu_rxhdr_t *cpu_rxhdr,
        uint8_t *pkt, size_t pkt_len,
        fte::flow_t iflow[], fte::flow_t rflow[])
{
    temp_ep = ep;
    temp_ten = ten;
    temp_dep = dep;
    ctx.init(cpu_rxhdr, pkt, pkt_len, iflow, rflow, NULL, 0);
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
