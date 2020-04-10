//
// Copyright (c) 2019 Pensando Systems, Inc.
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of mapping
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/lib/utils/utils.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/mapping.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "nic/apollo/api/impl/apulu/tep_impl.hpp"
#include "nic/apollo/api/impl/apulu/subnet_impl.hpp"
#include "nic/apollo/api/impl/apulu/vnic_impl.hpp"
#include "nic/apollo/api/impl/apulu/vpc_impl.hpp"
#include "nic/apollo/api/impl/apulu/nexthop_group_impl.hpp"
#include "nic/apollo/api/impl/apulu/mapping_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"
#include "nic/apollo/p4/include/apulu_defines.h"
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd.h"
#include "gen/p4gen/p4/include/ftl.h"
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_ftl.h"

using sdk::table::sdk_table_api_params_t;

namespace api {
namespace impl {

/// \defgroup PDS_MAPPING_IMPL - mapping entry datapath implementation
/// \ingroup PDS_MAPPING
/// @{

mapping_impl *
mapping_impl::factory(pds_mapping_spec_t *spec) {
    mapping_impl    *impl;

    impl = mapping_impl_db()->alloc();
    if (unlikely(impl == NULL)) {
        return NULL;
    }
    new (impl) mapping_impl();
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

impl_base *
mapping_impl::clone(void) {
    mapping_impl *cloned_impl;

    cloned_impl = mapping_impl_db()->alloc();
    new (cloned_impl) mapping_impl();
    // deep copy is not needed as we don't store pointers
    // TODO: revisit this for update handling !!
    //*cloned_impl = *this;
    return cloned_impl;
}

sdk_ret_t
mapping_impl::free(mapping_impl *impl) {
    destroy(impl);
    return SDK_RET_OK;
}

mapping_impl *
mapping_impl::build(pds_mapping_key_t *key, mapping_entry *mapping) {
    sdk_ret_t ret;
    bool is_local;
    vpc_entry *vpc;
    mapping_impl *impl;
    ip_addr_t public_ip;
    subnet_entry *subnet;
    device_entry *device;
    uint32_t num_class_id;
    pds_obj_key_t vpc_key;
    p4pd_error_t p4pd_ret;
    mapping_swkey_t mapping_key;
    nat_rewrite_entry_t nat_data;
    bool public_ip_valid = false;
    sdk_table_api_params_t tparams;
    mapping_appdata_t mapping_data;
    rxdma_mapping_swkey_t rxdma_mapping_key;
    local_mapping_swkey_t local_mapping_key;
    mapping_tag_info_entry_t mapping_tag_data;
    local_mapping_appdata_t local_mapping_data;
    rxdma_mapping_appdata_t rxdma_mapping_data;
    uint32_t class_id[PDS_MAX_TAGS_PER_MAPPING];
    local_mapping_appdata_t pub_ip_local_mapping_data;
    uint32_t rxdma_mapping_tag_idx = PDS_IMPL_RSVD_TAG_HW_ID;
    sdk::table::handle_t mapping_hdl = sdk::table::handle_t::null();
    sdk::table::handle_t rxdma_mapping_hdl = sdk::table::handle_t::null();
    sdk::table::handle_t rxdma_mapping_public_ip_hdl = sdk::table::handle_t::null();
    sdk::table::handle_t local_mapping_overlay_ip_hdl = sdk::table::handle_t::null();
    sdk::table::handle_t local_mapping_public_ip_hdl = sdk::table::handle_t::null();

    device = device_find();
    if (key->type == PDS_MAPPING_TYPE_L2) {
        subnet = subnet_find(&key->subnet);
        if (unlikely(subnet == NULL)) {
            return NULL;
        }
        vpc_key = subnet->vpc();
        vpc = vpc_find(&vpc_key);
        if (unlikely(vpc == NULL)) {
            return NULL;
        }
    } else {
        vpc = vpc_find(&key->vpc);
        if (unlikely(vpc == NULL)) {
            return NULL;
        }
    }

    // read mapping table and infer if this is also a local mapping
    if (key->type == PDS_MAPPING_TYPE_L2) {
        PDS_IMPL_FILL_L2_MAPPING_SWKEY(&mapping_key,
                                       ((subnet_impl *)subnet->impl())->hw_id(),
                                       key->mac_addr);
    } else {
        PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key,
                                       ((vpc_impl *)vpc->impl())->hw_id(),
                                       &key->ip_addr);
    }
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL, &mapping_data,
                                   0, sdk::table::handle_t::null());
    ret = mapping_impl_db()->mapping_tbl()->get(&tparams);
    if (ret != SDK_RET_OK) {
        return NULL;
    }
    mapping_hdl = tparams.handle;
    is_local = (mapping_data.nexthop_type == NEXTHOP_TYPE_NEXTHOP);

    // read local mapping table, this also tells us if there are public
    // IP mappings
    if (is_local) {
        PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_mapping_key,
                                             ((vpc_impl *)vpc->impl())->hw_id(),
                                             &key->ip_addr);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key, NULL,
                                       &local_mapping_data, 0,
                                       sdk::table::handle_t::null());
        ret = mapping_impl_db()->local_mapping_tbl()->get(&tparams);
        if (ret != SDK_RET_OK) {
            return NULL;
        }
        local_mapping_overlay_ip_hdl = tparams.handle;
        if (local_mapping_data.ip_type != MAPPING_TYPE_OVERLAY) {
            // this is either public IP or IP entry for some other purpose
            return NULL;
        }
        public_ip_valid = (local_mapping_data.xlate_id !=
                               PDS_IMPL_RSVD_NAT_HW_ID) ? true : false;
    }

    // it is possible that this overlay mapping has tags configured, read them
    num_class_id = 0;
    if (key->type == PDS_MAPPING_TYPE_L3) {
        PDS_IMPL_FILL_RXDMA_IP_MAPPING_KEY(&rxdma_mapping_key,
                                           ((vpc_impl *)vpc->impl())->hw_id(),
                                           &key->ip_addr);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &rxdma_mapping_key,
                                       NULL, &rxdma_mapping_data,
                                       0, sdk::table::handle_t::null());
        ret = mapping_impl_db()->rxdma_mapping_tbl()->get(&tparams);
        if (ret == SDK_RET_OK) {
            rxdma_mapping_hdl = tparams.handle;
            rxdma_mapping_tag_idx = rxdma_mapping_data.tag_idx;
            if (rxdma_mapping_tag_idx != PDS_IMPL_RSVD_TAG_HW_ID) {
                ret = mapping_tag_data.read(rxdma_mapping_tag_idx);
                if (unlikely(ret != SDK_RET_OK)) {
                    PDS_TRACE_ERR("Failed to read MAPPING_TAG table "
                                  "at idx %u", rxdma_mapping_tag_idx);
                    return NULL;
                }
                // retrieve the class id values
                if (mapping_tag_data.classid0 !=
                        PDS_IMPL_RSVD_MAPPING_CLASS_ID) {
                    class_id[0] = mapping_tag_data.classid0;
                    num_class_id++;
                }
                if (mapping_tag_data.classid1 !=
                        PDS_IMPL_RSVD_MAPPING_CLASS_ID) {
                    class_id[1] = mapping_tag_data.classid1;
                    num_class_id++;
                }
                if (mapping_tag_data.classid1 !=
                        PDS_IMPL_RSVD_MAPPING_CLASS_ID) {
                    class_id[2] = mapping_tag_data.classid3;
                    num_class_id++;
                }
                if (mapping_tag_data.classid1 !=
                        PDS_IMPL_RSVD_MAPPING_CLASS_ID) {
                    class_id[3] = mapping_tag_data.classid3;
                    num_class_id++;
                }
                if (mapping_tag_data.classid1 !=
                        PDS_IMPL_RSVD_MAPPING_CLASS_ID) {
                    class_id[4] = mapping_tag_data.classid4;
                    num_class_id++;
                }
            }
        }
    }

    // if public mapping exists, NAT table provides the public IP
    if (public_ip_valid) {
        memset(&nat_data, 0, nat_data.entry_size());
        ret = nat_data.read(local_mapping_data.xlate_id);
        if (ret != SDK_RET_OK) {
            return NULL;
        }

        // read public IP from NAT data
        P4_IPADDR_TO_IPADDR(nat_data.ip, public_ip, key->ip_addr.af);

        // read local mapping for pub IP to get it's xlate index
        PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_mapping_key,
                                             ((vpc_impl *)vpc->impl())->hw_id(),
                                             &public_ip);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key, NULL,
                                       &pub_ip_local_mapping_data, 0,
                                       sdk::table::handle_t::null());
        ret = mapping_impl_db()->local_mapping_tbl()->get(&tparams);
        if (ret != SDK_RET_OK) {
            return NULL;
        }
        local_mapping_public_ip_hdl = tparams.handle;

        // read the rxdma MAPPING table entry of public IP if this mapping
        // has tags configured
        if (num_class_id) {
            PDS_IMPL_FILL_RXDMA_IP_MAPPING_KEY(&rxdma_mapping_key,
                                               ((vpc_impl *)vpc->impl())->hw_id(),
                                               &public_ip);
            PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &rxdma_mapping_key,
                                           NULL, &rxdma_mapping_data,
                                           0, sdk::table::handle_t::null());
            ret = mapping_impl_db()->rxdma_mapping_tbl()->get(&tparams);
            if (ret != SDK_RET_OK) {
                PDS_TRACE_ERR("Failed to read rxdma MAPPING table entry for "
                              "public IP %s", ipaddr2str(&public_ip));
                return NULL;
            }
            rxdma_mapping_public_ip_hdl = tparams.handle;
            // NOTE: we already read the class ids earlier
        }
    }

    // all required tables are read, allocate impl object and build it
    impl = mapping_impl_db()->alloc();
    if (unlikely(impl == NULL)) {
        return NULL;
    }
    new (impl) mapping_impl();

    mapping->set_local(is_local);
    if (is_local) {
        impl->vnic_hw_id_ = local_mapping_data.vnic_id;
    } else {
        impl->vnic_hw_id_ = PDS_IMPL_RSVD_VNIC_HW_ID;
    }
    impl->vpc_hw_id_ = ((vpc_impl *)vpc->impl())->hw_id();
    impl->subnet_hw_id_ = mapping_data.egress_bd_id;
    if (mapping_data.nexthop_valid) {
        impl->nexthop_type_ = mapping_data.nexthop_type;
        impl->nexthop_id_ = mapping_data.nexthop_id;
    } else {
        impl->nexthop_id_ = PDS_IMPL_SYSTEM_DROP_NEXTHOP_HW_ID;
    }
    if (public_ip_valid) {
        mapping->set_public_ip(&public_ip);
        impl->to_public_ip_nat_idx_ = local_mapping_data.xlate_id;
        impl->to_overlay_ip_nat_idx_ = pub_ip_local_mapping_data.xlate_id;
        impl->rxdma_mapping_public_ip_hdl_ = rxdma_mapping_public_ip_hdl;
    }
    impl->mapping_hdl_ = mapping_hdl;
    impl->local_mapping_overlay_ip_hdl_ = local_mapping_overlay_ip_hdl;
    impl->local_mapping_public_ip_hdl_ = local_mapping_public_ip_hdl;
    impl->rxdma_local_mapping_tag_idx_ = local_mapping_data.tag_idx;
    impl->rxdma_mapping_tag_idx_ = rxdma_mapping_tag_idx;
    impl->rxdma_mapping_hdl_ = rxdma_mapping_hdl;
    impl->num_class_id_ = num_class_id;
    mapping->set_num_tags(num_class_id);
    memcpy(impl->class_id_, class_id, sizeof(class_id));

    return impl;
}

