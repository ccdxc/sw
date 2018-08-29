//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// Handles CRUD APIs for all filters
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/plugins/cfg/nw/filter.hpp"
#include "nic/hal/plugins/cfg/lif/lif.hpp"
#include "nic/hal/plugins/cfg/lif/lif_api.hpp"
#include "nic/hal/src/utils/utils.hpp"
#include "nic/hal/src/utils/if_utils.hpp"
#include "nic/gen/hal/include/hal_api_stats.hpp"

namespace hal {

// Proto types
hal_ret_t validate_filter_create (FilterSpec& spec, FilterResponse *rsp);
filter_t *find_filter_by_handle (hal_handle_t handle);
const char *filter_keyhandle_to_str (filter_t *filter);
filter_t *filter_lookup_by_key (filter_key_t *key);
static inline filter_t *filter_alloc_init (void);
static inline hal_ret_t filter_cleanup (filter_t *filter);
hal_ret_t filter_init_from_spec (filter_t *filter, const FilterSpec& spec);
hal_ret_t filter_prepare_rsp (FilterResponse *rsp, hal_ret_t ret, hal_handle_t hal_handle);
static hal_ret_t filter_create_add_cb (cfg_op_ctxt_t *cfg_ctxt);
static hal_ret_t filter_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt);
static hal_ret_t filter_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt);
static hal_ret_t filter_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt);
static inline hal_ret_t filter_add_to_db (filter_t *filter, hal_handle_t handle);
hal_ret_t validate_filter_delete_req (FilterDeleteRequest& req,
                                      FilterDeleteResponse *rsp);
static inline filter_t *filter_lookup_by_handle (hal_handle_t handle);
filter_t *filter_lookup_key_or_handle (const FilterKeyHandle& kh, lif_t **lif);
static hal_ret_t filter_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt);
static hal_ret_t filter_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt);
static inline hal_ret_t filter_del_from_db (filter_t *filter);
static hal_ret_t filter_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt);
static hal_ret_t filter_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt);
hal_ret_t filter_add_to_db_and_refs (filter_t *filter, lif_t *lif);
static bool filter_get_ht_cb (void *ht_entry, void *ctxt);
static void filter_process_get (filter_t *filter, FilterGetResponse *rsp);
const char *filter_bitset_to_str (uint32_t filter_bset);

//------------------------------------------------------------------------------
// filter create
//------------------------------------------------------------------------------
hal_ret_t
filter_create(FilterSpec& spec, FilterResponse *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    lif_t                       *lif = NULL;
    filter_t                    *filter = NULL;
    filter_key_t                key;
    dhl_entry_t                 dhl_entry = { 0 };
    cfg_op_ctxt_t               cfg_ctxt  = { 0 };
    filter_create_app_ctxt_t    app_ctxt  = { 0 };


    hal_api_trace(" API Begin: Filter create ");
    proto_msg_dump(spec);

    memset(&key, 0, sizeof(key));

    // validate filter proto
    ret = validate_filter_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("validation failed, ret : {}", ret);
        return ret;
    }

    auto kh = spec.key_or_handle();
    auto fk = kh.filter_key();

    lif = lif_lookup_key_or_handle(fk.lif_key_or_handle());
    if (!lif) {
        HAL_TRACE_ERR("Failed to find lif: {}",
                      lif_spec_keyhandle_to_str(fk.lif_key_or_handle()));
        ret = HAL_RET_LIF_NOT_FOUND;
        goto end;
    }

    // Initialize key from proto
    key.type = fk.type();
    key.lif_handle = lif->hal_handle;
    key.vlan = fk.vlan_id();
    MAC_UINT64_TO_ADDR(key.mac_addr, fk.mac_address());

    if ((filter = filter_lookup_by_key(&key))) {
        HAL_TRACE_ERR("Failed to create a filter, {} exists already",
                      filter_keyhandle_to_str(filter));
        ret = HAL_RET_ENTRY_EXISTS;
        goto end;
    }

    // instantiate a PI vrf object
    filter = filter_alloc_init();
    if (filter == NULL) {
        HAL_TRACE_ERR("Failed to alloc/init filter. Err: {}",
                      HAL_RET_OOM);
        ret = HAL_RET_OOM;
        goto end;
    }

    // initialize filter attrs from its spec
    ret = filter_init_from_spec(filter, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create filter. Init failed");
        goto end;
    }

    // allocate hal handle id
    filter->hal_handle = hal_handle_alloc(HAL_OBJ_ID_FILTER);
    if (filter->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("Failed to alloc handle for filter. Err: {}",
                      HAL_RET_HANDLE_INVALID);
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        filter_cleanup(filter);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    // form ctxt and call infra add
    app_ctxt.lif       = lif;
    dhl_entry.handle  = filter->hal_handle;
    dhl_entry.obj     = filter;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(filter->hal_handle, &cfg_ctxt,
                             filter_create_add_cb,
                             filter_create_commit_cb,
                             filter_create_abort_cb,
                             filter_create_cleanup_cb);

end:

    if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS)) {
        if (filter) {
           // free filter
            filter_cleanup(filter);
            filter = NULL;
        }
        HAL_API_STATS_INC(HAL_API_FILTER_CREATE_FAIL);
    } else {
        HAL_API_STATS_INC(HAL_API_FILTER_CREATE_SUCCESS);
    }

    filter_prepare_rsp(rsp, ret, filter ? filter->hal_handle : HAL_HANDLE_INVALID);
    return ret;
}

