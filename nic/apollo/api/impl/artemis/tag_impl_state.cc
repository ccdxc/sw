//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// tag datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/impl/artemis/tag_impl_state.hpp"

namespace api {
namespace impl {

tag_impl_state::tag_impl_state(pds_state *state) {
    v4_lpm_region_addr_ = state->mempartition()->start_addr("tag_v4");
    SDK_ASSERT(v4_lpm_region_addr_ != INVALID_MEM_ADDRESS);
    v4_lpm_table_size_ = state->mempartition()->block_size("tag_v4");
    v4_lpm_max_prefixes_ = state->mempartition()->max_elements("tag_v4") - 1;
    v6_lpm_region_addr_ = state->mempartition()->start_addr("tag_v6");
    SDK_ASSERT(v6_lpm_region_addr_ != INVALID_MEM_ADDRESS);
    v6_lpm_table_size_ = state->mempartition()->block_size("tag_v6");
    v6_lpm_max_prefixes_ = state->mempartition()->max_elements("tag_v6") - 1;
}

tag_impl_state::~tag_impl_state() {
}

sdk_ret_t
tag_impl_state::table_transaction_begin(void) {
    return SDK_RET_OK;
}

sdk_ret_t
tag_impl_state::table_transaction_end(void) {
    return SDK_RET_OK;
}

}    // namespace impl
}    // namespace api