sdk_ret_t
mapping_impl::reserve_public_ip_mapping_resources_(mapping_entry *mapping,
                                                   vpc_impl *vpc,
                                                   vnic_entry *vnic,
                                                   pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    mapping_swkey_t mapping_key;
    sdk_table_api_params_t tparams;
    local_mapping_swkey_t local_mapping_key;

    // reserve an entry in LOCAL_MAPPING table for public IP
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_mapping_key, vpc->hw_id(),
                                         &spec->public_ip);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key, NULL, NULL, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->local_mapping_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in LOCAL_MAPPING table "
                      "for public IP %s of mapping %s, vnic %s, err %u",
                      ipaddr2str(&spec->public_ip), mapping->key2str().c_str(),
                      vnic->key().str(), ret);
        return ret;
    }
    local_mapping_public_ip_hdl_ = tparams.handle;

    // reserve an entry in MAPING table for public IP
    PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key, vpc->hw_id(),
                                   &spec->public_ip);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL, NULL, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->mapping_tbl()->reserve(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in MAPPING table for public IP "
                      "%s of mapping %s, vnic %s, err %u",
                      ipaddr2str(&spec->public_ip), mapping->key2str().c_str(),
                      vnic->key().str(), ret);
        return ret;
    }
    mapping_public_ip_hdl_ = tparams.handle;
    PDS_TRACE_DEBUG("Rsvd public IP hdl 0x%lx in LOCAL_MAPPING table, "
                    "public IP handle in 0x%lx in MAPPING table",
                    local_mapping_public_ip_hdl_, mapping_public_ip_hdl_);
    return ret;
}

sdk_ret_t
mapping_impl::reserve_nat_resources_(mapping_entry *mapping, vnic_entry *vnic,
                                     pds_mapping_spec_t *spec) {
    sdk_ret_t ret;

    // reserve an entry for overlay IP to public IP xlation in NAT table
    ret = apulu_impl_db()->nat_idxr()->alloc(&to_public_ip_nat_idx_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry for public IP %s in NAT table "
                      "for mapping %s vnic %s, err %u",
                      ipaddr2str(&spec->public_ip), mapping->key2str().c_str(),
                      vnic->key().str(), ret);
        return ret;
    }

    // reserve an entry for public IP to overlay IP xlation in NAT table
    ret = apulu_impl_db()->nat_idxr()->alloc(&to_overlay_ip_nat_idx_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry for overlay IP in NAT table for "
                      "mapping %s, vnic %s, err %u", mapping->key2str().c_str(),
                      vnic->key().str(), ret);
        return ret;
    }
    PDS_TRACE_DEBUG("Rsvd public IP NAT handle 0x%lx, overlay IP NAT "
                    "handle 0x%lx", to_public_ip_nat_idx_,
                    to_overlay_ip_nat_idx_);
    return ret;
}

sdk_ret_t
mapping_impl::reserve_public_ip_rxdma_mapping_resources_(mapping_entry *mapping,
                  vpc_impl *vpc, vnic_entry *vnic, pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    sdk_table_api_params_t tparams;
    rxdma_mapping_swkey_t rxdma_mapping_key;

    // reserve an entry in rxdma MAPING table for public IP
    PDS_IMPL_FILL_RXDMA_IP_MAPPING_KEY(&rxdma_mapping_key, vpc->hw_id(),
                                       &spec->public_ip);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &rxdma_mapping_key,
                                   NULL, NULL, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->rxdma_mapping_tbl()->reserve(&tparams);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to reserve entry in rxdma MAPPING table for "
                      "public IP %s of mapping %s, vnic %s, err %u",
                      ipaddr2str(&spec->public_ip), mapping->key2str().c_str(),
                      vnic->key().str(), ret);
        return ret;
    }
    rxdma_mapping_public_ip_hdl_ = tparams.handle;
    return ret;
}


sdk_ret_t
mapping_impl::reserve_public_ip_resources_(mapping_entry *mapping,
                                           vpc_impl *vpc, vnic_entry *vnic,
                                           pds_mapping_spec_t *spec) {
    sdk_ret_t ret;

    ret = reserve_public_ip_mapping_resources_(mapping, vpc, vnic, spec);
    if (unlikely(ret != SDK_RET_OK)) {
        return ret;
    }
    // reserve NAT table entries
    ret = reserve_nat_resources_(mapping, vnic, spec);
    if (unlikely(ret != SDK_RET_OK)) {
        return ret;
    }
    // if tags are configured on this mapping, reserve an entry in the rxdma
    // MAPPING table
    if (spec->num_tags) {
        return reserve_public_ip_rxdma_mapping_resources_(mapping, vpc,
                                                          vnic, spec);
    }
    return ret;
}

sdk_ret_t
mapping_impl::allocate_tag_classes_(vpc_entry *vpc, bool local,
                                    mapping_entry *mapping,
                                    pds_mapping_spec_t *spec) {
    sdk_ret_t ret;

    // allocate class id for each tag
    for (uint32_t i = 0; i < spec->num_tags; i++) {
        ret = vpc_impl_db()->alloc_class_id(spec->tags[i],
                                            local, &class_id_[i]);
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to allocate class id for tag %u of "
                          "remote local mapping %s %s, err %u",
                          spec->tags[i], mapping->key().str(),
                          mapping->key2str().c_str(), ret);
            return ret;
        }
        num_class_id_++;
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::reserve_rxdma_mapping_tag_resources_(vpc_entry *vpc, bool local,
                                                   mapping_entry *mapping,
                                                   pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    sdk_table_api_params_t tparams;
    rxdma_mapping_swkey_t rxdma_mapping_key;

    if (spec->num_tags == 0) {
        return SDK_RET_OK;
    }

    // allocate class id for each tag configured for this mapping
    ret = allocate_tag_classes_(vpc, local, mapping, spec);
    if (unlikely(ret != SDK_RET_OK)) {
        return ret;
    }

    // allocate an index in the rxdma MAPPING_TAG table
    ret = mapping_impl_db()->mapping_tag_idxr()->alloc(&rxdma_mapping_tag_idx_);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to reserve entry in MAPPING_TAG table, "
                      "err %u", ret);
        return ret;
    }

    // allocate an entry in the rxdma MAPPING table for the overlay IP to
    // point to rxdma_mapping_tag_idx_ in rxdma MAPPING_TAG table
    PDS_IMPL_FILL_RXDMA_IP_MAPPING_KEY(&rxdma_mapping_key,
                                       ((vpc_impl *)vpc->impl())->hw_id(),
                                       &spec->skey.ip_addr);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &rxdma_mapping_key,
                                   NULL, NULL, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->rxdma_mapping_tbl()->reserve(&tparams);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to reserve entry in rxdma MAPPING table "
                      "for mapping %s, err %u",
                      mapping->key2str().c_str(), ret);
        return ret;
    }
    rxdma_mapping_hdl_ = tparams.handle;
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::reserve_local_mapping_resources_(mapping_entry *mapping,
                                               vpc_entry *vpc,
                                               vnic_entry *vnic,
                                               pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    mapping_swkey_t mapping_key;
    sdk_table_api_params_t tparams;
    local_mapping_swkey_t local_mapping_key;
    rxdma_mapping_swkey_t rxdma_mapping_key;

    // reserve an entry in LOCAL_MAPPING table for overlay IP
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_mapping_key,
                                         ((vpc_impl *)vpc->impl())->hw_id(),
                                         &spec->skey.ip_addr);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key, NULL, NULL, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->local_mapping_tbl()->reserve(&tparams);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to reserve entry in LOCAL_MAPPING table "
                      "for mapping %s, vnic %s, err %u",
                      mapping->key2str().c_str(), vnic->key().str(), ret);
        return ret;
    }
    local_mapping_overlay_ip_hdl_ = tparams.handle;

    // reserve an entry in MAPPING table for overlay IP
    PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key,
                                   ((vpc_impl *)vpc->impl())->hw_id(),
                                   &spec->skey.ip_addr);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL, NULL, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->mapping_tbl()->reserve(&tparams);
    if (unlikely(ret != SDK_RET_OK)) {
        PDS_TRACE_ERR("Failed to reserve entry in MAPPING table for "
                      "mapping %s, vnic %s, err %u",
                      mapping->key2str().c_str(), vnic->key().str(), ret);
        return ret;
    }
    mapping_hdl_ = tparams.handle;

    PDS_TRACE_DEBUG("Rsvd LOCAL_MAPPING handle 0x%lx, MAPPING handle 0x%lx",
                    local_mapping_overlay_ip_hdl_, mapping_hdl_);

    // if tags are configured on this mapping, allocate all needed resources
    if (spec->num_tags) {
        // allocate class id for each tag configured for this mapping
        ret = allocate_tag_classes_(vpc, true, mapping, spec);
        if (unlikely(ret != SDK_RET_OK)) {
            return ret;
        }

        // allocate an index in the rxdma LOCAL_MAPPING_TAG table
        ret = mapping_impl_db()->local_mapping_tag_idxr()->alloc(&rxdma_local_mapping_tag_idx_);
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to reserve entry in LOCAL_MAPPING_TAG table, "
                          "err %u", ret);
            return ret;
        }

        // allocate an index in the rxdma MAPPING_TAG table
        ret = mapping_impl_db()->mapping_tag_idxr()->alloc(&rxdma_mapping_tag_idx_);
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to reserve entry in MAPPING_TAG table, "
                          "err %u", ret);
            return ret;
        }

        // allocate an entry in the rxdma MAPPING table for the overlay IP to
        // point to rxdma_mapping_tag_idx_ in rxdma MAPPING_TAG table
        PDS_IMPL_FILL_RXDMA_IP_MAPPING_KEY(&rxdma_mapping_key,
                                           ((vpc_impl *)vpc->impl())->hw_id(),
                                           &spec->skey.ip_addr);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &rxdma_mapping_key,
                                       NULL, NULL, 0,
                                       sdk::table::handle_t::null());
        ret = mapping_impl_db()->rxdma_mapping_tbl()->reserve(&tparams);
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to reserve entry in rxdma MAPPING table "
                          "for mapping %s, vnic %s, err %u",
                          mapping->key2str().c_str(), vnic->key().str(), ret);
            return ret;
        }
        rxdma_mapping_hdl_ = tparams.handle;
    }

