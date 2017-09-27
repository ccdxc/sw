#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/utils.hpp"
// #include <endpoint_svc.hpp>
#include "nic/hal/src/endpoint.hpp"
#include "nic/include/pd_api.hpp"
#include <netinet/ether.h>

namespace hal {

void *
ep_get_l2_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((ep_t *)entry)->l2_key);
}

uint32_t
ep_compute_l2_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(ep_l2_key_t)) % ht_size;
}

bool
ep_compare_l2_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    HAL_TRACE_DEBUG("key1.l2_segid {}, key2.l2_segid {}",
                    ((ep_l2_key_t *)key1)->l2_segid,
                    ((ep_l2_key_t *)key2)->l2_segid);
    HAL_TRACE_DEBUG("key1.mac {}, key2.mac {}",
                    macaddr2str(((ep_l2_key_t *)key1)->mac_addr),
                    macaddr2str(((ep_l2_key_t *)key2)->mac_addr));

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
    // TODO: remove
    HAL_TRACE_DEBUG("L3 key hash {}",
                    utils::hash_algo::fnv_hash(key, sizeof(ep_l3_key_t)) % ht_size);
    return utils::hash_algo::fnv_hash(key, sizeof(ep_l3_key_t)) % ht_size;
}

bool
ep_compare_l3_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));

    // TODO: remove
    HAL_TRACE_DEBUG("key1.tid {}, key2.tid {}",
                    ((ep_l3_key_t *)key1)->tenant_id,
                    ((ep_l3_key_t *)key2)->tenant_id);
    HAL_TRACE_DEBUG("key1.ip {}, key2.ip {}",
                    ipaddr2str(&((ep_l3_key_t *)key1)->ip_addr),
                    ipaddr2str(&((ep_l3_key_t *)key2)->ip_addr));

    if (!memcmp(key1, key2, sizeof(ep_l3_key_t))) {
        return true;
    }
    return false;
}

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

