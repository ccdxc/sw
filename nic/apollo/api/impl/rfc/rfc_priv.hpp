//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// RFC library internal/private datastructures and APIs shared across pipelines
///
//----------------------------------------------------------------------------

#ifndef __RFC_PRIV_HPP__
#define __RFC_PRIV_HPP__

#include "nic/apollo/api/impl/lpm/lpm.hpp"
#include "nic/apollo/api/impl/rfc/rfc_tree.hpp"

namespace rfc {

bool rfc_p0_pfx_tree_inode_eq_cb(inode_t *inode1, inode_t *inode2);
bool rfc_p0_port_tree_inode_eq_cb(inode_t *inode1, inode_t *inode2);
bool rfc_p0_proto_port_tree_inode_eq_cb(inode_t *inode1, inode_t *inode2);
void rfc_itree_dump(rfc_tree_t *rfc_tree, itree_type_t tree_type);

/**
 * @brief    dump the contents of given equivalence table
 * @param[in] rfc_table RFC table with all the information about the tree
 */
void rfc_eq_class_table_dump(rfc_table_t *rfc_table);

sdk_ret_t rfc_ctxt_init(rfc_ctxt_t *rfc_ctxt, policy_t *policy,
                        mem_addr_t base_addr, uint32_t mem_size);
void rfc_ctxt_destroy(rfc_ctxt_t *rfc_ctxt);

}    // namespace rfc

#endif    // __RFC_PRIV_HPP__
