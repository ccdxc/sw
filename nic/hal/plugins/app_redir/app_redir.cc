//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/pd/cpupkt_api.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "app_redir_plugin.hpp"
#include "app_redir.hpp"
#include "app_redir_headers.hpp"
#include "app_redir_cb_ops.hpp"

namespace hal {
namespace app_redir {

/*
 * Consideration for TCP/TLS during proxy flow creation
 */
typedef enum {
    PROXY_FLOW_TYPE_APP_REDIR,
    PROXY_FLOW_TYPE_INCLUDE_TCP
} proxy_flow_type_include_t;


static sdk_spinlock_t       mirror_create_lock;

#if APP_REDIR_VISIBILITY_USE_MIRROR_SESSION
static mirror_session_id_t  visib_mirror_session_id;
#else
static oif_list_id_t        visib_oif_list_id;
#endif

/*
 * One time initialization outside of any thread context
 */
void
app_redir_init(void)
{
    SDK_SPINLOCK_INIT(&mirror_create_lock, PTHREAD_PROCESS_PRIVATE);
}


/*
 * Evaluate and set the reverse role
 */
static inline void
app_redir_rev_role_set(fte::ctx_t& ctx)
{
    app_redir_ctx_t&   redir_ctx = app_redir_ctxref(ctx);

    redir_ctx.set_chain_rev_role(ctx.role() == hal::FLOW_ROLE_INITIATOR ?
                                 hal::FLOW_ROLE_RESPONDER :
                                 hal::FLOW_ROLE_INITIATOR);
}


/*
 * Set pipeline action
 * (pipeline_action_t not available in fte_ctx.hpp which necessitates
 * using a bool to maintain state).
 */
static inline void
app_redir_pipeline_action_set(fte::ctx_t& ctx,
                              fte::pipeline_action_t action)
{
    app_redir_ctx_t&   redir_ctx = app_redir_ctxref(ctx);

    redir_ctx.set_pipeline_end(action == fte::PIPELINE_END);
}


/*
 * Return pipeline action continue or end
 */
static inline fte::pipeline_action_t
app_redir_pipeline_action(fte::ctx_t& ctx)
{
    app_redir_ctx_t&   redir_ctx = app_redir_ctxref(ctx);

    return redir_ctx.pipeline_end() ?
           fte::PIPELINE_END : fte::PIPELINE_CONTINUE;
}


/*
 * Set feature status and return the same status.
 */
static hal_ret_t
app_redir_feature_status_set(fte::ctx_t& ctx,
                             hal_ret_t status,
                             bool pipeline_end_on_error=true)
{
    if (status != HAL_RET_OK) {
        ctx.set_feature_status(status);
        if (pipeline_end_on_error) {
            app_redir_pipeline_action_set(ctx, fte::PIPELINE_END);
        }
    }

    return status;
}


/*
 * Wrapper for app_redir_rawrcb_create()
 */
static hal_ret_t
_app_redir_rawrcb_create(fte::ctx_t& ctx,
                         uint32_t cb_id)
{
    rawrcb_t    rawrcb;

    /*
     * For rawrcb, we need to set up chain_rxq_base and its related attributes.
     * However, when both chain_rxq_base and chain_txq_base are set to zero,
     * PD will automatically supply the correct ARQ defaults for chain_rxq_base.
     * When multi cores are supported, only the chain_rxq_ring_index_select
     * would need to be specified here.
     */
    app_redir_rawrcb_init(cb_id, rawrcb);
    rawrcb.cpu_id = fte::fte_id();
    if (cb_id != APP_REDIR_SPAN_RAWRCB_ID) {
        rawrcb.chain_rxq_ring_index_select = rawrcb.cpu_id;
    }
    return app_redir_rawrcb_create(rawrcb);
}


/*
 * Wrapper for app_redir_rawccb_create()
 */
static hal_ret_t
_app_redir_rawccb_create(fte::ctx_t& ctx,
                         uint32_t cb_id)
{
    rawccb_t    rawccb;

    /*
     * For rawccb, PD will supply the correct defaults when my_txq_base is
     * left at zero.
     */
    app_redir_rawccb_init(cb_id, rawccb);
    rawccb.cpu_id = fte::fte_id();
    return app_redir_rawccb_create(rawccb);
}


/*
 * Establish service chaining by creating control blocks (cb) for
 * P4+ raw redirect programs.
 */
static hal_ret_t
app_redir_rawrcb_rawccb_create(fte::ctx_t& ctx)
{
    app_redir_ctx_t&    redir_ctx = app_redir_ctxref(ctx);
    hal_ret_t           ret = HAL_RET_OK;

    /*
     * DOL infra creates CBs thru different means so no-op here in that case.
     * Also, we will have created CBs for both initiator/responder roles
     * in the first invocation.
     */
    if (ctx.pkt() && (ctx.role() ==  hal::FLOW_ROLE_INITIATOR) &&
        (redir_ctx.chain_flow_id() != APP_REDIR_SPAN_RAWRCB_ID)) {

        ret = _app_redir_rawccb_create(ctx, redir_ctx.chain_flow_id());
        if (ret == HAL_RET_OK) {
            ret = _app_redir_rawrcb_create(ctx, redir_ctx.chain_flow_id());
        }
        if (ret == HAL_RET_OK) {
            ret = _app_redir_rawccb_create(ctx, redir_ctx.chain_rev_flow_id());
        }
        if (ret == HAL_RET_OK) {
            ret = _app_redir_rawrcb_create(ctx, redir_ctx.chain_rev_flow_id());
        }
    }

    return app_redir_feature_status_set(ctx, ret);
}


/*
 * Wrapper for app_redir_proxyrcb_create()
 */
static hal_ret_t
_app_redir_proxyrcb_create(fte::ctx_t& ctx,
                           uint32_t cb_id,
                           hal::flow_role_t role,
                           uint32_t rev_cb_id,
                           app_redir_span_type_t redir_span_type)
{
    const flow_key_t&   flow_key = ctx.get_key(role);
    proxyrcb_t          proxyrcb;
    hal_ret_t           ret;

    /*
     * For proxyrcb, we need to set up chain_rxq_base and its related
     * attributes. However, when chain_rxq_base is set to zero, PD will
     * automatically supply the correct ARQ defaults for chain_rxq_base.
     * When multi cores are supported, only the chain_rxq_ring_index_select
     * would need to be specified here.
     */
    app_redir_proxyrcb_init(cb_id, proxyrcb);
    proxyrcb.chain_rxq_ring_index_select = fte::fte_id();
    proxyrcb.role = role;
    proxyrcb.rev_cb_id = rev_cb_id;
    proxyrcb.redir_span = redir_span_type != APP_REDIR_SPAN_NONE;
    ret = app_redir_proxyrcb_flow_key_build(proxyrcb, flow_key);
    if (ret == HAL_RET_OK) {
        ret = app_redir_proxyrcb_create(proxyrcb);
    }

    return ret;
}


/*
 * Wrapper for app_redir_proxyccb_create()
 */
static hal_ret_t
_app_redir_proxyccb_create(fte::ctx_t& ctx,
                           uint32_t cb_id,
                           uint16_t chain_txq_lif,
                           uint8_t chain_txq_qtype,
                           uint32_t chain_txq_qid,
                           uint8_t chain_txq_ring,
                           app_redir_span_type_t redir_span_type)
{
    proxyccb_t  proxyccb;
    hal_ret_t   ret;

    /*
     * For proxyccb, PD will supply the correct defaults when my_txq_base
     * is left at zero, but for chain_txq info, appropriate value will be
     * constructed by app_redir_proxyccb_chain_txq_build().
     */
    app_redir_proxyccb_init(cb_id, proxyccb);
    proxyccb.redir_span = redir_span_type != APP_REDIR_SPAN_NONE;
    ret = app_redir_proxyccb_chain_txq_build(proxyccb, chain_txq_lif,
                             chain_txq_qtype, chain_txq_qid, chain_txq_ring);
    if (ret == HAL_RET_OK) {
        ret = app_redir_proxyccb_create(proxyccb);
    }

    return ret;
}


/*
 * Establish service chaining by creating control blocks (cb) for
 * P4+ TCP/TLS proxy redirect programs.
 */
static hal_ret_t
app_redir_proxyrcb_proxyccb_create(fte::ctx_t& ctx)
{
    app_redir_ctx_t&    redir_ctx = app_redir_ctxref(ctx);
    hal_ret_t           ret = HAL_RET_OK;

    /*
     * DOL infra creates CBs thru different means so no-op here in that case.
     * Also, we will have created CBs for both initiator/responder roles
     * in the first invocation (tcp_exec_trigger_connection() depends on
     * our CBs for setting its redirect configurations).
     */
    if (ctx.pkt() && (ctx.role() ==  hal::FLOW_ROLE_INITIATOR)) {
        const flow_key_t& flow_key = ctx.key();

        if (flow_key.dir == hal::FLOW_DIR_FROM_DMA) {

            /*
             * initiator host-to-network: SYN, ACK
             *   TLS: chain_flow_id will be encrypt flow,
             *        chain_rev_flow_id will be decrypt flow
             *   TCP: chain_flow_id will have reverse tuple (for sending
             *           SYNACK to to host)
             *        chain_rev_flow_id will have original tuple (for sending
             *           SYN to network)
             *   tcpcb[chain_flow_id] -> proxyrcb[chain_flow_id] -> ARQ ->
             *         proxyccb[chain_flow_id] -> tlscb[chain_flow_id] ->
             *        tcpcb[chain_rev_flow_id] -> network
             * responder network-to-host: SYNACK
             *   tcpcb[chain_rev_flow_id] -> tlscb[chain_rev_flow_id] ->
             *         proxyrcb[chain_rev_flow_id] -> ARQ ->
             *         proxyccb[chain_rev_flow_id] -> tcpcb[chain_flow_id] -> host
             */
            ret = _app_redir_proxyrcb_create(ctx, redir_ctx.chain_flow_id(),
                                             hal::FLOW_ROLE_INITIATOR,
                                             redir_ctx.chain_rev_flow_id(),
                                             redir_ctx.redir_span_type());
            if (ret == HAL_RET_OK) {
                ret = _app_redir_proxyccb_create(ctx, redir_ctx.chain_flow_id(),
                           SERVICE_LIF_TLS_PROXY, 0, redir_ctx.chain_flow_id(),
                           0, redir_ctx.redir_span_type());
            }
            if (ret == HAL_RET_OK) {
                ret = _app_redir_proxyrcb_create(ctx, redir_ctx.chain_rev_flow_id(),
                                                 hal::FLOW_ROLE_RESPONDER,
                                                 redir_ctx.chain_flow_id(),
                                                 redir_ctx.redir_span_type());
            }
            if (ret == HAL_RET_OK) {
                ret = _app_redir_proxyccb_create(ctx, redir_ctx.chain_rev_flow_id(),
                           SERVICE_LIF_TCP_PROXY, 0, redir_ctx.chain_flow_id(),
                           0, redir_ctx.redir_span_type());
            }

        } else {

            /*
             * initiator network-to-host: SYN, ACK
             *   TLS: chain_flow_id will be decrypt flow,
             *        chain_rev_flow_id will be encrypt flow
             *   TCP: chain_flow_id will have reverse tuple (for sending
             *           SYNACK to to network)
             *        chain_rev_flow_id will have original tuple (for sending
             *           SYN to host)
             *   tcpcb[chain_flow_id] -> tlscb[chain_flow_id] ->
             *         proxyrcb[chain_flow_id] -> ARQ ->
             *         proxyccb[chain_flow_id] -> tcpcb[chain_rev_flow_id] -> host
             * responder host-to-network: SYNACK
             *   tcpcb[chain_rev_flow_id] -> proxyrcb[chain_rev_flow_id] -> ARQ ->
             *         proxyccb[chain_rev_flow_id] -> tlscb[chain_rev_flow_id] ->
             *        tcpcb[chain_flow_id] -> network
             */
            ret = _app_redir_proxyrcb_create(ctx, redir_ctx.chain_flow_id(),
                                             hal::FLOW_ROLE_INITIATOR,
                                             redir_ctx.chain_rev_flow_id(),
                                             redir_ctx.redir_span_type());
            if (ret == HAL_RET_OK) {

                ret = _app_redir_proxyccb_create(ctx, redir_ctx.chain_flow_id(),
                           SERVICE_LIF_TCP_PROXY, 0, redir_ctx.chain_rev_flow_id(),
                           0, redir_ctx.redir_span_type());
            }
            if (ret == HAL_RET_OK) {
                ret = _app_redir_proxyrcb_create(ctx, redir_ctx.chain_rev_flow_id(),
                                                 hal::FLOW_ROLE_RESPONDER,
                                                 redir_ctx.chain_flow_id(),
                                                 redir_ctx.redir_span_type());
            }
            if (ret == HAL_RET_OK) {
                ret = _app_redir_proxyccb_create(ctx, redir_ctx.chain_rev_flow_id(),
                           SERVICE_LIF_TLS_PROXY, 0, redir_ctx.chain_rev_flow_id(),
                           0, redir_ctx.redir_span_type());
            }
        }
    }

    return app_redir_feature_status_set(ctx, ret);
}


/*
 * Create and initialize mirror session for SPAN (visibility mode).
 * Also create the rawrcb/rawccb needed for redirecting SPAN packets.
 */
static hal_ret_t
app_redir_span_create_init(fte::ctx_t& ctx)
{
    rawrcb_t            *rawrcb;
    hal_ret_t           ret = HAL_RET_OK;

    SDK_SPINLOCK_LOCK(&mirror_create_lock);
    rawrcb = find_rawrcb_by_id(APP_REDIR_SPAN_RAWRCB_ID);
    if (!rawrcb) {

#if APP_REDIR_VISIBILITY_USE_MIRROR_SESSION
        ret = app_redir_mirror_session_create(visib_mirror_session_id);
#else
        ret = app_redir_ing_replication_create(visib_oif_list_id);
#endif
        if (ret == HAL_RET_OK) {
            ret = _app_redir_rawccb_create(ctx, APP_REDIR_SPAN_RAWRCB_ID);
        }
        if (ret == HAL_RET_OK) {
            ret = _app_redir_rawrcb_create(ctx, APP_REDIR_SPAN_RAWRCB_ID);
        }
    }

    SDK_SPINLOCK_UNLOCK(&mirror_create_lock);
    return ret;
}


/*
 * Application packet Tx enqueue function
 */
static hal_ret_t
app_redir_pkt_check_tx_enqueue(fte::ctx_t& ctx)
{
    app_redir_ctx_t&                redir_ctx = app_redir_ctxref(ctx);
    uint8_t                         *pkt;
    const fte::cpu_rxhdr_t          *cpu_rxhdr;
    size_t                          pkt_len;
    hal::pd::cpu_to_p4plus_header_t cpu_header;
    hal::pd::p4plus_to_p4_header_t  p4_header;
    hal_ret_t                       ret;

    if (!ctx.pkt() || redir_ctx.chain_pkt_enqueued() ||

        /*
         * Queue the packet for forwarding to the next service chain if not
         * dropped by any app features and the packet did not come from SPAN.
         */
        !redir_ctx.redir_policy_applic() || !redir_ctx.chain_pkt_verdict_pass() ||
        redir_ctx.chain_pkt_span_instance()) {

        return HAL_RET_OK;
    }

    /*
     * Register current ctx chain queue ID with cpupkt API
     * for sending packet.
     */
    cpu_rxhdr = ctx.cpu_rxhdr();
    memset(&cpu_header, 0, sizeof(cpu_header));
    memset(&p4_header, 0, sizeof(p4_header));
    cpu_header.flags = redir_ctx.redir_flags();
    cpu_header.src_lif = cpu_rxhdr->src_lif;
    p4_header.lkp_inst = 1;

    /*
     * Enqueue packet without app redirect headers
     */
    pkt = ctx.pkt();
    pkt_len = ctx.pkt_len();
    if (!redir_ctx.redir_miss_pkt_p()) {
        pkt += redir_ctx.hdr_len_total();
        pkt_len -= redir_ctx.hdr_len_total();
    }

    HAL_TRACE_DEBUG("{} flow_id {} pkt_len {} src_lif {}", __FUNCTION__,
                    redir_ctx.chain_flow_id(), pkt_len, cpu_header.src_lif);
    pd::pd_cpupkt_register_tx_queue_args_t args;
    pd::pd_func_args_t          pd_func_args = {0};
    args.ctxt = redir_ctx.arm_ctx();
    args.type = redir_ctx.chain_wring_type();
    args.queue_id = redir_ctx.chain_flow_id();
    pd_func_args.pd_cpupkt_register_tx_queue= &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_CPU_REG_TXQ, &pd_func_args);

#if 0
    ret = hal::pd::cpupkt_register_tx_queue(redir_ctx.arm_ctx(),
                                            redir_ctx.chain_wring_type(),
                                            redir_ctx.chain_flow_id());
#endif
    if (ret == HAL_RET_OK) {
        ret = ctx.queue_txpkt(pkt, pkt_len, &cpu_header, &p4_header,
                              SERVICE_LIF_APP_REDIR, redir_ctx.chain_qtype(),
                              redir_ctx.chain_flow_id(), redir_ctx.chain_ring(),
                              redir_ctx.chain_wring_type());
    }

