// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __APP_REDIR_CTX_H__
#define __APP_REDIR_CTX_H__

#include <stdint.h>
#include "nic/include/fte.hpp"

namespace hal {
namespace app_redir {

const std::string FTE_FEATURE_APP_REDIR_MISS("pensando.io/app-redir:app-redir-miss");
const std::string FTE_FEATURE_APP_REDIR("pensando.io/app-redir:app-redir");
const std::string FTE_FEATURE_APP_REDIR_APPID("pensando.io/app-redir:app-redir-appid");
const std::string FTE_FEATURE_APP_REDIR_FINI("pensando.io/app-redir:app-redir-fini");

// Application redirect context
typedef enum {
    APP_REDIR_VERDICT_PASS,     // pass the packet
    APP_REDIR_VERDICT_BLOCK,    // block the packet
} app_redir_verdict_t;

class app_redir_ctx_t {
public:
    static void init(void *state)
    {
        app_redir_ctx_t *app_ctx = (app_redir_ctx_t*)state;
        app_ctx->redir_flags_            = 0;
        app_ctx->hdr_len_total_          = 0;
        app_ctx->chain_flow_id_          = 0;
        app_ctx->chain_rev_role_         = hal::FLOW_ROLE_NONE;
        app_ctx->chain_ring_             = 0;
        app_ctx->chain_qtype_            = APP_REDIR_RAWC_QTYPE;
        app_ctx->chain_wring_type_       = types::WRING_TYPE_APP_REDIR_RAWC;
        app_ctx->chain_pkt_verdict_      = APP_REDIR_VERDICT_PASS;
        app_ctx->pipeline_end_           = false;
        app_ctx->chain_pkt_enqueued_     = false;
        app_ctx->redir_policy_applic_    = false;
        app_ctx->tcp_tls_proxy_flow_     = false;
        app_ctx->redir_miss_pkt_p_       = nullptr;
        app_ctx->proxy_flow_info_        = nullptr;
        app_ctx->arm_ctx_                = nullptr;
        app_ctx->appid_updated_          = false;
        app_ctx->appid_info_             = { };
    };

    uint16_t redir_flags() const { return redir_flags_; }
    void set_redir_flags(uint16_t flags) { redir_flags_ = flags; }

    uint16_t hdr_len_total() const { return hdr_len_total_; }
    void set_hdr_len_total(uint16_t hdr_len_total) { hdr_len_total_ = hdr_len_total; }

    bool chain_pkt_enqueued() const { return chain_pkt_enqueued_; }
    void set_chain_pkt_enqueued(bool yesno) { chain_pkt_enqueued_ = yesno; }

    app_redir_verdict_t chain_pkt_verdict() const { return chain_pkt_verdict_; }
    void set_chain_pkt_verdict(app_redir_verdict_t verdict) { chain_pkt_verdict_ = verdict; }

    bool chain_pkt_verdict_pass(void)
    {
        return (chain_pkt_verdict_ != APP_REDIR_VERDICT_BLOCK);
    }

    bool chain_pkt_span_instance(void)
    {
        return !!(redir_flags_ & PEN_APP_REDIR_SPAN_INSTANCE);
    }

    bool pipeline_end() const { return pipeline_end_; }
    void set_pipeline_end(bool yesno) { pipeline_end_ = yesno; }

    bool redir_policy_applic() const { return redir_policy_applic_; }
    void set_redir_policy_applic(bool yesno) { redir_policy_applic_ = yesno; }

    bool tcp_tls_proxy_flow() const { return tcp_tls_proxy_flow_; }
    void set_tcp_tls_proxy_flow(bool yesno) { tcp_tls_proxy_flow_ = yesno; }

    uint8_t chain_qtype() const { return chain_qtype_; }
    void set_chain_qtype(uint8_t chain_qtype) { chain_qtype_ = chain_qtype; }

    uint8_t chain_ring() const { return chain_ring_; }
    void set_chain_ring(uint8_t chain_ring) { chain_ring_ = chain_ring; }

    uint32_t chain_flow_id() const { return chain_flow_id_; }
    void set_chain_flow_id(uint32_t chain_flow_id) { chain_flow_id_ = chain_flow_id; }

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

