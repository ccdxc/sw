// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __APP_REDIR_CTX_H__
#define __APP_REDIR_CTX_H__

#include <stdint.h>
#include "nic/include/fte.hpp"
#include "nic/p4/nw/include/defines.h"

namespace hal {
namespace app_redir {

const std::string FTE_FEATURE_APP_REDIR_MISS("pensando.io/app-redir:app-redir-miss");
const std::string FTE_FEATURE_APP_REDIR("pensando.io/app-redir:app-redir");
const std::string FTE_FEATURE_APP_REDIR_APPID("pensando.io/app-redir:app-redir-appid");
const std::string FTE_FEATURE_APP_REDIR_FINI("pensando.io/app-redir:app-redir-fini");

// Application redirect verdict
typedef enum {
    APP_REDIR_VERDICT_PASS,     // pass the packet
    APP_REDIR_VERDICT_BLOCK,    // block the packet
} __PACK__ app_redir_verdict_t;

// Redirect SPAN type
typedef enum {
    APP_REDIR_SPAN_NONE,
    APP_REDIR_SPAN_INGRESS,
    APP_REDIR_SPAN_EGRESS,

    // when SPAN is applicable, this is the default type
    APP_REDIR_SPAN_APPLIC_DEFAULT_TYPE = APP_REDIR_SPAN_INGRESS 
} __PACK__ app_redir_span_type_t;

/*
 * Two ways of supporting visibility mode: using a mirror session or 
 * ingress replication. 
 *
 * Mirror session has a limitation that P4 only parses up to the L2 header 
 * on the span packet so we'd have to work around that with 
 * app_redir_rx_span_parse_workaround() below.
 *
 * Ingress replication does not have such limitation but is available for
 * ingress copy only. In addition, replication will back pressure the source
 * if either the flow's destination queue or the replication-to queue is full.
 *
 * In the case of mirror session, the span copy would be dropped if the
 * mirror-to queue were full.
 * 
 */
#define APP_REDIR_VISIBILITY_USE_MIRROR_SESSION     0


// appID state
#define APPID_STATE(ENTRY)                                          \
    ENTRY(APPID_STATE_INIT,        0,  "APPID_STATE_INIT")         \
    ENTRY(APPID_STATE_NEEDED,      1,  "APPID_STATE_NEEDED")       \
    ENTRY(APPID_STATE_IN_PROGRESS, 2,  "APPID_STATE_IN_PROGRESS")  \
    ENTRY(APPID_STATE_FOUND,       3,  "APPID_STATE_FOUND")        \
    ENTRY(APPID_STATE_NOT_FOUND,   4,  "APPID_STATE_NOT_FOUND")    \
    ENTRY(APPID_STATE_ABORT,       5,  "APPID_STATE_ABORT")        \
    ENTRY(APPID_STATE_STOPPED,     6,  "APPID_STATE_STOPPED")        \

DEFINE_ENUM(appid_state_t, APPID_STATE)
#undef APPID_STATE

const size_t APPID_MAX_DEPTH = 4;
typedef struct appid_info_s {
    // NOTE - Session state should remain as first entry in the struct
    fte::feature_session_state_t session_state_;         // Feature session state
    appid_state_t state_;
    appid_id_t ids_[APPID_MAX_DEPTH];
    void* cleanup_handle_;
    uint8_t id_count_;
} __PACK__ appid_info_t;

inline appid_id_t appid_info_id(const appid_info_t& info) {
    return info.id_count_ ? info.ids_[info.id_count_ - 1] : 0;
}
inline appid_id_t appid_info_id(const appid_info_t& info, uint8_t idx) {
    assert(idx < APPID_MAX_DEPTH);
    return (idx < info.id_count_) ? info.ids_[idx] : 0;
}
inline std::ostream& operator<<(std::ostream& os, const appid_info_t& val)
{
    os << "{state=" << val.state_;
    if (val.id_count_) {
        for (uint8_t i = 0; i < val.id_count_; i++) {
            os << ",id=" << appid_info_id(val, i);
        }
    }
    return os << "}";
}

class app_redir_ctx_t {
public:
    static void init(void *state)
    {
        app_redir_ctx_t *app_ctx = (app_redir_ctx_t*)state;
        app_ctx->redir_flags_            = 0;
        app_ctx->hdr_len_total_          = 0;
        app_ctx->chain_flow_id_          = 0;
        app_ctx->chain_rev_flow_id_      = 0;
        app_ctx->chain_rev_role_         = hal::FLOW_ROLE_NONE;
        app_ctx->chain_ring_             = 0;
        app_ctx->chain_qtype_            = APP_REDIR_RAWC_QTYPE;
        app_ctx->chain_wring_type_       = types::WRING_TYPE_APP_REDIR_RAWC;
        app_ctx->chain_pkt_verdict_      = APP_REDIR_VERDICT_PASS;
        app_ctx->pipeline_end_           = false;
        app_ctx->chain_pkt_enqueued_     = false;
        app_ctx->redir_policy_applic_    = false;
        app_ctx->tcp_tls_proxy_flow_     = false;
        app_ctx->redir_span_type_        = APP_REDIR_SPAN_NONE;
        app_ctx->redir_miss_pkt_p_       = nullptr;
        app_ctx->proxy_flow_info_        = nullptr;
        app_ctx->arm_ctx_                = nullptr;
        app_ctx->appid_updated_          = false;
        app_ctx->appid_info_             = nullptr;
    };

