//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <google/protobuf/util/json_util.h>
#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/gen/hal/include/hal_api_stats.hpp"
#include "nic/hal/src/utils/utils.hpp"
#include "nic/hal/src/nw/endpoint.hpp"
#include "nic/hal/src/nw/interface.hpp"
#include "nic/hal/src/nw/nh.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/endpoint_api.hpp"
#include "nic/hal/src/firewall/nwsec.hpp"
#include "nic/hal/src/firewall/nwsec_group.hpp"

namespace hal {

dhcp_status_func_t dhcp_status_func = nullptr;
arp_status_func_t arp_status_func = nullptr;

//-----------------------------------------------------------------------------
// hash table l2key => ht_entry
//  - Get key from entry
//-----------------------------------------------------------------------------
void *
ep_get_l2_key_func (void *entry)
{
    hal_handle_id_ht_entry_t    *ht_entry;
    ep_t                        *ep = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    ep = find_ep_by_handle(ht_entry->handle_id);
    return (void *)&(ep->l2_key);
}

//-----------------------------------------------------------------------------
// hash table l2_key => entry - compute hash
//-----------------------------------------------------------------------------
uint32_t
ep_compute_l2_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(ep_l2_key_t)) % ht_size;
}

//-----------------------------------------------------------------------------
// hash table l2_key => entry - compare function
//-----------------------------------------------------------------------------
bool
ep_compare_l2_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(ep_l2_key_t))) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// Get l3 key function
//------------------------------------------------------------------------------
void *
ep_get_l3_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((ep_l3_entry_t *)entry)->l3_key);
}

//------------------------------------------------------------------------------
// Compute l3 hash
//------------------------------------------------------------------------------
uint32_t
ep_compute_l3_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(ep_l3_key_t)) % ht_size;
}

//------------------------------------------------------------------------------
// Compare l3 key
//------------------------------------------------------------------------------
bool
ep_compare_l3_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(ep_l3_key_t))) {
        return true;
    }
    return false;
}

// allocate a ep instance
static inline ep_t *
ep_alloc (void)
{
    ep_t    *ep;

    ep = (ep_t *)g_hal_state->ep_slab()->alloc();
    if (ep == NULL) {
        return NULL;
    }
    return ep;
}

// initialize a ep instance
static inline ep_t *
ep_init (ep_t *ep)
{
    if (!ep) {
        return NULL;
    }
    memset(ep, 0, sizeof(ep_t));
    HAL_SPINLOCK_INIT(&ep->slock, PTHREAD_PROCESS_SHARED);

    sdk::lib::dllist_reset(&ep->ip_list_head);
    sdk::lib::dllist_reset(&ep->session_list_head);
    ep->nh_list = block_list::factory(sizeof(hal_handle_t));
    ep->sgs.sg_id_cnt = 0;
    memset(&ep->sgs.arr_sg_id, 0 , MAX_SG_PER_ARRAY);

    return ep;
}

// allocate and initialize a ep instance
static inline ep_t *
ep_alloc_init (void)
{
    return ep_init(ep_alloc());
}

//------------------------------------------------------------------------------
// Partial Cleanup of Object: (Generally called when destroying original after
//                             copying original to clone)
// - clone will have its own slock. So we can destroy in original
// - Lists are copied to clone, so dont destory lists
//------------------------------------------------------------------------------
static inline hal_ret_t
ep_free (ep_t *ep)
{
    HAL_SPINLOCK_DESTROY(&ep->slock);

    // TODO: may have to free list of ip entries
    hal::delay_delete_to_slab(HAL_SLAB_EP, ep);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Total Cleanup of Object:
// - Assume nothing ni this object is being used
// - So destorying lists and freeing up
//------------------------------------------------------------------------------
static inline hal_ret_t
ep_cleanup (ep_t *ep)
{
    block_list::destroy(ep->nh_list);

    ep_free(ep);

    return HAL_RET_OK;
}

ep_t *
find_ep_by_l2_key (l2seg_id_t l2seg_id, const mac_addr_t mac_addr)
{
    hal_handle_id_ht_entry_t    *entry;
    ep_l2_key_t                 l2_key = { 0 };
    ep_t                        *ep;

    l2_key.l2_segid = l2seg_id;
    memcpy(&l2_key.mac_addr, mac_addr, ETH_ADDR_LEN);

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
        ep_l2_ht()->lookup(&l2_key);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {
        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() ==
                   HAL_OBJ_ID_ENDPOINT);
        ep = (ep_t *)hal_handle_get_obj(entry->handle_id);
        return ep;
    }
    return NULL;
}

// find EP from hal handle
ep_t *
find_ep_by_handle (hal_handle_t handle)
{
    hal_handle *handle_obj;

    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }

    // check for object type
    handle_obj = hal_handle_get_from_handle_id(handle);
    if (!handle_obj || handle_obj->obj_id() != HAL_OBJ_ID_ENDPOINT) {
        return NULL;
    }
    return (ep_t *)hal_handle_get_obj(handle);
}

// find EP from l3 key
ep_t *
find_ep_by_l3_key (ep_l3_key_t *ep_l3_key)
{
    ep_l3_entry_t    *ep_l3_entry;

    HAL_ASSERT(ep_l3_key != NULL);
    ep_l3_entry =
        (ep_l3_entry_t *)g_hal_state->ep_l3_entry_ht()->lookup(ep_l3_key);
    if (ep_l3_entry == NULL) {
        return NULL;
    }
    return find_ep_by_handle(ep_l3_entry->ep_hal_handle);
}

// find EP from v4 key
ep_t *
find_ep_by_v4_key (vrf_id_t tid, uint32_t v4_addr)
{
    ep_l3_key_t    l3_key = { 0 };

    l3_key.vrf_id = tid;
    l3_key.ip_addr.af = IP_AF_IPV4;
    l3_key.ip_addr.addr.v4_addr = v4_addr;
    return find_ep_by_l3_key(&l3_key);
}

// find EP from v6 key
ep_t *
find_ep_by_v6_key (vrf_id_t tid, const ip_addr_t *ip_addr)
{
    ep_l3_key_t    l3_key = { 0 };

    l3_key.vrf_id = tid;
    memcpy(&l3_key.ip_addr, ip_addr, sizeof(ip_addr_t));
    return find_ep_by_l3_key(&l3_key);
}

//------------------------------------------------------------------------------
// insert an ep to l2 db
//------------------------------------------------------------------------------
static inline hal_ret_t
ep_add_to_l2_db (ep_t *ep, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Adding EP to L2 hash table");
    // allocate an entry to establish mapping from l2key to its handle
    entry =
        (hal_handle_id_ht_entry_t *)
            g_hal_state->hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from vrf id to its handle
    entry->handle_id = handle;
    sdk_ret = g_hal_state->ep_l2_ht()->insert_with_key(&ep->l2_key,
                                                       entry, &entry->ht_ctxt);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add l2 key to handle mapping, err : {}", ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    // TODO: Check if this is the right place
    ep->hal_handle = handle;

    return ret;
}

//------------------------------------------------------------------------------
// delete an ep from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
ep_del_from_l2_db (ep_t *ep)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Removing EP from L2 hash table");
    // remove from hash table
    entry =
        (hal_handle_id_ht_entry_t *)
            g_hal_state->ep_l2_ht()->remove(&ep->l2_key);

    if (entry) {
        // free up
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert an ep to l3 db
//------------------------------------------------------------------------------
static inline hal_ret_t
ep_add_to_l3_db (ep_l3_key_t *l3_key, ep_ip_entry_t *ep_ip,
                 hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    ep_l3_entry_t               *entry;

    HAL_TRACE_DEBUG("Adding to ep l2 hash table");
    // allocate an entry to establish mapping from l3key to its handle
    entry = (ep_l3_entry_t *)g_hal_state->ep_l3_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from vrf id to its handle
    entry->ep_hal_handle = handle;
    entry->l3_key = *l3_key;
    entry->ep_ip = ep_ip;
    entry->ht_ctxt.reset();
    sdk_ret =
        g_hal_state->ep_l3_entry_ht()->insert_with_key(l3_key,
                                                       entry, &entry->ht_ctxt);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add l2 key to handle mapping, err : {}", ret);
        hal::delay_delete_to_slab(HAL_SLAB_EP_L3_ENTRY, entry);
    }

    return ret;
}

