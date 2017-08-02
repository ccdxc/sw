#ifndef __INTERFACE_HPP__
#define __INTERFACE_HPP__

#include <base.h>
#include <eth.h>
#include <list.hpp>
#include <ht.hpp>
#include <bitmap.hpp>
#include <l2segment.hpp>
#include <interface.pb.h>
// #include <interface_svc.hpp>

using hal::utils::ht_ctxt_t;
using hal::utils::dllist_ctxt_t;
using hal::utils::bitmap;

namespace hal {

typedef uint16_t if_hw_id_t;
typedef uint64_t if_id_t;
typedef uint32_t lif_id_t;

// LIF structure
// TODO: capture Q information etc.
typedef struct lif_s {
    hal_spinlock_t      slock;                       // lock to protect this structure
    lif_id_t            lif_id;                      // lif id assigned
    mac_addr_t          mac_addr;                    // LIF's MAC address, if any
    uint32_t            port_num;                    // LIF's port number
    intf::IfStatus      admin_status;                // admin status
    bool                vlan_strip_en;               // vlan strip enable
    bool                allmulti;                    // All multicast enable

    // operational state of interface
    hal_handle_t        hal_handle;                  // HAL allocated handle

    ht_ctxt_t           ht_ctxt;                     // lif id based hash table ctxt
    ht_ctxt_t           hal_handle_ht_ctxt;          // hal handle based hash table ctxt
    dllist_ctxt_t       if_list_head;                // interfaces behind this lif

    void                *pd_lif;
} __PACK__ lif_t;

// Interface strucutre
typedef struct if_s {
    hal_spinlock_t      slock;                       // lock to protect this structure
    if_id_t             if_id;                       // interface id
    intf::IfType        if_type;                     // interface type
    intf::IfStatus      if_admin_status;             // admin status

    union {
        // enic interface info
        struct {
            intf::IfEnicType    enic_type;           // type of ENIC
            hal_handle_t        lif_handle;          // handle to corresponding LIF
			tenant_id_t			tid;				 // tenant id
            l2seg_id_t          l2seg_id;            // user VLAN or L2 segment
            mac_addr_t          mac_addr;            // EP's MAC addr
            vlan_id_t           encap_vlan;          // vlan enabled on this if
        } __PACK__;

        // uplink interface info
        // TODO: grouped port and pc together for now !!
        struct {
            uint32_t      uplink_port_num;           // uplink port number
            uint32_t      uplink_pc_num;             // uplink port channel number
            l2seg_id_t    native_l2seg;              // native (vlan) on uplink (pc)
            bitmap        *vlans;                    // vlans up on this interface
        };
    } __PACK__;

    // operational state of interface
    hal_handle_t        hal_handle;                  // HAL allocated handle
    uint32_t            num_ep;                      // no. of endpoints
    intf::IfStatus      if_op_status;                // operational status

    // PD state
    if_hw_id_t          hw_id;                       // h/w id used in data plane lookups
    pd::pd_if_t         *pd;                         // all PD specific state

    // meta data maintained for interface
    ht_ctxt_t           ht_ctxt;                     // interface id based hash table ctxt
    ht_ctxt_t           hw_ht_ctxt;                  // h/w id based hash table ctxt
    ht_ctxt_t           hal_handle_ht_ctxt;          // hal handle based hash table ctxt
    dllist_ctxt_t       ep_list_head;                // endpoints behind this interface
    dllist_ctxt_t       session_list_head;           // session from this

    // PD Uplink/Enic ... Interpret based on type ... Careful!!
    void                *pd_if;                      
} __PACK__ if_t;

// max. number of interfaces supported  (TODO: we can take this from cfg file)
#define HAL_MAX_INTERFACES                           2048
#define HAL_MAX_LIFS                                 1024

// allocate a LIF instance
static inline lif_t *
lif_alloc (void)
{
    lif_t    *lif;

    lif = (lif_t *)g_hal_state->lif_slab()->alloc();
    if (lif == NULL) {
        return NULL;
    }
    return lif;
}

// initialize a LIF instance
static inline lif_t *
lif_init (lif_t *lif)
{
    if (!lif) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&lif->slock, PTHREAD_PROCESS_PRIVATE);

    // initialize the operational state

