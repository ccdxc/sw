/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    rfc.cc
 *
 * @brief   RFC library implementation
 */

#include <string>
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/rfc/rte_bitmap_utils.hpp"
#include "nic/apollo/lpm/lpm.hpp"
#include "nic/apollo/rfc/rfc.hpp"
#include "nic/apollo/rfc/rfc_tree.hpp"
#include "nic/apollo/rfc/rfc_utils.hpp"
#include "nic/apollo/p4/include/sacl_defines.h"
#include "gen/p4gen/apollo_rxdma/include/apollo_rxdma_p4pd.h"
#include "gen/p4gen/apollo_txdma/include/apollo_txdma_p4pd.h"

using std::string;

namespace rfc {

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
        "IP pfx " + string(ippfx2str(&rule->match.l3_match.ip_pfx)) + ", ";
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
    SDK_ASSERT(policy->af == IP_AF_IPV4);
    addr_inode = &addr_itable->nodes[0];
    port_inode = &port_itable->nodes[0];
    proto_port_inode = &proto_port_itable->nodes[0];
    for (rule_num = 0; rule_num < policy->num_rules; rule_num++) {
        rule = &policy->rules[rule_num];
        rfc_policy_rule_dump(policy, rule_num);
        itable_add_address_inodes(rule_num, addr_inode,
                                  &rule->match.l3_match.ip_pfx);
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

static inline sdk_ret_t
rfc_compute_p0_classes (rfc_ctxt_t *rfc_ctxt)
{
    uint32_t    num_intervals;
    itable_t    *addr_itable = &rfc_ctxt->pfx_tree.itable;
    itable_t    *port_itable = &rfc_ctxt->port_tree.itable;
    itable_t    *proto_port_itable = &rfc_ctxt->proto_port_tree.itable;
    inode_t     *inode;

    /**
     * walk over the address interval table, compute class id and class bitmap
     * for each unique interval
     */
    num_intervals = 0;
    rte_bitmap_reset(rfc_ctxt->cbm);
    for (uint32_t i = 0; i < addr_itable->num_nodes; i++) {
        inode = &addr_itable->nodes[i];
        if (inode->rfc.start) {
            rte_bitmap_set(rfc_ctxt->cbm, inode->rfc.rule_no);
        } else {
            rte_bitmap_clear(rfc_ctxt->cbm, inode->rfc.rule_no);
        }

        /**< skip next interval if key is same, note that we updated the bmap */
        if (((i + 1) < addr_itable->num_nodes) &&
            IPADDR_EQ(&inode->ipaddr,
                      &addr_itable->nodes[i+1].ipaddr)) {
            continue;
        }
        inode->rfc.class_id =
            rfc_compute_class_id(rfc_ctxt, &rfc_ctxt->pfx_tree.rfc_table,
                                 rfc_ctxt->cbm, rfc_ctxt->cbm_size);
        addr_itable->nodes[num_intervals++] = *inode;
    }
    rfc_ctxt->pfx_tree.num_intervals = num_intervals;
    PDS_TRACE_DEBUG("No. of interval nodes in prefix itree %u", num_intervals);
    PDS_TRACE_DEBUG("No. of equivalence classes in prefix tree %u",
                    rfc_ctxt->pfx_tree.rfc_table.num_classes);
    if (num_intervals > SACL_IPV4_TREE_MAX_NODES) {
        PDS_TRACE_ERR("No. of interval nodes in prefix itree %u exceeded "
                      "max supported nodes %u", num_intervals,
                      SACL_IPV4_TREE_MAX_NODES);
        return sdk::SDK_RET_NO_RESOURCE;
    }
    if (rfc_ctxt->pfx_tree.rfc_table.num_classes >
            rfc_ctxt->pfx_tree.rfc_table.max_classes) {
        PDS_TRACE_ERR("No. of equivalence classes in prefix tree %u exceeded "
                      "max supported classes %u",
                      rfc_ctxt->pfx_tree.rfc_table.num_classes,
                      rfc_ctxt->pfx_tree.rfc_table.max_classes);
        return sdk::SDK_RET_NO_RESOURCE;
    }

    num_intervals = 0;
    rte_bitmap_reset(rfc_ctxt->cbm);
    for (uint32_t i = 0; i < port_itable->num_nodes; i++) {
        inode = &port_itable->nodes[i];
        if (inode->rfc.start) {
            rte_bitmap_set(rfc_ctxt->cbm, inode->rfc.rule_no);
        } else {
            rte_bitmap_clear(rfc_ctxt->cbm, inode->rfc.rule_no);
        }

        /**< skip next interval if key is same, note that we updated the bmap */
        if (((i + 1) < port_itable->num_nodes) &&
            (inode->port == port_itable->nodes[i+1].port)) {
            continue;
        }
        inode->rfc.class_id =
            rfc_compute_class_id(rfc_ctxt, &rfc_ctxt->port_tree.rfc_table,
                                 rfc_ctxt->cbm, rfc_ctxt->cbm_size);
        port_itable->nodes[num_intervals++] = *inode;
    }
    rfc_ctxt->port_tree.num_intervals = num_intervals;
    PDS_TRACE_DEBUG("No. of interval nodes in port itree %u", num_intervals);
    PDS_TRACE_DEBUG("No. of equivalence classes in port tree %u",
                    rfc_ctxt->port_tree.rfc_table.num_classes);
    if (num_intervals > SACL_SPORT_TREE_MAX_NODES) {
        PDS_TRACE_ERR("No. of interval nodes in port itree %u exceeded "
                      "max supported nodes %u", num_intervals,
                      SACL_SPORT_TREE_MAX_NODES);
        return sdk::SDK_RET_NO_RESOURCE;
    }
    if (rfc_ctxt->port_tree.rfc_table.num_classes >
            rfc_ctxt->port_tree.rfc_table.max_classes) {
        PDS_TRACE_ERR("No. of equivalence classes in port tree %u exceeded "
                      "max supported classes %u",
                      rfc_ctxt->port_tree.rfc_table.num_classes,
                      rfc_ctxt->port_tree.rfc_table.max_classes);
        return sdk::SDK_RET_NO_RESOURCE;
    }

    num_intervals = 0;
    rte_bitmap_reset(rfc_ctxt->cbm);
    for (uint32_t i = 0; i < proto_port_itable->num_nodes; i++) {
        inode = &proto_port_itable->nodes[i];
        if (inode->rfc.start) {
            rte_bitmap_set(rfc_ctxt->cbm, inode->rfc.rule_no);
        } else {
            rte_bitmap_clear(rfc_ctxt->cbm, inode->rfc.rule_no);
        }

        /**< skip next interval if key is same, note that we updated the bmap */
        if (((i + 1) < proto_port_itable->num_nodes) &&
            (inode->key32 == proto_port_itable->nodes[i+1].key32)) {
            continue;
        }
        inode->rfc.class_id =
            rfc_compute_class_id(rfc_ctxt, &rfc_ctxt->proto_port_tree.rfc_table,
                                 rfc_ctxt->cbm, rfc_ctxt->cbm_size);
        proto_port_itable->nodes[num_intervals++] = *inode;
    }
    rfc_ctxt->proto_port_tree.num_intervals = num_intervals;
    PDS_TRACE_DEBUG("No. of interval nodes in (proto, port) itree %u",
                    num_intervals);
    PDS_TRACE_DEBUG("No. of equivalence classes in (proto, port) tree %u",
                    rfc_ctxt->proto_port_tree.rfc_table.num_classes);
    if (num_intervals > SACL_PROTO_DPORT_TREE_MAX_NODES) {
        PDS_TRACE_ERR("No. of interval nodes in (proto, port) itree %u "
                      "exceeded max supported nodes %u", num_intervals,
                      SACL_PROTO_DPORT_TREE_MAX_NODES);
        return sdk::SDK_RET_NO_RESOURCE;
    }
    if (rfc_ctxt->proto_port_tree.rfc_table.num_classes >
            rfc_ctxt->proto_port_tree.rfc_table.max_classes) {
        PDS_TRACE_ERR("No. of equivalence classes in (proto, port) tree %u "
                      "exceeded max supported classes %u",
                      rfc_ctxt->proto_port_tree.rfc_table.num_classes,
                      rfc_ctxt->proto_port_tree.rfc_table.max_classes);
        return sdk::SDK_RET_NO_RESOURCE;
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
    std::stringstream    cbm_ss;

    PDS_TRACE_DEBUG("Number of equivalence classes %u", rfc_table->num_classes);
    for (uint32_t i = 0; i < rfc_table->num_classes; i++) {
        rte_bitmap2str(rfc_table->cbm_table[i], cbm_ss);
        PDS_TRACE_DEBUG("class id %u, cbm %s", i, cbm_ss.str().c_str());
        cbm_ss.clear();
        cbm_ss.str("");
    }
}

static inline void
rfc_p0_eq_class_tables_dump (rfc_ctxt_t *rfc_ctxt)
{
    PDS_TRACE_DEBUG("RFC P0 prefix tree equivalence class table dump :");
    rfc_eq_class_table_dump(&rfc_ctxt->pfx_tree.rfc_table);
    PDS_TRACE_DEBUG("RFC P0 port tree equivalence class table dump :");
    rfc_eq_class_table_dump(&rfc_ctxt->port_tree.rfc_table);
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

    if (unlikely(policy->num_rules == 0)) {
        return sdk::SDK_RET_INVALID_ARG;
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
