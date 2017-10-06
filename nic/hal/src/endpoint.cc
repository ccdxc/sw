#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/utils.hpp"
#include "nic/hal/src/endpoint.hpp"
#include "nic/include/pd_api.hpp"
// #include <netinet/ether.h>

namespace hal {

// ----------------------------------------------------------------------------
// hash table l2key => ht_entry
//  - Get key from entry
// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
// hash table l2_key => entry - compute hash
// ----------------------------------------------------------------------------
uint32_t
ep_compute_l2_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(ep_l2_key_t)) % ht_size;
}

// ----------------------------------------------------------------------------
// hash table l2_key => entry - compare function
// ----------------------------------------------------------------------------
bool
ep_compare_l2_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
#if 0
    HAL_TRACE_DEBUG("key1.l2_segid {}, key2.l2_segid {}",
                    ((ep_l2_key_t *)key1)->l2_segid,
                    ((ep_l2_key_t *)key2)->l2_segid);
    HAL_TRACE_DEBUG("key1.mac {}, key2.mac {}",
                    macaddr2str(((ep_l2_key_t *)key1)->mac_addr),
                    macaddr2str(((ep_l2_key_t *)key2)->mac_addr));
#endif

    if (!memcmp(key1, key2, sizeof(ep_l2_key_t))) {
        return true;
    }
    return false;
}

void *
ep_get_l3_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((ep_l3_entry_t *)entry)->l3_key);
}

uint32_t
ep_compute_l3_hash_func (void *key, uint32_t ht_size)
{
#if 0
    HAL_TRACE_DEBUG("L3 key hash {}",
                    utils::hash_algo::fnv_hash(key, sizeof(ep_l3_key_t)) % ht_size);
#endif
    return utils::hash_algo::fnv_hash(key, sizeof(ep_l3_key_t)) % ht_size;
}

bool
ep_compare_l3_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));

#if 0
    HAL_TRACE_DEBUG("key1.tid {}, key2.tid {}",
                    ((ep_l3_key_t *)key1)->tenant_id,
                    ((ep_l3_key_t *)key2)->tenant_id);
    HAL_TRACE_DEBUG("key1.ip {}, key2.ip {}",
                    ipaddr2str(&((ep_l3_key_t *)key1)->ip_addr),
                    ipaddr2str(&((ep_l3_key_t *)key2)->ip_addr));
#endif

    if (!memcmp(key1, key2, sizeof(ep_l3_key_t))) {
        return true;
    }
    return false;
}

#if 0
void *
ep_get_handle_key_func(void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((ep_t *)entry)->hal_handle);
}

uint32_t
ep_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
ep_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}
#endif