#if 0
    // if IP/MAC binding checks are enabled, reserve entries in IP_MAC_BINDING
    // table so we can point from the corresponding L2 entry in the
    // LOCAL_MAPPING table
    if (vnic->binding_checks_en() && !vnic->switch_vnic() &&
        (spec->skey.ip_addr.af == IP_AF_IPV4)) {
        ret = mapping_impl_db()->ip_mac_binding_idxr()->alloc(&ipv4_mac_binding_hw_id_);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to reserve entry in IP_MAC_BINDING table for "
                          "mapping %s, vnic %s, err %u",
                          mapping->key2str().c_str(), vnic->key().str(), ret);
            goto error;
        }
    }
#endif

    // check if this mapping has public IP
    if (spec->public_ip_valid) {
        ret = reserve_public_ip_resources_(mapping, (vpc_impl *)vpc->impl(),
                                           vnic, spec);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to reserve resources for public IP %s "
                          "of mapping %s, vnic %s, err %u",
                          ipaddr2str(&spec->public_ip),
                          mapping->key2str().c_str(), vnic->key().str(), ret);
            return ret;
        }
    }
    return ret;
}

sdk_ret_t
mapping_impl::reserve_remote_mapping_resources_(mapping_entry *mapping,
                                                vpc_entry *vpc,
                                                subnet_entry *subnet,
                                                pds_mapping_spec_t *spec) {
    sdk_ret_t ret = SDK_RET_OK;
    mapping_swkey_t mapping_key;
    sdk_table_api_params_t tparams;

    // reserve an entry in MAPPING table
    if (spec->skey.type == PDS_MAPPING_TYPE_L3) {
        PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key,
                                       ((vpc_impl *)vpc->impl())->hw_id(),
                                       &spec->skey.ip_addr);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL, NULL, 0,
                                       sdk::table::handle_t::null());
        ret = mapping_impl_db()->mapping_tbl()->reserve(&tparams);
    } else if (spec->skey.type == PDS_MAPPING_TYPE_L2) {
        PDS_IMPL_FILL_L2_MAPPING_SWKEY(&mapping_key,
                                       ((subnet_impl *)subnet->impl())->hw_id(),
                                       spec->skey.mac_addr);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL, NULL, 0,
                                       sdk::table::handle_t::null());
        ret = mapping_impl_db()->mapping_tbl()->reserve(&tparams);
    }
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to reserve entry in MAPPING table for "
                      "mapping %s, err %u", mapping->key2str().c_str(), ret);
        return ret;
    }
    mapping_hdl_ = tparams.handle;

    // if tags are configured on this mapping, allocate all needed resources
    if (spec->num_tags) {
        ret = reserve_rxdma_mapping_tag_resources_(vpc, false, mapping, spec);
    }
    return ret;
}

sdk_ret_t
mapping_impl::reserve_resources(api_base *api_obj, api_base *orig_obj,
                                api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    vpc_entry *vpc;
    vnic_entry *vnic;
    subnet_entry *subnet;
    pds_mapping_spec_t *spec;
    mapping_entry *new_mapping = (mapping_entry *)api_obj;
    mapping_entry *curr_mapping = (mapping_entry *)orig_obj;

    spec = &obj_ctxt->api_params->mapping_spec;
    switch (obj_ctxt->api_op) {
    case API_OP_CREATE:
        // record the fact that resource reservation was attempted
        // NOTE: even if we partially acquire resources and fail eventually,
        //       this will ensure that proper release of resources will happen
        api_obj->set_rsvd_rsc();
        if (spec->is_local) {
            PDS_TRACE_DEBUG("Reserving resources for mapping %s, local %u, "
                            "subnet %s, vnic %s, pub_ip_valid %u, pub_ip %s",
                            new_mapping->key2str().c_str(), spec->is_local,
                            spec->subnet.str(), spec->vnic.str(),
                            spec->public_ip_valid,
                            ipaddr2str(&spec->public_ip));
            // reserve all local IP mapping resources
            vpc = vpc_find(&spec->skey.vpc);
            vnic = vnic_find(&spec->vnic);
            if (unlikely(vnic == NULL)) {
                PDS_TRACE_ERR("Failed to find vnic %s, local mapping %s "
                              "resource reservation failed", spec->vnic.str(),
                              new_mapping->key2str().c_str());
                return SDK_RET_INVALID_ARG;
            }
            return reserve_local_mapping_resources_(new_mapping, vpc,
                                                    vnic, spec);
        }

        // reserve all remote MAC or IP mapping resources
        PDS_TRACE_DEBUG("Reserving resources for mapping %s local %u, subnet %s, "
                        "nh type %s-%s", new_mapping->key2str().c_str(),
                        spec->is_local, spec->subnet.str(),
                        (spec->nh_type == PDS_NH_TYPE_OVERLAY) ? "tep" : "nh group",
                        (spec->nh_type == PDS_NH_TYPE_OVERLAY) ?
                            spec->tep.str() : spec->nh_group.str());
        if (spec->skey.type == PDS_MAPPING_TYPE_L2) {
            vpc = NULL;
            subnet = subnet_find(&spec->skey.subnet);
        } else {
            vpc = vpc_find(&spec->skey.vpc);
            subnet = NULL;
        }
        return reserve_remote_mapping_resources_(new_mapping, vpc,
                                                 subnet, spec);

    case API_OP_UPDATE:
        // record the fact that resource reservation was attempted
        // NOTE: even if we partially acquire resources and fail eventually,
        //       this will ensure that proper release of resources will happen
        api_obj->set_rsvd_rsc();
        if (spec->is_local) {
            PDS_TRACE_DEBUG("Reserving resources for mapping %s, local %u, "
                            "subnet %s, vnic %s, pub_ip_valid %u, pub_ip %s",
                            new_mapping->key2str().c_str(), spec->is_local,
                            spec->subnet.str(), spec->vnic.str(),
                            spec->public_ip_valid,
                            ipaddr2str(&spec->public_ip));
            vpc = vpc_find(&spec->skey.vpc);
            vnic = vnic_find(&spec->vnic);
            if (unlikely(vnic == NULL)) {
                PDS_TRACE_ERR("Failed to find vnic %s, local mapping %s "
                              "resource reservation failed", spec->vnic.str(),
                              new_mapping->key2str().c_str());
                return SDK_RET_INVALID_ARG;
            }
            if (obj_ctxt->upd_bmap & PDS_MAPPING_UPD_PUBLIC_IP) {
                if (spec->public_ip_valid) {
                    // we need to acquire new resources for public IP in mapping
                    // tables and also in NAT table. There are two cases here:
                    // case 1 - current object has public IP which is being
                    //          updated, in this case we can reuse NAT table
                    //          resources, so we only have to allocate mapping
                    //          table resources.
                    // case 2 - current object has no public IP, so allocate
                    //          all resources needed for public IP
                    if (curr_mapping->is_public_ip_valid()) {
                        // case 1 - reserve only mapping table resources for
                        //          the updated public IP
                        ret = reserve_public_ip_mapping_resources_(new_mapping,
                                  (vpc_impl *)vpc->impl(), vnic, spec);
                    } else {
                        // case 2 - reserve all resources needed for the new
                        //          public IP for this mapping
                        ret = reserve_public_ip_resources_(new_mapping,
                                  (vpc_impl *)vpc->impl(), vnic, spec);
                    }
                } else {
                    // we don't need to reserve resource(s) for public IP
                    local_mapping_public_ip_hdl_ =
                        sdk::table::handle_t::null();
                    mapping_public_ip_hdl_ = sdk::table::handle_t::null();
                    to_public_ip_nat_idx_ = to_overlay_ip_nat_idx_ =
                        PDS_IMPL_RSVD_NAT_HW_ID;
                    ret = SDK_RET_OK;
                }
                return ret;
            }
        } else {
            // except for tags, if any, remote mapping uses existing resources
            if (obj_ctxt->upd_bmap & PDS_MAPPING_UPD_TAGS_ADD) {
                // allocate all the resources needed to support tags
                ret = reserve_rxdma_mapping_tag_resources_(vpc, false,
                                                           new_mapping, spec);
                if (unlikely(ret != SDK_RET_OK)) {
                    return ret;
                }
            } else if (obj_ctxt->upd_bmap & PDS_MAPPING_UPD_TAGS_UPD) {
                // probably some new tags and/or some same old tags and/or
                // some tags got removed, allocate class ids needed in all cases
                ret = allocate_tag_classes_(vpc, false, new_mapping, spec);
                if (unlikely(ret != SDK_RET_OK)) {
                    return ret;
                }
            }
            // NOTE: for other two cases, no need to allocate any new resources
        }
        break;

    default:
        return SDK_RET_INVALID_OP;
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::nuke_resources(api_base *api_obj) {
    sdk_ret_t ret;
    mapping_swkey_t mapping_key;
    sdk_table_api_params_t tparams;
    local_mapping_swkey_t local_mapping_key;
    mapping_entry *mapping = (mapping_entry *)api_obj;

    if (mapping_hdl_.valid()) {
        // remove mapping entry for overlay IP or MAC
        if (mapping->skey().type == PDS_MAPPING_TYPE_L3) {
            PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key, vpc_hw_id_,
                                           &mapping->skey().ip_addr);
        } else {
            PDS_IMPL_FILL_L2_MAPPING_SWKEY(&mapping_key, subnet_hw_id_,
                                           mapping->skey().mac_addr)
        }
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL, NULL, 0,
                                       sdk::table::handle_t::null());
        ret = mapping_impl_db()->mapping_tbl()->remove(&tparams);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to remove entry in MAPPING table for %s, err %u",
                          mapping->key2str().c_str(), ret);
            return ret;
        }
    }

    // release any class ids allocated for this mapping
    for (uint32_t i = 0; i < num_class_id_; i++) {
        ret = vpc_impl_db()->free_class_id(class_id_[i], mapping->is_local());
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to free classid %u allocated to remote "
                          "mapping %s, err %u",
                          api_obj->key2str().c_str(), ret);
            // continue freeing the resources
        }
    }

    if (mapping->is_local() == false) {
        // nothing else to do if there is no local mapping
        return ret;
    }

    if (local_mapping_overlay_ip_hdl_.valid()) {
        // delete local mapping entry for overlay ip
        PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_mapping_key, vpc_hw_id_,
                                             &mapping->skey().ip_addr);
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key, NULL, NULL, 0,
                                       sdk::table::handle_t::null());
        ret = mapping_impl_db()->local_mapping_tbl()->remove(&tparams);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to remove entry in LOCAL_MAPPING table %s, "
                          "err %u", mapping->key2str().c_str(), ret);
            return ret;
        }
    }

#if 0
    // free the IP_MAC_BINDING table entries reserved, if any
    if (ipv4_mac_binding_hw_id_ != PDS_IMPL_RSVD_IP_MAC_BINDING_HW_ID) {
        mapping_impl_db()->ip_mac_binding_idxr()->free(ipv4_mac_binding_hw_id_);
    }
