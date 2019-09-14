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

namespace rfc {

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
