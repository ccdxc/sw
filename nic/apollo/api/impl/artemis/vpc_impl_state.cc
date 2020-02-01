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
#include "nic/apollo/api/impl/artemis/vpc_impl_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_VPC_IMPL_STATE - vpc database functionality
/// \ingroup PDS_VPC
/// @{

vpc_impl_state::vpc_impl_state(pds_state *state) {
    // create indexer for vpc hw id allocation and reserve 0th entry
    vpc_idxr_ = rte_indexer::factory(PDS_MAX_VPC + 2, false, true);
    SDK_ASSERT(vpc_idxr_ != NULL);
}

vpc_impl_state::~vpc_impl_state() {
    rte_indexer::destroy(vpc_idxr_);
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
