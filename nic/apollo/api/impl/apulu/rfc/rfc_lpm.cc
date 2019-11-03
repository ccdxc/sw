/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    rfc_lpm.cc
 *
 * @brief   RFC library interaction with LPM library
 */

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/impl/lpm/lpm.hpp"
#include "nic/apollo/api/impl/rfc/rfc_tree.hpp"
#include "nic/apollo/api/impl/apulu/rfc/rfc_utils.hpp"
#include "nic/apollo/p4/include/apulu_sacl_defines.h"

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
    case ITREE_TYPE_IPV4_SIP_ACL:
        *tree_base_addr = rfc_block_base_addr + SACL_IPV4_SIP_TABLE_OFFSET;
        *tree_size = SACL_IPV4_SIP_TABLE_SIZE;
        break;

    case ITREE_TYPE_IPV6_SIP_ACL:
        *tree_base_addr = rfc_block_base_addr + SACL_IPV6_SIP_TABLE_OFFSET;
        *tree_size = SACL_IPV6_SIP_TABLE_SIZE;
        break;

    case ITREE_TYPE_IPV4_DIP_ACL:
        *tree_base_addr = rfc_block_base_addr + SACL_DIP_TABLE_OFFSET;
        *tree_size = SACL_IPV4_DIP_TABLE_SIZE;
        break;

    case ITREE_TYPE_IPV6_DIP_ACL:
        *tree_base_addr = rfc_block_base_addr + SACL_DIP_TABLE_OFFSET;
        *tree_size = SACL_IPV6_DIP_TABLE_SIZE;
        break;

    case ITREE_TYPE_PORT:
        *tree_base_addr = rfc_block_base_addr + SACL_SPORT_TABLE_OFFSET;
        *tree_size = SACL_SPORT_TABLE_SIZE;
        break;

    case ITREE_TYPE_PROTO_PORT:
        *tree_base_addr = rfc_block_base_addr + SACL_PROTO_DPORT_TABLE_OFFSET;
        *tree_size = SACL_PROTO_DPORT_TABLE_SIZE;
        break;

    default:
        SDK_ASSERT(0);
        break;
    }
    return SDK_RET_OK;
}

