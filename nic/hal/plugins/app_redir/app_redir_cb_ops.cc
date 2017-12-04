#include "app_redir.hpp"
#include "app_redir_cb_ops.hpp"

namespace hal {
namespace app_redir {

/*
 * Build a raw redirect control block for interfacing with rawrcb_create().
 */     
static void
app_redir_rawrcb_spec_build(const rawrcb_t& rawrcb,
                            RawrCbSpec& spec)
{
    spec.mutable_key_or_handle()->set_rawrcb_id(rawrcb.cb_id);

    /*
     * For a given flow, one of 2 types of redirection applies:
     *   1) Redirect to an RxQ, or
     *   2) Redirect to a P4+ TxQ
     */
    if (rawrcb.chain_txq_base) {
        assert(!rawrcb.chain_rxq_base);

        spec.set_chain_txq_base(rawrcb.chain_txq_base);
        spec.set_chain_txq_ring_indices_addr(rawrcb.chain_txq_ring_indices_addr);
        spec.set_chain_txq_ring_size_shift(rawrcb.chain_txq_ring_size_shift);
        spec.set_chain_txq_entry_size_shift(rawrcb.chain_txq_entry_size_shift);
        spec.set_chain_txq_ring_index_select(rawrcb.chain_txq_ring_index_select);
        spec.set_chain_txq_lif(rawrcb.chain_txq_lif);
        spec.set_chain_txq_qtype(rawrcb.chain_txq_qtype);
        spec.set_chain_txq_qid(rawrcb.chain_txq_qid);

    } else {
        spec.set_chain_rxq_base(rawrcb.chain_rxq_base);
        spec.set_chain_rxq_ring_indices_addr(rawrcb.chain_rxq_ring_indices_addr);
        spec.set_chain_rxq_ring_size_shift(rawrcb.chain_rxq_ring_size_shift);
        spec.set_chain_rxq_entry_size_shift(rawrcb.chain_rxq_entry_size_shift);
        spec.set_chain_rxq_ring_index_select(rawrcb.chain_rxq_ring_index_select);
    }

    spec.set_rawrcb_flags(rawrcb.rawrcb_flags);
}


/*
 * Wrapper for creating a raw redirect control block.
 */     
hal_ret_t
app_redir_rawrcb_create(const rawrcb_t& rawrcb)
{
    RawrCbSpec      spec;
    RawrCbResponse  rsp;
    hal_ret_t       ret;

    HAL_TRACE_DEBUG("{} for cb_id {}", __FUNCTION__, rawrcb.cb_id);
    app_redir_rawrcb_spec_build(rawrcb, spec);

    ret = rawrcb_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{} failed for cb_id {} ret {} rsp: ",
                      __FUNCTION__, rawrcb.cb_id, ret, rsp.api_status());
    }

    return ret;
}


/*
 * Wrapper for updating a raw redirect control block.
 */     
hal_ret_t
app_redir_rawrcb_update(const rawrcb_t& rawrcb)
{
    RawrCbSpec      spec;
    RawrCbResponse  rsp;
    hal_ret_t       ret;

    HAL_TRACE_DEBUG("{} for cb_id {}", __FUNCTION__, rawrcb.cb_id);
    app_redir_rawrcb_spec_build(rawrcb, spec);

    ret = rawrcb_update(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{} failed for cb_id {} ret {} rsp: ",
                      __FUNCTION__, rawrcb.cb_id, ret, rsp.api_status());
    }

    return ret;
}


/*
 * Build a raw chain control block for interfacing with rawccb_create().
 */     
static void
app_redir_rawccb_spec_build(const rawccb_t& rawccb,
                            RawcCbSpec& spec)
{
    spec.mutable_key_or_handle()->set_rawccb_id(rawccb.cb_id);

    spec.set_my_txq_base(rawccb.my_txq_base);
    spec.set_my_txq_ring_size_shift(rawccb.my_txq_ring_size_shift);
    spec.set_my_txq_entry_size_shift(rawccb.my_txq_entry_size_shift);

    spec.set_chain_txq_base(rawccb.chain_txq_base);
    spec.set_chain_txq_ring_indices_addr(rawccb.chain_txq_ring_indices_addr);
    spec.set_chain_txq_ring_size_shift(rawccb.chain_txq_ring_size_shift);
    spec.set_chain_txq_entry_size_shift(rawccb.chain_txq_entry_size_shift);
    spec.set_chain_txq_lif(rawccb.chain_txq_lif);
    spec.set_chain_txq_qtype(rawccb.chain_txq_qtype);
    spec.set_chain_txq_qid(rawccb.chain_txq_qid);
    spec.set_chain_txq_ring(rawccb.chain_txq_ring);

    spec.set_rawccb_flags(rawccb.rawccb_flags);
}


/*
 * Wrapper for creating a raw chain control block.
 */     
