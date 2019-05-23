//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of service mapping
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/service.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/impl/artemis/pds_impl_state.hpp"
#include "nic/apollo/api/impl/artemis/artemis_impl.hpp"
#include "nic/apollo/api/impl/artemis/service_impl.hpp"
#include "nic/apollo/p4/include/defines.h"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "gen/p4gen/artemis/include/p4pd.h"

using sdk::table::sdk_table_api_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_SVC_MAPPING_IMPL - service mapping datapath implementation
/// \ingroup PDS_SERVICE
/// @{

#define PDS_IMPL_FILL_SVC_MAPPING_SWKEY(key, vpc_hw_id, vip_or_dip, svc_port,  \
                                        provider_ip)                           \
{                                                                              \
    (key)->key_metadata_mapping_port = svc_port;                               \
    if ((vip_or_dip)->af == IP_AF_IPV6) {                                      \
        sdk::lib::memrev((key)->key_metadata_mapping_ip,                       \
                         (vip_or_dip)->addr.v6_addr.addr8, IP6_ADDR8_LEN);     \
        if (provider_ip) {                                                     \
            sdk::lib::memrev((key)->key_metadata_mapping_ip2,                  \
                             (provider_ip)->addr.v6_addr.addr8, IP6_ADDR8_LEN);\
        }                                                                      \
    } else {                                                                   \
        /* key is initialized to zero by the caller */                         \
        memcpy((key)->key_metadata_mapping_ip,                                 \
               &(vip_or_dip)->addr.v4_addr, IP4_ADDR8_LEN);                    \
        if (provider_ip) {                                                     \
            memcpy((key)->key_metadata_mapping_ip2,                            \
                   &(provider_ip)->addr.v4_addr, IP4_ADDR8_LEN);               \
        }                                                                      \
    }                                                                          \
    (key)->vnic_metadata_vpc_id = vpc_hw_id;                                   \
}

#define local_ip_mapping_action    action_u.local_ip_mapping_local_ip_mapping_info
#define PDS_IMPL_FILL_LOCAL_IP_MAPPING_APPDATA(data, vnic_hw_id, vpc_hw_id,  \
                                               svc_tag, xidx1, xidx2)        \
{                                                                            \
    (data)->local_ip_mapping_action.vnic_id = (vnic_hw_id);                  \
    (data)->local_ip_mapping_action.vpc_id = (vpc_hw_id);                    \
    (data)->local_ip_mapping_action.service_tag = (svc_tag);                 \
    (data)->local_ip_mapping_action.pa_or_ca_xlate_idx= (uint16_t)xidx1;     \
    (data)->local_ip_mapping_action.public_xlate_idx = (uint16_t)xidx2;      \
}

svc_mapping_impl *
svc_mapping_impl::factory(pds_svc_mapping_spec_t *mapping) {
    svc_mapping_impl    *impl;

    impl = svc_mapping_impl_db()->alloc();
    if (unlikely(impl == NULL)) {
        return NULL;
    }
    return impl;
}

void
svc_mapping_impl::soft_delete(svc_mapping_impl *impl) {
    impl->~svc_mapping_impl();
    svc_mapping_impl_db()->free(impl);
}

void
svc_mapping_impl::destroy(svc_mapping_impl *impl) {
    svc_mapping_impl::soft_delete(impl);
}

svc_mapping_impl *
svc_mapping_impl::build(pds_svc_mapping_key_t *key) {
    return NULL;
}

sdk_ret_t
svc_mapping_impl::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    vpc_entry *vpc;
    pds_svc_mapping_spec_t *spec;
    sdk_table_api_params_t api_params;
    service_mapping_swkey_t svc_mapping_key = { 0 };

    spec = &obj_ctxt->api_params->svc_mapping_spec;
    vpc = vpc_db()->find(&spec->key.vpc);

    // reserve an entry in SERVICE_MAPPING with (VIP, port) as key
    PDS_IMPL_FILL_SVC_MAPPING_SWKEY(&svc_mapping_key,
                                    vpc->hw_id(), &spec->key.vip,
                                    spec->key.svc_port,
                                    &spec->backend_provider_ip);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &svc_mapping_key,
                                   NULL, 0, sdk::table::handle_t::null());
    ret = svc_mapping_impl_db()->svc_mapping_tbl()->reserve(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve (vip, pa-ip, port) entry in "
                      "SERVICE_MAPPING table for mapping %s, err %u",
                      orig_obj->key2str().c_str(), ret);
        return ret;
    }
    vip_to_dip_handle_ = api_params.handle;

    // reserve an entry in SERVICE_MAPPING with (overlay_ip, port) as key
    vpc = vpc_db()->find(&spec->vpc);
    memset(&svc_mapping_key, 0, sizeof(svc_mapping_key));
    PDS_IMPL_FILL_SVC_MAPPING_SWKEY(&svc_mapping_key,
                                    vpc->hw_id(), &spec->backend_ip,
                                    spec->svc_port, (ip_addr_t *)NULL);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &svc_mapping_key,
                                   NULL, 0, sdk::table::handle_t::null());
    ret = svc_mapping_impl_db()->svc_mapping_tbl()->reserve(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve (backend-ip, port) entry in "
                      "SERVICE_MAPPING table for mapping %s, err %u",
                      orig_obj->key2str().c_str(), ret);
        return ret;
    }
    dip_to_vip_handle_ = api_params.handle;
    return SDK_RET_OK;
}

sdk_ret_t
svc_mapping_impl::nuke_resources(api_base *api_obj) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
svc_mapping_impl::release_resources(api_base *api_obj) {
    return SDK_RET_OK;
}

sdk_ret_t
svc_mapping_impl::program_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
svc_mapping_impl::cleanup_hw(api_base *api_obj, obj_ctxt_t *obj_ctxt) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
svc_mapping_impl::update_hw(api_base *curr_obj, api_base *prev_obj,
                        obj_ctxt_t *obj_ctxt) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
svc_mapping_impl::activate_hw(api_base *api_obj, pds_epoch_t epoch,
                          api_op_t api_op, obj_ctxt_t *obj_ctxt) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
svc_mapping_impl::read_hw(obj_key_t *key, obj_info_t *info, void *arg) {
    return SDK_RET_INVALID_OP;
}

/// \@}    // end of PDS_SVC_MAPPING_IMPL

}    // namespace impl
}    // namespace api
