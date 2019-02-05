/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    rfc_tree.cc
 *
 * @brief   RFC tree related helper APIs
 */

#include "nic/apollo/rfc/rfc.hpp"
#include "nic/apollo/rfc/rfc_tree.hpp"

namespace rfc {

void
itable_add_address_inodes (uint32_t rule, inode_t *addr_inode, ip_prefix_t *pfx)
{
    /**< fill the itree node corresponding to start of the IP prefix */
    ip_prefix_ip_low(pfx, &addr_inode->ipaddr);
    addr_inode->rfc.class_id = 0;    /**< class id will be computed later on */
    addr_inode->rfc.rule_no = rule;
    addr_inode->rfc.start = TRUE;
    addr_inode->rfc.pad = 0;

    /**< fill the itree node corresponding to end of the IP prefix */
    addr_inode++;
    ip_prefix_ip_next(pfx, &addr_inode->ipaddr);
    addr_inode->rfc.class_id = 0;    /**< class id will be computed later on */
    addr_inode->rfc.rule_no = rule;
    addr_inode->rfc.start = FALSE;
    addr_inode->rfc.pad = 0;
}

void
itable_add_port_inodes (uint32_t rule, inode_t *port_inode,
                        port_range_t *port_range)
{
    port_inode->port = port_range->port_lo;
    port_inode->rfc.class_id = 0;
    port_inode->rfc.rule_no = rule;
    port_inode->rfc.start = TRUE;
    port_inode->rfc.pad = 0;

    port_inode++;
    port_inode->port = port_range->port_hi;
    port_inode->rfc.class_id = 0;
    port_inode->rfc.rule_no = rule;
    port_inode->rfc.start = FALSE;
    port_inode->rfc.pad = 0;
}

void
itable_add_proto_port_inodes (uint32_t rule, inode_t *proto_port_inode,
                              uint8_t ip_proto, port_range_t *port_range)
{
    proto_port_inode->key32 = (ip_proto << 16) | port_range->port_lo;
    proto_port_inode->rfc.class_id = 0;
    proto_port_inode->rfc.rule_no = rule;
    proto_port_inode->rfc.start = TRUE;
    proto_port_inode->rfc.pad = 0;

    proto_port_inode++;
    if (ip_proto) {
        proto_port_inode->key32 = (ip_proto << 16) | port_range->port_hi;
    } else {
        proto_port_inode->key32 = 0x00FFFFFF;
    }
    proto_port_inode->rfc.class_id = 0;
    proto_port_inode->rfc.rule_no = rule;
    proto_port_inode->rfc.start = FALSE;
    proto_port_inode->rfc.pad = 0;
}

void
rfc_trees_destroy (rfc_trees_t *rfc_trees)
{
    if (rfc_trees->pfx_tree.itable.nodes) {
        free(rfc_trees->pfx_tree.itable.nodes);
    }
    if (rfc_trees->port_tree.itable.nodes) {
        free(rfc_trees->port_tree.itable.nodes);
    }
    if (rfc_trees->proto_port_tree.itable.nodes) {
        free(rfc_trees->proto_port_tree.itable.nodes);
    }
    if (rfc_trees->cbm) {
        free(rfc_trees->cbm);
    }
}

sdk_ret_t
rfc_trees_init (rfc_trees_t *rfc_trees, policy_t *policy)
{
    uint8_t     *bits;
    uint32_t    num_nodes = (policy->num_rules << 1) + 1;

    rfc_trees->pfx_tree.itable.nodes =
        (inode_t *)malloc(sizeof(inode_t) * num_nodes);
    if (rfc_trees->pfx_tree.itable.nodes == NULL) {
        return sdk::SDK_RET_OOM;
    }

    rfc_trees->port_tree.itable.nodes =
        (inode_t *)malloc(sizeof(inode_t) * num_nodes);
    if (rfc_trees->port_tree.itable.nodes == NULL) {
        goto cleanup;
    }

    rfc_trees->proto_port_tree.itable.nodes =
        (inode_t *)malloc(sizeof(inode_t) * num_nodes);
    if (rfc_trees->proto_port_tree.itable.nodes == NULL) {
        goto cleanup;
    }
    rfc_trees->cbm_size = rte_bitmap_get_memory_footprint(policy->max_rules);
    bits = (uint8_t *)calloc(1, RTE_CACHE_LINE_ROUNDUP(rfc_trees->cbm_size));
    if (bits) {
        rfc_trees->cbm = rte_bitmap_init(policy->max_rules, bits,
                                         rfc_trees->cbm_size);
    }
    return SDK_RET_OK;

cleanup:

    rfc_trees_destroy(rfc_trees);
    return sdk::SDK_RET_OOM;
}

}    // namespace rfc
