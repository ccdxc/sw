//
// Copyright (c) 2019 Pensando Systems, Inc.
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of mapping
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/mapping.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/impl/artemis/pds_impl_state.hpp"
#include "nic/apollo/api/impl/artemis/vnic_impl.hpp"
#include "nic/apollo/api/impl/artemis/mapping_impl.hpp"
#include "nic/apollo/api/impl/artemis/nexthop_impl.hpp"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"
#include "nic/sdk/platform/capri/capri_p4.hpp"

using sdk::table::sdk_table_api_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_MAPPING_IMPL - mapping entry datapath implementation
/// \ingroup PDS_MAPPING
/// @{

// TODO: IP address type (i.e., v4 or v6 bit) is not part of the key
#define PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(key, vpc_hw_id, ip)             \
{                                                                            \
    memset((key), 0, sizeof(*(key)));                                        \
    (key)->vnic_metadata_vpc_id = vpc_hw_id;                                 \
    if ((ip)->af == IP_AF_IPV6) {                                            \
        sdk::lib::memrev((key)->key_metadata_mapping_ip,                     \
                         (ip)->addr.v6_addr.addr8, IP6_ADDR8_LEN);           \
    } else {                                                                 \
        memcpy((key)->key_metadata_mapping_ip,                               \
               &(ip)->addr.v4_addr, IP4_ADDR8_LEN);                          \
    }                                                                        \
}

#define local_ip_mapping_action    action_u.local_ip_mapping_local_ip_mapping_info
#define PDS_IMPL_FILL_LOCAL_IP_MAPPING_APPDATA(data, vnic_hw_id, vpc_hw_id,  \
                                               svc_tag, xidx1, xidx2,        \
                                               nat46_xidx)                   \
{                                                                            \
    memset((data), 0, sizeof(*(data)));                                      \
    (data)->action_id = LOCAL_IP_MAPPING_LOCAL_IP_MAPPING_INFO_ID;           \
    (data)->local_ip_mapping_action.vnic_id = (vnic_hw_id);                  \
    (data)->local_ip_mapping_action.vpc_id = (vpc_hw_id);                    \
    (data)->local_ip_mapping_action.service_tag = (svc_tag);                 \
    (data)->local_ip_mapping_action.pa_or_ca_xlate_idx= (uint16_t)xidx1;     \
    (data)->local_ip_mapping_action.public_xlate_idx = (uint16_t)xidx2;      \
    (data)->local_ip_mapping_action.ca6_xlate_idx = (uint8_t)nat46_xidx;     \
}

#define PDS_IMPL_FILL_MAPPING_SWKEY(key, vpc_hw_id, ip)                      \
{                                                                            \
    memset((key), 0, sizeof(*(key)));                                        \
    (key)->rx_to_tx_hdr_vpc_id = vpc_hw_id;                                 \
    if ((ip)->af == IP_AF_IPV6) {                                            \
        sdk::lib::memrev((key)->rx_to_tx_hdr_remote_ip,                     \
                         (ip)->addr.v6_addr.addr8, IP6_ADDR8_LEN);           \
    } else {                                                                 \
        memcpy((key)->rx_to_tx_hdr_remote_ip,                               \
               &(ip)->addr.v4_addr, IP4_ADDR8_LEN);                          \
    }                                                                        \
}

#define PDS_IMPL_FILL_MAPPING_APPDATA(data, nh_id)                           \
{                                                                            \
    memset((data), 0, sizeof(*(data)));                                      \
    (data)->nexthop_group_index = (nh_id);                                   \
}

mapping_impl *
mapping_impl::factory(pds_mapping_spec_t *spec) {
    mapping_impl    *impl;
    device_entry    *device;

    impl = mapping_impl_db()->alloc();
    if (unlikely(impl == NULL)) {
        return NULL;
    }
    new (impl) mapping_impl();
    device = device_db()->find();
    impl->is_local_ = spec->is_local;
    return impl;
}

