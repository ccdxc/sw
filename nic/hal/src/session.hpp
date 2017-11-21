#ifndef __SESSION_HPP__
#define __SESSION_HPP__

#include "nic/include/base.h"
#include "nic/include/list.hpp"
#include "nic/utils/ht/ht.hpp"
#include "nic/include/ip.h"
#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/l2segment.hpp"
#include "nic/hal/src/endpoint.hpp"
#include "nic/gen/proto/hal/session.pb.h"
#include "nic/include/pd.hpp"
#include "nic/gen/iris/include/p4pd.h"
#include "nic/hal/src/qos.hpp"

using hal::utils::ht_ctxt_t;
using hal::utils::dllist_ctxt_t;

using session::FlowKeyL2;
using session::FlowKeyV4;
using session::FlowKeyV6;
using session::FlowKey;
using session::FlowData;
using session::FlowInfo;
using session::FlowSpec;
using session::ConnTrackInfo;
using session::SessionSpec;
using session::SessionStatus;
using session::SessionResponse;
using session::SessionRequestMsg;
using session::SessionResponseMsg;
using session::SessionDeleteRequestMsg;
using session::SessionDeleteResponseMsg;
using session::SessionGetRequest;
using session::SessionGetRequestMsg;
using session::SessionGetResponse;
using session::SessionGetResponseMsg;
using types::IPProtocol;

