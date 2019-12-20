//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// DHCP API object handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/dhcp.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

// DHCP relay API implementation
dhcp_relay::dhcp_relay() {
    ht_ctxt_.reset();
}

dhcp_relay::~dhcp_relay() {
}

dhcp_relay *
dhcp_relay::factory(pds_dhcp_relay_spec_t *spec) {
    dhcp_relay *relay;

    // create DHCP relay entry with defaults, if any
    relay = dhcp_db()->alloc_relay();
    return relay;
}

void
dhcp_relay::destroy(dhcp_relay *relay) {
    relay->~dhcp_relay();
    dhcp_db()->free(relay);
}

api_base *
dhcp_relay::clone(api_ctxt_t *api_ctxt) {
    dhcp_relay *cloned_relay;

    cloned_relay = dhcp_db()->alloc_relay();
    if (cloned_relay) {
        new (cloned_relay) dhcp_relay();
        cloned_relay->init_config(api_ctxt);
    }
    return cloned_relay;
}

sdk_ret_t
dhcp_relay::free(dhcp_relay *relay) {
    relay->~dhcp_relay();
    dhcp_db()->free(relay);
    return SDK_RET_OK;
}

sdk_ret_t
dhcp_relay::init_config(api_ctxt_t *api_ctxt) {
    key_.id = api_ctxt->api_params->dhcp_relay_spec.key.id;
    return SDK_RET_OK;
}

void
dhcp_relay::fill_spec_(pds_dhcp_relay_spec_t *spec) {
}

sdk_ret_t
dhcp_relay::read(pds_dhcp_relay_info_t *info) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
dhcp_relay::add_to_db(void) {
    return dhcp_db()->insert(this);
}

sdk_ret_t
dhcp_relay::del_from_db(void) {
    if (dhcp_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
dhcp_relay::update_db(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (dhcp_db()->remove((dhcp_relay *)orig_obj)) {
        return dhcp_db()->insert(this);
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
dhcp_relay::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_DHCP_RELAY, this);
}

// DHCP policy API implementation

dhcp_policy::dhcp_policy() {
    ht_ctxt_.reset();
}

dhcp_policy *
dhcp_policy::factory(pds_dhcp_policy_spec_t *spec) {
    dhcp_policy *policy;

    // create DHCP policy entry with defaults, if any
    policy = dhcp_db()->alloc_policy();
    return policy;
}

dhcp_policy::~dhcp_policy() {
}

void
dhcp_policy::destroy(dhcp_policy *policy) {
    policy->~dhcp_policy();
    dhcp_db()->free(policy);
}

api_base *
dhcp_policy::clone(api_ctxt_t *api_ctxt) {
    dhcp_policy *cloned_policy;

    cloned_policy = dhcp_db()->alloc_policy();
    if (cloned_policy) {
        new (cloned_policy) dhcp_policy();
        cloned_policy->init_config(api_ctxt);
    }
    return cloned_policy;
}

sdk_ret_t
dhcp_policy::free(dhcp_policy *policy) {
    policy->~dhcp_policy();
    dhcp_db()->free(policy);
    return SDK_RET_OK;
}

sdk_ret_t
dhcp_policy::init_config(api_ctxt_t *api_ctxt) {
    key_.id = api_ctxt->api_params->dhcp_policy_spec.key.id;
    return SDK_RET_OK;
}

void
dhcp_policy::fill_spec_(pds_dhcp_policy_spec_t *spec) {
    // TODO:
}

sdk_ret_t
dhcp_policy::read(pds_dhcp_policy_info_t *info) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
dhcp_policy::add_to_db(void) {
    return dhcp_db()->insert(this);
}

sdk_ret_t
dhcp_policy::del_from_db(void) {
    if (dhcp_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
dhcp_policy::update_db(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (dhcp_db()->remove((dhcp_policy *)orig_obj)) {
        return dhcp_db()->insert(this);
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
dhcp_policy::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_DHCP_POLICY, this);
}

}    // namespace api
