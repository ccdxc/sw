//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// stubs for interfaces needed by learn impl interface
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/learn/learn_impl_base.hpp"

namespace learn {
namespace impl {

uint16_t
arm_to_p4_hdr_sz (void)
{
    return 0;
}

uint16_t
p4_to_arm_hdr_sz (void)
{
    return 0;
}

const char *
learn_lif_name (void)
{
    return nullptr;
}

sdk_ret_t
extract_info_from_p4_hdr (char *pkt, learn_info_t *info)
{
    return SDK_RET_ERR;
}

void
arm_to_p4_tx_hdr_fill (char *tx_hdr, p4_tx_info_t *tx_info)
{
    return;
}

sdk_ret_t
remote_mapping_find (pds_mapping_key_t *key)
{
    return SDK_RET_ERR;
}

}    // namespace impl
}    // namespace learn