namespace hal {

// forward declarations
typedef struct flow_s flow_t;
typedef struct session_s session_t;
typedef struct app_session_s app_session_t;

typedef uint32_t        session_id_t;
typedef uint32_t        app_session_id_t;

// flow type depends on the type of packets flow is for
enum flow_type_t {
    FLOW_TYPE_L2   = 0,    // non-IP flow
    FLOW_TYPE_V4   = 1,    // IPv4 flow
    FLOW_TYPE_V6   = 2,    // IPv6 flow
};

// flow direction tells whether flow is from or to the workload
enum flow_direction_t {
    FLOW_DIR_FROM_ENIC   = 0,
    FLOW_DIR_FROM_UPLINK = 1,
};

#define FLOW_ROLES(ENTRY)                                           \
    ENTRY(FLOW_ROLE_INITIATOR,      0, "iflow")                     \
    ENTRY(FLOW_ROLE_RESPONDER,      1, "rflow")                     \
    ENTRY(FLOW_ROLE_NONE,           2, "none") 

DEFINE_ENUM(flow_role_t, FLOW_ROLES)
#undef FLOW_ROLES
    
#define FLOW_END_TYPES(ENTRY)                                       \
    ENTRY(FLOW_END_TYPE_HOST,        0, "FLOW_END_TYPE_HOST")       \
    ENTRY(FLOW_END_TYPE_NETWORK,     1, "FLOW_END_TYPE_NETWORK")    \
    ENTRY(FLOW_END_TYPE_P4PLUS,      2, "FLOW_END_TYPE_P4PLUS")

DEFINE_ENUM(flow_end_type_t, FLOW_END_TYPES)
#undef FLOW_END_TYPES

#define SESSION_DIRECTIONS(ENTRY)                                   \
    ENTRY(SESSION_DIR_H,     0, "SESSION_DIR_H")                    \
    ENTRY(SESSION_DIR_N,     1, "SESSION_DIR_N")                    \

DEFINE_ENUM(session_dir_t, SESSION_DIRECTIONS)
#undef SESSION_DIRECTIONS

// NAT types
enum nat_type_t {
    NAT_TYPE_NONE         = 0,
    NAT_TYPE_SNAT         = 1,    // source NAT
    NAT_TYPE_DNAT         = 2,    // destination NAT
    NAT_TYPE_TWICE_NAT    = 3,    // twice NAT
};

// exceptions seen on flows
// TODO: define more !!
#define FLOW_EXPN_TCP_SEGMENTS_OUT_OF_ORDER          0x1
#define FLOW_EXPN_TCP_OVERLAPPING_SEGMENTS           0x2

// alternate flow key structure
typedef struct flow_key_s {
    uint8_t              flow_type:4;    // one of the flow types defined above
    uint8_t              dir:1;

    union {
        // L2 flow key information
        struct {
            l2seg_id_t    l2seg_id;          // L2 segment
            mac_addr_t    smac;              // source MAC address of flow
            mac_addr_t    dmac;              // destination MAC address of flow
            uint16_t      ether_type;        // ether type
        } __PACK__;

        // IPv4/IPv6 flow key information
        struct {
            vrf_id_t            vrf_id;    // vrf id
            ipvx_addr_t            sip;          // source IP address
            ipvx_addr_t            dip;          // destination IP address
            IPProtocol             proto;        // IP protocol
            union {
                struct {
                    uint8_t        icmp_type;    // ICMP type
                    uint8_t        icmp_code;    // ICMP code
                    uint16_t       icmp_id;      // ICMP identifier
                } __PACK__;
                struct {
                    uint16_t       sport;        // TCP/UDP source port
                    uint16_t       dport;        // TCP/UDP destination port
                } __PACK__;
                struct {
                    uint32_t        spi;         // ESP header's SPI
                } __PACK__;
            } __PACK__;
        } __PACK__;

    } __PACK__;
} __PACK__ flow_key_t;

//spdlog formatter for flow_key_t
const char* flowkey2str(const flow_key_t& key);
inline std::ostream& operator<<(std::ostream& os, const flow_key_t& key)
{
    return os << flowkey2str(key);
}

// flow config
typedef struct flow_cfg_s {
    flow_key_t                key;                 // flow's key
    uint16_t                  action:3;            // flow action(s)
    uint16_t                  role:1;              // flow role (initiator or responder)
    uint16_t                  nat_type:3;          // type of NAT
    ip_addr_t                 nat_sip;             // source NAT IP, if any
    ip_addr_t                 nat_dip;             // destination NAT IP, if any
    uint16_t                  nat_sport;           // NAT source port
    uint16_t                  nat_dport;           // NAT destination port
    qos_action_t              in_qos_action;       // Ingress qos action
    qos_action_t              eg_qos_action;       // Egress qos action
    uint8_t                   eg_mirror_session;   // Mirror sessions in egress direction
    uint8_t                   ing_mirror_session;  // Mirror sessions in ingress direction
} __PACK__ flow_cfg_t;

typedef struct flow_pgm_attrs_s {
    uint64_t                  role:1;              // flow role (initiator or responder)
    uint64_t                  drop:1;              // drop for this flow
    uint64_t                  mac_sa_rewrite:1;    // rewrite src mac
    uint64_t                  mac_da_rewrite:1;    // rewrite dst mac
    uint64_t                  ttl_dec:1;           // decrement ttl
    uint64_t                  mcast_en:1;          // mcast enable
    uint64_t                  tunnel_orig:1;       // tunnel originate
    uint64_t                  lport:11;            // dest lport 
    uint64_t                  qid_en:1;            // qid enabled
    uint64_t                  qtype:3;             // Qtype
    uint64_t                  qid:24;              // Qid
    uint64_t                  lkp_inst:1;          // Flow instance
    uint64_t                  dot1p_en:1;          //enable dot1p rewrite
    uint64_t                  dscp_en:1;           //enable dscp rewrite
    uint64_t                  expected_src_lif_en:1;  // src lif check for host pinning

    uint32_t                  vrf_hwid;         // source l2seg vrf hwid (lkp_vrf)
    rewrite_actions_en        rw_act;              // rewrite action
    uint32_t                  rw_idx;              // rewrite index
    tunnel_rewrite_actions_en tnnl_rw_act;         // tunnel rewrite action
    uint32_t                  tnnl_rw_idx;         // tunnel rewrite index
    uint32_t                  tnnl_vnid;           // tunnel vnid / encap vlan
    ip_addr_t                 nat_ip;              // NAT IP, (dip for dnat/twice_nat, sip for snat)
    uint16_t                  nat_l4_port;         // NAT L4 port
    uint32_t                  twice_nat_idx;       // Twice nat index
    uint8_t                   dot1p;               // Cos value
    uint8_t                   dscp;                // dscp value
    uint16_t                  mcast_ptr;           // Mcast Pointer
    uint32_t                  expected_src_lif;    // src lif for host pinning check
} __PACK__ flow_pgm_attrs_t;

// flow state
struct flow_s {
    hal_spinlock_t    slock;               // lock to protect this structure
    bool              is_aug_flow;         // is an augment flow

    flow_cfg_t        config;              // flow config
    flow_pgm_attrs_t  pgm_attrs;           // table program attributes
    flow_t            *reverse_flow;       // reverse flow data
    flow_t            *assoc_flow;         // valid only if flow has an associated flow
    session_t         *session;            // session this flow belongs to, if any

    // PD state
    pd::pd_flow_t     *pd;                 // all PD specific state

