//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vpc datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/impl/apulu/vpc_impl_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_VPC_IMPL_STATE - vpc database functionality
/// \ingroup PDS_VPC
/// @{

vpc_impl_state::vpc_impl_state(pds_state *state) {
}

vpc_impl_state::~vpc_impl_state() {
}

sdk_ret_t
vpc_impl_state::table_transaction_begin(void) {
    return SDK_RET_OK;
}

sdk_ret_t
vpc_impl_state::table_transaction_end(void) {
    return SDK_RET_OK;
}

/// @}

}    // namespace impl
}    // namespace api