    uint16_t redir_flags() const { return redir_flags_; }
    void set_redir_flags(uint16_t flags) { redir_flags_ = flags; }

    uint16_t hdr_len_total() const { return hdr_len_total_; }
    void set_hdr_len_total(uint16_t hdr_len_total) { hdr_len_total_ = hdr_len_total; }

    bool chain_pkt_enqueued() const { return chain_pkt_enqueued_; }
    void set_chain_pkt_enqueued(bool yesno) { chain_pkt_enqueued_ = yesno; }

    app_redir_verdict_t chain_pkt_verdict() const { return chain_pkt_verdict_; }
    void set_chain_pkt_verdict(app_redir_verdict_t verdict) 
    { 
        if (chain_pkt_verdict_ != APP_REDIR_VERDICT_BLOCK) {
            chain_pkt_verdict_ = verdict;
        }
    }

    bool chain_pkt_verdict_pass(void)
    {
        return (chain_pkt_verdict_ != APP_REDIR_VERDICT_BLOCK);
    }

    bool chain_pkt_span_instance(void)
    {
        return redir_span_type_ != APP_REDIR_SPAN_NONE;
    }

    bool pipeline_end() const { return pipeline_end_; }
    void set_pipeline_end(bool yesno) { pipeline_end_ = yesno; }

    bool redir_policy_applic() const { return redir_policy_applic_; }
    void set_redir_policy_applic(bool yesno) { redir_policy_applic_ = yesno; }

    bool tcp_tls_proxy_flow() const { return tcp_tls_proxy_flow_; }
    void set_tcp_tls_proxy_flow(bool yesno) { tcp_tls_proxy_flow_ = yesno; }

    app_redir_span_type_t redir_span_type() const { return redir_span_type_; }
    void set_redir_span_type(app_redir_span_type_t redir_span_type)
    { 
        if (redir_span_type_ == APP_REDIR_SPAN_NONE) {
            redir_span_type_ = redir_span_type;
        }
    }

    uint8_t chain_qtype() const { return chain_qtype_; }
    void set_chain_qtype(uint8_t chain_qtype) { chain_qtype_ = chain_qtype; }

    uint8_t chain_ring() const { return chain_ring_; }
    void set_chain_ring(uint8_t chain_ring) { chain_ring_ = chain_ring; }

    uint32_t chain_flow_id() const { return chain_flow_id_; }
    void set_chain_flow_id(uint32_t chain_flow_id) { chain_flow_id_ = chain_flow_id; }

    uint32_t chain_rev_flow_id() const { return chain_rev_flow_id_; }
    void set_chain_rev_flow_id(uint32_t chain_rev_flow_id) { chain_rev_flow_id_ = chain_rev_flow_id; }

    hal::flow_role_t chain_rev_role() const { return chain_rev_role_; }
    void set_chain_rev_role(hal::flow_role_t chain_rev_role) { chain_rev_role_ = chain_rev_role; }