    // meta data maintained for flow
    ht_ctxt_t         flow_key_ht_ctxt;    // flow key based hash table context
} __PACK__;

typedef struct flow_state_s {
    uint64_t        create_ts;           // flow create timestamp
    uint64_t        last_pkt_ts;         // last packet timestamp
    uint64_t        packets;             // packet count on this flow
    uint64_t        bytes;               // byte count on this flow
    uint32_t        exception_bmap;      // exceptions seen on this flow
    session::FlowTCPState   state;             // flow state

    uint32_t        tcp_seq_num;
    uint32_t        tcp_ack_num;
    uint32_t        tcp_win_sz;
    int32_t         syn_ack_delta;            // ACK delta of iflow
    uint16_t        tcp_mss;
    uint8_t         tcp_win_scale;
    uint8_t         tcp_ws_option_sent : 1;
    uint8_t         tcp_ts_option_sent : 1;
    uint8_t         tcp_sack_perm_option_sent : 1;
} __PACK__ flow_state_t;

inline std::ostream& operator<<(std::ostream& os, const flow_state_t& val)
{
    os << "{state=" << val.state;
    if (val.syn_ack_delta) {
        os << ", syn_ack_delta=" << val.syn_ack_delta;
    }
    if (val.tcp_seq_num) {
        os << ", tcp_seq_num=" << val.tcp_seq_num;
    }
    if (val.tcp_ack_num) {
        os << ", tcp_ack_num=" << val.tcp_ack_num;
    }
    return os << "}";
}

typedef struct session_state_s {
    uint8_t             tcp_ts_option:1;
    uint8_t             tcp_sack_perm_option:1;
    flow_state_t        iflow_state;
    flow_state_t        rflow_state;
} __PACK__ session_state_t;

typedef struct session_cfg_s {
    uint8_t             tcp_ts_option:1;
    uint8_t             tcp_sack_perm_option:1;
    session_id_t        session_id;               // unique session id
    uint16_t            conn_track_en:1;          // enable connection tracking
} __PACK__ session_cfg_t;

static const uint8_t MAX_SESSION_FLOWS = 2;
typedef struct session_args_s {
    session_cfg_t      *session;                          // session config
    flow_cfg_t         *iflow[MAX_SESSION_FLOWS];         // initiator flow
    flow_cfg_t         *rflow[MAX_SESSION_FLOWS];         // responder flow
    flow_pgm_attrs_t   *iflow_attrs[MAX_SESSION_FLOWS];   // initiator flow attrs
    flow_pgm_attrs_t   *rflow_attrs[MAX_SESSION_FLOWS];   // responder flow attrs

    session_state_t    *session_state;            // connection tracking state
    bool               valid_rflow;               // Rflow valid ?

    vrf_t           *vrf;                   // vrf
    ep_t               *sep;                      // spurce ep
    ep_t               *dep;                      // dest ep
    if_t               *sif;                      // source interface
    if_t               *dif;                      // dest interface
    l2seg_t            *sl2seg;                   // source l2seg
    l2seg_t            *dl2seg;                   // dest l2seg
    SessionSpec        *spec;                     // session spec
    SessionResponse    *rsp;                      // session response
} __PACK__ session_args_t;

//------------------------------------------------------------------------------
// A session consists of a initiator flow and responder flow
//   - initiator flow is the flow on which the first packet arrived
//   - responder flow is the reverse flow for the initiator flow
// In some cases, it is possible to have uni-directional flows and in such cases
// session consists of only single flow
//------------------------------------------------------------------------------
struct session_s {
    hal_spinlock_t      slock;                    // lock to protect this structure
    session_cfg_t       config;                   // session config
    flow_t              *iflow;                   // initiator flow
    flow_t              *rflow;                   // responder flow, if any
    app_session_t       *app_session;             // app session this L4 session is part of, if any
    vrf_t               *vrf;                     // vrf

    // PD state
    pd::pd_session_t    *pd;                      // all PD specific state