//------------------------------------------------------------------------------
// delete an ep from l3 db
//------------------------------------------------------------------------------
static inline hal_ret_t
ep_del_from_l3_db (ep_l3_key_t *l3_key)
{
    ep_l3_entry_t               *l3_entry;

    HAL_TRACE_DEBUG("Removing EP from L3 hash table");
    // remove from hash table
    l3_entry = (ep_l3_entry_t *)g_hal_state->ep_l3_entry_ht()->remove(l3_key);

    if (l3_entry) {
        // free up
        hal::delay_delete_to_slab(HAL_SLAB_EP_L3_ENTRY, l3_entry);
    }

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// dump endpoint spec
//-----------------------------------------------------------------------------
static inline void
endpoint_dump (EndpointSpec& spec)
{
    std::string    ep_cfg;

    if (hal::utils::hal_trace_level() < hal::utils::trace_debug) {
        return;
    }
    google::protobuf::util::MessageToJsonString(spec, &ep_cfg);
    HAL_TRACE_DEBUG("Endpoint configuration:");
    HAL_TRACE_DEBUG("{}", ep_cfg.c_str());
}

//------------------------------------------------------------------------------
// validate an incoming endpoint create request
//------------------------------------------------------------------------------
static hal_ret_t
validate_endpoint_create (EndpointSpec& spec, EndpointResponse *rsp)
{
    mac_addr_t mac_addr;

    if (!spec.has_vrf_key_handle()) {
        HAL_TRACE_ERR("Endpoint spec has no vrf key");
        return HAL_RET_VRF_ID_INVALID;
    }
    if (spec.vrf_key_handle().vrf_id() == HAL_VRF_ID_INVALID) {
        HAL_TRACE_ERR("Invalid vrf id");
        return HAL_RET_VRF_ID_INVALID;
    }

    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("EP doesn't have key or handle");
        return HAL_RET_INVALID_ARG;
    }

    if ((spec.key_or_handle().key_or_handle_case() !=
             EndpointKeyHandle::kEndpointKey) ||
        (spec.key_or_handle().endpoint_key().endpoint_l2_l3_key_case() !=
             EndpointKey::kL2Key)) {
        HAL_TRACE_ERR("EP doesnt have L2 key");
        return HAL_RET_INVALID_ARG;
    }

    if (!spec.has_endpoint_attrs()) {
        HAL_TRACE_ERR("EP doesnt have attributes");
        return HAL_RET_INVALID_ARG;
    }

    if (!spec.key_or_handle().endpoint_key().l2_key().has_l2segment_key_handle() ||
       ((spec.key_or_handle().endpoint_key().l2_key().l2segment_key_handle().key_or_handle_case() == L2SegmentKeyHandle::kSegmentId) &&
        (spec.key_or_handle().endpoint_key().l2_key().l2segment_key_handle().segment_id() == HAL_L2SEGMENT_ID_INVALID)) ||
       ((spec.key_or_handle().endpoint_key().l2_key().l2segment_key_handle().key_or_handle_case() == L2SegmentKeyHandle::kL2SegmentHandle) &&
        (spec.key_or_handle().endpoint_key().l2_key().l2segment_key_handle().l2segment_handle() == HAL_HANDLE_INVALID))) {
        HAL_TRACE_ERR("L2seg key/handle not valid");
        return HAL_RET_HANDLE_INVALID;
    }

    if (!spec.key_or_handle().endpoint_key().l2_key().has_l2segment_key_handle() ||
       ((spec.endpoint_attrs().interface_key_handle().key_or_handle_case() == InterfaceKeyHandle::kInterfaceId) &&
        (spec.endpoint_attrs().interface_key_handle().interface_id() == HAL_IFINDEX_INVALID)) ||
       ((spec.endpoint_attrs().interface_key_handle().key_or_handle_case() == InterfaceKeyHandle::kIfHandle) &&
        (spec.endpoint_attrs().interface_key_handle().if_handle() == HAL_HANDLE_INVALID))) {
        HAL_TRACE_ERR("Interface handle not valid");
        return HAL_RET_HANDLE_INVALID;
    }

    MAC_UINT64_TO_ADDR(mac_addr, spec.key_or_handle().endpoint_key().l2_key().mac_address());
    if (IS_MCAST_MAC_ADDR(mac_addr)) {
        HAL_TRACE_ERR("EP is being created with mcast MACa ddr  {}",
                      macaddr2str(mac_addr));
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// PD Call to allocate PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
endpoint_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t               ret        = HAL_RET_OK;
    pd::pd_ep_create_args_t pd_ep_args = { 0 };
    dllist_ctxt_t           *lnode     = NULL;
    dhl_entry_t             *dhl_entry = NULL;
    ep_t                    *ep        = NULL;
    ep_create_app_ctxt_t    *app_ctxt  = NULL;

    HAL_ASSERT(cfg_ctxt != NULL);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (ep_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    ep = (ep_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("EP create add cb {}", ep_l2_key_to_str(ep));

    // PD Call to allocate PD resources and HW programming
    pd::pd_ep_create_args_init(&pd_ep_args);
    pd_ep_args.ep = ep;
    pd_ep_args.vrf = app_ctxt->vrf;
    pd_ep_args.l2seg = app_ctxt->l2seg;
    pd_ep_args.intf = app_ctxt->hal_if;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_EP_CREATE, (void *)&pd_ep_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create ep pd, err : {}", ret);
    }
    return ret;
}


//------------------------------------------------------------------------------
// 1. Update PI DBs as endpoint_create_add_cb() was a success
//      a. Create the flood list
//      b. Add to endpoint id hash table
//------------------------------------------------------------------------------
hal_ret_t
endpoint_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t            ret          = HAL_RET_OK;
    dllist_ctxt_t        *lnode       = NULL;
    dhl_entry_t          *dhl_entry   = NULL;
    ep_t                 *ep          = NULL;
    vrf_t                *vrf         = NULL;
    hal_handle_t         hal_handle   = 0;
    dllist_ctxt_t        *ip_lnode    = NULL;
    ep_ip_entry_t        *pi_ip_entry = NULL;
    ep_create_app_ctxt_t *app_ctxt    = NULL;
    ep_l3_key_t          l3_key       = {0};

    HAL_ASSERT(cfg_ctxt != NULL);
    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (ep_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    ep = (ep_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;
    vrf = app_ctxt->vrf;

    HAL_TRACE_DEBUG("EP create commit cb {}", ep_l2_key_to_str(ep));

    // add EP to L2 DB
    ret = ep_add_to_l2_db (ep, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add EP {} to L2 DB, err : {}",
                      ep_l2_key_to_str(ep), ret);
        goto end;
    }

    // add EP to L3 DB
    dllist_for_each(ip_lnode, &ep->ip_list_head) {
        pi_ip_entry = dllist_entry(ip_lnode, ep_ip_entry_t, ep_ip_lentry);
        l3_key.vrf_id = vrf->vrf_id;
        l3_key.ip_addr = pi_ip_entry->ip_addr;
        ret = ep_add_to_l3_db(&l3_key, pi_ip_entry, hal_handle);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to add EP {} to L3 DB, err : {}",
                          ep_l2_key_to_str(ep), ret);
            goto end;
        }
        HAL_TRACE_DEBUG("Added EP ({}, {}) to L3 DB",
                        l3_key.vrf_id, ipaddr2str(&l3_key.ip_addr));
    }

    // TODO: Increment the ref counts of dependent objects
    //  - Have to increment ref count for vrf

end:

    if (ret != HAL_RET_OK) {
    }
    return ret;
}

//------------------------------------------------------------------------------
// EP Cleanup.
//  - PI Cleanup
//  - Removes the existence of this EP in HAL
//------------------------------------------------------------------------------
hal_ret_t
endpoint_cleanup(ep_t *ep)
{
    dllist_ctxt_t    *curr, *next;
    ep_ip_entry_t    *pi_ip_entry = NULL;
    hal_ret_t        ret          = HAL_RET_OK;
    vrf_t            *vrf         = NULL;
    ep_l3_key_t      l3_key       = { 0 };

    vrf = vrf_lookup_by_handle(ep->vrf_handle);

    // remove EP from L2 DB
    ret = ep_del_from_l2_db(ep);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete EP from L3 DB, err : {}", ret);
        goto end;
    }
    HAL_TRACE_DEBUG("Deleted EP {} from L2 DB", ep_l2_key_to_str(ep));

    // remove EP from L3 DB
    dllist_for_each_safe(curr, next, &ep->ip_list_head) {
        pi_ip_entry = dllist_entry(curr, ep_ip_entry_t, ep_ip_lentry);
        l3_key.vrf_id = vrf->vrf_id;
        l3_key.ip_addr = pi_ip_entry->ip_addr;
        ret = ep_del_from_l3_db(&l3_key);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to del EP:{} from L3 DB",
                          ep_l2_key_to_str(ep));
            goto end;
        }
        HAL_TRACE_DEBUG("Deleted EP ({}, {}) from L3 DB",
                        l3_key.vrf_id, ipaddr2str(&l3_key.ip_addr));
    }

    // free EP
    ret = ep_free(ep);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to free EP");
        goto end;
    }

end:

    return ret;
}

//------------------------------------------------------------------------------
// endpoint_create_add_cb was a failure
// 1. call delete to PD
//      a. Deprogram HW
//      b. Clean up resources
//      c. Free PD object
// 2. Remove object from hal_handle id based hash table in infra
// 3. Free PI vrf
//------------------------------------------------------------------------------
hal_ret_t
endpoint_create_abort_cleanup (ep_t *ep, hal_handle_t hal_handle)
{
    pd::pd_ep_delete_args_t         pd_ep_args = { 0 };
    hal_ret_t                       ret = HAL_RET_OK;           

    HAL_TRACE_DEBUG("EP create abort cb");
    if (ep->pd) {
        pd::pd_ep_delete_args_init(&pd_ep_args);
        pd_ep_args.ep = ep;
        ret = pd::hal_pd_call(pd::PD_FUNC_ID_EP_DELETE, (void *)&pd_ep_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to delete ep pd, err : {}", ret);
        }
    }

    // 2. remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    // 3. free PI EP
    // endpoint_cleanup(ep);

    return ret;
}

hal_ret_t
endpoint_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                       ret = HAL_RET_OK;
    dllist_ctxt_t                   *lnode = NULL;
    dhl_entry_t                     *dhl_entry = NULL;
    ep_t                            *ep = NULL;
    hal_handle_t                    hal_handle = 0;

    HAL_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    ep = (ep_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    ret = endpoint_create_abort_cleanup(ep, hal_handle);
    
    return ret;
}

//-----------------------------------------------------------------------------
// Dummy create cleanup callback
//-----------------------------------------------------------------------------
hal_ret_t
endpoint_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// Converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
ep_prepare_rsp (EndpointResponse *rsp, hal_ret_t ret,
                hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_endpoint_status()->set_endpoint_handle(hal_handle);
    }

    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

static if_id_t
allocate_pin_ifid_for_endpoint (ep_t *ep)
{
    return ep->l2_key.mac_addr[3];
}

