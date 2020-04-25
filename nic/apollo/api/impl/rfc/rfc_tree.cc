/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    rfc_tree.cc
 *
 * @brief   RFC tree related helper APIs
 */

#include "include/sdk/platform.hpp"
#include "nic/apollo/api/impl/rfc/rfc.hpp"
#include "nic/apollo/api/impl/rfc/rfc_tree.hpp"
#include "nic/apollo/api/impl/rfc/rfc_impl.hpp"

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
    tag_inode->key32 = tag + 1;
    tag_inode->rfc.class_id = 0;    // class id will be computed later on
    tag_inode->rfc.rule_no = rule;
    tag_inode->rfc.start = FALSE;
    tag_inode->rfc.pad = 0;
}

void
itable_update_l4_any (rule_l4_match_t *l4Match)
{
    l4Match->dport_range.port_lo = 0;
    l4Match->dport_range.port_hi = 65535;
    l4Match->sport_range.port_lo = 0;
    l4Match->sport_range.port_hi = 65535;
}

void
itable_update_icmp_type_code (rule_l4_match_t *l4Match)
{
    if (l4Match->type_match_type == MATCH_SPECIFIC) {
        if (l4Match->code_match_type == MATCH_SPECIFIC) {
            l4Match->dport_range.port_lo = ((l4Match->icmp_type << 8) |
                                             l4Match->icmp_code);
            l4Match->dport_range.port_hi = l4Match->dport_range.port_lo;
        } else {
            l4Match->dport_range.port_lo = ((l4Match->icmp_type << 8));
            l4Match->dport_range.port_hi = ((l4Match->icmp_type << 8) |
                                             255);
        }
    } else {
        l4Match->dport_range.port_lo = 0;
        l4Match->dport_range.port_hi = 65535;
    }

    l4Match->sport_range.port_lo = 0;
    l4Match->sport_range.port_hi = 65535;
}

inode_t *
itable_add_port_inodes (uint32_t rule, inode_t *port_inode,
                        rule_l4_match_t *l4Match)
{
    port_inode->port = l4Match->sport_range.port_lo;
    port_inode->rfc.class_id = 0;
    port_inode->rfc.rule_no = rule;
    port_inode->rfc.start = TRUE;
    port_inode->rfc.pad = 0;
    port_inode++;

    if (l4Match->sport_range.port_hi != 65535) {
        port_inode->port = l4Match->sport_range.port_hi + 1;
        port_inode->rfc.class_id = 0;
        port_inode->rfc.rule_no = rule;
        port_inode->rfc.start = FALSE;
        port_inode->rfc.pad = 0;
        port_inode++;
    }

    return port_inode;
}

void
itable_add_proto_port_inodes (uint32_t rule, inode_t *proto_port_inode,
                              rule_l3_match_t *l3Match,
                              rule_l4_match_t *l4Match)
{
    if (l3Match->proto_match_type == MATCH_SPECIFIC) {
        proto_port_inode->key32 = (l3Match->ip_proto << 24) |
                                   l4Match->dport_range.port_lo;
    } else {
        proto_port_inode->key32 = 0;
    }

    proto_port_inode->rfc.class_id = 0;
    proto_port_inode->rfc.rule_no = rule;
    proto_port_inode->rfc.start = TRUE;
    proto_port_inode->rfc.pad = 0;

    proto_port_inode++;
    proto_port_inode->key32 = (l3Match->ip_proto << 24) |
                              (l4Match->dport_range.port_hi + 1);
    proto_port_inode->rfc.class_id = 0;
    proto_port_inode->rfc.rule_no = rule;
    proto_port_inode->rfc.start = FALSE;
    proto_port_inode->rfc.pad = 0;
}

