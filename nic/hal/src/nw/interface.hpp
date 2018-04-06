//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __INTERFACE_HPP__
#define __INTERFACE_HPP__

#include "nic/include/base.h"
#include "nic/include/eth.h"
#include "nic/include/ip.h"
#include "sdk/list.hpp"
#include "sdk/ht.hpp"
#include "nic/include/bitmap.hpp"
#include "nic/gen/proto/hal/interface.pb.h"
#include "nic/hal/src/nw/vrf.hpp"
#include "nic/hal/src/nw/l2segment.hpp"
#include "nic/hal/src/lif/lif.hpp"
#include "nic/hal/src/utils/utils.hpp"

using sdk::lib::ht_ctxt_t;
using sdk::lib::dllist_ctxt_t;
using hal::utils::bitmap;

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
using intf::InterfaceStatus;
using intf::InterfaceResponse;
using kh::InterfaceKeyHandle;
using intf::InterfaceRequestMsg;
using intf::InterfaceResponseMsg;
using intf::InterfaceDeleteRequestMsg;
using intf::InterfaceDeleteRequest;
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
using intf::GetQStateRequestMsg;
using intf::GetQStateResponseMsg;
using intf::SetQStateRequestMsg;
using intf::SetQStateResponseMsg;
using intf::IfType;

namespace hal {

#define IF_ID_CPU 1003


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

// Interface strucutre
typedef struct if_s {
    hal_spinlock_t      slock;                      // lock to protect this structure
    if_id_t             if_id;                      // interface id
    intf::IfType        if_type;                    // interface type
    intf::IfStatus      if_admin_status;            // admin status
    vrf_id_t            tid;                        // vrf id (TODO: what is this for ?)

    union {
        // enic interface info
        struct {
            intf::IfEnicType    enic_type;          // type of ENIC
            hal_handle_t        lif_handle;         // handle to corresponding LIF
            hal_handle_t        l2seg_handle;       // handle to l2seg
            mac_addr_t          mac_addr;           // EP's MAC addr
            vlan_id_t           encap_vlan;         // vlan enabled on this if
            // classic mode fields
            hal_handle_t        native_l2seg_clsc;  // native l2seg
            hal_handle_t        pinned_uplink;      // pinned uplink
        } __PACK__;

        // uplink interface info
        struct {
            // doesnt have to exist. hence storing the id. have to exist only
            //    on add_l2seg_to_uplink
            l2seg_id_t          native_l2seg;       // native (vlan) on uplink (pc).


            // TOOD: List of L2segs on this Uplink
            // uplink if
            struct {
                uint32_t        uplink_port_num;    // uplink port number
                bool            is_pc_mbr;          // is a PC member
                hal_handle_t    uplinkpc_handle;    // PC its part of
            } __PACK__;
            // uplink PC if
            struct {
                // uint32_t      uplink_pc_num;     // uplink port channel number
                // block_list      *pc_mbr_list;
            } __PACK__;
        } __PACK__;
        // tunnel interface info
        struct {
            intf::IfTunnelEncapType encap_type;     // type of Encap
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
                /* Add structs for other tunnel types */
            } __PACK__;
        } __PACK__;
    } __PACK__;

    // operational state of interface
    hal_handle_t        hal_handle;             // HAL allocated handle
    uint32_t            num_ep;                 // no. of endpoints
    intf::IfStatus      if_op_status;           // operational status

    // forward references
    dllist_ctxt_t       mbr_if_list_head;       // list of member ports for uplink PC
    dllist_ctxt_t       l2seg_list_clsc_head;   // l2segments in classic nic
                                                // mode for enic ifs.
                                                // l2seg_entry_classic_t
    // back references
    // Uplinks
    dllist_ctxt_t       l2seg_list_head;        // l2segments, add_l2seg_on_uplink
    dllist_ctxt_t       enicif_list_head;       // enicifs, Classic enics
    block_list          *acl_list[IF_ACL_REF_TYPES]; // List of acls matching on src if

    // dllist_ctxt_t       ep_list_head;       // endpoints behind this interface
    // dllist_ctxt_t       session_list_head;  // session from this

    dllist_ctxt_t       mc_entry_list_head;    // mc_entries that have this if in its OIF list

    // PD Uplink/Enic ... Interpret based on type ... Careful!!
    void                *pd_if;
} __PACK__ if_t;

typedef struct if_create_app_ctxt_s {
    // l2seg_t    *l2seg;                                 // valid for enic if
    lif_t      *lif;                                   // valid for enic if
} __PACK__ if_create_app_ctxt_t;

typedef struct if_update_app_ctxt_s {
    union {
        // uplink interface/pc info
        struct {
            bool            native_l2seg_change;
            // bool            l2segids_change;
            l2seg_t         *native_l2seg;             // native (vlan) on uplink (pc)

            // only to PC
            bool            mbrlist_change;
            dllist_ctxt_t   *add_mbrlist;
            dllist_ctxt_t   *del_mbrlist;
            dllist_ctxt_t   *aggr_mbrlist;
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
        } __PACK__;
    } __PACK__;

} __PACK__ if_update_app_ctxt_t;

// max. number of interfaces supported  (TODO: we can take this from cfg file)
#define HAL_MAX_INTERFACES                           2048

