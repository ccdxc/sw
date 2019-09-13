/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    rfc.cc
 *
 * @brief   RFC library implementation
 */

#include <string>
#include "nic/apollo/core/trace.hpp"
#include "rte_bitmap_utils.hpp"
#include "nic/apollo/api/impl/lpm/lpm.hpp"
#include "nic/apollo/rfc/rfc.hpp"
#include "rfc_tree.hpp"
#include "rfc_utils.hpp"
#include "nic/apollo/p4/include/sacl_defines.h"
#include "gen/p4gen/apollo_rxdma/include/apollo_rxdma_p4pd.h"
#include "gen/p4gen/apollo_txdma/include/apollo_txdma_p4pd.h"

using std::string;

namespace rfc {

typedef bool (rfc_p0_tree_inode_eq_cb_t)(inode_t *inode1, inode_t *inode2);

/**
 * @brief    dump a given policy rule
 * @param[in]    policy    policy table
 * @param[in]    rule_num  rule number of the rule to dump
 */
static inline void
rfc_policy_rule_dump (policy_t *policy, uint32_t rule_num)
{
    rule_t    *rule = &policy->rules[rule_num];
    string    rule_str = "";

    if (policy->direction == RULE_DIR_INGRESS) {
        rule_str += "ing, ";
    } else {
        rule_str += "egr, ";
    }
    if (policy->af == IP_AF_IPV4) {
        rule_str += "IPv4, ";
    } else {
        rule_str += "IPv6, ";
    }
    if (rule->stateful) {
        rule_str += "stateful : ";
    } else {
        rule_str += "stateless : ";
    }
    rule_str += "match = (proto " +
                    std::to_string(rule->match.l3_match.ip_proto) + ", ";
    rule_str+=
        "Src IP pfx " + string(ippfx2str(&rule->match.l3_match.src_ip_pfx)) + ", ";
    rule_str+=
        "Dst IP pfx " + string(ippfx2str(&rule->match.l3_match.dst_ip_pfx)) + ", ";
    if (rule->match.l3_match.ip_proto == IP_PROTO_ICMP) {
        rule_str += "ICMP type/code " +
                       std::to_string(rule->match.l4_match.icmp_type) +
                       std::to_string(rule->match.l4_match.icmp_code) + ", ";
    } else if ((rule->match.l3_match.ip_proto == IP_PROTO_UDP) ||
               (rule->match.l3_match.ip_proto == IP_PROTO_TCP)) {
        rule_str +=
            "sport range " +
            std::to_string(rule->match.l4_match.sport_range.port_lo) + "-" +
            std::to_string(rule->match.l4_match.sport_range.port_hi) + ", ";
        rule_str +=
            "dport range " +
            std::to_string(rule->match.l4_match.dport_range.port_lo) + "-" +
            std::to_string(rule->match.l4_match.dport_range.port_hi) + ") ";
    }
    if (policy->policy_type == POLICY_TYPE_FIREWALL) {
        rule_str += "action = ";
        if (rule->action_data.fw_action.action == SECURITY_RULE_ACTION_ALLOW) {
            rule_str += "allow";
        } else {
            rule_str += "unknown";
        }
    }
    PDS_TRACE_DEBUG_NO_HEADER("%s", rule_str.c_str());
}

/**
 * @brief    walk the policy rules and build all phase 0 RFC interval tables
 * @param[in]    rfc_ctxt    pointer to RFC trees
 *               intervals and class ids
 * @param[in]    num_nodes number of nodes in the itables
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
rfc_build_itables (rfc_ctxt_t *rfc_ctxt)
{
    rule_t      *rule;
    uint32_t    rule_num = 0;
    policy_t    *policy = rfc_ctxt->policy;
    itable_t    *addr_itable = &rfc_ctxt->pfx_tree.itable;
    itable_t    *port_itable = &rfc_ctxt->port_tree.itable;
    itable_t    *proto_port_itable = &rfc_ctxt->proto_port_tree.itable;
    inode_t     *addr_inode, *port_inode, *proto_port_inode;

    /** walk the policy and start building tables */
    //SDK_ASSERT(policy->af == IP_AF_IPV4);
    addr_inode = &addr_itable->nodes[0];
    port_inode = &port_itable->nodes[0];
    proto_port_inode = &proto_port_itable->nodes[0];
    for (rule_num = 0; rule_num < policy->num_rules; rule_num++) {
        rule = &policy->rules[rule_num];
        rfc_policy_rule_dump(policy, rule_num);
        if (policy->direction == RULE_DIR_INGRESS) {
            itable_add_address_inodes(rule_num, addr_inode,
                                      &rule->match.l3_match.src_ip_pfx);
        } else {
            itable_add_address_inodes(rule_num, addr_inode,
                                      &rule->match.l3_match.dst_ip_pfx);
        }
        itable_add_port_inodes(rule_num, port_inode,
                               &rule->match.l4_match.sport_range);
        itable_add_proto_port_inodes(rule_num, proto_port_inode,
                                     rule->match.l3_match.ip_proto,
                                     &rule->match.l4_match.dport_range);
        addr_inode += 2;
        port_inode += 2;
        proto_port_inode += 2;
    }
    addr_itable->num_nodes = port_itable->num_nodes =
        proto_port_itable->num_nodes = rule_num << 1;
    return SDK_RET_OK;
}