//------------------------------------------------------------------------------
// Host Pinning Mode:
// If this mode is enabled, then this routine will pin the endpoint to an
// uplink port/port-channel.
//-- Applies only to Local endpoints. NOP for Remote EPs.
//------------------------------------------------------------------------------
static hal_ret_t
pin_endpoint (ep_t *ep)
{
    if_t        *hal_if = NULL;
    if_id_t     pin_ifid = 0;

    ep->pinned_if_handle = HAL_HANDLE_INVALID;
    if (is_forwarding_mode_host_pinned() == FALSE) {
        HAL_TRACE_DEBUG("Forwarding mode is not host-pinned, no-op");
        return HAL_RET_OK;
    }

    if (ep->ep_flags & EP_FLAGS_REMOTE) {
        HAL_TRACE_DEBUG("no-op for remote EP.");
        return HAL_RET_OK;
    }

    // TEMP: Initial Commit only.
    // To be replaced by a proper pin selection algorithm.
    pin_ifid = allocate_pin_ifid_for_endpoint(ep);
    hal_if = find_if_by_id(pin_ifid);
    if (hal_if == NULL) {
        HAL_TRACE_ERR("Interface {} not found for Id", pin_ifid);
        return HAL_RET_IF_NOT_FOUND;
    }
    ep->pinned_if_handle = hal_if->hal_handle;
    HAL_TRACE_DEBUG("Pinning EP to IF Id:{}", pin_ifid);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Initialize EP from Spec
//------------------------------------------------------------------------------
hal_ret_t
ep_init_from_spec (ep_t *ep, const EndpointSpec& spec, bool create)
{
    int                             i, num_ips = 0, num_sgs = 0;
    vrf_id_t                        tid;
    L2SegmentKeyHandle              l2seg_key_handle;
    InterfaceKeyHandle              if_key_handle;
    vrf_t                           *vrf = NULL;
    l2seg_t                         *l2seg = NULL;
    if_t                            *hal_if = NULL;
    ep_l3_entry_t                   **l3_entry = NULL;
    ep_ip_entry_t                   **ip_entry = NULL;
    nwsec_group_t                   *nwsec_group = NULL;
    hal_ret_t                       ret = HAL_RET_OK;

    // Fetch the VRF information
    tid = spec.vrf_key_handle().vrf_id();
    vrf = vrf_lookup_by_id(tid);

    // Fetch the L2 segment information
    l2seg_key_handle =
        spec.key_or_handle().endpoint_key().l2_key().l2segment_key_handle();
    l2seg = l2seg_lookup_key_or_handle(l2seg_key_handle);

    // Fetch the interface information
    if_key_handle = spec.endpoint_attrs().interface_key_handle();
    hal_if = if_lookup_key_or_handle(if_key_handle);

    // Initialize the EP
    ep->l2_key.l2_segid = l2seg->seg_id;
    MAC_UINT64_TO_ADDR(ep->l2_key.mac_addr, spec.key_or_handle().endpoint_key().l2_key().mac_address());
    HAL_TRACE_DEBUG("L2seg id {}, Mac {} if {}", l2seg->seg_id,
                    ether_ntoa((struct ether_addr*)(ep->l2_key.mac_addr)),
                    hal_if->if_id);
    ep->l2seg_handle = l2seg->hal_handle;
    ep->if_handle = hal_if->hal_handle;
    ep->vrf_handle = vrf->hal_handle;
    ep->useg_vlan = spec.endpoint_attrs().useg_vlan();
    ep->ep_flags = EP_FLAGS_LEARN_SRC_CFG;
    if (hal_if->if_type == intf::IF_TYPE_ENIC) {
        ep->ep_flags |= EP_FLAGS_LOCAL;
        HAL_TRACE_DEBUG("Setting local flag in EP {}", ep->ep_flags);
    } else {
        ep->ep_flags |= EP_FLAGS_REMOTE;
        HAL_TRACE_DEBUG("Setting remote flag in EP {}", ep->ep_flags);
    }

    // Process Host pinning mode, if enabled.
    ret = pin_endpoint(ep);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // allocate memory for each IP entry in the EP
    num_ips = spec.endpoint_attrs().ip_address_size();
    HAL_TRACE_DEBUG("Num IPs in EP {}", num_ips);
    if (num_ips) {
        l3_entry = (ep_l3_entry_t **)HAL_CALLOC(HAL_MEM_ALLOC_EP,
                                                num_ips * sizeof(ep_l3_entry_t *));
        ip_entry = (ep_ip_entry_t **)HAL_CALLOC(HAL_MEM_ALLOC_EP,
                                                num_ips * sizeof(ep_ip_entry_t *));

        for (i = 0; i < num_ips; i++) {
            l3_entry[i] = (ep_l3_entry_t *)g_hal_state->ep_l3_entry_slab()->alloc();
            if (l3_entry[i] == NULL) {
                ret = HAL_RET_OOM;
                goto end;
            }

            ip_entry[i] = (ep_ip_entry_t *)g_hal_state->ep_ip_entry_slab()->alloc();
            if (ip_entry[i] == NULL) {
                ret = HAL_RET_OOM;
                goto end;
            }
        }
    }

    // handle IP address information, if any
    // TODO: check if any IPs are already known to us already !!
    sdk::lib::dllist_reset(&ep->ip_list_head);
    for (i = 0; i < num_ips; i++) {
        // add the IP to EP
        sdk::lib::dllist_reset(&ip_entry[i]->ep_ip_lentry);
        ip_addr_spec_to_ip_addr(&ip_entry[i]->ip_addr, spec.endpoint_attrs().ip_address(i));
        ip_entry[i]->ip_flags = EP_FLAGS_LEARN_SRC_CFG;
        sdk::lib::dllist_add(&ep->ip_list_head, &ip_entry[i]->ep_ip_lentry);
    }

    if (create) {
        // Alloc HAL Handle
        ep->hal_handle = hal_handle_alloc(HAL_OBJ_ID_ENDPOINT);
        if (ep->hal_handle == HAL_HANDLE_INVALID) {
            HAL_TRACE_ERR("Failed to alloc handle for EP");
            ep_cleanup(ep);
            ret = HAL_RET_HANDLE_INVALID;
            goto end;
        }
    }

    num_sgs = spec.endpoint_attrs().sg_key_handle_size();
    if (num_sgs) {
        //To Do:Handle cases where the num_sgs greater that MAX_SG_PER_ARRAY
        for (i = 0; i < num_sgs; i++) {
            /* Lookup the SG by handle and then get the SG-id */
            nwsec_group = nwsec_group_lookup_key_or_handle(spec.endpoint_attrs().sg_key_handle(i));
            if (!nwsec_group) {
                ret = HAL_RET_NWSEC_ID_INVALID;
                goto end;
            }
            ep->sgs.arr_sg_id[i] = nwsec_group->sg_id;
            ep->sgs.sg_id_cnt++;
            ep->sgs.next_sg_p = NULL;
            ret = add_ep_to_security_group(nwsec_group->sg_id, ep->hal_handle);
            HAL_ASSERT_RETURN(ret == HAL_RET_OK, ret);
        }
    }

end:

    return ret;
}

//------------------------------------------------------------------------------
// process a endpoint create request
// TODO: check if EP or any of its IPs exists already
//------------------------------------------------------------------------------
hal_ret_t
endpoint_create (EndpointSpec& spec, EndpointResponse *rsp)
{
    hal_ret_t                       ret = HAL_RET_OK;
    vrf_id_t                        tid;
    InterfaceKeyHandle              if_key_handle;
    ep_t                            *ep = NULL;
    vrf_t                           *vrf = NULL;
    l2seg_t                         *l2seg = NULL;
    if_t                            *hal_if = NULL;
    ep_create_app_ctxt_t            app_ctxt;
    dhl_entry_t                     dhl_entry = { 0 };
    cfg_op_ctxt_t                   cfg_ctxt = { 0 };
    L2SegmentKeyHandle              l2seg_key_handle;

    // dump incoming request
    endpoint_dump(spec);

    ret = validate_endpoint_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // fetch the vrf information
    tid = spec.vrf_key_handle().vrf_id();
    vrf = vrf_lookup_by_id(tid);
    if (vrf == NULL) {
        ret = HAL_RET_VRF_NOT_FOUND;
        goto end;
    }

    // fetch the L2 segment information
    l2seg_key_handle =
        spec.key_or_handle().endpoint_key().l2_key().l2segment_key_handle();
    l2seg = l2seg_lookup_key_or_handle(l2seg_key_handle);
    if (l2seg == NULL) {
        HAL_TRACE_ERR("Failed to find l2seg id {}, handle {}",
                      l2seg_key_handle.segment_id(),
                      l2seg_key_handle.l2segment_handle());
        ret = HAL_RET_L2SEG_NOT_FOUND;
        goto end;
    }

    // fetch the interface information
    if_key_handle = spec.endpoint_attrs().interface_key_handle();
    hal_if = if_lookup_key_or_handle(if_key_handle);
    if (hal_if == NULL) {
        ret = HAL_RET_IF_NOT_FOUND;
        goto end;
    }

    // instantiate EP
    ep = ep_alloc_init();
    if (ep == NULL) {
        HAL_TRACE_ERR("Failed to allocate handle/memory ret: {}", ret);
        ret = HAL_RET_OOM;
        goto end;
    }

    // initialize the EP record
    ret = ep_init_from_spec(ep, spec, true);
    if (ret != HAL_RET_OK) {
        rsp->set_api_status(hal_prepare_rsp(ret));
        goto end;
    }

    // form ctxt and call infra add
    app_ctxt.vrf = vrf;
    app_ctxt.l2seg = l2seg;
    app_ctxt.hal_if = hal_if;

    dhl_entry.handle = ep->hal_handle;
    dhl_entry.obj = ep;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(ep->hal_handle, &cfg_ctxt,
                             endpoint_create_add_cb,
                             endpoint_create_commit_cb,
                             endpoint_create_abort_cb,
                             endpoint_create_cleanup_cb);

end:

    if (ret != HAL_RET_OK && ret != HAL_RET_ENTRY_EXISTS) {
        if (ep) {
            ep_cleanup(ep);
            ep = NULL;
        }
        HAL_API_STATS_INC(HAL_API_ENDPOINT_CREATE_FAIL);
    } else {
        HAL_API_STATS_INC(HAL_API_ENDPOINT_CREATE_SUCCESS);
    }

    // TODO: free up ip_entry and l3_entry

    ep_prepare_rsp(rsp, ret, ep ? ep->hal_handle : HAL_HANDLE_INVALID);
    return ret;
}

//------------------------------------------------------------------------------
// validate an incoming endpoint update request
//------------------------------------------------------------------------------
static hal_ret_t
validate_endpoint_update_spec (EndpointUpdateRequest& req, EndpointResponse *rsp)
{
    hal_ret_t       ret = HAL_RET_OK;

    // check if vrf id is valid
    if (!req.has_vrf_key_handle() ||
        req.vrf_key_handle().vrf_id() == HAL_VRF_ID_INVALID) {
        HAL_TRACE_ERR("Invalid vrf id");
        ret = HAL_RET_VRF_ID_INVALID;
        goto end;
    }

    // check if ep's key or handle is passed
    if (!req.has_key_or_handle()) {
        ret = HAL_RET_INVALID_ARG;
        HAL_TRACE_ERR("EP spec has no key or handle");
        goto end;
    }

end:
    return ret;
}

hal_ret_t
ep_handle_if_change (ep_t *ep, hal_handle_t new_if_handle)
{
    dllist_ctxt_t                   *curr, *next;
    hal_handle_id_list_entry_t      *entry = NULL;
    session_t                       *session = NULL;
    ep_fte_event_t                  fte_event;
    hal_ret_t                       ret = HAL_RET_OK;

    memset(&fte_event, 0, sizeof(ep_fte_event_t));

    fte_event.type          = EP_FTE_EVENT_IF_CHANGE;
    fte_event.old_if_handle = ep->if_handle;
    fte_event.new_if_handle = new_if_handle;

    // Walk though session list and call FTE provided API
    dllist_for_each_safe(curr, next, &ep->session_list_head) {
        entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        session = find_session_by_handle(entry->handle_id);
        HAL_ABORT(session != NULL);
        // TODO: vmotion: Call FTE API to handle
        // ret = fte_handle_if_change(session, ep, &fte_event);
    }

    return ret;
}

//------------------------------------------------------------------------------
// This is the first call back infra does for update.
// 1. PD Call to update PD
// 2. Update Other objects to update new l2seg properties
//------------------------------------------------------------------------------
hal_ret_t
endpoint_update_upd_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                       ret = HAL_RET_OK;
    pd::pd_ep_update_args_t         pd_ep_args = { 0 };
    dllist_ctxt_t                   *lnode = NULL;
    dhl_entry_t                     *dhl_entry = NULL;
    ep_t                            *ep = NULL/*, *ep_clone = NULL*/;
    ep_update_app_ctxt_t            *app_ctxt = NULL;

    HAL_ASSERT(cfg_ctxt != NULL);
    lnode     = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt  = (ep_update_app_ctxt_t *)cfg_ctxt->app_ctxt;
    ep = (ep_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("EP update cb");
    pd::pd_ep_update_args_init(&pd_ep_args);
    pd_ep_args.ep            = ep;
    pd_ep_args.iplist_change = app_ctxt->iplist_change;
    pd_ep_args.add_iplist    = app_ctxt->add_iplist;
    pd_ep_args.del_iplist    = app_ctxt->del_iplist;
    pd_ep_args.app_ctxt      = app_ctxt;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_EP_UPDATE, (void *)&pd_ep_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update ep pd, err : {}", ret);
    }

    // make FTE calls to process sessions
    ret = ep_handle_if_change(ep, app_ctxt->new_if_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed EP update, err : {}", ret);
        goto end;
    }

end:

    return ret;
}