//------------------------------------------------------------------------------
// process a filter delete request
//------------------------------------------------------------------------------
hal_ret_t
filter_delete (FilterDeleteRequest& req, FilterDeleteResponse *rsp)
{
    hal_ret_t             ret = HAL_RET_OK;
    filter_t              *filter = NULL;
    cfg_op_ctxt_t         cfg_ctxt = { 0 };
    dhl_entry_t           dhl_entry = { 0 };
    lif_t                 *lif = NULL;
    const FilterKeyHandle &kh = req.key_or_handle();
    filter_delete_app_ctxt_t    app_ctxt  = { 0 };

    hal_api_trace(" API Begin: Filter delete ");
    proto_msg_dump(req);

    // validate the request message
    ret = validate_filter_delete_req(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("filter delete validation failed, err : {}", ret);
        goto end;
    }

    filter = filter_lookup_key_or_handle(kh, &lif);
    if (filter == NULL) {
        HAL_TRACE_ERR("Failed to find filter");
        ret = HAL_RET_FILTER_NOT_FOUND;
        goto end;
    }
    // TODO: Do we have to put back-refs of filters in LIF??
    if (lif == NULL) {
        HAL_TRACE_DEBUG("Failed to find lif. "
                        "Going ahead with deletion of filter.");
    }

    HAL_TRACE_DEBUG("Deleting filter {}", filter_keyhandle_to_str(filter));

    // form ctxt and call infra del
    app_ctxt.lif       = lif;
    dhl_entry.handle   = filter->hal_handle;
    dhl_entry.obj      = filter;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(filter->hal_handle, &cfg_ctxt,
                             filter_delete_del_cb,
                             filter_delete_commit_cb,
                             filter_delete_abort_cb,
                             filter_delete_cleanup_cb);
end:

    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC (HAL_API_FILTER_DELETE_SUCCESS);
    } else {
        HAL_API_STATS_INC (HAL_API_FILTER_DELETE_FAIL);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));
    return ret;
}

//------------------------------------------------------------------------------
// process a filter get request
//------------------------------------------------------------------------------
hal_ret_t
filter_get (FilterGetRequest& req, FilterGetResponseMsg *rsp)
{
    filter_t        *filter;

    if (!req.has_key_or_handle()) {
        g_hal_state->filter_ht()->walk(filter_get_ht_cb, rsp);
    } else {
        auto kh = req.key_or_handle();
        filter = filter_lookup_key_or_handle(kh, NULL);
        auto response = rsp->add_response();
        if (filter == NULL) {
            response->set_api_status(types::API_STATUS_NOT_FOUND);
            HAL_API_STATS_INC(HAL_API_FILTER_GET_FAIL);
            return HAL_RET_FILTER_NOT_FOUND;
        } else {
            filter_process_get(filter, response);
        }
    }

    HAL_API_STATS_INC(HAL_API_FILTER_GET_SUCCESS);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// callback invoked from filter hash table while processing filter get request
//------------------------------------------------------------------------------
static bool
filter_get_ht_cb (void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t *entry = (hal_handle_id_ht_entry_t *)ht_entry;
    FilterGetResponseMsg *rsp          = (FilterGetResponseMsg *)ctxt;
    FilterGetResponse *response        = rsp->add_response();
    filter_t          *filter             = NULL;

    filter = (filter_t *)hal_handle_get_obj(entry->handle_id);
    filter_process_get(filter, response);

    // return false here, so that we walk through all hash table entries.
    return false;
}

//------------------------------------------------------------------------------
// process a get request for a given filter
//------------------------------------------------------------------------------
static void
filter_process_get (filter_t *filter, FilterGetResponse *rsp)
{
    FilterSpec              *spec  = rsp->mutable_spec();
    FilterKeyHandle         *kh    = spec->mutable_key_or_handle();
    FilterKey               *key   = kh->mutable_filter_key();

    key->mutable_lif_key_or_handle()->set_lif_handle(filter->key.lif_handle);
    key->set_mac_address(MAC_TO_UINT64(filter->key.mac_addr));
    key->set_type(filter->key.type);
    key->set_vlan_id(filter->key.vlan);

    // fill operational state of this filter
    rsp->mutable_status()->set_filter_handle(filter->hal_handle);

    rsp->set_api_status(types::API_STATUS_OK);
}


//------------------------------------------------------------------------------
// delete add callback for filter delete
//------------------------------------------------------------------------------
static hal_ret_t
filter_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    filter_t                    *filter = NULL;
    lif_t                       *lif = NULL;
    filter_delete_app_ctxt_t    *app_ctxt  = NULL;

    HAL_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (filter_delete_app_ctxt_t *)cfg_ctxt->app_ctxt;

    filter = (filter_t *)dhl_entry->obj;
    lif = app_ctxt->lif;

    // lif can be NULL if lif is removed before the filters.
    if (lif) {
        // trigger lif to apply filter on all ENICs.
        ret = lif_handle_egress_en(lif, &filter->key, false);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to handle egress enable on lif: {}",
                          lif_keyhandle_to_str(lif));
            goto end;
        }
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// filter delete commit callback
//------------------------------------------------------------------------------
static hal_ret_t
filter_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t          ret = HAL_RET_OK;
    dllist_ctxt_t      *lnode = NULL;
    dhl_entry_t        *dhl_entry = NULL;
    filter_t           *filter = NULL;
    hal_handle_t       hal_handle = 0;
    lif_t              *lif = NULL;

    HAL_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    filter = (filter_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("filter delete commit cb {}",
                    filter_keyhandle_to_str(filter));

    // remove back refs from lif
    lif = find_lif_by_handle(filter->key.lif_handle);
    ret = lif_del_filter(lif, filter);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to del rel. from lif");
        goto end;
    }

    ret = filter_del_from_db(filter);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to del filter {} to db, err : {}",
                      filter_keyhandle_to_str(filter), ret);
        goto end;
    }

    hal_handle_free(hal_handle);

    filter_cleanup(filter);