#endif

    // nothing else to do if public IP is not configured
    if (!mapping->is_public_ip_valid()) {
        return SDK_RET_OK;
    }

    if (local_mapping_public_ip_hdl_.valid()) {
        // remove public IP mapping
        PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key, vpc_hw_id_,
                                       &mapping->public_ip());
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL, NULL,
                                       0, sdk::table::handle_t::null());
        ret = mapping_impl_db()->mapping_tbl()->remove(&tparams);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to remove entry in MAPPING table for %s, err %u",
                          mapping->key2str().c_str(), ret);
            return ret;
        }

        // remove public IP local mapping
        PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_mapping_key, vpc_hw_id_,
                                             &mapping->public_ip());
        PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key, NULL,
                                       NULL, 0, sdk::table::handle_t::null());
        ret = mapping_impl_db()->local_mapping_tbl()->remove(&tparams);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to remove entry in LOCAL_MAPPING table for %s, "
                          "err %u", mapping->key2str().c_str(), ret);
            return ret;
        }

        // free NAT entries
        if (to_public_ip_nat_idx_ != PDS_IMPL_RSVD_NAT_HW_ID) {
            apulu_impl_db()->nat_idxr()->free(to_public_ip_nat_idx_);
        }

        if (to_overlay_ip_nat_idx_ != PDS_IMPL_RSVD_NAT_HW_ID) {
            apulu_impl_db()->nat_idxr()->free(to_overlay_ip_nat_idx_);
        }
    }

    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::release_local_mapping_resources_(api_base *api_obj) {
    sdk_ret_t ret;
    sdk_table_api_params_t tparams = { 0 };

    if (local_mapping_overlay_ip_hdl_.valid()) {
        tparams.handle = local_mapping_overlay_ip_hdl_;
        mapping_impl_db()->local_mapping_tbl()->release(&tparams);
    }
    if (mapping_hdl_.valid()) {
        tparams.handle = mapping_hdl_;
        mapping_impl_db()->mapping_tbl()->release(&tparams);
    }
    if (local_mapping_public_ip_hdl_.valid()) {
        tparams.handle = local_mapping_public_ip_hdl_;
        mapping_impl_db()->local_mapping_tbl()->release(&tparams);
    }
    if (mapping_public_ip_hdl_.valid()) {
        tparams.handle = mapping_public_ip_hdl_;
        mapping_impl_db()->mapping_tbl()->release(&tparams);
    }
#if 0
    if (ipv4_mac_binding_hw_id_ != PDS_IMPL_RSVD_IP_MAC_BINDING_HW_ID) {
        mapping_impl_db()->ip_mac_binding_idxr()->free(ipv4_mac_binding_hw_id_);
    }
#endif
    if (to_public_ip_nat_idx_ != PDS_IMPL_RSVD_NAT_HW_ID) {
        apulu_impl_db()->nat_idxr()->free(to_public_ip_nat_idx_);
    }
    if (to_overlay_ip_nat_idx_ != PDS_IMPL_RSVD_NAT_HW_ID) {
        apulu_impl_db()->nat_idxr()->free(to_overlay_ip_nat_idx_);
    }
    if (rxdma_local_mapping_tag_idx_ != PDS_IMPL_RSVD_TAG_HW_ID) {
        mapping_impl_db()->local_mapping_tag_idxr()->free(rxdma_local_mapping_tag_idx_);
    }
    if (rxdma_mapping_tag_idx_ != PDS_IMPL_RSVD_TAG_HW_ID) {
        mapping_impl_db()->mapping_tag_idxr()->free(rxdma_mapping_tag_idx_);
    }
    if (rxdma_mapping_hdl_.valid()) {
        tparams.handle = rxdma_mapping_hdl_;
        mapping_impl_db()->rxdma_mapping_tbl()->release(&tparams);
    }
    if (rxdma_mapping_public_ip_hdl_.valid()) {
        tparams.handle = rxdma_mapping_public_ip_hdl_;
        mapping_impl_db()->rxdma_mapping_tbl()->release(&tparams);
    }
    // release any class ids allocated for this mapping
    for (uint32_t i = 0; i < num_class_id_; i++) {
        ret = vpc_impl_db()->free_class_id(class_id_[i], true);
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to free classid %u allocated to local "
                          "mapping %s, err %u",
                          api_obj->key2str().c_str(), ret);
            // continue freeing the resources
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::release_remote_mapping_resources_(api_base *api_obj) {
    sdk_ret_t ret;
    sdk_table_api_params_t tparams = { 0 };

    if (mapping_hdl_.valid()) {
        tparams.handle = mapping_hdl_;
        mapping_impl_db()->mapping_tbl()->release(&tparams);
    }
    if (rxdma_mapping_tag_idx_ != PDS_IMPL_RSVD_TAG_HW_ID) {
        mapping_impl_db()->mapping_tag_idxr()->free(rxdma_mapping_tag_idx_);
    }
    if (rxdma_mapping_hdl_.valid()) {
        tparams.handle = rxdma_mapping_hdl_;
        mapping_impl_db()->rxdma_mapping_tbl()->release(&tparams);
    }

    // release any class ids allocated for this mapping
    for (uint32_t i = 0; i < num_class_id_; i++) {
        ret = vpc_impl_db()->free_class_id(class_id_[i], false);
        if (unlikely(ret != SDK_RET_OK)) {
            PDS_TRACE_ERR("Failed to free classid %u allocated to remote "
                          "mapping %s, err %u",
                          api_obj->key2str().c_str(), ret);
            // continue freeing the resources
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::release_resources(api_base *api_obj) {
    mapping_entry *mapping = (mapping_entry *)api_obj;
    if (mapping->is_local()) {
        return release_local_mapping_resources_(api_obj);
    }
    return release_remote_mapping_resources_(api_obj);
}

sdk_ret_t
mapping_impl::add_nat_entries_(mapping_entry *mapping,
                               pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    p4pd_error_t p4pd_ret;
    nat_rewrite_entry_t nat_data;

    // add private to public IP xlation NAT entry
    PDS_IMPL_FILL_NAT_DATA(&nat_data, &spec->public_ip, 0);
    ret = nat_data.write(to_public_ip_nat_idx_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program NAT table entry %u for overlay IP "
                      "to public IP xlation for mapping %s",
                      to_public_ip_nat_idx_, mapping->key2str().c_str());
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }

    // add public to private/overlay IP xlation NAT entry
    PDS_IMPL_FILL_NAT_DATA(&nat_data, &spec->skey.ip_addr, 0);
    ret = nat_data.write(to_overlay_ip_nat_idx_);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program NAT table entry %u for public IP "
                      "to overlay IP xlation for mapping %s",
                      to_overlay_ip_nat_idx_, mapping->key2str().c_str());
        return sdk::SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

void
mapping_impl::fill_public_ip_mapping_key_data_(
                  vpc_impl *vpc, subnet_entry *subnet,
                  vnic_entry *vnic, vnic_impl *vnic_impl_obj,
                  local_mapping_swkey_t *local_mapping_key,
                  local_mapping_appdata_t *local_mapping_data,
                  sdk::table::handle_t local_mapping_public_ip_hdl,
                  sdk_table_api_params_t *local_mapping_tbl_params,
                  mapping_swkey_t *mapping_key, mapping_appdata_t *mapping_data,
                  sdk::table::handle_t mapping_hdl,
                  sdk_table_api_params_t *mapping_tbl_params,
                  rxdma_mapping_swkey_t *rxdma_mapping_key,
                  rxdma_mapping_appdata_t *rxdma_mapping_data,
                  sdk::table::handle_t rxdma_mapping_public_ip_hdl,
                  sdk_table_api_params_t *rxdma_mapping_tbl_params,
                  pds_mapping_spec_t *spec) {

    // fill LOCAL_MAPPING table entry key, data for public IP
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(local_mapping_key, vpc->hw_id(),
                                         &spec->public_ip);
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_APPDATA(local_mapping_data,
        vnic_impl_obj->hw_id(), to_overlay_ip_nat_idx_,
        rxdma_local_mapping_tag_idx_,
        (vnic->binding_checks_en() &&
         (spec->public_ip.af == IP_AF_IPV4)) ? true : false,
        vnic_impl_obj->binding_hw_id(),
        PDS_IMPL_RSVD_IP_MAC_BINDING_HW_ID, vnic->tagged(),
        MAPPING_TYPE_PUBLIC);
    PDS_IMPL_FILL_TABLE_API_PARAMS(local_mapping_tbl_params,
                                   local_mapping_key,
                                   NULL, local_mapping_data,
                                   LOCAL_MAPPING_LOCAL_MAPPING_INFO_ID,
                                   local_mapping_public_ip_hdl_);

    // fill MAPPING table entry key, data and table params for public IP
    PDS_IMPL_FILL_IP_MAPPING_SWKEY(mapping_key, vpc->hw_id(),
                                   &spec->public_ip);
    memset(mapping_data, 0, sizeof(*mapping_data));
    mapping_data->is_local = TRUE;
    mapping_data->nexthop_valid = TRUE;
    mapping_data->nexthop_type = NEXTHOP_TYPE_NEXTHOP;
    mapping_data->nexthop_id = vnic_impl_obj->nh_idx();
    mapping_data->egress_bd_id = ((subnet_impl *)subnet->impl())->hw_id();
    sdk::lib::memrev(mapping_data->dmaci, vnic->mac(), ETH_ADDR_LEN);
    PDS_IMPL_FILL_TABLE_API_PARAMS(mapping_tbl_params, mapping_key,
                                   NULL, mapping_data,
                                   MAPPING_MAPPING_INFO_ID,
                                   mapping_hdl);

    // fill rxdma MAPPING table entry key, data and table params for public IP
    if (spec->num_tags) {
        memset(rxdma_mapping_data, 0, sizeof(*rxdma_mapping_data));
        PDS_IMPL_FILL_RXDMA_IP_MAPPING_KEY(rxdma_mapping_key, vpc->hw_id(),
                                           &spec->public_ip);
        rxdma_mapping_data->tag_idx = rxdma_mapping_tag_idx_;
        PDS_IMPL_FILL_TABLE_API_PARAMS(rxdma_mapping_tbl_params,
                                       rxdma_mapping_key,
                                       NULL, rxdma_mapping_data,
                                       RXDMA_MAPPING_RXDMA_MAPPING_INFO_ID,
                                       rxdma_mapping_public_ip_hdl);
    }
    return;
}

sdk_ret_t
mapping_impl::add_public_ip_entries_(vpc_impl *vpc, subnet_entry *subnet,
                                     vnic_entry *vnic, vnic_impl *vnic_impl_obj,
                                     mapping_entry *mapping,
                                     pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    mapping_swkey_t mapping_key;
    mapping_appdata_t mapping_data;
    local_mapping_swkey_t local_mapping_key;
    rxdma_mapping_swkey_t rxdma_mapping_key;
    sdk_table_api_params_t mapping_tbl_params;
    local_mapping_appdata_t local_mapping_data;
    rxdma_mapping_appdata_t rxdma_mapping_data;
    sdk_table_api_params_t local_mapping_tbl_params;
    sdk_table_api_params_t rxdma_mapping_tbl_params;

    // program the NAT table first
    ret = add_nat_entries_(mapping, spec);
    SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);

    fill_public_ip_mapping_key_data_(vpc, subnet, vnic, vnic_impl_obj,
                                     &local_mapping_key, &local_mapping_data,
                                     local_mapping_public_ip_hdl_,
                                     &local_mapping_tbl_params,
                                     &mapping_key, &mapping_data,
                                     mapping_public_ip_hdl_,
                                     &mapping_tbl_params,
                                     &rxdma_mapping_key, &rxdma_mapping_data,
                                     rxdma_mapping_public_ip_hdl_,
                                     &rxdma_mapping_tbl_params,
                                     spec);

    // add entry to LOCAL_MAPPING table for public IP
    ret = mapping_impl_db()->local_mapping_tbl()->insert(&local_mapping_tbl_params);
    SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);

    // add entry to MAPPING table for public IP
    ret = mapping_impl_db()->mapping_tbl()->insert(&mapping_tbl_params);
    SDK_ASSERT(ret == SDK_RET_OK);

    // add entry to rxdma MAPPING table for overlay IP
    if (spec->num_tags) {
        ret = mapping_impl_db()->rxdma_mapping_tbl()->insert(&rxdma_mapping_tbl_params);
        SDK_ASSERT(ret == SDK_RET_OK);
    }
    return SDK_RET_OK;
}

void
mapping_impl::fill_local_overlay_ip_mapping_key_data_(
                  vpc_impl *vpc, subnet_entry *subnet,
                  vnic_entry *vnic, vnic_impl *vnic_impl_obj,
                  local_mapping_swkey_t *local_mapping_key,
                  local_mapping_appdata_t *local_mapping_data,
                  sdk::table::handle_t local_mapping_overlay_ip_hdl,
                  sdk_table_api_params_t *local_mapping_tbl_params,
                  mapping_swkey_t *mapping_key, mapping_appdata_t *mapping_data,
                  sdk::table::handle_t mapping_hdl,
                  sdk_table_api_params_t *mapping_tbl_params,
                  rxdma_mapping_swkey_t *rxdma_mapping_key,
                  rxdma_mapping_appdata_t *rxdma_mapping_data,
                  sdk::table::handle_t rxdma_mapping_hdl,
                  sdk_table_api_params_t *rxdma_mapping_tbl_params,
                  pds_mapping_spec_t *spec) {
    // fill LOCAL_MAPPING table entry key, data and table params for overlay IP
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(local_mapping_key, vpc->hw_id(),
                                         &spec->skey.ip_addr);
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_APPDATA(local_mapping_data,
        vnic_impl_obj->hw_id(), to_public_ip_nat_idx_,
        rxdma_local_mapping_tag_idx_,
        (vnic->binding_checks_en() && (spec->skey.ip_addr.af == IP_AF_IPV4)) ?
             true : false, vnic_impl_obj->binding_hw_id(),
        PDS_IMPL_RSVD_IP_MAC_BINDING_HW_ID, vnic->tagged(),
        MAPPING_TYPE_OVERLAY);
    PDS_IMPL_FILL_TABLE_API_PARAMS(local_mapping_tbl_params,
                                   local_mapping_key, NULL,
                                   local_mapping_data,
                                   LOCAL_MAPPING_LOCAL_MAPPING_INFO_ID,
                                   local_mapping_overlay_ip_hdl);

    // fill MAPPING table entry key, data and table params for overlay IP
    PDS_IMPL_FILL_IP_MAPPING_SWKEY(mapping_key, vpc->hw_id(),
                                   &spec->skey.ip_addr);
    memset(mapping_data, 0, sizeof(*mapping_data));
    mapping_data->is_local = TRUE;
    mapping_data->nexthop_valid = TRUE;
    mapping_data->nexthop_type = NEXTHOP_TYPE_NEXTHOP;
    mapping_data->nexthop_id = vnic_impl_obj->nh_idx();
    mapping_data->egress_bd_id = ((subnet_impl *)subnet->impl())->hw_id();
    sdk::lib::memrev(mapping_data->dmaci, vnic->mac(), ETH_ADDR_LEN);
    PDS_IMPL_FILL_TABLE_API_PARAMS(mapping_tbl_params, mapping_key,
                                   NULL, mapping_data,
                                   MAPPING_MAPPING_INFO_ID, mapping_hdl);

    // fill rxdma MAPPING table entry key, data and table params for overlay IP
    if (spec->num_tags) {
        memset(rxdma_mapping_data, 0, sizeof(*rxdma_mapping_data));
        PDS_IMPL_FILL_RXDMA_IP_MAPPING_KEY(rxdma_mapping_key, vpc->hw_id(),
                                           &spec->skey.ip_addr);
        rxdma_mapping_data->tag_idx = rxdma_mapping_tag_idx_;
        PDS_IMPL_FILL_TABLE_API_PARAMS(rxdma_mapping_tbl_params,
                                       rxdma_mapping_key,
                                       NULL, rxdma_mapping_data,
                                       RXDMA_MAPPING_RXDMA_MAPPING_INFO_ID,
                                       rxdma_mapping_hdl);
    }
    return;
}

sdk_ret_t
mapping_impl::add_overlay_ip_mapping_entries_(vpc_impl *vpc,
                                              subnet_entry *subnet,
                                              vnic_entry *vnic,
                                              vnic_impl *vnic_impl_obj,
                                              pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    mapping_swkey_t mapping_key;
    mapping_appdata_t mapping_data;
    rxdma_mapping_swkey_t rxdma_mapping_key;
    local_mapping_swkey_t local_mapping_key;
    sdk_table_api_params_t mapping_tbl_params;
    local_mapping_appdata_t local_mapping_data;
    rxdma_mapping_appdata_t rxdma_mapping_data;
    sdk_table_api_params_t local_mapping_tbl_params;
    sdk_table_api_params_t rxdma_mapping_tbl_params;

    // fill key & data for overlay IP MAPPING and LOCAL_MAPPING table entries
    fill_local_overlay_ip_mapping_key_data_(vpc, subnet, vnic, vnic_impl_obj,
                                            &local_mapping_key,
                                            &local_mapping_data,
                                            local_mapping_overlay_ip_hdl_,
                                            &local_mapping_tbl_params,
                                            &mapping_key, &mapping_data,
                                            mapping_hdl_, &mapping_tbl_params,
                                            &rxdma_mapping_key,
                                            &rxdma_mapping_data,
                                            rxdma_mapping_hdl_,
                                            &rxdma_mapping_tbl_params,
                                            spec);

    // add entry to LOCAL_MAPPING table for overlay IP
    ret = mapping_impl_db()->local_mapping_tbl()->insert(&local_mapping_tbl_params);
    SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);

    // add entry to MAPPING table for overlay IP
    ret = mapping_impl_db()->mapping_tbl()->insert(&mapping_tbl_params);
    SDK_ASSERT(ret == SDK_RET_OK);

    // add entry to rxdma MAPPING table for overlay IP
    if (spec->num_tags) {
        ret = mapping_impl_db()->rxdma_mapping_tbl()->insert(&rxdma_mapping_tbl_params);
        SDK_ASSERT(ret == SDK_RET_OK);
    }
    return ret;
}