//------------------------------------------------------------------------------
// Make a clone
//-- Both PI and PD objects cloned.
//------------------------------------------------------------------------------
hal_ret_t
ep_make_clone (ep_t *ep, ep_t **ep_clone)
{
    pd::pd_ep_make_clone_args_t args;

    *ep_clone = ep_alloc_init();

    memcpy(*ep_clone, ep, sizeof(ep_t));

    args.ep = ep;
    args.clone = *ep_clone;
    pd::hal_pd_call(pd::PD_FUNC_ID_EP_MAKE_CLONE, (void *)&args);

    // after clone always reset lists
    dllist_reset(&(*ep_clone)->session_list_head);

    return HAL_RET_OK;
}

hal_ret_t
ep_copy_ip_list (ep_t *dst_ep, ep_t *src_ep)
{
    dllist_ctxt_t   *curr, *next;
    ep_ip_entry_t   *pi_ip_entry = NULL;

    ep_print_ips(src_ep);
    sdk::lib::dllist_reset(&dst_ep->ip_list_head);
    dllist_for_each_safe(curr, next, &(src_ep->ip_list_head)) {
        pi_ip_entry = dllist_entry(curr, ep_ip_entry_t, ep_ip_lentry);
        HAL_TRACE_DEBUG("copying to clone IP {}",
                        ipaddr2str(&(pi_ip_entry->ip_addr)));
        sdk::lib::dllist_del(&pi_ip_entry->ep_ip_lentry);
        sdk::lib::dllist_add(&dst_ep->ip_list_head, &pi_ip_entry->ep_ip_lentry);
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// After all hw programming is done
//  1. Free original PI & PD endpoint
// Note: Infra make clone as original by replacing original pointer by clone.
//------------------------------------------------------------------------------
hal_ret_t
endpoint_update_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_ep_mem_free_args_t            pd_ep_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    ep_t                        *ep = NULL, *ep_clone = NULL;
    ep_update_app_ctxt_t        *app_ctxt = NULL;

    HAL_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (ep_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    ep = (ep_t *)dhl_entry->obj;
    ep_clone = (ep_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("EP update commit cb");

    // TODO: move like how we did in if for uplink mbrs
    ep_copy_ip_list(ep_clone, ep);
    endpoint_update_pi_with_iplist(ep_clone, app_ctxt->add_iplist,
                                   app_ctxt->del_iplist);

    // Free PD
    pd::pd_ep_mem_free_args_init(&pd_ep_args);
    pd_ep_args.ep = ep;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_EP_MEM_FREE, (void *)&pd_ep_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete ep pd, err : {}", ret);
    }

    // free PI
    ep_free(ep);

    ep_print_ips(ep_clone);
    return ret;
}

//------------------------------------------------------------------------------
// Update didnt go through.
//  1. Free the clones
//------------------------------------------------------------------------------
hal_ret_t
endpoint_update_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_ep_mem_free_args_t   pd_ep_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    ep_t                        *ep = NULL;
    ep_update_app_ctxt_t        *app_ctxt = NULL;

    HAL_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (ep_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    // assign clone as we are trying to free only the clone
    ep = (ep_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("EP update abort cb");

    // free PD
    pd::pd_ep_mem_free_args_init(&pd_ep_args);
    pd_ep_args.ep = ep;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_EP_MEM_FREE, (void *)&pd_ep_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete ep pd, err : {}", ret);
    }

    // free up add & del ip lists
    endpoint_cleanup_ip_list(&app_ctxt->add_iplist);
    endpoint_cleanup_ip_list(&app_ctxt->del_iplist);

    // free PI
    ep_free(ep);
    return ret;
}

hal_ret_t
endpoint_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// helper to get endpoint from key or handle
//------------------------------------------------------------------------------
hal_ret_t
find_ep (EndpointKeyHandle kh, ep_t **ep, ::types::ApiStatus *api_status)
{
    ep_l3_key_t            l3_key    = { 0 };
    l2seg_t                *l2seg    = NULL;
    mac_addr_t             mac_addr;
    vrf_t                  *vrf = NULL;

    if (kh.key_or_handle_case() == EndpointKeyHandle::kEndpointKey) {
        auto ep_key = kh.endpoint_key();
        if (ep_key.has_l2_key()) {
            auto ep_l2_key = ep_key.l2_key();
            MAC_UINT64_TO_ADDR(mac_addr, ep_l2_key.mac_address());
            HAL_TRACE_DEBUG("l2 key : seg {}, handle {}, mac {}",
                            ep_l2_key.l2segment_key_handle().segment_id(),
                            ep_l2_key.l2segment_key_handle().l2segment_handle(),
                            macaddr2str(mac_addr));
            l2seg = l2seg_lookup_key_or_handle(ep_l2_key.l2segment_key_handle());
            if (l2seg == NULL) {
                HAL_TRACE_ERR("Failed to find l2seg id {}, handle {}",
                              ep_l2_key.l2segment_key_handle().segment_id(),
                              ep_l2_key.l2segment_key_handle().l2segment_handle());
                return HAL_RET_L2SEG_NOT_FOUND;
            }
            *ep = find_ep_by_l2_key(l2seg->seg_id, mac_addr);
        } else if (ep_key.has_l3_key()) {
            auto ep_l3_key = ep_key.l3_key();
            vrf = vrf_lookup_key_or_handle(ep_l3_key.vrf_key_handle());
            if (vrf == NULL) {
                HAL_TRACE_ERR("Failed to find vrf {}",
                              vrf_spec_keyhandle_to_str(ep_l3_key.vrf_key_handle()));
                return HAL_RET_VRF_NOT_FOUND;

            }
            l3_key.vrf_id = vrf->vrf_id;
            ip_addr_spec_to_ip_addr(&l3_key.ip_addr,
                    ep_l3_key.ip_address());
            *ep = find_ep_by_l3_key(&l3_key);
        } else {
            if (api_status) {
                *api_status = types::API_STATUS_INVALID_ARG;
            }
            return HAL_RET_INVALID_ARG;
        }
    } else if (kh.key_or_handle_case() ==
            EndpointKeyHandle::kEndpointHandle) {
        *ep = find_ep_by_handle(kh.endpoint_handle());
    } else {
        if (api_status) {
            *api_status = types::API_STATUS_INVALID_ARG;
        }
        return HAL_RET_INVALID_ARG;
    }

    if (!(*ep)) {
        if (api_status) {
            *api_status = types::API_STATUS_INVALID_ARG;
        }
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Validating ep update changes
//------------------------------------------------------------------------------
#define EP_VMOTION_STATE_CHECK(old_state, new_state) \
    (ep->vmotion_state == old_state && app_ctxt->new_vmotion_state == new_state)
hal_ret_t
endpoint_validate_update_change (ep_t *ep, ep_update_app_ctxt_t *app_ctxt)
{
    hal_ret_t               ret     = HAL_RET_OK;
    if_t                    *hal_if = NULL;

    HAL_ASSERT_RETURN(ep != NULL, HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN(app_ctxt != NULL, HAL_RET_INVALID_ARG);

    if (app_ctxt->vmotion_state_change) {
        if (EP_VMOTION_STATE_CHECK(VMOTION_STATE_NONE, VMOTION_STATE_START) ||
            EP_VMOTION_STATE_CHECK(VMOTION_STATE_START, VMOTION_STATE_SETUP) ||
            EP_VMOTION_STATE_CHECK(VMOTION_STATE_SETUP, VMOTION_STATE_ACTIVATE) ||
            EP_VMOTION_STATE_CHECK(VMOTION_STATE_ACTIVATE, VMOTION_END)) {
            HAL_TRACE_DEBUG("Vmotion state change : {} => {}",
                            ep->vmotion_state, app_ctxt->new_vmotion_state);
        } else {
            HAL_TRACE_ERR("Invalid vmotion state change: {} => {}",
                          ep->vmotion_state,
                          app_ctxt->new_vmotion_state);
            ret = HAL_RET_INVALID_ARG;
            goto end;
        }

        // Vmotion None => Start
        if (EP_VMOTION_STATE_CHECK(VMOTION_STATE_NONE, VMOTION_STATE_START)) {
            // check if "if" changed to tunnel
            if (!app_ctxt->if_change) {
                HAL_TRACE_ERR("Invalid update, if has to change");
                ret = HAL_RET_INVALID_ARG;
                goto end;
            }

            hal_if = find_if_by_handle(app_ctxt->new_if_handle);
            if (!hal_if) {
                HAL_TRACE_ERR("Invalid update, new if {} not present",
                              app_ctxt->new_if_handle);
                ret = HAL_RET_INVALID_ARG;
                goto end;
            }

            if (hal_if->if_type != intf::IF_TYPE_TUNNEL) {
                HAL_TRACE_ERR("Invalid update, if has to be tunnel "
                              "if if_hdl : {}, if_type : {}",
                              app_ctxt->new_if_handle, hal_if->if_type);
                ret = HAL_RET_INVALID_ARG;
                goto end;
            }
        }

        // Vmotion Start => Setup, Setup => Activate, Activate => End
        if (EP_VMOTION_STATE_CHECK(VMOTION_STATE_START,
                                   VMOTION_STATE_SETUP) ||
            EP_VMOTION_STATE_CHECK(VMOTION_STATE_SETUP,
                                   VMOTION_STATE_ACTIVATE) ||
            EP_VMOTION_STATE_CHECK(VMOTION_STATE_ACTIVATE,
                                   VMOTION_END)) {
            // For now, assumption is nothing else changes
            if (app_ctxt->iplist_change || app_ctxt->if_change) {
                HAL_TRACE_ERR("Invalid update. vmotion update "
                              "requires nothing else should change"
                              "iplist_chg : {}, if_change : {}",
                              app_ctxt->iplist_change, app_ctxt->if_change);
                ret = HAL_RET_INVALID_ARG;
                goto end;
            }
        }
    }

end:

    return ret;
}

hal_ret_t
endpoint_check_update (EndpointUpdateRequest& req, ep_t *ep,
                       ep_update_app_ctxt_t *app_ctxt)
{
    hal_ret_t               ret = HAL_RET_OK;

    HAL_ASSERT_RETURN(ep != NULL, HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN(app_ctxt != NULL, HAL_RET_INVALID_ARG);

    app_ctxt->if_change = false;
    app_ctxt->vmotion_state_change = false;
    app_ctxt->new_if_handle = HAL_HANDLE_INVALID;
    if (req.has_endpoint_attrs()) {
        auto hal_if =
            if_lookup_key_or_handle((req.endpoint_attrs().interface_key_handle()));
        if (!hal_if) {
             ret = HAL_RET_INVALID_ARG;
             goto end;
        }

        if (ep->if_handle != hal_if->hal_handle) {
            app_ctxt->if_change = true;
            app_ctxt->new_if_handle = hal_if->hal_handle;
        }

        // Check if vmotion state changed
        if (ep->vmotion_state != req.endpoint_attrs().vmotion_state()) {
            app_ctxt->vmotion_state_change = true;
            app_ctxt->new_vmotion_state = req.endpoint_attrs().vmotion_state();
        }
    }

    // validate changes
    ret = endpoint_validate_update_change(ep, app_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("EP update change validation failed");
        goto end;
    }

end:

    return ret;
}

hal_ret_t
endpoint_l2seg_update(EndpointUpdateRequest& req, ep_t *ep,
                      bool *l2seg_change, hal_handle_t *new_l2seg_hdl)
{
    *l2seg_change = false;
#if 0
    if (ep->l2seg_handle != req.l2_segment_handle()) {
        *l2seg_change = true;
        *new_l2seg_hdl = req.l2_segment_handle();
    }
#endif

    return HAL_RET_OK;
}

bool
ip_in_ep(ip_addr_t *ip, ep_t *ep, ep_ip_entry_t **ip_entry)
{
    dllist_ctxt_t       *lnode = NULL;
    ep_ip_entry_t       *pi_ip_entry = NULL;

    lnode = ep->ip_list_head.next;
    dllist_for_each(lnode, &(ep->ip_list_head)) {
        pi_ip_entry = (ep_ip_entry_t *)((char *)lnode -
                offsetof(ep_ip_entry_t, ep_ip_lentry));
        if (!memcmp(ip, &pi_ip_entry->ip_addr, sizeof(ip_addr_t))) {
            if (ip_entry) {
                *ip_entry = pi_ip_entry;
            }
            return true;
        }
    }

    return false;
}

#define IP_UPDATE_OP_ADD     0
#define IP_UPDATE_OP_DELETE  1

static hal_ret_t
endpoint_update_ip_op (ep_t *ep, ip_addr_t *ip_addr,
                       uint64_t learn_src_flag, uint32_t op)
{
    dllist_ctxt_t           *add_iplist = NULL;
    dllist_ctxt_t           *del_iplist = NULL;
    cfg_op_ctxt_t           cfg_ctxt = { 0 };
    dhl_entry_t             dhl_entry = { 0 };
    ep_update_app_ctxt_t    app_ctxt = { 0 };
    hal_ret_t               ret = HAL_RET_OK;
    ep_ip_entry_t           *ep_ipe = NULL;
    dllist_ctxt_t           *lnode = NULL;
    ep_ip_entry_t           *pi_ip_entry = NULL;


    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    add_iplist = (dllist_ctxt_t *)HAL_CALLOC(
            HAL_MEM_ALLOC_DLLIST, sizeof(dllist_ctxt_t));
    HAL_ABORT(add_iplist != NULL);
    del_iplist = (dllist_ctxt_t *)HAL_CALLOC(
            HAL_MEM_ALLOC_DLLIST, sizeof(dllist_ctxt_t));
    HAL_ABORT(del_iplist != NULL);
    sdk::lib::dllist_reset(add_iplist);
    sdk::lib::dllist_reset(del_iplist);
    ep_ipe = (ep_ip_entry_t *)g_hal_state->ep_ip_entry_slab()->alloc();
    HAL_ABORT(ep_ipe != NULL);
    memcpy(&ep_ipe->ip_addr, ip_addr, sizeof(ip_addr_t));
    /*
     * TODO: We have to check the source flags correctly.
     * For instance, we can't just update the learn src flag blindly
     * if we learn't if from config.
     */
    ep_ipe->ip_flags = learn_src_flag;
    sdk::lib::dllist_reset(&ep_ipe->ep_ip_lentry);
    if (op == IP_UPDATE_OP_ADD) {
        sdk::lib::dllist_add(add_iplist, &ep_ipe->ep_ip_lentry);
        ep_ipe->pd = NULL;
    } else if (op == IP_UPDATE_OP_DELETE) {
        sdk::lib::dllist_add(del_iplist, &ep_ipe->ep_ip_lentry);
        dllist_for_each(lnode, &(ep->ip_list_head)) {
            pi_ip_entry = dllist_entry(lnode, ep_ip_entry_t, ep_ip_lentry);
            if (!memcmp(ip_addr, &pi_ip_entry->ip_addr, sizeof(ip_addr_t))) {
                if (!(pi_ip_entry->ip_flags & learn_src_flag)) {
                    /* If learn source is different, don't delete it */
                    goto out;
                }
                ep_ipe->pd = pi_ip_entry->pd;
                break;
            }
        }
        /* This API expects IP to be present */
        if (ep_ipe->pd == NULL) {
            HAL_ABORT(0);
        }
    } else {
        HAL_ABORT(0);
    }

    app_ctxt.iplist_change = true;
    app_ctxt.add_iplist = add_iplist;
    app_ctxt.del_iplist = del_iplist;

    ep_make_clone(ep, (ep_t **)&dhl_entry.cloned_obj);
    // form ctxt and call infra update object
    dhl_entry.handle = ep->hal_handle;
    dhl_entry.obj = ep;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(ep->hal_handle, &cfg_ctxt,
                             endpoint_update_upd_cb,
                             endpoint_update_commit_cb,
                             endpoint_update_abort_cb,
                             endpoint_update_cleanup_cb);

 out:

    HAL_FREE(HAL_MEM_ALLOC_DLLIST, add_iplist);
    HAL_FREE(HAL_MEM_ALLOC_DLLIST, del_iplist);

    hal::hal_cfg_db_close();
    return ret;
}

hal_ret_t
endpoint_update_ip_add (ep_t *ep, ip_addr_t *ip_addr,
                        uint64_t learn_src_flag)
{
    hal_ret_t               ret = HAL_RET_OK;

    // check if this IP is already part of this
    if (ip_in_ep(ip_addr, ep, NULL)) {
        return ret;
    }

    return endpoint_update_ip_op(ep, ip_addr, learn_src_flag, IP_UPDATE_OP_ADD);
}

hal_ret_t
endpoint_update_ip_delete (ep_t *ep, ip_addr_t *ip_addr,
                           uint64_t learn_src_flag)
{
    hal_ret_t               ret = HAL_RET_OK;

    // proceed only if IP is part of this entry
    if (!ip_in_ep(ip_addr, ep, NULL)) {
        return ret;
    }

    return endpoint_update_ip_op(ep, ip_addr,
            learn_src_flag, IP_UPDATE_OP_DELETE);
}

hal_ret_t
endpoint_ip_list_update (EndpointUpdateRequest& req, ep_t *ep,
                         bool *iplist_change,
                         dllist_ctxt_t **add_iplist,
                         dllist_ctxt_t **del_iplist)
{
    hal_ret_t       ret = HAL_RET_OK;
    uint16_t        num_ips = 0, i = 0;
    ip_addr_t       ip_addr;
    ep_ip_entry_t   *ep_ipe = NULL;
    dllist_ctxt_t   *lnode = NULL;
    ep_ip_entry_t   *pi_ip_entry = NULL;
    bool            ip_exists = false;

    *iplist_change = false;

    *add_iplist = (dllist_ctxt_t *)HAL_CALLOC(HAL_MEM_ALLOC_DLLIST, sizeof(dllist_ctxt_t));
    HAL_ABORT(*add_iplist != NULL);
    *del_iplist = (dllist_ctxt_t *)HAL_CALLOC(HAL_MEM_ALLOC_DLLIST, sizeof(dllist_ctxt_t));
    HAL_ABORT(*del_iplist != NULL);

    sdk::lib::dllist_reset(*add_iplist);
    sdk::lib::dllist_reset(*del_iplist);

    num_ips = req.endpoint_attrs().ip_address_size();
    ep_print_ips(ep);
    for (i = 0; i < num_ips; i++) {
        ip_addr_spec_to_ip_addr(&ip_addr, req.endpoint_attrs().ip_address(i));
        if (ip_in_ep(&ip_addr, ep, NULL)) {
            continue;
        } else {
            // Create ep_ip_entry and add it to add list
            ep_ipe = (ep_ip_entry_t *)g_hal_state->ep_ip_entry_slab()->alloc();
            HAL_ABORT(ep_ipe != NULL);
            memcpy(&ep_ipe->ip_addr, &ip_addr, sizeof(ip_addr_t));
            ep_ipe->ip_flags = EP_FLAGS_LEARN_SRC_CFG;
            ep_ipe->pd = NULL;
            sdk::lib::dllist_reset(&ep_ipe->ep_ip_lentry);
            sdk::lib::dllist_add(*add_iplist, &ep_ipe->ep_ip_lentry);
            *iplist_change = true;
        }
    }

    ep_print_ips(ep);

    HAL_TRACE_DEBUG("Checking for deleted IPs");
    // lnode = ep->ip_list_head.next;
    dllist_for_each(lnode, &(ep->ip_list_head)) {
        pi_ip_entry = dllist_entry(lnode, ep_ip_entry_t, ep_ip_lentry);
        HAL_TRACE_DEBUG("Checking for ip: {}",
                        ipaddr2str(&(pi_ip_entry->ip_addr)));
        for (i = 0; i < num_ips; i++) {
            ip_addr_spec_to_ip_addr(&ip_addr, req.endpoint_attrs().ip_address(i));
            if (!memcmp(&ip_addr, &pi_ip_entry->ip_addr, sizeof(ip_addr_t))) {
                ip_exists = true;
                break;
            } else {
                continue;
            }
        }
        if (!ip_exists) {
            // Have to delet the IP
            // Create ep_ip_entry, copy it over and add it to del list
            // Note: Both PIs are pointing to the same PD.
            ep_ipe = (ep_ip_entry_t *)g_hal_state->ep_ip_entry_slab()->alloc();
            HAL_ABORT(ep_ipe != NULL);
            memcpy(&ep_ipe->ip_addr, &pi_ip_entry->ip_addr, sizeof(ip_addr_t));
            ep_ipe->ip_flags = pi_ip_entry->ip_flags;
            ep_ipe->pd = pi_ip_entry->pd;
            sdk::lib::dllist_reset(&ep_ipe->ep_ip_lentry);
            sdk::lib::dllist_add(*del_iplist, &ep_ipe->ep_ip_lentry);
            *iplist_change = true;
            HAL_TRACE_DEBUG("Added to delete list IP {}",
                            ipaddr2str(&(ep_ipe->ip_addr)));
        }
        ip_exists = false;
    }
    if (!*iplist_change) {
        endpoint_cleanup_ip_list(add_iplist);
        endpoint_cleanup_ip_list(del_iplist);
    }

    return ret;
}

//----------------------------------------------------------------------------
// clean up IP list
//----------------------------------------------------------------------------
hal_ret_t
endpoint_cleanup_ip_list(dllist_ctxt_t **list)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (*list == NULL) {
        return ret;
    }
    endpoint_free_ip_list(*list);
    HAL_FREE(HAL_MEM_ALLOC_DLLIST, *list);
    *list = NULL;

    return ret;
}

//-----------------------------------------------------------------------------
// free elements in ip list
//-----------------------------------------------------------------------------
hal_ret_t
endpoint_free_ip_list(dllist_ctxt_t *iplist)
{
    hal_ret_t       ret = HAL_RET_OK;
    dllist_ctxt_t   *curr, *next;
    ep_ip_entry_t   *pi_ip_entry;

    dllist_for_each_safe(curr, next, iplist) {
        pi_ip_entry = dllist_entry(curr, ep_ip_entry_t, ep_ip_lentry);
        // delete from list
        sdk::lib::dllist_del(&pi_ip_entry->ep_ip_lentry);
        // free the entry
        hal::delay_delete_to_slab(HAL_SLAB_EP_IP_ENTRY, pi_ip_entry);
    }

    return ret;
}

hal_ret_t
endpoint_update_pi_with_iplist (ep_t *ep, dllist_ctxt_t *add_iplist,
                                dllist_ctxt_t *del_iplist)
{
    hal_ret_t       ret = HAL_RET_OK;
    dllist_ctxt_t   *curr, *next;
    ep_ip_entry_t   *pi_ip_entry = NULL, *del_ip_entry = NULL;
    ep_l3_key_t     l3_key = { 0 };
    vrf_t           *vrf = NULL;

    vrf = vrf_lookup_by_handle(ep->vrf_handle);
    if (vrf == NULL) {
        HAL_TRACE_ERR("Failed to find vrf with handle {}", ep->vrf_handle);
        ret = HAL_RET_VRF_NOT_FOUND;
        goto end;
    }

    // Handling new IPs
    dllist_for_each_safe(curr, next, add_iplist) {
        pi_ip_entry = dllist_entry(curr, ep_ip_entry_t, ep_ip_lentry);
        HAL_TRACE_DEBUG("Adding new IP {}",
                        ipaddr2str(&(pi_ip_entry->ip_addr)));

        sdk::lib::dllist_del(&pi_ip_entry->ep_ip_lentry);

        // Insert into EP's ip list
        sdk::lib::dllist_add(&ep->ip_list_head, &pi_ip_entry->ep_ip_lentry);

        // Insert to L3 hash table with (VRF, IP) key
        l3_key.vrf_id = vrf->vrf_id;
        l3_key.ip_addr = pi_ip_entry->ip_addr;
        ret = ep_add_to_l3_db(&l3_key, pi_ip_entry, ep->hal_handle);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to add EP {} to L3 DB", ep_l2_key_to_str(ep));
            goto end;
        }
        HAL_TRACE_DEBUG("Added EP ({}, {}) to L3 DB",
                        l3_key.vrf_id, ipaddr2str(&l3_key.ip_addr));


#if 0
        l3_entry = (ep_l3_entry_t *)HAL_CALLOC(HAL_MEM_ALLOC_EP,
                sizeof(ep_l3_entry_t));
        l3_entry->l3_key.vrf_id = vrf->vrf_id;
        memcpy(&l3_entry->l3_key.ip_addr, &pi_ip_entry->ip_addr, sizeof(ip_addr_t));
        l3_entry->ep = ep;
        l3_entry->ep_ip = pi_ip_entry;
        l3_entry->ep_l3_ht_ctxt.reset();
        g_hal_state->ep_l3_entry_ht()->insert(l3_entry,
                                              &l3_entry->ep_l3_ht_ctxt);
        HAL_TRACE_DEBUG("Added ({}, {}) to DB",
                        l3_entry->l3_key.vrf_id,
                        ipaddr2str(&l3_entry->l3_key.ip_addr));
#endif
    }

    // dump EP's IPs
    ep_print_ips(ep);

    // Handling removed IPs
    dllist_for_each_safe(curr, next, del_iplist) {
        pi_ip_entry = dllist_entry(curr, ep_ip_entry_t, ep_ip_lentry);
        HAL_TRACE_DEBUG("Deleting IP {}", ipaddr2str(&(pi_ip_entry->ip_addr)));

        sdk::lib::dllist_del(&pi_ip_entry->ep_ip_lentry);

        if (ip_in_ep(&pi_ip_entry->ip_addr, ep, &del_ip_entry)) {
            sdk::lib::dllist_del(&del_ip_entry->ep_ip_lentry);

            // Remove from hash table
            l3_key.vrf_id = vrf->vrf_id;
            l3_key.ip_addr = pi_ip_entry->ip_addr;
            ret = ep_del_from_l3_db(&l3_key);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to del EP {} from L3 DB",
                              ep_l2_key_to_str(ep));
                goto end;
            }
            HAL_TRACE_DEBUG("Deleted EP ({}, {}) from L3 DB",
                            l3_key.vrf_id,
                            ipaddr2str(&l3_key.ip_addr));

            // Free IP entry
            hal::delay_delete_to_slab(HAL_SLAB_EP_IP_ENTRY, del_ip_entry);

            // Free IP entry created for delete
            hal::delay_delete_to_slab(HAL_SLAB_EP_IP_ENTRY, pi_ip_entry);
#if 0
            l3_key.vrf_id = ep->vrf_id;
            memcpy(&l3_key.ip_addr, &del_ip_entry->ip_addr, sizeof(ip_addr_t));
            l3_entry = (ep_l3_entry_t *)g_hal_state->ep_l3_entry_ht()->remove(&l3_key);
            HAL_TRACE_DEBUG("Removed ({}, {}) from DB",
                            l3_key.vrf_id,
                            ipaddr2str(&l3_key.ip_addr));
            // Free L3 entry
            HAL_FREE(HAL_MEM_ALLOC_EP, l3_entry);
            // Free IP entry
            g_hal_state->ep_ip_entry_slab()->free(del_ip_entry);

            // Free IP entry created for delete
            g_hal_state->ep_ip_entry_slab()->free(pi_ip_entry);
#endif
        } else {
            // IPs which are getting deleted should be present in PI EP
            HAL_ASSERT(0);
        }
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// process a endpoint update request
//------------------------------------------------------------------------------
hal_ret_t
endpoint_update (EndpointUpdateRequest& req, EndpointResponse *rsp)
{
    hal_ret_t               ret           = HAL_RET_OK;
    vrf_id_t                tid;
    ep_t                    *ep           = NULL;
    vrf_t                   *vrf          = NULL;
    ApiStatus               api_status;
    bool                    if_change     = false, l2seg_change = false,
                            iplist_change = false;
    hal_handle_t            new_if_hdl    = 0, new_l2seg_hdl    = 0;
    dllist_ctxt_t           *add_iplist   = NULL, *del_iplist   = NULL;
    cfg_op_ctxt_t           cfg_ctxt      = { 0 };
    dhl_entry_t             dhl_entry     = { 0 };
    ep_update_app_ctxt_t    app_ctxt      = { 0 };

    ret = validate_endpoint_update_spec(req, rsp);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // fetch the vrf information
    tid = req.vrf_key_handle().vrf_id();
    vrf = vrf_lookup_by_id(tid);
    if (vrf == NULL) {
        HAL_TRACE_ERR("Failed to find vrf for tid {}", tid);
        ret = HAL_RET_VRF_NOT_FOUND;
        goto end;
    }

    // fetch the ep
    ret = find_ep(req.key_or_handle(), &ep, &api_status);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to fetch endpoint");
        rsp->set_api_status(api_status);
        goto end;
    }

    // check for change
    ret = endpoint_check_update(req, ep, &app_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Update change check failed: ret : {}", ret);
        goto end;
    }

    // check for l2seg change
    ret = endpoint_l2seg_update(req, ep, &l2seg_change, &new_l2seg_hdl);
    HAL_ABORT(ret == HAL_RET_OK);

    // check for ip change
    ret = endpoint_ip_list_update(req, ep, &iplist_change,
                                  &add_iplist, &del_iplist);
    HAL_ABORT(ret == HAL_RET_OK);


    if (!app_ctxt.if_change && !l2seg_change && !iplist_change) {
        HAL_TRACE_ERR("No change in EP update");
        goto end;
    }

    if (if_change) {
        // call actions
        app_ctxt.if_change = true;
        app_ctxt.new_if_handle = new_if_hdl;
    }

    if (l2seg_change) {
        // call actions
    }

    if (iplist_change) {
        app_ctxt.iplist_change = true;
        app_ctxt.add_iplist = add_iplist;
        app_ctxt.del_iplist = del_iplist;
    }

#if 0
        // call actions
        // 1. PD Call
        pd_ep_upd_args.iplist_change = true;
        pd_ep_upd_args.add_iplist = add_iplist;
        pd_ep_upd_args.del_iplist = del_iplist;
        // Revisit the position of this call if multiple changes
        // have to be sent to PD
        ret = pd::pd_ep_update(&pd_ep_upd_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("EP-Update: IPlist change to PD failed. ret:{}",
                    ret);
            rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
            goto end;
        }

        // Update PI
        endpoint_update_pi_with_iplist(ep, add_iplist, del_iplist);
#endif

    ep_make_clone(ep, (ep_t **)&dhl_entry.cloned_obj);

    // form ctxt and call infra update object
    dhl_entry.handle  = ep->hal_handle;
    dhl_entry.obj     = ep;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(ep->hal_handle, &cfg_ctxt,
                             endpoint_update_upd_cb,
                             endpoint_update_commit_cb,
                             endpoint_update_abort_cb,
                             endpoint_update_cleanup_cb);

end:

    if (add_iplist) {
        HAL_FREE(HAL_MEM_ALLOC_DLLIST, add_iplist);
    }
    if (del_iplist) {
        HAL_FREE(HAL_MEM_ALLOC_DLLIST, del_iplist);
    }

    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC(HAL_API_ENDPOINT_UPDATE_SUCCESS);
    } else {
        HAL_API_STATS_INC(HAL_API_ENDPOINT_UPDATE_FAIL);
    }
    ep_prepare_rsp(rsp, ret, ep ? ep->hal_handle : HAL_HANDLE_INVALID);
    return ret;
}

//------------------------------------------------------------------------------
// validate ep delete request
//------------------------------------------------------------------------------
static hal_ret_t
validate_endpoint_delete_req (EndpointDeleteRequest& req,
                          EndpointDeleteResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("EP spec has no key or handle");
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// validate ep delete
//------------------------------------------------------------------------------
static hal_ret_t
validate_endpoint_delete (ep_t *ep)
{
    hal_ret_t   ret = HAL_RET_OK;

    if (ep->nh_list->num_elems()) {
        ret = HAL_RET_OBJECT_IN_USE;
        HAL_TRACE_ERR("EP delete failure, NHs still referring:");
        hal_print_handles_block_list(ep->nh_list);
    }

    return ret;
}

//------------------------------------------------------------------------------
// 1. PD Call to delete PD and free up resources and deprogram HW
//------------------------------------------------------------------------------
hal_ret_t
endpoint_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_ep_delete_args_t     pd_ep_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    ep_t                        *ep = NULL;

    HAL_ASSERT(cfg_ctxt != NULL);

    // TODO: Check the dependency ref count for the ep.
    //       If its non zero, fail the delete.


    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    ep = (ep_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("EP del cb");

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_ep_delete_args_init(&pd_ep_args);
    pd_ep_args.ep = ep;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_EP_DELETE, (void *)&pd_ep_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to delete ep pd, err : {}", ret);
    }
    return ret;
}

//------------------------------------------------------------------------------
// Update PI DBs as vrf_delete_del_cb() was a succcess
//      a. Delete from vrf id hash table
//      b. Remove object from handle id based hash table
//      c. Free PI vrf
//------------------------------------------------------------------------------
hal_ret_t
endpoint_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    ep_t                        *ep = NULL;
    hal_handle_t                hal_handle = 0;

    HAL_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    ep = (ep_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("Delete commit cb");

    ret = endpoint_cleanup(ep);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to del ep from db, err : {}", ret);
        goto end;
    }

    hal_handle_free(hal_handle);

    // TODO: Decrement the ref counts of dependent objects
    //  - Have to decrement ref count for ep profile

end:

    return ret;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
endpoint_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
endpoint_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a nwsec delete request
//------------------------------------------------------------------------------
hal_ret_t
endpoint_delete (EndpointDeleteRequest& req,
                 EndpointDeleteResponse *rsp)
{
    hal_ret_t                       ret = HAL_RET_OK;
    vrf_id_t                     tid;
    ep_t                            *ep = NULL;
    vrf_t                        *vrf = NULL;
    cfg_op_ctxt_t                   cfg_ctxt = { 0 };
    dhl_entry_t                     dhl_entry = { 0 };
    ApiStatus                       api_status;

    // validate the request message
    ret = validate_endpoint_delete_req(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("EP delete validation failed, err : {}", ret);
        goto end;
    }

    // fetch the vrf information
    tid = req.vrf_key_handle().vrf_id();
    vrf = vrf_lookup_by_id(tid);
    if (vrf == NULL) {
        HAL_TRACE_ERR("Failed to find vrf for tid {}", tid);
        ret = HAL_RET_VRF_NOT_FOUND;
        // rsp->set_api_status(types::API_STATUS_NOT_FOUND);
        goto end;
    }

    // fetch the ep
    ret = find_ep(req.key_or_handle(), &ep, &api_status);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to find EP");
        goto end;
    }

    HAL_TRACE_DEBUG("Deleting EP {}", ep_l2_key_to_str(ep));

    // validate the EP delete
    ret = validate_endpoint_delete(ep);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("EP delete validation failed, err : {}", ret);
        goto end;
    }


    // form ctxt and call infra add
    dhl_entry.handle = ep->hal_handle;
    dhl_entry.obj = ep;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(ep->hal_handle, &cfg_ctxt,
                             endpoint_delete_del_cb,
                             endpoint_delete_commit_cb,
                             endpoint_delete_abort_cb,
                             endpoint_delete_cleanup_cb);

end:

    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC(HAL_API_ENDPOINT_DELETE_SUCCESS);
    } else {
        HAL_API_STATS_INC(HAL_API_ENDPOINT_DELETE_FAIL);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));
    return ret;
}