end:
    return ret;
}

//------------------------------------------------------------------------------
// filter delete fails, nothing to do
//------------------------------------------------------------------------------
static hal_ret_t
filter_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// filter delete fails, nothing to do
//------------------------------------------------------------------------------
static hal_ret_t
filter_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate filter delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_filter_delete_req (FilterDeleteRequest& req, FilterDeleteResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("filter spec has no key or handle");
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}


hal_ret_t
validate_filter_create (FilterSpec& spec, FilterResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // make sure key-handle field is set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("filter didn't have key or handle. Err: {}",
                      HAL_RET_INVALID_ARG);
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    // key has to be set
    auto kh = spec.key_or_handle();
    if (kh.key_or_handle_case() != FilterKeyHandle::kFilterKey) {
        HAL_TRACE_ERR("Filter Key is not set");
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    // lif has to be set in the key
    auto fk = kh.filter_key();
    if (!fk.has_lif_key_or_handle()) {
        HAL_TRACE_ERR("filter didn't have lif. Err: {}",
                      HAL_RET_INVALID_ARG);
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    if (fk.type() == kh::FILTER_LIF) {
        if (fk.vlan_id() != 0) {
            HAL_TRACE_ERR("Filter type is Lif,*,* but vlan is non-zero");
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            return HAL_RET_INVALID_ARG;
        }

        if (fk.mac_address() != 0) {
            HAL_TRACE_ERR("Filter type is Lif,*,* but mac is non-zero");
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            return HAL_RET_INVALID_ARG;
        }
    }

    if (fk.type() == kh::FILTER_LIF_MAC) {
        if (fk.vlan_id() != 0) {
            HAL_TRACE_ERR("Filter type is MAC but vlan is non-zero");
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            return HAL_RET_INVALID_ARG;
        }
    }

    if (fk.type() == kh::FILTER_LIF_VLAN) {
        if (fk.mac_address() != 0) {
            HAL_TRACE_ERR("Filter type is Vlan but mac is non-zero");
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            return HAL_RET_INVALID_ARG;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
// Create add callback for filter create
//------------------------------------------------------------------------------
static hal_ret_t
filter_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    filter_t                    *filter = NULL;
    lif_t                       *lif = NULL;
    filter_create_app_ctxt_t    *app_ctxt  = NULL;

    HAL_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (filter_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    filter = (filter_t *)dhl_entry->obj;
    lif = app_ctxt->lif;

    // trigger lif to apply filter on all ENICs.
    ret = lif_handle_egress_en(lif, &filter->key, true);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to handle egress enable on lif: {}",
                      lif_keyhandle_to_str(lif));
        goto end;
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// Create commit callback
//------------------------------------------------------------------------------
static hal_ret_t
filter_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret        = HAL_RET_OK;
    dllist_ctxt_t               *lnode     = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    filter_t                    *filter       = NULL;
    lif_t                       *lif = NULL;
    filter_create_app_ctxt_t    *app_ctxt  = NULL;

    HAL_ASSERT(cfg_ctxt != NULL);
    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (filter_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    filter = (filter_t *)dhl_entry->obj;
    lif = app_ctxt->lif;

    ret = filter_add_to_db_and_refs(filter, lif);

    return ret;
}

hal_ret_t
filter_add_to_db_and_refs (filter_t *filter, lif_t *lif)
{
    hal_ret_t ret        = HAL_RET_OK;

    // add to filter id hash table
    ret = filter_add_to_db(filter, filter->hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add filter {} to db, err : {}",
                      filter_keyhandle_to_str(filter), ret);
    }

    // Add filter to lif's filter list
    ret = lif_add_filter(lif, filter);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add rel. from lif");
        goto end;
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// Create abort callback
//------------------------------------------------------------------------------
static hal_ret_t
filter_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t ret                        = HAL_RET_OK;
    dllist_ctxt_t *lnode                 = NULL;
    dhl_entry_t *dhl_entry               = NULL;
    filter_t *filter                     = NULL;
    hal_handle_t hal_handle              = 0;

    HAL_ASSERT(cfg_ctxt != NULL);
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    filter = (filter_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("filter {} create abort cb",
                    filter_keyhandle_to_str(filter));

    hal_handle_free(hal_handle);

    return ret;
}

//----------------------------------------------------------------------------
// dummy create cleanup callback
//----------------------------------------------------------------------------
static hal_ret_t
filter_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize a filter object from its spec
//------------------------------------------------------------------------------
hal_ret_t
filter_init_from_spec (filter_t *filter, const FilterSpec& spec)
{
    hal_ret_t          ret = HAL_RET_OK;
    filter_key_t       *key = &filter->key;
    lif_t              *lif = NULL;

    auto kh = spec.key_or_handle();
    auto fk = kh.filter_key();

    lif = lif_lookup_key_or_handle(fk.lif_key_or_handle());
    if (!lif) {
        HAL_TRACE_ERR("Unable to find lif: {}",
                      lif_spec_keyhandle_to_str(fk.lif_key_or_handle()));
        return ret;
    }

    key->type       = fk.type();
    key->lif_handle = lif->hal_handle;
    key->vlan       = fk.vlan_id();
    MAC_UINT64_TO_ADDR(key->mac_addr, fk.mac_address());

    return ret;
}

//------------------------------------------------------------------------------
// converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
filter_prepare_rsp (FilterResponse *rsp, hal_ret_t ret, hal_handle_t hal_handle)
{
    if ((ret == HAL_RET_OK) || (ret == HAL_RET_ENTRY_EXISTS)) {
        rsp->mutable_filter_status()->set_filter_handle(hal_handle);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
//  - Get key from entry
//-----------------------------------------------------------------------------
void *
filter_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t    *ht_entry;
    filter_t                    *filter = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    filter = find_filter_by_handle(ht_entry->handle_id);
    return (void *)&(filter->key);
}

//-----------------------------------------------------------------------------
// hash table key => entry - compute hash
//-----------------------------------------------------------------------------
uint32_t
filter_compute_key_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(filter_key_t)) % ht_size;
}

//-----------------------------------------------------------------------------
// hash table key => entry - compare function
//-----------------------------------------------------------------------------
bool
filter_compare_key_func (void *key1, void *key2)
{
#if 0
    HAL_TRACE_DEBUG("key1: {}, key2: {}",
                    filter_key_to_str((filter_key_t*)key1),
                    filter_key_to_str((filter_key_t*)key2));
#endif
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(filter_key_t))) {
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
// find filter by handle
//-----------------------------------------------------------------------------
filter_t *
find_filter_by_handle (hal_handle_t handle)
{
    hal_handle *handle_obj;

    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }

    // check for object type
    handle_obj = hal_handle_get_from_handle_id(handle);
    if (!handle_obj || handle_obj->obj_id() != HAL_OBJ_ID_FILTER) {
        return NULL;
    }
    return (filter_t *)hal_handle_get_obj(handle);
}

//------------------------------------------------------------------------------
// allocate a filter instance
//------------------------------------------------------------------------------
static inline filter_t *
filter_alloc (void)
{
    filter_t    *filter;

    filter = (filter_t *)g_hal_state->filter_slab()->alloc();
    if (filter == NULL) {
        return NULL;
    }
    return filter;
}

//------------------------------------------------------------------------------
// initialize a filter instance
//------------------------------------------------------------------------------
static inline filter_t *
filter_init (filter_t *filter)
{
    if (!filter) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&filter->slock, PTHREAD_PROCESS_SHARED);

    // initialize the operational state
    filter->hal_handle   = HAL_HANDLE_INVALID;
    memset(&filter->key, 0, sizeof(filter_key_t));

    return filter;
}

//------------------------------------------------------------------------------
// allocate and initialize a filter instance
//------------------------------------------------------------------------------
static inline filter_t *
filter_alloc_init (void)
{
    return filter_init(filter_alloc());
}

// free filter instance
// Note: This is not a deep free wherein the list or other pointers have to
//       be freed separately
static inline hal_ret_t
filter_free (filter_t *filter)
{
    HAL_SPINLOCK_DESTROY(&filter->slock);
    hal::delay_delete_to_slab(HAL_SLAB_FILTER, filter);
    return HAL_RET_OK;
}

// anti filter_alloc_init
static inline hal_ret_t
filter_cleanup (filter_t *filter)
{
    filter_free(filter);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Lookup filter from key or handle
//------------------------------------------------------------------------------
filter_t *
filter_lookup_key_or_handle (const FilterKeyHandle& kh, lif_t **lif)
{
    filter_t     *filter = NULL;
    filter_key_t key;


    if (kh.key_or_handle_case() == FilterKeyHandle::kFilterKey) {
        auto fk = kh.filter_key();
        memset(&key, 0, sizeof(key));

        // Find lif
        if (lif) {
            *lif = lif_lookup_key_or_handle(fk.lif_key_or_handle());
            if (!*lif) {
                HAL_TRACE_ERR("Failed to find lif for : {}",
                              lif_spec_keyhandle_to_str(fk.lif_key_or_handle()));
                goto end;
            }
        }

        // Initialize key from proto
        key.type = fk.type();
        key.lif_handle = (*lif)->hal_handle;
        key.vlan = fk.vlan_id();
        MAC_UINT64_TO_ADDR(key.mac_addr, fk.mac_address());

        filter = filter_lookup_by_key(&key);
    } else if (kh.key_or_handle_case() == FilterKeyHandle::kFilterHandle) {
        filter = filter_lookup_by_handle(kh.filter_handle());

        // Find lif
        if (lif) {
            *lif = find_lif_by_handle(filter->key.lif_handle);
            if (!*lif) {
                HAL_TRACE_DEBUG("Failed to find lif for : {}",
                                filter->key.lif_handle);
                goto end;
            }
        }
    }

end:
    return filter;
}

//------------------------------------------------------------------------------
// find a filter instance by its key
//------------------------------------------------------------------------------
filter_t *
filter_lookup_by_key (filter_key_t *key)
{
    hal_handle_id_ht_entry_t    *entry;
    filter_t                    *filter;

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->filter_ht()->lookup(key);
    if (entry && (entry->handle_id != HAL_HANDLE_INVALID)) {

        // check for object type
        HAL_ASSERT(hal_handle_get_from_handle_id(entry->handle_id)->obj_id() ==
                HAL_OBJ_ID_FILTER);

        filter = (filter_t *)hal_handle_get_obj(entry->handle_id);
        return filter;
    }
    return NULL;
}

//------------------------------------------------------------------------------
// find a filter instance by its handle
//------------------------------------------------------------------------------
static inline filter_t *
filter_lookup_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }

    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("Failed to find object with handle {}", handle);
        return NULL;
    }
    if (hal_handle->obj_id() != HAL_OBJ_ID_FILTER) {
        HAL_TRACE_DEBUG("Failed to find filter with handle {}", handle);
        return NULL;
    }
    return (filter_t *)hal_handle->obj();
}

