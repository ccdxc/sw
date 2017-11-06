#pragma once

#include "nic/include/base.h"
#include "nic/hal/src/session.hpp"
#include "nic/hal/src/proxy.hpp"
#include "nic/hal/src/rawrcb.hpp"
#include "nic/include/cpupkt_headers.hpp"
#include "nic/include/cpupkt_api.hpp"
#include <netinet/ether.h>
#include "nic/include/fte_db.hpp"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/include/app_redir_headers.hpp"

namespace fte {

// Flow update codes
#define FTE_FLOW_UPDATE_CODES(ENTRY)                                    \
    ENTRY(FLOWUPD_ACTION,        0, "update flow action (allow/deny)")  \
    ENTRY(FLOWUPD_HEADER_REWRITE,1, "modify the header")                \
    ENTRY(FLOWUPD_HEADER_PUSH,   2, "push header")                      \
    ENTRY(FLOWUPD_HEADER_POP,    3, "pop header")                       \
    ENTRY(FLOWUPD_FLOW_STATE,    4,  "connection tracking state")       \
    ENTRY(FLOWUPD_FWDING_INFO,   5, "fwding info")                      \
    ENTRY(FLOWUPD_KEY,           6, "flow key update")                  \
    ENTRY(FLOWUPD_MCAST_COPY,    7, "flow mcast copy update")           \
    ENTRY(FLOWUPD_INGRESS_INFO,  8, "ingress info")            \

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
    {                                               \
        obj.valid_hdrs  |= fte::FTE_HEADER_ ## hdr; \
        obj.valid_flds.fld = 1;                     \
        obj.hdr.fld = val;                          \
    }

#define HEADER_COPY_FLD(dst, src, hdr, fld)         \
    if (src.valid_flds.fld) {                       \
        HEADER_SET_FLD(dst, hdr, fld, src.hdr.fld); \
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
            ipv4_addr_t sip;
            ipv4_addr_t dip;
            uint8_t   ttl;
            uint8_t   dscp;
        } __PACK__ ipv4;
        struct {
            ipv6_addr_t sip;
            ipv6_addr_t dip;
            uint8_t   ttl;
            uint8_t   dscp;
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
    hal::if_t *dif;
    hal::l2seg_t *dl2seg;
} fwding_info_t;

std::ostream& operator<<(std::ostream& os, const fwding_info_t& val);

typedef struct mcast_info_s {
    uint8_t  mcast_en;
    uint16_t mcast_ptr;
} mcast_info_t;

std::ostream& operator<<(std::ostream& os, const mcast_info_t& val);

typedef struct ingress_info_s {
    // Expected source interface for host pinning mode
    hal::if_t *expected_sif;
} ingress_info_t;

std::ostream& operator<<(std::ostream& os, const ingress_info_t& val);

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
        hal::flow_key_t key;
        mcast_info_t mcast_info;
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

typedef struct lifqid_s lifqid_t;
struct lifqid_s {
    uint64_t lif : 11;
    uint64_t qtype: 3;
    uint64_t qid : 24;
} __PACK__;

const lifqid_t FLOW_MISS_LIFQ = {hal::SERVICE_LIF_CPU, 0, types::CPUCB_ID_FLOWMISS};
const lifqid_t TCP_CLOSE_LIFQ = {hal::SERVICE_LIF_CPU, 0, types::CPUCB_ID_TCP_CLOSE};
const lifqid_t ALG_CFLOW_LIFQ = {hal::SERVICE_LIF_CPU, 0, types::CPUCB_ID_RELIABLE_COPY};
const lifqid_t NACL_REDIRECT_LIFQ = {hal::SERVICE_LIF_CPU, 0, types::CPUCB_ID_NACL_REDIRECT};
const lifqid_t NACL_LOG_LIFQ = {hal::SERVICE_LIF_CPU, 0, types::CPUCB_ID_NACL_LOG};
const lifqid_t QUIESCE_LIFQ   = {hal::SERVICE_LIF_CPU, 0, types::CPUCB_ID_QUIESCE};
const lifqid_t TCP_PROXY_LIFQ = {hal::SERVICE_LIF_TCP_PROXY, 0, 0};
const lifqid_t TLS_PROXY_LIFQ = {hal::SERVICE_LIF_TLS_PROXY, 0, 0};
const lifqid_t APP_REDIR_LIFQ = {hal::SERVICE_LIF_APP_REDIR, 0, 0};

inline std::ostream& operator<<(std::ostream& os, const lifqid_t& lifq)
{
    return os << fmt::format("{{lif={}, qtype={}, qid={}}}",
                             lifq.lif, lifq.qtype, lifq.qid);
}

inline bool operator==(const lifqid_t& lifq1, const lifqid_t& lifq2)
{
    return (((lifq1.lif == lifq2.lif) && (lifq1.qtype == lifq2.qtype) && \
            (lifq1.qid == lifq2.qid)));
}

// Application redirect context
typedef enum {
    APP_REDIR_VERDICT_PASS,     // pass the packet
    APP_REDIR_VERDICT_BLOCK,    // block the packet
} app_redir_verdict_t;

class app_redir_ctx_t {
public:
    void init()
    {
        redir_flags_            = 0;
        hdr_len_total_          = 0;
        chain_flow_id_          = 0;
        chain_rev_role_         = hal::FLOW_ROLE_NONE;
        chain_ring_             = 0;
        chain_qtype_            = hal::APP_REDIR_RAWC_QTYPE;
        chain_wring_type_       = types::WRING_TYPE_APP_REDIR_RAWC;
        chain_pkt_verdict_      = APP_REDIR_VERDICT_PASS;
        pipeline_end_           = false;
        chain_pkt_enqueued_     = false;
        redir_policy_applic_    = false;
        redir_miss_pkt_p_       = nullptr;
        proxy_flow_info_        = nullptr;
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

    pen_app_redir_header_v1_full_t& redir_miss_hdr() { return redir_miss_hdr_; }
    size_t redir_miss_hdr_size() { return sizeof(redir_miss_hdr_); }

private:
    uint16_t                        redir_flags_;
    uint16_t                        hdr_len_total_;
    types::WRingType                chain_wring_type_;
    bool                            chain_pkt_enqueued_ : 1,
                                    pipeline_end_       : 1,
                                    redir_policy_applic_: 1;
    app_redir_verdict_t             chain_pkt_verdict_;
    pen_app_redir_header_v1_full_t  redir_miss_hdr_;
    uint8_t                         *redir_miss_pkt_p_;
    hal::proxy_flow_info_t          *proxy_flow_info_;
    uint32_t                        chain_flow_id_;
    hal::flow_role_t                chain_rev_role_;    // rflow role
    uint8_t                         chain_qtype_;
    uint8_t                         chain_ring_;
};

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
};

typedef hal::pd::p4_to_p4plus_cpu_pkt_t cpu_rxhdr_t;
class flow_t;
class ctx_t;

typedef void (*completion_handler_t) (ctx_t &ctx, bool fail);

static const uint8_t MAX_FLOW_KEYS = 3;

// FTE context passed between features in a pipeline
class ctx_t {
public:
    static const uint8_t MAX_STAGES = hal::MAX_SESSION_FLOWS; // max no.of times a pkt enters p4 pipeline
    static const uint8_t MAX_QUEUED_PKTS = 2;  // max queued pkts for tx
    static const uint8_t MAX_QUEUED_HANDLERS = 16; // max queued completion handlers