static void
ep_to_ep_get_response (ep_t *ep, EndpointGetResponse *response)
{
    dllist_ctxt_t                      *lnode = NULL;
    ep_ip_entry_t                      *pi_ip_entry = NULL;
    EplearnStatus                      *ep_learn_status;
    pd::pd_ep_get_args_t               args = {0};
    hal_ret_t                          ret = HAL_RET_OK;

    auto vrf       = vrf_lookup_by_handle(ep->vrf_handle);
    auto interface = find_if_by_handle(ep->if_handle);

    response->mutable_spec()->mutable_vrf_key_handle()->set_vrf_id(vrf ? vrf->vrf_id : HAL_HANDLE_INVALID);
    response->mutable_spec()->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_segment_id(ep->l2_key.l2_segid);
    response->mutable_spec()->mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(MAC_TO_UINT64(ep->l2_key.mac_addr));
    response->mutable_spec()->mutable_endpoint_attrs()->mutable_interface_key_handle()->set_interface_id(interface->if_id);
    response->mutable_spec()->mutable_endpoint_attrs()->set_useg_vlan(ep->useg_vlan);

    response->mutable_status()->set_endpoint_handle(ep->hal_handle);
    response->mutable_status()->set_learn_source_config(ep->ep_flags & EP_FLAGS_LEARN_SRC_CFG);
    response->mutable_status()->set_is_endpoint_local(ep->ep_flags & EP_FLAGS_LOCAL);

    lnode = ep->ip_list_head.next;
    dllist_for_each(lnode, &(ep->ip_list_head)) {
        pi_ip_entry = (ep_ip_entry_t *)((char *)lnode -
                offsetof(ep_ip_entry_t, ep_ip_lentry));
        types::IPAddress *ip_addr_spec =
                response->mutable_spec()->mutable_endpoint_attrs()->add_ip_address();
        ip_addr_to_spec(ip_addr_spec, &pi_ip_entry->ip_addr);

        EndpointIpAddress *endpoint_ip_addr = response->mutable_status()->add_ip_address();
        ip_addr_to_spec(endpoint_ip_addr->mutable_ip_address(), &pi_ip_entry->ip_addr);
        ep_learn_status = endpoint_ip_addr->mutable_learn_status();
        if ((pi_ip_entry->ip_flags & EP_FLAGS_LEARN_SRC_DHCP) &&
                (dhcp_status_func != nullptr)) {
            dhcp_status_func(vrf->vrf_id, &pi_ip_entry->ip_addr,
                    ep_learn_status->mutable_dhcp_status());
            endpoint_ip_addr->set_learn_source_dhcp(true);
        } else if ((pi_ip_entry->ip_flags & EP_FLAGS_LEARN_SRC_ARP) &&
                (arp_status_func != nullptr)) {
            arp_status_func(vrf->vrf_id, &pi_ip_entry->ip_addr,
                    ep_learn_status->mutable_arp_status());
            endpoint_ip_addr->set_learn_source_dhcp(true);
        } else if (pi_ip_entry->ip_flags & EP_FLAGS_LEARN_SRC_CFG) {
            endpoint_ip_addr->set_learn_source_dhcp(pi_ip_entry->ip_flags & EP_FLAGS_LEARN_SRC_CFG);
            endpoint_ip_addr->set_learn_source_config(true);
        }
    }

    HAL_TRACE_DEBUG("Finished PI get");

    // Getting PD information
    args.ep = ep;
    args.rsp = response;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_EP_GET, (void *)&args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to do PD get for Endpoint err: {}",
                      ret);
    }

    response->set_api_status(types::API_STATUS_OK);
}

