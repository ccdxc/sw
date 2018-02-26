// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/gft.hpp"
#include "nic/include/pd_api.hpp"

#define GFT_EMFE_SET_FLAGS_FROM_SPEC(entry, spec, field, flag)               \
do {                                                                         \
    if ((spec).field()) {                                                    \
        (entry)->flags |= GFT_EMFE_ ## flag;                                 \
    }                                                                        \
} while (0)

#define GFT_EMFE_SET_HEADERS_FROM_SPEC(headers, hdrs_spec, hdr, flag)        \
do {                                                                         \
        if (hdrs_spec.hdr()) {                                               \
            *headers |= GFT_HEADER_ ## flag;                                 \
        }                                                                    \
} while (0)

#define GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec, field, flag)      \
do {                                                                         \
    if (fields_spec.field()) {                                               \
        *fields |= GFT_HEADER_FIELD_ ## flag;                                \
    }                                                                        \
} while (0)


namespace hal {

//----------------------------------------------------------------------------
// get GFT exact match profile's key
//----------------------------------------------------------------------------
void *
gft_exact_match_profile_id_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t     *ht_entry;
    gft_exact_match_profile_t    *profile = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    profile =
        (gft_exact_match_profile_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(profile->profile_id);
}

//----------------------------------------------------------------------------
// compute the hash given GFT exact match profile's key (i.e., profile id)
//----------------------------------------------------------------------------
uint32_t
gft_exact_match_profile_id_compute_hash_func (void *key, uint32_t ht_size)
{
    HAL_ASSERT(key != NULL);
    return sdk::lib::hash_algo::fnv_hash(key,
               sizeof(gft_profile_id_t)) % ht_size;
}

//----------------------------------------------------------------------------
// compare the keys of GFT exact match profiles
//----------------------------------------------------------------------------
bool
gft_exact_match_profile_id_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(gft_profile_id_t *)key1 == *(gft_profile_id_t *)key2) {
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
// get GFT header transposition profile's key
//----------------------------------------------------------------------------
void *
gft_hdr_transposition_profile_id_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t       *ht_entry;
    gft_hdr_xposition_profile_t    *profile = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    profile =
        (gft_hdr_xposition_profile_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(profile->profile_id);
}

//-----------------------------------------------------------------------------
// compute hash given GFT header transposition profile's key (i.e., profile id)
//-----------------------------------------------------------------------------
uint32_t
gft_hdr_transposition_profile_id_compute_hash_func (void *key, uint32_t ht_size)
{
    HAL_ASSERT(key != NULL);
    return sdk::lib::hash_algo::fnv_hash(key,
               sizeof(gft_profile_id_t)) % ht_size;
}

//----------------------------------------------------------------------------
// compare the keys of GFT header transposition profiles
//----------------------------------------------------------------------------
bool
gft_hdr_transposition_profile_id_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(gft_profile_id_t *)key1 == *(gft_profile_id_t *)key2) {
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
// get GFT exact match flow entry's key
//----------------------------------------------------------------------------
void *
gft_exact_match_flow_entry_id_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t        *ht_entry;
    gft_exact_match_flow_entry_t    *flow_entry = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    flow_entry =
        (gft_exact_match_flow_entry_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(flow_entry->flow_entry_id);
}

//------------------------------------------------------------------------------
// compute the hash given GFT exact match flow entry's key (i.e., flow entry id)
//------------------------------------------------------------------------------
uint32_t
gft_exact_match_flow_entry_id_compute_hash_func (void *key, uint32_t ht_size)
{
    HAL_ASSERT(key != NULL);
    return sdk::lib::hash_algo::fnv_hash(key,
               sizeof(gft_flow_entry_id_t)) % ht_size;
}

//----------------------------------------------------------------------------
// compare the keys of GFT exact match flow entries
//----------------------------------------------------------------------------
bool
gft_exact_match_flow_entry_id_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(gft_flow_entry_id_t *)key1 == *(gft_flow_entry_id_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// insert a GFT exact match profile to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
gft_emp_add_to_db (gft_exact_match_profile_t *profile, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Adding GFT exact match profile {} to db",
                    profile->profile_id);

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
                hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from profile id to its handle
    entry->handle_id = handle;
    sdk_ret = g_hal_state->gft_exact_match_profile_id_ht()->
                  insert_with_key(&profile->profile_id, entry, &entry->ht_ctxt);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add GFT exact match profile {} to db, "
                      "err : {}", profile->profile_id, ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    return ret;
}

//------------------------------------------------------------------------------
// delete a GFT exact match profile from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
gft_emp_del_from_db (gft_exact_match_profile_t *profile)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Removing GFT exact match profile {} from db",
                    profile->profile_id);

    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)
                g_hal_state->gft_exact_match_profile_id_ht()->remove(&profile->profile_id);

    // free up
    hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate a given GFT exact match profile config spec
//------------------------------------------------------------------------------
static hal_ret_t
validate_gft_emp_create (GftExactMatchProfileSpec& spec,
                         GftExactMatchProfileResponse *rsp)
{
    hal_ret_t       ret = HAL_RET_OK;
    uint32_t        num_hgem_profiles = 0;

    num_hgem_profiles = spec.exact_match_profiles_size();
    if (!num_hgem_profiles) {
        HAL_TRACE_ERR("No header group exact match profiles");
        ret = HAL_RET_INVALID_ARG;
        goto end;

    }

end:
    return ret;
}

#define GFT_EMP_READ_HDRS(SPEC_FLD_NAME, HDR_MASK_BIT)                      \
    if (hdrs.SPEC_FLD_NAME()) {                                             \
        tmp->headers |= GFT_HEADER_ ## HDR_MASK_BIT;                        \
    }

#define GFT_EMP_READ_FIELDS(SPEC_FLD_NAME, HDR_MASK_BIT)                    \
    if (fields.SPEC_FLD_NAME()) {                                           \
        tmp->match_fields |= GFT_HEADER_FIELD_ ## HDR_MASK_BIT;             \
    }
