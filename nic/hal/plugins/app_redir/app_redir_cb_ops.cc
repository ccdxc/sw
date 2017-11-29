#include "app_redir.hpp"
#include "app_redir_cb_ops.hpp"

namespace hal {
namespace app_redir {

static void
app_redir_rawrcb_spec_build(uint32_t cb_id,
                            const rawrcb_t *rawrcb,
                            RawrCbSpec& spec)
{
    spec.mutable_key_or_handle()->set_rawrcb_id(cb_id);

    /*
     * For a given flow, one of 2 types of redirection applies:
     *   1) Redirect to an RxQ, or
     *   2) Redirect to a P4+ TxQ
     */
    if (rawrcb->chain_txq_base) {
        assert(!rawrcb->chain_rxq_base);

        spec.set_chain_txq_base(rawrcb->chain_txq_base);
        spec.set_chain_txq_ring_indices_addr(rawrcb->chain_txq_ring_indices_addr);
        spec.set_chain_txq_ring_size_shift(rawrcb->chain_txq_ring_size_shift);
        spec.set_chain_txq_entry_size_shift(rawrcb->chain_txq_entry_size_shift);
        spec.set_chain_txq_ring_index_select(rawrcb->chain_txq_ring_index_select);
        spec.set_chain_txq_lif(rawrcb->chain_txq_lif);
        spec.set_chain_txq_qtype(rawrcb->chain_txq_qtype);
        spec.set_chain_txq_qid(rawrcb->chain_txq_qid);

    } else {
        spec.set_chain_rxq_base(rawrcb->chain_rxq_base);
        spec.set_chain_rxq_ring_indices_addr(rawrcb->chain_rxq_ring_indices_addr);
        spec.set_chain_rxq_ring_size_shift(rawrcb->chain_rxq_ring_size_shift);
        spec.set_chain_rxq_entry_size_shift(rawrcb->chain_rxq_entry_size_shift);
        spec.set_chain_rxq_ring_index_select(rawrcb->chain_rxq_ring_index_select);
    }

    spec.set_rawrcb_flags(rawrcb->rawrcb_flags);
}


hal_ret_t
app_redir_rawrcb_create(uint32_t cb_id,
                        const rawrcb_t *rawrcb)
{
    RawrCbSpec      spec;
    RawrCbResponse  rsp;
    hal_ret_t       ret;

    HAL_TRACE_DEBUG("{} for cb_id {}", __FUNCTION__, cb_id);
    app_redir_rawrcb_spec_build(cb_id, rawrcb, spec);

    ret = rawrcb_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{} failed for cb_id {} ret {} rsp: ",
                      __FUNCTION__, cb_id, ret, rsp.api_status());
    }

    return ret;
}


hal_ret_t
app_redir_rawrcb_update(uint32_t cb_id,
                        const rawrcb_t *rawrcb)
{
    RawrCbSpec      spec;
    RawrCbResponse  rsp;
    hal_ret_t       ret;

    HAL_TRACE_DEBUG("{} for cb_id {}", __FUNCTION__, cb_id);
    app_redir_rawrcb_spec_build(cb_id, rawrcb, spec);

    ret = rawrcb_update(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{} failed for cb_id {} ret {} rsp: ",
                      __FUNCTION__, cb_id, ret, rsp.api_status());
    }

    return ret;
}


static void
app_redir_rawccb_spec_build(uint32_t cb_id,
                            const rawccb_t *rawccb,
                            RawcCbSpec& spec)
{
    spec.mutable_key_or_handle()->set_rawccb_id(cb_id);

    spec.set_my_txq_base(rawccb->my_txq_base);
    spec.set_my_txq_ring_size_shift(rawccb->my_txq_ring_size_shift);
    spec.set_my_txq_entry_size_shift(rawccb->my_txq_entry_size_shift);

    spec.set_chain_txq_base(rawccb->chain_txq_base);
    spec.set_chain_txq_ring_indices_addr(rawccb->chain_txq_ring_indices_addr);
    spec.set_chain_txq_ring_size_shift(rawccb->chain_txq_ring_size_shift);
    spec.set_chain_txq_entry_size_shift(rawccb->chain_txq_entry_size_shift);
    spec.set_chain_txq_lif(rawccb->chain_txq_lif);
    spec.set_chain_txq_qtype(rawccb->chain_txq_qtype);
    spec.set_chain_txq_qid(rawccb->chain_txq_qid);
    spec.set_chain_txq_ring(rawccb->chain_txq_ring);

    spec.set_rawccb_flags(rawccb->rawccb_flags);
}


hal_ret_t
app_redir_rawccb_create(uint32_t cb_id,
                        const rawccb_t *rawccb)
{
    RawcCbSpec      spec;
    RawcCbResponse  rsp;
    hal_ret_t       ret;

    HAL_TRACE_DEBUG("{} for cb_id {}", __FUNCTION__, cb_id);
    app_redir_rawccb_spec_build(cb_id, rawccb, spec);

    ret = rawccb_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{} failed for cb_id {} ret {} rsp: ",
                      __FUNCTION__, cb_id, ret, rsp.api_status());
    }

    return ret;
}


hal_ret_t
app_redir_rawccb_update(uint32_t cb_id,
                        const rawccb_t *rawccb)
{
    RawcCbSpec      spec;
    RawcCbResponse  rsp;
    hal_ret_t       ret;

    HAL_TRACE_DEBUG("{} for cb_id {}", __FUNCTION__, cb_id);
    app_redir_rawccb_spec_build(cb_id, rawccb, spec);

    ret = rawccb_update(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{} failed for cb_id {} ret {} rsp: ",
                      __FUNCTION__, cb_id, ret, rsp.api_status());
    }

    return ret;
}


} // namespace app_redir
} // namespace hal