    redir_ctx.set_chain_pkt_enqueued(true);
    return app_redir_feature_status_set(ctx, ret);
}


/*
 * Build FTE flow_key from app_redir header.
 */
static void
app_redir_flow_key_build_from_redir_hdr(fte::ctx_t& ctx,
                                        hal::flow_direction_t dir,
                                        const pen_app_redir_header_v1_full_t& app_hdr)
{
    flow_key_t      flow_key = {0};

    flow_key.dir = dir;
    flow_key.svrf_id = flow_key.dvrf_id = ntohs(app_hdr.proxy.vrf);
    if (app_hdr.proxy.af == AF_INET) {
        flow_key.flow_type = hal::FLOW_TYPE_V4;
        flow_key.sip.v4_addr = ntohl(app_hdr.proxy.ip_sa[0]);
        flow_key.dip.v4_addr = ntohl(app_hdr.proxy.ip_da[0]);
    } else {
        flow_key.flow_type = hal::FLOW_TYPE_V6;
        memcpy(&flow_key.sip.v6_addr, &app_hdr.proxy.ip_sa[0],
               sizeof(flow_key.sip.v6_addr));
        memcpy(&flow_key.dip.v6_addr, &app_hdr.proxy.ip_da[0],
               sizeof(flow_key.dip.v6_addr));
    }

    flow_key.proto = (types::IPProtocol)app_hdr.proxy.ip_proto;
    flow_key.sport = ntohs(app_hdr.proxy.sport);
    flow_key.dport = ntohs(app_hdr.proxy.dport);

    HAL_TRACE_DEBUG("{} {}", __FUNCTION__, flow_key);
    ctx.set_key(flow_key);
}


/*
 * Build FTE flow_key from proxyrcb
 */
static void
app_redir_flow_key_build_from_proxyrcb(fte::ctx_t& ctx,
                                       const proxyrcb_t& proxyrcb)
{
    flow_key_t      flow_key = {0};

    flow_key.dir = proxyrcb.dir;
    flow_key.svrf_id = flow_key.dvrf_id = ntohs(proxyrcb.vrf);
    if (proxyrcb.af == AF_INET) {
        flow_key.flow_type = hal::FLOW_TYPE_V4;
        flow_key.sip.v4_addr = ntohl(proxyrcb.ip_sa.v4_addr);
        flow_key.dip.v4_addr = ntohl(proxyrcb.ip_da.v4_addr);
    } else {
        flow_key.flow_type = hal::FLOW_TYPE_V6;
        memcpy(&flow_key.sip.v6_addr, &proxyrcb.ip_sa.v6_addr,
               sizeof(flow_key.sip.v6_addr));
        memcpy(&flow_key.dip.v6_addr, &proxyrcb.ip_da.v6_addr,
               sizeof(flow_key.dip.v6_addr));
    }

    flow_key.proto = (types::IPProtocol)proxyrcb.ip_proto;
    flow_key.sport = ntohs(proxyrcb.sport);
    flow_key.dport = ntohs(proxyrcb.dport);

    HAL_TRACE_DEBUG("{} {}", __FUNCTION__, flow_key);
    ctx.set_key(flow_key);
}


/*
 * Insert app redirect headers for a packet resulting in flow miss
 */
static hal_ret_t
app_redir_miss_hdr_insert(fte::ctx_t& ctx,
                          uint8_t format)
{
    app_redir_ctx_t&                redir_ctx = app_redir_ctxref(ctx);
    pen_app_redir_header_v1_full_t& redir_miss_hdr = redir_ctx.redir_miss_hdr();
    const flow_key_t&               flow_key = ctx.key();
    const fte::cpu_rxhdr_t          *cpu_rxhdr;
    size_t                          hdr_len = 0;
    hal_ret_t                       ret = HAL_RET_OK;
    uint16_t                        redir_flags;

    /*
     * No-op if header already inserted
     */
    if (redir_ctx.redir_miss_pkt_p()) {
        goto done;
    }

    redir_miss_hdr.app.h_proto = htons(PEN_APP_REDIR_ETHERTYPE);
    cpu_rxhdr = ctx.cpu_rxhdr();
    redir_miss_hdr.ver.format = format;
    redir_flags = PEN_APP_REDIR_L3_CSUM_CHECKED |
                  PEN_APP_REDIR_L4_CSUM_CHECKED;
    if (redir_ctx.redir_span_type() != APP_REDIR_SPAN_NONE) {
        redir_flags |= PEN_APP_REDIR_SPAN_INSTANCE;
    }

    switch (format) {

    case PEN_RAW_REDIR_V1_FORMAT:
        hdr_len = PEN_APP_REDIR_VERSION_HEADER_SIZE +
                  PEN_RAW_REDIR_HEADER_V1_SIZE;
        redir_miss_hdr.ver.hdr_len = htons(hdr_len);
        redir_miss_hdr.raw.vrf = htons(flow_key.svrf_id);
        redir_miss_hdr.raw.flags = htons(redir_flags);
        redir_miss_hdr.raw.flow_id = htonl(redir_ctx.chain_flow_id());
        redir_ctx.set_redir_miss_pkt_p(ctx.pkt());
        redir_miss_hdr.raw.redir_miss_pkt_p = (uint64_t)ctx.pkt();

        HAL_TRACE_DEBUG("{} hdr_len {} format {}", __FUNCTION__,
                        hdr_len + PEN_APP_REDIR_HEADER_SIZE, format);
        break;

    case PEN_PROXY_REDIR_V1_FORMAT:
        hdr_len = PEN_APP_REDIR_VERSION_HEADER_SIZE +
                  PEN_PROXY_REDIR_HEADER_V1_SIZE;
        redir_miss_hdr.ver.hdr_len = htons(hdr_len);
        redir_miss_hdr.proxy.vrf = htons(flow_key.svrf_id);
        redir_miss_hdr.proxy.flags = htons(redir_flags);
        redir_miss_hdr.proxy.flow_id = htonl(redir_ctx.chain_flow_id());
        redir_miss_hdr.proxy.tcp_flags = cpu_rxhdr->tcp_flags;
        redir_ctx.set_redir_miss_pkt_p(ctx.pkt());

        /*
         * Construct proxy header from flow_key
         */
        redir_miss_hdr.proxy.ip_proto = IPPROTO_TCP;
        redir_miss_hdr.proxy.sport = htons(flow_key.sport);
        redir_miss_hdr.proxy.dport = htons(flow_key.dport);
        if (flow_key.flow_type == hal::FLOW_TYPE_V4) {
            redir_miss_hdr.proxy.af = AF_INET;
            redir_miss_hdr.proxy.ip_sa[0] = htonl(flow_key.sip.v4_addr);
            redir_miss_hdr.proxy.ip_da[0] = htonl(flow_key.dip.v4_addr);
        } else {
            redir_miss_hdr.proxy.af = AF_INET6;
            memcpy(&redir_miss_hdr.proxy.ip_sa[0], &flow_key.sip.v6_addr,
                   sizeof(redir_miss_hdr.proxy.ip_sa));
            memcpy(&redir_miss_hdr.proxy.ip_da[0], &flow_key.dip.v6_addr,
                   sizeof(redir_miss_hdr.proxy.ip_da));
        }

        HAL_TRACE_DEBUG("{} hdr_len {} format {}", __FUNCTION__,
                        hdr_len + PEN_APP_REDIR_HEADER_SIZE, format);
        break;

    default:
        HAL_TRACE_ERR("{} unknown format {}", __FUNCTION__, format);
        ret = HAL_RET_APP_REDIR_FORMAT_UNKNOWN;
        break;
    }

done:
    return app_redir_feature_status_set(ctx, ret);
}


/*
 * Validate Pensando app header in redirected packet.
 */
static hal_ret_t
app_redir_app_hdr_validate(fte::ctx_t& ctx)
{
    app_redir_ctx_t&                redir_ctx = app_redir_ctxref(ctx);
    const flow_key_t&               flow_key = ctx.key();
    pen_app_redir_header_v1_full_t  *app_hdr;
    const fte::cpu_rxhdr_t          *cpu_rxhdr;
    size_t                          pkt_len;
    size_t                          hdr_len;
    uint16_t                        h_proto;
    uint16_t                        redir_flags = 0;
    hal_ret_t                       ret = HAL_RET_OK;

    app_hdr = (pen_app_redir_header_v1_full_t *)ctx.pkt();
    pkt_len = ctx.pkt_len();
    if (redir_ctx.redir_miss_pkt_p()) {

        /*
         * Assume certain pkt_len for validation below
         */
        app_hdr = &redir_ctx.redir_miss_hdr();
        pkt_len += redir_ctx.redir_miss_hdr_size();
    }

    if (pkt_len < PEN_APP_REDIR_HEADER_MIN_SIZE) {
        HAL_TRACE_ERR("{} pkt_len {} too small", __FUNCTION__, pkt_len);
        ret = HAL_RET_APP_REDIR_HDR_LEN_ERR;
        goto done;
    }

    cpu_rxhdr = ctx.cpu_rxhdr();
    h_proto = ntohs(app_hdr->app.h_proto);
    hdr_len = ntohs(app_hdr->ver.hdr_len);
    HAL_TRACE_DEBUG("{} pkt_len {} h_proto {:#x} hdr_len {} format {}",
                    __FUNCTION__, pkt_len, h_proto, hdr_len, app_hdr->ver.format);
    if (h_proto != PEN_APP_REDIR_ETHERTYPE) {
        HAL_TRACE_ERR("{} unexpected h_proto {:#x}", __FUNCTION__, h_proto);
        ret = HAL_RET_APP_REDIR_HDR_ERR;
        goto done;
    }

    switch (app_hdr->ver.format) {

    case PEN_RAW_REDIR_V1_FORMAT:

        /*
         * Tenant ID not accessible from P4+ so have to set it here
         */
        redir_flags = ntohs(app_hdr->raw.flags);
        app_hdr->raw.vrf = htons(flow_key.svrf_id);
        HAL_TRACE_DEBUG("{} flow_id {:#x} redir_flags {:#x} vrf {}", __FUNCTION__,
                        ntohl(app_hdr->raw.flow_id), redir_flags, flow_key.svrf_id);
        if ((pkt_len < PEN_RAW_REDIR_HEADER_V1_FULL_SIZE) ||
            ((hdr_len - PEN_APP_REDIR_VERSION_HEADER_SIZE) !=
                        PEN_RAW_REDIR_HEADER_V1_SIZE)) {
            HAL_TRACE_ERR("{} format {} invalid pkt_len {} or hdr_len {}",
                          __FUNCTION__, pkt_len, app_hdr->ver.format, hdr_len);
            ret = HAL_RET_APP_REDIR_HDR_LEN_ERR;
            goto done;
        }

        redir_ctx.set_chain_qtype(APP_REDIR_RAWC_QTYPE);
        redir_ctx.set_chain_wring_type(types::WRING_TYPE_APP_REDIR_RAWC);
        break;

    case PEN_PROXY_REDIR_V1_FORMAT:
        redir_flags = ntohs(app_hdr->proxy.flags);
        HAL_TRACE_DEBUG("{} flow_id {:#x} redir_flags {:#x} vrf {}", __FUNCTION__,
                        ntohl(app_hdr->proxy.flow_id), redir_flags,
                        ntohs(app_hdr->proxy.vrf));
        if ((pkt_len < PEN_PROXY_REDIR_HEADER_V1_FULL_SIZE) ||
            ((hdr_len - PEN_APP_REDIR_VERSION_HEADER_SIZE) !=
                        PEN_PROXY_REDIR_HEADER_V1_SIZE)) {
            HAL_TRACE_ERR("{} format {} invalid pkt_len {} or hdr_len {}",
                          __FUNCTION__, pkt_len, app_hdr->ver.format, hdr_len);
            ret = HAL_RET_APP_REDIR_HDR_LEN_ERR;
            goto done;
        }

        /*
         * For proxy flow miss, the current packet came with full L2/L3/L4 headers
         * from which FTE already extracted the key. For flow hit, only the
         * app_redir proxy header is available and we have to extract the
         * flow key on FTE's behalf.
         */
        if (ctx.app_redir_pipeline()) {
            app_redir_flow_key_build_from_redir_hdr(ctx,
                           (hal::flow_direction_t)cpu_rxhdr->lkp_dir, *app_hdr);
        }

        redir_ctx.set_chain_qtype(APP_REDIR_PROXYC_QTYPE);
        redir_ctx.set_chain_wring_type(types::WRING_TYPE_APP_REDIR_PROXYC);
        break;

    default:
        HAL_TRACE_ERR("{} unknown format {}", __FUNCTION__,
                      app_hdr->ver.format);
        ret = HAL_RET_APP_REDIR_FORMAT_UNKNOWN;
        goto done;
    }

    redir_ctx.set_redir_flags(redir_flags);
    redir_ctx.set_hdr_len_total(hdr_len + PEN_APP_REDIR_HEADER_SIZE);
    if (redir_flags & PEN_APP_REDIR_SPAN_INSTANCE) {
        redir_ctx.set_redir_span_type(APP_REDIR_SPAN_APPLIC_DEFAULT_TYPE);
    }

done:
    app_redir_feature_status_set(ctx, ret);
    return ret;
}


/*
 * Return pointer to current packet, taking into account of inserted
 * app redirect headers in the flow miss case.
 */
uint8_t *
app_redir_pkt(fte::ctx_t& ctx)
{
    app_redir_ctx_t&   redir_ctx = app_redir_ctxref(ctx);

    /*
     * Skip packet Rx in TCP proxy pipeline which was not
     * marked as applicable.
     */
    if (ctx.tcp_proxy_pipeline() && !redir_ctx.redir_policy_applic()) {
        return nullptr;
    }

    return redir_ctx.redir_miss_pkt_p() ?
           (uint8_t *)&redir_ctx.redir_miss_hdr() : ctx.pkt();
}


/*
 * Return length of current packet, taking into account of inserted
 * app redirect headers in the flow miss case.
 */
size_t
app_redir_pkt_len(fte::ctx_t& ctx)
{
    app_redir_ctx_t&        redir_ctx = app_redir_ctxref(ctx);
    size_t                  pkt_len;

    /*
     * Skip packet Rx in TCP proxy pipeline which was not
     * marked as applicable.
     */
    if (ctx.tcp_proxy_pipeline() && !redir_ctx.redir_policy_applic()) {
        return 0;
    }

    pkt_len = ctx.pkt_len();
    if (redir_ctx.redir_miss_pkt_p()) {
        pkt_len = redir_ctx.tcp_tls_proxy_flow() ?
                  redir_ctx.hdr_len_total()      :
                  redir_ctx.hdr_len_total() + pkt_len;
    }

    return pkt_len;
}


/*
 * TCP/TLS proxy handle their own nflow/hflow 3whs control packet Tx
 * so we should not attempt any Tx of our own.
 */
static void
app_redir_tcp_control_tx_block(fte::ctx_t& ctx)
{
    app_redir_ctx_t&    redir_ctx = app_redir_ctxref(ctx);

    if ((ctx.flow_miss() && redir_ctx.tcp_tls_proxy_flow()) ||
        ctx.tcp_proxy_pipeline()) {

        app_redir_pkt_verdict_set(ctx, APP_REDIR_VERDICT_BLOCK);
    }
}


/*
 * Forward redirected packet to application or simply loop it back (DOL case).
 */
static hal_ret_t
app_redir_pkt_process(fte::ctx_t& ctx)
{
    app_redir_tcp_control_tx_block(ctx);

    return app_redir_app_hdr_validate(ctx);
}


/*
 * Return proxy info for the current flow.
 */
static proxy_flow_info_t *
app_redir_proxy_flow_info_find(fte::ctx_t& ctx,
                               flow_key_t& flow_key,
                               proxy_flow_type_include_t flow_type_include)
{
    app_redir_ctx_t&        redir_ctx = app_redir_ctxref(ctx);
    proxy_flow_info_t       *pfi;
    types::ProxyType        type;

    pfi = redir_ctx.proxy_flow_info() ?
          redir_ctx.proxy_flow_info() :
          proxy_get_flow_info(types::PROXY_TYPE_APP_REDIR, flow_key);

    if (!pfi) {
        pfi = proxy_get_flow_info(types::PROXY_TYPE_APP_REDIR_SPAN,
                                  flow_key);
    }
    if (!pfi) {
        pfi = proxy_get_flow_info(types::PROXY_TYPE_APP_REDIR_PROXY_TCP,
                                  flow_key);
    }
    if (!pfi) {
        pfi = proxy_get_flow_info(types::PROXY_TYPE_APP_REDIR_PROXY_TCP_SPAN,
                                  flow_key);
    }

    if (!pfi && (flow_type_include == PROXY_FLOW_TYPE_INCLUDE_TCP)) {

        /*
         * See if flow was configured as TCP/TLS proxy flow
         */
        pfi = proxy_get_flow_info(types::PROXY_TYPE_TCP, flow_key);
    }

    if (pfi) {
        type = pfi->proxy->type;
        HAL_TRACE_DEBUG("app_redir proxy flow type {}", type);

        redir_ctx.set_tcp_tls_proxy_flow((type == types::PROXY_TYPE_TCP) ||
                          (type == types::PROXY_TYPE_APP_REDIR_PROXY_TCP) ||
                          (type == types::PROXY_TYPE_APP_REDIR_PROXY_TCP_SPAN));
        if ((type == types::PROXY_TYPE_APP_REDIR_SPAN) ||
            (type == types::PROXY_TYPE_APP_REDIR_PROXY_TCP_SPAN)) {
            redir_ctx.set_redir_span_type(APP_REDIR_SPAN_APPLIC_DEFAULT_TYPE);
        }

    } else {
        HAL_TRACE_DEBUG("app_redir is not applicable for the flow");
    }
    return pfi;
}


/*
 * Return proxy info for the current flow if, by configuration or by certain
 * test criteria, the flow is subject to app redirect.
 */
static proxy_flow_info_t *
app_redir_proxy_flow_info_get(fte::ctx_t& ctx,
                              flow_key_t& flow_key,
                              proxy_flow_type_include_t flow_type_include)
{
    app_redir_ctx_t&        redir_ctx = app_redir_ctxref(ctx);
    proxy_flow_info_t       *pfi;
    proxy_flow_info_t       *rpfi = NULL;
    flow_key_t              rev_flow_key;
    hal_ret_t               ret = HAL_RET_OK;
    types::ProxyType        type;
    bool                    alloc_qid;

    pfi = app_redir_proxy_flow_info_find(ctx, flow_key, flow_type_include);
    if (!pfi) {

        /*
         * Create flow only for flow miss case
         */
        if (!ctx.pkt() || !ctx.flow_miss() || !redir_ctx.redir_policy_applic()) {
            goto done;
        }

        if (ctx.existing_session()) {

            /*
             * Ignore direction - always set to 0 for flow_info lookup.
             */
            rev_flow_key = ctx.get_key(redir_ctx.chain_rev_role());
            rev_flow_key.dir = 0;

            /*
             * lkpvrf is present in flow_key_t but the proxy_flow gRPC infra
             * does not support it, hence we also set it to 0 here.
             */
            rev_flow_key.lkpvrf = 0;
            rpfi = app_redir_proxy_flow_info_find(ctx, rev_flow_key,
                                                  flow_type_include);
            if (!rpfi) {
                HAL_TRACE_DEBUG("app_redir existing_session for rev_role {} "
                                "not found", redir_ctx.chain_rev_role());

                ret = HAL_RET_FLOW_NOT_FOUND;
                goto done;
            }
        }

        /*
         * Create the flow;
         * for raw span, HW always spans to qtype 0 and qid 0, lif is derived
         * from the mirror session's lport_id. Once in rawrcb, P4+ will hash
         * on flow and spray to appropriate ARQ.
         */
        type = redir_ctx.redir_span_type() == APP_REDIR_SPAN_NONE ?
               types::PROXY_TYPE_APP_REDIR : types::PROXY_TYPE_APP_REDIR_SPAN;

        alloc_qid = !ctx.existing_session() &&
                    (type != types::PROXY_TYPE_APP_REDIR_SPAN);
        ret = proxy_flow_enable(type, flow_key, alloc_qid, NULL, NULL);
        if (ret != HAL_RET_OK) {
            goto done;
        }

        pfi = proxy_get_flow_info(type, flow_key);
        assert(pfi);
        if (rpfi) {
            pfi->qid1 = rpfi->qid1;
            pfi->qid2 = rpfi->qid2;
        }
    }

    if (pfi) {
        redir_ctx.set_proxy_flow_info(pfi);
        redir_ctx.set_redir_policy_applic(true);
    }

done:
    app_redir_feature_status_set(ctx, ret);
    return pfi;
}


/*
 * Update fwding info for the current flow, redirecting it to us.
 */
static hal_ret_t
app_redir_flow_fwding_update(fte::ctx_t& ctx,
                             proxy_flow_info_t *pfi,
                             flow_key_t& flow_key)
{
    app_redir_ctx_t&        redir_ctx = app_redir_ctxref(ctx);
    fte::flow_update_t      flowupd = {type: fte::FLOWUPD_FWDING_INFO};
    hal_ret_t               ret = HAL_RET_OK;

    assert(pfi);
    if (ctx.role() ==  hal::FLOW_ROLE_INITIATOR) {
        flowupd.fwding.qid = pfi->qid1;
        redir_ctx.set_chain_flow_id(pfi->qid1);
        redir_ctx.set_chain_rev_flow_id(pfi->qid2);
    } else {
        flowupd.fwding.qid = pfi->qid2;
        redir_ctx.set_chain_flow_id(pfi->qid2);
        redir_ctx.set_chain_rev_flow_id(pfi->qid1);
    }

    if (!redir_ctx.tcp_tls_proxy_flow()) {

        if (redir_ctx.redir_span_type() == APP_REDIR_SPAN_NONE) {
            flowupd.fwding.lport = pfi->proxy->meta->lif_info[0].lport_id;
            flowupd.fwding.qid_en = true;
            flowupd.fwding.qtype = APP_REDIR_RAWR_QTYPE;
            HAL_TRACE_DEBUG("app_redir flow forwarding role: {} qid1: {} qid2: {}",
                            ctx.role(), pfi->qid1, pfi->qid2);
            HAL_TRACE_DEBUG("app_redir updating lport = {} for sport = {} dport = {}",
                            flowupd.fwding.lport, flow_key.sport, flow_key.dport);
        } else {
            HAL_TRACE_DEBUG("app_redir flow forwarding role: {} span_type: {}",
                            ctx.role(), redir_ctx.redir_span_type());
            ret = app_redir_span_create_init(ctx);

#if APP_REDIR_VISIBILITY_USE_MIRROR_SESSION
            flowupd.type = fte::FLOWUPD_MIRROR_INFO;
            flowupd.mirror_info.mirror_en = true;
            flowupd.mirror_info.ing_mirror_session = 0;
            flowupd.mirror_info.egr_mirror_session = 0;
            if (redir_ctx.redir_span_type() == APP_REDIR_SPAN_INGRESS) {
                flowupd.mirror_info.proxy_ing_mirror_session =
                                              1 << visib_mirror_session_id;
                flowupd.mirror_info.proxy_egr_mirror_session = 0;
            } else {
                flowupd.mirror_info.proxy_ing_mirror_session = 0;
                flowupd.mirror_info.proxy_egr_mirror_session =
                                              1 << visib_mirror_session_id;
            }
#else
            if (redir_ctx.redir_span_type() == APP_REDIR_SPAN_INGRESS) {
                flowupd.type = fte::FLOWUPD_MCAST_COPY;
                flowupd.mcast_info.mcast_en = true;
                flowupd.mcast_info.proxy_mcast_ptr = visib_oif_list_id;
                flowupd.mcast_info.mcast_ptr = 0;
            } else {
                HAL_TRACE_ERR("app_redir ingress replication cannot be used "
                              "for egress");
                ret = HAL_RET_INVALID_OP;
            }
#endif
        }

        if (ret == HAL_RET_OK) {
            ret = ctx.update_flow(flowupd);
        }
    }
    return app_redir_feature_status_set(ctx, ret);
}


/*
 * Wrapper for app_redir_proxy_flow_info_get() and app_redir_flow_fwding_update().
 */
static hal_ret_t
app_redir_proxy_flow_info_update(fte::ctx_t& ctx,
                                 proxy_flow_type_include_t flow_type_include)
{
    flow_key_t              flow_key;
    proxy_flow_info_t       *pfi;
    hal_ret_t               ret = HAL_RET_OK;

    /*
     * Ignore direction - always set to 0 for flow_info lookup.
     */
    flow_key = ctx.key();
    flow_key.dir = 0;

    /*
     * lkpvrf is present in flow_key_t but the proxy_flow gRPC infra
     * does not support it, hence we also set it to 0 here.
     */
    flow_key.lkpvrf = 0;
    pfi = app_redir_proxy_flow_info_get(ctx, flow_key, flow_type_include);
    if (pfi) {
        ret = app_redir_flow_fwding_update(ctx, pfi, flow_key);
    }

    return ret;
}


/*
 * Set app redirct verdict for the current packet, independent of any
 * FTE drop decision made in other pipelines/features.
 */
void
app_redir_pkt_verdict_set(fte::ctx_t& ctx,
                          app_redir_verdict_t verdict)
{
    app_redir_ctx_t&   redir_ctx = app_redir_ctxref(ctx);

    HAL_TRACE_DEBUG("{} verdict {}", __FUNCTION__, verdict);
    redir_ctx.set_chain_pkt_verdict(verdict);
}


/*
 * Exported API for outside features to indicate redirect policy
 * applicability for the current flow.
 */
hal_ret_t
app_redir_policy_applic_set(fte::ctx_t& ctx,
                            app_redir_span_type_t redir_span_type)
{
    app_redir_ctx_t&        redir_ctx = app_redir_ctxref(ctx);
    hal_ret_t               ret = HAL_RET_OK;

    redir_ctx.set_redir_policy_applic(true);
    redir_ctx.set_redir_span_type(redir_span_type);

    /*
     * Update flow to redirect to us.
     */
    if (ctx.pkt() && ctx.flow_miss()) {

        app_redir_proxy_flow_info_update(ctx, PROXY_FLOW_TYPE_INCLUDE_TCP);

        /*
         * Insert app redirect header for the flow miss case
         */
        ret = app_redir_miss_hdr_insert(ctx,
                                 redir_ctx.tcp_tls_proxy_flow() ?
                                 PEN_PROXY_REDIR_V1_FORMAT      :
                                 PEN_RAW_REDIR_V1_FORMAT);
        if (ret == HAL_RET_OK) {
            ret = app_redir_pkt_process(ctx);
        }
    }

    return app_redir_feature_status_set(ctx, ret);
}


/*
 * Process 3whs packet from TCP proxy pipeline.
 */
static hal_ret_t
app_redir_tcp_pipeline_process(fte::ctx_t& ctx)
{
    app_redir_ctx_t&        redir_ctx = app_redir_ctxref(ctx);
    const fte::cpu_rxhdr_t  *cpu_rxhdr = ctx.cpu_rxhdr();
    const proxyrcb_t        *proxyrcb;
    hal_ret_t               ret = HAL_RET_OK;
    uint32_t                flow_id;
    uint8_t                 tcp_flags;

    /*
     * When Rx from the TCP proxy LIF, we expect to see a SYNACK or an ACK.
     * The SYNACK should come from the responder flow (which is the hflow)
     * while the ACK is from the initiator flow (which is the nflow).
     */
    flow_id = cpu_rxhdr->qid;
    proxyrcb = find_proxyrcb_by_id(flow_id);
    if (proxyrcb) {
        tcp_flags = cpu_rxhdr->tcp_flags;
        HAL_TRACE_DEBUG("{} dir {} role {} cb_id {} rev_cb_id {} tcp_flags 0x{:x}",
                        __FUNCTION__, proxyrcb->dir, proxyrcb->role,
                        proxyrcb->cb_id, proxyrcb->rev_cb_id, tcp_flags);
        switch (proxyrcb->role) {

        case hal::FLOW_ROLE_INITIATOR:
            if ((tcp_flags & (TCP_FLAG_SYN | TCP_FLAG_ACK)) == TCP_FLAG_ACK) {

                /*
                 * flow_id for storing into L7 header will consistently
                 * be the hflow which is the responder flow_id.
                 */
                flow_id = proxyrcb->rev_cb_id;
                redir_ctx.set_redir_policy_applic(true);
            }
            break;

        case hal::FLOW_ROLE_RESPONDER:
            if ((tcp_flags & (TCP_FLAG_SYN | TCP_FLAG_ACK)) ==
                             (TCP_FLAG_SYN | TCP_FLAG_ACK)) {
                redir_ctx.set_redir_policy_applic(true);
            }
            break;

        default:
            HAL_TRACE_ERR("{} invalid role {}", __FUNCTION__, proxyrcb->role);
            break;
        }


        if (redir_ctx.redir_policy_applic()) {
            HAL_TRACE_DEBUG("{} redir_policy_applic flow_id {}",
                            __FUNCTION__, flow_id);

            redir_ctx.set_chain_flow_id(flow_id);
            ctx.set_role((hal::flow_role_t)proxyrcb->role);
            if (proxyrcb->redir_span) {
                redir_ctx.set_redir_span_type(APP_REDIR_SPAN_APPLIC_DEFAULT_TYPE);
            }

            app_redir_flow_key_build_from_proxyrcb(ctx, *proxyrcb);
            ret = app_redir_miss_hdr_insert(ctx, PEN_PROXY_REDIR_V1_FORMAT);
            if (ret == HAL_RET_OK) {
                ret = app_redir_pkt_process(ctx);
            }
        }
    }

    return app_redir_feature_status_set(ctx, ret);
}


/*
 * App redirect pipeline flow miss exec handler
 */
fte::pipeline_action_t
app_redir_miss_exec(fte::ctx_t& ctx)
{
    /*
     * Evaluate initial flow ID
     */
    app_redir_rev_role_set(ctx);
    app_redir_proxy_flow_info_update(ctx, PROXY_FLOW_TYPE_APP_REDIR);
    app_redir_tcp_control_tx_block(ctx);

    return app_redir_pipeline_action(ctx);
}


/*
 * App redirect pipeline flow hit exec handler
 */
fte::pipeline_action_t
app_redir_exec(fte::ctx_t& ctx)
{
    app_redir_ctx_t&   redir_ctx = app_redir_ctxref(ctx);

    assert(ctx.pkt());

    /*
     * Handle TCP/TLS proxy handshake
     */
    if (ctx.tcp_proxy_pipeline()) {
        app_redir_tcp_pipeline_process(ctx);

    } else {
        redir_ctx.set_chain_flow_id(ctx.cpu_rxhdr()->qid);
        redir_ctx.set_redir_policy_applic(true);
        app_redir_rev_role_set(ctx);
        app_redir_pkt_process(ctx);
    }

    return app_redir_pipeline_action(ctx);
}


/*
 * Presumably executed as last feature in the redirect pipeline:
 * send or block the current packet based on the type of redirect
 * and packet verdict.
 */
fte::pipeline_action_t
app_redir_exec_fini(fte::ctx_t& ctx)
{
    app_redir_ctx_t&        redir_ctx = app_redir_ctxref(ctx);
    hal_ret_t               ret = HAL_RET_OK;

    if (!ctx.drop()) {

        if (ctx.flow_miss()) {

            if (ctx.pkt() && redir_ctx.proxy_flow_info()) {

                /*
                 * Note that FTE postpones the flow programming until after
                 * the pipeline exits so the ordering here actually
                 * works out correctly, that is, the CBs will end up
                 * getting programmed first.
                 */
                ret = redir_ctx.tcp_tls_proxy_flow() ?
                      app_redir_proxyrcb_proxyccb_create(ctx) :
                      app_redir_rawrcb_rawccb_create(ctx);
            }
        }

        /*
         * Check and queue the packet for forwarding to the next service chain
         * if applicable.
         */
        if (ret == HAL_RET_OK) {
            app_redir_pkt_check_tx_enqueue(ctx);
        }
    }

    return app_redir_pipeline_action(ctx);
}


} // namespace app_redir
} // namespace hal