void
rfc_table_destroy (rfc_table_t *rfc_table)
{
    rfc_table->cbm_map.clear();
    for (uint32_t i = 0; i < RFC_MAX_EQ_CLASSES; i++) {
        if (rfc_table->cbm_table[i].cbm) {
            free(rfc_table->cbm_table[i].cbm);
            rfc_table->cbm_table[i].cbm = NULL;
        }
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
rfc_ctxt_init (rfc_ctxt_t *rfc_ctxt, policy_params_t *policy_params)
{
    uint8_t *bits;
    policy_t *policy = &policy_params->policy;
    uint32_t num_nodes = (policy->num_rules << 1) + 1;

    memset(rfc_ctxt, 0, sizeof(rfc_ctxt_t));
    rfc_ctxt->policy = policy;

    // setup memory for SIP LPM tree
    rfc_ctxt->sip_tree.type = RFC_TREE_TYPE_SIP;
    rfc_ctxt->sip_tree.itable.nodes =
        (inode_t *)calloc(1, sizeof(inode_t) * num_nodes);
    if (rfc_ctxt->sip_tree.itable.nodes == NULL) {
        return sdk::SDK_RET_OOM;
    }
    new (&rfc_ctxt->sip_tree.rfc_table.cbm_map) cbm_map_t();
    rfc_ctxt->sip_tree.rfc_table.max_classes =
        (policy->af == IP_AF_IPV4) ? sacl_sip_v4_tree_max_classes() :
                                     sacl_sip_v6_tree_max_classes();

    // setup memory for DIP LPM tree
    rfc_ctxt->dip_tree.type = RFC_TREE_TYPE_DIP;
    rfc_ctxt->dip_tree.itable.nodes =
        (inode_t *)calloc(1, sizeof(inode_t) * num_nodes);
    if (rfc_ctxt->dip_tree.itable.nodes == NULL) {
        return sdk::SDK_RET_OOM;
    }
    new (&rfc_ctxt->dip_tree.rfc_table.cbm_map) cbm_map_t();
    rfc_ctxt->dip_tree.rfc_table.max_classes =
            (policy->af == IP_AF_IPV4) ? sacl_dip_v4_tree_max_classes() :
                                         sacl_dip_v6_tree_max_classes();

    // setup memory for stag "tree"
    rfc_ctxt->stag_tree.type = RFC_TREE_TYPE_STAG;
    rfc_ctxt->stag_tree.itable.nodes =
        (inode_t *)calloc(1, sizeof(inode_t) * num_nodes);
    if (rfc_ctxt->stag_tree.itable.nodes == NULL) {
        return sdk::SDK_RET_OOM;
    }
    new (&rfc_ctxt->stag_tree.rfc_table.cbm_map) cbm_map_t();
    rfc_ctxt->stag_tree.rfc_table.max_classes = sacl_stag_tree_max_classes();

    // setup memory for dtag "tree"
    rfc_ctxt->dtag_tree.type = RFC_TREE_TYPE_DTAG;
    rfc_ctxt->dtag_tree.itable.nodes =
        (inode_t *)calloc(1, sizeof(inode_t) * num_nodes);
    if (rfc_ctxt->dtag_tree.itable.nodes == NULL) {
        return sdk::SDK_RET_OOM;
    }
    new (&rfc_ctxt->dtag_tree.rfc_table.cbm_map) cbm_map_t();
    rfc_ctxt->dtag_tree.rfc_table.max_classes = sacl_dtag_tree_max_classes();

    // setup memory for sport LPM tree
    rfc_ctxt->port_tree.type = RFC_TREE_TYPE_PORT;
    rfc_ctxt->port_tree.itable.nodes =
        (inode_t *)calloc(1, sizeof(inode_t) * num_nodes);
    if (rfc_ctxt->port_tree.itable.nodes == NULL) {
        goto cleanup;
    }
    new (&rfc_ctxt->port_tree.rfc_table.cbm_map) cbm_map_t();
    rfc_ctxt->port_tree.rfc_table.max_classes =
        sacl_sport_tree_max_classes();

    // setup memory for protocol + dport LPM tree
    rfc_ctxt->proto_port_tree.type = RFC_TREE_TYPE_PROTO_PORT;
    rfc_ctxt->proto_port_tree.itable.nodes =
        (inode_t *)calloc(1, sizeof(inode_t) * num_nodes);
    if (rfc_ctxt->proto_port_tree.itable.nodes == NULL) {
        goto cleanup;
    }
    new (&rfc_ctxt->proto_port_tree.rfc_table.cbm_map) cbm_map_t();
    rfc_ctxt->proto_port_tree.rfc_table.max_classes =
        sacl_proto_dport_tree_max_classes();

    // setup P1 table
    new (&rfc_ctxt->p1_table.cbm_map) cbm_map_t();
    rfc_ctxt->p1_table.max_classes = sacl_p1_max_classes();

    // setup P2 table
    new (&rfc_ctxt->p2_table.cbm_map) cbm_map_t();
    rfc_ctxt->p2_table.max_classes = sacl_p2_max_classes();

    // setup the scratch pad memory
    rfc_ctxt->cbm_size =
        RTE_CACHE_LINE_ROUNDUP(rte_bitmap_get_memory_footprint(policy->max_rules));
    posix_memalign((void **)&bits, CACHE_LINE_SIZE, rfc_ctxt->cbm_size);
    if (bits) {
        rfc_ctxt->cbm = rte_bitmap_init(policy->max_rules, bits,
                                        rfc_ctxt->cbm_size);
    }
    rfc_ctxt->base_addr = policy_params->rfc_tree_root_addr;
    rfc_ctxt->mem_size = policy_params->rfc_mem_size;
    rfc_ctxt->tag2class_cb = policy_params->tag2class_cb;
    rfc_ctxt->tag2class_cb_ctxt = policy_params->tag2class_cb_ctxt;
    return SDK_RET_OK;

cleanup:

    rfc_ctxt_destroy(rfc_ctxt);
    return sdk::SDK_RET_OOM;
}

}    // namespace rfc
