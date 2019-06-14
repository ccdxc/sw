/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    rfc_tree.cc
 *
 * @brief   RFC tree related helper APIs
 */

#include "nic/apollo/rfc/rfc.hpp"
#include "nic/apollo/api/impl/artemis/rfc/rfc_tree.hpp"
#include "nic/apollo/p4/include/artemis_sacl_defines.h"

namespace rfc {

void
itable_add_address_inodes (uint32_t rule, inode_t *addr_inode, ip_prefix_t *pfx)
{
    // fill the itree node corresponding to start of the IP prefix
    ip_prefix_ip_low(pfx, &addr_inode->ipaddr);
    addr_inode->rfc.class_id = 0;    // class id will be computed later on
    addr_inode->rfc.rule_no = rule;
    addr_inode->rfc.start = TRUE;
    addr_inode->rfc.pad = 0;

    // fill the itree node corresponding to end of the IP prefix
    addr_inode++;
    ip_prefix_ip_next(pfx, &addr_inode->ipaddr);
    addr_inode->rfc.class_id = 0;    // class id will be computed later on
    addr_inode->rfc.rule_no = rule;
    addr_inode->rfc.start = FALSE;
    addr_inode->rfc.pad = 0;
}

void
itable_add_address_range_inodes (uint32_t rule, inode_t *addr_inode,
                                 ipvx_range_t *range)
{
    // fill the itree node corresponding to start of the IP prefix
    addr_inode->ipaddr.af = range->af;
    addr_inode->ipaddr.addr = range->ip_lo;
    addr_inode->rfc.class_id = 0;    // class id will be computed later on
    addr_inode->rfc.rule_no = rule;
    addr_inode->rfc.start = TRUE;
    addr_inode->rfc.pad = 0;

    // fill the itree node corresponding to end of the IP prefix
    addr_inode++;
    addr_inode->ipaddr.af = range->af;
    addr_inode->ipaddr.addr = range->ip_hi;
    addr_inode->rfc.class_id = 0;    // class id will be computed later on
    addr_inode->rfc.rule_no = rule;
    addr_inode->rfc.start = FALSE;
    addr_inode->rfc.pad = 0;
}

void
itable_add_tag_inodes (uint32_t rule, inode_t *tag_inode, uint32_t tag)
{
    tag_inode->key32 = tag;
    tag_inode->rfc.class_id = 0;
    tag_inode->rfc.rule_no = rule;
    tag_inode->rfc.start = TRUE;
    tag_inode->rfc.pad = 0;

    tag_inode++;
    tag_inode->key32 = tag;
    tag_inode->rfc.class_id = 0;    // class id will be computed later on
    tag_inode->rfc.rule_no = rule;
    tag_inode->rfc.start = FALSE;
    tag_inode->rfc.pad = 0;
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
    if (port_range->port_hi != 65535) {
        port_inode->port = port_range->port_hi + 1;
    } else {
        port_inode->port = 65535;
    }
    port_inode->rfc.class_id = 0;
    port_inode->rfc.rule_no = rule;
    port_inode->rfc.start = FALSE;
    port_inode->rfc.pad = 0;
}

void
itable_add_proto_port_inodes (uint32_t rule, inode_t *proto_port_inode,
                              uint8_t ip_proto, port_range_t *port_range)
{
    uint16_t   port_hi;

    proto_port_inode->key32 = (ip_proto << 16) | port_range->port_lo;
    proto_port_inode->rfc.class_id = 0;
    proto_port_inode->rfc.rule_no = rule;
    proto_port_inode->rfc.start = TRUE;
    proto_port_inode->rfc.pad = 0;

    proto_port_inode++;
    if (port_range->port_hi != 65535) {
        port_hi = port_range->port_hi + 1;
    } else {
        port_hi = port_range->port_hi;
    }
    if (ip_proto) {
        proto_port_inode->key32 = (ip_proto << 16) | port_hi;
    } else {
        proto_port_inode->key32 = 0x00FFFFFF;
    }
    proto_port_inode->rfc.class_id = 0;
    proto_port_inode->rfc.rule_no = rule;
    proto_port_inode->rfc.start = FALSE;
    proto_port_inode->rfc.pad = 0;
}

static inline void
rfc_table_destroy (rfc_table_t *rfc_table)
{
    rfc_table->cbm_map.clear();
    for (uint32_t i = 0; i < rfc_table->num_classes; i++) {
        free(rfc_table->cbm_table[i]);
    }
}

static inline void
rfc_tree_destroy (rfc_tree_t *rfc_tree)
{
    if (rfc_tree->itable.nodes) {
        free(rfc_tree->itable.nodes);
    }
    rfc_table_destroy(&rfc_tree->rfc_table);
}

void
rfc_ctxt_destroy (rfc_ctxt_t *rfc_ctxt)
{
    rfc_tree_destroy(&rfc_ctxt->sip_tree);
    rfc_tree_destroy(&rfc_ctxt->dip_tree);
    rfc_tree_destroy(&rfc_ctxt->port_tree);
    rfc_tree_destroy(&rfc_ctxt->proto_port_tree);
    rfc_tree_destroy(&rfc_ctxt->stag_tree);
    rfc_tree_destroy(&rfc_ctxt->dtag_tree);
    rfc_table_destroy(&rfc_ctxt->p1_table);
    rfc_table_destroy(&rfc_ctxt->p2_table);
    if (rfc_ctxt->cbm) {
        free(rfc_ctxt->cbm);
    }
}

sdk_ret_t
rfc_ctxt_init (rfc_ctxt_t *rfc_ctxt, policy_t *policy,
               mem_addr_t base_addr, uint32_t mem_size)
{
    uint8_t     *bits;
    uint32_t    num_nodes = (policy->num_rules << 1) + 1;

    memset(rfc_ctxt, 0, sizeof(rfc_ctxt_t));
    rfc_ctxt->policy = policy;

    // setup memory for SIP LPM tree
    rfc_ctxt->sip_tree.itable.nodes =
        (inode_t *)malloc(sizeof(inode_t) * num_nodes);
    if (rfc_ctxt->sip_tree.itable.nodes == NULL) {
        return sdk::SDK_RET_OOM;
    }
    new (&rfc_ctxt->sip_tree.rfc_table.cbm_map) cbm_map_t();
    rfc_ctxt->sip_tree.rfc_table.max_classes =
        (policy->af == IP_AF_IPV4) ? SACL_IPV4_SIP_TREE_MAX_CLASSES :
                                     SACL_IPV6_SIP_TREE_MAX_CLASSES;

    // setup memory for DIP LPM tree
    rfc_ctxt->dip_tree.itable.nodes =
        (inode_t *)malloc(sizeof(inode_t) * num_nodes);
    if (rfc_ctxt->dip_tree.itable.nodes == NULL) {
        return sdk::SDK_RET_OOM;
    }
    new (&rfc_ctxt->dip_tree.rfc_table.cbm_map) cbm_map_t();
    rfc_ctxt->dip_tree.rfc_table.max_classes =
            (policy->af == IP_AF_IPV4) ? SACL_IPV4_DIP_TREE_MAX_CLASSES :
                                         SACL_IPV6_DIP_TREE_MAX_CLASSES;

    // setup memory for sport LPM tree
    rfc_ctxt->port_tree.itable.nodes =
        (inode_t *)malloc(sizeof(inode_t) * num_nodes);
    if (rfc_ctxt->port_tree.itable.nodes == NULL) {
        goto cleanup;
    }
    new (&rfc_ctxt->port_tree.rfc_table.cbm_map) cbm_map_t();
    rfc_ctxt->port_tree.rfc_table.max_classes =
        SACL_SPORT_TREE_MAX_CLASSES;

    // setup memory for protocol + dport LPM tree
    rfc_ctxt->proto_port_tree.itable.nodes =
        (inode_t *)malloc(sizeof(inode_t) * num_nodes);
    if (rfc_ctxt->proto_port_tree.itable.nodes == NULL) {
        goto cleanup;
    }
    new (&rfc_ctxt->proto_port_tree.rfc_table.cbm_map) cbm_map_t();
    rfc_ctxt->proto_port_tree.rfc_table.max_classes =
        SACL_PROTO_DPORT_TREE_MAX_CLASSES;

    // setup memory for stag "tree"
    rfc_ctxt->stag_tree.itable.nodes =
        (inode_t *)malloc(sizeof(inode_t) * num_nodes);
    if (rfc_ctxt->stag_tree.itable.nodes == NULL) {
        return sdk::SDK_RET_OOM;
    }
    new (&rfc_ctxt->stag_tree.rfc_table.cbm_map) cbm_map_t();
    rfc_ctxt->stag_tree.rfc_table.max_classes = SACL_TAG_TREE_MAX_CLASSES;

    // setup memory for dtag "tree"
    rfc_ctxt->dtag_tree.itable.nodes =
        (inode_t *)malloc(sizeof(inode_t) * num_nodes);
    if (rfc_ctxt->dtag_tree.itable.nodes == NULL) {
        return sdk::SDK_RET_OOM;
    }
    new (&rfc_ctxt->dtag_tree.rfc_table.cbm_map) cbm_map_t();
    rfc_ctxt->dtag_tree.rfc_table.max_classes = SACL_TAG_TREE_MAX_CLASSES;

    // setup P1 table
    new (&rfc_ctxt->p1_table.cbm_map) cbm_map_t();
    rfc_ctxt->p1_table.max_classes = SACL_P1_MAX_CLASSES;

    // setup P2 table
    new (&rfc_ctxt->p2_table.cbm_map) cbm_map_t();
    rfc_ctxt->p2_table.max_classes = SACL_P2_MAX_CLASSES;

    // setup the scratch pad memory
    rfc_ctxt->cbm_size =
        RTE_CACHE_LINE_ROUNDUP(rte_bitmap_get_memory_footprint(policy->max_rules));
    posix_memalign((void **)&bits, CACHE_LINE_SIZE, rfc_ctxt->cbm_size);
    if (bits) {
        rfc_ctxt->cbm = rte_bitmap_init(policy->max_rules, bits,
                                        rfc_ctxt->cbm_size);
    }
    rfc_ctxt->base_addr = base_addr;
    rfc_ctxt->mem_size = mem_size;
    return SDK_RET_OK;

cleanup:

    rfc_ctxt_destroy(rfc_ctxt);
    return sdk::SDK_RET_OOM;
}

}    // namespace rfc
