//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// subnet entry handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

subnet_entry::subnet_entry() {
    // SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    v4_route_table_.id = PDS_ROUTE_TABLE_ID_INVALID;
    v6_route_table_.id = PDS_ROUTE_TABLE_ID_INVALID;
    ing_v4_policy_.id = PDS_POLICY_ID_INVALID;
    ing_v6_policy_.id = PDS_POLICY_ID_INVALID;
    egr_v4_policy_.id = PDS_POLICY_ID_INVALID;
    egr_v6_policy_.id = PDS_POLICY_ID_INVALID;
    ht_ctxt_.reset();
    hw_id_ = 0xFFFF;
}

subnet_entry *
subnet_entry::factory(pds_subnet_spec_t *spec) {
    subnet_entry *subnet;

    // create subnet entry with defaults, if any
    subnet = subnet_db()->subnet_alloc();
    if (subnet) {
        new (subnet) subnet_entry();
    }
    return subnet;
}

subnet_entry::~subnet_entry() {
    // TODO: fix me
    // SDK_SPINLOCK_DESTROY(&slock_);
}

void
subnet_entry::destroy(subnet_entry *subnet) {
    subnet->release_resources();
    subnet->~subnet_entry();
    subnet_db()->subnet_free(subnet);
}

sdk_ret_t
subnet_entry::init_config(api_ctxt_t *api_ctxt) {
    pds_subnet_spec_t *spec = &api_ctxt->api_params->subnet_spec;

    key_.id = spec->key.id;
    v4_route_table_.id = spec->v4_route_table.id;
    v6_route_table_.id = spec->v6_route_table.id;
    ing_v4_policy_.id = spec->ing_v4_policy.id;
    ing_v6_policy_.id = spec->ing_v6_policy.id;
    egr_v4_policy_.id = spec->egr_v4_policy.id;
    egr_v6_policy_.id = spec->egr_v6_policy.id;
    memcpy(&vr_mac_, &spec->vr_mac, sizeof(mac_addr_t));
    this->ht_ctxt_.reset();
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    if (subnet_db()->subnet_idxr()->alloc((uint32_t *)&this->hw_id_) !=
            sdk::lib::indexer::SUCCESS) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::program_config(obj_ctxt_t *obj_ctxt) {
    // there is no h/w programming for subnet config but a h/w id is needed so
    // we can use while programming vnics, routes etc.
    pds_subnet_spec_t *spec = &obj_ctxt->api_params->subnet_spec;

    PDS_TRACE_DEBUG(
        "Creating subnet (vcn %u, subnet %u), pfx %s, vr ip %s, "
        "vr_mac %s, v4 route table %u, v6 route table %u, "
        "ingress v4 policy %u, ingress v6 policy %u, "
        "egress v4 policy %u, egress v6 policy %u",
        spec->vcn.id, key_.id, ippfx2str(&spec->pfx),
        ipaddr2str(&spec->vr_ip), macaddr2str(spec->vr_mac),
        spec->v4_route_table.id, spec->v6_route_table.id,
        spec->ing_v4_policy.id, spec->ing_v6_policy.id,
        spec->egr_v4_policy.id, spec->egr_v6_policy.id);
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::release_resources(void) {
    if (hw_id_ != 0xFF) {
        subnet_db()->subnet_idxr()->free(hw_id_);
    }
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::cleanup_config(obj_ctxt_t *obj_ctxt) {
    // impl->cleanup_hw();
    // there is no h/w programming for VCN config, so nothing to cleanup
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // impl->update_hw();
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
subnet_entry::activate_config(pds_epoch_t epoch, api_op_t api_op,
                              obj_ctxt_t *obj_ctxt) {
    pds_subnet_spec_t *spec = &obj_ctxt->api_params->subnet_spec;

    // there is no h/w programming for subnet config, so nothing to activate
    PDS_TRACE_DEBUG("Created subnet (vcn %u, subnet %u)", spec->vcn.id,
                    key_.id);
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // nothing to update on subnet, so no updates supported
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
subnet_entry::add_to_db(void) {
    return subnet_db()->subnet_ht()->insert_with_key(&key_, this, &ht_ctxt_);
}

sdk_ret_t
subnet_entry::del_from_db(void) {
    subnet_db()->subnet_ht()->remove(&key_);
    return SDK_RET_OK;
}

sdk_ret_t
subnet_entry::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_SUBNET, this);
}

}    // namespace api