static inline sdk_ret_t
local_mapping_tag_fill_class_id_ (local_mapping_tag_info_entry_t *tag_entry,
                                  uint32_t idx, uint32_t class_id) {
    switch (idx) {
    case 0:
        tag_entry->classid0 = class_id;
        break;
    case 1:
        tag_entry->classid1 = class_id;
        break;
    case 2:
        tag_entry->classid2 = class_id;
        break;
    case 3:
        tag_entry->classid3 = class_id;
        break;
    case 4:
        tag_entry->classid4 = class_id;
        break;
    default:
        PDS_TRACE_ERR("Invalid class id index %u for mapping", idx);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
mapping_tag_fill_class_id_ (mapping_tag_info_entry_t *tag_entry,
                            uint32_t idx, uint32_t class_id) {
    switch (idx) {
    case 0:
        tag_entry->classid0 = class_id;
        break;
    case 1:
        tag_entry->classid1 = class_id;
        break;
    case 2:
        tag_entry->classid2 = class_id;
        break;
    case 3:
        tag_entry->classid3 = class_id;
        break;
    case 4:
        tag_entry->classid4 = class_id;
        break;
    default:
        PDS_TRACE_ERR("Invalid class id index %u for mapping", idx);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::add_local_mapping_entries_(vpc_entry *vpc, subnet_entry *subnet,
                                         mapping_entry *mapping,
                                         pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    vnic_entry *vnic;
    vnic_impl *vnic_impl_obj;
    mapping_swkey_t mapping_key;
    mapping_appdata_t mapping_data;
    sdk_table_api_params_t tparams;
    local_mapping_swkey_t local_mapping_key;
    mapping_tag_info_entry_t mapping_tag_data;
    local_mapping_appdata_t local_mapping_data;
    local_mapping_tag_info_entry_t local_mapping_tag_data;

    // get the vnic corresponding to this local mapping
    vnic = vnic_find(&spec->vnic);
    vnic_impl_obj = (vnic_impl *)vnic->impl();

    // program rxdma LOCAL_MAPPING_TAG and MAPPING_TAG tables
    if (spec->num_tags) {
        memset(&local_mapping_tag_data, 0, local_mapping_tag_data.entry_size());
        memset(&mapping_tag_data, 0, mapping_tag_data.entry_size());
        // while programming always program all class-ids including invalid ones
        for (uint32_t i = 0; i < PDS_MAX_TAGS_PER_MAPPING; i++) {
            local_mapping_tag_fill_class_id_(&local_mapping_tag_data, i,
                                             class_id_[i]);
            mapping_tag_fill_class_id_(&mapping_tag_data, i, class_id_[i]);
        }
        ret = local_mapping_tag_data.write(rxdma_local_mapping_tag_idx_);
        SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
        ret = mapping_tag_data.write(rxdma_mapping_tag_idx_);
        SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
    }

    // program public IP related entries in the datapath first
    if (spec->public_ip_valid) {
        ret = add_public_ip_entries_((vpc_impl *)vpc->impl(), subnet, vnic,
                                     vnic_impl_obj, mapping, spec);
        SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
    }
    // and then program the overlay IP mapping specific entries
    ret = add_overlay_ip_mapping_entries_((vpc_impl *)vpc->impl(), subnet,
                                          vnic, (vnic_impl *)vnic->impl(),
                                          spec);
    SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::fill_remote_mapping_key_data_(
                  vpc_entry *vpc, subnet_impl *subnet,
                  mapping_swkey_t *mapping_key, mapping_appdata_t *mapping_data,
                  sdk::table::handle_t mapping_hdl,
                  sdk_table_api_params_t *mapping_table_params,
                  rxdma_mapping_swkey_t *rxdma_mapping_key,
                  rxdma_mapping_appdata_t *rxdma_mapping_data,
                  sdk::table::handle_t rxdma_mapping_hdl,
                  sdk_table_api_params_t *rxdma_mapping_tbl_params,
                  pds_mapping_spec_t *spec) {
    tep_entry *tep;
    tep_impl *tep_impl_obj;
    nexthop_group *nh_group;
    sdk_table_api_params_t tparams;
    nexthop_group_impl *nhgroup_impl;

    // fill MAPPING table entry for overlay IP or MAC
    if (spec->skey.type == PDS_MAPPING_TYPE_L3) {
        PDS_IMPL_FILL_IP_MAPPING_SWKEY(mapping_key,
                                       ((vpc_impl *)vpc->impl())->hw_id(),
                                       &spec->skey.ip_addr);
    } else {
        PDS_IMPL_FILL_L2_MAPPING_SWKEY(mapping_key, subnet->hw_id(),
                                       spec->skey.mac_addr);
    }
    memset(mapping_data, 0, sizeof(*mapping_data));
    mapping_data->nexthop_valid = TRUE;
    switch (spec->nh_type) {
    case PDS_NH_TYPE_OVERLAY:
        tep = tep_find(&spec->tep);
        tep_impl_obj = (tep_impl *)tep->impl();
        mapping_data->nexthop_type = NEXTHOP_TYPE_TUNNEL;
        mapping_data->nexthop_id = tep_impl_obj->hw_id1();
        break;

    case PDS_NH_TYPE_OVERLAY_ECMP:
        nh_group = nexthop_group_find(&spec->nh_group);
        nhgroup_impl = (nexthop_group_impl *)nh_group->impl();
        mapping_data->nexthop_type = NEXTHOP_TYPE_ECMP;
        mapping_data->nexthop_id = nhgroup_impl->hw_id();
        break;

    default:
        return SDK_RET_INVALID_ARG;
        break;
    }
    mapping_data->egress_bd_id = subnet->hw_id();
    if (spec->skey.type == PDS_MAPPING_TYPE_L2) {
        sdk::lib::memrev(mapping_data->dmaci, spec->skey.mac_addr,
                         ETH_ADDR_LEN);
    } else {
        sdk::lib::memrev(mapping_data->dmaci, spec->overlay_mac, ETH_ADDR_LEN);
    }
    PDS_IMPL_FILL_TABLE_API_PARAMS(mapping_table_params, mapping_key,
                                   NULL, mapping_data,
                                   MAPPING_MAPPING_INFO_ID, mapping_hdl);

    // fill rxdma MAPPING table entry key, data and table params for overlay IP
    PDS_IMPL_FILL_RXDMA_IP_MAPPING_KEY(rxdma_mapping_key,
                                       ((vpc_impl *)vpc->impl())->hw_id(),
                                       &spec->skey.ip_addr);
    memset(rxdma_mapping_data, 0, sizeof(*rxdma_mapping_data));
    rxdma_mapping_data->tag_idx = rxdma_mapping_tag_idx_;
    PDS_IMPL_FILL_TABLE_API_PARAMS(rxdma_mapping_tbl_params,
                                   rxdma_mapping_key,
                                   NULL, rxdma_mapping_data,
                                   RXDMA_MAPPING_RXDMA_MAPPING_INFO_ID,
                                   rxdma_mapping_hdl);
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::add_remote_mapping_entries_(vpc_entry *vpc, subnet_entry *subnet,
                                          mapping_entry *mapping,
                                          pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    mapping_swkey_t mapping_key;
    mapping_appdata_t mapping_data;
    rxdma_mapping_swkey_t rxdma_mapping_key;
    mapping_tag_info_entry_t mapping_tag_data;
    sdk_table_api_params_t mapping_tbl_params;
    rxdma_mapping_appdata_t rxdma_mapping_data;
    sdk_table_api_params_t rxdma_mapping_tbl_params;

    // program txdma MAPPING_TAG table
    if (rxdma_mapping_tag_idx_ != PDS_IMPL_RSVD_TAG_HW_ID) {
        memset(&mapping_tag_data, 0, mapping_tag_data.entry_size());
        // while programming always program all class-ids including invalid ones
        for (uint32_t i = 0; i < PDS_MAX_TAGS_PER_MAPPING; i++) {
            mapping_tag_fill_class_id_(&mapping_tag_data, i, class_id_[i]);
        }
        ret = mapping_tag_data.write(rxdma_mapping_tag_idx_);
        SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
    }

    // fill key & data for MAPPING table entry for overlay IP or MAC
    ret = fill_remote_mapping_key_data_(vpc, (subnet_impl *)subnet->impl(),
                                        &mapping_key, &mapping_data,
                                        mapping_hdl_, &mapping_tbl_params,
                                        &rxdma_mapping_key,
                                        &rxdma_mapping_data,
                                        rxdma_mapping_hdl_,
                                        &rxdma_mapping_tbl_params,
                                        spec);
    SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);


    // add entry to MAPPING table for overlay IP or MAC
    ret = mapping_impl_db()->mapping_tbl()->insert(&mapping_tbl_params);
    SDK_ASSERT(ret == SDK_RET_OK);

    // add entry to rxdma MAPPING table for overlay IP
    if (spec->num_tags) {
        ret = mapping_impl_db()->rxdma_mapping_tbl()->insert(&rxdma_mapping_tbl_params);
        SDK_ASSERT(ret == SDK_RET_OK);
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::activate_create_(pds_epoch_t epoch, mapping_entry *mapping,
                               api_obj_ctxt_t *obj_ctxt,
                               pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    vpc_entry *vpc;
    subnet_entry *subnet;

    vpc = vpc_find(&spec->skey.vpc);
    subnet = subnet_find(&spec->subnet);
    PDS_TRACE_DEBUG("Activating (%s) %s %s create, subnet %s, tep %s, "
                    "overlay mac %s, fabric encap type %u "
                    "fabric encap value %u, vnic %s",
                    spec->is_local ? "local" : "remote",
                    mapping->key().str(), mapping->key2str().c_str(),
                    spec->subnet.str(), spec->tep.str(),
                    macaddr2str(spec->overlay_mac), spec->fabric_encap.type,
                    spec->fabric_encap.val.value, spec->vnic.str());
    if (spec->is_local) {
        ret = add_local_mapping_entries_(vpc, subnet, mapping, spec);
        if (ret != SDK_RET_OK) {
            goto error;
        }
        if (!device_find()->overlay_routing_enabled()) {
            if (spec->skey.type == PDS_MAPPING_TYPE_L3) {
                ret = mapping_impl_db()->insert_dhcp_binding(spec);
                if (ret != SDK_RET_OK) {
                    return ret;
                }
            }
        }
    } else {
        ret = add_remote_mapping_entries_(vpc, subnet, mapping, spec);
        if (ret != SDK_RET_OK) {
            goto error;
        }
    }
    ret = mapping_db()->persist(mapping, spec);
    if (unlikely(ret != SDK_RET_OK)) {
        goto error;
    }
    return SDK_RET_OK;

error:

    // TODO: take care of MAC entries also while printing !!
    PDS_TRACE_ERR("Failed to program %s, err %u",
                  mapping->key2str().c_str(), ret);
    return ret;
}

sdk_ret_t
mapping_impl::deactivate_l2_mapping_entry_(pds_obj_key_t subnet,
                                           mac_addr_t mac_addr) {
    sdk_ret_t ret;
    mapping_swkey_t mapping_key;
    sdk_table_api_params_t tparams;
    mapping_appdata_t mapping_data = { 0 };

    PDS_IMPL_FILL_L2_MAPPING_SWKEY(&mapping_key, subnet_hw_id_, mac_addr);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL, &mapping_data,
                                   0, sdk::table::handle_t::null());
    ret = mapping_impl_db()->mapping_tbl()->update(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to deactivate mapping (subnet %s, mac %s)",
                      subnet.str(), macaddr2str(mac_addr));
    }
    return ret;
}

sdk_ret_t
mapping_impl::deactivate_ip_mapping_entry_(pds_obj_key_t vpc, ip_addr_t *ip) {
    sdk_ret_t ret;
    mapping_swkey_t mapping_key;
    sdk_table_api_params_t tparams;
    mapping_appdata_t mapping_data = { 0 };

    PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key, vpc_hw_id_, ip);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL, &mapping_data,
                                   0, sdk::table::handle_t::null());
    ret = mapping_impl_db()->mapping_tbl()->update(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to deactivate mapping (vpc %s, ip %s)",
                      vpc.str(), ipaddr2str(ip));
    }

    return ret;
}

sdk_ret_t
mapping_impl::deactivate_ip_local_mapping_entry_(pds_obj_key_t vpc,
                                                 ip_addr_t *ip) {
    sdk_ret_t ret;
    sdk_table_api_params_t tparams;
    local_mapping_swkey_t local_mapping_key;
    mapping_appdata_t local_mapping_data = { 0 };

    // deactivate mapping entry
    ret = deactivate_ip_mapping_entry_(vpc, ip);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // deactivate local mapping entry
    PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_mapping_key, vpc_hw_id_, ip);
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key, NULL,
                                   &local_mapping_data, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->local_mapping_tbl()->update(&tparams);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to update mapping (vpc %s, ip %s)",
                      vpc.str(), ipaddr2str(ip));
    }
    return ret;
}