static inline void
if_lock (if_t *hal_if, const char *fname, int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("operlock:locking if:{} from {}:{}:{}",
                    hal_if->if_id, fname, lineno, fxname);
    HAL_SPINLOCK_LOCK(&hal_if->slock);
}

static inline void
if_unlock (if_t *hal_if, const char *fname, int lineno, const char *fxname)
{
    HAL_TRACE_DEBUG("operlock:unlocking if:{} from {}:{}:{}",
                    hal_if->if_id, fname, lineno, fxname);
    HAL_SPINLOCK_UNLOCK(&hal_if->slock);
}

static inline bool
is_if_type_tunnel (if_t *if_p)
{
    return (if_p->if_type == intf::IF_TYPE_TUNNEL);
}

if_t *find_if_by_id(if_id_t if_id);
if_t *find_if_by_handle(hal_handle_t handle);
hal_handle_id_ht_entry_t *find_handle_obj_by_if_id(if_id_t if_id);
lif_t *find_lif_by_if_handle(hal_handle_t if_handle);
extern void *if_id_get_key_func(void *entry);
extern uint32_t if_id_compute_hash_func(void *key, uint32_t ht_size);
extern bool if_id_compare_key_func(void *key1, void *key2);
hal_ret_t uplinkif_add_uplinkpc(if_t *upif, if_t *uppc);
hal_ret_t uplinkif_del_uplinkpc(if_t *upif, if_t *uppc);
hal_ret_t uplinkpc_update_mbrs_relation(dllist_ctxt_t *mbr_list,
                                        if_t *uppc, bool add);
hal_ret_t uplinkpc_add_uplinkif(if_t *uppc, if_t *upif);
hal_ret_t uplinkpc_del_uplinkif(if_t *uppc, if_t *upif);
hal_ret_t uplinkpc_mbr_list_update(InterfaceSpec& spec, if_t *hal_if,
                                   bool *mbrlist_change,
                                   dllist_ctxt_t **add_mbrlist,
                                   dllist_ctxt_t **del_mbrlist,
                                   dllist_ctxt_t **aggr_mbrlist);
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

hal_ret_t enic_if_create(intf::InterfaceSpec& spec,
                         intf::InterfaceResponse *rsp,
                         if_t *hal_if);
hal_ret_t uplink_if_create(intf::InterfaceSpec& spec,
                           intf::InterfaceResponse *rsp,
                           if_t *hal_if);
hal_ret_t uplink_pc_create(intf::InterfaceSpec& spec,
                           intf::InterfaceResponse *rsp,
                           if_t *hal_if);
hal_ret_t cpu_if_create(intf::InterfaceSpec& spec,
                        intf::InterfaceResponse *rsp,
                        if_t *hal_if);
hal_ret_t app_redir_if_create(intf::InterfaceSpec& spec,
                              intf::InterfaceResponse *rsp,
                              if_t *hal_if);
hal_ret_t uplink_if_update(intf::InterfaceSpec& spec,
                           intf::InterfaceResponse *rsp,
                           if_t *hal_if, void *if_args);
hal_ret_t uplink_pc_update(intf::InterfaceSpec& spec,
                           intf::InterfaceResponse *rsp,
                           if_t *hal_if,
                           void *if_args);
hal_ret_t tunnel_if_create(intf::InterfaceSpec& spec,
                           intf::InterfaceResponse *rsp,
                           if_t *hal_if);
hal_ret_t get_lif_handle_for_enic_if(intf::InterfaceSpec& spec,
                                     intf::InterfaceResponse *rsp,
                                     if_t *hal_if);
hal_ret_t get_lif_handle_for_cpu_if(intf::InterfaceSpec& spec,
                                    intf::InterfaceResponse *rsp,
                                    if_t *hal_if);
hal_ret_t get_lif_handle_for_app_redir_if(intf::InterfaceSpec& spec,
                                          intf::InterfaceResponse *rsp,
                                          if_t *hal_if);
if_t *if_lookup_key_or_handle(const kh::InterfaceKeyHandle& key_handle);

void LifGetQState(const intf::QStateGetReq &req, intf::QStateGetResp *resp);
void LifSetQState(const intf::QStateSetReq &req, intf::QStateSetResp *resp);

hal_ret_t lif_create(intf::LifSpec& spec, intf::LifResponse *rsp,
                     lif_hal_info_t *lif_hal_info);
hal_ret_t interface_create(intf::InterfaceSpec& spec,
                           intf::InterfaceResponse *rsp);
hal_ret_t interface_update(intf::InterfaceSpec& spec,
                           intf::InterfaceResponse *rsp);
hal_ret_t interface_delete(intf::InterfaceDeleteRequest& req,
                           intf::InterfaceDeleteResponse *rsp);
hal_ret_t interface_get(intf::InterfaceGetRequest& spec,
                        intf::InterfaceGetResponseMsg *rsp);
hal_ret_t add_l2seg_on_uplink(intf::InterfaceL2SegmentSpec& spec,
                              intf::InterfaceL2SegmentResponse *rsp);
hal_ret_t del_l2seg_on_uplink(intf::InterfaceL2SegmentSpec& spec,
                              intf::InterfaceL2SegmentResponse *rsp);

hal_ret_t if_marshall_cb(void *obj, uint8_t *mem, uint32_t len, uint32_t *mlen);

}    // namespace hal

#endif    // __INTERFACE_HPP__