    hal_ret_t init(cpu_rxhdr_t *cpu_rxhdr, uint8_t *pkt, size_t pkt_len,
                   flow_t iflow[], flow_t rflow[]);
    hal_ret_t init(SessionSpec *spec, SessionResponse *rsp,
                   flow_t iflow[], flow_t rflow[]);
    hal_ret_t process();

    hal_ret_t update_flow(const flow_update_t& flowupd, const hal::flow_role_t role);

    hal_ret_t update_flow(const flow_update_t& flowupd);

    // Get key based on role
    const hal::flow_key_t& get_key(hal::flow_role_t role);

    // Flow has drop action set (installs flow with drop action)
    bool drop_flow() { return drop_flow_; }

    // Drop packet
    bool drop() const { return drop_; }
    void set_drop() {
        drop_ = true;
        HAL_TRACE_DEBUG("fte::set_drop feature={}", feature_name_);
    }

    // direction of the current pkt
    hal::flow_direction_t direction() {return (hal::flow_direction_t)(key_.dir); };

    // role of the current flow being processed
    hal::flow_role_t role() const { return role_; }
    void set_role(hal::flow_role_t role) { role_ = role; }

    // flow key of the current pkts flow
    const hal::flow_key_t& key() const { return key_; }
    
    // Following are valid only for packets punted to ARM
    const cpu_rxhdr_t* cpu_rxhdr() const { return cpu_rxhdr_; }
    uint8_t* pkt() const { return pkt_; }
    size_t pkt_len() const { return pkt_len_; }