static bool
ep_get_ht_cb (void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t *entry      = (hal_handle_id_ht_entry_t *)ht_entry;
    EndpointGetResponseMsg   *response   = (EndpointGetResponseMsg *)ctxt;
    ep_t                     *ep         = NULL;
    EndpointGetResponse      *rsp;

    ep = (ep_t *)hal_handle_get_obj(entry->handle_id);

    rsp = response->add_response();

    ep_to_ep_get_response(ep, rsp);

    // Always return false here, so that we walk through all hash table
    // entries.
    return false;
}


//------------------------------------------------------------------------------
// process a endpoint get request
//------------------------------------------------------------------------------
hal_ret_t
endpoint_get (EndpointGetRequest& req, EndpointGetResponseMsg *rsp)
{
    ep_l3_key_t            l3_key = { 0 };
    mac_addr_t             mac_addr = { 0 };
    ep_t                   *ep;
    EndpointGetResponse    *response;

    if (!req.has_vrf_key_handle() ||
        req.vrf_key_handle().vrf_id() == HAL_VRF_ID_INVALID) {
        g_hal_state->ep_l2_ht()->walk(ep_get_ht_cb, rsp);
        HAL_API_STATS_INC(HAL_API_ENDPOINT_GET_SUCCESS);
        return HAL_RET_OK;
    }

    response = rsp->add_response();

    if (req.has_key_or_handle()) {
        auto kh = req.key_or_handle();
        if (kh.key_or_handle_case() == EndpointKeyHandle::kEndpointKey) {
            auto ep_key = kh.endpoint_key();
            if (ep_key.has_l2_key()) {
                auto ep_l2_key = ep_key.l2_key();
                MAC_UINT64_TO_ADDR(mac_addr, ep_l2_key.mac_address());
                ep = find_ep_by_l2_key(ep_l2_key.l2segment_key_handle().segment_id(), mac_addr);
            } else if (ep_key.has_l3_key()) {
                auto ep_l3_key = ep_key.l3_key();
                l3_key.vrf_id = req.vrf_key_handle().vrf_id();
                ip_addr_spec_to_ip_addr(&l3_key.ip_addr,
                                        ep_l3_key.ip_address());
                ep = find_ep_by_l3_key(&l3_key);
            } else {
                response->set_api_status(types::API_STATUS_INVALID_ARG);
                HAL_API_STATS_INC(HAL_API_ENDPOINT_GET_FAIL);
                return HAL_RET_INVALID_ARG;
            }
        } else if (kh.key_or_handle_case() ==
                       EndpointKeyHandle::kEndpointHandle) {
            ep = find_ep_by_handle(kh.endpoint_handle());
        } else {
            response->set_api_status(types::API_STATUS_INVALID_ARG);
            HAL_API_STATS_INC(HAL_API_ENDPOINT_GET_FAIL);
            return HAL_RET_INVALID_ARG;
        }
    } else {
        response->set_api_status(types::API_STATUS_INVALID_ARG);
        HAL_API_STATS_INC(HAL_API_ENDPOINT_GET_FAIL);
        return HAL_RET_INVALID_ARG;
    }

    if (ep == NULL) {
        response->set_api_status(types::API_STATUS_NOT_FOUND);
        HAL_API_STATS_INC(HAL_API_ENDPOINT_GET_FAIL);
        return HAL_RET_EP_NOT_FOUND;
    }

    ep_to_ep_get_response(ep, response);

    HAL_API_STATS_INC(HAL_API_ENDPOINT_GET_SUCCESS);

    return HAL_RET_OK;
}