    // initialize meta information
    lif->ht_ctxt.reset();
    lif->hal_handle_ht_ctxt.reset();
    utils::dllist_reset(&lif->if_list_head);

    return lif;
}

// allocate and initialize a interface instance
static inline lif_t *
lif_alloc_init (void)
{
    return lif_init(lif_alloc());
}

static inline hal_ret_t
lif_free (lif_t *lif)
{
    HAL_SPINLOCK_DESTROY(&lif->slock);
    g_hal_state->lif_slab()->free(lif);
    return HAL_RET_OK;
}

static inline lif_t *
find_lif_by_id (lif_id_t lif_id)
{
    return (lif_t *)g_hal_state->lif_id_ht()->lookup(&lif_id);
}

static inline lif_t *
find_lif_by_handle (hal_handle_t handle)
{
    return (lif_t *)g_hal_state->lif_hal_handle_ht()->lookup(&handle);
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
    hal_if->vlans = NULL;
    // hal_if->vlan_bmap = NULL;

    // initialize the operational state
    hal_if->num_ep = 0;
    hal_if->pd = NULL;

    // initialize meta information
    hal_if->ht_ctxt.reset();
    hal_if->hal_handle_ht_ctxt.reset();
    hal_if->hw_ht_ctxt.reset();
    utils::dllist_reset(&hal_if->ep_list_head);
    utils::dllist_reset(&hal_if->session_list_head);

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
    if (hal_if->vlans) {
        delete hal_if->vlans;
    }
#if 0
    if (hal_if->vlan_bmap) {
        delete hal_if->vlan_bmap;
    }
#endif
    g_hal_state->if_slab()->free(hal_if);
    return HAL_RET_OK;
}

static inline if_t *
find_if_by_id (if_id_t if_id)
{
    return (if_t *)g_hal_state->if_id_ht()->lookup(&if_id);
}

static inline if_t *
find_if_by_handle (hal_handle_t handle)
{
    return (if_t *)g_hal_state->if_hal_handle_ht()->lookup(&handle);
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

extern void *lif_get_key_func(void *entry);
extern uint32_t lif_compute_hash_func(void *key, uint32_t ht_size);
extern bool lif_compare_key_func(void *key1, void *key2);

extern void *lif_get_handle_key_func(void *entry);
extern uint32_t lif_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool lif_compare_handle_key_func(void *key1, void *key2);

extern void *if_get_key_func(void *entry);
extern uint32_t if_compute_hash_func(void *key, uint32_t ht_size);
extern bool if_compare_key_func(void *key1, void *key2);

extern void *if_get_hw_key_func(void *entry);
extern uint32_t if_compute_hw_hash_func(void *key, uint32_t ht_size);
extern bool if_compare_hw_key_func(void *key1, void *key2);

extern void *if_get_handle_key_func(void *entry);
extern uint32_t if_compute_handle_hash_func(void *key, uint32_t ht_size);
extern bool if_compare_handle_key_func(void *key1, void *key2);

hal_ret_t enicif_create(intf::InterfaceSpec& spec, 
                        intf::InterfaceResponse *rsp,
                        if_t *hal_if);
hal_ret_t uplinkif_create(intf::InterfaceSpec& spec, 
                          intf::InterfaceResponse *rsp,
                          if_t *hal_if);
hal_ret_t get_lif_hdl_for_enicif(intf::InterfaceSpec& spec, 
                                 intf::InterfaceResponse *rsp,
                                 if_t *hal_if);

hal_ret_t lif_create(intf::LifSpec& spec, intf::LifResponse *rsp);
hal_ret_t interface_create(intf::InterfaceSpec& spec,
                           intf::InterfaceResponse *rsp);
hal_ret_t interface_update(intf::InterfaceSpec& spec,
                           intf::InterfaceResponse *rsp);
hal_ret_t interface_get(intf::InterfaceGetRequest& spec,
                        intf::InterfaceGetResponse *rsp);

hal_ret_t add_l2seg_on_uplink(intf::InterfaceL2SegmentSpec& spec,
                              intf::InterfaceL2SegmentResponse *rsp);

hal_ret_t del_l2seg_on_uplink(intf::InterfaceL2SegmentSpec& spec,
                              intf::InterfaceL2SegmentResponse *rsp);
}    // namespace hal

#endif    // __INTERFACE_HPP__

