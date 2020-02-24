//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of DHCP objects
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/dhcp.hpp"
#include "nic/apollo/api/impl/apulu/dhcp_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_DHCP_IMPL - DHCP datapath implementation
/// \ingroup PDS_DHCP
/// \@{

dhcp_relay_impl *
dhcp_relay_impl::factory(pds_dhcp_relay_spec_t *spec) {
    dhcp_relay_impl *impl;

    impl = dhcp_impl_db()->alloc_relay();
    if (unlikely(impl == NULL)) {
        return NULL;
    }
    new (impl) dhcp_relay_impl();
    return impl;
}

void
dhcp_relay_impl::soft_delete(dhcp_relay_impl *impl) {
    impl->~dhcp_relay_impl();
    dhcp_impl_db()->free(impl);
}

void
dhcp_relay_impl::destroy(dhcp_relay_impl *impl) {
    dhcp_relay_impl::soft_delete(impl);
}

impl_base *
dhcp_relay_impl::clone(void) {
    dhcp_relay_impl *cloned_impl;

    cloned_impl = dhcp_impl_db()->alloc_relay();
    new (cloned_impl) dhcp_relay_impl();
    // deep copy is not needed as we don't store pointers
    *cloned_impl = *this;
    return cloned_impl;
}

sdk_ret_t
dhcp_relay_impl::free(dhcp_relay_impl *impl) {
    destroy(impl);
    return SDK_RET_OK;
}

dhcp_relay_impl *
dhcp_relay_impl::build(pds_obj_key_t *key, dhcp_relay *relay) {
    dhcp_relay_impl *impl;

    impl = dhcp_impl_db()->alloc_relay();
    if (unlikely(impl == NULL)) {
        return NULL;
    }
    new (impl) dhcp_relay_impl();
    return impl;
}

sdk_ret_t
dhcp_relay_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                             pds_epoch_t epoch, api_op_t api_op,
                             api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    pds_mapping_key_t *key;
    pds_dhcp_relay_spec_t *spec;

    return SDK_RET_OK;

    switch (api_op) {
    case API_OP_CREATE:
    case API_OP_UPDATE:
        spec = &obj_ctxt->api_params->dhcp_relay_spec;
        // install the NACL
        break;

    case API_OP_DELETE:
        // clear the NACL
        break;

    default:
        ret = sdk::SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

sdk_ret_t
dhcp_relay_impl::read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info) {
    return SDK_RET_INVALID_OP;
}

/// \@}    // end of PDS_DHCP_IMPL

}    // namespace impl
}    // namespace api
