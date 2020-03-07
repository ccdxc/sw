//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// mirror session datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/include/pds_mirror.hpp"
#include "nic/apollo/api/impl/apollo/pds_impl_state.hpp"
#include "nic/apollo/api/impl/apollo/mirror_impl.hpp"
#include "nic/apollo/api/impl/apollo/mirror_impl_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_MIRROR_IMPL_STATE - mirror session database functionality
/// \ingroup PDS_MIRROR
/// \@{

mirror_impl_state::mirror_impl_state(pds_state *state) {
    session_bmap_ = 0;
    mirror_impl_slab_ = slab::factory("mirror-impl", PDS_SLAB_ID_MIRROR_IMPL,
                                      sizeof(mirror_impl), 8, true, true);
    SDK_ASSERT(mirror_impl_slab_ != NULL);
}

mirror_impl_state::~mirror_impl_state() {
    slab::destroy(mirror_impl_slab_);
}

mirror_impl *
mirror_impl_state::alloc(void) {
    return ((mirror_impl *)mirror_impl_slab_->alloc());
}

void
mirror_impl_state::free(mirror_impl *impl) {
    mirror_impl_slab_->free(impl);
}

sdk_ret_t
mirror_impl_state::alloc_hw_id(pds_obj_key_t *key, uint16_t *hw_id) {
    if (session_bmap_ != 0xFF) {
        for (uint8_t i = 0; i < PDS_MAX_MIRROR_SESSION; i++) {
            if (!(session_bmap_ & (1 << i))) {
                *hw_id = i;
                return SDK_RET_OK;
            }
        }
    }
    return SDK_RET_NO_RESOURCE;
}

sdk_ret_t
mirror_impl_state::free_hw_id(uint16_t hw_id) {
    if (hw_id >= PDS_MAX_MIRROR_SESSION) {
        return SDK_RET_INVALID_ARG;
    }
    hw_id &= ~(1 << hw_id);
    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
