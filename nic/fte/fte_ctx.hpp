#pragma once

#include "nic/include/base.h"
#include "nic/hal/src/session.hpp"
#include "nic/hal/src/proxy.hpp"
#include "nic/include/cpupkt_headers.hpp"
#include "nic/include/cpupkt_api.hpp"
#include <netinet/ether.h>

namespace fte {

// Flow update codes
#define FTE_FLOW_UPDATE_CODES(ENTRY)                                    \
    ENTRY(FLOWUPD_ACTION,        0, "update flow action (allow/deny)")  \
    ENTRY(FLOWUPD_HEADER_REWRITE,1, "modify the header")                \
    ENTRY(FLOWUPD_HEADER_PUSH,   2, "push header")                      \
    ENTRY(FLOWUPD_HEADER_POP,    3, "pop header")                       \
    ENTRY(FLOWUPD_FLOW_STATE,    4,  "connection tracking state")        \
    ENTRY(FLOWUPD_FWDING_INFO,   5, "fwding info")                      \
    ENTRY(FLOWUPD_KEY,           6, "flow key update")                  \

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
    uint32_t tenant_id:1;
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
            uint32_t tenant_id;
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
} fwding_info_t;

std::ostream& operator<<(std::ostream& os, const fwding_info_t& val);

typedef struct flow_update_s {
    flow_update_type_t type;
    union {
        session::FlowAction action;
        header_rewrite_info_t header_rewrite;
        header_push_info_t header_push;
        header_pop_info_t header_pop;
        flow_state_t flow_state;
        fwding_info_t fwding;
        hal::flow_key_t key;
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

const lifqid_t FLOW_MISS_LIFQ = {hal::SERVICE_LIF_CPU, 0, 0};
const lifqid_t TCP_PROXY_LIFQ = {hal::SERVICE_LIF_TCP_PROXY, 0, 0};
const lifqid_t TLS_PROXY_LIFQ = {hal::SERVICE_LIF_TLS_PROXY, 0, 0};

inline std::ostream& operator<<(std::ostream& os, const lifqid_t& lifq)
{
    return os << fmt::format("{{lif={}, qtype={}, qid={}}}",
                             lifq.lif, lifq.qtype, lifq.qid);
}

// pkt info for queued tx packets
typedef struct txpkt_info_s txpkt_info_t;
struct txpkt_info_s {
    hal::pd::cpu_to_p4plus_header_t cpu_header;
    hal::pd::p4plus_to_p4_header_t  p4plus_header;
    uint8_t                        *pkt;
    size_t                          pkt_len;
    lifqid_t                        lifq;   // Dest lif/qtype/qid
    uint8_t                         ring_number; // arm ring
};

typedef hal::pd::p4_to_p4plus_cpu_pkt_t cpu_rxhdr_t;
class flow_t;

static const uint8_t MAX_FLOW_KEYS = 2;

// FTE context passed between features in a pipeline
class ctx_t {
public:
    static const uint8_t MAX_STAGES = hal::MAX_SESSION_FLOWS; // max no.of times a pkt enters p4 pipeline
    static const uint8_t MAX_QUEUED_PKTS = 2;  // max queued pkts for tx

    hal_ret_t init(cpu_rxhdr_t *cpu_rxhdr, uint8_t *pkt, size_t pkt_len,
                   flow_t iflow[], flow_t rflow[]);
    hal_ret_t init(SessionSpec *spec, SessionResponse *rsp,
                   flow_t iflow[], flow_t rflow[]);

    hal_ret_t update_flow(const flow_update_t& flowupd, const hal::flow_role_t role);

    hal_ret_t update_flow(const flow_update_t& flowupd);

    hal_ret_t update_gft();

    hal_ret_t build_wildcard_key(hal::flow_key_t& key);
 
    uint8_t construct_lookup_keys(hal::flow_key_t *key);

    // Get key based on role
    const hal::flow_key_t& get_key(hal::flow_role_t role);

    // Firewall action
    bool drop() const { return drop_; }

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
                          uint8_t  ring_number = CPU_SCHED_RING_ASQ);
    hal_ret_t send_queued_pkts(hal::pd::cpupkt_ctxt_t* arm_ctx);

    //proto spec is valid when flow update triggered via hal proto api
    bool protobuf_request() { return sess_spec_ != NULL; }
    session::SessionSpec* sess_spec() {return sess_spec_; }
    session::SessionResponse* sess_resp() {return sess_resp_; }
    hal::session_t* session() { return session_; }

    const lifqid_t& arm_lifq() const { return arm_lifq_; }
    void set_arm_lifq(const lifqid_t& arm_lifq) {arm_lifq_= arm_lifq;}

    uint8_t stage() const { return role_ == hal::FLOW_ROLE_INITIATOR ? istage_ : rstage_; }
    hal_ret_t advance_to_next_stage();

    // name of the feature being executed
    const char* feature_name() const { return feature_name_; } 
    void set_feature_name(const char* name) { feature_name_ = name; }

    // return staus of the feature handler
    hal_ret_t feature_status() const { return feature_status_; } 
    void set_feature_status(hal_ret_t ret) { feature_status_ = ret; }

    bool flow_miss() const { return (((session_ == NULL) || \
                     ((role_ == hal::FLOW_ROLE_RESPONDER) && (pgm_rflow_ == true))) && \
                      (arm_lifq_.lif == hal::SERVICE_LIF_CPU)); }
    bool valid_rflow() const { return valid_rflow_; }
    bool pgm_rflow()  const { return pgm_rflow_; }
    void set_pgm_rflow(bool pgm) { pgm_rflow_ = pgm; }

    hal::tenant_t *tenant() const { return tenant_; }
    hal::l2seg_t *sl2seg() const { return sl2seg_; }
    hal::l2seg_t *dl2seg() const { return dl2seg_; }
    hal::if_t *sif() const { return sif_; }
    hal::if_t *dif() const { return dif_; }
    hal::ep_t *sep() const { return sep_; }
    hal::ep_t *dep() const { return dep_; }
    hal::alg_proto_state_t alg_proto_state() const { return alg_proto_state_; }
    void  set_alg_proto_state(hal::alg_proto_state_t state) { alg_proto_state_ = state; }

    nwsec::ALGName alg_proto() const { return alg_proto_; }
    void set_alg_proto(nwsec::ALGName proto) { alg_proto_ = proto; }

    bool hal_cleanup() const { return cleanup_hal_; }
    void set_hal_cleanup(bool val) { cleanup_hal_ = val; }
    void swap_flow_objs();

private:
    lifqid_t              arm_lifq_;
    hal::flow_key_t       key_;

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

    bool                  drop_;           // Drop the packeto
    hal::session_t        *session_;
    bool                  cleanup_hal_;    // Cleanup hal session

    hal::flow_role_t      role_;            // current flow role
    uint8_t               istage_;          // current iflow stage
    uint8_t               rstage_;          // current rflow stage
    bool                  valid_rflow_;     // Is rflow valid
    bool                  pgm_rflow_;       // Is rflow software only ?
    flow_t                *iflow_[MAX_STAGES];       // iflow 
    flow_t                *rflow_[MAX_STAGES];       // rflow 

    hal::tenant_t         *tenant_;
    hal::l2seg_t          *sl2seg_;
    hal::l2seg_t          *dl2seg_;
    hal::if_t             *sif_;
    hal::if_t             *dif_;
    hal::ep_t             *sep_;
    hal::ep_t             *dep_;
    nwsec::ALGName        alg_proto_;         // ALG Proto to be applied
    hal::alg_proto_state_t  alg_proto_state_; // ALG Proto state machine state

    hal_ret_t init_flows(flow_t iflow[], flow_t rflow[]);
    hal_ret_t lookup_flow_objs();
    hal_ret_t lookup_session();
    hal_ret_t create_session();
    hal_ret_t extract_flow_key();
    hal_ret_t update_for_dnat(hal::flow_role_t role,
                              const header_rewrite_info_t& header);
    static hal_ret_t extract_flow_key_from_spec(hal::flow_key_t *key,
                                                const FlowKey&  flow_spec_key,
                                                hal::tenant_id_t tid);
};

} // namespace fte