    uint8_t *redir_miss_pkt_p() const { return redir_miss_pkt_p_; }
    void set_redir_miss_pkt_p(uint8_t *redir_miss_pkt_p) { redir_miss_pkt_p_ = redir_miss_pkt_p; }

    hal::proxy_flow_info_t *proxy_flow_info() { return proxy_flow_info_; }
    void set_proxy_flow_info(hal::proxy_flow_info_t *proxy_flow_info) { proxy_flow_info_ = proxy_flow_info; }

    types::WRingType chain_wring_type() const { return chain_wring_type_; }
    void set_chain_wring_type(types::WRingType chain_wring_type) { chain_wring_type_ = chain_wring_type; }

    hal::pd::cpupkt_ctxt_t* arm_ctx() const { return arm_ctx_; }
    void set_arm_ctx(hal::pd::cpupkt_ctxt_t* arm_ctx) {arm_ctx_= arm_ctx;}

    pen_app_redir_header_v1_full_t& redir_miss_hdr() { return redir_miss_hdr_; }
    size_t redir_miss_hdr_size() { return sizeof(redir_miss_hdr_); }

    bool appid_updated() const {
      if(!appid_info_)
          return false;
      else
          return (appid_updated_ && appid_info_->id_count_ != 0);
    }
    void set_appid_updated(bool updated) { appid_updated_ = updated; }

    appid_info_t* appid_info() { return appid_info_; };
    void update_appid_info(appid_info_t& info) {
        uint32_t off = sizeof(appid_info_->session_state_);
        uint32_t len = sizeof(info) - off;
        memcpy((uint8_t*)appid_info_+off, (uint8_t*)&info+off, len);
    };
    void set_appid_info(appid_info_t* info) { appid_info_ = info; };

    appid_state_t appid_state() const { return appid_info_->state_; }
    void set_appid_state(appid_state_t state) {
      if(state == APPID_STATE_NEEDED)
          set_appid_needed();
      else if(appid_info_)
        appid_info_->state_ = state;
    }

    bool appid_completed() {
      if(!appid_info_)
          return false;
      else if(appid_info_->state_ == APPID_STATE_FOUND ||
           appid_info_->state_ == APPID_STATE_NOT_FOUND ||
           appid_info_->state_ == APPID_STATE_STOPPED ||
           appid_info_->state_ == APPID_STATE_ABORT) {
          return true;
      }
      return false;
    }

    void set_appid_needed() {
        if(appid_info_) {
            HAL_ASSERT(0);
        } else {
            appid_info_ = (appid_info_t *)HAL_CALLOC(hal::HAL_MEM_ALLOC_APPID_INFO,
                                            sizeof(appid_info_t));
        }
        appid_info_->state_ = APPID_STATE_NEEDED;
    }
    bool appid_in_progress() {
      if(!appid_info_)
          return false;
      else
          return (appid_info_->state_ == APPID_STATE_NEEDED) ||
                 (appid_info_->state_ == APPID_STATE_IN_PROGRESS) ;
    }
    bool appid_started() {
      return (appid_in_progress() || appid_completed()) ;
    }
    bool appid_needed() {
      if(!appid_info_)
          return false;
      else
          return appid_info_->state_ == APPID_STATE_NEEDED;
    }

    static inline void appid_info_clear_ids(appid_info_t& info) {
        info.id_count_ = 0;
        memset(info.ids_, 0, sizeof(info.ids_));
    }

    static inline void appid_info_init(appid_info_t& info) {
        memset(&info, 0, sizeof(appid_info_t));
    }