//------------------------------------------------------------------------------
// insert an ep to l2 db
//------------------------------------------------------------------------------
static inline hal_ret_t
ep_add_to_l2_db (ep_t *ep, hal_handle_t handle)
{
    hal_ret_t                   ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("pi-ep:{}:adding to ep l2 hash table", 
                    __FUNCTION__);
    // allocate an entry to establish mapping from l2key to its handle
    entry =
        (hal_handle_id_ht_entry_t *)g_hal_state->
        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from tenant id to its handle
    entry->handle_id = handle;
    ret = g_hal_state->ep_l2_ht()->insert_with_key(&ep->l2_key,
                                                   entry, &entry->ht_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-ep:{}:failed to add l2 key to handle mapping, "
                      "err : {}", __FUNCTION__, ret);
        g_hal_state->hal_handle_id_ht_entry_slab()->free(entry);
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

    HAL_TRACE_DEBUG("pi-ep:{}:removing from l2 hash table", __FUNCTION__);
    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->ep_l2_ht()->
            remove(&ep->l2_key);

    if (entry) {
        // free up
        g_hal_state->hal_handle_id_ht_entry_slab()->free(entry);
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
    ep_l3_entry_t               *entry;

    HAL_TRACE_DEBUG("pi-ep:{}:adding to ep l2 hash table", 
                    __FUNCTION__);
    // allocate an entry to establish mapping from l3key to its handle
    entry = (ep_l3_entry_t *)g_hal_state->ep_l3_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from tenant id to its handle
    entry->ep_hal_handle = handle;
    entry->l3_key = *l3_key;
    entry->ep_ip = ep_ip;
    entry->ht_ctxt.reset();
    ret = g_hal_state->ep_l3_entry_ht()->insert_with_key(l3_key,
                                                         entry, &entry->ht_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-ep:{}:failed to add l2 key to handle mapping, "
                      "err : {}", __FUNCTION__, ret);
        g_hal_state->hal_handle_id_ht_entry_slab()->free(entry);
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

    HAL_TRACE_DEBUG("pi-ep:{}:removing from l3 hash table", __FUNCTION__);
    // remove from hash table
    l3_entry = (ep_l3_entry_t *)g_hal_state->ep_l3_entry_ht()->remove(l3_key);

    if (l3_entry) {
        // free up
        g_hal_state->ep_l3_entry_slab()->free(l3_entry);
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate an incoming endpoint create request
//------------------------------------------------------------------------------
static hal_ret_t
validate_endpoint_create (EndpointSpec& spec, EndpointResponse *rsp)
{
    if (!spec.has_meta() ||
        spec.meta().tenant_id() == HAL_TENANT_ID_INVALID) {
        HAL_TRACE_ERR("pi-ep:{}:tenant id not valid",
                      __FUNCTION__);
        return HAL_RET_TENANT_ID_INVALID;
    }

    if (spec.l2_segment_handle() == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("pi-ep:{}:l2seg handle not valid",
                      __FUNCTION__);
        return HAL_RET_HANDLE_INVALID;
    }

    if (spec.interface_handle() == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("pi-ep:{}:interface handle not valid",
                      __FUNCTION__);
        return HAL_RET_HANDLE_INVALID;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// PD Call to allocate PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
endpoint_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_ep_args_t            pd_ep_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    ep_t                        *ep = NULL;
    ep_create_app_ctxt_t        *app_ctxt = NULL; 

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-ep:{}: invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (ep_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    ep = (ep_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-ep:{}:create add CB {}",
                    __FUNCTION__, ep_l2_key_to_str(ep));

    // PD Call to allocate PD resources and HW programming
    pd::pd_ep_args_init(&pd_ep_args);
    pd_ep_args.ep = ep;
    pd_ep_args.tenant = app_ctxt->tenant;
    pd_ep_args.l2seg = app_ctxt->l2seg;
    pd_ep_args.intf = app_ctxt->hal_if;
    ret = pd::pd_ep_create(&pd_ep_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-ep:{}:failed to create ep pd, err : {}", 
                      __FUNCTION__, ret);
    }

end:
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
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    ep_t                        *ep = NULL;
    tenant_t                    *tenant = NULL;
    hal_handle_t                hal_handle = 0;
    dllist_ctxt_t               *ip_lnode = NULL;
    ep_ip_entry_t               *pi_ip_entry = NULL;
    ep_create_app_ctxt_t        *app_ctxt = NULL; 
    ep_l3_key_t                 l3_key = { 0 };

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-ep:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (ep_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    ep = (ep_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;
    tenant = app_ctxt->tenant;

    HAL_TRACE_DEBUG("pi-ep:{}:create commit CB {}",
                    __FUNCTION__, ep_l2_key_to_str(ep));

    // Add EP to L2 DB
    ret = ep_add_to_l2_db (ep, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-ep:{}:unable to add EP:{} to L2 DB", 
                      __FUNCTION__, ep_l2_key_to_str(ep));
        goto end;
    }

    HAL_TRACE_ERR("pi-ep:{}:added EP:{} to L2 DB", 
                  __FUNCTION__, ep_l2_key_to_str(ep));

    // Add EP to L3 DB
    dllist_for_each(ip_lnode, &ep->ip_list_head) {
        pi_ip_entry = dllist_entry(ip_lnode, ep_ip_entry_t, ep_ip_lentry);
        l3_key.tenant_id = tenant->tenant_id;
        l3_key.ip_addr = pi_ip_entry->ip_addr;
        ret = ep_add_to_l3_db(&l3_key, pi_ip_entry, hal_handle);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-ep:{}:unable to add EP:{} to L3 DB", 
                          __FUNCTION__, ep_l2_key_to_str(ep));
            goto end;
        }
        HAL_TRACE_DEBUG("pi-ep:{}:added EP ({}, {}) to L3 DB",
                        __FUNCTION__,
                        l3_key.tenant_id,
                        ipaddr2str(&l3_key.ip_addr));
        HAL_TRACE_DEBUG("pi-ep:{}:added EP ({}, {}) to L3 DB",
                        __FUNCTION__,
                        l3_key.tenant_id,
                        ipaddr2str(&pi_ip_entry->ip_addr));
    }

    // TODO: Increment the ref counts of dependent objects
    //  - Have to increment ref count for tenant

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
    hal_ret_t       ret = HAL_RET_OK;
    dllist_ctxt_t   *curr, *next;
    ep_ip_entry_t   *pi_ip_entry = NULL;
    tenant_t        *tenant = NULL;
    ep_l3_key_t     l3_key = { 0 };

    tenant = tenant_lookup_by_handle(ep->tenant_handle);

    // Remove EP from L2 DB
    ret = ep_del_from_l2_db(ep);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-ep:{}:enable to delete EP from L3 DB", __FUNCTION__);
        goto end;
    }
    HAL_TRACE_ERR("pi-ep:{}:deleted EP:{} from L2 DB", 
                  __FUNCTION__, ep_l2_key_to_str(ep));

    // Remove EP from L3 DB
    dllist_for_each_safe(curr, next, &ep->ip_list_head) {
        pi_ip_entry = dllist_entry(curr, ep_ip_entry_t, ep_ip_lentry);
        l3_key.tenant_id = tenant->tenant_id;
        l3_key.ip_addr = pi_ip_entry->ip_addr;
        ret = ep_del_from_l3_db(&l3_key);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-ep:{}:unable to del EP:{} from L3 DB", 
                          __FUNCTION__, ep_l2_key_to_str(ep));
            goto end;
        }
        HAL_TRACE_DEBUG("pi-ep:{}:deleted EP ({}, {}) from L3 DB",
                        __FUNCTION__,
                        l3_key.tenant_id,
                        ipaddr2str(&l3_key.ip_addr));
    }

    // Free EP
    ret = ep_free(ep);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-ep:{}:unable to free EP", __FUNCTION__);
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
// 3. Free PI tenant 
//------------------------------------------------------------------------------
hal_ret_t
endpoint_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                       ret = HAL_RET_OK;
    pd::pd_ep_args_t                pd_ep_args = { 0 };
    dllist_ctxt_t                   *lnode = NULL;
    dhl_entry_t                     *dhl_entry = NULL;
    ep_t                            *ep = NULL;
    hal_handle_t                    hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-ep:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    ep = (ep_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-ep:{}:create abort CB {}", __FUNCTION__);

    // 1. delete call to PD
    if (ep->pd) {
        pd::pd_ep_args_init(&pd_ep_args);
        pd_ep_args.ep = ep;
        ret = pd::pd_ep_delete(&pd_ep_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-ep:{}:failed to delete ep pd, err : {}", 
                          __FUNCTION__, ret);
        }
    }

    // 2. remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    // 3. Free PI tenant
    endpoint_cleanup(ep);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
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
// - Applies only to Local endpoints. NOP for Remote EPs.
//------------------------------------------------------------------------------
static hal_ret_t
pin_endpoint (ep_t *ep)
{
    if_t        *hal_if = NULL;
    if_id_t     pin_ifid = 0;        
    
    ep->pinned_if_handle = HAL_HANDLE_INVALID;
    if (g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_DEFAULT) {
        HAL_TRACE_DEBUG("{}: NOP for forwarding_mode:default.", __FUNCTION__);
        return HAL_RET_OK;
    }

    if (ep->ep_flags & EP_FLAGS_REMOTE) {
        HAL_TRACE_DEBUG("{}: NOP for remote ep.", __FUNCTION__);
        return HAL_RET_OK;
    }

    // TEMP: Initial Commit only.
    // To be replaced by a proper pin selection algorithm.
    pin_ifid = allocate_pin_ifid_for_endpoint(ep);
    hal_if = find_if_by_id(pin_ifid);
    if (hal_if == NULL) {
        HAL_TRACE_ERR("{}: Interface not found for Id:{}.",
                      __FUNCTION__, pin_ifid);
        return HAL_RET_IF_NOT_FOUND;
    }

    ep->pinned_if_handle = hal_if->hal_handle;
    HAL_TRACE_DEBUG("{}: Pinning EP to IF Id:{}",
                    __FUNCTION__, pin_ifid);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a endpoint create request
// TODO: check if EP or any of its IPs exists already
//------------------------------------------------------------------------------
hal_ret_t
endpoint_create (EndpointSpec& spec, EndpointResponse *rsp)
{
    hal_ret_t                       ret = HAL_RET_OK;
    int                             i, num_ips = 0;
    tenant_id_t                     tid;
    hal_handle_t                    if_handle, l2seg_handle;
    ep_t                            *ep = NULL;
    tenant_t                        *tenant = NULL;
    l2seg_t                         *l2seg = NULL;
    if_t                            *hal_if = NULL;
    ep_l3_entry_t                   **l3_entry = NULL;
    ep_ip_entry_t                   **ip_entry = NULL;
    // pd::pd_ep_args_t                pd_ep_args; 
    ep_create_app_ctxt_t            app_ctxt;
    dhl_entry_t                     dhl_entry = { 0 };
    cfg_op_ctxt_t                   cfg_ctxt = { 0 };

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("pi-ep:{}: ep create for id {}", __FUNCTION__, 
                    spec.meta().tenant_id());

    ret = validate_endpoint_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // fetch the tenant information
    tid = spec.meta().tenant_id();
    tenant = tenant_lookup_by_id(tid);
    if (tenant == NULL) {
        ret = HAL_RET_TENANT_NOT_FOUND;
        goto end;
    }

    // fetch the L2 segment information
    l2seg_handle = spec.l2_segment_handle();
    l2seg = find_l2seg_by_handle(l2seg_handle);
    if (l2seg == NULL) {
        ret = HAL_RET_L2SEG_NOT_FOUND;
        goto end;
    }

    // fetch the interface information
    if_handle = spec.interface_handle();
    hal_if = find_if_by_handle(if_handle);
    if (hal_if == NULL) {
        ret = HAL_RET_IF_NOT_FOUND;
        goto end;
    }

    // instantiate EP
    ep = ep_alloc_init();
    if (ep == NULL) {
        HAL_TRACE_ERR("pi-ep:{}:unable to allocate handle/memory ret: {}",
                      __FUNCTION__, ret);
        ret = HAL_RET_OOM;
        goto end;
    }

    // initialize the EP record
    HAL_SPINLOCK_INIT(&ep->slock, PTHREAD_PROCESS_PRIVATE);
    ep->l2_key.l2_segid = l2seg->seg_id;
    MAC_UINT64_TO_ADDR(ep->l2_key.mac_addr, spec.mac_address());
    HAL_TRACE_DEBUG("PI-EP:{}: Seg Id:{}, Mac: {} If: {}", __FUNCTION__, 
                    l2seg->seg_id, 
                    ether_ntoa((struct ether_addr*)(ep->l2_key.mac_addr)),
                    hal_if->if_id);
    ep->l2seg_handle = l2seg_handle;
    ep->if_handle = if_handle;
    ep->tenant_handle = tenant->hal_handle;
    ep->useg_vlan = spec.useg_vlan();
    ep->ep_flags = EP_FLAGS_LEARN_SRC_CFG;
    if (hal_if->if_type == intf::IF_TYPE_ENIC) {
        ep->ep_flags |= EP_FLAGS_LOCAL;
        HAL_TRACE_DEBUG("setting local flag in ep: {}", ep->ep_flags);
    } else {
        ep->ep_flags |= EP_FLAGS_REMOTE;
        HAL_TRACE_DEBUG("setting remote flag in ep: {}", ep->ep_flags);
    }

    // Process Host pinning mode, if enabled.
    ret = pin_endpoint(ep);
    if (ret != HAL_RET_OK) {
        rsp->set_api_status(hal_prepare_rsp(ret));
        goto end;
    }

    // allocate memory for each IP entry in the EP
    num_ips = spec.ip_address_size();
    if (num_ips) {
        l3_entry = (ep_l3_entry_t **)HAL_CALLOC(HAL_MEM_ALLOC_EP,
                                                num_ips * sizeof(ep_l3_entry_t *));
        ip_entry = (ep_ip_entry_t **)HAL_CALLOC(HAL_MEM_ALLOC_EP,
                                                num_ips * sizeof(ep_ip_entry_t *));

        for (i = 0; i < num_ips; i++) {
            l3_entry[i] = (ep_l3_entry_t *)g_hal_state->ep_l3_entry_slab()->alloc();
            if (l3_entry[i] == NULL) {
                ret = HAL_RET_OOM;
                rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
                goto end;
            }

            ip_entry[i] = (ep_ip_entry_t *)g_hal_state->ep_ip_entry_slab()->alloc();
            if (ip_entry[i] == NULL) {
                ret = HAL_RET_OOM;
                rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
                goto end;
            }
        }
    }

    // handle IP address information, if any
    // TODO: check if any IPs are already known to us already !!
    utils::dllist_reset(&ep->ip_list_head);
    for (i = 0; i < num_ips; i++) {
        // add the IP to EP
        utils::dllist_reset(&ip_entry[i]->ep_ip_lentry);
        ip_addr_spec_to_ip_addr(&ip_entry[i]->ip_addr, spec.ip_address(i));
        ip_entry[i]->ip_flags = EP_FLAGS_LEARN_SRC_CFG; 
        ep->ep_flags |= EP_FLAGS_LEARN_SRC_CFG;
        utils::dllist_add(&ep->ip_list_head, &ip_entry[i]->ep_ip_lentry);
    }

    // allocate hal handle id
    ep->hal_handle = hal_handle_alloc(HAL_OBJ_ID_ENDPOINT);
    if (ep->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("pi-ep:{}: failed to alloc handle", 
                      __FUNCTION__);
        ret =HAL_RET_HANDLE_INVALID;
        goto end;
    }

    // form ctxt and call infra add
    app_ctxt.tenant = tenant;
    app_ctxt.l2seg = l2seg;
    app_ctxt.hal_if = hal_if;

    dhl_entry.handle = ep->hal_handle;
    dhl_entry.obj = ep;
    cfg_ctxt.app_ctxt = &app_ctxt;
    utils::dllist_reset(&cfg_ctxt.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(ep->hal_handle, &cfg_ctxt, 
                             endpoint_create_add_cb,
                             endpoint_create_commit_cb,
                             endpoint_create_abort_cb, 
                             endpoint_create_cleanup_cb);







#if 0

    // handle is allocated only after PD programming is SUCCESS
    ep->hal_handle = hal_alloc_handle();
    ep->hal_handle_ht_ctxt.reset();
    g_hal_state->ep_hal_handle_ht()->insert(ep, &ep->hal_handle_ht_ctxt);

    // add L2 lookup entry for this EP with (L2SEG, MAC) key
    ep->l2key_ht_ctxt.reset();
    g_hal_state->ep_l2_ht()->insert(ep, &ep->l2key_ht_ctxt);

    // insert this EP in the tenant's EP list
    utils::dllist_reset(&ep->tenant_ep_lentry);
    HAL_SPINLOCK_LOCK(&tenant->slock);
    utils::dllist_add(&tenant->ep_list_head, &ep->tenant_ep_lentry);
    HAL_SPINLOCK_UNLOCK(&tenant->slock);

    // insert this EP in the L2 segment's EP list
    utils::dllist_reset(&ep->l2seg_ep_lentry);
    HAL_SPINLOCK_LOCK(&l2seg->slock);
    utils::dllist_add(&l2seg->ep_list_head, &ep->l2seg_ep_lentry);
    HAL_SPINLOCK_UNLOCK(&l2seg->slock);

    // insert this EP in the interface' EP list
    utils::dllist_reset(&ep->if_ep_lentry);
    HAL_SPINLOCK_LOCK(&hal_if->slock);
    utils::dllist_add(&hal_if->ep_list_head, &ep->if_ep_lentry);
    HAL_SPINLOCK_UNLOCK(&hal_if->slock);
#endif
#if 0
    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_endpoint_status()->set_endpoint_handle(ep->hal_handle);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return HAL_RET_OK;
#endif

    // initialize session list
    // utils::dllist_reset(&hal_if->session_list_head);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_endpoint_status()->set_endpoint_handle(ep->hal_handle);

end:

    if (ret != HAL_RET_OK) {
        if (ep) {
            ep_free(ep);
            ep = NULL;
        }
    }

    ep_prepare_rsp(rsp, ret, ep ? ep->hal_handle : 0);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate an incoming endpoint update request
//------------------------------------------------------------------------------
static hal_ret_t
validate_endpoint_update (EndpointUpdateRequest& req, EndpointResponse *rsp)
{
    hal_ret_t       ret = HAL_RET_OK;

    // Check if tenant id is valid
    if (!req.has_meta() ||
        req.meta().tenant_id() == HAL_TENANT_ID_INVALID) {
        HAL_TRACE_ERR("pi-ep:{}:tenant id invalid", __FUNCTION__);
        ret = HAL_RET_TENANT_ID_INVALID;
        goto end;
    }

    // Check if ep's key or handle is passed
    if (!req.has_key_or_handle()) {
        ret = HAL_RET_INVALID_ARG;
        HAL_TRACE_ERR("pi-ep:{}:spec has no key or handle", __FUNCTION__);
        goto end;
    }

end:
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
    pd::pd_ep_upd_args_t            pd_ep_args = { 0 };
    dllist_ctxt_t                   *lnode = NULL;
    dhl_entry_t                     *dhl_entry = NULL;
    ep_t                            *ep = NULL/*, *ep_clone = NULL*/;
    ep_update_app_ctxt_t            *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-ep{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (ep_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    ep = (ep_t *)dhl_entry->obj;
    // ep_clone = (ep_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("pi-ep:{}: update upd cb ", __FUNCTION__);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_ep_upd_args_init(&pd_ep_args);
    pd_ep_args.ep = ep;
    pd_ep_args.iplist_change = app_ctxt->iplist_change;
    pd_ep_args.add_iplist = app_ctxt->add_iplist;
    pd_ep_args.del_iplist = app_ctxt->del_iplist;
    ret = pd::pd_ep_update(&pd_ep_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-ep:{}:failed to update ep pd, err : {}",
                      __FUNCTION__, ret);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// Make a clone
// - Both PI and PD objects cloned. 
//------------------------------------------------------------------------------
hal_ret_t
ep_make_clone (ep_t *ep, ep_t **ep_clone)
{
    *ep_clone = ep_alloc_init();
    memcpy(*ep_clone, ep, sizeof(ep_t));

    pd::pd_ep_make_clone(ep, *ep_clone);

    return HAL_RET_OK;
}

hal_ret_t
ep_copy_ip_list (ep_t *dst_ep, ep_t *src_ep)
{
    dllist_ctxt_t   *curr, *next;
    ep_ip_entry_t   *pi_ip_entry = NULL;

    HAL_TRACE_DEBUG("pi-ep:{}:Before copy EP's IPs:");
    ep_print_ips(src_ep);

    utils::dllist_reset(&dst_ep->ip_list_head);

    dllist_for_each_safe(curr, next, &(src_ep->ip_list_head)) {
        pi_ip_entry = dllist_entry(curr, ep_ip_entry_t, ep_ip_lentry);
        HAL_TRACE_DEBUG("PI-EP:{}: Copying to clone ip: {}", 
                __FUNCTION__, ipaddr2str(&(pi_ip_entry->ip_addr)));

        utils::dllist_del(&pi_ip_entry->ep_ip_lentry);

        utils::dllist_add(&dst_ep->ip_list_head, &pi_ip_entry->ep_ip_lentry);
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// After all hw programming is done
//  1. Free original PI & PD nwsec.
// Note: Infra make clone as original by replacing original pointer by clone.
//------------------------------------------------------------------------------
hal_ret_t
endpoint_update_commit_cb(cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_ep_args_t            pd_ep_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    ep_t                        *ep = NULL, *ep_clone = NULL;
    ep_update_app_ctxt_t        *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-ep{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (ep_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    ep = (ep_t *)dhl_entry->obj;
    ep_clone = (ep_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("pi-ep:{}:update commit CB",
                    __FUNCTION__);

    // Update PI structures
    ep_copy_ip_list(ep_clone, ep);
    endpoint_update_pi_with_iplist(ep_clone, app_ctxt->add_iplist,
                                   app_ctxt->del_iplist);

    // Free PD
    pd::pd_ep_args_init(&pd_ep_args);
    pd_ep_args.ep = ep;
    ret = pd::pd_ep_mem_free(&pd_ep_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-ep:{}:failed to delete ep pd, err : {}",
                      __FUNCTION__, ret);
    }

    // Free PI
    ep_free(ep);
end:
    HAL_TRACE_DEBUG("pi-ep:{}:Clone EP's IPs:");
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
    pd::pd_ep_args_t            pd_ep_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    ep_t                        *ep = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-ep{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    // assign clone as we are trying to free only the clone
    ep = (ep_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("pi-ep:{}:update commit CB",
                    __FUNCTION__);

    // Free PD
    pd::pd_ep_args_init(&pd_ep_args);
    pd_ep_args.ep = ep;
    ret = pd::pd_ep_mem_free(&pd_ep_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-ep:{}:failed to delete ep pd, err : {}",
                      __FUNCTION__, ret);
    }

    // Free PI
    ep_free(ep);
end:

    return ret;
}

hal_ret_t
endpoint_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Generic function to fetch endpoint from key or handle
//------------------------------------------------------------------------------
hal_ret_t
fetch_endpoint(tenant_id_t tid, EndpointKeyHandle kh, ep_t **ep, 
               ::types::ApiStatus *api_status)
{
    ep_l3_key_t            l3_key = { 0 };
    mac_addr_t             mac_addr = { 0 };

    if (kh.key_or_handle_case() == EndpointKeyHandle::kEndpointKey) {
        auto ep_key = kh.endpoint_key();
        if (ep_key.has_l2_key()) {
            auto ep_l2_key = ep_key.l2_key();
            MAC_UINT64_TO_ADDR(mac_addr, ep_l2_key.mac_address());
            *ep = find_ep_by_l2_key(ep_l2_key.l2_segment_handle(), mac_addr);
        } else if (ep_key.has_l3_key()) {
            auto ep_l3_key = ep_key.l3_key();
            l3_key.tenant_id = tid;
            ip_addr_spec_to_ip_addr(&l3_key.ip_addr,
                    ep_l3_key.ip_address());
            *ep = find_ep_by_l3_key(&l3_key);
        } else {
            *api_status = types::API_STATUS_INVALID_ARG;
            return HAL_RET_INVALID_ARG;
        }
    } else if (kh.key_or_handle_case() ==
            EndpointKeyHandle::kEndpointHandle) {
        *ep = find_ep_by_handle(kh.endpoint_handle());
    } else {
        *api_status = types::API_STATUS_INVALID_ARG;
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

hal_ret_t
endpoint_if_update(EndpointUpdateRequest& req, ep_t *ep, 
                   bool *if_change, hal_handle_t *new_if_hdl) 
{
    *if_change = false;

    if (ep->if_handle != req.interface_handle()) {
        *if_change = true;
        *new_if_hdl = req.interface_handle();
    }

    return HAL_RET_OK;
}

hal_ret_t
endpoint_l2seg_update(EndpointUpdateRequest& req, ep_t *ep, 
                      bool *l2seg_change, hal_handle_t *new_l2seg_hdl) 
{
    *l2seg_change = false;

    if (ep->l2seg_handle != req.l2_segment_handle()) {
        *l2seg_change = true;
        *new_l2seg_hdl = req.l2_segment_handle();
    }

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

hal_ret_t
endpoint_ip_list_update(EndpointUpdateRequest& req, ep_t *ep,
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

    utils::dllist_reset(*add_iplist);
    utils::dllist_reset(*del_iplist);

    num_ips = req.ip_address_size();
    HAL_TRACE_DEBUG("pi-ep:{}:Before Checking for added IPs. Num:{}", 
                    __FUNCTION__, num_ips);
    ep_print_ips(ep);
    for (i = 0; i < num_ips; i++) {
        ip_addr_spec_to_ip_addr(&ip_addr, req.ip_address(i));
        if (ip_in_ep(&ip_addr, ep, NULL)) {
            continue;
        } else {
            // Create ep_ip_entry and add it to add list
            ep_ipe = (ep_ip_entry_t *)g_hal_state->ep_ip_entry_slab()->alloc();
            HAL_ABORT(ep_ipe != NULL);
            memcpy(&ep_ipe->ip_addr, &ip_addr, sizeof(ip_addr_t));
            ep_ipe->ip_flags = EP_FLAGS_LEARN_SRC_CFG;
            ep_ipe->pd = NULL;
            utils::dllist_reset(&ep_ipe->ep_ip_lentry);
            utils::dllist_add(*add_iplist, &ep_ipe->ep_ip_lentry);
            *iplist_change = true;
        }
    }

    HAL_TRACE_DEBUG("pi-ep:{}:Done Checking for added IPs", __FUNCTION__);
    ep_print_ips(ep);
    HAL_TRACE_DEBUG("pi-ep:{}:Checking for deleted IPs", __FUNCTION__);

    // lnode = ep->ip_list_head.next;
    dllist_for_each(lnode, &(ep->ip_list_head)) {
        pi_ip_entry = dllist_entry(lnode, ep_ip_entry_t, ep_ip_lentry);
        HAL_TRACE_DEBUG("PI-EP-Update:{}: Checking for ip: {}", 
                __FUNCTION__, ipaddr2str(&(pi_ip_entry->ip_addr)));
        for (i = 0; i < num_ips; i++) {
            ip_addr_spec_to_ip_addr(&ip_addr, req.ip_address(i));
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
            utils::dllist_reset(&ep_ipe->ep_ip_lentry);
            utils::dllist_add(*del_iplist, &ep_ipe->ep_ip_lentry);
            *iplist_change = true;
            HAL_TRACE_DEBUG("PI-EP-Update:{}: Added to delete list ip: {}", 
                    __FUNCTION__, ipaddr2str(&(ep_ipe->ip_addr)));
        }
        ip_exists = false;
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
    // ep_l3_entry_t   *l3_entry = NULL;
    ep_l3_key_t     l3_key = { 0 };
    tenant_t        *tenant = NULL;

    tenant = tenant_lookup_by_handle(ep->tenant_handle);
    if (tenant == NULL) {
        HAL_TRACE_ERR("pi-ep:{}:unable to find tenant", __FUNCTION__);
        ret = HAL_RET_TENANT_NOT_FOUND;
        goto end;
    }

    // Handling new IPs
    dllist_for_each_safe(curr, next, add_iplist) {
        pi_ip_entry = dllist_entry(curr, ep_ip_entry_t, ep_ip_lentry);
        HAL_TRACE_DEBUG("PI-EP-Update:{}: Adding new IP {}", 
                __FUNCTION__, ipaddr2str(&(pi_ip_entry->ip_addr)));

        utils::dllist_del(&pi_ip_entry->ep_ip_lentry);

        // Insert into EP's ip list 
        utils::dllist_add(&ep->ip_list_head, &pi_ip_entry->ep_ip_lentry);

        // Insert to L3 hash table with (VRF, IP) key
        l3_key.tenant_id = tenant->tenant_id;
        l3_key.ip_addr = pi_ip_entry->ip_addr;
        ret = ep_add_to_l3_db(&l3_key, pi_ip_entry, ep->hal_handle);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-ep:{}:unable to add EP:{} to L3 DB", 
                          __FUNCTION__, ep_l2_key_to_str(ep));
            goto end;
        }
        HAL_TRACE_DEBUG("pi-ep:{}:added EP ({}, {}) to L3 DB",
                        __FUNCTION__,
                        l3_key.tenant_id,
                        ipaddr2str(&l3_key.ip_addr));
    

#if 0
        l3_entry = (ep_l3_entry_t *)HAL_CALLOC(HAL_MEM_ALLOC_EP,
                sizeof(ep_l3_entry_t));
        l3_entry->l3_key.tenant_id = tenant->tenant_id;
        memcpy(&l3_entry->l3_key.ip_addr, &pi_ip_entry->ip_addr, sizeof(ip_addr_t));
        l3_entry->ep = ep;
        l3_entry->ep_ip = pi_ip_entry;
        l3_entry->ep_l3_ht_ctxt.reset();
        g_hal_state->ep_l3_entry_ht()->insert(l3_entry,
                                              &l3_entry->ep_l3_ht_ctxt);
        HAL_TRACE_DEBUG("Added ({}, {}) to DB",
                        l3_entry->l3_key.tenant_id,
                        ipaddr2str(&l3_entry->l3_key.ip_addr));
#endif
    }


    HAL_TRACE_DEBUG("EP's IPs:");
    ep_print_ips(ep);

    // Handling removed IPs
    dllist_for_each_safe(curr, next, del_iplist) {
        pi_ip_entry = dllist_entry(curr, ep_ip_entry_t, ep_ip_lentry);
        HAL_TRACE_DEBUG("PI-EP-Update:{}: Deleting IP {}", 
                __FUNCTION__, ipaddr2str(&(pi_ip_entry->ip_addr)));

        utils::dllist_del(&pi_ip_entry->ep_ip_lentry);

        if (ip_in_ep(&pi_ip_entry->ip_addr, ep, &del_ip_entry)) {
            utils::dllist_del(&del_ip_entry->ep_ip_lentry);

            // Remove from hash table
            l3_key.tenant_id = tenant->tenant_id;
            l3_key.ip_addr = pi_ip_entry->ip_addr;
            ret = ep_del_from_l3_db(&l3_key);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("pi-ep:{}:unable to del EP:{} from L3 DB", 
                              __FUNCTION__, ep_l2_key_to_str(ep));
                goto end;
            }
            HAL_TRACE_DEBUG("pi-ep:{}:deleted EP ({}, {}) from L3 DB",
                            __FUNCTION__,
                            l3_key.tenant_id,
                            ipaddr2str(&l3_key.ip_addr));

            // Free IP entry
            g_hal_state->ep_ip_entry_slab()->free(del_ip_entry);

            // Free IP entry created for delete
            g_hal_state->ep_ip_entry_slab()->free(pi_ip_entry);
#if 0
            l3_key.tenant_id = ep->tenant_id;
            memcpy(&l3_key.ip_addr, &del_ip_entry->ip_addr, sizeof(ip_addr_t));
            l3_entry = (ep_l3_entry_t *)g_hal_state->ep_l3_entry_ht()->remove(&l3_key);
            HAL_TRACE_DEBUG("Removed ({}, {}) from DB",
                            l3_key.tenant_id,
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
    hal_ret_t               ret = HAL_RET_OK;
    tenant_id_t             tid;
    ep_t                    *ep = NULL;
    tenant_t                *tenant = NULL;
    ApiStatus               api_status;
    bool                    if_change = false, l2seg_change = false, 
                            iplist_change = false;
    hal_handle_t            new_if_hdl = 0, new_l2seg_hdl = 0;
    dllist_ctxt_t           *add_iplist = NULL, *del_iplist = NULL;
    // pd::pd_ep_upd_args_t    pd_ep_upd_args;
    cfg_op_ctxt_t           cfg_ctxt = { 0 };
    dhl_entry_t             dhl_entry = { 0 };
    ep_update_app_ctxt_t    app_ctxt = { 0 };

    HAL_TRACE_DEBUG("--------------   EP Update API Start  ------------------");
    ret = validate_endpoint_update(req, rsp);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // fetch the tenant information
    tid = req.meta().tenant_id();
    tenant = tenant_lookup_by_id(tid);
    if (tenant == NULL) {
        HAL_TRACE_ERR("pi-ep:{}:failed to find tenant for tid:{}", 
                      __FUNCTION__, tid);
        ret = HAL_RET_TENANT_NOT_FOUND;
        goto end;
    }

    // fetch the ep
    ret = fetch_endpoint(tid, req.key_or_handle(), &ep, &api_status);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-ep:{}:failed to fetch endpoint",
                      __FUNCTION__);
        rsp->set_api_status(api_status);
        goto end;
    }

    // check for if change
    ret = endpoint_if_update(req, ep, &if_change, &new_if_hdl);
    HAL_ABORT(ret == HAL_RET_OK);

    // check for l2seg change
    ret = endpoint_l2seg_update(req, ep, &l2seg_change, &new_l2seg_hdl);
    HAL_ABORT(ret == HAL_RET_OK);

    // check for ip change
    ret = endpoint_ip_list_update(req, ep, &iplist_change, &add_iplist, &del_iplist);
    HAL_ABORT(ret == HAL_RET_OK);


    if (!if_change && !l2seg_change && !iplist_change) {
        HAL_TRACE_ERR("pi-ep:{}:no change in ep update: noop", __FUNCTION__);
        goto end;
    }

    if (if_change) {
        // call actions
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
    dhl_entry.handle = ep->hal_handle;
    dhl_entry.obj = ep;
    cfg_ctxt.app_ctxt = &app_ctxt;
    utils::dllist_reset(&cfg_ctxt.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
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

    ep_prepare_rsp(rsp, ret, ep ? ep->hal_handle : 0);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

//------------------------------------------------------------------------------
// validate ep delete request
//------------------------------------------------------------------------------
static hal_ret_t
validate_endpoint_delete (EndpointDeleteRequest& req,
                          EndpointDeleteResponseMsg *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("pi-ep:{}:spec has no key or handle", __FUNCTION__);
        ret =  HAL_RET_INVALID_ARG;
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
    pd::pd_ep_args_t            pd_ep_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    ep_t                        *ep = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-ep:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // TODO: Check the dependency ref count for the ep. 
    //       If its non zero, fail the delete.


    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    ep = (ep_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-ep:{}:delete del CB",
                    __FUNCTION__);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_ep_args_init(&pd_ep_args);
    pd_ep_args.ep = ep;
    ret = pd::pd_ep_delete(&pd_ep_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-ep:{}:failed to delete ep pd, err : {}", 
                      __FUNCTION__, ret);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// Update PI DBs as tenant_delete_del_cb() was a succcess
//      a. Delete from tenant id hash table
//      b. Remove object from handle id based hash table
//      c. Free PI tenant
//------------------------------------------------------------------------------
hal_ret_t
endpoint_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    ep_t                        *ep = NULL;
    hal_handle_t                hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-ep:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    ep = (ep_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-ep:{}:delete commit CB",
                    __FUNCTION__);

    ret = endpoint_cleanup(ep);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-ep:{}:failed to del ep from db, err : {}", 
                      __FUNCTION__, ret);
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
                 EndpointDeleteResponseMsg *rsp)
{
    hal_ret_t                       ret = HAL_RET_OK;
    tenant_id_t                     tid;
    ep_t                            *ep = NULL;
    tenant_t                        *tenant = NULL;
    cfg_op_ctxt_t                   cfg_ctxt = { 0 };
    dhl_entry_t                     dhl_entry = { 0 };
    ApiStatus                       api_status;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");

    // validate the request message
    ret = validate_endpoint_delete(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-nwsec:{}:nwsec delete validation failed, ret : {}",
                      __FUNCTION__, ret);
        goto end;
    }

    // fetch the tenant information
    tid = req.meta().tenant_id();
    tenant = tenant_lookup_by_id(tid);
    if (tenant == NULL) {
        HAL_TRACE_ERR("pi-ep:{}:failed to find tenant for tid:{}", 
                      __FUNCTION__, tid);
        ret = HAL_RET_TENANT_NOT_FOUND;
        // rsp->set_api_status(types::API_STATUS_TENANT_NOT_FOUND);
        goto end;
    }

    // fetch the ep
    ret = fetch_endpoint(tid, req.key_or_handle(), &ep, &api_status);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-ep:{}:failed to find EP", __FUNCTION__);
        goto end;
    }

    HAL_TRACE_DEBUG("pi-ep:{}:deleting ep:{}", 
                    __FUNCTION__, ep_l2_key_to_str(ep)); 

    // form ctxt and call infra add
    dhl_entry.handle = ep->hal_handle;
    dhl_entry.obj = ep;
    cfg_ctxt.app_ctxt = NULL;
    utils::dllist_reset(&cfg_ctxt.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(ep->hal_handle, &cfg_ctxt, 
                             endpoint_delete_del_cb,
                             endpoint_delete_commit_cb,
                             endpoint_delete_abort_cb, 
                             endpoint_delete_cleanup_cb);

end:
    rsp->add_api_status(hal_prepare_rsp(ret));
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}



static void
ep_to_ep_get_response (ep_t *ep, EndpointGetResponse *response)
{
    response->mutable_spec()->mutable_meta()->set_tenant_id(tenant_lookup_by_handle(ep->tenant_handle)->tenant_id);
    response->mutable_spec()->set_l2_segment_handle(ep->l2seg_handle);
    response->mutable_spec()->set_mac_address(MAC_TO_UINT64(ep->l2_key.mac_addr));
    response->mutable_spec()->set_interface_handle(ep->if_handle);
    response->mutable_spec()->set_useg_vlan(ep->useg_vlan);

    response->mutable_status()->set_endpoint_handle(ep->hal_handle);
    response->mutable_status()->set_learn_source_dhcp(ep->ep_flags & EP_FLAGS_LEARN_SRC_DHCP);
    response->mutable_status()->set_learn_source_arp(ep->ep_flags & EP_FLAGS_LEARN_SRC_ARP);
    response->mutable_status()->set_learn_source_rarp(ep->ep_flags & EP_FLAGS_LEARN_SRC_RARP);
    response->mutable_status()->set_learn_source_config(ep->ep_flags & EP_FLAGS_LEARN_SRC_CFG);
    response->mutable_status()->set_is_endpoint_local(ep->ep_flags & EP_FLAGS_LOCAL);
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

    response = rsp->add_response();
    if (!req.has_meta() ||
        req.meta().tenant_id() == HAL_TENANT_ID_INVALID) {
        rsp->set_api_status(types::API_STATUS_TENANT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    if (req.has_key_or_handle()) {
        auto kh = req.key_or_handle();
        if (kh.key_or_handle_case() == EndpointKeyHandle::kEndpointKey) {
            auto ep_key = kh.endpoint_key();
            if (ep_key.has_l2_key()) {
                auto ep_l2_key = ep_key.l2_key();
                MAC_UINT64_TO_ADDR(mac_addr, ep_l2_key.mac_address());
                ep = find_ep_by_l2_key(ep_l2_key.l2_segment_handle(), mac_addr);
            } else if (ep_key.has_l3_key()) {
                auto ep_l3_key = ep_key.l3_key();
                l3_key.tenant_id = req.meta().tenant_id();
                ip_addr_spec_to_ip_addr(&l3_key.ip_addr,
                                        ep_l3_key.ip_address());
                ep = find_ep_by_l3_key(&l3_key);
            } else {
                rsp->set_api_status(types::API_STATUS_INVALID_ARG);
                return HAL_RET_INVALID_ARG;
            }
        } else if (kh.key_or_handle_case() ==
                       EndpointKeyHandle::kEndpointHandle) {
            ep = find_ep_by_handle(kh.endpoint_handle());
        } else {
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            return HAL_RET_INVALID_ARG;
        }
    } else {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    if (ep == NULL) {
        rsp->set_api_status(types::API_STATUS_ENDPOINT_NOT_FOUND);
        return HAL_RET_EP_NOT_FOUND;
    }

    ep_to_ep_get_response(ep, response);
    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

const char *
ep_l2_key_to_str(ep_t *ep)
{
    static thread_local char       ep_str[4][50];
    static thread_local uint8_t    ep_str_next = 0;
    char                           *buf;

    buf = ep_str[ep_str_next++ & 0x3];
    memset(buf, 0, 50);
    if (ep) {
        snprintf(buf, 50, "%d::%s", ep->l2_key.l2_segid,
                ether_ntoa((struct ether_addr*)(ep->l2_key.mac_addr)));
    }
    return buf;
}

void
ep_print_ips(ep_t *ep)
{
    dllist_ctxt_t   *lnode = NULL;
    ep_ip_entry_t   *pi_ip_entry = NULL;

    dllist_for_each(lnode, &(ep->ip_list_head)) {
        pi_ip_entry = dllist_entry(lnode, ep_ip_entry_t, ep_ip_lentry);
        HAL_TRACE_DEBUG("PI-EP:{}: ip: {}", 
                __FUNCTION__, ipaddr2str(&(pi_ip_entry->ip_addr)));
    }
}


}    // namespace hal