const char *
ep_l2_key_to_str (ep_t *ep)
{
    static thread_local char       ep_str[4][50];
    static thread_local uint8_t    ep_str_next = 0;
    char                           *buf;

    buf = ep_str[ep_str_next++ & 0x3];
    memset(buf, 0, 50);
    if (ep) {
        snprintf(buf, 50, "%lu::%s", ep->l2_key.l2_segid,
                 ether_ntoa((struct ether_addr *)(ep->l2_key.mac_addr)));
    }
    return buf;
}

void
ep_print_ips (ep_t *ep)
{
    dllist_ctxt_t   *lnode = NULL;
    ep_ip_entry_t   *pi_ip_entry = NULL;

    dllist_for_each(lnode, &(ep->ip_list_head)) {
        pi_ip_entry = dllist_entry(lnode, ep_ip_entry_t, ep_ip_lentry);
        HAL_TRACE_DEBUG("IP {}", ipaddr2str(&(pi_ip_entry->ip_addr)));
    }
}

//-----------------------------------------------------------------------------
// Adds session into EP
//-----------------------------------------------------------------------------
hal_ret_t
ep_add_session (ep_t *ep, session_t *session)
{
    hal_ret_t                   ret = HAL_RET_OK;
    hal_handle_id_list_entry_t  *entry = NULL;

    if (ep == NULL || session == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // allocate the entry
    entry = (hal_handle_id_list_entry_t *)g_hal_state->
                hal_handle_id_list_entry_slab()->alloc();
    if (entry == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }
    entry->handle_id = session->hal_handle;

    ep_lock(ep, __FILENAME__, __LINE__, __func__);
    sdk::lib::dllist_add(&ep->session_list_head, &entry->dllist_ctxt);
    ep_unlock(ep, __FILENAME__, __LINE__, __func__);

end:

    HAL_TRACE_DEBUG("add ep {}/{} => session {}/{}, ret : {}",
                    ep_l2_key_to_str(ep), ep->hal_handle,
                    session->config.session_id, session->hal_handle,
                    ret);
    return ret;
}

//-----------------------------------------------------------------------------
// Adds nh to ep back refs
//-----------------------------------------------------------------------------
hal_ret_t
ep_add_nh (ep_t *ep, nexthop_t *nh)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (ep == NULL || nh == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    ep_lock(ep, __FILENAME__, __LINE__, __func__);      // lock
    ret = ep->nh_list->insert(&nh->hal_handle);
    ep_unlock(ep, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to add nh {} to ep {}",
                        nh->nh_id, ep_l2_key_to_str(ep));
        goto end;
    }

    HAL_TRACE_DEBUG("Added nh {} to ep {}", nh->nh_id, ep_l2_key_to_str(ep));
end:
    return ret;
}