    static inline appid_id_t appid_info_id(const appid_info_t& info) {
        return info.id_count_ ? info.ids_[info.id_count_ - 1] : 0;
    }
    static inline appid_id_t appid_info_id(const appid_info_t& info, uint8_t idx) {
        assert(idx < APPID_MAX_DEPTH);
        return (idx < info.id_count_) ? info.ids_[idx] : 0;
    }
    static inline void appid_info_set_id(appid_info_t& info, appid_id_t id,
                           uint8_t idx = APPID_MAX_DEPTH) {
        assert(info.id_count_ < APPID_MAX_DEPTH);
        if (idx >= APPID_MAX_DEPTH) idx = info.id_count_;
        info.ids_[idx] = id;
        info.id_count_++;
    }

private:
    uint16_t                        redir_flags_;
    uint16_t                        hdr_len_total_;
    types::WRingType                chain_wring_type_;
    bool                            chain_pkt_enqueued_ : 1,
                                    pipeline_end_       : 1,
                                    redir_policy_applic_: 1,
                                    tcp_tls_proxy_flow_ : 1;
    pen_app_redir_header_v1_full_t  redir_miss_hdr_;
    uint8_t                         *redir_miss_pkt_p_;
    hal::proxy_flow_info_t          *proxy_flow_info_;
    hal::pd::cpupkt_ctxt_t          *arm_ctx_;
    uint32_t                        chain_flow_id_;
    uint32_t                        chain_rev_flow_id_;
    hal::flow_role_t                chain_rev_role_;    // rflow role
    uint8_t                         chain_qtype_;
    uint8_t                         chain_ring_;
    app_redir_verdict_t             chain_pkt_verdict_;
    app_redir_span_type_t           redir_span_type_;
    appid_info_t                    *appid_info_;
    bool                            appid_updated_;
};


inline app_redir_ctx_t* app_redir_ctx(fte::ctx_t& ctx,
                                      bool from_appid_feature = true) {
    if(from_appid_feature)
        return (app_redir_ctx_t*)ctx.feature_state();
    else
        return (app_redir_ctx_t*)ctx.feature_state(FTE_FEATURE_APP_REDIR_APPID);
}

inline app_redir_ctx_t& app_redir_ctxref(fte::ctx_t& ctx,
                                         bool from_appid_feature = false) {
    return *app_redir_ctx(ctx, from_appid_feature);
}

/*
 * Workaround (hopefully temporary) for the issue where, for a SPAN packet, 
 * P4 does not provide any parsed info beyond L2.
 */
inline void app_redir_rx_span_parse_workaround(fte::ctx_t& ctx)
{
    fte::cpu_rxhdr_t    *cpu_rxhdr;
    ether_header_t      *ethhdr;
    ipv4_header_t       *iphdr;
    ipv6_header_t       *iphdr6;
    pen_app_redir_header_v1_full_t *app_hdr;
    uint16_t            redir_flags;
    uint16_t            ethertype;
    uint16_t            payload_len;
    uint16_t            pkt_len;
    uint16_t            hdr_len;
    uint8_t             proto;

    if (ctx.pkt() && ctx.app_redir_pipeline()) {
        cpu_rxhdr = (fte::cpu_rxhdr_t *)ctx.cpu_rxhdr();
        if ((cpu_rxhdr->lkp_type != FLOW_KEY_LOOKUP_TYPE_MAC) &&
            (cpu_rxhdr->l3_offset < 0)) {

            app_hdr = (pen_app_redir_header_v1_full_t *)ctx.pkt();
            if ((ctx.pkt_len() >= PEN_RAW_REDIR_HEADER_V1_FULL_SIZE) &&
                (ntohs(app_hdr->app.h_proto) == PEN_APP_REDIR_ETHERTYPE) &&
                (app_hdr->ver.format == PEN_RAW_REDIR_V1_FORMAT)) {

                redir_flags = ntohs(app_hdr->raw.flags);
                if (redir_flags & PEN_APP_REDIR_SPAN_INSTANCE) {
                    assert((cpu_rxhdr->l2_offset >= 0) &&
                           (cpu_rxhdr->payload_offset >= 0));

                    ethhdr = (ether_header_t *)(ctx.pkt() + cpu_rxhdr->l2_offset);
                    ethertype = cpu_rxhdr->flags & CPU_FLAGS_VLAN_VALID ?
                                ntohs(((vlan_header_t*)ethhdr)->etype) :
                                ntohs(ethhdr->etype);

                    cpu_rxhdr->l3_offset = cpu_rxhdr->payload_offset;
                    cpu_rxhdr->l4_offset = -1;
                    proto = IPPROTO_RAW;
                    switch (ethertype) {

                    case ETH_P_IP:
                        iphdr = (ipv4_header_t *)(ctx.pkt() + cpu_rxhdr->l3_offset);
                        cpu_rxhdr->lkp_type = FLOW_KEY_LOOKUP_TYPE_IPV4;
                        hdr_len = iphdr->ihl * sizeof(uint32_t);
                        if (hdr_len >= sizeof(ipv4_header_t)) {
                            cpu_rxhdr->l4_offset = cpu_rxhdr->l3_offset + hdr_len;
                            proto = iphdr->protocol;
                        }
                        break;

                    case ETH_P_IPV6:
                        iphdr6 = (ipv6_header_t *)(ctx.pkt() + cpu_rxhdr->l3_offset);
                        cpu_rxhdr->lkp_type = FLOW_KEY_LOOKUP_TYPE_IPV6;

                        /*
                         * payload_len is the size of the payload, including any
                         * extension headers. It is 0 when a hop-by-hop extension
                         * header carries a jumbo payload option.
                         */
                        payload_len = ntohs(iphdr6->payload_len);
                        if (payload_len) {

                            /*
                             * pkt_len = packet length - (L7 hdr length + L2 hdr length)
                             */
                            pkt_len = ctx.pkt_len() - cpu_rxhdr->payload_offset;
                            if (pkt_len > payload_len) {
                                hdr_len = pkt_len - payload_len;
                                if (hdr_len >= sizeof(ipv6_header_t)) {
                                    cpu_rxhdr->l4_offset = cpu_rxhdr->l3_offset + 
                                                           hdr_len;
                                    proto = iphdr6->nexthdr;
                                }
                            }
                        }
                        break;

                    default:
                        break;
                    }

                    switch (proto) {

                    case IPPROTO_TCP:
                        cpu_rxhdr->payload_offset = cpu_rxhdr->l4_offset + 
                                                    sizeof(tcp_header_t);
                        break;

                    case IPPROTO_UDP:
                        cpu_rxhdr->payload_offset = cpu_rxhdr->l4_offset + 
                                                    sizeof(udp_header_t);
                        break;

                    case IPPROTO_ICMP:
                    case IPPROTO_ICMPV6:
                        cpu_rxhdr->payload_offset = cpu_rxhdr->l4_offset + 
                                                    sizeof(icmp_header_t);
                        break;

                    default:
                        cpu_rxhdr->payload_offset = -1;
                        break;
                    }
                }
            }
        }
    }
}

/*
 * Return true if the current packet was received for a raw flow in
 * the app redirect pipeline.
 */
inline bool app_redir_pkt_rx_raw(fte::ctx_t& ctx)
{
    if (ctx.pkt() && ctx.app_redir_pipeline()) {
        if (ctx.cpu_rxhdr()->qtype != APP_REDIR_PROXYR_QTYPE) {

#if APP_REDIR_VISIBILITY_USE_MIRROR_SESSION
            app_redir_rx_span_parse_workaround(ctx);
#endif
            return true;
        }
    }

    return false;
}

/*
 * Return true if app_redir owns responsibility for Tx (or suppress Tx)
 * of the current packet.
 */
inline bool app_redir_pkt_tx_ownership(fte::ctx_t& ctx)
{
    app_redir_ctx_t *redir_ctx = app_redir_ctx(ctx, false);

    if (redir_ctx && redir_ctx->redir_policy_applic()) {
        return !redir_ctx->tcp_tls_proxy_flow() && 
               !redir_ctx->chain_pkt_span_instance();
    }

    return false;
}

inline hal_ret_t
app_to_appid(std::string app, uint32_t& appid)
{
    // TODO: Hack for now
    if(app == "DNS") {
        appid = 617;
    } else if(app == "MYSQL") {
        appid = 747;
    } else if(app == "HTTP") {
        appid = 676;
    } else if(app == "HTTPS") {
        appid = 1122;
    } else if(app == "POSTGRES") {
        appid = 791;
    } else if(app == "MONGO") {
        appid = 2000000;
    } else if(app == "CASSANDRA") {
        appid = 2000001;
    } else if(app == "ANY") {
        appid = 0;
    } else {
        return HAL_RET_ERR;
    }
    return HAL_RET_OK;
}

}
}


#endif  // __APP_REDIR_CTX_H__
