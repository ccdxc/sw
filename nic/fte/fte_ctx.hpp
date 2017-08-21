#pragma once

#include <base.h>
#include <session.hpp>

namespace fte {

// Flow update codes
#define FTE_FLOW_UPDATE_CODES(ENTRY)                                    \
    ENTRY(FLOWUPD_ACTION,        0, "update flow action (allow/deny)")  \
    ENTRY(FLOWUPD_HEADER_MODIFY, 1, "modify the header")                \
    ENTRY(FLOWUPD_HEADER_PUSH,   2, "push header")                      \
    ENTRY(FLOWUPD_HEADER_POP,    3, "pop header")                       \
    ENTRY(FLOWUPD_CONN_TRACK,    4, "connection tracking")              \
    ENTRY(FLOWUPD_FWDING_INFO,   5, "fwding info")                      \
    
DEFINE_ENUM(flow_update_type_t, FTE_FLOW_UPDATE_CODES)
#undef FTE_FLOW_UPDATE_CODES

typedef struct flow_update_s flow_update_t;
struct flow_update_s {
    flow_update_type_t type;
    union {
        struct {
            uint8_t deny      : 1;     // deny flow
        } __PACK__ action;
        struct {
        } __PACK__ header_modify;
        struct {
        } __PACK__ header_push;
        struct {
        } __PACK__ header_pop;
        struct {
            uint8_t enable: 1;    // enable connection tracking
            uint8_t state: 4;     // TCP state
            int32_t syn_ack_delta;
        } __PACK__ conn_track;
        struct {
            uint8_t  qid_en: 1;
            uint64_t lif : 11;
            uint64_t qtype: 3;
            uint64_t qid: 24;
        } __PACK__ fwding_info;
    };
}__PACK__;

// TODO (goli) temp struct until we define the header
struct phv_t {
    uint64_t lif : 11;
    uint64_t qtype: 3;
    uint64_t qid: 24;

    uint8_t lkp_type;
    uint8_t lkp_dir;

    uint16_t lkp_vrf;
    uint8_t  lkp_proto;
    uint8_t lkp_src[16];
    uint8_t lkp_dst[16];
    uint16_t lkp_sport;
    uint16_t lkp_dport;
};

// FTE lif qid (TODO - uses std types)
typedef struct lifqid_s lifqid_t;
struct lifqid_s {
    uint64_t lif : 11;
    uint64_t qtype: 3;
    uint64_t qid : 24;
} __PACK__;

inline std::ostream& operator<<(std::ostream& os, const lifqid_t& lifq)
{
    return os << fmt::format("{{lif={}, qtype={}, qid={}}}",
                             lifq.lif, lifq.qtype, lifq.qid);
}

// FTE context passed between features in a pipeline
typedef struct ctx_s ctx_t;
struct ctx_s {
    uint8_t               drop:1;
    uint8_t               flow_miss:1;
    uint8_t               rflow_valid:1;

    lifqid_t              arm_lifq;

    phv_t                 *phv;
    uint8_t                *pkt;
    size_t                pkt_len;

    hal::session_t        *session;  // Not set for flow miss first packet
    hal::session_cfg_t    session_cfg;
    hal::flow_cfg_t       iflow;
    hal::flow_cfg_t       rflow;
    hal::flow_cfg_t       *flow; // current pkt flow (points to either iflow or rflow)


    hal::tenant_t         *tenant;
    hal::l2seg_t          *sl2seg;
    hal::l2seg_t          *dl2seg;
    hal::if_t             *sif;
    hal::if_t             *dif;
    hal::ep_t             *sep;
    hal::ep_t             *dep;
} __PACK__;

extern hal_ret_t ctx_init(ctx_t &ctx, phv_t *phv, uint8_t *pkt, size_t pkt_len);
extern hal_ret_t ctx_update_iflow(ctx_t&, const flow_update_t&);
extern hal_ret_t ctx_update_rflow(ctx_t&, const flow_update_t&);

// Update flow of the current packet (i or r)
extern hal_ret_t ctx_update_flow(ctx_t&, const flow_update_t&);
// Update reverse flow of the current packet's flow (i or r)
extern hal_ret_t ctx_update_reverse_flow(ctx_t&, const flow_update_t&);

} // namespace fte