hal_ret_t
app_redir_rawccb_create(const rawccb_t& rawccb)
{
    RawcCbSpec      spec;
    RawcCbResponse  rsp;
    hal_ret_t       ret;

    HAL_TRACE_DEBUG("{} for cb_id {}", __FUNCTION__, rawccb.cb_id);
    app_redir_rawccb_spec_build(rawccb, spec);

    ret = rawccb_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{} failed for cb_id {} ret {} rsp: ",
                      __FUNCTION__, rawccb.cb_id, ret, rsp.api_status());
    }

    return ret;
}


/*
 * Wrapper for updating a raw chain control block.
 */     
hal_ret_t
app_redir_rawccb_update(const rawccb_t& rawccb)
{
    RawcCbSpec      spec;
    RawcCbResponse  rsp;
    hal_ret_t       ret;

    HAL_TRACE_DEBUG("{} for cb_id {}", __FUNCTION__, rawccb.cb_id);
    app_redir_rawccb_spec_build(rawccb, spec);

    ret = rawccb_update(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{} failed for cb_id {} ret {} rsp: ",
                      __FUNCTION__, rawccb.cb_id, ret, rsp.api_status());
    }

    return ret;
}


/*
 * Build a proxy redirect control block for interfacing with proxyrcb_create().
 */     
static void
app_redir_proxyrcb_spec_build(const proxyrcb_t& proxyrcb,
                              ProxyrCbSpec& spec)
{
    spec.mutable_key_or_handle()->set_proxyrcb_id(proxyrcb.cb_id);

    spec.set_my_txq_base(proxyrcb.my_txq_base);
    spec.set_my_txq_ring_size_shift(proxyrcb.my_txq_ring_size_shift);
    spec.set_my_txq_entry_size_shift(proxyrcb.my_txq_entry_size_shift);

    spec.set_chain_rxq_base(proxyrcb.chain_rxq_base);
    spec.set_chain_rxq_ring_indices_addr(proxyrcb.chain_rxq_ring_indices_addr);
    spec.set_chain_rxq_ring_size_shift(proxyrcb.chain_rxq_ring_size_shift);
    spec.set_chain_rxq_entry_size_shift(proxyrcb.chain_rxq_entry_size_shift);
    spec.set_chain_rxq_ring_index_select(proxyrcb.chain_rxq_ring_index_select);
    spec.set_proxyrcb_flags(proxyrcb.proxyrcb_flags);

    if (proxyrcb.af == AF_INET) {
        spec.mutable_ip_sa()->set_v4_addr(proxyrcb.ip_sa.v4_addr);
        spec.mutable_ip_da()->set_v4_addr(proxyrcb.ip_da.v4_addr);
    } else {
        spec.mutable_ip_sa()->set_v6_addr(&proxyrcb.ip_sa.v6_addr,
                                          IP6_ADDR8_LEN);
        spec.mutable_ip_da()->set_v6_addr(&proxyrcb.ip_da.v6_addr,
                                          IP6_ADDR8_LEN);
    }

    spec.set_sport(proxyrcb.sport);
    spec.set_dport(proxyrcb.dport);
    spec.set_vrf(proxyrcb.vrf);
    spec.set_af(proxyrcb.af);
    spec.set_ip_proto(types::IPProtocol(proxyrcb.ip_proto));
}


/*
 * Incorporate flow_key to the argument proxy redirect control block.
 * The control block itself isn't programmed to HW until 
 * app_redir_proxyrcb_create() or app_redir_proxyrcb_create() is called.
 */     
hal_ret_t
app_redir_proxyrcb_flow_key_build(const flow_key_t& flow_key,
                                  proxyrcb_t& proxyrcb)
{
    /*
     * Currently the only type of proxy supported by app redir
     * is TCP/TLS proxy.
     */
    HAL_TRACE_DEBUG("{} for cb_id {}", __FUNCTION__, proxyrcb.cb_id);
    assert((flow_key.proto == IPPROTO_TCP) &&
           ((flow_key.flow_type == hal::FLOW_TYPE_V4) ||
            (flow_key.flow_type == hal::FLOW_TYPE_V6)));

    proxyrcb.vrf = htons(flow_key.vrf_id);
    proxyrcb.ip_proto = IPPROTO_TCP;
    proxyrcb.sport = htons(flow_key.sport);
    proxyrcb.dport = htons(flow_key.dport);
    if (flow_key.flow_type == hal::FLOW_TYPE_V4) {
        proxyrcb.af = AF_INET;
        proxyrcb.ip_sa.v4_addr = htonl(flow_key.sip.v4_addr);
        proxyrcb.ip_da.v4_addr = htonl(flow_key.dip.v4_addr);
    } else {
        proxyrcb.af = AF_INET6;
        memcpy(&proxyrcb.ip_sa.v6_addr, &flow_key.sip.v6_addr,
               sizeof(proxyrcb.ip_sa.v6_addr));
        memcpy(&proxyrcb.ip_da.v6_addr, &flow_key.dip.v6_addr,
               sizeof(proxyrcb.ip_da.v6_addr));
    }

    return HAL_RET_OK;
}


/*
 * Wrapper for creating a proxy redirect control block.
 */     