void
mapping_impl::soft_delete(mapping_impl *impl) {
    impl->~mapping_impl();
    mapping_impl_db()->free(impl);
}

void
mapping_impl::destroy(mapping_impl *impl) {
    mapping_impl::soft_delete(impl);
}

void
mapping_impl::set_is_local(bool val) {
    is_local_ = val;
}

mapping_impl *
mapping_impl::build(pds_mapping_key_t *key, mapping_entry *mapping) {
    sdk_ret_t ret;
    vpc_entry *vpc;
    mapping_impl *impl;
    uint16_t vnic_hw_id;
    bool local_mapping = false;
    sdk_table_api_params_t api_params;
    local_ip_mapping_swkey_t local_ip_mapping_key;
    local_ip_mapping_actiondata_t local_ip_mapping_data;

    vpc = vpc_db()->find(&key->vpc);
    if (unlikely(vpc == NULL)) {
        return NULL;
    }

    impl = mapping_impl_db()->alloc();
    if (unlikely(impl == NULL)) {
        return NULL;
    }
    new (impl) mapping_impl();
    return NULL;
}

sdk_ret_t
mapping_impl::reserve_remote_mapping_resources_(api_base *api_obj,
                                                vpc_entry *vpc,
                                                pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    mapping_swkey_t mapping_key;
    sdk_table_api_params_t api_params;

    // reserve an entry in the MAPPING table
    PDS_IMPL_FILL_MAPPING_SWKEY(&mapping_key, vpc->hw_id(),
                                &spec->key.ip_addr);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &mapping_key, NULL,
                                   NULL, 0, sdk::table::handle_t::null());
    ret = mapping_impl_db()->mapping_tbl()->reserve(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in MAPPING table for "
                      "mapping %s, err %u", api_obj->key2str().c_str(), ret);
        goto error;
    }
    mapping_hdl_ = api_params.handle;

    // reserve an entry in NEXTHOP table
    ret = nexthop_impl_db()->nh_tbl()->reserve(&nh_idx_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in NH table, err %u", ret);
        return ret;
    }
    return SDK_RET_OK;

error:

    return ret;
}

