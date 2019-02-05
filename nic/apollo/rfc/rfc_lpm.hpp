/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    rfc_lpm.hpp
 *
 * @brief   RFC library interactions with LPM library
 */

#if !defined (__RFC_LPM_HPP__)
#define __RFC_LPM_HPP__

#include "nic/apollo/rfc/rfc.hpp"
#include "nic/apollo/rfc/rfc_tree.hpp"

namespace rfc {

sdk_ret_t rfc_build_lpm_trees(policy_t *policy, rfc_trees_t *rfc_trees);

}    // namespace rfc

#endif    /** __RFC_TREE_HPP__ */
