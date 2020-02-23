//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __INTERFACE_HPP__
#define __INTERFACE_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "lib/list/list.hpp"
#include "lib/ht/ht.hpp"
#include "lib/bitmap/bitmap.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/hal/plugins/cfg/nw/l2segment.hpp"
#include "nic/hal/plugins/cfg/lif/lif.hpp"
#include "nic/hal/src/utils/utils.hpp"

#include "gen/proto/interface.pb.h"
using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;

using intf::LifSpec;
using kh::LifKeyHandle;
using intf::LifRequestMsg;
using intf::LifResponse;
using intf::LifResponseMsg;
using intf::LifDeleteRequestMsg;
using intf::LifDeleteResponseMsg;
using intf::LifGetRequestMsg;
using intf::LifGetResponseMsg;
using intf::InterfaceSpec;
using intf::InterfaceStats;
using intf::InterfaceStatus;
using intf::InterfaceResponse;
using kh::InterfaceKeyHandle;
using intf::InterfaceRequestMsg;
using intf::InterfaceResponseMsg;
using intf::InterfaceDeleteRequest;
using intf::InterfaceDeleteRequestMsg;
using intf::InterfaceDeleteResponseMsg;
using intf::InterfaceDeleteResponse;
using intf::InterfaceGetRequest;
using intf::InterfaceGetRequestMsg;
using intf::InterfaceGetResponse;
using intf::InterfaceGetResponseMsg;
using intf::InterfaceL2SegmentRequestMsg;
using intf::InterfaceL2SegmentSpec;
using intf::InterfaceL2SegmentResponseMsg;
using intf::InterfaceL2SegmentResponse;
using intf::QStateGetReq;
using intf::QStateGetResp;
using intf::QStateSetReq;
using intf::QStateSetResp;
using intf::GetQStateRequestMsg;
using intf::GetQStateResponseMsg;
using intf::SetQStateRequestMsg;
using intf::SetQStateResponseMsg;
using intf::IfType;
using intf::IfStatus;
using intf::IfEnicType;
using intf::IfEnicInfo;
using intf::IfTunnelEncapType;
using intf::InterfaceStats;

