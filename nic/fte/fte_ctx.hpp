#pragma once

#include "nic/include/base.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include "nic/hal/src/internal/rawrcb.hpp"
#include "nic/include/cpupkt_headers.hpp"
#include "nic/include/cpupkt_api.hpp"
#include <netinet/ether.h>
#include "gen/proto/types.pb.h"
#include "nic/include/app_redir_headers.hpp"
#include "nic/include/app_redir_shared.h"
#include "gen/proto/fwlog.pb.h"
#include "nic/utils/agent_api/agent_api.hpp"
#include "nic/fte/fte_core.hpp"

using fwlog::FWEvent;

#define MAX_FEATURES 255

namespace fte {

// FTE pipeline events
#define FTE_PIPELINE_EVENT_ENTRIES(ENTRY)              \
    ENTRY(FTE_SESSION_CREATE,            0, "create")  \
    ENTRY(FTE_SESSION_UPDATE,            1, "update")  \
    ENTRY(FTE_SESSION_DELETE,            2, "delete")  \
    ENTRY(FTE_SESSION_GET,               3, "get")     

DEFINE_ENUM(pipeline_event_t, FTE_PIPELINE_EVENT_ENTRIES)
#undef FTE_PIPELINE_EVENT_ENTRIES

// Flow update codes
#define FTE_FLOW_UPDATE_CODES(ENTRY)                                            \
    ENTRY(FLOWUPD_ACTION,               0, "update flow action (allow/deny)")   \
    ENTRY(FLOWUPD_HEADER_REWRITE,       1, "modify the header")                 \
    ENTRY(FLOWUPD_HEADER_PUSH,          2, "push header")                       \
    ENTRY(FLOWUPD_HEADER_POP,           3, "pop header")                        \
    ENTRY(FLOWUPD_FLOW_STATE,           4,  "connection tracking state")        \
    ENTRY(FLOWUPD_FWDING_INFO,          5, "fwding info")                       \
    ENTRY(FLOWUPD_LKP_INFO,             6, "flow lookup update")                \
    ENTRY(FLOWUPD_MCAST_COPY,           7, "flow mcast copy update")            \
    ENTRY(FLOWUPD_INGRESS_INFO,         8, "ingress info")                      \
    ENTRY(FLOWUPD_MIRROR_INFO,          9, "mirror info")                       \
    ENTRY(FLOWUPD_QOS_INFO,             10, "qos info")                         \
    ENTRY(FLOWUPD_EXPORT_INFO,          11, "export info")                      \
    ENTRY(FLOWUPD_LKP_KEY,              12, "flow key")                         \
    ENTRY(FLOWUPD_AGING_INFO,           13, "Session Aging info")               \
    ENTRY(FLOWUPD_SFW_INFO,             14, "SFW info")                         \

DEFINE_ENUM(flow_update_type_t, FTE_FLOW_UPDATE_CODES)
#undef FTE_FLOW_UPDATE_CODES

// Header update codes
#define FTE_HEADER_UPDATE_CODES(ENTRY)          \
    ENTRY(HEADER_REWRITE,   0, "rewrite")       \
    ENTRY(HEADER_PUSH,   1, "push")             \
    ENTRY(HEADER_POP,   2, "pop")               \

DEFINE_ENUM(header_update_type_t, FTE_HEADER_UPDATE_CODES)
#undef FTE_HEADER_UPDATE_CODES

// Header types
typedef uint32_t header_type_t;

static const header_type_t FTE_HEADER_ether       = 0x00000001;
static const header_type_t FTE_HEADER_ipv4        = 0x00000002;
static const header_type_t FTE_HEADER_ipv6        = 0x00000004;
static const header_type_t FTE_HEADER_tcp         = 0x00000008;
static const header_type_t FTE_HEADER_udp         = 0x00000010;
static const header_type_t FTE_HEADER_icmp        = 0x00000020;
static const header_type_t FTE_HEADER_vxlan       = 0x00000040;
static const header_type_t FTE_HEADER_vxlan_gpe   = 0x00000080;
static const header_type_t FTE_HEADER_geneve      = 0x00000100;
static const header_type_t FTE_HEADER_nvgre       = 0x00000200;
static const header_type_t FTE_HEADER_gre         = 0x00000400;
static const header_type_t FTE_HEADER_ip_in_ip    = 0x00000800;
static const header_type_t FTE_HEADER_erspan      = 0x00001000;
static const header_type_t FTE_HEADER_mpls        = 0x00002000;
static const header_type_t FTE_HEADER_ipsec_esp   = 0x00004000;

static const header_type_t FTE_L2_HEADERS = FTE_HEADER_ether;
static const header_type_t FTE_L3_HEADERS = FTE_HEADER_ipv4 | FTE_HEADER_ipv6;
static const header_type_t FTE_L4_HEADERS = FTE_HEADER_tcp | FTE_HEADER_udp | FTE_HEADER_icmp;
static const header_type_t FTE_L2ENCAP_HEADERS = FTE_HEADER_mpls;
static const header_type_t FTE_L3ENCAP_HEADERS = FTE_HEADER_vxlan | FTE_HEADER_vxlan_gpe |
    FTE_HEADER_geneve | FTE_HEADER_nvgre | FTE_HEADER_gre | FTE_HEADER_ip_in_ip |
    FTE_HEADER_erspan | FTE_HEADER_ipsec_esp;
static const header_type_t FTE_ENCAP_HEADERS = FTE_L2ENCAP_HEADERS | FTE_L3ENCAP_HEADERS;

// checks if no bits are set
#define BITS_ZERO(bits) ((bits) == 0)
// checks if no of bits set is zero or  one
#define BITS_ZERO_OR_ONE(bits) (((bits) & ((bits)-1)) == 0)
// checks exactly one bit is set
#define BITS_ONE(bits) (((bits) != 0) && BITS_ZERO_OR_ONE(bits))

// checks header combination is vlaid (no dups)
static inline bool valid_headers(header_type_t header_types) {
    return BITS_ZERO_OR_ONE(header_types & FTE_L2_HEADERS) &&
        BITS_ZERO_OR_ONE(header_types & FTE_L3_HEADERS) &&
        BITS_ZERO_OR_ONE(header_types & (FTE_L4_HEADERS | FTE_ENCAP_HEADERS));
}

// checks header combination is a valid tunnel header group
// L2 + L2ENCAP or  L2 + L3 + L3ENCAP
static inline bool valid_tunnel_headers(header_type_t header_types) {
    return BITS_ONE(header_types & FTE_L2_HEADERS) &&
        ((BITS_ONE(header_types & FTE_L2ENCAP_HEADERS) &&
          BITS_ZERO(header_types & (FTE_L3_HEADERS|FTE_L3ENCAP_HEADERS))) ||
         (BITS_ONE(header_types & FTE_L3_HEADERS) &&
          BITS_ONE(header_types & FTE_L3ENCAP_HEADERS) &&
          BITS_ZERO(header_types & FTE_L2ENCAP_HEADERS)));
}

// Header fields
typedef struct header_fld_s {
    uint32_t dummy:1; //place holder fld for empty headers
    uint32_t smac:1;
    uint32_t dmac:1;
    uint32_t vlan_id:1;
    uint32_t dot1p:1;
    uint32_t svrf_id:1;
    uint32_t dvrf_id:1;
    uint32_t sip:1;
    uint32_t dip:1;
    uint32_t ttl:1;
    uint32_t dscp:1;
    uint32_t sport:1;
    uint32_t dport:1;
    uint32_t entropy:1;
    uint32_t vrf_id:1;
    uint32_t eompls:1;
    uint32_t label0:1;
    uint32_t label1:1;
    uint32_t label2:1;
} __PACK__ header_fld_t;

#define HEADER_SET_FLD(obj, hdr, fld, val)          \
    {                                                   \
        (obj).valid_hdrs  |= fte::FTE_HEADER_ ## hdr;   \
        (obj).valid_flds.fld = 1;                       \
        (obj).hdr.fld = val;                            \
    }

#define HEADER_COPY_FLD(dst, src, hdr, fld)         \
    if ((src).valid_flds.fld) {                     \
        HEADER_SET_FLD(dst, hdr, fld, (src).hdr.fld);   \
    }

#define HEADER_FORMAT_FLD(out, obj, header, fld)                    \
    if (obj.valid_flds.fld) {                                       \
        out.write(#header "." #fld "={}, ", obj.header.fld);        \
    }

#define HEADER_FORMAT_IPV4_FLD(out, obj, header, fld)                   \
    if (obj.valid_flds.fld) {                                           \
        out.write(#header "." #fld "={}, ", ipv4addr2str(obj.header.fld)); \
    }

typedef struct header_rewrite_info_s {
    header_type_t valid_hdrs;
    header_fld_t  valid_flds;
    struct {
        uint8_t dec_ttl:1;
    } __PACK__ flags;

    struct {
        ether_addr smac;
        ether_addr dmac;
        uint16_t   vlan_id;
        uint8_t    dot1p;
    } __PACK__ ether;

    union {
        struct {
            uint32_t    svrf_id;
            uint32_t    dvrf_id;
            ipv4_addr_t sip;
            ipv4_addr_t dip;
            uint8_t     ttl;
            uint8_t     dscp;
        } __PACK__ ipv4;
        struct {
            uint32_t    svrf_id;
            uint32_t    dvrf_id;
            ipv6_addr_t sip;
            ipv6_addr_t dip;
            uint8_t     ttl;
            uint8_t     dscp;
        } __PACK__ ipv6;
    };

    union {
        struct {
            uint16_t sport;
            uint16_t dport;
        } __PACK__ tcp;
        struct {
            uint16_t sport;
            uint16_t dport;
        } __PACK__ udp;
    };
} __PACK__ header_rewrite_info_t;

std::ostream& operator<<(std::ostream& os, const header_rewrite_info_t& val);

typedef struct mpls_label_s {
    uint32_t label;
    uint8_t exp;
    uint8_t bos;
    uint8_t ttl;
} __PACK__ mpls_label_t;

std::ostream& operator<<(std::ostream& os, const mpls_label_t& val);

typedef struct header_push_info_s {
    header_type_t valid_hdrs;
    header_fld_t  valid_flds;

    struct {
        ether_addr smac;
        ether_addr dmac;
        uint16_t   vlan_id;
    } __PACK__ ether;

    union {
        struct {
            ipv4_addr_t sip;
            ipv4_addr_t dip;
        } __PACK__ ipv4;
        struct {
            ipv6_addr_t sip;
            ipv6_addr_t dip;
        } __PACK__ ipv6;
    };

    union {
        struct {
            uint32_t vrf_id;
        } vxlan, vxlan_gpe, nvgre, geneve;
        struct {
            uint8_t dummy; // placeholder to make the HEADER_FLD_SET macro happy
        } gre, erspan, ip_in_ip, ipsec_esp;
        struct {
            uint8_t eompls;
            mpls_label_t label0, label1, label2;
        } mpls;
    };
} __PACK__ header_push_info_t;

std::ostream& operator<<(std::ostream& os, const header_push_info_t& val);

typedef struct header_pop_info_s {
    //empty
} __PACK__ header_pop_info_t;

std::ostream& operator<<(std::ostream& os, const header_pop_info_t& val);

typedef hal::flow_state_t flow_state_t;
typedef session::FlowAction flow_action_t;

typedef struct fwding_info_s {
    uint64_t lport:11;
    uint64_t qid_en:1;
    uint64_t qtype:3;
    uint64_t qid:24;
    hal::ep_t *dep;
    hal::if_t *dif;
    hal::l2seg_t *dl2seg;
    bool update_qos;
} fwding_info_t;

std::ostream& operator<<(std::ostream& os, const fwding_info_t& val);

typedef struct mcast_info_s {
    uint8_t  mcast_en;
    uint16_t mcast_ptr;
    uint16_t proxy_mcast_ptr;
} mcast_info_t;

std::ostream& operator<<(std::ostream& os, const mcast_info_t& val);

typedef struct aging_info_s {
    uint32_t idle_timeout;
} aging_info_t;

std::ostream& operator<<(std::ostream& os, const aging_info_t& val);

typedef struct sfw_flow_info_s {
    uint8_t   skip_sfw_reval;
    uint8_t   sfw_action;
    uint64_t  sfw_rule_id;
} sfw_flow_info_t;

std::ostream& operator<<(std::ostream& os, const sfw_flow_info_t& val);

typedef struct ingress_info_s {
    // Expected source interface for host pinning mode
    hal::if_t *expected_sif;
    uint32_t   hw_lif_id;
} ingress_info_t;

std::ostream& operator<<(std::ostream& os, const ingress_info_t& val);

typedef struct mirror_info_s {
    bool mirror_en;
    uint8_t egr_mirror_session;
    uint8_t ing_mirror_session;
    uint8_t proxy_egr_mirror_session;
    uint8_t proxy_ing_mirror_session;
} mirror_info_t;

std::ostream& operator<<(std::ostream& os, const mirror_info_t& val);

typedef struct export_info_s {
    uint8_t export_en;
    uint8_t export_id1;
    uint8_t export_id2;
    uint8_t export_id3;
    uint8_t export_id4;
} export_info_t;

std::ostream& operator<<(std::ostream& os, const export_info_t& val);

typedef struct qos_info_s {
    bool qos_class_en;
    uint32_t qos_class_id;
} qos_info_t;

std::ostream& operator<<(std::ostream& os, const qos_info_t& val);

typedef struct lkp_info_s {
    uint32_t vrf_hwid; // vrf/l2seg id of the flow key
} lkp_info_t;

std::ostream& operator<<(std::ostream& os, const lkp_info_t& val);

typedef struct flow_update_s {
    flow_update_type_t type;
    union {
        session::FlowAction action;
        header_rewrite_info_t header_rewrite;
        header_push_info_t header_push;
        header_pop_info_t header_pop;
        flow_state_t flow_state;
        fwding_info_t fwding;
        ingress_info_t ingress_info;
        qos_info_t qos_info;
        lkp_info_t lkp_info;
        mcast_info_t mcast_info;
        mirror_info_t mirror_info;
        export_info_t export_info;
        hal::flow_key_t    key;
        aging_info_t  aging_info;
        sfw_flow_info_t sfw_info;
    };
}__PACK__ flow_update_t;


typedef struct header_update_s {
    header_update_type_t type;
    union {
        header_rewrite_info_t header_rewrite;
        header_push_info_t header_push;
        header_pop_info_t header_pop;
    };
} __PACK__ header_update_t;

const lifqid_t FLOW_MISS_LIFQ = {HAL_LIF_CPU, 0, types::CPUCB_ID_FLOWMISS};
const lifqid_t TCP_CLOSE_LIFQ = {HAL_LIF_CPU, 0, types::CPUCB_ID_TCP_CLOSE};
const lifqid_t ALG_CFLOW_LIFQ = {HAL_LIF_CPU, 0, types::CPUCB_ID_RELIABLE_COPY};
const lifqid_t NACL_REDIRECT_LIFQ = {HAL_LIF_CPU, 0, types::CPUCB_ID_NACL_REDIRECT};
const lifqid_t NACL_LOG_LIFQ = {HAL_LIF_CPU, 0, types::CPUCB_ID_NACL_LOG};
const lifqid_t QUIESCE_LIFQ   = {HAL_LIF_CPU, 0, types::CPUCB_ID_QUIESCE};
const lifqid_t TCP_PROXY_LIFQ = {SERVICE_LIF_TCP_PROXY, 0, 0};
const lifqid_t TLS_PROXY_LIFQ = {SERVICE_LIF_TLS_PROXY, 0, 0};
const lifqid_t APP_REDIR_LIFQ = {SERVICE_LIF_APP_REDIR, 0, 0};
const lifqid_t FTE_SPAN_LIFQ = {HAL_LIF_CPU, 0, types::CPUCB_ID_FTE_SPAN};

std::string hex_str(const uint8_t *buf, size_t sz);

#define MAX_FEATURE_NAME 64
typedef char feature_name_t[MAX_FEATURE_NAME];

//------------------------------------------------------------------------------
// Feature sepcific session state
// Used for storing as dllist in the hal::session_t
// (this is embedded inside feature state slab)
//------------------------------------------------------------------------------
typedef struct feature_session_state_s {
    fte::feature_name_t feature_name;                    // Name of the feature
    dllist_ctxt_t       session_feature_lentry;  // session's feature list context
} __PACK__ feature_session_state_t;

class ctx_t;

typedef void (*completion_handler_t) (ctx_t &ctx, bool fail);

//------------------------------------------------------------------------------
// Feature specific state
// Used for storing as an array inside fte::ctx_t for faster lookups using featutre id
//------------------------------------------------------------------------------
struct feature_state_t {
    const char                   *name;  // name of the feature
    void                         *ctx_state;
    feature_session_state_t      *session_state;
    completion_handler_t          completion_handler;
};

uint16_t feature_id (const std::string &name);
size_t feature_state_size (uint16_t *num_features);
void feature_state_init (feature_state_t *feature_state, uint16_t num_features);
uint16_t get_num_features (void);
const std::string& feature_id_to_name (uint16_t feature_id);

// Callback definition for FTE to call per queued
// packet for cleanup
typedef void (*post_xmit_cb_t)(uint8_t *pkt);

// pkt info for queued tx packets
typedef struct txpkt_info_s txpkt_info_t;
struct txpkt_info_s {
    hal::pd::cpu_to_p4plus_header_t cpu_header;
    hal::pd::p4plus_to_p4_header_t  p4plus_header;
    uint8_t                        *pkt;
    size_t                          pkt_len;
    lifqid_t                        lifq;   // Dest lif/qtype/qid
    uint8_t                         ring_number; // arm ring
    types::WRingType                wring_type;
    post_xmit_cb_t                  cb;
};

typedef hal::pd::p4_to_p4plus_cpu_pkt_t cpu_rxhdr_t;

//------------------------------------------------------------------------
// FTE IPC flow logging info - This data structure holds all the info
// that needs to be populated in the IPC protobuf (fwlog.proto). We dont
// want to hold the protobuf class in the context for performance reasons
// If any new field needs to be populated in the pipeline, it needs to be
// added to this data structure.
// -----------------------------------------------------------------------
typedef struct fte_flow_log_info_s {
    uint64_t               rule_id;
    nwsec::SecurityAction  sfw_action;
    nwsec::ALGName         alg;
    hal_handle_t           parent_session_id;
    uint64_t               iflow_packets;
    uint64_t               iflow_bytes;
    uint64_t               rflow_packets;
    uint64_t               rflow_bytes;
} fte_flow_log_info_t;

// API to increment per-instance stats per feature
void incr_inst_feature_stats (uint16_t feature_id, hal_ret_t rc=HAL_RET_OK,
                              bool set_rc=false);

void incr_inst_fte_rx_stats (cpu_rxhdr_t *rxhdr, size_t pkt_len);
void incr_inst_fte_tx_stats (size_t pkt_len);
void incr_inst_fte_tx_stats_batch(uint16_t pktcount);
void incr_fte_retransmit_packets(void);

typedef struct fte_feature_stats_s {
    uint64_t        drop_pkts;                 // Number of packets dropped by the feature
    uint64_t        drop_reason[HAL_RET_ERR];  // Number of drops seen per drop reason code
} fte_feature_stats_t;

//----------------------------------------------------------------------------------
// FTE per-thread stats
//----------------------------------------------------------------------------------

struct fte_stats_t;

#define FTE_MAX_CPU_QUEUES 3 // ARQ, ASCQ, ASQ
typedef struct fte_txrx_stats_s {
    uint64_t             flow_miss_pkts;
    uint64_t             redirect_pkts;
    uint64_t             cflow_pkts;
    uint64_t             tcp_close_pkts;
    uint64_t             tls_proxy_pkts;
    int                  num_cpu_pkt_queues;
    hal::pd::cpupkt_global_info_t glinfo;
    hal::pd::cpupkt_qinfo_stats_t qinfo[FTE_MAX_CPU_QUEUES];
} __PACK__ fte_txrx_stats_t;

typedef struct fte_hbm_stats_ {
    struct {
        uint64_t          cps;                         // Number of connections per second processed by this FTE
        uint64_t          cps_hwm;                     // Max. Number of connections per second processed by this FTE at any time
        uint64_t          pps;                         // Number of packets per second processed by this FTE
        uint64_t          pps_hwm;                     // Max. Number of packets per second processed by this FTE at any time
    } __PACK__ cpsstats;
    struct {
        uint64_t          flow_miss_pkts;              // Number of flow miss packets processed by this FTE
        uint64_t          flow_retransmit_pkts;        // Number of flow retransmit packets processed by this FTE
        uint64_t          redirect_pkts;               // Number of NACL redirect packets processed by this FTE
        uint64_t          cflow_pkts;                  // Number of ALG control flow packets processed by this FTE
        uint64_t          tcp_close_pkts;              // Number of TCP close packets processed by this FTE
        uint64_t          tls_proxy_pkts;              // Number of TLS proxy packets processed by this FTE
        uint64_t          fte_span_pkts;               // Number of FTE Span packets processed by this FTE
        uint64_t          softq_req;                   // Number of softq requests processed by this FTE
        uint64_t          queued_tx_pkts;              // Number of packets queued from this FTE to be transmitted
        uint64_t          freed_tx_pkts;               // Number of dropped/non-flowmiss packets for which the CPU buffers are freed 
        uint64_t          max_session_drop_pkts;       // Number of pkts dropped because of max sessions
    } __PACK__ qstats;
} __PACK__ fte_hbm_stats_t;

struct fte_stats_t {
    fte_hbm_stats_t      *fte_hbm_stats;               // FTE CPS & Q stats
    uint64_t              fte_errors[HAL_RET_ERR];     // Number of FTE errors encountered
    fte_feature_stats_t   feature_stats[MAX_FEATURES]; // Number of drops per feature
};
std::ostream& operator<<(std::ostream& os, const fte_stats_t& val);

class flow_t;
class ctx_t;

static const uint8_t MAX_FLOW_KEYS = 3;

// FTE context passed between features in a pipeline
class ctx_t {
public:
    static const uint8_t MAX_STAGES = hal::MAX_SESSION_FLOWS; // max no.of times a pkt enters p4 pipeline
    static const uint16_t MAX_QUEUED_PKTS = 512;  // max queued pkts for tx, to enable batching
    static const uint8_t MAX_QUEUED_HANDLERS = 16; // max queued completion handlers

    hal_ret_t init(cpu_rxhdr_t *cpu_rxhdr, uint8_t *pkt, size_t pkt_len, bool copied_pkt,
                   flow_t iflow[], flow_t rflow[],
                   feature_state_t feature_state[], uint16_t num_features);
    hal_ret_t init(SessionSpec *spec, SessionStatus *status, SessionStats *stats,
                   SessionResponse *rsp, flow_t iflow[], flow_t rflow[],
                   feature_state_t feature_state[], uint16_t num_features);
    hal_ret_t init(hal::session_t *session, flow_t iflow[], flow_t rflow[],
                   feature_state_t feature_state[], uint16_t num_features);
    hal_ret_t process(void);
    void process_tcp_queues(void *tcp_ctx);

    hal_ret_t update_flow(const flow_update_t& flowupd, const hal::flow_role_t role);

    hal_ret_t update_flow(const flow_update_t& flowupd);


    // Flow has drop action set (installs flow with drop action)
    bool drop_flow() { return drop_flow_; }

    // Flow configured to mirror to a proxy
    bool get_proxy_mirror_flow(hal::flow_role_t role);
    bool proxy_mirror_flow() { return get_proxy_mirror_flow(role_); }

    // Drop packet
    bool drop() const { return drop_; }
    void set_drop() {
        drop_ = true;
        // name seems to be null sometimes.
        // HAL_TRACE_DEBUG("fte::set_drop feature={}", feature_name_);
        HAL_TRACE_DEBUG("fte::set_drop feature_id: {}", feature_id_);
    }


    // role of the current flow being processed
    hal::flow_role_t role() const { return role_; }
    void set_role(hal::flow_role_t role) { role_ = role; }

    // flow key of the current pkt's flow
    const hal::flow_key_t& key() const { return key_; }
    void set_key(const hal::flow_key_t& key) { key_ = key; }
    // Get key based on role (role defualts to current flow's role if not specified)
    const hal::flow_key_t& get_key(hal::flow_role_t role = hal::FLOW_ROLE_NONE) const;


    // direction of the current pkt
    hal::flow_direction_t direction() const { return (hal::flow_direction_t)(key().dir); }
    // direction of the currnet flow being processed
    hal::flow_direction_t flow_direction() const { return (hal::flow_direction_t)(get_key().dir); }


    // Following are valid only for packets punted to ARM
    const cpu_rxhdr_t* cpu_rxhdr() const { return cpu_rxhdr_; }
    uint8_t* pkt() const { return pkt_; }
    size_t pkt_len() const { return pkt_len_; }
    void set_pkt(cpu_rxhdr_t *rxhdr, uint8_t *pkt, size_t len) { cpu_rxhdr_ = rxhdr; pkt_ = pkt; pkt_len_ = len; }

    // Queue pkt for tx (in case of flow_miss rx pkt is
    // transmitted if no pkts are queued for tx)
    hal_ret_t queue_txpkt(uint8_t *pkt, size_t pkt_len,
                          hal::pd::cpu_to_p4plus_header_t *cpu_header = NULL,
                          hal::pd::p4plus_to_p4_header_t  *p4plus_header = NULL,
                          uint16_t dest_lif = HAL_LIF_CPU,
                          uint8_t  qtype = CPU_ASQ_QTYPE,
                          uint32_t qid = CPU_ASQ_QID,
                          uint8_t  ring_number = CPU_SCHED_RING_ASQ,
                          types::WRingType wring_type = types::WRING_TYPE_ASQ,
                          post_xmit_cb_t cb = NULL);
    hal_ret_t send_queued_pkts(hal::pd::cpupkt_ctxt_t* arm_ctx);
    hal_ret_t send_queued_pkts_new(hal::pd::cpupkt_ctxt_t* arm_ctx);

    //proto spec is valid when flow update triggered via hal proto api
    bool protobuf_request() { return sess_spec_ != NULL; }
    session::SessionSpec* sess_spec() {return sess_spec_; }
    session::SessionResponse* sess_resp() {return sess_resp_; }
    session::SessionGetResponse* sess_get_resp() { return sess_get_resp_; }
    void set_sess_get_resp(session::SessionGetResponse* val) { sess_get_resp_ = val; }
    hal::session_t* session() { return session_; }
    bool existing_session() const { return session_ != NULL; }

    const lifqid_t& arm_lifq() const { return arm_lifq_; }
    void set_arm_lifq(const lifqid_t& arm_lifq) {arm_lifq_= arm_lifq;}

    uint8_t stage() const { return role_ == hal::FLOW_ROLE_INITIATOR ? istage_ : rstage_; }
    hal_ret_t advance_to_next_stage();

    // name of the feature being executed
    const char* feature_name() const { return feature_name_; }

    void set_feature_name(const char *name, uint16_t feature_id) {
        feature_name_ = name;
        feature_id_ = feature_id;
    }

    void set_feature_name(const char *name) {
        feature_name_ = name;
        feature_id_ = feature_id(name);
    }

    // return staus of the feature handler
    hal_ret_t feature_status() const { return feature_status_; }
    void set_feature_status(hal_ret_t ret) {
         if (ret != HAL_RET_OK)
             incr_inst_feature_stats(feature_id_, ret, true);

         feature_status_ = ret;
    }

    // completion handlere rgistrations, registered handlers are called at the end of the
    // packet processing (after updating the gft)
    // callback's 'fail' argument is set to true if the pkt processing failed and
    // flow is not getting installed.
    hal_ret_t register_completion_handler(completion_handler_t handler) {
        feature_state_[feature_id_].completion_handler = handler;
        HAL_TRACE_VERBOSE("fte: feature={} queued completion handler {:p}",
                         feature_name_, (void*)handler);
        return HAL_RET_OK;
    }

    bool completion_handler_registered() {
        return feature_state_[feature_id_].completion_handler != nullptr;
    }

    bool flow_miss() const { return (arm_lifq_== FLOW_MISS_LIFQ); }
    bool app_redir_pipeline() const { return (arm_lifq_.lif == APP_REDIR_LIFQ.lif); }
    bool tcp_proxy_pipeline() const { return (arm_lifq_.lif == TCP_PROXY_LIFQ.lif); }
    bool tcp_close() const { return (arm_lifq_ == TCP_CLOSE_LIFQ); }
    bool fte_span() const { return (arm_lifq_ == FTE_SPAN_LIFQ); }
    bool alg_cflow() const { return (arm_lifq_ == ALG_CFLOW_LIFQ); }

    bool valid_iflow() const { return valid_iflow_; }
    void set_valid_iflow(bool val) { valid_iflow_ = val; }

    bool valid_rflow() const { return valid_rflow_; }
    void set_valid_rflow(bool val) { valid_rflow_ = val; }

    bool is_ipfix_flow() const { return is_ipfix_flow_; }
    void set_is_ipfix_flow(bool val) { is_ipfix_flow_ = val; }

    flow_t *flow(hal::flow_role_t role, uint8_t stage=0) {
         if (role == hal::FLOW_ROLE_INITIATOR) {
             return (iflow_[stage]);
         } else {
             return (rflow_[stage]);
         }
    }

    bool ignore_session_create() const { return ignore_session_create_; }
    void set_ignore_session_create(bool val) {
        ignore_session_create_ = val;
        ipc_logging_disable_ = val;
    }

    hal::vrf_t *svrf() const { return svrf_; }
    hal::vrf_t *dvrf() const { return dvrf_; }
    hal::l2seg_t *sl2seg() const { return sl2seg_; }
    hal::l2seg_t *dl2seg() const { return dl2seg_; }
    hal::if_t *sif() const { return sif_; }
    hal::if_t *dif() const { return dif_; }
    void set_sif(hal::if_t *sif) { sif_ = sif; }
    void set_dif(hal::if_t *dif) { dif_ = dif; }
    void set_sep(hal::ep_t *ep) { sep_ = ep; sep_handle_ = ep->hal_handle;}
    hal::ep_t *sep() const { return sep_; }
    hal::ep_t *dep() const { return dep_; }
    hal_handle_t sep_handle() { return sep_handle_; }
    hal_handle_t dep_handle() { return dep_handle_; }
    bool vlan_valid();
    bool tunnel_terminated();

    bool hal_cleanup() const { return cleanup_hal_; }
    void set_hal_cleanup(bool val) { cleanup_hal_ = val; }
    void swap_flow_objs();
    bool is_proxy_enabled();
    bool is_proxy_flow();

    void *feature_state(const std::string &name = "") {
        uint16_t id = name.length() ? feature_id(name) : feature_id_;
        return (id < num_features_) ? feature_state_[id].ctx_state : nullptr;
    }

    void *feature_state(uint16_t fid) {
        return (fid < num_features_) ? feature_state_[fid].ctx_state : nullptr;
    }

    feature_session_state_t *
    feature_session_state(const std::string &name = "") {
        uint16_t id = name.length() ? feature_id(name) : feature_id_;
        return (id < num_features_) ? feature_state_[id].session_state : nullptr;
    }

    hal_ret_t register_feature_session_state(feature_session_state_t *state) {
        if (session_) {
            HAL_TRACE_ERR("fte: feature={} inserting session state for an existing session",
                          feature_name_);
            return HAL_RET_INVALID_OP;
        }

        strncpy(state->feature_name, feature_name_, sizeof(state->feature_name));

        dllist_reset(&state->session_feature_lentry);

        HAL_TRACE_VERBOSE("fte: feature={} register session state {:p}",
                        feature_name_, (void*)state);
        feature_state_[feature_id_].session_state = state;
        return HAL_RET_OK;
    }

    pipeline_event_t  pipeline_event(void) { return event_; }
    void set_pipeline_event(pipeline_event_t ev) { event_ = ev; }
    bool force_delete() { return force_delete_; }
    void set_force_delete(bool val) { force_delete_ = val; }
    void set_use_vrf(hal::vrf_t *vrf) {use_vrf_ = vrf; }
    bool l3_tunnel_flow() { return l3_tunnel_flow_; }
    void set_l3_tunnel_flow(bool l3_tunnel_flow) { l3_tunnel_flow_ = l3_tunnel_flow; }
    void set_flow_lkupid(uint32_t flow_lookup_id) { flow_lkupid_ = flow_lookup_id; }
    fte_flow_log_info_t* flow_log(hal::flow_role_t role=hal::FLOW_ROLE_NONE) {
        if (role == hal::FLOW_ROLE_NONE) role = role_;

        if (role == hal::FLOW_ROLE_INITIATOR) {
            return &iflow_log_[istage_];
        } else {
            return &rflow_log_[rstage_];
        }
    }
    inline void add_flow_logging(hal::flow_key_t key, hal_handle_t sess_hdl,
                          fte_flow_log_info_t *fwlog);
    void set_ipc_logging_disable(bool val) { ipc_logging_disable_ = val; }
    bool ipc_logging_disable(void) { return ipc_logging_disable_; }
    bool is_flow_swapped(void) { return is_flow_swapped_; }
    uint16_t payload_len(void) { return payload_len_; }
    void set_featureid_bitmap(uint64_t bitmap) { featureid_bitmap_ = bitmap; }
    uint64_t featureid_bitmap(void) { return featureid_bitmap_; }
    uint16_t get_feature_id(void) { return feature_id_; }

    // protected methods accessed by gtest
protected:
    hal_ret_t init(const lifqid_t &lifq, feature_state_t feature_state[],
                   uint16_t num_features);

    cpu_rxhdr_t           *cpu_rxhdr_; // metadata from p4 to cpu
    uint8_t               *pkt_;
    size_t                pkt_len_;
    bool                  copied_pkt_;

private:
    lifqid_t              arm_lifq_;
    hal::flow_key_t       key_;
    hal::flow_key_t       rkey_; // rflow key
    hal::vrf_t            *use_vrf_;  // use vrf vlan to reinject uplink pkts
    bool                  vlan_tag_valid_;

    // pkts queued for tx
    uint16_t               txpkt_cnt_;
    txpkt_info_t           txpkts_[MAX_QUEUED_PKTS];
    bool                   enq_or_free_rx_pkt_; // Rx Packets from PMD layer should be enqueued back to the pipeline
                                                // or the resource should be explicitly freed up in case it is dropped
                                                // Most of the non-flowmiss (ALG/TCP close), the packet is looked up and
                                                // freed. If we dont do this, we might leak the packet buffers.

    session::SessionSpec           *sess_spec_;
    session::SessionStatus         *sess_status_;
    session::SessionStats          *sess_stats_;
    session::SessionResponse       *sess_resp_;
    session::SessionGetResponse    *sess_get_resp_;

    const char*           feature_name_;   // Name of the feature being executed (for logging)
    uint16_t              feature_id_;     // ID of the feature being executed
    hal_ret_t             feature_status_; // feature exit status (set by features to pass the error status)
    feature_state_t*      feature_state_;  // feature specific states
    uint16_t              num_features_;   // num of features

    bool                  drop_;           // Drop the packet
    bool                  drop_flow_;   // fw action is drop, installs drop flow
    hal::session_t        *session_;
    bool                  cleanup_hal_;    // Cleanup hal session

    hal::flow_role_t      role_;            // current flow role
    uint8_t               istage_;          // current iflow stage
    uint8_t               rstage_;          // current rflow stage
    bool                  valid_iflow_;     // Is iflow valid
    bool                  valid_rflow_;     // Is rflow valid
    bool                  is_ipfix_flow_;   // Is rflow valid
    bool                  ignore_session_create_; //ignore session creation for the flow.
    flow_t                *iflow_[MAX_STAGES];       // iflow
    flow_t                *rflow_[MAX_STAGES];       // rflow
    bool                  force_delete_;     // Force delete session

    hal::vrf_t            *svrf_;
    hal::vrf_t            *dvrf_;
    hal::l2seg_t          *sl2seg_;
    hal::l2seg_t          *dl2seg_;
    hal::if_t             *sif_;
    hal::if_t             *dif_;
    hal::ep_t             *sep_;
    hal::ep_t             *dep_;
    hal_handle_t          sep_handle_;
    hal_handle_t          dep_handle_;
    uint32_t              flow_lkupid_;
    bool                  l3_tunnel_flow_;
    pipeline_event_t      event_;
    bool                  update_session_;
    fte_flow_log_info_t   iflow_log_[MAX_STAGES];
    fte_flow_log_info_t   rflow_log_[MAX_STAGES];
    ipc_logger            *logger_;
    bool                  ipc_logging_disable_;
    hal::session_args_t   session_args;
    hal::session_cfg_t    session_cfg;
    hal::session_state_t  session_state;

    hal::flow_cfg_t       iflow_cfg_list[MAX_STAGES];
    hal::flow_cfg_t       rflow_cfg_list[MAX_STAGES];
    hal::flow_pgm_attrs_t iflow_attrs_list[MAX_STAGES];
    hal::flow_pgm_attrs_t rflow_attrs_list[MAX_STAGES];
    bool                  is_flow_swapped_;
    uint16_t              payload_len_;
    uint64_t              featureid_bitmap_;

    void init_ctxt_from_session(hal::session_t *session);
    hal_ret_t init_flows(flow_t iflow[], flow_t rflow[]);
    hal_ret_t update_flow_table();
    hal_ret_t lookup_flow_objs();
    hal_ret_t lookup_session();
    hal_ret_t create_session();
    hal_ret_t extract_flow_key();
    void invoke_completion_handlers(bool fail);
    hal_ret_t update_for_dnat(hal::flow_role_t role,
                              const header_rewrite_info_t& header);
    hal_ret_t update_for_snat(hal::flow_role_t role,
                              const header_rewrite_info_t& header);
    hal_ret_t update_flow_qos_class_id(flow_t *flow,
                                       hal::flow_pgm_attrs_t *flow_attrs);

    static void swap_flow_key(const hal::flow_key_t &key, hal::flow_key_t *rkey);

};

} // namespace fte