    // meta data maintained for session
    hal_handle_t        hal_handle;               // hal handle for this session
    ht_ctxt_t           session_id_ht_ctxt;       // session id based hash table ctxt
    ht_ctxt_t           hal_handle_ht_ctxt;       // hal handle based hash table ctxt
    ht_ctxt_t           hal_iflow_ht_ctxt;        // hal iflow based hash table ctxt
    ht_ctxt_t           hal_rflow_ht_ctxt;        // hal rflow based hash table ctxt 
    dllist_ctxt_t       sep_session_lentry;       // source EP's session list context
    dllist_ctxt_t       dep_session_lentry;       // destination EP's session list context
    dllist_ctxt_t       sif_session_lentry;       // source interface's session list context
    dllist_ctxt_t       dif_session_lentry;       // destination interface's session list context
    dllist_ctxt_t       sl2seg_session_lentry;    // source L2 segment's session list context
    dllist_ctxt_t       dl2seg_session_lentry;    // destination L2 segment's session list context
    dllist_ctxt_t       vrf_session_lentry;    // vrf's session list context
} __PACK__;

//---------------------------------------------------------------------------------
// ALG status per session
//---------------------------------------------------------------------------------
typedef struct alg_info_s {
    nwsec::ALGName      alg;                       // ALG that this app session represents
    uint32_t            parse_errors;              // Number of parse errors seen
    union {
        struct { /* TFTP Info */
            uint32_t       unknown_opcode;         // Number of packets seen with unknown opcode
        } __PACK__;
        struct { /* RPC Info */
            uint32_t       num_data_sess;          // Number of data session per control session
        } __PACK__;
    } __PACK__;
} alg_info_t;

//------------------------------------------------------------------------------
// app session consists of bunch of L4 sessions, app sessions are used by
// Application Layer Gateway(ALG)s. For example, ftp app session can consist of
// one control L4 session and multiple data sessions.
//------------------------------------------------------------------------------
#define MAX_L4SESSIONS_PER_APP    8
struct app_session_s {
    hal_spinlock_t      slock;                     // lock to protect this structure
    app_session_id_t    app_session_id;            // unique app session identifier
    alg_info_t          alg_info;                  // Oper status, counters corresponding to this session
    hal_handle_t        hal_handle;                // hal handle for this app session
    dllist_ctxt_t       l4_session_list_head;      // all L4 sessions in this session
    ht_ctxt_t           app_session_id_ht_ctxt;    // session id based hash table context
} __PACK__;

// max. number of session supported  (TODO: we can take this from cfg file)
#define HAL_MAX_SESSIONS                             524288
#define HAL_MAX_FLOWS                                (HAL_MAX_SESSIONS << 1)

static inline session_t *
find_session_by_handle (hal_handle_t handle)
{
    return (session_t *)g_hal_state->session_hal_handle_ht()->lookup(&handle);
}

static inline session_t *
find_session_by_id (session_id_t session_id)
{
    return (session_t *)g_hal_state->session_id_ht()->lookup(&session_id);
}


extern void *session_get_key_func(void *entry);
extern uint32_t session_compute_hash_func(void *key, uint32_t ht_size);
extern bool session_compare_key_func(void *key1, void *key2);

extern void *session_get_handle_key_func(void *entry);
extern uint32_t session_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool session_compare_handle_key_func(void *key1, void *key2);

extern void *session_get_iflow_key_func(void *entry);
extern uint32_t session_compute_iflow_hash_func(void *key, uint32_t ht_size);
extern bool session_compare_iflow_key_func(void *key1, void *key2);

extern void *session_get_rflow_key_func(void *entry);
extern uint32_t session_compute_rflow_hash_func(void *key, uint32_t ht_size);
extern bool session_compare_rflow_key_func(void *key1, void *key2);

extern void *flow_get_key_func(void *entry);
extern uint32_t flow_compute_hash_func(void *key, uint32_t ht_size);
extern bool flow_compare_key_func(void *key1, void *key2);

extern hal_ret_t ep_get_from_flow_key(const flow_key_t* key,
                                      ep_t **sep, ep_t **dep);
bool flow_needs_associate_flow(const flow_key_t *flow_key);

extern hal_ret_t session_release(session_t *session);

hal_ret_t extract_flow_key_from_spec (vrf_id_t tid,
                                      flow_key_t *flow_key, 
                                      const FlowKey& flow_spec_key);

hal_ret_t session_create(const session_args_t *args, hal_handle_t *session_handle, session_t **sess);
hal_ret_t session_update(const session_args_t *args, session_t *session);
hal_ret_t session_delete(const session_args_t *args, session_t *session);
hal::session_t *session_lookup(flow_key_t key, flow_role_t *role);


hal_ret_t session_get(session::SessionGetRequest& spec,
                      session::SessionGetResponse *rsp);

}    // namespace hal

#endif    // __SESSION_HPP__

