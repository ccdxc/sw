/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    rfc_tree.cc
 *
 * @brief   pipeline specific APIs for RFC implementation
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/impl/rfc/rfc_impl.hpp"
#include "nic/apollo/api/impl/rfc/rfc_priv.hpp"
#include "nic/apollo/api/impl/apulu/rfc/rfc_utils.hpp"
#include "nic/apollo/p4/include/apulu_sacl_defines.h"

namespace rfc {

uint16_t
sacl_sip_v4_tree_max_classes (void)
{
    return SACL_IPV4_SIP_TREE_MAX_CLASSES;
}

uint16_t
sacl_dip_v4_tree_max_classes (void)
{
    return SACL_IPV4_DIP_TREE_MAX_CLASSES;
}

uint16_t
sacl_sip_v6_tree_max_classes (void)
{
    return SACL_IPV6_SIP_TREE_MAX_CLASSES;
}

uint16_t
sacl_dip_v6_tree_max_classes (void)
{
    return SACL_IPV6_DIP_TREE_MAX_CLASSES;
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
    return SACL_TAG_TREE_MAX_CLASSES;
}

uint16_t
sacl_dtag_tree_max_classes (void)
{
    return SACL_TAG_TREE_MAX_CLASSES;
}

uint16_t
sacl_p1_max_classes (void)
{
    return SACL_P1_MAX_CLASSES;
}

uint16_t
sacl_p2_max_classes (void)
{
    return SACL_P2_MAX_CLASSES;
}

typedef bool (*rfc_p0_tree_inode_eq_cb_t)(inode_t *inode1, inode_t *inode2);
typedef uint16_t (*rfc_compute_class_id_cb_t)(rfc_ctxt_t *rfc_ctxt,
                                              rfc_table_t *rfc_table,
                                              rte_bitmap *cbm,
                                              uint32_t cbm_size,
                                              void *cb_ctxt);

/**
* @brief    walk the policy rules and build all phase 0 RFC interval tables
* @param[in]    rfc_ctxt    pointer to RFC trees
*               intervals and class ids
* @param[in]    num_nodes number of nodes in the itables
* @return    SDK_RET_OK on success, failure status code on error
*/
sdk_ret_t
rfc_build_itables (rfc_ctxt_t *rfc_ctxt)
{
    rule_t      *rule;
    uint32_t    rule_num = 0;
    policy_t    *policy = rfc_ctxt->policy;
    itable_t    *sip_itable = &rfc_ctxt->sip_tree.itable;
    itable_t    *dip_itable = &rfc_ctxt->dip_tree.itable;
    itable_t    *port_itable = &rfc_ctxt->port_tree.itable;
    itable_t    *proto_port_itable = &rfc_ctxt->proto_port_tree.itable;
    itable_t    *stag_itable = &rfc_ctxt->stag_tree.itable;
    itable_t    *dtag_itable = &rfc_ctxt->dtag_tree.itable;
    inode_t     *sip_inode, *dip_inode, *port_inode, *proto_port_inode;
    inode_t     *stag_inode, *dtag_inode;

    /** walk the policy and start building tables */
    sip_inode = &sip_itable->nodes[0];
    stag_inode = &stag_itable->nodes[0];
    dip_inode = &dip_itable->nodes[0];
    dtag_inode = &dtag_itable->nodes[0];
    port_inode = &port_itable->nodes[0];
    proto_port_inode = &proto_port_itable->nodes[0];
    for (rule_num = 0; rule_num < policy->num_rules; rule_num++) {
        rule = &policy->rules[rule_num];
        rfc_policy_rule_dump(policy, rule_num);

        // handle source IP match conditions
        if (rule->attrs.match.l3_match.src_match_type == IP_MATCH_PREFIX) {
            itable_add_address_inodes(rule_num, sip_inode,
                                      &rule->attrs.match.l3_match.src_ip_pfx);
            sip_inode += 2;
        } else if (rule->attrs.match.l3_match.src_match_type == IP_MATCH_RANGE) {
            itable_add_address_range_inodes(rule_num, sip_inode,
                                            &rule->attrs.match.l3_match.src_ip_range);
            sip_inode += 2;
        } else if (rule->attrs.match.l3_match.src_match_type == IP_MATCH_TAG) {
            itable_add_tag_inodes(rule_num, stag_inode,
                                  rule->attrs.match.l3_match.src_tag);
            stag_inode += 2;
        } else if (rule->attrs.match.l3_match.src_match_type == IP_MATCH_NONE) {
            rule->attrs.match.l3_match.src_ip_pfx.addr.af = policy->af;
            itable_add_address_inodes(rule_num, sip_inode,
                                      &rule->attrs.match.l3_match.src_ip_pfx);
            sip_inode += 2;
        }

        // handle destination IP match conditions
        if (rule->attrs.match.l3_match.dst_match_type == IP_MATCH_PREFIX) {
            itable_add_address_inodes(rule_num, dip_inode,
                                      &rule->attrs.match.l3_match.dst_ip_pfx);
            dip_inode += 2;
        } else if (rule->attrs.match.l3_match.dst_match_type == IP_MATCH_RANGE) {
            itable_add_address_range_inodes(rule_num, dip_inode,
                                            &rule->attrs.match.l3_match.dst_ip_range);
            dip_inode += 2;
        } else if (rule->attrs.match.l3_match.dst_match_type == IP_MATCH_TAG) {
            itable_add_tag_inodes(rule_num, dtag_inode,
                                  rule->attrs.match.l3_match.dst_tag);
            dtag_inode += 2;
        } else if (rule->attrs.match.l3_match.dst_match_type == IP_MATCH_NONE) {
            rule->attrs.match.l3_match.dst_ip_pfx.addr.af = policy->af;
            itable_add_address_inodes(rule_num, dip_inode,
                                      &rule->attrs.match.l3_match.dst_ip_pfx);
            dip_inode += 2;
        }

        if (rule->attrs.match.l3_match.proto_match_type != MATCH_SPECIFIC) {
            itable_update_l4_any(&rule->attrs.match.l4_match);
        } else if (rule->attrs.match.l3_match.ip_proto == IP_PROTO_ICMP) {
            itable_update_icmp_type_code(&rule->attrs.match.l4_match);
        } else if ((rule->attrs.match.l3_match.ip_proto != IP_PROTO_TCP) &&
                   (rule->attrs.match.l3_match.ip_proto != IP_PROTO_UDP)){
            itable_update_l4_any(&rule->attrs.match.l4_match);
        }

        // handle source port match condition
        port_inode = itable_add_port_inodes(rule_num, port_inode,
                                            &rule->attrs.match.l4_match);

        // handle protocol and destination port match condition
        itable_add_proto_port_inodes(rule_num, proto_port_inode,
                                     &rule->attrs.match.l3_match,
                                     &rule->attrs.match.l4_match);
        proto_port_inode += 2;
    }
    sip_itable->num_nodes = sip_inode - &sip_itable->nodes[0];
    stag_itable->num_nodes = stag_inode - &stag_itable->nodes[0];
    dip_itable->num_nodes = dip_inode - &dip_itable->nodes[0];
    dtag_itable->num_nodes = dtag_inode - &dtag_itable->nodes[0];
    port_itable->num_nodes = port_inode - &port_itable->nodes[0];
    proto_port_itable->num_nodes = proto_port_inode - &proto_port_itable->nodes[0];
    return SDK_RET_OK;
}

static inline bool
rfc_p0_tag_tree_inode_eq_cb (inode_t *inode1, inode_t *inode2) {
    if (inode1->key32 == inode2->key32) {
        return true;
    }
    return false;
}

static inline sdk_ret_t
rfc_compute_p0_itree_classes (rfc_ctxt_t *rfc_ctxt, rfc_tree_t *rfc_tree,
                              rfc_p0_tree_inode_eq_cb_t inode_eq_cb,
                              rfc_compute_class_id_cb_t rfc_compute_class_id_cb,
                              uint32_t max_tree_nodes)
{
    inode_t *inode;
    class_id_cb_ctxt_t cb_ctxt;
    uint32_t num_intervals = 0;
    itable_t *itable = &rfc_tree->itable;
    rfc_table_t *rfc_table = &rfc_tree->rfc_table;

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
        cb_ctxt.tree = rfc_tree;
        cb_ctxt.inode = inode;
        inode->rfc.class_id =
            rfc_compute_class_id_cb(rfc_ctxt, rfc_table, rfc_ctxt->cbm,
                                    rfc_ctxt->cbm_size, &cb_ctxt);
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
              rfc_p0_pfx_tree_inode_eq_cb, rfc_compute_class_id_cb,
              (rfc_ctxt->policy->af == IP_AF_IPV4) ?
                  SACL_IPV4_SIP_TREE_MAX_NODES : SACL_IPV6_SIP_TREE_MAX_NODES);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    ret = rfc_compute_p0_itree_classes(rfc_ctxt, &rfc_ctxt->stag_tree,
              rfc_p0_tag_tree_inode_eq_cb, rfc_compute_class_id_cb,
              SACL_TAG_TREE_MAX_CLASSES);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    ret = rfc_compute_p0_itree_classes(rfc_ctxt, &rfc_ctxt->dip_tree,
              rfc_p0_pfx_tree_inode_eq_cb, rfc_compute_class_id_cb,
              (rfc_ctxt->policy->af == IP_AF_IPV4) ?
                  SACL_IPV4_DIP_TREE_MAX_NODES : SACL_IPV6_DIP_TREE_MAX_NODES);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    ret = rfc_compute_p0_itree_classes(rfc_ctxt, &rfc_ctxt->dtag_tree,
              rfc_p0_tag_tree_inode_eq_cb, rfc_compute_class_id_cb,
              SACL_TAG_TREE_MAX_CLASSES);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    ret = rfc_compute_p0_itree_classes(rfc_ctxt, &rfc_ctxt->port_tree,
                                       rfc_p0_port_tree_inode_eq_cb,
                                       rfc_compute_class_id_cb,
                                       SACL_SPORT_TREE_MAX_NODES);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    ret = rfc_compute_p0_itree_classes(rfc_ctxt, &rfc_ctxt->proto_port_tree,
                                       rfc_p0_proto_port_tree_inode_eq_cb,
                                       rfc_compute_class_id_cb,
                                       SACL_PROTO_DPORT_TREE_MAX_NODES);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    return SDK_RET_OK;
}

}    // namespace rfc