hal_ret_t
app_redir_proxyrcb_create(const proxyrcb_t& proxyrcb)
{
    ProxyrCbSpec      spec;
    ProxyrCbResponse  rsp;
    hal_ret_t         ret;

    HAL_TRACE_DEBUG("{} for cb_id {}", __FUNCTION__, proxyrcb.cb_id);
    app_redir_proxyrcb_spec_build(proxyrcb, spec);

    ret = proxyrcb_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{} failed for cb_id {} ret {} rsp: ",
                      __FUNCTION__, proxyrcb.cb_id, ret, rsp.api_status());
    }

    return ret;
}


/*
 * Wrapper for updating a proxy redirect control block.
 */     
hal_ret_t
app_redir_proxyrcb_update(const proxyrcb_t& proxyrcb)
{
    ProxyrCbSpec      spec;
    ProxyrCbResponse  rsp;
    hal_ret_t         ret;

    HAL_TRACE_DEBUG("{} for cb_id {}", __FUNCTION__, proxyrcb.cb_id);
    app_redir_proxyrcb_spec_build(proxyrcb, spec);

    ret = proxyrcb_update(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{} failed for cb_id {} ret {} rsp: ",
                      __FUNCTION__, proxyrcb.cb_id, ret, rsp.api_status());
    }

    return ret;
}


/*
 * Build a proxy chain control block for interfacing with proxyccb_create().
 */     
static void
app_redir_proxyccb_spec_build(const proxyccb_t& proxyccb,
                              ProxycCbSpec& spec)
{
    spec.mutable_key_or_handle()->set_proxyccb_id(proxyccb.cb_id);

    spec.set_my_txq_base(proxyccb.my_txq_base);
    spec.set_my_txq_ring_size_shift(proxyccb.my_txq_ring_size_shift);
    spec.set_my_txq_entry_size_shift(proxyccb.my_txq_entry_size_shift);

    spec.set_chain_txq_base(proxyccb.chain_txq_base);
    spec.set_chain_txq_ring_indices_addr(proxyccb.chain_txq_ring_indices_addr);
    spec.set_chain_txq_ring_size_shift(proxyccb.chain_txq_ring_size_shift);
    spec.set_chain_txq_entry_size_shift(proxyccb.chain_txq_entry_size_shift);
    spec.set_chain_txq_lif(proxyccb.chain_txq_lif);
    spec.set_chain_txq_qtype(proxyccb.chain_txq_qtype);
    spec.set_chain_txq_qid(proxyccb.chain_txq_qid);
    spec.set_chain_txq_ring(proxyccb.chain_txq_ring);

    spec.set_proxyccb_flags(proxyccb.proxyccb_flags);
}


/*
 * Incorporate next service chain's TxQ info to the argument proxy chain
 * control block. The control block itself isn't programmed to HW until
 * app_redir_proxyccb_create() or app_redir_proxyccb_create() is called.
 */     
hal_ret_t
app_redir_proxyccb_chain_txq_build(const flow_key_t& flow_key,
                                   proxyccb_t& proxyccb)
{
    HAL_TRACE_DEBUG("{} for cb_id {}", __FUNCTION__, proxyccb.cb_id);

    proxyccb.chain_txq_lif = htons(app_redir_proxyc_chain_lif_eval(flow_key));

    /*
     * When chain_txq_base is left at zero, PD will automatically fill
     * in appropriate values for chain_txq_base, chain_txq_ring_size_shift,
     * chain_txq_entry_size_shift, and chain_txq_ring_indices_addr based on
     * {chain_txq_lif, chain_txq_qtype, chain_txq_qid}
     */
    proxyccb.chain_txq_base  = 0;
    proxyccb.chain_txq_qtype = 0;
    proxyccb.chain_txq_ring  = 0;
    proxyccb.chain_txq_qid = htonl(proxyccb.cb_id & PROXYCCB_NUM_ENTRIES_MASK);

    return HAL_RET_OK;
}


/*
 * Wrapper for creating a proxy chain control block.
 */     
hal_ret_t
app_redir_proxyccb_create(const proxyccb_t& proxyccb)
{
    ProxycCbSpec      spec;
    ProxycCbResponse  rsp;
    hal_ret_t         ret;

    HAL_TRACE_DEBUG("{} for cb_id {}", __FUNCTION__, proxyccb.cb_id);
    app_redir_proxyccb_spec_build(proxyccb, spec);

    ret = proxyccb_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{} failed for cb_id {} ret {} rsp: ",
                      __FUNCTION__, proxyccb.cb_id, ret, rsp.api_status());
    }

    return ret;
}


/*
 * Wrapper for updating a proxy chain control block.
 */     
hal_ret_t
app_redir_proxyccb_update(const proxyccb_t& proxyccb)
{
    ProxycCbSpec      spec;
    ProxycCbResponse  rsp;
    hal_ret_t         ret;

    HAL_TRACE_DEBUG("{} for cb_id {}", __FUNCTION__, proxyccb.cb_id);
    app_redir_proxyccb_spec_build(proxyccb, spec);

    ret = proxyccb_update(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{} failed for cb_id {} ret {} rsp: ",
                      __FUNCTION__, proxyccb.cb_id, ret, rsp.api_status());
    }

    return ret;
}


} // namespace app_redir
} // namespace hal