//------------------------------------------------------------------------------
// initialize GFT exact match profile object from the config spec
//------------------------------------------------------------------------------
static inline hal_ret_t
gft_emp_init_from_spec (gft_exact_match_profile_t *profile,
                        GftExactMatchProfileSpec& spec)
{
    hal_ret_t                               ret = HAL_RET_OK;
    uint32_t                                num_hgem_profiles = 0;
    GftHeaders                              hdrs;
    GftHeaderFields                         fields;
    gft_hdr_group_exact_match_profile_t     *tmp = NULL;
    GftHeaderGroupExactMatchProfile         hg_emp;

    num_hgem_profiles = spec.exact_match_profiles_size();
    if (!num_hgem_profiles) {
        HAL_TRACE_ERR("No header group exact match profiles");
        ret = HAL_RET_INVALID_ARG;
        goto end;

    }
    profile->hgem_profiles = (gft_hdr_group_exact_match_profile_t *)
        HAL_CALLOC(HAL_MEM_ALLOC_HGEM, 
                   num_hgem_profiles * 
                   sizeof(gft_hdr_group_exact_match_profile_t));
    HAL_TRACE_DEBUG("Received {} of hdr group match profiles", 
                    num_hgem_profiles);

    profile->num_hdr_group_exact_match_profiles = num_hgem_profiles;
    profile->profile_id = spec.key_or_handle().profile_id();

    tmp = profile->hgem_profiles;
    for (uint32_t i = 0; i < num_hgem_profiles; i++) {
        hg_emp = spec.exact_match_profiles(i);
        hdrs = hg_emp.headers();
        fields = hg_emp.match_fields();

        GFT_EMP_READ_HDRS(ethernet_header, ETHERNET);
        GFT_EMP_READ_HDRS(ipv4_header, IPV4);
        GFT_EMP_READ_HDRS(ipv6_header, IPV6);
        GFT_EMP_READ_HDRS(tcp_header, TCP);
        GFT_EMP_READ_HDRS(udp_header, UDP);
        GFT_EMP_READ_HDRS(icmp_header, ICMP);
        GFT_EMP_READ_HDRS(no_encap, NO_ENCAP);
        GFT_EMP_READ_HDRS(ip_in_ip_encap, IP_IN_IP_ENCAP);
        GFT_EMP_READ_HDRS(ip_in_gre_encap, IP_IN_GRE_ENCAP);
        GFT_EMP_READ_HDRS(nvgre_encap, NVGRE_ENCAP);
        GFT_EMP_READ_HDRS(vxlan_encap, VXLAN_ENCAP);

        GFT_EMP_READ_FIELDS(dst_mac_addr, DST_MAC_ADDR);
        GFT_EMP_READ_FIELDS(src_mac_addr, SRC_MAC_ADDR);
        GFT_EMP_READ_FIELDS(eth_type, ETH_TYPE);
        GFT_EMP_READ_FIELDS(customer_vlan_id, CUSTOMER_VLAN_ID);
        GFT_EMP_READ_FIELDS(provider_vlan_id, PROVIDER_VLAN_ID);
        GFT_EMP_READ_FIELDS(dot1p_priority, 8021P_PRIORITY);
        GFT_EMP_READ_FIELDS(src_ip_addr, SRC_IP_ADDR);
        GFT_EMP_READ_FIELDS(dst_ip_addr, DST_IP_ADDR);
        GFT_EMP_READ_FIELDS(ip_ttl, TTL);
        GFT_EMP_READ_FIELDS(ip_protocol, IP_PROTOCOL);
        GFT_EMP_READ_FIELDS(ip_dscp, IP_DSCP);
        GFT_EMP_READ_FIELDS(src_port, TRANSPORT_SRC_PORT);
        GFT_EMP_READ_FIELDS(dst_port, TRANSPORT_DST_PORT);
        GFT_EMP_READ_FIELDS(tcp_flags, TCP_FLAGS);
        GFT_EMP_READ_FIELDS(tenant_id, TENANT_ID);
        GFT_EMP_READ_FIELDS(icmp_type, ICMP_TYPE);
        GFT_EMP_READ_FIELDS(icmp_code, ICMP_CODE);
        GFT_EMP_READ_FIELDS(oob_vlan, OOB_VLAN);
        GFT_EMP_READ_FIELDS(oob_tenant_id, OOB_TENANT_ID);
        GFT_EMP_READ_FIELDS(gre_protocol, GRE_PROTOCOL);

        tmp++;
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// lookup GFT exact match profile from key or handle
//------------------------------------------------------------------------------
static gft_exact_match_profile_t *
gft_emp_lookup_key_or_handle (const GftExactMatchProfileKeyHandle& kh)
{
    gft_exact_match_profile_t    *profile = NULL;

    if (kh.key_or_handle_case() == GftExactMatchProfileKeyHandle::kProfileId) {
        profile = find_gft_exact_match_profile_by_id(kh.profile_id());
    } else if (kh.key_or_handle_case() ==
               GftExactMatchProfileKeyHandle::kProfileHandle) {
        profile = find_gft_exact_match_profile_by_handle(kh.profile_handle());
    }

    return profile;
}

//------------------------------------------------------------------------------
// pd call to allocate pd resources and h/w programming
//------------------------------------------------------------------------------
static hal_ret_t
gft_emp_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                                    ret = HAL_RET_OK;
    pd::pd_gft_exact_match_profile_args_t        args = { 0 };
    dllist_ctxt_t                                *lnode = NULL;
    dhl_entry_t                                  *dhl_entry = NULL;
    gft_exact_match_profile_t                    *profile = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("Failed to create GFT exact match profile");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    profile = (gft_exact_match_profile_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("GFT exact match profile create add callback {}",
                    profile->profile_id);

    // PD Call to allocate PD resources and h/w programming, if any
    args.exact_match_profile = profile;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_GFT_EXACT_MATCH_PROFILE_CREATE, 
                          (void *)&args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create pd exact match profile, err : {}", ret);
    }

end:

    return ret;
}

//------------------------------------------------------------------------------
// update PI databases as gft_emp_create_add_cb was a success
//------------------------------------------------------------------------------
static hal_ret_t
gft_emp_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret        = HAL_RET_OK;
    dllist_ctxt_t               *lnode     = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    gft_exact_match_profile_t   *prof      = NULL;
    hal_handle_t                hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("invalid cfg_ctxt");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    prof = (gft_exact_match_profile_t  *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    ret = gft_emp_add_to_db(prof, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed to add prof {} to db, err : {}", 
                prof->profile_id, ret);
        goto end;
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// gft_emp_create_add_cb was a failure, do the cleanup
//------------------------------------------------------------------------------
hal_ret_t
gft_emp_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// dummy create cleanup callback
//-----------------------------------------------------------------------------
static hal_ret_t
gft_emp_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// convert hal_ret_t to API status and prepare response to the API call
//------------------------------------------------------------------------------
static hal_ret_t
gft_emp_prepare_rsp (GftExactMatchProfileResponse *rsp,
                     gft_exact_match_profile_t *profile,
                     hal_ret_t ret)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_status()->set_profile_handle(profile->hal_handle);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a GFT exact match profile create request
//------------------------------------------------------------------------------
hal_ret_t
gft_exact_match_profile_create (GftExactMatchProfileSpec& spec,
                                GftExactMatchProfileResponse *rsp)
{
    hal_ret_t                    ret;
    gft_exact_match_profile_t    *profile;
    gft_emp_create_app_ctxt_t    app_ctxt; // = { 0 };
    cfg_op_ctxt_t                cfg_ctxt = { 0 };
    dhl_entry_t                  dhl_entry = { 0 };

    hal_api_trace(" API Start: GFT EMP Create");
    // validate the request message
    ret = validate_gft_emp_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        // api_status already set, just return
        HAL_TRACE_ERR("GFT exact match profile object validation failure, "
                      "err : {}", ret);
        return ret;
    }

    HAL_TRACE_DEBUG("Creating GFT exact match profile id {}",
                    spec.key_or_handle().profile_id());

    // check if this profile exists already, and reject if one is found
    profile = gft_emp_lookup_key_or_handle(spec.key_or_handle());
    if (profile) {
        HAL_TRACE_ERR("Failed to create GFT exact match profile, "
                      "profile {} exists already",
                      spec.key_or_handle().profile_id());
        return HAL_RET_ENTRY_EXISTS;
    }

    // instantiate the gft exact match profile
    profile = gft_exact_match_profile_alloc_init();
    if (profile == NULL) {
        ret = HAL_RET_OOM;
        HAL_TRACE_ERR("Failed to allocate GFT exact match profile, err : {}",
                      ret);
        goto end;
    }

    // consume the config
    gft_emp_init_from_spec(profile, spec);

    gft_exact_match_profile_print(profile);

    // allocate hal handle id
    profile->hal_handle = hal_handle_alloc(HAL_OBJ_ID_GFT_EXACT_MATCH_PROFILE);
    if (profile->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("Failed to alloc handle for GFT exact match profile {}",
                      profile->profile_id);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    // form ctxt and call infra add
    dhl_entry.handle = profile->hal_handle;
    dhl_entry.obj = profile;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(profile->hal_handle, &cfg_ctxt,
                             gft_emp_create_add_cb,
                             gft_emp_create_commit_cb,
                             gft_emp_create_abort_cb,
                             gft_emp_create_cleanup_cb);

end:

    if (ret != HAL_RET_OK) {
        if (profile) {
            profile = NULL;
        }
    }
    gft_emp_prepare_rsp(rsp, profile, ret);
    hal_api_trace(" API End: GFT EMP Create");
    return ret;
}

//------------------------------------------------------------------------------
// insert a GFT header transposition profile to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
gft_htp_add_to_db (gft_hdr_xposition_profile_t *profile, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Adding GFT header transposition profile {} to db",
                    profile->profile_id);

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
                hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from profile id to its handle
    entry->handle_id = handle;
    sdk_ret = g_hal_state->gft_hdr_transposition_profile_id_ht()->
                  insert_with_key(&profile->profile_id, entry, &entry->ht_ctxt);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add GFT header transposition profile {} to db, "
                      "err : {}", profile->profile_id, ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    return ret;
}

//------------------------------------------------------------------------------
// delete a GFT header transposition profile from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
gft_htp_del_from_db (gft_hdr_xposition_profile_t *profile)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Removing GFT header transposition profile {} from db",
                    profile->profile_id);

    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)
                g_hal_state->gft_hdr_transposition_profile_id_ht()->
                    remove(&profile->profile_id);

    // free up
    hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate a given GFT header transposition profile config spec