    // Queue pkt for tx (in case of flow_miss rx pkt is 
    // transmitted if no pkts are queued for tx)
    hal_ret_t queue_txpkt(uint8_t *pkt, size_t pkt_len,
                          hal::pd::cpu_to_p4plus_header_t *cpu_header = NULL,
                          hal::pd::p4plus_to_p4_header_t  *p4plus_header = NULL,
                          uint16_t dest_lif = hal::SERVICE_LIF_CPU, 
                          uint8_t  qtype = CPU_ASQ_QTYPE,
                          uint32_t qid = CPU_ASQ_QID,
                          uint8_t  ring_number = CPU_SCHED_RING_ASQ,
                          types::WRingType wring_type = types::WRING_TYPE_ASQ);
    hal_ret_t send_queued_pkts(hal::pd::cpupkt_ctxt_t* arm_ctx);

    //proto spec is valid when flow update triggered via hal proto api
    bool protobuf_request() { return sess_spec_ != NULL; }
    session::SessionSpec* sess_spec() {return sess_spec_; }
    session::SessionResponse* sess_resp() {return sess_resp_; }
    hal::session_t* session() { return session_; }
    bool existing_session() const { return existing_session_; }

    const lifqid_t& arm_lifq() const { return arm_lifq_; }
    void set_arm_lifq(const lifqid_t& arm_lifq) {arm_lifq_= arm_lifq;}

    hal::pd::cpupkt_ctxt_t* arm_ctx() const { return arm_ctx_; }
    void set_arm_ctx(hal::pd::cpupkt_ctxt_t* arm_ctx) {arm_ctx_= arm_ctx;}

    uint8_t stage() const { return role_ == hal::FLOW_ROLE_INITIATOR ? istage_ : rstage_; }
    hal_ret_t advance_to_next_stage();

    // name of the feature being executed
    const char* feature_name() const { return feature_name_; } 
    void set_feature_name(const char* name) { feature_name_ = name; }

    // return staus of the feature handler
    hal_ret_t feature_status() const { return feature_status_; } 
    void set_feature_status(hal_ret_t ret) { feature_status_ = ret; }

    // completion handlere rgistrations, registered handlers are called at the end of the
    // packet processing (after updating the gft)
    // callback's 'fail' argument is set to true if the pkt processing failed and
    // flow is not getting installed.
    hal_ret_t register_completion_handler(completion_handler_t handler) {
        HAL_ASSERT_RETURN(num_handlers_ + 1 < MAX_QUEUED_HANDLERS, HAL_RET_INVALID_OP);
        completion_handlers_[num_handlers_++] = handler;
        HAL_TRACE_DEBUG("fte: feature={} queued completion handler {:p} num_handlers: {}", 
                         feature_name_, (void*)handler, num_handlers_);
        return HAL_RET_OK;
    }

    bool flow_miss() const { return (arm_lifq_==FLOW_MISS_LIFQ); }

    bool valid_rflow() const { return valid_rflow_; }
    void set_valid_rflow(bool val) { valid_rflow_ = val; }