//------------------------------------------------------------------------------
// validate an incoming endpoint create request
//------------------------------------------------------------------------------
static hal_ret_t
validate_endpoint_create (EndpointSpec& spec, EndpointResponse *rsp)
{
    if (!spec.has_meta() ||
        spec.meta().tenant_id() == HAL_TENANT_ID_INVALID) {
        rsp->set_api_status(types::API_STATUS_TENANT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    if (spec.l2_segment_handle() == HAL_HANDLE_INVALID) {
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        return HAL_RET_HANDLE_INVALID;
    }

    if (spec.interface_handle() == HAL_HANDLE_INVALID) {
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        return HAL_RET_HANDLE_INVALID;
    }

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
        rsp->set_api_status(types::API_STATUS_TENANT_ID_INVALID);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // Check if ep's key or handle is passed
    if (!req.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_ENDPOINT_UPD_KEY_HDL_INVALID);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

#if 0
    if (req.has_key_or_handle()) {
        if (req.key_or_handle().has_endpoint_key()) {
            // Key
        } else {
            // Handle
        }
    } else {
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        ret = HAL_RET_INVALID_ARG;
    }
#endif


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
    hal_ret_t          ret;
    int                i, num_ips;
    tenant_id_t        tid;
    hal_handle_t       if_handle, l2seg_handle;
    ep_t               *ep = NULL;
    tenant_t           *tenant = NULL;
    l2seg_t            *l2seg = NULL;
    if_t               *hal_if = NULL;
    ep_l3_entry_t      **l3_entry = NULL;
    ep_ip_entry_t      **ip_entry = NULL;
    pd::pd_ep_args_t   pd_ep_args; 

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-EP:{}: EP Create for id {}", __FUNCTION__, 
                    spec.meta().tenant_id());

    ret = validate_endpoint_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // fetch the tenant information
    tid = spec.meta().tenant_id();
    tenant = tenant_lookup_by_id(tid);
    if (tenant == NULL) {
        ret = HAL_RET_INVALID_ARG;
        rsp->set_api_status(types::API_STATUS_TENANT_NOT_FOUND);
        goto end;
    }


    // fetch the L2 segment information
    l2seg_handle = spec.l2_segment_handle();
    l2seg = find_l2seg_by_handle(l2seg_handle);
    if (l2seg == NULL) {
        ret = HAL_RET_INVALID_ARG;
        rsp->set_api_status(types::API_STATUS_L2_SEGMENT_NOT_FOUND);
        goto end;
    }

    // fetch the interface information
    if_handle = spec.interface_handle();
    hal_if = find_if_by_handle(if_handle);
    if (hal_if == NULL) {
        ret = HAL_RET_INVALID_ARG;
        HAL_TRACE_ERR("{}: Unknown if handle {}", __func__, if_handle);
        rsp->set_api_status(types::API_STATUS_INTERFACE_NOT_FOUND);
        goto end;
    }

    // allocate memory for the EP record
    ep = (ep_t *)g_hal_state->ep_slab()->alloc();
    if (ep == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_OOM;
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
    ep->tenant_id = tid;
    ep->useg_vlan = spec.useg_vlan();
    ep->ep_flags = EP_FLAGS_LEARN_SRC_CFG;
    if (hal_if->if_type == intf::IF_TYPE_ENIC) {
        ep->ep_flags |= EP_FLAGS_LOCAL;
        HAL_TRACE_DEBUG("Setting local flag in ep: {}", ep->ep_flags);
    } else {
        ep->ep_flags |= EP_FLAGS_REMOTE;
        HAL_TRACE_DEBUG("Setting remote flag in ep: {}", ep->ep_flags);
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

        // add a hash table lookup entry for this EP with (VRF, IP) key
        l3_entry[i]->l3_key.tenant_id = ep->tenant_id;
        l3_entry[i]->l3_key.ip_addr = ip_entry[i]->ip_addr;
        l3_entry[i]->ep = ep;
        l3_entry[i]->ep_ip = ip_entry[i];
        l3_entry[i]->ep_l3_ht_ctxt.reset();
        g_hal_state->ep_l3_entry_ht()->insert(l3_entry[i],
                                              &l3_entry[i]->ep_l3_ht_ctxt);
        HAL_TRACE_DEBUG("Added ({}, {}) to EP",
                        l3_entry[i]->l3_key.tenant_id,
                        ipaddr2str(&l3_entry[i]->l3_key.ip_addr));
    }


    // PD Call
    pd::pd_ep_args_init(&pd_ep_args);
    pd_ep_args.tenant = tenant;
    pd_ep_args.l2seg = l2seg;
    pd_ep_args.intf = hal_if;
    pd_ep_args.ep = ep;
    ret = pd::pd_ep_create(&pd_ep_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD EP create failure, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

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

    // initialize session list
    utils::dllist_reset(&hal_if->session_list_head);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_endpoint_status()->set_endpoint_handle(ep->hal_handle);

end:

    if (ret != HAL_RET_OK) {
        if (ep) {
            g_hal_state->ep_slab()->free(ep);
        }

        if (l3_entry) {
            for (i = 0; i < num_ips; i++)  {
                if (l3_entry[i]) {
                    g_hal_state->ep_l3_entry_slab()->free(l3_entry[i]);
                } else {
                    break;
                }
            }
            HAL_FREE(HAL_MEM_ALLOC_EP, l3_entry);
        }

        if (ip_entry) {
            for (i = 0; i < num_ips; i++)  {
                if (ip_entry[i]) {
                    g_hal_state->ep_ip_entry_slab()->free(ip_entry[i]);
                } else {
                    break;
                }
            }
            HAL_FREE(HAL_MEM_ALLOC_EP, ip_entry);
        }
    }

    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
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
    ep_l3_entry_t   *l3_entry = NULL;
    ep_l3_key_t     l3_key = { 0 };

    // Handling new IPs
    dllist_for_each_safe(curr, next, add_iplist) {
        pi_ip_entry = dllist_entry(curr, ep_ip_entry_t, ep_ip_lentry);
        HAL_TRACE_DEBUG("PI-EP-Update:{}: Adding new IP {}", 
                __FUNCTION__, ipaddr2str(&(pi_ip_entry->ip_addr)));

        utils::dllist_del(&pi_ip_entry->ep_ip_lentry);

        // Insert into EP's ip list 
        utils::dllist_add(&ep->ip_list_head, &pi_ip_entry->ep_ip_lentry);

        // Insert to L3 hash table with (VRF, IP) key
        l3_entry = (ep_l3_entry_t *)HAL_CALLOC(HAL_MEM_ALLOC_EP,
                sizeof(ep_l3_entry_t));
        l3_entry->l3_key.tenant_id = ep->tenant_id;
        memcpy(&l3_entry->l3_key.ip_addr, &pi_ip_entry->ip_addr, sizeof(ip_addr_t));
        l3_entry->ep = ep;
        l3_entry->ep_ip = pi_ip_entry;
        l3_entry->ep_l3_ht_ctxt.reset();
        g_hal_state->ep_l3_entry_ht()->insert(l3_entry,
                                              &l3_entry->ep_l3_ht_ctxt);
        HAL_TRACE_DEBUG("Added ({}, {}) to DB",
                        l3_entry->l3_key.tenant_id,
                        ipaddr2str(&l3_entry->l3_key.ip_addr));
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

        } else {
            // IPs which are getting deleted should be present in PI EP
            HAL_ASSERT(0);
        }
    }

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
    pd::pd_ep_upd_args_t    pd_ep_upd_args;

    HAL_TRACE_DEBUG("--------------   EP Update API Start  ------------------");
    ret = validate_endpoint_update(req, rsp);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // fetch the tenant information
    tid = req.meta().tenant_id();
    tenant = tenant_lookup_by_id(tid);
    if (tenant == NULL) {
        ret = HAL_RET_INVALID_ARG;
        rsp->set_api_status(types::API_STATUS_TENANT_NOT_FOUND);
        goto end;
    }

    // fetch the ep
    ret = fetch_endpoint(tid, req.key_or_handle(), &ep, &api_status);
    if (ret != HAL_RET_OK) {
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

    // Initializing PD structure
    pd::pd_ep_upd_args_init(&pd_ep_upd_args);
    pd_ep_upd_args.ep = ep;

    if (if_change) {
        // call actions
    }

    if (l2seg_change) {
        // call actions
    }

    if (iplist_change) {
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
    }

    if (add_iplist) {
        HAL_FREE(HAL_MEM_ALLOC_DLLIST, add_iplist);
    }
    if (del_iplist) {
        HAL_FREE(HAL_MEM_ALLOC_DLLIST, del_iplist);
    }

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_endpoint_status()->set_endpoint_handle(ep->hal_handle);
    
end:
    HAL_TRACE_DEBUG("--------------   EP Update API End   -------------------");
    return ret;
}

static void
ep_to_ep_get_response (ep_t *ep, EndpointGetResponse *response)
{
    response->mutable_spec()->mutable_meta()->set_tenant_id(ep->tenant_id);
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

