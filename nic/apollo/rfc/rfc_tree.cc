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
rfc_ctxt_destroy (rfc_ctxt_t *rfc_ctxt)
{
    if (rfc_ctxt->pfx_tree.itable.nodes) {
        free(rfc_ctxt->pfx_tree.itable.nodes);
    }
    if (rfc_ctxt->port_tree.itable.nodes) {
        free(rfc_ctxt->port_tree.itable.nodes);
    }
    if (rfc_ctxt->proto_port_tree.itable.nodes) {
        free(rfc_ctxt->proto_port_tree.itable.nodes);
    }
    if (rfc_ctxt->cbm) {
        free(rfc_ctxt->cbm);
    }
}

sdk_ret_t
rfc_ctxt_init (rfc_ctxt_t *rfc_ctxt, policy_t *policy)
{
    uint8_t     *bits;
    uint32_t    num_nodes = (policy->num_rules << 1) + 1;

    memset(rfc_ctxt, 0, sizeof(rfc_ctxt_t));
    rfc_ctxt->pfx_tree.itable.nodes =
        (inode_t *)malloc(sizeof(inode_t) * num_nodes);
    if (rfc_ctxt->pfx_tree.itable.nodes == NULL) {
        return sdk::SDK_RET_OOM;
    }
    new (&rfc_ctxt->pfx_tree.rfc_table.cbm_map) cbm_map_t();

    rfc_ctxt->port_tree.itable.nodes =
        (inode_t *)malloc(sizeof(inode_t) * num_nodes);
    if (rfc_ctxt->port_tree.itable.nodes == NULL) {
        goto cleanup;
    }
    new (&rfc_ctxt->port_tree.rfc_table.cbm_map) cbm_map_t();

    rfc_ctxt->proto_port_tree.itable.nodes =
        (inode_t *)malloc(sizeof(inode_t) * num_nodes);
    if (rfc_ctxt->proto_port_tree.itable.nodes == NULL) {
        goto cleanup;
    }
    new (&rfc_ctxt->proto_port_tree.rfc_table.cbm_map) cbm_map_t();

    new (&rfc_ctxt->p1_table.cbm_map) cbm_map_t();
    new (&rfc_ctxt->p2_table.cbm_map) cbm_map_t();

    rfc_ctxt->cbm_size = rte_bitmap_get_memory_footprint(policy->max_rules);
    bits = (uint8_t *)calloc(1, RTE_CACHE_LINE_ROUNDUP(rfc_ctxt->cbm_size));
    if (bits) {
        rfc_ctxt->cbm = rte_bitmap_init(policy->max_rules, bits,
                                         rfc_ctxt->cbm_size);
    }
    return SDK_RET_OK;

cleanup:

    rfc_ctxt_destroy(rfc_ctxt);
    return sdk::SDK_RET_OOM;
}

}    // namespace rfc
