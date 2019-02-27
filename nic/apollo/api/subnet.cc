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
subnet_entry::factory(pds_subnet_spec_t *pds_subnet) {
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
    pds_subnet_spec_t *pds_subnet = &api_ctxt->api_params->subnet_info;

    key_.id = pds_subnet->key.id;
    v4_route_table_.id = pds_subnet->v4_route_table.id;
    v6_route_table_.id = pds_subnet->v6_route_table.id;
    ing_v4_policy_.id = pds_subnet->ing_v4_policy.id;
    ing_v6_policy_.id = pds_subnet->ing_v6_policy.id;
    egr_v4_policy_.id = pds_subnet->egr_v4_policy.id;
    egr_v6_policy_.id = pds_subnet->egr_v6_policy.id;
    memcpy(&vr_mac_, &pds_subnet->vr_mac, sizeof(mac_addr_t));
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
    pds_subnet_spec_t *pds_subnet = &obj_ctxt->api_params->subnet_info;

    PDS_TRACE_DEBUG(
        "Creating subnet (vcn %u, subnet %u), pfx %s, vr ip %s, "
        "vr_mac %s, v4 route table %u, v6 route table %u"
        "ingress v4 policy %u, ingress v6 policy %u"
        "egress v4 policy %u, egress v6 policy %u",
        pds_subnet->vcn.id, key_.id, ippfx2str(&pds_subnet->pfx),
        ipaddr2str(&pds_subnet->vr_ip), macaddr2str(pds_subnet->vr_mac),
        pds_subnet->v4_route_table.id, pds_subnet->v6_route_table.id,
        pds_subnet->ing_v4_policy.id, pds_subnet->ing_v6_policy.id,
        pds_subnet->egr_v4_policy.id, pds_subnet->egr_v6_policy.id);
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
    pds_subnet_spec_t *pds_subnet = &obj_ctxt->api_params->subnet_info;

    // there is no h/w programming for subnet config, so nothing to activate
    PDS_TRACE_DEBUG("Created subnet (vcn %u, subnet %u)", pds_subnet->vcn.id,
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
