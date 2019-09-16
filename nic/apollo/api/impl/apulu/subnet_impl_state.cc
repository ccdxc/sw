//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// subnet datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/impl/apulu/subnet_impl_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_SUBNET_IMPL_STATE - subnet database functionality
/// \ingroup PDS_SUBNET
/// @{

subnet_impl_state::subnet_impl_state(pds_state *state) {
}

subnet_impl_state::~subnet_impl_state() {
}

sdk_ret_t
subnet_impl_state::table_transaction_begin(void) {
    return SDK_RET_OK;
}

sdk_ret_t
subnet_impl_state::table_transaction_end(void) {
    return SDK_RET_OK;
}

sdk_ret_t
subnet_impl_state::table_stats(debug::table_stats_get_cb_t cb, void *ctxt) {
    return SDK_RET_OK;
}

/// @}

}    // namespace impl
}    // namespace api