//------------------------------------------------------------------------------
static hal_ret_t
validate_gft_htp_create (GftHeaderTranspositionProfileSpec& spec,
                         GftHeaderTranspositionProfileResponse *rsp)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize GFT header transposition profile object from the config spec
//------------------------------------------------------------------------------
static inline void
gft_htp_init_from_spec (gft_hdr_xposition_profile_t *profile,
                        GftHeaderTranspositionProfileSpec& spec)
{
}

//------------------------------------------------------------------------------
// lookup GFT header transposition profile from key or handle
//------------------------------------------------------------------------------
static gft_hdr_xposition_profile_t *
gft_htp_lookup_key_or_handle (const GftHeaderTranspositionProfileKeyHandle& kh)
{
    gft_hdr_xposition_profile_t    *profile = NULL;

    if (kh.key_or_handle_case() == GftHeaderTranspositionProfileKeyHandle::kProfileId) {
        profile = find_gft_hdr_xposition_profile_by_id(kh.profile_id());
    } else if (kh.key_or_handle_case() ==
               GftHeaderTranspositionProfileKeyHandle::kProfileHandle) {
        profile = find_gft_hdr_xposition_profile_by_handle(kh.profile_handle());
    }

    return profile;
}

//------------------------------------------------------------------------------
// pd call to allocate pd resources and h/w programming
//------------------------------------------------------------------------------
static hal_ret_t
gft_htp_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                      ret = HAL_RET_OK;
    //pd::pd_gft_args_t            pd_gft_args = { 0 };
    dllist_ctxt_t                  *lnode = NULL;
    dhl_entry_t                    *dhl_entry = NULL;
    gft_hdr_xposition_profile_t    *profile = NULL;
    //gft_htp_create_app_ctxt_t    *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("Failed to create GFT hdr transposition profile");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    //app_ctxt = (//gft_htp_create_app_ctxt_t *)cfg_ctxt->app_ctxt;
    profile = (gft_hdr_xposition_profile_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("GFT hdr transposition profile create add callback {}",
                    profile->profile_id);

    // PD Call to allocate PD resources and h/w programming, if any
    //pd::pd_gft_exact_match_profile_create_args_init(&pd_gft_args);
    //pd_l2seg_args.l2seg = l2seg;
    //pd_l2seg_args.vrf = app_ctxt->vrf;
    //ret = pd::hal_pd_call(pd::PD_FUNC_ID_L2SEG_CREATE, (void *)&pd_l2seg_args);
    //if (ret != HAL_RET_OK) {
        //HAL_TRACE_ERR("{}:failed to create l2seg pd, err : {}",
                      //__FUNCTION__, ret);
    //}

end:

    return ret;
}

//------------------------------------------------------------------------------
// update PI databases as gft_htp_create_add_cb was a success
//------------------------------------------------------------------------------
static hal_ret_t
gft_htp_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// gft_htp_create_add_cb was a failure, do the cleanup
//------------------------------------------------------------------------------
hal_ret_t
gft_htp_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// dummy create cleanup callback
//-----------------------------------------------------------------------------
static hal_ret_t
gft_htp_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// convert hal_ret_t to API status and prepare response to the API call
//------------------------------------------------------------------------------
static hal_ret_t
gft_htp_prepare_rsp (GftHeaderTranspositionProfileResponse *rsp,
                     gft_hdr_xposition_profile_t *profile,
                     hal_ret_t ret)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_status()->set_profile_handle(profile->hal_handle);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a GFT header transposition profile create request
//------------------------------------------------------------------------------
hal_ret_t
gft_header_transposition_profile_create (
    GftHeaderTranspositionProfileSpec &spec,
    GftHeaderTranspositionProfileResponse *rsp)
{
    hal_ret_t                      ret;
    gft_hdr_xposition_profile_t    *profile;
    gft_htp_create_app_ctxt_t      app_ctxt; // = { 0 };
    cfg_op_ctxt_t                  cfg_ctxt = { 0 };
    dhl_entry_t                    dhl_entry = { 0 };

    HAL_TRACE_DEBUG("GFT header transposition profile create");
    // validate the request message
    ret = validate_gft_htp_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        // api_status already set, just return
        HAL_TRACE_ERR("GFT header transposition profile object validation failure, "
                      "err : {}", ret);
        return ret;
    }

    HAL_TRACE_DEBUG("Creating GFT header transposition id {}",
                    spec.key_or_handle().profile_id());

    // check if this profile exists already, and reject if one is found
    profile = gft_htp_lookup_key_or_handle(spec.key_or_handle());
    if (profile) {
        HAL_TRACE_ERR("Failed to create GFT header transposition profile, "
                      "profile {} exists already",
                      spec.key_or_handle().profile_id());
        return HAL_RET_ENTRY_EXISTS;
    }

    // instantiate the gft header transposition profile
    profile = gft_hdr_transposition_profile_alloc_init();
    if (profile == NULL) {
        ret = HAL_RET_OOM;
        HAL_TRACE_ERR("Failed to allocate GFT header transposition profile, err : {}",
                      ret);
        goto end;
    }

    // consume the config
    gft_htp_init_from_spec(profile, spec);

    // allocate hal handle id
    profile->hal_handle = hal_handle_alloc(HAL_OBJ_ID_GFT_HDR_TRANSPOSITION_PROFILE);
    if (profile->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("Failed to alloc handle for GFT header transposition profile {}",
                      profile->profile_id);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    // form ctxt and call infra add
    dhl_entry.handle = profile->hal_handle;
    dhl_entry.obj = profile;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(profile->hal_handle, &cfg_ctxt,
                             gft_htp_create_add_cb,
                             gft_htp_create_commit_cb,
                             gft_htp_create_abort_cb,
                             gft_htp_create_cleanup_cb);

end:

    if (ret != HAL_RET_OK) {
        if (profile) {
            profile = NULL;
        }
    }
    gft_htp_prepare_rsp(rsp, profile, ret);
    return ret;
}

