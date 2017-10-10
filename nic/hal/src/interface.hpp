#ifndef __INTERFACE_HPP__
#define __INTERFACE_HPP__

#include "nic/include/base.h"
#include "nic/include/eth.h"
#include "nic/include/ip.h"
#include "nic/include/list.hpp"
#include "nic/utils/ht/ht.hpp"
#include "nic/include/bitmap.hpp"
#include "nic/proto/hal/interface.pb.h"
#include "nic/hal/src/tenant.hpp"
#include "nic/hal/src/l2segment.hpp"
#include "nic/hal/src/lif.hpp"

using hal::utils::ht_ctxt_t;
using hal::utils::dllist_ctxt_t;
using hal::utils::bitmap;

using intf::LifSpec;
using intf::LifKeyHandle;
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
using intf::InterfaceKeyHandle;
using intf::InterfaceRequestMsg;
using intf::InterfaceResponseMsg;
using intf::InterfaceDeleteRequestMsg;
using intf::InterfaceDeleteRequest;
using intf::InterfaceDeleteResponseMsg;
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

namespace hal {

typedef uint64_t if_id_t;

// Interface strucutre
typedef struct if_s {
    hal_spinlock_t      slock;                       // lock to protect this structure
    if_id_t             if_id;                       // interface id
    intf::IfType        if_type;                     // interface type
    intf::IfStatus      if_admin_status;             // admin status
    tenant_id_t         tid;                         // tenant id (TODO: what is this for ?)

    union {
        // enic interface info
        struct {
            intf::IfEnicType    enic_type;           // type of ENIC
            hal_handle_t        lif_handle;          // handle to corresponding LIF
            hal_handle_t        l2seg_handle;        // handle to l2seg
            mac_addr_t          mac_addr;            // EP's MAC addr
            vlan_id_t           encap_vlan;          // vlan enabled on this if
        } __PACK__;

        // uplink interface info
        struct {
            l2seg_id_t          native_l2seg;              // native (vlan) on uplink (pc)

            // TOOD: List of L2segs on this Uplink
            // uplink if
            struct {
                uint32_t      uplink_port_num;       // uplink port number
            } __PACK__;
            // uplink PC if
            struct {
                uint32_t      uplink_pc_num;         // uplink port channel number
            } __PACK__;
        } __PACK__;
        // tunnel interface info
        struct {
            intf::IfTunnelEncapType encap_type;   // type of Encap
            union {
                /* VxLAN tunnel info */
                struct {
                    ip_addr_t vxlan_ltep; // Local TEP
                    ip_addr_t vxlan_rtep; // Remote TEP
                } __PACK__;
                /* Add structs for other tunnel types */
            } __PACK__;
        } __PACK__;
    } __PACK__;

    // operational state of interface
    hal_handle_t        hal_handle;         // HAL allocated handle
    uint32_t            num_ep;             // no. of endpoints
    intf::IfStatus      if_op_status;       // operational status

    // meta data maintained for interface
    dllist_ctxt_t       l2seg_list_head;    // l2segments - valid only for uplinks
    dllist_ctxt_t       mbr_if_list_head;   // list of member ports for uplink PC
    // dllist_ctxt_t       ep_list_head;       // endpoints behind this interface
    // dllist_ctxt_t       session_list_head;  // session from this

    // PD Uplink/Enic ... Interpret based on type ... Careful!!
    void                *pd_if;                      
} __PACK__ if_t;

typedef struct if_create_app_ctxt_s {
    l2seg_t    *l2seg;                                 // valid for enic if
    lif_t      *lif;                                   // valid for enic if 
} __PACK__ if_create_app_ctxt_t;

typedef struct if_update_app_ctxt_s {
    union {
        // uplink interface/pc info
        struct {
            bool            native_l2seg_change;
            bool            l2segids_change;
            l2seg_t         *native_l2seg;             // native (vlan) on uplink (pc)

            // only to PC
            bool            mbrlist_change;
            dllist_ctxt_t   *add_mbrlist;
            dllist_ctxt_t   *del_mbrlist;
            dllist_ctxt_t   *aggr_mbrlist;
        } __PACK__;
    } __PACK__;

} __PACK__ if_update_app_ctxt_t;

// max. number of interfaces supported  (TODO: we can take this from cfg file)
#define HAL_MAX_INTERFACES                           2048

static inline void 
if_lock(if_t *hal_if)
{
    HAL_TRACE_DEBUG("{}:locking if:{}", __FUNCTION__, hal_if->if_id);
    HAL_SPINLOCK_LOCK(&hal_if->slock);
}

static inline void 
if_unlock(if_t *hal_if)
{
    HAL_TRACE_DEBUG("{}:unlocking if:{}", __FUNCTION__, hal_if->if_id);
    HAL_SPINLOCK_UNLOCK(&hal_if->slock);
}

// allocate a interface instance
static inline if_t *
if_alloc (void)
{
    if_t    *hal_if;

    hal_if = (if_t *)g_hal_state->if_slab()->alloc();
    if (hal_if == NULL) {
        return NULL;
    }
    return hal_if;
}

// initialize a interface instance
static inline if_t *
if_init (if_t *hal_if)
{
    if (!hal_if) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&hal_if->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state
    hal_if->num_ep = 0;
    hal_if->pd_if = NULL;

    // initialize meta information
    utils::dllist_reset(&hal_if->l2seg_list_head);
    utils::dllist_reset(&hal_if->mbr_if_list_head);

    return hal_if;
}

// allocate and initialize a interface instance
static inline if_t *
if_alloc_init (void)
{
    return if_init(if_alloc());
}

static inline hal_ret_t
if_free (if_t *hal_if)
{
    HAL_SPINLOCK_DESTROY(&hal_if->slock);
    HAL_TRACE_DEBUG("pi-if:{}:trying to free", __FUNCTION__);
    g_hal_state->if_slab()->free(hal_if);
    return HAL_RET_OK;
}

static inline hal_handle_id_ht_entry_t *
find_handle_obj_by_if_id (if_id_t if_id)
{
    hal_handle_id_ht_entry_t    *entry = NULL;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
        if_id_ht()->lookup(&if_id);

    return entry;
}

static inline if_t *
find_if_by_id (if_id_t if_id)
{
    hal_handle_id_ht_entry_t    *entry;
    if_t                        *hal_if;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
        if_id_ht()->lookup(&if_id);
    if (entry) {

        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() == 
                   HAL_OBJ_ID_INTERFACE);

        hal_if = (if_t *)hal_handle_get_obj(entry->handle_id);
        return hal_if;
    }
    return NULL;
}