sdk_ret_t
mapping_impl::activate_delete_(pds_epoch_t epoch, mapping_entry *mapping) {
    sdk_ret_t ret;

    if (!mapping->is_local()) {
        PDS_TRACE_DEBUG("Deleting remote mapping %s",
                        mapping->key2str().c_str());
        if (mapping->skey().type == PDS_MAPPING_TYPE_L3) {
            // delete the L3 mapping
            ret = deactivate_ip_mapping_entry_(mapping->skey().vpc,
                                               &mapping->skey().ip_addr);
        } else {
            // must be L2 mapping
            ret = deactivate_l2_mapping_entry_(mapping->skey().subnet,
                                               mapping->skey().mac_addr);
        }
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete remote mapping %s, err %u",
                          mapping->key2str().c_str(), ret);
            // continue cleanup !!
        }
    } else {
        PDS_TRACE_DEBUG("Deleting local mapping %s", mapping->key2str().c_str());
        ret = deactivate_ip_local_mapping_entry_(mapping->skey().vpc,
                                                 &mapping->skey().ip_addr);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete local mapping %s, err %u",
                          mapping->key2str().c_str(), ret);
            // continue cleanup !!
        }
        if (mapping->is_public_ip_valid()) {
            ret = deactivate_ip_local_mapping_entry_(mapping->skey().vpc,
                                                     &mapping->public_ip());
        }
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to delete local mapping %s, err %u",
                          mapping->key2str().c_str(), ret);
            // continue cleanup !!
        }
        if (!device_find()->overlay_routing_enabled()) {
            if (mapping->skey().type == PDS_MAPPING_TYPE_L3) {
                ret = mapping_impl_db()->remove_dhcp_binding(
                          mapping->key().str());
           }
        }
    }
    ret = mapping_db()->perish(mapping->key());
    return ret;
}

