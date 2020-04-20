/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    rfc_impl.cc
 *
 * @brief   pipeline specific APIs for RFC implementation
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/impl/rfc/rfc.hpp"
#include "nic/apollo/api/impl/rfc/rfc_tree.hpp"
#include "nic/apollo/api/impl/rfc/rfc_priv.hpp"
#include "nic/apollo/api/impl/rfc/rte_bitmap_utils.hpp"
#include "nic/apollo/api/impl/apollo/rfc/rfc_utils.hpp"
#include "nic/apollo/p4/include/sacl_defines.h"

namespace rfc {

typedef bool (rfc_p0_tree_inode_eq_cb_t)(inode_t *inode1, inode_t *inode2);

uint16_t
sacl_sip_v4_tree_max_classes (void)
{
    return SACL_IPV4_TREE_MAX_CLASSES;
}

uint16_t
sacl_dip_v4_tree_max_classes (void)
{
    return 0;
}

uint16_t
sacl_sip_v6_tree_max_classes (void)
{
    return SACL_IPV6_TREE_MAX_CLASSES;
}

uint16_t
sacl_dip_v6_tree_max_classes (void)
{
    return 0;
}

uint16_t
sacl_sport_tree_max_classes (void)
{
    return SACL_SPORT_TREE_MAX_CLASSES;
}

uint16_t
sacl_proto_dport_tree_max_classes (void)
{
    return SACL_PROTO_DPORT_TREE_MAX_CLASSES;
}

uint16_t
sacl_stag_tree_max_classes (void)
{
    return 0;
}

uint16_t
sacl_dtag_tree_max_classes (void)
{
    return 0;
}

uint16_t
sacl_p1_max_classes (void)
{
    return SACL_P1_MAX_CLASSES;
}

uint16_t
sacl_p2_max_classes (void)
{
    return 0;
}

sdk_ret_t
rfc_build_itables (rfc_ctxt_t *rfc_ctxt)
{
    rule_t      *rule;
    uint32_t    rule_num = 0;
    policy_t    *policy = rfc_ctxt->policy;
    itable_t    *addr_itable = &rfc_ctxt->sip_tree.itable;
    itable_t    *port_itable = &rfc_ctxt->port_tree.itable;
    itable_t    *proto_port_itable = &rfc_ctxt->proto_port_tree.itable;
    inode_t     *addr_inode, *port_inode, *proto_port_inode;

    /** walk the policy and start building tables */
    addr_inode = &addr_itable->nodes[0];
    port_inode = &port_itable->nodes[0];
    proto_port_inode = &proto_port_itable->nodes[0];
    for (rule_num = 0; rule_num < policy->num_rules; rule_num++) {
        rule = &policy->rules[rule_num];
        rfc_policy_rule_dump(policy, rule_num);
        if (ip_prefix_is_zero(&rule->attrs.match.l3_match.dst_ip_pfx)) {
            // ingress policy
            rule->attrs.match.l3_match.src_ip_pfx.addr.af = policy->af;
            itable_add_address_inodes(rule_num, addr_inode,
                                      &rule->attrs.match.l3_match.src_ip_pfx);
        } else {
            // egress policy
            rule->attrs.match.l3_match.dst_ip_pfx.addr.af = policy->af;
            itable_add_address_inodes(rule_num, addr_inode,
                                      &rule->attrs.match.l3_match.dst_ip_pfx);
        }
        port_inode = itable_add_port_inodes(rule_num, port_inode,
                                            &rule->attrs.match.l4_match);
        itable_add_proto_port_inodes(rule_num, proto_port_inode,
                                     &rule->attrs.match.l3_match,
                                     &rule->attrs.match.l4_match);
        addr_inode += 2;
        proto_port_inode += 2;
    }
    addr_itable->num_nodes = addr_inode - &addr_itable->nodes[0];
    port_itable->num_nodes = port_inode - &port_itable->nodes[0];
    proto_port_itable->num_nodes =
        proto_port_inode - &proto_port_itable->nodes[0];
    return SDK_RET_OK;
}

static inline sdk_ret_t
rfc_compute_p0_itree_classes (rfc_ctxt_t *rfc_ctxt, rfc_tree_t *rfc_tree,
                              rfc_p0_tree_inode_eq_cb_t inode_eq_cb,
                              uint32_t max_tree_nodes)
{
    uint32_t       num_intervals = 0;
    itable_t       *itable = &rfc_tree->itable;
    rfc_table_t    *rfc_table = &rfc_tree->rfc_table;
    inode_t        *inode;

    /**
     * walk over the interval table, compute class id and class bitmap for each
     * unique interval
     */
    rte_bitmap_reset(rfc_ctxt->cbm);
    for (uint32_t i = 0; i < itable->num_nodes; i++) {
        inode = &itable->nodes[i];
        if (inode->rfc.start) {
            rte_bitmap_set(rfc_ctxt->cbm, inode->rfc.rule_no);
        } else {
            rte_bitmap_clear(rfc_ctxt->cbm, inode->rfc.rule_no);
        }

        /**< skip next interval if key is same, note that we updated the bmap */
        if (((i + 1) < itable->num_nodes) &&
            inode_eq_cb(inode, &itable->nodes[i+1])) {
            continue;
        }
        inode->rfc.class_id =
            rfc_compute_class_id(rfc_ctxt, rfc_table,
                                 rfc_ctxt->cbm, rfc_ctxt->cbm_size);
        itable->nodes[num_intervals++] = *inode;
    }
    rfc_tree->num_intervals = num_intervals;

    if (num_intervals > max_tree_nodes) {
        PDS_TRACE_ERR("No. of interval nodes in itree %u exceeded "
                      "max supported nodes %u", num_intervals,
                      max_tree_nodes);
        return sdk::SDK_RET_NO_RESOURCE;
    }
    if (rfc_table->num_classes > rfc_table->max_classes) {
        PDS_TRACE_ERR("No. of equivalence classes in tree %u exceeded "
                      "max supported classes %u",
                      rfc_table->num_classes, rfc_table->max_classes);
        return sdk::SDK_RET_NO_RESOURCE;
    }
    return SDK_RET_OK;
}

sdk_ret_t
rfc_compute_p0_classes (rfc_ctxt_t *rfc_ctxt)
{
    sdk_ret_t   ret;

    ret = rfc_compute_p0_itree_classes(rfc_ctxt, &rfc_ctxt->sip_tree,
                                       rfc_p0_pfx_tree_inode_eq_cb,
                                       rfc_ctxt->policy->af == IP_AF_IPV4 ?
                                       SACL_IPV4_TREE_MAX_NODES:
                                       SACL_IPV6_TREE_MAX_NODES);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    ret = rfc_compute_p0_itree_classes(rfc_ctxt, &rfc_ctxt->port_tree,
                                       rfc_p0_port_tree_inode_eq_cb,
                                       SACL_SPORT_TREE_MAX_NODES);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    ret = rfc_compute_p0_itree_classes(rfc_ctxt, &rfc_ctxt->proto_port_tree,
                                       rfc_p0_proto_port_tree_inode_eq_cb,
                                       SACL_PROTO_DPORT_TREE_MAX_NODES);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    return SDK_RET_OK;
}

}    // namespace rfc