//------------------------------------------------------------------------------
// insert a GFT exact match flow entry to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
gft_emfe_add_to_db (gft_exact_match_flow_entry_t *flow_entry,
                    hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Adding GFT exact match flow entry {} to db",
                    flow_entry->flow_entry_id);

    entry = (hal_handle_id_ht_entry_t *)g_hal_state->
                hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from flow entry id to its handle
    entry->handle_id = handle;
    sdk_ret = g_hal_state->gft_exact_match_flow_entry_id_ht()->
                  insert_with_key(&flow_entry->flow_entry_id,
                                  entry, &entry->ht_ctxt);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add GFT exact flow entry {} to db, "
                      "err : {}", flow_entry->flow_entry_id, ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);

    return ret;
}

//------------------------------------------------------------------------------
// delete a GFT exact match flow entry from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
gft_emfe_del_from_db (gft_exact_match_flow_entry_t *flow_entry)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Removing GFT exact match flow entry {} from db",
                    flow_entry->flow_entry_id);

    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)
                g_hal_state->gft_exact_match_flow_entry_id_ht()->
                    remove(&flow_entry->flow_entry_id);

    // free up
    hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate a given GFT exact match flow entry config spec
//------------------------------------------------------------------------------
static hal_ret_t
validate_gft_emfe_create (GftExactMatchFlowEntrySpec& spec,
                          GftExactMatchFlowEntryResponse *rsp)
{
    return HAL_RET_OK;
}

static inline void
gft_emfe_init_flags_from_spec (gft_exact_match_flow_entry_t *flow_entry,
                              GftExactMatchFlowEntrySpec& spec)
{
    GFT_EMFE_SET_FLAGS_FROM_SPEC(flow_entry, spec,
                                 add_in_activated_state, ADD_IN_ACTIVATED_STATE);
    GFT_EMFE_SET_FLAGS_FROM_SPEC(flow_entry, spec,
                                 rdma_flow, RDMA_FLOW);
    GFT_EMFE_SET_FLAGS_FROM_SPEC(flow_entry, spec,
                                 redirect_to_vport_ingress_queue,
                                 REDIRECT_TO_INGRESS_QUEUE_OF_VPORT);
    GFT_EMFE_SET_FLAGS_FROM_SPEC(flow_entry, spec,
                                 redirect_to_vport_egress_queue,
                                 REDIRECT_TO_EGRESS_QUEUE_OF_VPORT);
    GFT_EMFE_SET_FLAGS_FROM_SPEC(flow_entry, spec,
                                 redirect_to_vport_ingress_queue_if_ttl_is_one,
                                 REDIRECT_TO_INGRESS_QUEUE_OF_VPORT_IF_TTL_IS_ONE);
    GFT_EMFE_SET_FLAGS_FROM_SPEC(flow_entry, spec,
                                 redirect_to_vport_egress_queue_if_ttl_is_one,
                                 REDIRECT_TO_EGRESS_QUEUE_OF_VPORT_IF_TTL_IS_ONE);
    GFT_EMFE_SET_FLAGS_FROM_SPEC(flow_entry, spec,
                                 copy_all_packets, COPY_ALL_PACKETS);
    GFT_EMFE_SET_FLAGS_FROM_SPEC(flow_entry, spec,
                                 copy_first_packet, COPY_FIRST_PACKET);
    GFT_EMFE_SET_FLAGS_FROM_SPEC(flow_entry, spec,
                                 copy_when_tcp_flag_set, COPY_WHEN_TCP_FLAG_SET);
    GFT_EMFE_SET_FLAGS_FROM_SPEC(flow_entry, spec,
                                 custom_action_present, CUSTOM_ACTION_PRESENT);
    GFT_EMFE_SET_FLAGS_FROM_SPEC(flow_entry, spec,
                                 meta_action_before_transposition,
                                 META_ACTION_BEFORE_HEADER_TRANSPOSITION);
    GFT_EMFE_SET_FLAGS_FROM_SPEC(flow_entry, spec,
                                 copy_after_tcp_fin_flag_set,
                                 COPY_AFTER_TCP_FIN_FLAG_SET);
    GFT_EMFE_SET_FLAGS_FROM_SPEC(flow_entry, spec,
                                 copy_after_tcp_rst_flag_set,
                                 COPY_AFTER_TCP_RST_FLAG_SET);
}

static inline void
gft_emfe_init_headers_from_spec (uint32_t *headers,
                                 const GftHeaders& headers_spec)
{
    GFT_EMFE_SET_HEADERS_FROM_SPEC(headers, headers_spec,
                                   ethernet_header, ETHERNET);
    GFT_EMFE_SET_HEADERS_FROM_SPEC(headers, headers_spec,
                                   ipv4_header, IPV4);
    GFT_EMFE_SET_HEADERS_FROM_SPEC(headers, headers_spec,
                                   ipv6_header, IPV6);
    GFT_EMFE_SET_HEADERS_FROM_SPEC(headers, headers_spec,
                                   tcp_header, TCP);
    GFT_EMFE_SET_HEADERS_FROM_SPEC(headers, headers_spec,
                                   udp_header, UDP);
    GFT_EMFE_SET_HEADERS_FROM_SPEC(headers, headers_spec,
                                   icmp_header, ICMP);
    GFT_EMFE_SET_HEADERS_FROM_SPEC(headers, headers_spec,
                                   no_encap, NO_ENCAP);
    GFT_EMFE_SET_HEADERS_FROM_SPEC(headers, headers_spec,
                                   ip_in_ip_encap, IP_IN_IP_ENCAP);
    GFT_EMFE_SET_HEADERS_FROM_SPEC(headers, headers_spec,
                                   ip_in_gre_encap, IP_IN_GRE_ENCAP);
    GFT_EMFE_SET_HEADERS_FROM_SPEC(headers, headers_spec,
                                   nvgre_encap, NVGRE_ENCAP);
    GFT_EMFE_SET_HEADERS_FROM_SPEC(headers, headers_spec,
                                   vxlan_encap, VXLAN_ENCAP);
}

static inline void
gft_emfe_init_fields_from_spec (uint64_t *fields,
                                const GftHeaderFields& fields_spec)
{
    GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec,
                                  dst_mac_addr, DST_MAC_ADDR);
    GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec,
                                  src_mac_addr, SRC_MAC_ADDR);
    GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec,
                                  eth_type, ETH_TYPE);
    GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec,
                                  customer_vlan_id, CUSTOMER_VLAN_ID);
    GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec,
                                  provider_vlan_id, PROVIDER_VLAN_ID);
    GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec,
                                  dot1p_priority, 8021P_PRIORITY);
    GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec,
                                  src_ip_addr, SRC_IP_ADDR);
    GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec,
                                  dst_ip_addr, DST_IP_ADDR);
    GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec,
                                  ip_ttl, TTL);
    GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec,
                                  ip_protocol, IP_PROTOCOL);
    GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec,
                                  ip_dscp, IP_DSCP);
    GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec,
                                  src_port, TRANSPORT_SRC_PORT);
    GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec,
                                  dst_port, TRANSPORT_DST_PORT);
    GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec,
                                  tcp_flags, TCP_FLAGS);
    GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec,
                                  tenant_id, TENANT_ID);
    GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec,
                                  icmp_type, ICMP_TYPE);
    GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec,
                                  icmp_code, ICMP_CODE);
    GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec,
                                  oob_vlan, OOB_VLAN);
    GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec,
                                  oob_tenant_id, OOB_TENANT_ID);
    GFT_EMFE_SET_FIELDS_FROM_SPEC(fields, fields_spec,
                                  gre_protocol, GRE_PROTOCOL);
}