namespace hal {

// TMPORT ALL
#define TM_PORT_ALL         0xFFFFFFFF
#define TM_PORT_UPLINK_ALL  0xFFFFFFFE

// 0 - 127: Reserved IFs. Internally created in HAL.
#define IF_ID_CPU 1

// Max. number of uplinks
#define HAL_MAX_UPLINK_IF 32

#define	MAX_MIRROR_SESSION_DEST	8

extern uint8_t g_num_uplink_ifs;
extern std::vector<uint8_t > g_uplink_if_ids;

// l2seg entry used for classic enic if
typedef struct if_l2seg_entry_s {
    hal_handle_t    l2seg_handle;                   // l2segment handle
    void            *pd;                            // pd pointer
    dllist_ctxt_t   lentry;                         // list context
} __PACK__ if_l2seg_entry_t;

#define IF_ACL_REF_TYPE(ENTRY)                                \
    ENTRY(IF_ACL_REF_TYPE_SRC,              0, "src-if")      \
    ENTRY(IF_ACL_REF_TYPE_DEST,             1, "dest-if")     \
    ENTRY(IF_ACL_REF_TYPE_REDIRECT,         2, "redirect-if") \
    ENTRY(IF_ACL_REF_TYPES,                 3, "ref-types")

DEFINE_ENUM(if_acl_ref_type_t, IF_ACL_REF_TYPE)
#undef IF_ACL_REF_TYPE

enum uplink_erspan_direction_t {
    UPLINK_ERSPAN_DIRECTION_INGRESS = 0,
    UPLINK_ERSPAN_DIRECTION_EGRESS  = 1,
};

typedef struct if_mirror_info_s {
    uint8_t  tx_sessions_count;
    uint8_t  rx_sessions_count;
    uint32_t tx_session_id[MAX_MIRROR_SESSION_DEST];
    uint32_t rx_session_id[MAX_MIRROR_SESSION_DEST];
} if_mirror_info_t;

// Interface strucutre
typedef struct if_s {
    sdk_spinlock_t      slock;                      // lock to protect this structure
    if_id_t             if_id;                      // interface id
    IfType              if_type;                    // interface type
    IfStatus            if_admin_status;            // admin status
    vrf_id_t            tid;                        // vrf id (TODO: what is this for ?)

    union {
        // enic interface info
        struct {
            IfEnicType          enic_type;          // type of ENIC
            hal_handle_t        lif_handle;         // handle to corresponding LIF
            hal_handle_t        l2seg_handle;       // handle to l2seg
            mac_addr_t          mac_addr;           // EP's MAC addr
            vlan_id_t           encap_vlan;         // vlan enabled on this if
            // classic mode fields
            hal_handle_t        native_l2seg_clsc;  // native l2seg
            hal_handle_t        pinned_uplink;      // pinned uplink
            // Smart host nic mode
            bool                egress_en;          // valid only for smart nic host-pin/switch
            bool                lif_learned;        // lif mapping learnt thru ep learn
        } __PACK__;

        // uplink interface info
        struct {
            // doesnt have to exist. hence storing the id. have to exist only
            //    on add_l2seg_to_uplink
            l2seg_id_t          native_l2seg;       // native (vlan) on uplink (pc).

            // TOOD: List of L2segs on this Uplink
            // uplink if
            struct {
                uint32_t        fp_port_num;
                uint32_t        uplink_port_num;    // uplink port number
                bool            is_pc_mbr;          // is a PC member
                hal_handle_t    uplinkpc_handle;    // PC its part of
                bool            is_oob_management;  // is a OOB management port
            } __PACK__;
            // uplink PC if
            struct {
                // uint32_t      uplink_pc_num;     // uplink port channel number
                // block_list      *pc_mbr_list;
            } __PACK__;
        } __PACK__;
        // tunnel interface info
        struct {
            IfTunnelEncapType encap_type;           // type of Encap
            hal_handle_t rtep_ep_handle;            // Remote TEP EP's handle
            // TODO: have to add back ref from ep
            union {
                /* VxLAN tunnel info */
                struct {
                    ip_addr_t vxlan_ltep;           // Local TEP
                    ip_addr_t vxlan_rtep;           // Remote TEP
                } __PACK__;
                /* GRE tunnel info */
                struct {
                    ip_addr_t gre_source;   // Tunnel source
                    ip_addr_t gre_dest;     // Tunnel destination
                    uint32_t  gre_mtu;
                    uint32_t  gre_ttl;
                } __PACK__;
                /* Proprietary MPLSoUDP tunnel */
                struct {
                    ipv4_addr_t     substrate_ip;    // Substrate/tunnel outer dest. ip-address (incoming)
                    ipv4_addr_t     overlay_ip[2];   // Overlay/inner dest. ip-address (incoming)
                    mpls_tag_t      mpls_if[2];      // MPLS tag information (incoming)
                    ipv4_addr_t     tun_dst_ip;      // Tunnel/outer dest. ip-address (outgoing)
                    mpls_tag_t      mpls_tag;        // MPLS tag information (outgoing)
                    ipv4_prefix_t   source_gw;       // Source gateway ipv4 prefix for MPLSoUDP
                    mac_addr_t      gw_mac_da;       // Dest. MAC address of the gateway
                    mac_addr_t      overlay_mac;     // Overlay MAC address
                    mac_addr_t      pf_mac;          // MAC Address of the PF (Physical Function)
                    uint64_t        ingress_bw;      // Ingress BW (in KBytes/sec)
                    uint64_t        egress_bw;       // Egress BW (in KBytes/sec)
                    uint8_t         num_overlay_ip;
                    uint8_t         num_mpls_if;
                    uint32_t        lif_id;
                } __PACK__;
                /* Add structs for other tunnel types */
            } __PACK__;
        } __PACK__;
    } __PACK__;

    // operational state of interface
    hal_handle_t        hal_handle;             // HAL allocated handle
    uint32_t            num_ep;                 // no. of endpoints
    IfStatus            if_op_status;           // operational status

    // forward references
    block_list          *mbr_if_list;           // list of member ports for uplink PC
    dllist_ctxt_t       l2seg_list_clsc_head;   // l2segments in classic nic
                                                // mode for enic ifs.
                                                // l2seg_entry_classic_t
    // back references
    block_list          *l2seg_list;            // l2segments, add_l2seg_on_uplink
    block_list          *enicif_list;           // enicifs, Classic enics
    block_list          *acl_list[IF_ACL_REF_TYPES]; // List of acls matching on src if
    block_list          *nh_list;               // next hops
    block_list          *ep_list;               // endpoints
#if 0
    dllist_ctxt_t       mbr_if_list_head;       // list of member ports for uplink PC
    dllist_ctxt_t       l2seg_list_clsc_head;   // l2segments in classic nic
                                                // mode for enic ifs.
                                                // l2seg_entry_classic_t
#endif
    // back references
    // Uplinks
    // dllist_ctxt_t       l2seg_list_head;        // l2segments, add_l2seg_on_uplink
    // dllist_ctxt_t       enicif_list_head;       // enicifs, Classic enics

    // dllist_ctxt_t       ep_list_head;       // endpoints behind this interface
    // dllist_ctxt_t       session_list_head;  // session from this

    dllist_ctxt_t       mc_entry_list_head;    // mc_entries that have this if in its OIF list

    // PD Uplink/Enic ... Interpret based on type ... Careful!!
    void                *pd_if;

    // MirrorSessions related fields
    if_mirror_info_t    mirror_spec;
    if_mirror_info_t    mirror_cfg;
    uint8_t             tx_mirror_session_id[MAX_MIRROR_SESSION_DEST];
    uint8_t             rx_mirror_session_id[MAX_MIRROR_SESSION_DEST];
    uplink_erspan_direction_t direction;

    uint32_t                  hw_lif_id;
    uint32_t                  lport_id;
} __PACK__ if_t;

typedef struct if_create_app_ctxt_s {
    // l2seg_t    *l2seg;                                 // valid for enic if
    // lif_t      *lif;                                   // valid for enic if
} __PACK__ if_create_app_ctxt_t;

typedef struct if_update_app_ctxt_s {
    union {
        // uplink interface/pc info
        struct {
            bool            native_l2seg_change;
            // bool            l2segids_change;
            l2seg_t         *native_l2seg;             // native (vlan) on uplink (pc)

            bool           is_oob_change;
            bool           is_oob;

            // only to PC
            bool            mbrlist_change;
            block_list      *add_mbrlist;
            block_list      *del_mbrlist;
            block_list      *aggr_mbrlist;


            // dllist_ctxt_t   *add_mbrlist;
            // dllist_ctxt_t   *del_mbrlist;
            // dllist_ctxt_t   *aggr_mbrlist;
        } __PACK__;

        // enicif interface info
        struct {
            // classic: native l2seg change
            bool            native_l2seg_clsc_change;
            hal_handle_t    new_native_l2seg_clsc;

            // classic: pinned uplink change
            bool            pinned_uplink_change;
            hal_handle_t    new_pinned_uplink;

            // classic: l2seg list change
            bool            l2segclsclist_change;
            dllist_ctxt_t   *add_l2segclsclist;
            dllist_ctxt_t   *del_l2segclsclist;
            // dllist_ctxt_t   *aggr_l2segclsclist;

            // host-pin: lif change
            bool            lif_change;
            lif_t           *lif;

            // host-pin: encap vlan change
            bool            encap_vlan_change;
            vlan_id_t       new_encap_vlan;
        } __PACK__;
    } __PACK__;

} __PACK__ if_update_app_ctxt_t;

// max. number of interfaces supported  (TODO: we can take this from cfg file)
#define HAL_MAX_INTERFACES                           2048

typedef struct if_port_event_cb_ctxt_s {
    uint32_t fp_port_num;
    if_t *hal_if;
} __PACK__ if_port_event_cb_ctxt_t;

typedef struct if_port_timer_ctxt_s {
    uint32_t           port_num;
    port_oper_status_t oper_status;
    port_event_t       event;
    port_speed_t       port_speed;
    port_type_t        port_type;
    uint32_t           num_lanes;
} __PACK__ if_port_timer_ctxt_t;

static inline void
if_lock (if_t *hal_if, const char *fname, int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("operlock:locking if:{} from {}:{}:{}",
                    hal_if->if_id, fname, lineno, fxname);
    SDK_SPINLOCK_LOCK(&hal_if->slock);
}

static inline void
if_unlock (if_t *hal_if, const char *fname, int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("operlock:unlocking if:{} from {}:{}:{}",
                    hal_if->if_id, fname, lineno, fxname);
    SDK_SPINLOCK_UNLOCK(&hal_if->slock);
}

static inline bool
is_if_type_tunnel (if_t *if_p)
{
    return (if_p->if_type == intf::IF_TYPE_TUNNEL);
}

if_t *find_if_by_id(if_id_t if_id);
if_t *find_if_by_handle(hal_handle_t handle);
if_id_t find_if_id_from_hal_handle(hal_handle_t handle);
hal_handle_id_ht_entry_t *find_handle_obj_by_if_id(if_id_t if_id);
hal_handle_t find_hal_handle_from_if_id (if_id_t if_id);
lif_t *find_lif_by_if_handle(hal_handle_t if_handle);
extern void *if_id_get_key_func(void *entry);
extern uint32_t if_id_key_size(void);
hal_ret_t uplinkif_add_uplinkpc(if_t *upif, if_t *uppc);
hal_ret_t uplinkif_del_uplinkpc(if_t *upif, if_t *uppc);
hal_ret_t uplinkpc_update_mbrs_relation(block_list *mbr_list,
                                        if_t *uppc, bool add);
hal_ret_t uplinkpc_add_uplinkif(if_t *uppc, if_t *upif);
hal_ret_t uplinkpc_del_uplinkif(if_t *uppc, if_t *upif);
hal_ret_t uplinkpc_mbr_list_update(InterfaceSpec& spec, if_t *hal_if,
                                   bool *mbrlist_change,
                                   block_list **add_mbrlist,
                                   block_list **del_mbrlist,
                                   block_list **aggr_mbrlist);
hal_ret_t interface_cleanup_handle_list(dllist_ctxt_t **list);
void enicif_print_l2seg_entry_list(dllist_ctxt_t  *list);
void enicif_free_l2seg_entry_list(dllist_ctxt_t *list);
hal_ret_t enic_if_upd_l2seg_list_update(InterfaceSpec& spec, if_t *hal_if,
                                        bool *l2seglist_change,
                                        dllist_ctxt_t **add_l2seglist,
                                        dllist_ctxt_t **del_l2seglist);
hal_ret_t enicif_update_l2segs_relation(dllist_ctxt_t *l2segs_list,
                                        if_t *hal_if, bool add);
hal_ret_t enicif_update_l2segs_oif_lists(if_t *hal_if, lif_t *lif, bool add);
hal_ret_t enicif_cleanup_l2seg_entry_list(dllist_ctxt_t **list);
bool l2seg_in_classic_enicif(if_t *hal_if, hal_handle_t l2seg_handle,
                             if_l2seg_entry_t **l2seg_entry);

hal_ret_t enic_if_create(const InterfaceSpec& spec,
                         if_t *hal_if);
hal_ret_t uplink_if_create(const InterfaceSpec& spec,
                           if_t *hal_if);
hal_ret_t uplink_pc_create(const InterfaceSpec& spec,
                           if_t *hal_if);
hal_ret_t cpu_if_create(const InterfaceSpec& spec,
                        if_t *hal_if);
hal_ret_t app_redir_if_create(const InterfaceSpec& spec,
                              if_t *hal_if);
hal_ret_t uplink_if_update(InterfaceSpec& spec,
                           InterfaceResponse *rsp,
                           if_t *hal_if, void *if_args);
hal_ret_t uplink_pc_update(InterfaceSpec& spec,
                           InterfaceResponse *rsp,
                           if_t *hal_if,
                           void *if_args);
hal_ret_t tunnel_if_create(const InterfaceSpec& spec,
                           if_t *hal_if);
hal_ret_t get_lif_handle_for_enic_if(const InterfaceSpec& spec,
                                     if_t *hal_if);
hal_ret_t get_lif_handle_for_cpu_if(const InterfaceSpec& spec,
                                    if_t *hal_if);
hal_ret_t get_lif_handle_for_app_redir_if(const InterfaceSpec& spec,
                                          if_t *hal_if);
if_t *if_lookup_key_or_handle(const InterfaceKeyHandle& key_handle);
const char *if_spec_keyhandle_to_str(const InterfaceKeyHandle& key_handle);
const char *if_keyhandle_to_str(if_t *hal_if);

hal_ret_t lif_create(LifSpec& spec, LifResponse *rsp,
                     lif_hal_info_t *lif_hal_info);
hal_ret_t interface_create(InterfaceSpec& spec,
                           InterfaceResponse *rsp);
hal_ret_t interface_update(InterfaceSpec& spec,
                           InterfaceResponse *rsp);
hal_ret_t interface_delete(InterfaceDeleteRequest& req,
                           InterfaceDeleteResponse *rsp);
hal_ret_t interface_get(InterfaceGetRequest& spec,
                        InterfaceGetResponseMsg *rsp);
hal_ret_t add_l2seg_on_uplink(InterfaceL2SegmentSpec& spec,
                              InterfaceL2SegmentResponse *rsp);
hal_ret_t del_l2seg_on_uplink(InterfaceL2SegmentSpec& spec,
                              InterfaceL2SegmentResponse *rsp);

hal_ret_t if_store_cb(void *obj, uint8_t *mem, uint32_t len, uint32_t *mlen);
uint32_t if_restore_cb(void *obj, uint32_t len);

hal_ret_t enicif_update_egress_en(if_t *hal_if, bool egress_en);
hal_ret_t get_lif_handle_from_spec (const InterfaceSpec& spec,
                                    hal_handle_t *lif_handle);
hal_ret_t if_update_classic_oif_lists(if_t *hal_if,
                                      lif_update_app_ctxt_t *lif_upd);
hal_ret_t enic_update_lif(if_t *hal_if, lif_t *new_lif,
                          if_t **new_hal_if);
hal_ret_t enicif_clsc_l2seglist_change_update_oiflists(if_t *hal_if,
                                                       dllist_ctxt_t *l2seg_list,
                                                       bool add);
hal_ret_t hal_if_repin_l2segs(if_t *uplink);
hal_ret_t hal_if_repin_mirror_sessions(if_t *uplink);
hal_ret_t hal_if_repin_ipfix_flows(if_t *uplink);
hal_ret_t if_port_oper_state_process_event(uint32_t fp_port_num, port_event_t event);
const char*if_status_to_str(IfStatus status);
sdk_ret_t port_event_timer_cb (void *timer, uint32_t timer_id, void *ctxt);
void port_event_cb (port_event_info_t *port_event_info);
uint32_t uplink_if_get_idx (if_t *hal_if);
bool enicif_is_swm(if_t *hal_if);
hal_ret_t enicif_update_host_prom(bool add);

}    // namespace hal

#endif    // __INTERFACE_HPP__

