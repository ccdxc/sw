//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/iris/include/hal_state.hpp"
#include "app_redir.hpp"
#include "app_redir_cb_ops.hpp"
#include "nic/hal/src/internal/internal.hpp"

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

    spec.set_ascq_base(rawrcb.ascq_base);
    spec.set_ascq_sem_inf_addr(rawrcb.ascq_sem_inf_addr);
    spec.set_rawrcb_flags(rawrcb.rawrcb_flags);
    spec.set_cpu_id(rawrcb.cpu_id);
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
        HAL_TRACE_ERR("{} failed for cb_id {} ret {} rsp: {}",
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
        HAL_TRACE_ERR("{} failed for cb_id {} ret {} rsp: {}",
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
    spec.set_ascq_base(rawccb.ascq_base);
    spec.set_ascq_sem_inf_addr(rawccb.ascq_sem_inf_addr);
    spec.set_chain_txq_ring_size_shift(rawccb.chain_txq_ring_size_shift);
    spec.set_chain_txq_entry_size_shift(rawccb.chain_txq_entry_size_shift);
    spec.set_chain_txq_lif(rawccb.chain_txq_lif);
    spec.set_chain_txq_qtype(rawccb.chain_txq_qtype);
    spec.set_chain_txq_qid(rawccb.chain_txq_qid);
    spec.set_chain_txq_ring(rawccb.chain_txq_ring);
    spec.set_cpu_id(rawccb.cpu_id);

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
        HAL_TRACE_ERR("{} failed for cb_id {} ret {} rsp: {}",
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
        HAL_TRACE_ERR("{} failed for cb_id {} ret {} rsp: {}",
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
    spec.set_dir(proxyrcb.dir);
    spec.set_role(proxyrcb.role);
    spec.set_rev_cb_id(proxyrcb.rev_cb_id);
    spec.set_redir_span(proxyrcb.redir_span);

    if (proxyrcb.af == AF_INET) {
        spec.mutable_ip_sa()->set_ip_af(types::IP_AF_INET);
        spec.mutable_ip_sa()->set_v4_addr(proxyrcb.ip_sa.v4_addr);
        spec.mutable_ip_da()->set_ip_af(types::IP_AF_INET);
        spec.mutable_ip_da()->set_v4_addr(proxyrcb.ip_da.v4_addr);
    } else {
        spec.mutable_ip_sa()->set_ip_af(types::IP_AF_INET6);
        spec.mutable_ip_sa()->set_v6_addr(&proxyrcb.ip_sa.v6_addr,
                                          IP6_ADDR8_LEN);
        spec.mutable_ip_da()->set_ip_af(types::IP_AF_INET6);
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
 * app_redir_proxyrcb_create() is called.
 */
hal_ret_t
app_redir_proxyrcb_flow_key_build(proxyrcb_t& proxyrcb,
                                  const flow_key_t& flow_key)
{
    /*
     * Currently the only type of proxy supported by app redir
     * is TCP/TLS proxy.
     */
    HAL_TRACE_DEBUG("{} for cb_id {}", __FUNCTION__, proxyrcb.cb_id);
    assert((flow_key.proto == IPPROTO_TCP) &&
           ((flow_key.flow_type == hal::FLOW_TYPE_V4) ||
            (flow_key.flow_type == hal::FLOW_TYPE_V6)));

    proxyrcb.vrf = htons(flow_key.svrf_id);
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
        HAL_TRACE_ERR("{} failed for cb_id {} ret {} rsp: {}",
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
        HAL_TRACE_ERR("{} failed for cb_id {} ret {} rsp: {}",
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
    spec.set_redir_span(proxyccb.redir_span);

    spec.set_proxyccb_flags(proxyccb.proxyccb_flags);
}


/*
 * Incorporate next service chain's TxQ info to the argument proxy chain
 * control block. The control block itself isn't programmed to HW until
 * app_redir_proxyccb_create() is called.
 */
hal_ret_t
app_redir_proxyccb_chain_txq_build(proxyccb_t& proxyccb,
                                   uint16_t chain_txq_lif,
                                   uint8_t chain_txq_qtype,
                                   uint32_t chain_txq_qid,
                                   uint8_t chain_txq_ring)
{
    HAL_TRACE_DEBUG("{} for cb_id {}", __FUNCTION__, proxyccb.cb_id);

    /*
     * When chain_txq_base is left at zero, PD will automatically fill
     * in appropriate values for chain_txq_base, chain_txq_ring_size_shift,
     * chain_txq_entry_size_shift, and chain_txq_ring_indices_addr based on
     * {chain_txq_lif, chain_txq_qtype, chain_txq_qid}
     */
    proxyccb.chain_txq_base  = 0;
    proxyccb.chain_txq_lif   = chain_txq_lif;
    proxyccb.chain_txq_qtype = chain_txq_qtype;
    proxyccb.chain_txq_qid   = chain_txq_qid;
    proxyccb.chain_txq_ring  = chain_txq_ring;

    /*
     * TCP proxy (but not TLS proxy) expects to receive descriptor that has
     * already been adjusted to point to the beginning of the AOL area.
     */
    if (chain_txq_lif == SERVICE_LIF_TCP_PROXY) {
        proxyccb.proxyccb_flags |= APP_REDIR_CHAIN_DESC_ADD_AOL_OFFSET;
    }
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
        HAL_TRACE_ERR("{} failed for cb_id {} ret {} rsp: {}",
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
        HAL_TRACE_ERR("{} failed for cb_id {} ret {} rsp: {}",
                      __FUNCTION__, proxyccb.cb_id, ret, rsp.api_status());
    }

    return ret;
}


/*
 * Build a mirror session spec for interfacing with mirror_session_create()
 */
static void
app_redir_mirror_session_spec_build(MirrorSessionSpec& spec)
{
    if_id_t     app_redir_if_id = get_app_redir_if_id();

    HAL_TRACE_DEBUG("{} session {} for if_id {}", __FUNCTION__,
                    MIRROR_SESSION_APP_REDIR_VISIB_ID, app_redir_if_id);
    spec.mutable_key_or_handle()->set_mirrorsession_id(MIRROR_SESSION_APP_REDIR_VISIB_ID);
    spec.mutable_local_span_if()->set_interface_id(app_redir_if_id);
    spec.set_snaplen(0);
}


/*
 * Wrapper for creating a mirror session for app redirect
 */
hal_ret_t
app_redir_mirror_session_create(mirror_session_id_t &ret_id)
{
    MirrorSessionSpec       spec;
    MirrorSessionResponse   rsp;
    hal_ret_t               ret;

    app_redir_mirror_session_spec_build(spec);
    ret_id = spec.key_or_handle().mirrorsession_id();

    ret = mirror_session_create(spec, &rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("{} failed ret {} rsp: {}",
                      __FUNCTION__, ret, rsp.api_status());
    }

    return ret;
}


/*
 * Ingress replication is the alternative when mirror session is not
 * used for visibility mode.
 */
hal_ret_t
app_redir_ing_replication_create(oif_list_id_t &ret_id)
{
    if_id_t     app_redir_if_id = get_app_redir_if_id();
    l2seg_t     l2seg = {0};
    oif_t       oif = {};
    hal_ret_t   ret;

    ret = oif_list_create(&ret_id);
    if (ret == HAL_RET_OK) {
        ret = oif_list_set_honor_ingress(ret_id);
    }

    if (ret == HAL_RET_OK) {
        oif.intf = find_if_by_id(app_redir_if_id);

        /*
         * Fields qid, l2seg and purpose are ignored by oif_list_add_oif()
         * when i/f is of type IF_TYPE_APP_REDIR
         */
        oif.qid = 0;
        oif.l2seg = &l2seg;
        oif.purpose = intf::LIF_QUEUE_PURPOSE_NONE;
        ret = oif_list_add_oif(ret_id, &oif);
    }

    if (ret == HAL_RET_OK) {
        HAL_TRACE_DEBUG("{} successful for if_id {} list_id {}", __FUNCTION__,
                        app_redir_if_id, ret_id);
    } else {
        HAL_TRACE_ERR("{} failed for if_id {}", __FUNCTION__, app_redir_if_id);
    }
    return ret;
}


} // namespace app_redir
} // namespace hal