/**
 * @brief        build phase 0 LPM tree for given RFC tree
 * @param[in] lpm_itbable    LPM interval table containing all the intervals
 *                           corresponding to the RFC tree
 * @param[in] rfc_tree       RFC prefix/port/proto-port tree
 * @param[in] tree_base_addr base address where the LPM interval tree will be
 *                           written in memory
 * @param[in] mem_size       size of the memory block reserved for this LPM
 * @param[in] max_rules      max. number of rules supported per policy
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
rfc_build_lpm_tree (lpm_itable_t *lpm_itable, rfc_tree_t *rfc_tree,
                    mem_addr_t tree_base_addr, uint32_t mem_size,
                    uint32_t max_rules)
{
    sdk_ret_t    ret;
    itable_t     *itable = &rfc_tree->itable;

    for (uint32_t i = 0; i < lpm_itable->num_intervals; i++) {
        if ((lpm_itable->tree_type == ITREE_TYPE_IPV4_SIP_ACL) ||
            (lpm_itable->tree_type == ITREE_TYPE_IPV6_SIP_ACL)) {
            lpm_itable->nodes[i].ipaddr = itable->nodes[i].ipaddr;
        } else if ((lpm_itable->tree_type == ITREE_TYPE_IPV4_DIP_ACL) ||
                   (lpm_itable->tree_type == ITREE_TYPE_IPV6_DIP_ACL)) {
                    lpm_itable->nodes[i].ipaddr = itable->nodes[i].ipaddr;
        } else if (lpm_itable->tree_type == ITREE_TYPE_PORT) {
            lpm_itable->nodes[i].port = itable->nodes[i].port;
        } else if (lpm_itable->tree_type == ITREE_TYPE_PROTO_PORT) {
            lpm_itable->nodes[i].key32 = itable->nodes[i].key32;
        }
        lpm_itable->nodes[i].data = itable->nodes[i].rfc.class_id;
    }
    ret = lpm_build_tree(lpm_itable, (lpm_itable->num_intervals > 0) ?
                                     lpm_itable->nodes[lpm_itable->num_intervals-1].data:0,
                         max_rules, tree_base_addr, mem_size);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to build RFC tree type %u, err : %u",
                      lpm_itable->tree_type, ret);
    }
    return ret;
}

/**
 * @brief        for given RFC context, build all the phase 0 LPM trees
 * @param[in] rfc_ctxt  RFC context carrying all the intermediate state
 * @param[in] rfc_tree_root_addr    address in memory to write the entire
 *                                  RFC policy
 * @param[in] mem_sz                size of the memory address reserved
 *                                  for this RFC policy
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
rfc_build_lpm_trees (rfc_ctxt_t *rfc_ctxt,
                     mem_addr_t rfc_tree_root_addr, uint32_t mem_sz)
{
    sdk_ret_t       ret;
    lpm_itable_t    itable;
    uint32_t        max_intervals, tree_size, nodes;
    mem_addr_t      tree_base_addr;

    max_intervals = MAX(rfc_ctxt->sip_tree.num_intervals,
                        rfc_ctxt->dip_tree.num_intervals);
    max_intervals = MAX(rfc_ctxt->port_tree.num_intervals,
                        max_intervals);
    max_intervals = MAX(rfc_ctxt->proto_port_tree.num_intervals,
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

    /**< build LPM tree for th SIP portion of the rules */
    itable.num_intervals = rfc_ctxt->sip_tree.num_intervals;
    if (rfc_ctxt->policy->af == IP_AF_IPV4) {
        itable.tree_type = ITREE_TYPE_IPV4_SIP_ACL;
        nodes = SACL_IPV4_SIP_TREE_MAX_NODES >> 1;
    } else {
        itable.tree_type = ITREE_TYPE_IPV6_SIP_ACL;
        nodes = SACL_IPV6_SIP_TREE_MAX_NODES >> 1;
    }

    tree_base_addr_size(rfc_tree_root_addr, itable.tree_type,
                        &tree_base_addr, &tree_size);
    ret = rfc_build_lpm_tree(&itable, &rfc_ctxt->sip_tree,
                             tree_base_addr, tree_size, nodes);
    if (ret != SDK_RET_OK) {
        goto cleanup;
    }

    /**< build LPM tree for th DIP portion of the rules */
    itable.num_intervals = rfc_ctxt->dip_tree.num_intervals;
    if (rfc_ctxt->policy->af == IP_AF_IPV4) {
        itable.tree_type = ITREE_TYPE_IPV4_DIP_ACL;
        nodes = SACL_IPV4_DIP_TREE_MAX_NODES >> 1;
    } else {
        itable.tree_type = ITREE_TYPE_IPV6_DIP_ACL;
        nodes = SACL_IPV6_DIP_TREE_MAX_NODES >> 1;
    }
    tree_base_addr_size(rfc_tree_root_addr, itable.tree_type,
                        &tree_base_addr, &tree_size);
    ret = rfc_build_lpm_tree(&itable, &rfc_ctxt->dip_tree,
                             tree_base_addr, tree_size, nodes);
    if (ret != SDK_RET_OK) {
        goto cleanup;
    }

    /**< build LPM tree for the port match portion of the rules */
    itable.tree_type = ITREE_TYPE_PORT;
    itable.num_intervals = rfc_ctxt->port_tree.num_intervals;
    tree_base_addr_size(rfc_tree_root_addr, itable.tree_type,
                        &tree_base_addr, &tree_size);
    ret = rfc_build_lpm_tree(&itable, &rfc_ctxt->port_tree,
                             tree_base_addr, tree_size,
                             SACL_SPORT_TREE_MAX_NODES >> 1);
    if (ret != SDK_RET_OK) {
        goto cleanup;
    }

    /**< build LPM tree for the (protocol, port) match portion of the rules */
    itable.tree_type = ITREE_TYPE_PROTO_PORT;
    itable.num_intervals = rfc_ctxt->proto_port_tree.num_intervals;
    tree_base_addr_size(rfc_tree_root_addr, itable.tree_type,
                        &tree_base_addr, &tree_size);
    ret = rfc_build_lpm_tree(&itable, &rfc_ctxt->proto_port_tree,
                             tree_base_addr, tree_size,
                             SACL_PROTO_DPORT_TREE_MAX_NODES >> 1);

cleanup:

    if (itable.nodes) {
        free(itable.nodes);
    }
    return ret;
}

}    // namespace rfc