//-----------------------------------------------------------------------------
// Remove nh from ep
//-----------------------------------------------------------------------------
hal_ret_t
ep_del_nh (ep_t *ep, nexthop_t *nh)
{
    hal_ret_t                   ret = HAL_RET_OK;

    if (ep == NULL || nh == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    ep_lock(ep, __FILENAME__, __LINE__, __func__);      // lock
    ret = ep->nh_list->remove(&nh->hal_handle);
    ep_unlock(ep, __FILENAME__, __LINE__, __func__);    // unlock
    if (ret != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to add nh {} to ep {}",
                        nh->nh_id, ep_l2_key_to_str(ep));
        goto end;
    }

    HAL_TRACE_DEBUG("Deleted nh {} from ep {}", nh->nh_id,
                    ep_l2_key_to_str(ep));
end:
    return ret;
}

//-----------------------------------------------------------------------------
// Remove If from lif list
//-----------------------------------------------------------------------------
hal_ret_t
ep_del_session (ep_t *ep, session_t *session)
{
    hal_ret_t                   ret = HAL_RET_IF_NOT_FOUND;
    hal_handle_id_list_entry_t  *entry = NULL;
    dllist_ctxt_t               *curr = NULL, *next = NULL;

    ep_lock(ep, __FILENAME__, __LINE__, __func__);      // lock
    dllist_for_each_safe(curr, next, &ep->session_list_head) {
        entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        if (entry->handle_id == session->hal_handle) {
            // Remove from list
            sdk::lib::dllist_del(&entry->dllist_ctxt);
            // Free the entry
            hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_LIST_ENTRY, entry);
            ret = HAL_RET_OK;
        }
    }
    ep_unlock(ep, __FILENAME__, __LINE__, __func__);    // unlock

    HAL_TRACE_DEBUG("add ep =/=> session, ids: {} =/=> {}, "
                    "hdls: {} => {}, ret:{}",
                    ep_l2_key_to_str(ep), session->config.session_id,
                    ep->hal_handle, session->hal_handle, ret);
    return ret;
}


void
register_dhcp_ep_status_callback (dhcp_status_func_t func)
{
    dhcp_status_func = func;
}

void
register_arp_ep_status_callback (arp_status_func_t func)
{
    arp_status_func = func;
}

//-----------------------------------------------------------------------------
// given a endpoint, marshall it for persisting the endpoint state (spec, status, stats)
//
// obj points to endpoint object i.e., ep_t
// mem is the memory buffer to serialize the state into
// len is the length of the buffer provided
// mlen is to be filled by this function with marshalled state length
//-----------------------------------------------------------------------------
hal_ret_t
ep_store_cb (void *obj, uint8_t *mem, uint32_t len, uint32_t *mlen)
{
    EndpointGetResponse     ep_info;
    uint32_t                serialized_state_sz;
    ep_t                    *ep = (ep_t *)obj;

    HAL_TRACE_DEBUG("Storing EPs");
    HAL_ASSERT((ep != NULL) && (mlen != NULL));
    *mlen = 0;

    // get all information about this ep (includes spec, status & stats)
    ep_to_ep_get_response(ep, &ep_info);
    serialized_state_sz = ep_info.ByteSizeLong();
    if (serialized_state_sz > len) {
        HAL_TRACE_ERR("Failed to marshall EP {}, not enough room, "
                      "required size {}, available size {}",
                      ep_l2_key_to_str(ep), serialized_state_sz, len);
        return HAL_RET_OOM;
    }

    // serialize all the state
    if (ep_info.SerializeToArray(mem, serialized_state_sz) == false) {
        HAL_TRACE_ERR("Failed to serialize EP {}", ep_l2_key_to_str(ep));
        return HAL_RET_OOM;
    }
    *mlen = serialized_state_sz;
    HAL_TRACE_DEBUG("Marshalled EP {}, len {}",
                    ep_l2_key_to_str(ep), serialized_state_sz);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize an EP's oper status from its status object
//------------------------------------------------------------------------------
static hal_ret_t
ep_init_from_status (ep_t *ep, const EndpointStatus& status)
{
    ep->hal_handle = status.endpoint_handle();
    if (status.learn_source_config()) {
        ep->ep_flags |= EP_FLAGS_LEARN_SRC_CFG;
    }
    if (status.is_endpoint_local()) {
        ep->ep_flags |= EP_FLAGS_LOCAL;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize an EP's oper stats from its stats object
//------------------------------------------------------------------------------
static hal_ret_t
ep_init_from_stats (ep_t *ep, const EndpointStats& stats)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// ep's restore add PD
//------------------------------------------------------------------------------
static hal_ret_t
ep_restore_add (ep_t *ep, vrf_t *vrf,
                l2seg_t *l2seg, if_t *hal_if,
                const EndpointGetResponse& ep_info)
{
    hal_ret_t                    ret;
    pd::pd_ep_restore_args_t    pd_ep_args = { 0 };

    // restore pd state
    pd::pd_ep_restore_args_init(&pd_ep_args);
    pd_ep_args.ep = ep;
    pd_ep_args.vrf = vrf;
    pd_ep_args.l2seg = l2seg;
    pd_ep_args.intf = hal_if;

    pd_ep_args.ep_status = &ep_info.status();
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_EP_RESTORE, &pd_ep_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to restore EP {} pd, err : {}",
                       ep_l2_key_to_str(ep), ret);
    }

    return ret;
}

static hal_ret_t
ep_restore_commit (ep_t *ep, vrf_t *vrf,
                   const EndpointGetResponse& ep_info)
{
    dllist_ctxt_t        *ip_lnode    = NULL;
    ep_ip_entry_t        *pi_ip_entry = NULL;
    ep_l3_key_t          l3_key       = {0};
    hal_ret_t            ret          = HAL_RET_OK;

    // add EP to L2 DB
    ret = ep_add_to_l2_db (ep, ep->hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add EP {} to L2 DB, err : {}",
                      ep_l2_key_to_str(ep), ret);
        goto end;
    }

    // add EP to L3 DB
    dllist_for_each(ip_lnode, &ep->ip_list_head) {
        pi_ip_entry = dllist_entry(ip_lnode, ep_ip_entry_t, ep_ip_lentry);
        l3_key.vrf_id = vrf->vrf_id;
        l3_key.ip_addr = pi_ip_entry->ip_addr;
        ret = ep_add_to_l3_db(&l3_key, pi_ip_entry, ep->hal_handle);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to add EP {} to L3 DB, err : {}",
                          ep_l2_key_to_str(ep), ret);
            goto end;
        }
        HAL_TRACE_DEBUG("Added EP ({}, {}) to L3 DB",
                        l3_key.vrf_id, ipaddr2str(&l3_key.ip_addr));
    }

end:

    return ret;
}

static hal_ret_t
ep_restore_abort (ep_t *ep, const EndpointGetResponse& ep_info)
{
    HAL_TRACE_ERR("Aborting EP {} restore", ep_l2_key_to_str(ep));
    endpoint_create_abort_cleanup(ep, ep->hal_handle);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// ep's restore cb.
//  - restores EP to the PI and PD state before the upgrade
//------------------------------------------------------------------------------
uint32_t
ep_restore_cb (void *obj, uint32_t len)
{
    hal_ret_t               ret;
    EndpointGetResponse     ep_info;
    ep_t                    *ep;
    vrf_id_t                tid;
    L2SegmentKeyHandle      l2seg_key_handle;
    InterfaceKeyHandle      if_key_handle;
    vrf_t                   *vrf = NULL;
    l2seg_t                 *l2seg = NULL;
    if_t                    *hal_if = NULL;
 
    HAL_TRACE_DEBUG("Restoring EPs");

    // de-serialize the object
    if (ep_info.ParseFromArray(obj, len) == false) {
        HAL_TRACE_ERR("Failed to de-serialize a serialized EP obj");
        HAL_ASSERT(0);
        return 0;
    }

    // fetch the vrf information
    tid = ep_info.spec().vrf_key_handle().vrf_id();
    vrf = vrf_lookup_by_id(tid);
    if (vrf == NULL) {
        HAL_TRACE_ERR("Failed to find vrf while restoring EP");
        return HAL_RET_VRF_NOT_FOUND;
    }

    // fetch the L2 segment information
    l2seg_key_handle =
        ep_info.spec().key_or_handle().endpoint_key().l2_key().l2segment_key_handle();
    l2seg = l2seg_lookup_key_or_handle(l2seg_key_handle);
    if (l2seg == NULL) {
        HAL_TRACE_ERR("Failed to find l2seg while restoring EP");
        return HAL_RET_L2SEG_NOT_FOUND;
    }

    // fetch the interface information
    if_key_handle = ep_info.spec().endpoint_attrs().interface_key_handle();
    hal_if = if_lookup_key_or_handle(if_key_handle);
    if (hal_if == NULL) {
        HAL_TRACE_ERR("Failed to find hal_if while restoring EP");
        return HAL_RET_IF_NOT_FOUND;
    }

    // allocate EP obj from slab
    ep = ep_alloc_init();
    if (ep == NULL) {
        HAL_TRACE_ERR("Failed to alloc/init EP, err : {}", ret);
        return 0;
    }

    ep_init_from_status(ep, ep_info.status());
    // initialize ep attrs from its spec
    ep_init_from_spec(ep, ep_info.spec(), false);
    ep_init_from_stats(ep, ep_info.stats());

    // repopulate handle db
    hal_handle_insert(HAL_OBJ_ID_ENDPOINT, ep->hal_handle, (void *)ep);

    ret = ep_restore_add(ep, vrf, l2seg, hal_if, ep_info);
    if (ret != HAL_RET_OK) {
        ep_restore_abort(ep, ep_info);
    }
    ep_restore_commit(ep, vrf, ep_info);

    return 0;    // TODO: fix me
}

}    // namespace hal