sdk_ret_t
mapping_impl::reserve_local_ip_mapping_resources_(api_base *api_obj,
                                                  vpc_entry *vpc,
                                                  pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    mapping_swkey_t mapping_key;
    sdk_table_api_params_t api_params;
    local_ip_mapping_swkey_t local_ip_mapping_key;

    // reserve an entry in LOCAL_IP_MAPPING table with overlay IP as the key
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_ip_mapping_key,
                                         vpc->hw_id(), &spec->key.ip_addr);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &local_ip_mapping_key, NULL,
                                   NULL, 0, sdk::table::handle_t::null());
    ret = mapping_impl_db()->local_ip_mapping_tbl()->reserve(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in LOCAL_IP_MAPPING table "
                      "for local mapping %s, err %u",
                      api_obj->key2str().c_str(), ret);
        return ret;
    }
    overlay_ip_hdl_ = api_params.handle;

    ret = reserve_remote_mapping_resources_(api_obj, vpc, spec);
    if (unlikely(ret != SDK_RET_OK)) {
        goto error;
    }

    // if public IP and provider IP are not there, no more mappings or xlation
    // entries are needed
    if (!spec->public_ip_valid && !spec->provider_ip_valid) {
        return SDK_RET_OK;
    }

    if (spec->public_ip_valid) {
        // reserve an entry in LOCAL_IP_MAPPING table with public IP as the key
        PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_ip_mapping_key,
                                             PDS_IMPL_PUBLIC_VPC_HW_ID,
                                             &spec->public_ip);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &local_ip_mapping_key, NULL,
                                       NULL, 0, sdk::table::handle_t::null());
        ret = mapping_impl_db()->local_ip_mapping_tbl()->reserve(&api_params);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to reserve entry in LOCAL_IP_MAPPING table "
                          "for public IP of mapping %s, err %u",
                          api_obj->key2str().c_str(), ret);
            goto error;
        }
        public_ip_hdl_ = api_params.handle;
        // reserve an entry in the NAT table for the overlay IP to public
        // IP xlation rewrite
        ret = artemis_impl_db()->nat_tbl()->reserve(&overlay_ip_to_public_ip_nat_hdl_);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to reserve overlay IP to public IP xlation "
                          "entry in NAT table for mapping %s, err %u",
                          api_obj->key2str().c_str(), ret);
            goto error;
        }
    }

    // reserve an entry in LOCAL_IP_MAPPING table with provider IP as the key
    if (spec->provider_ip_valid) {
        PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_ip_mapping_key,
                                             PDS_IMPL_PUBLIC_VPC_HW_ID,
                                             &spec->provider_ip);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &local_ip_mapping_key, NULL,
                                       NULL, 0, sdk::table::handle_t::null());
        ret = mapping_impl_db()->local_ip_mapping_tbl()->reserve(&api_params);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to reserve entry in LOCAL_IP_MAPPING table "
                          "for provider IP of mapping %s, err %u",
                          api_obj->key2str().c_str(), ret);
            goto error;
        }
        provider_ip_hdl_ = api_params.handle;
        // reserve an entry in the NAT table for the overlay IP to provider
        // IP xlation rewrite
        ret = artemis_impl_db()->nat_tbl()->reserve(&overlay_ip_to_provider_ip_nat_hdl_);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to reserve overlay IP to provider IP xlation "
                          "entry in NAT table for mapping %s, " "err %u",
                          api_obj->key2str().c_str(), ret);
            goto error;
        }
    }

    // reserve one entry in the NAT table for xlating from both public IP and
    // provider IP to overlay IP
    ret = artemis_impl_db()->nat_tbl()->reserve(&to_overlay_ip_nat_hdl_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve public/provider IP to overlay IP "
                      "xlation entry in NAT table for mapping %s, " "err %u",
                      api_obj->key2str().c_str(), ret);
        goto error;
    }
    return SDK_RET_OK;

error:
    // TODO: release all allocated resources
    return ret;
}

sdk_ret_t
mapping_impl::reserve_resources(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    vpc_entry *vpc;
    pds_mapping_spec_t *spec;

    spec = &obj_ctxt->api_params->mapping_spec;
    vpc = vpc_db()->find(&spec->key.vpc);

    PDS_TRACE_DEBUG("Reserving resources for mapping (vpc %u, ip %s), "
                    "local %u, subnet %u, tep %u, vnic %u, "
                    "pub_ip_valid %u pub_ip %s",
                    spec->key.vpc.id, ipaddr2str(&spec->key.ip_addr), is_local_,
                    spec->subnet.id, spec->tep.id,
                    spec->vnic.id, spec->public_ip_valid,
                    ipaddr2str(&spec->public_ip));

    if (is_local_) {
        return reserve_local_ip_mapping_resources_(orig_obj, vpc, spec);
    }
    return reserve_remote_mapping_resources_(orig_obj, vpc, spec);
}