    bool appid_updated() const { return (appid_updated_ && appid_info_.id_count_ != 0); }
    void set_appid_updated(bool updated) { appid_updated_ = updated; }

    hal::appid_info_t &appid_info() { return appid_info_; };
    void set_appid_info(hal::appid_info_t &info) { appid_info_ = info; };

    hal::appid_state_t appid_state() const { return appid_info_.state_; }
    void set_appid_state(hal::appid_state_t state) {appid_info_.state_ = state; }

    bool appid_completed() {
        if(appid_info_.state_ == hal::APPID_STATE_FOUND ||
           appid_info_.state_ == hal::APPID_STATE_NOT_FOUND ||
           appid_info_.state_ == hal::APPID_STATE_STOPPED ||
           appid_info_.state_ == hal::APPID_STATE_ABORT) {
          return true;
        }
        return false;
    }

    void set_appid_needed() {
        if(!appid_started())
            appid_info_.state_ = hal::APPID_STATE_NEEDED;
        else
            HAL_ASSERT(0);
    }
    void set_appid_not_needed() {
        if(!appid_started())
            appid_info_.state_ = hal::APPID_STATE_NOT_NEEDED;
        else
            HAL_ASSERT(0);
    }
    bool appid_in_progress() {
        return (appid_info_.state_ == hal::APPID_STATE_NEEDED) ||
               (appid_info_.state_ == hal::APPID_STATE_IN_PROGRESS) ;
    }
    bool appid_started() {
        return (appid_in_progress() || appid_completed()) ;
    }
    bool appid_needed() {
        return appid_info_.state_ == hal::APPID_STATE_NEEDED;
    }

    static inline void appid_info_clear_ids(hal::appid_info_t& info) { info.id_count_ = 0; }

    static inline void appid_info_init(hal::appid_info_t& info) {
        info.state_ = hal::APPID_STATE_INIT;
        info.cleanup_handle_ = nullptr;
        appid_info_clear_ids(info);
    }

    static inline appid_id_t appid_info_id(const hal::appid_info_t& info) {
        return info.id_count_ ? info.ids_[info.id_count_ - 1] : 0;
    }
    static inline appid_id_t appid_info_id(const hal::appid_info_t& info, uint8_t idx) {
        assert(idx < hal::APPID_MAX_DEPTH);
        return (idx < info.id_count_) ? info.ids_[idx] : 0;
    }
    static inline void appid_info_set_id(hal::appid_info_t& info, appid_id_t id,
                           uint8_t idx = hal::APPID_MAX_DEPTH) {
        assert(info.id_count_ < hal::APPID_MAX_DEPTH);
        if (idx >= hal::APPID_MAX_DEPTH) idx = info.id_count_;
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
    app_redir_verdict_t             chain_pkt_verdict_;
    pen_app_redir_header_v1_full_t  redir_miss_hdr_;
    uint8_t                         *redir_miss_pkt_p_;
    hal::proxy_flow_info_t          *proxy_flow_info_;
    hal::pd::cpupkt_ctxt_t          *arm_ctx_;
    uint32_t                        chain_flow_id_;
    hal::flow_role_t                chain_rev_role_;    // rflow role
    uint8_t                         chain_qtype_;
    uint8_t                         chain_ring_;
    hal::appid_info_t               appid_info_;
    bool                            appid_updated_;
};


inline app_redir_ctx_t* app_redir_ctx(fte::ctx_t& ctx,
                                      bool from_appid_feature = false) {
    if(from_appid_feature)
        return (app_redir_ctx_t*)ctx.feature_state();
    else
        return (app_redir_ctx_t*)ctx.feature_state(FTE_FEATURE_APP_REDIR);
}

inline app_redir_ctx_t& app_redir_ctxref(fte::ctx_t& ctx,
                                         bool from_appid_feature = false) {
    if(from_appid_feature)
        return *(app_redir_ctx_t*)ctx.feature_state();
    else
        return *(app_redir_ctx_t*)ctx.feature_state(FTE_FEATURE_APP_REDIR);
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
    }else if(app == "HTTPS") {
        appid = 1122;
    }else if(app == "ANY") {
        appid = 0;
    }
    return HAL_RET_OK;
}

}
}


#endif  // __APP_REDIR_CTX_H__
