//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// implementation of pipeline impl methods
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/pipeline_impl_base.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_PIPELINE_IMPL Pipeline wrapper implementation
/// @{

pipeline_impl_base *
pipeline_impl_base::factory(pipeline_cfg_t *pipeline_cfg) {
    pipeline_impl_base *impl;
    impl = apulu_impl::factory(pipeline_cfg);
    impl->pipeline_cfg_ = *pipeline_cfg;
    return impl;
}

void
pipeline_impl_base::destroy(pipeline_impl_base *impl) {
    apulu_impl::destroy(static_cast<apulu_impl *>(impl));
}

/// \@}

}    // namespace impl
}    // namespace api