sdk_ret_t
mapping_impl::nuke_resources(api_base *api_obj) {
    sdk_table_api_params_t    api_params = { 0 };

    if (is_local_) {
        if (overlay_ip_hdl_.valid()) {
            api_params.handle = overlay_ip_hdl_;
            mapping_impl_db()->local_ip_mapping_tbl()->remove(&api_params);
        }
        if (public_ip_hdl_.valid()) {
            api_params.handle = public_ip_hdl_;
            mapping_impl_db()->local_ip_mapping_tbl()->remove(&api_params);
        }
        if (provider_ip_hdl_.valid()) {
            api_params.handle = provider_ip_hdl_;
            mapping_impl_db()->local_ip_mapping_tbl()->remove(&api_params);
        }

        // TODO: change the api calls here once DM APIs are standardized
        if (overlay_ip_to_public_ip_nat_hdl_) {
            //api_params.handle = overlay_ip_to_public_ip_nat_hdl_;
            //artemis_impl_db()->nat_tbl()->remove(&api_params);
            artemis_impl_db()->nat_tbl()->remove(overlay_ip_to_public_ip_nat_hdl_);
        }
        if (overlay_ip_to_provider_ip_nat_hdl_) {
            //api_params.handle = overlay_ip_to_provider_ip_nat_hdl_;
            //artemis_impl_db()->nat_tbl()->remove(&api_params);
            artemis_impl_db()->nat_tbl()->remove(overlay_ip_to_provider_ip_nat_hdl_);
        }
        if (to_overlay_ip_nat_hdl_) {
            //api_params.handle = to_overlay_ip_nat_hdl_;
            //artemis_impl_db()->nat_tbl()->remove(&api_params);
            artemis_impl_db()->nat_tbl()->remove(to_overlay_ip_nat_hdl_);
        }
    }
    if (mapping_hdl_.valid()) {
        api_params.handle = mapping_hdl_;
        mapping_impl_db()->mapping_tbl()->remove(&api_params);
    }
    if (nh_idx_ != 0xFFFFFFFF) {
        nexthop_impl_db()->nh_tbl()->remove(nh_idx_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::release_local_ip_mapping_resources_(api_base *api_obj) {
    sdk_table_api_params_t    api_params = { 0 };

    if (overlay_ip_hdl_.valid()) {
        api_params.handle = overlay_ip_hdl_;
        mapping_impl_db()->local_ip_mapping_tbl()->release(&api_params);
    }
    if (public_ip_hdl_.valid()) {
        api_params.handle = public_ip_hdl_;
        mapping_impl_db()->local_ip_mapping_tbl()->release(&api_params);
    }
    if (provider_ip_hdl_.valid()) {
        api_params.handle = provider_ip_hdl_;
        mapping_impl_db()->local_ip_mapping_tbl()->release(&api_params);
    }
    if (mapping_hdl_.valid()) {
        api_params.handle = mapping_hdl_;
        mapping_impl_db()->mapping_tbl()->release(&api_params);
    }
    if (nh_idx_ != 0xFFFFFFFF) {
        nexthop_impl_db()->nh_tbl()->release(nh_idx_);
    }

    // TODO: change the api calls here once DM APIs are standardized
    if (overlay_ip_to_public_ip_nat_hdl_) {
        //api_params.handle = overlay_ip_to_public_ip_nat_hdl_;
        //artemis_impl_db()->nat_tbl()->release(&api_params);
        artemis_impl_db()->nat_tbl()->release(overlay_ip_to_public_ip_nat_hdl_);
    }
    if (overlay_ip_to_provider_ip_nat_hdl_) {
        //api_params.handle = overlay_ip_to_provider_ip_nat_hdl_;
        //artemis_impl_db()->nat_tbl()->release(&api_params);
        artemis_impl_db()->nat_tbl()->release(overlay_ip_to_provider_ip_nat_hdl_);
    }
    if (to_overlay_ip_nat_hdl_) {
        //api_params.handle = to_overlay_ip_nat_hdl_;
        //artemis_impl_db()->nat_tbl()->release(&api_params);
        artemis_impl_db()->nat_tbl()->release(to_overlay_ip_nat_hdl_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::release_remote_mapping_resources_(api_base *api_obj) {
    sdk_table_api_params_t    api_params = { 0 };

    if (mapping_hdl_.valid()) {
        api_params.handle = mapping_hdl_;
        mapping_impl_db()->mapping_tbl()->release(&api_params);
    }
    if (nh_idx_ != 0xFFFFFFFF) {
        nexthop_impl_db()->nh_tbl()->release(nh_idx_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::release_resources(api_base *api_obj) {
    if (is_local_) {
        return release_local_ip_mapping_resources_(api_obj);
    }
    return release_remote_mapping_resources_(api_obj);
}

#define nexthop_info    action_u.nexthop_nexthop_info
sdk_ret_t
mapping_impl::add_remote_mapping_entries_(vpc_entry *vpc,
                                          pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    tep_entry *tep;
    ip_addr_t *dipo;
    ip_addr_t ip_addr;
    mac_addr_t *mac;
    mapping_swkey_t mapping_key;
    mapping_appdata_t mapping_data;
    sdk_table_api_params_t api_params;
    nexthop_actiondata_t nh_data = { 0 };
    device_entry *device;

    PDS_IMPL_FILL_MAPPING_SWKEY(&mapping_key, vpc->hw_id(),
                                &spec->key.ip_addr);
    PDS_IMPL_FILL_MAPPING_APPDATA(&mapping_data, nh_idx_);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &mapping_key, NULL,
                                   &mapping_data, MAPPING_MAPPING_INFO_ID,
                                   mapping_hdl_);
    ret = mapping_impl_db()->mapping_tbl()->insert(&api_params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program entry in MAPPING table for "
                      "(vpc %u, IP %s), err %u\n", vpc->hw_id(),
                      ipaddr2str(&spec->key.ip_addr), ret);
        goto error;
    }

    tep = tep_db()->find(&spec->tep);
    device = device_db()->find();
    nh_data.action_id = NEXTHOP_NEXTHOP_INFO_ID;
    if (is_local()) {
        ip_addr = device->ip_addr();
        dipo = &ip_addr;
        mac = &device->mac();
        nh_data.nexthop_info.port = TM_PORT_UPLINK_0;
    } else {
        dipo = &tep->ip();
        mac = &tep->mac();
        nh_data.nexthop_info.port = TM_PORT_UPLINK_1;
    }
    nh_data.nexthop_info.vni = vpc->fabric_encap().val.vnid;
    if (spec->provider_ip_valid) {
        dipo = &spec->provider_ip;
    }
    if (dipo->af == IP_AF_IPV6) {
        nh_data.nexthop_info.ip_type = IPTYPE_IPV6;
        sdk::lib::memrev(nh_data.nexthop_info.dipo,
                         dipo->addr.v6_addr.addr8, IP6_ADDR8_LEN);
    } else {
        nh_data.nexthop_info.ip_type = IPTYPE_IPV4;
        memcpy(nh_data.nexthop_info.dipo,
               &dipo->addr.v4_addr, IP4_ADDR8_LEN);
    }
    sdk::lib::memrev(nh_data.nexthop_info.dmaco, *mac, ETH_ADDR_LEN);
    sdk::lib::memrev(nh_data.nexthop_info.dmaci,
                     spec->overlay_mac, ETH_ADDR_LEN);
    ret = nexthop_impl_db()->nh_tbl()->insert_atid(&nh_data, nh_idx_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program NEXTHOP table at %u, err %u",
                      nh_idx_, ret);
        goto error;
    }
    return SDK_RET_OK;

    // TODO:
    // should we reserve & program entries for public, provider IP of local
    // IP mappings ?

error:
    return ret;
}

sdk_ret_t
mapping_impl::add_nat_entries_(pds_mapping_spec_t *spec) {
    sdk_ret_t           ret;
    nat_actiondata_t    nat_data = { 0 };

    // if no public or provider IPs are configured, no need to install any
    // NAT entries
    if (!spec->public_ip_valid && !spec->provider_ip_valid) {
        return SDK_RET_OK;
    }

    // allocate NAT table entries
    if (spec->public_ip_valid) {
        // add private to public IP xlation NAT entry
        PDS_IMPL_FILL_NAT_DATA(&nat_data, &spec->public_ip);
        ret =
            artemis_impl_db()->nat_tbl()->insert_atid(&nat_data,
                                                      overlay_ip_to_public_ip_nat_hdl_);
        if (ret != SDK_RET_OK) {
            return ret;
        }
    }

    if (spec->provider_ip_valid) {
        // add private to provider IP xlation NAT entry
        PDS_IMPL_FILL_NAT_DATA(&nat_data, &spec->provider_ip);
        ret =
            artemis_impl_db()->nat_tbl()->insert_atid(&nat_data,
                                                      overlay_ip_to_provider_ip_nat_hdl_);
        if (ret != SDK_RET_OK) {
            return ret;
        }
    }

    // add provider & public to private IP xlation NAT entry
    PDS_IMPL_FILL_NAT_DATA(&nat_data, &spec->key.ip_addr);
    ret =
        artemis_impl_db()->nat_tbl()->insert_atid(&nat_data,
                                                  to_overlay_ip_nat_hdl_);
    if (ret != SDK_RET_OK) {
        goto error;
    }
    return SDK_RET_OK;

error:
    // TODO: handle cleanup in case of failure
    return ret;
}

sdk_ret_t
mapping_impl::add_local_ip_mapping_entries_(vpc_entry *vpc,
                                            pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    vnic_impl *vnic_impl_obj;
    sdk_table_api_params_t api_params = { 0 };
    local_ip_mapping_swkey_t local_ip_mapping_key;
    local_ip_mapping_actiondata_t local_ip_mapping_data;

    // add entry to LOCAL_IP_MAPPING table for overlay IP
    vnic_impl_obj = (vnic_impl *)vnic_db()->find(&spec->vnic)->impl();
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_ip_mapping_key,
                                         vpc->hw_id(), &spec->key.ip_addr);
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_APPDATA(&local_ip_mapping_data,
                                           vnic_impl_obj->hw_id(), vpc->hw_id(),
                                           spec->tags[0],
                                           overlay_ip_to_provider_ip_nat_hdl_,
                                           overlay_ip_to_public_ip_nat_hdl_,
                                           vpc->nat46_prefix_valid() ?
                                               vnic_impl_obj->hw_id() + 1 :
                                               PDS_IMPL_LOCAL_46_MAPPING_RSVD_ENTRY_IDX);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params, &local_ip_mapping_key, NULL,
                                   &local_ip_mapping_data,
                                   LOCAL_IP_MAPPING_LOCAL_IP_MAPPING_INFO_ID,
                                   overlay_ip_hdl_);
    ret = mapping_impl_db()->local_ip_mapping_tbl()->insert(&api_params);
    if (ret != SDK_RET_OK) {
        goto error;
    }

    if (spec->public_ip_valid) {
        // add entry to LOCAL_IP_MAPPING table for public IP
        PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_ip_mapping_key,
                                             PDS_IMPL_PUBLIC_VPC_HW_ID,
                                             &spec->public_ip);
        PDS_IMPL_FILL_LOCAL_IP_MAPPING_APPDATA(&local_ip_mapping_data,
                                               vnic_impl_obj->hw_id(),
                                               vpc->hw_id(), spec->tags[0],
                                               to_overlay_ip_nat_hdl_,
                                               PDS_IMPL_NAT_TBL_RSVD_ENTRY_IDX,
                                               PDS_IMPL_LOCAL_46_MAPPING_RSVD_ENTRY_IDX);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params,
                                       &local_ip_mapping_key, NULL,
                                       &local_ip_mapping_data,
                                       LOCAL_IP_MAPPING_LOCAL_IP_MAPPING_INFO_ID,
                                       public_ip_hdl_);
        ret = mapping_impl_db()->local_ip_mapping_tbl()->insert(&api_params);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }

    if (spec->provider_ip_valid) {
        // add entry to LOCAL_IP_MAPPING table for provider IP
        PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_ip_mapping_key,
                                             PDS_IMPL_PUBLIC_VPC_HW_ID,
                                             &spec->provider_ip);
        PDS_IMPL_FILL_LOCAL_IP_MAPPING_APPDATA(&local_ip_mapping_data,
                                               vnic_impl_obj->hw_id(),
                                               vpc->hw_id(), spec->tags[0],
                                               to_overlay_ip_nat_hdl_,
                                               PDS_IMPL_NAT_TBL_RSVD_ENTRY_IDX,
                                               PDS_IMPL_LOCAL_46_MAPPING_RSVD_ENTRY_IDX);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&api_params,
                                       &local_ip_mapping_key, NULL,
                                       &local_ip_mapping_data,
                                       LOCAL_IP_MAPPING_LOCAL_IP_MAPPING_INFO_ID,
                                       provider_ip_hdl_);
        ret = mapping_impl_db()->local_ip_mapping_tbl()->insert(&api_params);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }
    // add MAPPING table entries now
    return add_remote_mapping_entries_(vpc, spec);

error:
    // TODO: handle cleanup in case of failure
    return ret;
}