static inline bool
rfc_p0_pfx_tree_inode_eq_cb (inode_t *inode1, inode_t *inode2) {
    if (IPADDR_EQ(&inode1->ipaddr, &inode2->ipaddr)) {
        return true;
    }
    return false;
}

static inline bool
rfc_p0_port_tree_inode_eq_cb (inode_t *inode1, inode_t *inode2) {
    if (inode1->port == inode2->port) {
        return true;
    }
    return false;
}

static inline bool
rfc_p0_proto_port_tree_inode_eq_cb (inode_t *inode1, inode_t *inode2) {
    if (inode1->key32 == inode2->key32) {
        return true;
    }
    return false;
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

static inline void
rfc_itree_dump (rfc_tree_t *rfc_tree, itree_type_t tree_type)
{
    itable_t    *itable = &rfc_tree->itable;
    inode_t     *inode;

    PDS_TRACE_DEBUG("No. of interval nodes in itree %u",
                    rfc_tree->num_intervals);
    for (uint32_t i = 0; i < rfc_tree->num_intervals; i++) {
        inode = &itable->nodes[i];
        if ((tree_type == ITREE_TYPE_IPV4_ACL) ||
            (tree_type == ITREE_TYPE_IPV6_ACL)) {
            PDS_TRACE_DEBUG("inode %u, IP %s, classid %u, rule# %u, start %s",
                            i, ipaddr2str(&inode->ipaddr), inode->rfc.class_id,
                            inode->rfc.rule_no,
                            inode->rfc.start ? "true" : "false");
        } else if (tree_type == ITREE_TYPE_PORT) {
            PDS_TRACE_DEBUG("inode %u, port %u, classid %u, rule# %u, start %s",
                            i, inode->port, inode->rfc.class_id,
                            inode->rfc.rule_no,
                            inode->rfc.start ? "true" : "false");
        } else if (tree_type == ITREE_TYPE_PROTO_PORT) {
            PDS_TRACE_DEBUG("inode %u, proto %u, port %u, classid %u, "
                            "rule# %u, start %s", i,
                            (inode->key32 >> 16) & 0xFF,
                            inode->key32 & 0xFFFF, inode->rfc.class_id,
                            inode->rfc.rule_no,
                            inode->rfc.start ? "true" : "false");
        }
    }
}

static inline sdk_ret_t
rfc_compute_p0_classes (rfc_ctxt_t *rfc_ctxt)
{
    sdk_ret_t   ret;

    ret = rfc_compute_p0_itree_classes(rfc_ctxt, &rfc_ctxt->pfx_tree,
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

/**
 * @brief    dump the contents of given equivalence table
 * @param[in] rfc_ctxt    RFC context carrying all of the previous phases
 *                        information processed until now
 */
void
rfc_eq_class_table_dump (rfc_table_t *rfc_table)
{
    std::stringstream    a1ss, a2ss;

    PDS_TRACE_DEBUG("Number of equivalence classes %u", rfc_table->num_classes);
    for (uint32_t i = 0; i < rfc_table->num_classes; i++) {
        rte_bitmap2str(rfc_table->cbm_table[i], a1ss, a2ss);
        PDS_TRACE_DEBUG("class id %u, a1ss %s\na2ss %s", i,
                        a1ss.str().c_str(), a2ss.str().c_str());
        a1ss.clear();
        a1ss.str("");
        a2ss.clear();
        a2ss.str("");
    }
}

static inline void
rfc_p0_eq_class_tables_dump (rfc_ctxt_t *rfc_ctxt)
{
    PDS_TRACE_DEBUG("RFC P0 prefix interval tree dump :");
    rfc_itree_dump(&rfc_ctxt->pfx_tree,
                   (rfc_ctxt->policy->af == IP_AF_IPV4) ? ITREE_TYPE_IPV4_ACL:
                                                          ITREE_TYPE_IPV6_ACL);
    PDS_TRACE_DEBUG("RFC P0 prefix tree equivalence class table dump :");
    rfc_eq_class_table_dump(&rfc_ctxt->pfx_tree.rfc_table);

    PDS_TRACE_DEBUG("RFC P0 port interval tree dump :");
    rfc_itree_dump(&rfc_ctxt->port_tree, ITREE_TYPE_PORT);
    PDS_TRACE_DEBUG("RFC P0 port tree equivalence class table dump :");
    rfc_eq_class_table_dump(&rfc_ctxt->port_tree.rfc_table);

    PDS_TRACE_DEBUG("RFC P0 (proto, port) interval tree dump :");
    rfc_itree_dump(&rfc_ctxt->proto_port_tree, ITREE_TYPE_PROTO_PORT);
    PDS_TRACE_DEBUG("RFC P0 (proto, port) tree equivalence class table dump :");
    rfc_eq_class_table_dump(&rfc_ctxt->proto_port_tree.rfc_table);
}

/**
 * @brief    build interval tree based RFC LPM trees and index tables for
 *           subsequent RFC phases, starting at the given memory address
 * @param[in] policy           pointer to the policy
 * @param[in] rfc_tree_root    pointer to the memory address at which tree
 *                             should be built
 * @param[in] mem_size         memory block size provided (for error
 *                             detection)
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
rfc_policy_create (policy_t *policy, mem_addr_t rfc_tree_root_addr,
                   uint32_t mem_size)
{
    sdk_ret_t     ret;
    rfc_ctxt_t    rfc_ctxt;

    if (policy->num_rules > policy->max_rules) {
        return sdk::SDK_RET_NO_RESOURCE;
    }

    /**< allocate memory for all the RFC itree tables */
    ret = rfc_ctxt_init(&rfc_ctxt, policy, rfc_tree_root_addr, mem_size);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    /**< build all the interval trees with the given policy */
    rfc_build_itables(&rfc_ctxt);

    /**< sort intervals in all the trees */
    rfc_sort_itables(&rfc_ctxt);

    /**< compute equivalence classes for the intervals in the interval trees */
    rfc_compute_p0_classes(&rfc_ctxt);
    rfc_p0_eq_class_tables_dump(&rfc_ctxt);

    /**< build LPM trees for phase 0 of RFC */
    ret = rfc_build_lpm_trees(&rfc_ctxt, rfc_tree_root_addr, mem_size);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to build RFC LPM trees, err %u", ret);
        goto cleanup;
    }

    /**
     * build equivalence class index tables and result tables for subsequent
     * phases of RFC
     */
    rfc_build_eqtables(&rfc_ctxt);

cleanup:

    /**< free all the temporary state */
    rfc_ctxt_destroy(&rfc_ctxt);

    return ret;
}

}    // namespace rfc