    hal::vrf_t *vrf() const { return vrf_; }
    hal::l2seg_t *sl2seg() const { return sl2seg_; }
    hal::l2seg_t *dl2seg() const { return dl2seg_; }
    hal::if_t *sif() const { return sif_; }
    hal::if_t *dif() const { return dif_; }
    void set_sif(hal::if_t *sif) { sif_ = sif; }
    void set_dif(hal::if_t *dif) { dif_ = dif; }
    hal::ep_t *sep() const { return sep_; }
    hal::ep_t *dep() const { return dep_; }

    alg_proto_state_t alg_proto_state() const { return alg_proto_state_; }
    void  set_alg_proto_state(alg_proto_state_t state) { alg_proto_state_ = state; }

    nwsec::ALGName alg_proto() const { return alg_proto_; }
    void set_alg_proto(nwsec::ALGName proto) { alg_proto_ = proto; }

    alg_entry_t alg_entry() const { return alg_entry_; }
    void set_alg_entry(const alg_entry_t entry) { alg_entry_ = entry; }

    bool hal_cleanup() const { return cleanup_hal_; }
    void set_hal_cleanup(bool val) { cleanup_hal_ = val; }
    void swap_flow_objs();

    app_redir_ctx_t& app_redir() { return app_redir_; }

    bool skip_firewall() const { return skip_firewall_; }
    void set_skip_firewall(bool val) { skip_firewall_ = val; }

private:
    lifqid_t              arm_lifq_;
    hal::flow_key_t       key_;

    hal::pd::cpupkt_ctxt_t* arm_ctx_;
    cpu_rxhdr_t           *cpu_rxhdr_; // metadata from p4 to cpu
    uint8_t               *pkt_;
    size_t                pkt_len_;

    // pkts queued for tx
    uint8_t               txpkt_cnt_;
    txpkt_info_t          txpkts_[MAX_QUEUED_PKTS];

    session::SessionSpec           *sess_spec_;
    session::SessionResponse       *sess_resp_;

    const char*           feature_name_;   // Name of the feature being executed (for logging)
    hal_ret_t             feature_status_; // feature exit status (set by features to pass the error status)

    //completion handlers
    uint8_t               num_handlers_;
    completion_handler_t  completion_handlers_[MAX_QUEUED_HANDLERS];

    bool                  drop_;           // Drop the packet
    bool                  drop_flow_;   // fw action is drop, installs drop flow
    bool                  existing_session_;// Existing or new session ?
    hal::session_t        *session_;
    bool                  cleanup_hal_;    // Cleanup hal session

    hal::flow_role_t      role_;            // current flow role
    uint8_t               istage_;          // current iflow stage
    uint8_t               rstage_;          // current rflow stage
    bool                  valid_rflow_;     // Is rflow valid
    flow_t                *iflow_[MAX_STAGES];       // iflow 
    flow_t                *rflow_[MAX_STAGES];       // rflow 

    hal::vrf_t         *vrf_;
    hal::l2seg_t          *sl2seg_;
    hal::l2seg_t          *dl2seg_;
    hal::if_t             *sif_;
    hal::if_t             *dif_;
    hal::ep_t             *sep_;
    hal::ep_t             *dep_;
    nwsec::ALGName        alg_proto_;         // ALG Proto to be applied
    alg_proto_state_t     alg_proto_state_;   // ALG Proto state machine state
    alg_entry_t           alg_entry_;         // ALG entry in the wildcard table
    bool                  skip_firewall_;
    app_redir_ctx_t       app_redir_;

    hal_ret_t init_flows(flow_t iflow[], flow_t rflow[]);
    hal_ret_t update_flow_table();
    hal_ret_t lookup_flow_objs();
    hal_ret_t lookup_session();
    hal_ret_t create_session();
    hal_ret_t extract_flow_key();
    void invoke_completion_handlers(bool fail);
    hal_ret_t update_for_dnat(hal::flow_role_t role,
                              const header_rewrite_info_t& header);
    hal_ret_t build_wildcard_key(hal::flow_key_t& key);
    uint8_t construct_lookup_keys(hal::flow_key_t *key);
};

} // namespace fte