static inline void
gft_emfe_init_eth_fields_from_spec (uint32_t headers, uint64_t fields,
                                    gft_eth_fields_t *eth_fields,
                                    const GftEthFields& eth_fields_spec)
{
    if (headers & GFT_HEADER_ETHERNET) {
        if (fields & GFT_HEADER_FIELD_DST_MAC_ADDR) {
            MAC_UINT64_TO_ADDR(eth_fields->dmac,
                               eth_fields_spec.dst_mac_addr());
        }
        if (fields & GFT_HEADER_FIELD_SRC_MAC_ADDR) {
            MAC_UINT64_TO_ADDR(eth_fields->smac,
                               eth_fields_spec.src_mac_addr());
        }
        if (fields & GFT_HEADER_FIELD_ETH_TYPE) {
            eth_fields->eth_type = eth_fields_spec.eth_type();
        }
        if (fields & GFT_HEADER_FIELD_CUSTOMER_VLAN_ID) {
            eth_fields->customer_vlan_id =
                eth_fields_spec.customer_vlan_id();
        }
        if (fields & GFT_HEADER_FIELD_PROVIDER_VLAN_ID) {
            eth_fields->provider_vlan_id =
                eth_fields_spec.provider_vlan_id();
        }
        if (fields & GFT_HEADER_FIELD_8021P_PRIORITY) {
            eth_fields->priority = eth_fields_spec.priority();
        }
    }
}

static inline void
gft_emfe_init_ip_match_fields_from_spec (gft_hdr_group_exact_match_t *gft_em,
                                         GftHeaderGroupExactMatch& em_spec)
{
    if (gft_em->headers & (GFT_HEADER_IPV4 | GFT_HEADER_IPV6)) {
        if (gft_em->fields & GFT_HEADER_FIELD_SRC_IP_ADDR) {
            ip_addr_spec_to_ip_addr(&gft_em->src_ip_addr,
                                    em_spec.src_ip_addr());
        }
        if (gft_em->fields & GFT_HEADER_FIELD_DST_IP_ADDR) {
            ip_addr_spec_to_ip_addr(&gft_em->dst_ip_addr,
                                    em_spec.dst_ip_addr());
        }
        gft_em->ttl = em_spec.ip_ttl();
        gft_em->dscp = em_spec.ip_dscp();
        gft_em->ip_proto = em_spec.ip_protocol();
    }
}

static inline void
gft_emfe_init_ip_xposition_fields_from_spec (gft_hdr_group_xposition_t *gft_ht,
    GftHeaderGroupTransposition& ht_spec)
{
    if (gft_ht->headers & (GFT_HEADER_IPV4 | GFT_HEADER_IPV6)) {
        if (gft_ht->fields & GFT_HEADER_FIELD_SRC_IP_ADDR) {
            ip_addr_spec_to_ip_addr(&gft_ht->src_ip_addr,
                                    ht_spec.src_ip_addr());
        }
        if (gft_ht->fields & GFT_HEADER_FIELD_DST_IP_ADDR) {
            ip_addr_spec_to_ip_addr(&gft_ht->dst_ip_addr,
                                    ht_spec.dst_ip_addr());
        }
        gft_ht->ttl = ht_spec.ip_ttl();
        gft_ht->dscp = ht_spec.ip_dscp();
        gft_ht->ip_proto = ht_spec.ip_protocol();
    }
}

// TODO: take care of vxlan
static inline void
gft_emfe_init_match_encap_fields_from_spec (uint32_t headers, uint64_t fields,
    encap_or_transport_match_t *encap_or_transport,
    const EncapOrTransportMatch& encap_or_transport_spec)

{
    if ((headers & GFT_HEADER_UDP) && encap_or_transport_spec.has_udp_fields()) {
        if (fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) {
            encap_or_transport->udp.sport =
                encap_or_transport_spec.udp_fields().sport();
        }
        if (fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) {
            encap_or_transport->udp.dport =
                encap_or_transport_spec.udp_fields().dport();
        }
    }

    if ((headers & GFT_HEADER_TCP) && encap_or_transport_spec.has_tcp_fields()) {
        if (fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) {
            encap_or_transport->tcp.sport =
                encap_or_transport_spec.tcp_fields().sport();
        }
        if (fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) {
            encap_or_transport->tcp.dport =
                encap_or_transport_spec.tcp_fields().dport();
        }
        if (fields & GFT_HEADER_FIELD_TCP_FLAGS) {
            encap_or_transport->tcp.tcp_flags =
                encap_or_transport_spec.tcp_fields().tcp_flags();
        }
    }

    if ((headers & GFT_HEADER_ICMP) && encap_or_transport_spec.has_icmp_fields()) {
        if (fields & GFT_HEADER_FIELD_ICMP_TYPE) {
            encap_or_transport->icmp.type =
                encap_or_transport_spec.icmp_fields().type();
        }
        if (fields & GFT_HEADER_FIELD_ICMP_CODE) {
            encap_or_transport->icmp.code =
                encap_or_transport_spec.icmp_fields().code();
        }
    }
}

// TODO: take care of vxlan
static inline void
gft_emfe_init_xposition_encap_fields_from_spec (uint32_t headers,
    uint64_t fields,
    encap_or_transport_xposition_t *encap_or_transport,
    const EncapOrTransportTransposition& encap_or_transport_spec)

{
    if ((headers & GFT_HEADER_UDP) && encap_or_transport_spec.has_udp_fields()) {
        if (fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) {
            encap_or_transport->udp.sport =
                encap_or_transport_spec.udp_fields().sport();
        }
        if (fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) {
            encap_or_transport->udp.dport =
                encap_or_transport_spec.udp_fields().dport();
        }
    }

    if ((headers & GFT_HEADER_TCP) && encap_or_transport_spec.has_tcp_fields()) {
        if (fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) {
            encap_or_transport->tcp.sport =
                encap_or_transport_spec.tcp_fields().sport();
        }
        if (fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) {
            encap_or_transport->tcp.dport =
                encap_or_transport_spec.tcp_fields().dport();
        }
    }
}