sdk_ret_t
mapping_impl::program_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t             ret;
    pds_mapping_spec_t    *spec;
    vpc_entry             *vpc;
    subnet_entry          *subnet;
    tep_entry             *tep;
    ip_addr_t             ip_addr;

    spec = &obj_ctxt->api_params->mapping_spec;
    vpc = vpc_db()->find(&spec->key.vpc);
    subnet = subnet_db()->find(&spec->subnet);
    tep = tep_db()->find(&spec->tep);
    ip_addr = device_db()->find()->ip_addr();
    if (is_local_) {
        PDS_TRACE_DEBUG("Programming local mapping (vpc %u, ip %s), vnic %u, "
                        "subnet %u, tep %s, overlay mac %s, "
                        "fabric encap (%u, %u), public IP %s, provider IP %s",
                        spec->key.vpc.id, ipaddr2str(&spec->key.ip_addr),
                        spec->vnic.id, spec->subnet.id,
                        ipaddr2str(&ip_addr),
                        macaddr2str(spec->overlay_mac), spec->fabric_encap.type,
                        spec->fabric_encap.val.value,
                        spec->public_ip_valid ?
                            ipaddr2str(&spec->public_ip) : "None",
                        spec->provider_ip_valid ?
                            ipaddr2str(&spec->provider_ip) : "");
    } else {
        PDS_TRACE_DEBUG("Programming remote mapping (vpc %u, ip %s), vnic %u, "
                        "subnet %u, tep %s, overlay mac %s, "
                        "fabric encap (%u, %u), provider IP %s",
                        spec->key.vpc.id, ipaddr2str(&spec->key.ip_addr),
                        spec->vnic.id, spec->subnet.id,
                        ipaddr2str(&tep->ip()),
                        macaddr2str(spec->overlay_mac), spec->fabric_encap.type,
                        spec->fabric_encap.val.value,
                        spec->provider_ip_valid ?
                            ipaddr2str(&spec->provider_ip) : "None");
    }
    if (is_local_) {
        // allocate NAT table entries
        ret = add_nat_entries_(spec);
        if (ret != SDK_RET_OK) {
            goto error;
        }

        ret = add_local_ip_mapping_entries_(vpc, spec);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    } else {
        ret = add_remote_mapping_entries_(vpc, spec);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }
    return SDK_RET_OK;

error:

    return ret;
}

sdk_ret_t
mapping_impl::cleanup_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::update_hw(api_base *curr_obj, api_base *prev_obj,
                        api_obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                          pds_epoch_t epoch, api_op_t api_op,
                          api_obj_ctxt_t *obj_ctxt) {
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::read_local_mapping_(vpc_entry *vpc, pds_mapping_spec_t *spec) {
    // TODO: tag support
    spec->num_tags = 0;
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::read_remote_mapping_(vpc_entry *vpc, pds_mapping_spec_t *spec) {
    // TODO: tag support
    spec->num_tags = 0;
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
mapping_impl::read_hw(pds_mapping_key_t *key, pds_mapping_info_t *info) {
    sdk_ret_t ret;
    vpc_entry *vpc;
    nat_actiondata_t nat_data = { 0 };

    vpc = vpc_db()->find(&key->vpc);
    if (is_local_) {
        ret = read_local_mapping_(vpc, &info->spec);
    } else {
        ret = read_remote_mapping_(vpc, &info->spec);
    }
    if (ret != SDK_RET_OK) {
        return ret;
    }
    return ret;
}

/// \@}    // end of PDS_MAPPING_IMPL

}    // namespace impl
}    // namespace api