//------------------------------------------------------------------------------
// insert a filter to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
filter_add_to_db (filter_t *filter, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Adding filter {} to cfg db",
                    filter_keyhandle_to_str(filter));

    // allocate an entry to establish mapping from vrf id to its handle
    entry = (hal_handle_id_ht_entry_t *)
                g_hal_state->hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from filter key to its handle
    entry->handle_id = handle;
    sdk_ret = g_hal_state->filter_ht()->insert_with_key(&filter->key,
                                                        entry, &entry->ht_ctxt);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add filter {} to handle mapping, "
                      "err : {}", filter_keyhandle_to_str(filter), ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
        goto end;
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// delete a filter from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
filter_del_from_db (filter_t *filter)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Removing from filter {} from cfg db",
                    filter_keyhandle_to_str(filter));

    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->filter_ht()->
        remove(&filter->key);

    // free up
    hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);

    return HAL_RET_OK;
}

#define FILTER_SET(bitset, filter) bitset |= (1 << filter)
#define FILTER_UNSET(bitset, filter) bitset &= ~(1 << filter)
#define FILTER_CHECK(bitset, filter) bitset & (1 << filter)

//------------------------------------------------------------------------------
// Get ENIC's egress en from existing filters
//------------------------------------------------------------------------------
hal_ret_t
filter_check_enic (lif_t *lif, if_t *hal_if, bool *egress_en)
{
    hal_ret_t       ret = HAL_RET_OK;
    filter_key_t    enic_key_mac_vlan, enic_key_mac, enic_key_vlan, enic_key_lif;
    uint32_t        db_filters_bset = 0;

    *egress_en = false;

    memset(&enic_key_mac_vlan, 0, sizeof(filter_key_t));
    memset(&enic_key_mac, 0, sizeof(filter_key_t));
    memset(&enic_key_vlan, 0, sizeof(filter_key_t));
    memset(&enic_key_lif, 0, sizeof(filter_key_t));

    enic_key_mac_vlan.lif_handle = lif->hal_handle;
    enic_key_mac.lif_handle = lif->hal_handle;
    enic_key_vlan.lif_handle = lif->hal_handle;
    enic_key_lif.lif_handle = lif->hal_handle;

    enic_key_mac_vlan.type = kh::FILTER_LIF_MAC_VLAN;
    enic_key_mac_vlan.vlan = hal_if->encap_vlan;
    memcpy(enic_key_mac_vlan.mac_addr, hal_if->mac_addr, ETH_ADDR_LEN);

    enic_key_mac.type = kh::FILTER_LIF_MAC;
    memcpy(enic_key_mac.mac_addr, hal_if->mac_addr, ETH_ADDR_LEN);

    enic_key_vlan.type = kh::FILTER_LIF_VLAN;
    enic_key_vlan.vlan = hal_if->encap_vlan;

    enic_key_lif.type = kh::FILTER_LIF;

    // Check what filters match in DB
    if (filter_lookup_by_key(&enic_key_mac_vlan)) {
        FILTER_SET(db_filters_bset, kh::FILTER_LIF_MAC_VLAN);
    }
    if (filter_lookup_by_key(&enic_key_mac)) {
        FILTER_SET(db_filters_bset, kh::FILTER_LIF_MAC);
    }
    if (filter_lookup_by_key(&enic_key_vlan)) {
        FILTER_SET(db_filters_bset, kh::FILTER_LIF_VLAN);
    }
    if (filter_lookup_by_key(&enic_key_lif)) {
        FILTER_SET(db_filters_bset, kh::FILTER_LIF);
    }

    /*
     * Allow:
     *  - LIF, *, *: Generally this happens on prom. lif.
     *  - (LIF, Mac, *) & (LIF, *, Vlan): Both filters have to be present
     *  - (LIF, Mac, Vlan): Exact match and allow.
     */
    if (FILTER_CHECK(db_filters_bset, kh::FILTER_LIF) ||
        FILTER_CHECK(db_filters_bset, kh::FILTER_LIF_MAC_VLAN) ||
        (FILTER_CHECK(db_filters_bset, kh::FILTER_LIF_MAC) &&
         FILTER_CHECK(db_filters_bset, kh::FILTER_LIF_VLAN))) {
        *egress_en = true;
    }

    HAL_TRACE_DEBUG("Enic matched {}, egress_en: {}",
                    filter_bitset_to_str(db_filters_bset),
                    *egress_en);

    return ret;
}

