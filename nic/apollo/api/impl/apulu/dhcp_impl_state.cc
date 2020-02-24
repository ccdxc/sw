//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// DHCP datapath database handling
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_dhcp.hpp"
#include "nic/apollo/api/impl/apulu/apulu_impl.hpp"
#include "nic/apollo/api/impl/apulu/dhcp_impl.hpp"
#include "nic/apollo/api/impl/apulu/dhcp_impl_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_DHCP_IMPL_STATE - DHCP database functionality
/// \ingroup PDS_DHCP
/// \@{

dhcp_relay_impl *
dhcp_impl_state::alloc_relay(void) {
    return (dhcp_relay_impl *)SDK_CALLOC(SDK_MEM_ALLOC_PDS_DHCP_IMPL,
                                         sizeof(dhcp_relay_impl));
}

void
dhcp_impl_state::free(dhcp_relay_impl *impl) {
    SDK_FREE(SDK_MEM_ALLOC_PDS_DHCP_IMPL, impl);
}
/// \@}

}    // namespace impl
}    // namespace api
