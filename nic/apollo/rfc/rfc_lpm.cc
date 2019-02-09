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
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/p4/include/slacl_defines.h"

namespace rfc {

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

/**
 * @brief        given tree type and base address of the RFC policy block in
 *               HBM, return the address of the root node and the size of
 *               the LPM tree
 * @param[in] rfc_block_base_addr   base address of the memory block reserved
 *                                  for the entire policy including all LPM
 *                                  trees and index tables used in subsequent
 *                                  phases of RFC
 * @param[in] tree_type             type of the LPM tree we are building
 * @param[out] tree_base_addr       base address where the LPM tree starts
 * @param[out] tree_size            size of the LPM tree in bytes
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
tree_base_addr_size (mem_addr_t rfc_block_base_addr, itree_type_t tree_type,
                     mem_addr_t *tree_base_addr, uint32_t *tree_size)
{
    switch (tree_type) {
    case ITREE_TYPE_IPV4:
        *tree_base_addr = rfc_block_base_addr + SLACL_IPV4_TABLE_OFFSET;
        *tree_size = SLACL_IPV4_TABLE_SIZE;
        break;

    case ITREE_TYPE_PORT:
        *tree_base_addr = rfc_block_base_addr + SLACL_SPORT_TABLE_OFFSET;
        *tree_size = SLACL_SPORT_TABLE_SIZE;
        break;

    case ITREE_TYPE_PROTO_PORT:
        *tree_base_addr = rfc_block_base_addr + SLACL_PROTO_DPORT_TABLE_OFFSET;
        *tree_size = SLACL_PROTO_DPORT_TABLE_SIZE;
        break;

    default:
        SDK_ASSERT(0);
        break;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
rfc_build_lpm_tree (lpm_itable_t *lpm_itable, rfc_tree_t *rfc_tree,
                    mem_addr_t tree_base_addr, uint32_t mem_size,
                    uint32_t max_rules)
{
    sdk_ret_t    ret;
    itable_t     *itable = &rfc_tree->itable;

    for (uint32_t i = 0; i < lpm_itable->num_intervals; i++) {
        if ((lpm_itable->tree_type == ITREE_TYPE_IPV4) ||
            (lpm_itable->tree_type == ITREE_TYPE_IPV6)) {
            lpm_itable->nodes[i].ipaddr = itable->nodes[i].ipaddr;
        } else if (lpm_itable->tree_type == ITREE_TYPE_PORT) {
            lpm_itable->nodes[i].port = itable->nodes[i].port;
        } else if (lpm_itable->tree_type == ITREE_TYPE_PROTO_PORT) {
            lpm_itable->nodes[i].key32 = itable->nodes[i].key32;
        }
        lpm_itable->nodes[i].data = itable->nodes[i].rfc.class_id;
    }
    ret = lpm_build_tree(lpm_itable, rfc_tree->num_classes - 1,
                         max_rules, tree_base_addr, mem_size);
    if (ret != SDK_RET_OK) {
        OCI_TRACE_ERR("Failed to build RFC tree type %u, err : %u",
                      lpm_itable->tree_type, ret);
    }
    return ret;
}

sdk_ret_t
rfc_build_lpm_trees (policy_t *policy, rfc_trees_t *rfc_trees,
                     mem_addr_t rfc_tree_root_addr, uint32_t mem_sz)
{
    sdk_ret_t       ret;
    lpm_itable_t    itable;
    uint32_t        max_intervals, tree_size;
    mem_addr_t      tree_base_addr;

    max_intervals = MAX(rfc_trees->pfx_tree.num_intervals,
                        rfc_trees->port_tree.num_intervals);
    max_intervals = MAX(rfc_trees->proto_port_tree.num_intervals,
                        max_intervals);

    /**
     * allocate memory for max intervals so we don't have to allocate & free
     * per tree
     */
    memset(&itable, 0, sizeof(itable));
    itable.nodes = (lpm_inode_t *)malloc(max_intervals * sizeof(lpm_inode_t));
    if (unlikely(itable.nodes == NULL)) {
        return sdk::SDK_RET_OOM;
    }

    /**< build LPM tree for the prefix portion of the rules */
    itable.tree_type =
        (policy->af == IP_AF_IPV4) ? ITREE_TYPE_IPV4 : ITREE_TYPE_IPV6;
    itable.num_intervals = rfc_trees->pfx_tree.num_intervals;
    tree_base_addr_size(rfc_tree_root_addr, itable.tree_type,
                        &tree_base_addr, &tree_size);
    ret = rfc_build_lpm_tree(&itable, &rfc_trees->pfx_tree,
                             tree_base_addr, tree_size, policy->max_rules);
    if (ret != SDK_RET_OK) {
        goto cleanup;
    }

    /**< build LPM tree for the port match portion of the rules */
    itable.tree_type = ITREE_TYPE_PORT;
    itable.num_intervals = rfc_trees->port_tree.num_intervals;
    tree_base_addr_size(rfc_tree_root_addr, itable.tree_type,
                        &tree_base_addr, &tree_size);
    ret = rfc_build_lpm_tree(&itable, &rfc_trees->port_tree,
                             tree_base_addr, tree_size, policy->max_rules);
    if (ret != SDK_RET_OK) {
        goto cleanup;
    }

    /**< build LPM tree for the (protocol, port) match portion of the rules */
    itable.tree_type = ITREE_TYPE_PROTO_PORT;
    itable.num_intervals = rfc_trees->proto_port_tree.num_intervals;
    tree_base_addr_size(rfc_tree_root_addr, itable.tree_type,
                        &tree_base_addr, &tree_size);
    ret = rfc_build_lpm_tree(&itable, &rfc_trees->port_tree,
                             tree_base_addr, tree_size, policy->max_rules);
    if (ret != SDK_RET_OK) {
        goto cleanup;
    }
    return SDK_RET_OK;

cleanup:

    if (itable.nodes) {
        free(itable.nodes);
    }
    return ret;
}

}    // namespace rfc