//------------------------------------------------------------------------------
// initialize GFT exact match flow entry object from the config spec
//------------------------------------------------------------------------------
static inline void
gft_emfe_init_from_spec (gft_exact_match_flow_entry_t *flow_entry,
                         GftExactMatchFlowEntrySpec& spec)
{
    uint32_t                          i;
    gft_exact_match_profile_t         *gft_emp;
    // gft_hdr_xposition_profile_t       *gft_htp;
    gft_hdr_group_exact_match_t       *gft_em;
    gft_hdr_group_xposition_t         *gft_ht;

    HAL_ASSERT(spec.key_or_handle().key_or_handle_case() ==
                   GftExactMatchFlowEntryKeyHandle::kFlowEntryId);
    gft_emp = gft_emp_lookup_key_or_handle(spec.exact_match_profile());
    HAL_ASSERT(gft_emp);

#if 0
    // No need of xposition profile
    gft_htp = gft_htp_lookup_key_or_handle(spec.transposition_profile());
    HAL_ASSERT(gft_htp);
#endif

    flow_entry->num_exact_matches = spec.exact_matches_size();
    HAL_ASSERT(flow_entry->num_exact_matches != 0);
    flow_entry->exact_matches =
        (gft_hdr_group_exact_match_t *)HAL_CALLOC(HAL_MEM_ALLOC_GFT_EXACT_MATCH,
                                                  flow_entry->num_exact_matches *
                                                  sizeof(gft_hdr_group_exact_match_t));
    HAL_ASSERT(flow_entry->exact_matches != NULL);

    flow_entry->num_transpositions = spec.transpositions_size();
    HAL_ASSERT(flow_entry->num_transpositions != 0);
    flow_entry->transpositions =
        (gft_hdr_group_xposition_t *)HAL_CALLOC(HAL_MEM_ALLOC_GFT_HDR_GROUP_TRANSPOSITION,
                                                flow_entry->num_transpositions *
                                                sizeof(gft_hdr_group_xposition_t));
    HAL_ASSERT(flow_entry->transpositions != NULL);

    flow_entry->flow_entry_id = spec.key_or_handle().flow_entry_id();

    // set the flags field
    gft_emfe_init_flags_from_spec(flow_entry, spec);
    flow_entry->gft_emp_hal_handle = gft_emp->hal_handle;
    // No need of xposition profile
    // flow_entry->gft_htp_hal_handle = gft_htp->hal_handle;
    flow_entry->table_id = spec.table_id();
    flow_entry->vport_id = spec.vport_id();
    flow_entry->redirect_vport_id = spec.redirect_vport_id();
    flow_entry->ttl_one_redirect_vport_id = spec.ttl_one_redirect_vport_id();

    // initialize exact matches
    for (i = 0; i < flow_entry->num_exact_matches; i++) {
        auto em_spec = spec.exact_matches(i);
        gft_em = &flow_entry->exact_matches[i];
        gft_em->flags = 0;

        // set the headers bitmap
        gft_emfe_init_headers_from_spec(&gft_em->headers, em_spec.headers());

        // set fields bitmap
        gft_emfe_init_fields_from_spec(&gft_em->fields, em_spec.match_fields());

        // set eth fields
        gft_emfe_init_eth_fields_from_spec(gft_em->headers,
                                           gft_em->fields,
                                           &gft_em->eth_fields,
                                           em_spec.eth_fields());

        // set IP fields
        gft_emfe_init_ip_match_fields_from_spec(gft_em, em_spec);

        // set encap or transport fields
        gft_emfe_init_match_encap_fields_from_spec(gft_em->headers,
            gft_em->fields, &gft_em->encap_or_transport,
            em_spec.encap_or_transport());

    }

    // initialize transpositions
    for (i = 0; i < flow_entry->num_transpositions;  i++) {
        auto ht_spec = spec.transpositions(i);
        gft_ht = &flow_entry->transpositions[i];
        gft_ht->flags = 0;
        gft_ht->action =
            static_cast<gft_hdr_group_xposition_action_t>(ht_spec.action());

        // set headers bitmap
        gft_emfe_init_headers_from_spec(&gft_ht->headers, ht_spec.headers());

        // set fields bitmap
        gft_emfe_init_fields_from_spec(&gft_ht->fields,
                                       ht_spec.header_fields());

        // set eth fields
        gft_emfe_init_eth_fields_from_spec(gft_ht->headers,
                                           gft_ht->fields,
                                           &gft_ht->eth_fields,
                                           ht_spec.eth_fields());
        // set IP fields
        gft_emfe_init_ip_xposition_fields_from_spec(gft_ht, ht_spec);

        // set encap or transport fields
        gft_emfe_init_xposition_encap_fields_from_spec(gft_ht->headers,
            gft_ht->fields, &gft_ht->encap_or_transport,
            ht_spec.encap_or_transport());
    }
}

//------------------------------------------------------------------------------
// lookup GFT exact match flow entry from key or handle
//------------------------------------------------------------------------------
static gft_exact_match_flow_entry_t *
gft_emfe_lookup_key_or_handle (const GftExactMatchFlowEntryKeyHandle& kh)
{
    gft_exact_match_flow_entry_t    *flow_entry = NULL;

    if (kh.key_or_handle_case() == GftExactMatchFlowEntryKeyHandle::kFlowEntryId) {
        flow_entry = find_gft_exact_match_flow_entry_by_id(kh.flow_entry_id());
    } else if (kh.key_or_handle_case() ==
               GftExactMatchFlowEntryKeyHandle::kFlowEntryHandle) {
        flow_entry = find_gft_exact_match_flow_entry_by_handle(kh.flow_entry_handle());
    }

    return flow_entry;
}

//------------------------------------------------------------------------------
// pd call to allocate pd resources and h/w programming for GFT exact match
// flow entry
//------------------------------------------------------------------------------
static hal_ret_t
gft_emfe_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                                   ret = HAL_RET_OK;
    pd::pd_gft_exact_match_flow_entry_args_t    pd_args = { 0 };
    dllist_ctxt_t                               *lnode = NULL;
    dhl_entry_t                                 *dhl_entry = NULL;
    gft_exact_match_flow_entry_t                *flow_entry = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("Failed to create GFT exact match flow entry");
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    flow_entry = (gft_exact_match_flow_entry_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("GFT exact match flow entry create add callback {}",
                    flow_entry->flow_entry_id);

    // PD Call to allocate PD resources and h/w programming, if any
    pd_args.exact_match_profile = NULL;
    pd_args.hdr_xposition_profile = NULL;
    pd_args.exact_match_flow_entry = flow_entry;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_GFT_EXACT_MATCH_FLOW_ENTRY_CREATE,
                          (void *)&pd_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create exact match flow entry pd, err : {}",
                      ret);
    }

end:

    return ret;
}

//------------------------------------------------------------------------------
// update PI databases as gft_emfe_create_add_cb was a success
//------------------------------------------------------------------------------
static hal_ret_t
gft_emfe_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// gft_emfe_create_add_cb was a failure, do the cleanup
//------------------------------------------------------------------------------
hal_ret_t
gft_emfe_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// dummy create cleanup callback
//-----------------------------------------------------------------------------
static hal_ret_t
gft_emfe_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// convert hal_ret_t to API status and prepare response to the API call
//------------------------------------------------------------------------------
static hal_ret_t
gft_emfe_prepare_rsp (GftExactMatchFlowEntryResponse *rsp,
                      gft_exact_match_flow_entry_t *flow_entry,
                      hal_ret_t ret)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_status()->set_flow_entry_handle(flow_entry->hal_handle);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a GFT exact match flow entry create request
