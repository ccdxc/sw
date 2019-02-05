/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    rfc_lpm.cc
 *
 * @brief   RFC library interaction with LPM library
 */

#include "nic/apollo/lpm/lpm.hpp"
#include "nic/apollo/rfc/rfc_lpm.hpp"
#include "nic/apollo/rfc/rfc_tree.hpp"

namespace rfc {

sdk_ret_t
rfc_build_lpm_trees (policy_t *policy, rfc_trees_t *rfc_trees)
{
#if 0
    route_table_t    rtable;

    rtable =
        (route_table_t *)
            SDK_MALLOC(OCI_MEM_ALLOC_RFC_LPM,
                       sizeof(route_table_t) +
                           (route_table_info->num_routes * sizeof(route_t)));

    route_table =
#endif
    return SDK_RET_OK;
}

}    // namespace rfc
