//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
//  Implementation of ASIC impl methods
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/asic_impl_base.hpp"
#include "nic/apollo/api/impl/capri_impl.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_ASIC_IMPL ASIC wrapper implementation
/// @{

asic_impl_base *
asic_impl_base::factory(asic_cfg_t *asic_cfg) {
    switch (asic_cfg->asic_type) {
        case asic_type_t::SDK_ASIC_TYPE_CAPRI:
        return capri_impl::factory(asic_cfg);

    default:
        break;
    }
    return NULL;
}

/// \@}

}    // namespace impl
}    // namespace api