//------------------------------------------------------------------------------
// checks if enic's egress en changed
//------------------------------------------------------------------------------
hal_ret_t
filter_check_enic_with_filter (filter_key_t *upd_key, lif_t *lif, if_t *hal_if,
                               bool egress_en, bool *update_enic)
{
    hal_ret_t       ret = HAL_RET_OK;
    filter_key_t    enic_key_mac_vlan, enic_key_mac, enic_key_vlan, enic_key_lif;
    FilterType      upd_filter_match = kh::FILTER_NONE;
    uint32_t        db_filters_bset = 0, upd_filter_bset = 0;

    *update_enic = false;

    memset(&enic_key_mac_vlan, 0, sizeof(filter_key_t));
    memset(&enic_key_mac, 0, sizeof(filter_key_t));
    memset(&enic_key_vlan, 0, sizeof(filter_key_t));
    memset(&enic_key_lif, 0, sizeof(filter_key_t));

    enic_key_mac_vlan.lif_handle = lif->hal_handle;
    enic_key_mac.lif_handle = lif->hal_handle;
    enic_key_vlan.lif_handle = lif->hal_handle;
    enic_key_lif.lif_handle = lif->hal_handle;

    enic_key_mac_vlan.type = kh::FILTER_LIF_MAC_VLAN;
    enic_key_mac_vlan.vlan = hal_if->encap_vlan;
    memcpy(enic_key_mac_vlan.mac_addr, hal_if->mac_addr, ETH_ADDR_LEN);

    enic_key_mac.type = kh::FILTER_LIF_MAC;
    memcpy(enic_key_mac.mac_addr, hal_if->mac_addr, ETH_ADDR_LEN);

    enic_key_vlan.type = kh::FILTER_LIF_VLAN;
    enic_key_vlan.vlan = hal_if->encap_vlan;

    enic_key_lif.type = kh::FILTER_LIF;

    HAL_TRACE_DEBUG("Checking for IF: {}",
                    if_keyhandle_to_str(hal_if));
    HAL_TRACE_DEBUG("Filter being updated: {}", filter_key_to_str(upd_key));
    HAL_TRACE_DEBUG("Mac_Vlan Match Key: {}", filter_key_to_str(&enic_key_mac_vlan));
    HAL_TRACE_DEBUG("Mac Match Key: {}", filter_key_to_str(&enic_key_mac));
    HAL_TRACE_DEBUG("Vlan Match Key: {}", filter_key_to_str(&enic_key_vlan));
    HAL_TRACE_DEBUG("Lif Match Key: {}", filter_key_to_str(&enic_key_lif));
    if (!memcmp(upd_key, &enic_key_lif, sizeof(filter_key_t))) {
        upd_filter_match = kh::FILTER_LIF;
    } else if (!memcmp(upd_key, &enic_key_mac_vlan, sizeof(filter_key_t))) {
        upd_filter_match = kh::FILTER_LIF_MAC_VLAN;
    } else if (!memcmp(upd_key, &enic_key_mac, sizeof(filter_key_t))) {
        upd_filter_match = kh::FILTER_LIF_MAC;
    } else if (!memcmp(upd_key, &enic_key_vlan, sizeof(filter_key_t))) {
        upd_filter_match = kh::FILTER_LIF_VLAN;
    } else {
        HAL_TRACE_DEBUG("Filter Change: No-op for ENIC: {}",
                        if_keyhandle_to_str(hal_if));
        goto end;
    }

    FILTER_SET(upd_filter_bset, upd_filter_match);

    // Check what filters match in DB
    if (filter_lookup_by_key(&enic_key_lif)) {
        FILTER_SET(db_filters_bset, kh::FILTER_LIF);
    }
    if (filter_lookup_by_key(&enic_key_mac_vlan)) {
        FILTER_SET(db_filters_bset, kh::FILTER_LIF_MAC_VLAN);
    }
    if (filter_lookup_by_key(&enic_key_mac)) {
        FILTER_SET(db_filters_bset, kh::FILTER_LIF_MAC);
    }
    if (filter_lookup_by_key(&enic_key_vlan)) {
        FILTER_SET(db_filters_bset, kh::FILTER_LIF_VLAN);
    }

    HAL_TRACE_DEBUG("If: {}, Upd_filter: {}, db_filters_bset: {}",
                    if_keyhandle_to_str(hal_if),
                    filter_bitset_to_str(upd_filter_bset),
                    filter_bitset_to_str(db_filters_bset));

    if (egress_en) {
        // Update filter match should not be set
        HAL_ASSERT((upd_filter_bset & db_filters_bset) == 0);

        if (upd_filter_match == kh::FILTER_LIF) {
            HAL_TRACE_DEBUG("(Lif,*,*) filter. Update enic: {}",
                            if_keyhandle_to_str(hal_if));
                *update_enic = true;
        } else if (upd_filter_match == kh::FILTER_LIF_MAC_VLAN) {
            if (FILTER_CHECK(db_filters_bset, kh::FILTER_LIF_MAC) &&
                FILTER_CHECK(db_filters_bset, kh::FILTER_LIF_VLAN)) {
                HAL_TRACE_DEBUG("Already matched mac & vlan. "
                                "Now mac-vlan-filter. No-op");
            } else {
                HAL_TRACE_DEBUG("Mac-Vlan filter without (mac & vlan) filters. "
                                "Update enic: {}",
                                if_keyhandle_to_str(hal_if));
                *update_enic = true;
            }
        } else if (upd_filter_match == kh::FILTER_LIF_MAC) {
            if (FILTER_CHECK(db_filters_bset, kh::FILTER_LIF_MAC_VLAN)) {
                HAL_TRACE_DEBUG("Already matched mac_vlan. Now mac-filter. No-op");
            } else {
                if (FILTER_CHECK(db_filters_bset, kh::FILTER_LIF_VLAN)) {
                    HAL_TRACE_DEBUG("Mac filter with vlan filter present. "
                                    "Update enic: {}",
                                    if_keyhandle_to_str(hal_if));
                    *update_enic = true;
                } else {
                    HAL_TRACE_DEBUG("Mac filter without vlan filter. No-op");
                }
            }
        } else if (upd_filter_match == kh::FILTER_LIF_VLAN) {
            if (FILTER_CHECK(db_filters_bset, kh::FILTER_LIF_MAC_VLAN)) {
                HAL_TRACE_DEBUG("Already matched mac_vlan. Now vlan-filter. No-op");
            } else {
                if (FILTER_CHECK(db_filters_bset, kh::FILTER_LIF_MAC)) {
                    HAL_TRACE_DEBUG("Vlan filter with mac filter present. "
                                    "Update enic: {}",
                                    if_keyhandle_to_str(hal_if));
                    *update_enic = true;
                } else {
                    HAL_TRACE_DEBUG("Vlan filter without mac filter. No-op");
                }
            }
        }
    } else {
        // Update filter match should be set
        HAL_ASSERT(upd_filter_bset & db_filters_bset);

        if (upd_filter_match == kh::FILTER_LIF) {
            if (FILTER_CHECK(db_filters_bset, kh::FILTER_LIF_MAC_VLAN)) {
                HAL_TRACE_DEBUG("Already matched (lif,mac,vlan). "
                                "Now lif-filter removal. No-op");
            } else if (FILTER_CHECK(db_filters_bset, kh::FILTER_LIF_MAC) &&
                FILTER_CHECK(db_filters_bset, kh::FILTER_LIF_VLAN)) {
                HAL_TRACE_DEBUG("Already matched mac & vlan. "
                                "Now lif-filter removal. No-op");
            } else {
                HAL_TRACE_DEBUG("lif filter removal without (mac & vlan) or (mac_vlan) filters. "
                                "Disabling egress. Update enic: {}",
                                if_keyhandle_to_str(hal_if));
                *update_enic = true;
            }
        } else if (upd_filter_match == kh::FILTER_LIF_MAC_VLAN) {
            if (FILTER_CHECK(db_filters_bset, kh::FILTER_LIF)) {
                HAL_TRACE_DEBUG("Already matched (lif,*,*). "
                                "Now mac-vlan-filter removal. No-op");
            } else if (FILTER_CHECK(db_filters_bset, kh::FILTER_LIF_MAC) &&
                FILTER_CHECK(db_filters_bset, kh::FILTER_LIF_VLAN)) {
                HAL_TRACE_DEBUG("Already matched mac & vlan. "
                                "Now mac-vlan-filter removal. No-op");
            } else {
                HAL_TRACE_DEBUG("Mac-Vlan filter removal without (mac & vlan) or lif filters. "
                                "Disabling egress. Update enic: {}",
                                if_keyhandle_to_str(hal_if));
                *update_enic = true;
            }
        } else if (upd_filter_match == kh::FILTER_LIF_MAC) {
            if (FILTER_CHECK(db_filters_bset, kh::FILTER_LIF)) {
                HAL_TRACE_DEBUG("Already matched (lif,*,*). "
                                "Now mac filter removal. No-op");
            } else if (FILTER_CHECK(db_filters_bset, kh::FILTER_LIF_MAC_VLAN)) {
                HAL_TRACE_DEBUG("Already matched mac_vlan. "
                                "Now mac-filter removal. No-op");
            } else {
                if (FILTER_CHECK(db_filters_bset, kh::FILTER_LIF_VLAN)) {
                    HAL_TRACE_DEBUG("Mac filter removal with vlan filter present. "
                                    "Disabling egress. Update enic: {}",
                                    if_keyhandle_to_str(hal_if));
                    *update_enic = true;
                } else {
                    HAL_TRACE_DEBUG("Mac filter removal without vlan filter. No-op");
                }
            }
        } else if (upd_filter_match == kh::FILTER_LIF_VLAN) {
            if (FILTER_CHECK(db_filters_bset, kh::FILTER_LIF)) {
                HAL_TRACE_DEBUG("Already matched (lif,*,*). "
                                "Now vlan filter removal. No-op");
            } else if (FILTER_CHECK(db_filters_bset, kh::FILTER_LIF_MAC_VLAN)) {
                HAL_TRACE_DEBUG("Already matched mac_vlan. "
                                "Now vlan-filter removal. No-op");
            } else {
                if (FILTER_CHECK(db_filters_bset, kh::FILTER_LIF_MAC)) {
                    HAL_TRACE_DEBUG("Vlan filter removal with mac filter present. "
                                    "Disabling egress. Update enic: {}",
                                    if_keyhandle_to_str(hal_if));
                    *update_enic = true;
                } else {
                    HAL_TRACE_DEBUG("Vlan filter removal without mac filter. No-op");
                }
            }
        }
    }

end:
    return ret;
}
//------------------------------------------------------------------------------
// filter bitset to str
//------------------------------------------------------------------------------
const char *
filter_bitset_to_str (uint32_t filter_bset)
{
    static thread_local char       filter_str[4][100];
    static thread_local uint8_t    filter_str_next = 0;
    char                           *buf;

    buf = filter_str[filter_str_next++ & 0x3];
    memset(buf, 0, 100);
    snprintf(buf, 100,
             "Filters: %d - %s:%s:%s:%s",
             filter_bset,
             FILTER_CHECK(filter_bset, kh::FILTER_LIF) ? "(Lif, *, *)" : "",
             FILTER_CHECK(filter_bset, kh::FILTER_LIF_MAC) ? "(Lif, Mac, *)" : "",
             FILTER_CHECK(filter_bset, kh::FILTER_LIF_VLAN) ? "(Lif, *, Vlan)" : "",
             FILTER_CHECK(filter_bset, kh::FILTER_LIF_MAC_VLAN) ? "(Lif, Mac, Vlan)" : "");
    return buf;
}

