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
    v4_lpm_bmap_ = v6_lpm_bmap_ = 0;
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

sdk_ret_t
tag_impl_state::alloc(uint8_t af, uint32_t *lpm_block_idx) {
    uint8_t *lpm_bmap;

    lpm_bmap = (af == IP_AF_IPV4) ? &v4_lpm_bmap_ : &v6_lpm_bmap_;
    if (((*lpm_bmap) & 0x3) == 0x3) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    if (((*lpm_bmap) & 0x1) == 0x0) {
        *lpm_bmap |= 0x1;
    } else {
        *lpm_bmap |= 0x2;
    }
    return SDK_RET_OK;
}

sdk_ret_t
tag_impl_state::free(uint8_t af, uint32_t lpm_block_idx) {
    uint8_t *lpm_bmap;

    if (lpm_block_idx >= 2) {
        return SDK_RET_INVALID_ARG;
    }
    lpm_bmap = (af == IP_AF_IPV4) ? &v4_lpm_bmap_ : &v6_lpm_bmap_;
    *lpm_bmap &= ~(1 << lpm_block_idx);
    return SDK_RET_OK;
}

}    // namespace impl
}    // namespace api