//------------------------------------------------------------------------------
hal_ret_t
gft_exact_match_flow_entry_create (GftExactMatchFlowEntrySpec &spec,
                                   GftExactMatchFlowEntryResponse *rsp)
{
    hal_ret_t                       ret;
    gft_exact_match_flow_entry_t    *flow_entry;
    gft_emfe_create_app_ctxt_t      app_ctxt; // = { 0 };
    cfg_op_ctxt_t                   cfg_ctxt = { 0 };
    dhl_entry_t                     dhl_entry = { 0 };

    hal_api_trace(" API Start: GFT EMF Create");

    // validate the request message
    ret = validate_gft_emfe_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        // api_status already set, just return
        HAL_TRACE_ERR("GFT exact match flow entry object validation failure, "
                      "err : {}", ret);
        return ret;
    }

    HAL_TRACE_DEBUG("Creating GFT exact match flow entry id {}",
                    spec.key_or_handle().flow_entry_id());

    // check if this flow entry exists already, and reject if one is found
    flow_entry = gft_emfe_lookup_key_or_handle(spec.key_or_handle());
    if (flow_entry) {
        HAL_TRACE_ERR("Failed to create GFT exact match flow entry {} "
                      "exists already", spec.key_or_handle().flow_entry_id());
        return HAL_RET_ENTRY_EXISTS;
    }

    // instantiate the gft exact match flow entry 
    flow_entry = gft_exact_match_flow_entry_alloc_init();
    if (flow_entry == NULL) {
        ret = HAL_RET_OOM;
        HAL_TRACE_ERR("Failed to allocate GFT exact match flow entry, err : {}",
                      ret);
        goto end;
    }

    // consume the config
    gft_emfe_init_from_spec(flow_entry, spec);

    // allocate hal handle id
    flow_entry->hal_handle =
        hal_handle_alloc(HAL_OBJ_ID_GFT_EXACT_MATCH_FLOW_ENTRY);
    if (flow_entry->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("Failed to alloc handle for GFT exact match flow entry {}",
                      flow_entry->flow_entry_id);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    gft_exact_match_flow_entry_print(flow_entry);

    // form ctxt and call infra add
    dhl_entry.handle = flow_entry->hal_handle;
    dhl_entry.obj = flow_entry;
    cfg_ctxt.app_ctxt = &app_ctxt;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(flow_entry->hal_handle, &cfg_ctxt,
                             gft_emfe_create_add_cb,
                             gft_emfe_create_commit_cb,
                             gft_emfe_create_abort_cb,
                             gft_emfe_create_cleanup_cb);

end:

    if (ret != HAL_RET_OK) {
        if (flow_entry) {
            flow_entry = NULL;
        }
    }
    gft_emfe_prepare_rsp(rsp, flow_entry, ret);
    hal_api_trace(" API End: GFT EMF Create");
    return ret;
}

#define GFT_HDRS_FIELDS(VAR)                                                \
    if (VAR->headers & GFT_HEADER_ETHERNET) {                               \
        HAL_TRACE_DEBUG("Ethernet");                                        \
        if (VAR->fields & GFT_HEADER_FIELD_SRC_MAC_ADDR) {                  \
            HAL_TRACE_DEBUG("Src Mac: {}", macaddr2str(VAR->eth_fields.smac));\
        }                                                                   \
        if (VAR->fields & GFT_HEADER_FIELD_DST_MAC_ADDR) {                  \
            HAL_TRACE_DEBUG("Dst Mac: {}", macaddr2str(VAR->eth_fields.dmac));\
        }                                                                   \
        if (VAR->fields & GFT_HEADER_FIELD_ETH_TYPE) {                      \
            HAL_TRACE_DEBUG("Eth Type: {}", VAR->eth_fields.eth_type);      \
        }                                                                   \
        if (VAR->fields & GFT_HEADER_FIELD_CUSTOMER_VLAN_ID) {              \
            HAL_TRACE_DEBUG("CVlan: {}", VAR->eth_fields.customer_vlan_id); \
        }                                                                   \
    }                                                                       \
    if (VAR->headers & GFT_HEADER_IPV4 ||                                   \
        VAR->headers & GFT_HEADER_IPV6) {                                   \
        HAL_TRACE_DEBUG("{}",                                               \
                        (VAR->headers & GFT_HEADER_IPV4) ? "IPv4" : "IPv6");\
        if (VAR->fields & GFT_HEADER_FIELD_SRC_IP_ADDR) {                   \
            HAL_TRACE_DEBUG("Src IP: {}", ipaddr2str(&VAR->src_ip_addr));   \
        }                                                                   \
        if (VAR->fields & GFT_HEADER_FIELD_DST_IP_ADDR) {                   \
            HAL_TRACE_DEBUG("Dst IP: {}", ipaddr2str(&VAR->dst_ip_addr));   \
        }                                                                   \
        if (VAR->fields & GFT_HEADER_FIELD_IP_DSCP) {                       \
            HAL_TRACE_DEBUG("DSCP: {}",VAR->dscp );                         \
        }                                                                   \
        if (VAR->fields & GFT_HEADER_FIELD_IP_PROTOCOL) {                   \
            HAL_TRACE_DEBUG("IP Proto: {}",VAR->ip_proto);                  \
        }                                                                   \
        if (VAR->fields & GFT_HEADER_FIELD_TTL) {                           \
            HAL_TRACE_DEBUG("TTL: {}", VAR->ttl);                           \
        }                                                                   \
    }                                                                       \
    if (VAR->headers & GFT_HEADER_ICMP) {                                   \
        HAL_TRACE_DEBUG("ICMP");                                            \
        if (VAR->fields & GFT_HEADER_FIELD_ICMP_TYPE) {                     \
            HAL_TRACE_DEBUG("Type: {}", VAR->encap_or_transport.icmp.type); \
        }                                                                   \
        if (VAR->fields & GFT_HEADER_FIELD_ICMP_CODE) {                     \
            HAL_TRACE_DEBUG("Code: {}", VAR->encap_or_transport.icmp.code); \
        }                                                                   \
    }                                                                       \
    if (VAR->headers & GFT_HEADER_TCP) {                                    \
        HAL_TRACE_DEBUG("TCP");                                             \
        if (VAR->fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) {            \
            HAL_TRACE_DEBUG("Sport: {}", VAR->encap_or_transport.tcp.sport);\
        }                                                                   \
        if (VAR->fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) {            \
            HAL_TRACE_DEBUG("Dport: {}", VAR->encap_or_transport.tcp.dport);\
        }                                                                   \
        if (VAR->fields & GFT_HEADER_FIELD_TCP_FLAGS) {                     \
            HAL_TRACE_DEBUG("Dport: {}", VAR->encap_or_transport.tcp.tcp_flags);\
        }                                                                   \
    }                                                                       \
    if (VAR->headers & GFT_HEADER_UDP) {                                    \
        HAL_TRACE_DEBUG("UDP");                                             \
        if (VAR->fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) {            \
            HAL_TRACE_DEBUG("Sport: {}", VAR->encap_or_transport.udp.sport);\
        }                                                                   \
        if (VAR->fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) {            \
            HAL_TRACE_DEBUG("Dport: {}", VAR->encap_or_transport.udp.dport);\
        }                                                                   \
    }                                                                       \
    if (VAR->headers & GFT_HEADER_IP_IN_GRE_ENCAP) {                        \
        HAL_TRACE_DEBUG("GRE");                                             \
        if (VAR->fields & GFT_HEADER_FIELD_GRE_PROTOCOL) {                  \
            HAL_TRACE_DEBUG("Proto: {}", VAR->encap_or_transport.encap.gre_protocol);\
        }                                                                   \
    }                                                                       \
    if (VAR->headers & GFT_HEADER_VXLAN_ENCAP) {                            \
        HAL_TRACE_DEBUG("Vxlan");                                           \
        if (VAR->fields & GFT_HEADER_FIELD_TENANT_ID) {                     \
            HAL_TRACE_DEBUG("Tenant Id: {}", VAR->encap_or_transport.encap.tenant_id);\
        }                                                                   \
    }                                                                       

    




#if 0
        if (VAR ## ->fields & GFT_HEADER_FIELD_SRC_MAC_ADDR) {              \
            HAL_TRACE_DEBUG("Src IP: {}", ipaddr2str(&VAR->src_ip_addr));   \
        }                                                                   \
        if (VAR ## ->fields & GFT_HEADER_FIELD_DST_MAC_ADDR) {              \
            HAL_TRACE_DEBUG("Dst IP: {}", ipaddr2str(&VAR->dst_ip_addr));   \
        }                                                                   \
        if (VAR ## ->fields & GFT_HEADER_FIELD_ETH_TYPE) {              \
            HAL_TRACE_DEBUG("Eth Type: {}", VAR->);   \
        }                                               
