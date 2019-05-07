//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// Implementation of pipeline impl methods
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/pipeline_impl_base.hpp"
#include "nic/apollo/api/impl/apollo_impl.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_PIPELINE_IMPL Pipeline wrapper implementation
/// @{

pipeline_impl_base *
pipeline_impl_base::factory(pipeline_cfg_t *pipeline_cfg) {
    if (pipeline_cfg->name == "apollo") {
        pipeline_impl_base *impl;
        impl = apollo_impl::factory(pipeline_cfg);
        impl->pipeline_cfg_ = *pipeline_cfg;
        return impl;
    }
    return NULL;
}

void
pipeline_impl_base::destroy(pipeline_impl_base *impl) {
    if (impl->pipeline_cfg_.name == "apollo") {
        apollo_impl::destroy(static_cast<apollo_impl*>(impl));
    }
}

/// \@}

}    // namespace impl
}    // namespace api