static inline if_t *
find_if_by_handle (hal_handle_t handle)
{
    // check for object type
    HAL_ASSERT(hal_handle_get_from_handle_id(handle)->obj_id() == 
               HAL_OBJ_ID_INTERFACE);
    return (if_t *)hal_handle_get_obj(handle);
}

static inline bool
is_if_type_tunnel (if_t *if_p)
{
    return (if_p->if_type == intf::IF_TYPE_TUNNEL);
}

static inline lif_t *
find_lif_by_if_handle (hal_handle_t if_handle)
{
    if_t    *if_p;

    if_p = find_if_by_handle(if_handle);
    if (!if_p) {
        return NULL;
    }

    if (if_p->if_type == intf::IF_TYPE_ENIC) {
        return find_lif_by_handle(if_p->lif_handle);
    } else {
        return NULL;
    }

}

extern void *if_id_get_key_func(void *entry);
extern uint32_t if_id_compute_hash_func(void *key, uint32_t ht_size);
extern bool if_id_compare_key_func(void *key1, void *key2);

hal_ret_t uplinkpc_add_uplinkif (if_t *uppc, if_t *upif);
hal_ret_t uplinkpc_del_uplinkif (if_t *uppc, if_t *upif);
hal_ret_t
uplinkpc_mbr_list_update(InterfaceSpec& spec, if_t *hal_if,
                        bool *mbrlist_change,
                        dllist_ctxt_t **add_mbrlist, 
                        dllist_ctxt_t **del_mbrlist,
                        dllist_ctxt_t **aggr_mbrlist);



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
if_t *if_get_from_id_handle(const intf::InterfaceKeyHandle *key_handle);

void LifGetQState(const intf::QStateGetReq &req, intf::QStateGetResp *resp);
void LifSetQState(const intf::QStateSetReq &req, intf::QStateSetResp *resp);

hal_ret_t lif_create(intf::LifSpec& spec, intf::LifResponse *rsp, 
                     lif_hal_info_t *lif_hal_info);

hal_ret_t interface_create(intf::InterfaceSpec& spec,
                           intf::InterfaceResponse *rsp);
hal_ret_t interface_update(intf::InterfaceSpec& spec,
                           intf::InterfaceResponse *rsp);
hal_ret_t interface_delete(intf::InterfaceDeleteRequest& req,
                           intf::InterfaceDeleteResponseMsg *rsp);
hal_ret_t interface_get(intf::InterfaceGetRequest& spec,
                        intf::InterfaceGetResponse *rsp);

hal_ret_t add_l2seg_on_uplink(intf::InterfaceL2SegmentSpec& spec,
                              intf::InterfaceL2SegmentResponse *rsp);

hal_ret_t del_l2seg_on_uplink(intf::InterfaceL2SegmentSpec& spec,
                              intf::InterfaceL2SegmentResponse *rsp);
}    // namespace hal

#endif    // __INTERFACE_HPP__