sdk_ret_t
mapping_impl::upd_public_ip_entries_(vpc_impl *vpc, subnet_entry *subnet,
                                     vnic_entry *vnic, vnic_impl *vnic_impl_obj,
                                     mapping_entry *new_mapping,
                                     mapping_entry *orig_mapping,
                                     pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    mapping_swkey_t mapping_key;
    mapping_appdata_t mapping_data;
    local_mapping_swkey_t local_mapping_key;
    sdk_table_api_params_t mapping_tbl_params;
    local_mapping_appdata_t local_mapping_data;
    sdk_table_api_params_t local_mapping_tbl_params;
    mapping_impl *orig_mapping_impl = (mapping_impl *)orig_mapping->impl();

    // take over the public IP to overlay IP
    to_overlay_ip_nat_idx_ =  orig_mapping_impl->to_overlay_ip_nat_idx_;
    orig_mapping_impl->to_overlay_ip_nat_idx_ = PDS_IMPL_RSVD_NAT_HW_ID;

    // fill key & data of MAPPING and LOCAL_MAPPING table entries corresponding
    // to public IP
    fill_public_ip_mapping_key_data_(vpc, subnet, vnic, vnic_impl_obj,
                                      &local_mapping_key, &local_mapping_data,
                                      sdk::table::handle_t::null(),
                                      &local_mapping_tbl_params,
                                      &mapping_key, &mapping_data,
                                      sdk::table::handle_t::null(),
                                      &mapping_tbl_params,
                                      // TODO: updates for tags
                                      NULL, NULL,
                                      sdk::table::handle_t::null(), NULL,
                                      spec);

    // update LOCAL_MAPPING table entry of public IP
    ret = mapping_impl_db()->local_mapping_tbl()->update(&local_mapping_tbl_params);
    SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
    local_mapping_public_ip_hdl_ = local_mapping_tbl_params.handle;

    // update MAPPING table entry of public IP
    ret = mapping_impl_db()->mapping_tbl()->update(&mapping_tbl_params);
    SDK_ASSERT(ret == SDK_RET_OK);
    mapping_public_ip_hdl_ = mapping_tbl_params.handle;

    // now that cloned object took over the public IP mapping resources,
    // we need the original object to relinquish the ownership
    orig_mapping_impl->local_mapping_public_ip_hdl_ =
        sdk::table::handle_t::null();
    orig_mapping_impl->mapping_public_ip_hdl_ = sdk::table::handle_t::null();
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::upd_overlay_ip_mapping_entries_(vpc_impl *vpc,
                                              subnet_entry *subnet,
                                              vnic_entry *vnic,
                                              vnic_impl *vnic_impl_obj,
                                              mapping_entry *new_mapping,
                                              mapping_entry *orig_mapping,
                                              pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    mapping_swkey_t mapping_key;
    mapping_appdata_t mapping_data;
    local_mapping_swkey_t local_mapping_key;
    sdk_table_api_params_t mapping_tbl_params;
    local_mapping_appdata_t local_mapping_data;
    sdk_table_api_params_t local_mapping_tbl_params;
    mapping_impl *orig_mapping_impl = (mapping_impl *)orig_mapping->impl();

    // take over the overlay IP to public IP
    to_public_ip_nat_idx_ = orig_mapping_impl->to_public_ip_nat_idx_;
    orig_mapping_impl->to_public_ip_nat_idx_ = PDS_IMPL_RSVD_NAT_HW_ID;

    // fill key & data for ovrlay IP MAPPING and LOCAL_MAPPING table entries
    fill_local_overlay_ip_mapping_key_data_(vpc, subnet, vnic, vnic_impl_obj,
                                            &local_mapping_key,
                                            &local_mapping_data,
                                            sdk::table::handle_t::null(),
                                            &local_mapping_tbl_params,
                                            &mapping_key, &mapping_data,
                                            sdk::table::handle_t::null(),
                                            &mapping_tbl_params,
                                            // TODO: updates for tags
                                            NULL, NULL,
                                            sdk::table::handle_t::null(), NULL,
                                            spec);


    // update entry corresponding to overlay IP in LOCAL_MAPPING table
    ret = mapping_impl_db()->local_mapping_tbl()->update(&local_mapping_tbl_params);
    SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
    local_mapping_overlay_ip_hdl_ = local_mapping_tbl_params.handle;

    // update entry corresponding to overlay IP in MAPPING table
    ret = mapping_impl_db()->mapping_tbl()->update(&mapping_tbl_params);
    SDK_ASSERT(ret == SDK_RET_OK);
    mapping_hdl_ = mapping_tbl_params.handle;

    // now that cloned object took over the overlay IP mapping resources,
    // we need the original object to relinquish the ownership
    orig_mapping_impl->local_mapping_overlay_ip_hdl_ =
        sdk::table::handle_t::null();
    orig_mapping_impl->mapping_hdl_ = sdk::table::handle_t::null();
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::upd_local_mapping_entries_(vpc_entry *vpc,
                                         subnet_entry *subnet,
                                         mapping_entry *new_mapping,
                                         mapping_entry *orig_mapping,
                                         api_obj_ctxt_t *obj_ctxt,
                                         pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    vnic_entry *vnic;
    vnic_impl *vnic_impl_obj;
    bool upd_public_ip_mappings = true;
    mapping_impl *orig_impl = (mapping_impl *)orig_mapping->impl();

    // get the vnic corresponding to this local mapping
    vnic = vnic_find(&spec->vnic);
    vnic_impl_obj = (vnic_impl *)vnic->impl();

    // program public IP related entries in the datapath first, if there is a
    // change in the public IP for this mapping (either public IP has changed or
    // public IP is being assigned for the 1st time)
    if (obj_ctxt->upd_bmap & PDS_MAPPING_UPD_PUBLIC_IP) {
        if (spec->public_ip_valid) {
            if (orig_mapping->is_public_ip_valid()) {
                // in this case, mapping had a public IP previously configured
                // but that IP is being updated now
                //
                // before updating the NAT table /1st transfer the ownership of
                // the NAT table resources from original object to the cloned
                // object, after that this operation is same as
                // add_public_ip_entries_() because this is new/updated public
                // IP of the mapping we need to insert into mapping table(s) and
                // also since NAT table is index table, update and add are
                // handled the same way
                to_public_ip_nat_idx_ = orig_impl->to_public_ip_nat_idx_;
                to_overlay_ip_nat_idx_ =  orig_impl->to_overlay_ip_nat_idx_;
                ret = add_public_ip_entries_((vpc_impl *)vpc->impl(), subnet,
                                             vnic, vnic_impl_obj,
                                             new_mapping, spec);
                SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
                orig_impl->to_public_ip_nat_idx_ =
                    orig_impl->to_overlay_ip_nat_idx_ = PDS_IMPL_RSVD_NAT_HW_ID;
                upd_public_ip_mappings = false;
            } else {
                // previously we didn't have the public IP and now it is being
                // configured
                ret = add_public_ip_entries_((vpc_impl *)vpc->impl(),
                                             subnet, vnic, vnic_impl_obj,
                                             new_mapping, spec);
                upd_public_ip_mappings = false;
                SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
            }
        } else {
            // this is the case where previously public IP was configured
            // and the mapping is being updated without the public IP now .. we
            // don't have to do anything here because old version of the object
            // when nuked, will remove the public IP mapping entries and NAT
            // entries previously installed and free all the resources
            upd_public_ip_mappings = false;
        }
    }

    if (spec->public_ip_valid && upd_public_ip_mappings) {
        // only reason why we will update public IP mapping entries at this
        // point is because there is no public IP information change but other
        // attrs might have changed, so own the xfer the public IP related
        // resources
        to_overlay_ip_nat_idx_ = orig_impl->to_overlay_ip_nat_idx_;
        to_public_ip_nat_idx_ = orig_impl->to_public_ip_nat_idx_;
    }
    // and then update the overlay IP mapping specific entries
    ret = upd_overlay_ip_mapping_entries_((vpc_impl *)vpc->impl(), subnet,
                                          vnic, vnic_impl_obj,
                                          new_mapping, orig_mapping, spec);
    SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
    if (spec->public_ip_valid && upd_public_ip_mappings) {
        // public IP attribute itself hasn't been updated but we still have to
        // update rest of the information (e.g., forwarding related)
        ret = upd_public_ip_entries_((vpc_impl *)vpc->impl(), subnet,
                                     vnic, vnic_impl_obj,
                                     new_mapping, orig_mapping, spec);
        SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::upd_remote_mapping_entries_(vpc_entry *vpc,
                                          subnet_entry *subnet,
                                          mapping_entry *new_mapping,
                                          mapping_entry *orig_mapping,
                                          pds_mapping_spec_t *spec,
                                          api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    mapping_swkey_t mapping_key;
    mapping_appdata_t mapping_data;
    rxdma_mapping_swkey_t rxdma_mapping_key;
    mapping_tag_info_entry_t mapping_tag_data;
    rxdma_mapping_appdata_t rxdma_mapping_data;
    sdk_table_api_params_t mapping_table_params;
    rxdma_mapping_appdata_t *rxdma_mapping_data_p;
    sdk_table_api_params_t rxdma_mapping_tbl_params;
    mapping_impl *orig_mapping_impl = (mapping_impl *)orig_mapping->impl();


    if (obj_ctxt->upd_bmap & PDS_MAPPING_UPD_TAGS_ADD) {
        // program txdma MAPPING_TAG table first
        memset(&mapping_tag_data, 0, mapping_tag_data.entry_size());
        // while programming always program all class-ids including invalid ones
        for (uint32_t i = 0; i < PDS_MAX_TAGS_PER_MAPPING; i++) {
            mapping_tag_fill_class_id_(&mapping_tag_data, i, class_id_[i]);
        }
        ret = mapping_tag_data.write(rxdma_mapping_tag_idx_);
        SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
    } else if (obj_ctxt->upd_bmap & PDS_MAPPING_UPD_TAGS_DEL) {
        // update the MAPPING_TAG table entry with invalid tags
        // NOTE:
        // 1. in this case class_id_[] is filled with reserved invalid tags
        // 2. when the original object is deleted all tag related resources
        //    associated with it will be freed, so we don't need to xfer
        //    resource ownership to the cloned object
    } else if (obj_ctxt->upd_bmap & PDS_MAPPING_UPD_TAGS_UPD) {
        // take ownership of resources of needed resource(s) before
        // programming MAPPING_TAG table
        rxdma_mapping_tag_idx_ =  orig_mapping_impl->rxdma_mapping_tag_idx_;
        // program txdma MAPPING_TAG table
        memset(&mapping_tag_data, 0, mapping_tag_data.entry_size());
        // while programming always program all class-ids including invalid ones
        for (uint32_t i = 0; i < PDS_MAX_TAGS_PER_MAPPING; i++) {
            mapping_tag_fill_class_id_(&mapping_tag_data, i, class_id_[i]);
        }
        ret = mapping_tag_data.write(rxdma_mapping_tag_idx_);
        SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
    }

    // fill key & data for MAPPING table entry for overlay IP or MAC
    ret = fill_remote_mapping_key_data_(vpc, (subnet_impl *)subnet->impl(),
                                        &mapping_key, &mapping_data,
                                        sdk::table::handle_t::null(),
                                        &mapping_table_params,
                                        &rxdma_mapping_key,
                                        &rxdma_mapping_data,
                                        rxdma_mapping_hdl_,
                                        &rxdma_mapping_tbl_params,
                                        spec);
    SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);

    // take care of updating tag tables
    if (obj_ctxt->upd_bmap & PDS_MAPPING_UPD_TAGS_ADD) {
        ret = mapping_impl_db()->rxdma_mapping_tbl()->insert(&rxdma_mapping_tbl_params);
        SDK_ASSERT(ret == SDK_RET_OK);
    } else if (obj_ctxt->upd_bmap & PDS_MAPPING_UPD_TAGS_DEL) {
        ret = mapping_impl_db()->rxdma_mapping_tbl()->update(&rxdma_mapping_tbl_params);
        // NOTE: do not take ownership of the resources/handles as they will be
        //       freed when original object is deleted eventually at the end of this
        //       batch
        SDK_ASSERT(ret == SDK_RET_OK);
    } else if (obj_ctxt->upd_bmap & PDS_MAPPING_UPD_TAGS_UPD) {
        // update rxdma MAPPING table to point to the updated MAPPING_TAG table
        // entry
        ret = mapping_impl_db()->rxdma_mapping_tbl()->update(&rxdma_mapping_tbl_params);
        SDK_ASSERT(ret == SDK_RET_OK);
        rxdma_mapping_hdl_ = rxdma_mapping_tbl_params.handle;
        orig_mapping_impl->rxdma_mapping_hdl_ = sdk::table::handle_t::null();
        orig_mapping_impl->rxdma_mapping_tag_idx_ = PDS_IMPL_RSVD_TAG_HW_ID;
    }

    // update entry to MAPPING table for overlay IP or MAC
    ret = mapping_impl_db()->mapping_tbl()->update(&mapping_table_params);
    SDK_ASSERT_RETURN((ret == SDK_RET_OK), ret);
    mapping_hdl_ = mapping_table_params.handle;

    // now that cloned object took over the remote IP/MAC mapping resources
    // in MAPPING table we need the original object to relinquish the ownership
    orig_mapping_impl->mapping_hdl_ = sdk::table::handle_t::null();

    return SDK_RET_OK;
}

// NOTE:
// there is no nee to touch the kvstore or dhcp bindings that are strored
// because primary key to secondary key mapping doesn't change,
// TODO: we need to address case where vnic's MAC changes, should we fix
//       all DHCP bindings as well for the local IP mappings behind that vnic ?
sdk_ret_t
mapping_impl::activate_update_(pds_epoch_t epoch, mapping_entry *new_mapping,
                               mapping_entry *orig_mapping,
                               api_obj_ctxt_t *obj_ctxt,
                               pds_mapping_spec_t *spec) {
    sdk_ret_t ret;
    vpc_entry *vpc;
    subnet_entry *subnet;

    vpc = vpc_find(&spec->skey.vpc);
    subnet = subnet_find(&spec->subnet);
    PDS_TRACE_DEBUG("Activating (%s) %s %s update, subnet %s, tep %s, "
                    "overlay mac %s, fabric encap type %u "
                    "fabric encap value %u, vnic %s",
                    spec->is_local ? "local" : "remote",
                    new_mapping->key().str(), new_mapping->key2str().c_str(),
                    spec->subnet.str(), spec->tep.str(),
                    macaddr2str(spec->overlay_mac), spec->fabric_encap.type,
                    spec->fabric_encap.val.value, spec->vnic.str());

    if (spec->is_local) {
        ret = upd_local_mapping_entries_(vpc, subnet, new_mapping, orig_mapping,
                                         obj_ctxt, spec);
    } else {
        ret = upd_remote_mapping_entries_(vpc, subnet, new_mapping,
                                          orig_mapping, spec, obj_ctxt);
    }
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to update mapping %s, %s, err %u",
                      new_mapping->key().str(), new_mapping->key2str().c_str(),
                      ret);
    }
    return ret;
}

sdk_ret_t
mapping_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                          pds_epoch_t epoch, api_op_t api_op,
                          api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_mapping_key_t *key;
    pds_mapping_spec_t *spec;

    switch (api_op) {
    case API_OP_CREATE:
        spec = &obj_ctxt->api_params->mapping_spec;
        ret = activate_create_(epoch, (mapping_entry *)api_obj,
                               obj_ctxt, spec);
        break;

    case API_OP_DELETE:
        // spec is not available for DELETE operations
        ret = activate_delete_(epoch, (mapping_entry *)api_obj);
        break;

    case API_OP_UPDATE:
        spec = &obj_ctxt->api_params->mapping_spec;
        ret = activate_update_(epoch, (mapping_entry *)api_obj,
                               (mapping_entry *)orig_obj,
                               obj_ctxt, spec);
        break;

    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

sdk_ret_t
mapping_impl::read_remote_mapping_(vpc_entry *vpc, subnet_entry *subnet,
                                   pds_mapping_info_t *info) {
    sdk_ret_t ret;
    p4pd_error_t p4pd_ret;
    bd_actiondata_t bd_data;
    mapping_swkey_t mapping_key;
    nexthop_group_impl *nh_group;
    mapping_appdata_t mapping_data;
    sdk_table_api_params_t tparams;
    pds_mapping_spec_t *spec = &info->spec;
    pds_mapping_status_t *status = &info->status;

    if (info->spec.skey.type == PDS_MAPPING_TYPE_L3) {
        PDS_IMPL_FILL_IP_MAPPING_SWKEY(&mapping_key,
                                       ((vpc_impl *)vpc->impl())->hw_id(),
                                       &spec->skey.ip_addr);
    } else {
        PDS_IMPL_FILL_L2_MAPPING_SWKEY(&mapping_key,
                                       ((subnet_impl *)subnet->impl())->hw_id(),
                                       spec->skey.mac_addr);
    }
    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &mapping_key, NULL,
                                   &mapping_data, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->mapping_tbl()->get(&tparams);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    switch (mapping_data.nexthop_type) {
    case NEXTHOP_TYPE_TUNNEL:
        spec->nh_type = PDS_NH_TYPE_OVERLAY;
        break;
    case NEXTHOP_TYPE_ECMP:
        spec->nh_type = PDS_NH_TYPE_OVERLAY_ECMP;
        break;
    case NEXTHOP_TYPE_NEXTHOP:
        spec->nh_type = PDS_NH_TYPE_UNDERLAY;
        break;
    case NEXTHOP_TYPE_VPC:
        spec->nh_type = PDS_NH_TYPE_PEER_VPC;
        break;
    default:
        return SDK_RET_ERR;
    }
    sdk::lib::memrev(spec->overlay_mac, mapping_data.dmaci, ETH_ADDR_LEN);

    p4pd_ret = p4pd_global_entry_read(P4TBL_ID_BD, mapping_data.egress_bd_id,
                                      NULL, NULL, &bd_data);
    if (p4pd_ret != P4PD_SUCCESS) {
        return sdk::SDK_RET_HW_READ_ERR;
    }
    spec->fabric_encap.val.vnid = bd_data.bd_info.vni;
    spec->fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
    // TODO: tag support
    spec->num_tags = 0;
    status->subnet_hw_id = mapping_data.egress_bd_id;
    subnet_impl *impl = subnet_impl_db()->find(status->subnet_hw_id);
    if (impl) {
        spec->subnet = *impl->key();
    }
    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::read_local_mapping_(vpc_entry *vpc, subnet_entry *subnet,
                                  pds_mapping_info_t *info) {
    sdk_ret_t               ret;
    local_mapping_swkey_t   local_mapping_key;
    local_mapping_appdata_t local_mapping_data;
    sdk_table_api_params_t  tparams;
    nat_rewrite_entry_t     nat_data;
    pds_mapping_spec_t      *spec = &info->spec;
    pds_mapping_status_t    *status = &info->status;
    p4pd_error_t            p4pd_ret;

    // first read the remote mapping, it can provide all the info except vnic id
    ret = read_remote_mapping_(vpc, subnet, info);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // read the local mapping
    if (spec->skey.type == PDS_MAPPING_TYPE_L3) {
        PDS_IMPL_FILL_LOCAL_IP_MAPPING_SWKEY(&local_mapping_key,
                                             ((vpc_impl *)vpc->impl())->hw_id(),
                                             &spec->skey.ip_addr);
    } else {
        PDS_IMPL_FILL_LOCAL_L2_MAPPING_SWKEY(&local_mapping_key,
                                             ((subnet_impl *)subnet->impl())->hw_id(),
                                             spec->skey.mac_addr);
    }

    PDS_IMPL_FILL_TABLE_API_PARAMS(&tparams, &local_mapping_key, NULL,
                                   &local_mapping_data, 0,
                                   sdk::table::handle_t::null());
    ret = mapping_impl_db()->local_mapping_tbl()->get(&tparams);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    status->vnic_hw_id = local_mapping_data.vnic_id;
    // Set vnic uuid in spec
    vnic_impl *vnic = vnic_impl_db()->find(status->vnic_hw_id);
    if (vnic) {
        spec->vnic = *vnic->key();
    }

    if (local_mapping_data.xlate_id != PDS_IMPL_RSVD_NAT_HW_ID) {
        memset(&nat_data, 0, nat_data.entry_size());
        ret = nat_data.read(local_mapping_data.xlate_id);
        if (ret != SDK_RET_OK) {
            return sdk::SDK_RET_HW_READ_ERR;
        }
        spec->public_ip_valid = true;
        P4_IPADDR_TO_IPADDR(nat_data.ip, spec->public_ip,
                            spec->skey.ip_addr.af);
    }
    // TODO: tag support
    spec->num_tags = 0;

    return SDK_RET_OK;
}

sdk_ret_t
mapping_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    sdk_ret_t ret;
    vpc_entry *vpc;
    subnet_entry *subnet;
    pds_obj_key_t vpc_key;
    nat_rewrite_entry_t nat_data;
    pds_mapping_key_t *mkey = (pds_mapping_key_t *)key;
    pds_mapping_info_t *minfo = (pds_mapping_info_t *)info;
    mapping_entry *mapping = (mapping_entry *)api_obj;

    if (mkey->type == PDS_MAPPING_TYPE_L3) {
        vpc = vpc_find(&mkey->vpc);
        subnet = NULL;
    } else {
        subnet = subnet_find(&mkey->subnet);
        if (unlikely(subnet == NULL)) {
            return SDK_RET_INVALID_ARG;
        }
        vpc_key = subnet->vpc();
        vpc = vpc_find(&vpc_key);
    }
    if (vpc == NULL) {
        return SDK_RET_INVALID_ARG;
    }
    if (mapping->is_local()) {
        ret = read_local_mapping_(vpc, subnet, minfo);
    } else {
        ret = read_remote_mapping_(vpc, subnet, minfo);
    }
    return ret;
}

/// \@}

}    // namespace impl
}    // namespace api
