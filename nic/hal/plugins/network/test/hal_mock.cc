#include "nic/include/base.h"
#include "nic/include/fte_ctx.hpp"

class flow_t;
class phv_t;

hal::ep_t *temp_ep;
hal::tenant_t *temp_ten;
hal_ret_t fte::ctx_t::init(cpu_rxhdr_t *cpu_rxhdr, uint8_t *pkt, size_t pkt_len,
                           flow_t iflow[], flow_t rflow[]) {
    this->pkt_ = pkt;
    this->pkt_len_ = pkt_len;
    this->sep_ = temp_ep;
    this->tenant_ = temp_ten;
    return HAL_RET_OK;
}

void fte_ctx_init(fte::ctx_t &ctx, hal::tenant_t *ten, hal::ep_t *ep,
        fte::cpu_rxhdr_t *cpu_rxhdr, uint8_t *pkt, size_t pkt_len,
        fte::flow_t iflow[], fte::flow_t rflow[])
{
    temp_ep = ep;
    temp_ten = ten;
    ctx.init(cpu_rxhdr, pkt, pkt_len, iflow, rflow);
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
