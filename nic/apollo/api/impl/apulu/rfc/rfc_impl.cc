//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// pipeline specific implementation of RFC APIs
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/impl/rfc/rfc_impl.hpp"
#include "nic/apollo/p4/include/apulu_sacl_defines.h"

namespace rfc {

uint16_t
sacl_sip_v4_tree_max_classes (void)
{
    return SACL_IPV4_SIP_TREE_MAX_CLASSES;
}

uint16_t
sacl_dip_v4_tree_max_classes (void)
{
    return SACL_IPV4_DIP_TREE_MAX_CLASSES;
}

uint16_t
sacl_sip_v6_tree_max_classes (void)
{
    return SACL_IPV6_SIP_TREE_MAX_CLASSES;
}

uint16_t
sacl_dip_v6_tree_max_classes (void)
{
    return SACL_IPV6_DIP_TREE_MAX_CLASSES;
}

uint16_t
sacl_sport_tree_max_classes (void)
{
    return SACL_SPORT_TREE_MAX_CLASSES;
}

uint16_t
sacl_proto_dport_tree_max_classes (void)
{
    return SACL_PROTO_DPORT_TREE_MAX_CLASSES;
}

uint16_t
sacl_stag_tree_max_classes (void)
{
    return SACL_TAG_TREE_MAX_CLASSES;
}

uint16_t
sacl_dtag_tree_max_classes (void)
{
    return SACL_TAG_TREE_MAX_CLASSES;
}

uint16_t
sacl_p1_max_classes (void)
{
    return SACL_P1_MAX_CLASSES;
}

uint16_t
sacl_p2_max_classes (void)
{
    return SACL_P2_MAX_CLASSES;
}

sdk_ret_t
rfc_build_itables (rfc_ctxt_t *rfc_ctxt)
{
    return SDK_RET_ERR;
}

sdk_ret_t
rfc_sort_itables (rfc_ctxt_t *rfc_ctxt)
{
    return SDK_RET_ERR;
}

sdk_ret_t
rfc_compute_p0_classes (rfc_ctxt_t *rfc_ctxt)
{
    return SDK_RET_ERR;
}

sdk_ret_t
rfc_build_lpm_trees (rfc_ctxt_t *rfc_ctxt,
                     mem_addr_t rfc_tree_root_addr, uint32_t mem_size)
{
    return SDK_RET_ERR;
}

sdk_ret_t
rfc_build_eqtables (rfc_ctxt_t *rfc_ctxt)
{
    return SDK_RET_ERR;
}

}    // namespace rfc