//------------------------------------------------------------------------------
// PI filter key to str
//------------------------------------------------------------------------------
const char *
filter_key_to_str (filter_key_t *key)
{
    static thread_local char       filter_str[4][100];
    static thread_local uint8_t    filter_str_next = 0;
    char                           *buf;

    buf = filter_str[filter_str_next++ & 0x3];
    memset(buf, 0, 100);
    if (key) {
        snprintf(buf, 100,
                 "filter(lif_hdl: %lu, f_type: %d, f_mac:%s, f_vlan:%d)",
                 key->lif_handle,
                 int(key->type),
                 macaddr2str(key->mac_addr),
                 key->vlan);
    }
    return buf;
}

//------------------------------------------------------------------------------
// PI filter to str
//------------------------------------------------------------------------------
const char *
filter_keyhandle_to_str (filter_t *filter)
{
    static thread_local char       filter_str[4][100];
    static thread_local uint8_t    filter_str_next = 0;
    char                           *buf;
    filter_key_t                   *key = &filter->key;

    buf = filter_str[filter_str_next++ & 0x3];
    memset(buf, 0, 100);
    if (filter) {
        snprintf(buf, 100,
                 "filter(lif_hdl: %lu, f_type: %d, f_mac:%s, f_vlan:%d, "
                 "handle: %lu)",
                 key->lif_handle,
                 key->type,
                 macaddr2str(key->mac_addr),
                 key->vlan,
                 filter->hal_handle);
    }
    return buf;
}


} // namespace hal