#endif


void
gft_exact_match_flow_entry_print (gft_exact_match_flow_entry_t *fe)
{
    gft_hdr_group_exact_match_t *ghem;
    gft_hdr_group_xposition_t   *xpos;

    if (!fe) {
        return;
    }

    HAL_TRACE_DEBUG("Flow entry id: {}", fe->flow_entry_id);
    HAL_TRACE_DEBUG("EMP hal_handle: {}, HXP hal_handle: {}",
                    fe->gft_emp_hal_handle, fe->gft_htp_hal_handle);
    HAL_TRACE_DEBUG("table_id: {}, vport_id: {}, redirect_vport_id: {}"
                    "ttl_one_redirect_vport_id: {}, cache_hint: {}", 
                    fe->table_id, fe->vport_id, fe->redirect_vport_id,
                    fe->ttl_one_redirect_vport_id,
                    fe->cache_hint);
    HAL_TRACE_DEBUG("num_exact_matches: {}, num_transpositions: {}",
                    fe->num_exact_matches, fe->num_transpositions);
    HAL_TRACE_DEBUG("Match Profiles:");
    ghem = fe->exact_matches;
    for (uint32_t i = 0; i < fe->num_exact_matches; i++) {
        HAL_TRACE_DEBUG("Match Profile {}:", i);
        HAL_TRACE_DEBUG("flags: {}", ghem->flags);
        GFT_HDRS_FIELDS(ghem);
        ghem++;
    }

    HAL_TRACE_DEBUG("Transpositions:");
    xpos = fe->transpositions;
    for (uint32_t i = 0; i < fe->num_transpositions; i++) {
        HAL_TRACE_DEBUG("Transposition {}:", i);
        HAL_TRACE_DEBUG("flags: {}", xpos->flags);
        HAL_TRACE_DEBUG("action: {}", xpos->action);
        GFT_HDRS_FIELDS(xpos);
        xpos++;
    }
}

#define GFT_HDRS_MATCH_FIELDS(VAR)                                          \
    if (VAR->headers & GFT_HEADER_ETHERNET) {                               \
        HAL_TRACE_DEBUG("Ethernet");                                        \
        if (VAR->match_fields & GFT_HEADER_FIELD_SRC_MAC_ADDR) {            \
            HAL_TRACE_DEBUG("Src Mac");                                     \
        }                                                                   \
        if (VAR->match_fields & GFT_HEADER_FIELD_DST_MAC_ADDR) {            \
            HAL_TRACE_DEBUG("Dst Mac");                                     \
        }                                                                   \
        if (VAR->match_fields & GFT_HEADER_FIELD_ETH_TYPE) {                \
            HAL_TRACE_DEBUG("Eth Type");                                    \
        }                                                                   \
        if (VAR->match_fields & GFT_HEADER_FIELD_CUSTOMER_VLAN_ID) {        \
            HAL_TRACE_DEBUG("CVlan");                                       \
        }                                                                   \
    }                                                                       \
    if (VAR->headers & GFT_HEADER_IPV4 ||                                   \
        VAR->headers & GFT_HEADER_IPV6) {                                   \
        HAL_TRACE_DEBUG("{}",                                               \
                        (VAR->headers & GFT_HEADER_IPV4) ? "IPv4" : "IPv6");\
        if (VAR->match_fields & GFT_HEADER_FIELD_SRC_IP_ADDR) {             \
            HAL_TRACE_DEBUG("Src IP");                                      \
        }                                                                   \
        if (VAR->match_fields & GFT_HEADER_FIELD_DST_IP_ADDR) {             \
            HAL_TRACE_DEBUG("Dst IP");                                      \
        }                                                                   \
        if (VAR->match_fields & GFT_HEADER_FIELD_IP_DSCP) {                 \
            HAL_TRACE_DEBUG("DSCP");                                        \
        }                                                                   \
        if (VAR->match_fields & GFT_HEADER_FIELD_IP_PROTOCOL) {             \
            HAL_TRACE_DEBUG("IP Proto");                                    \
        }                                                                   \
        if (VAR->match_fields & GFT_HEADER_FIELD_TTL) {                     \
            HAL_TRACE_DEBUG("TTL");                                         \
        }                                                                   \
    }                                                                       \
    if (VAR->headers & GFT_HEADER_ICMP) {                                   \
        HAL_TRACE_DEBUG("ICMP");                                            \
        if (VAR->match_fields & GFT_HEADER_FIELD_ICMP_TYPE) {               \
            HAL_TRACE_DEBUG("Type");                                        \
        }                                                                   \
        if (VAR->match_fields & GFT_HEADER_FIELD_ICMP_CODE) {               \
            HAL_TRACE_DEBUG("Code");                                        \
        }                                                                   \
    }                                                                       \
    if (VAR->headers & GFT_HEADER_TCP) {                                    \
        HAL_TRACE_DEBUG("TCP");                                             \
        if (VAR->match_fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) {      \
            HAL_TRACE_DEBUG("Sport");                                       \
        }                                                                   \
        if (VAR->match_fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) {      \
            HAL_TRACE_DEBUG("Dport");                                       \
        }                                                                   \
        if (VAR->match_fields & GFT_HEADER_FIELD_TCP_FLAGS) {               \
            HAL_TRACE_DEBUG("Dport");                                       \
        }                                                                   \
    }                                                                       \
    if (VAR->headers & GFT_HEADER_UDP) {                                    \
        HAL_TRACE_DEBUG("UDP");                                             \
        if (VAR->match_fields & GFT_HEADER_FIELD_TRANSPORT_SRC_PORT) {      \
            HAL_TRACE_DEBUG("Sport");                                       \
        }                                                                   \
        if (VAR->match_fields & GFT_HEADER_FIELD_TRANSPORT_DST_PORT) {      \
            HAL_TRACE_DEBUG("Dport");                                       \
        }                                                                   \
    }                                                                       \
    if (VAR->headers & GFT_HEADER_IP_IN_GRE_ENCAP) {                        \
        HAL_TRACE_DEBUG("GRE");                                             \
        if (VAR->match_fields & GFT_HEADER_FIELD_GRE_PROTOCOL) {            \
            HAL_TRACE_DEBUG("Proto");                                       \
        }                                                                   \
    }                                                                       \
    if (VAR->headers & GFT_HEADER_VXLAN_ENCAP) {                            \
        HAL_TRACE_DEBUG("Vxlan");                                           \
        if (VAR->match_fields & GFT_HEADER_FIELD_TENANT_ID) {               \
            HAL_TRACE_DEBUG("Tenant Id");                                   \
        }                                                                   \
    }                                                                       

void
gft_exact_match_profile_print (gft_exact_match_profile_t *mp)
{
    gft_hdr_group_exact_match_profile_t *hgmp;

    if (!mp) {
        return;
    }

    HAL_TRACE_DEBUG("ExactMatchProfile: {}", mp->profile_id);
    HAL_TRACE_DEBUG("table_type: {}", mp->table_type);

    HAL_TRACE_DEBUG("num_hdr_groups: {}", 
                    mp->num_hdr_group_exact_match_profiles);
    hgmp = mp->hgem_profiles;
    for (uint32_t i = 0; i < mp->num_hdr_group_exact_match_profiles; i++) {
        HAL_TRACE_DEBUG("Headers to match for hdr_group{}:", i);
        GFT_HDRS_MATCH_FIELDS(hgmp);
    }
}

}    // namespace hal
