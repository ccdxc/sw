/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    rfc.cc
 *
 * @brief   RFC library implementation
 */

#include "nic/apollo/rfc/rfc.hpp"
#include "gen/p4gen/apollo_rxdma/include/apollo_rxdma_p4pd.h"
#include "gen/p4gen/apollo_txdma/include/apollo_txdma_p4pd.h"

namespace rfc {

typedef struct route_itree_node_data_s {
    uint32_t    nhid;    /**< nexthop id */
} __PACK__ route_itree_node_data_t;

typedef struct rfc_itree_node_data_s {
    uint16_t    class_id;      /**< RFC class id */
    uint16_t    rule_no:12;    /**< rule number */
    uint16_t    start:1;       /**< TRUE if this is start of the interval */
    uint16_t    pad:3;         /**< pad bits */
} __PACK__ rfc_itree_node_data_t;

/**< interval tree type */
typedef enum itree_type_e {
    ITREE_TYPE_NONE,              /**< invalid tree type */
    ITREE_TYPE_ROUTE_IPV4,        /**< IPv4 route interval tree */
    ITREE_TYPE_ROUTE_IPV6,        /**< IPv6 route interval tree */
    ITREE_TYPE_RFC_IPV4,          /**< tree with IPv4 address as key */
    ITREE_TYPE_RFC_IPV6,          /**< tree with IPv6 address as key */
    ITREE_TYPE_RFC_PORT,          /**< tree with port as key */
    ITREE_TYPE_RFC_PROTO_PORT,    /**< tree with protocol + port as key */
} itree_type_t;

/**< interval tree node */
typedef struct inode_s {
    union {
        ip_addr_t    ipaddr;    /**< IPv4/IPv6 address */
        uint16_t     port;      /**< src/dst port */
        uint32_t     key32;     /**< generic 32 bit key */
    } key;
    union {
        route_itree_node_data_t    route_data;    /**< route specific data */
        rfc_itree_node_data_t      rfc_data;      /**< RFC specific data */
    } data;
} __PACK__ inode_t;

/**< interval node table */
typedef struct itable_s {
    uint32_t    num_nodes;    /**< number of entries in interval table */
    inode_t     *nodes;       /**< interval table nodes */
} __PACK__ itable_t;

/**< RFC segment/interval trees used in phase 0 */
typedef struct rfc_itables_s {
    itable_t     addr_itable;
    itable_t     port_itable;
    itable_t     proto_port_itable;
} __PACK__ rfc_itables_t;

static inline void
itree_add_address_inodes (uint32_t rule, inode_t *addr_inode, ip_prefix_t *pfx)
{
    /**< fill the itree node corresponding to start of the IP prefix */
    ip_prefix_ip_low(pfx, &addr_inode->key.ipaddr);
    addr_inode->data.rfc_data.class_id = 0;    /**< class id will be computed later on */
    addr_inode->data.rfc_data.rule_no = rule;
    addr_inode->data.rfc_data.start = TRUE;
    addr_inode->data.rfc_data.pad = 0;

    /**< fill the itree node corresponding to end of the IP prefix */
    addr_inode++;
    ip_prefix_ip_next(pfx, &addr_inode->key.ipaddr);
    addr_inode->data.rfc_data.class_id = 0;    /**< class id will be computed later on */
    addr_inode->data.rfc_data.rule_no = rule;
    addr_inode->data.rfc_data.start = FALSE;
    addr_inode->data.rfc_data.pad = 0;
}

static inline void
itree_add_port_inodes (uint32_t rule, inode_t *port_inode,
                       port_range_t *port_range)
{
    port_inode->key.port = port_range->port_lo;
    port_inode->data.rfc_data.class_id = 0;
    port_inode->data.rfc_data.rule_no = rule;
    port_inode->data.rfc_data.start = TRUE;
    port_inode->data.rfc_data.pad = 0;

    port_inode++;
    port_inode->key.port = port_range->port_hi;
    port_inode->data.rfc_data.class_id = 0;
    port_inode->data.rfc_data.rule_no = rule;
    port_inode->data.rfc_data.start = FALSE;
    port_inode->data.rfc_data.pad = 0;
}

static inline void
itree_add_proto_port_inodes (uint32_t rule, inode_t *proto_port_inode,
                             uint8_t ip_proto, port_range_t *port_range)
{
    proto_port_inode->key.key32 = (ip_proto << 16) | port_range->port_lo;
    proto_port_inode->data.rfc_data.class_id = 0;
    proto_port_inode->data.rfc_data.rule_no = rule;
    proto_port_inode->data.rfc_data.start = TRUE;
    proto_port_inode->data.rfc_data.pad = 0;

    proto_port_inode++;
    if (ip_proto) {
        proto_port_inode->key.key32 = (ip_proto << 16) | port_range->port_hi;
    } else {
        proto_port_inode->key.key32 = 0x00FFFFFF;
    }
    proto_port_inode->data.rfc_data.class_id = 0;
    proto_port_inode->data.rfc_data.rule_no = rule;
    proto_port_inode->data.rfc_data.start = FALSE;
    proto_port_inode->data.rfc_data.pad = 0;
}

/**
 * @brief    walk the policy rules and build all phase 0 RFC interval trees
 * @param[in]    policy      policy table
 * @param[in]    rfc_tables  interval tables that will contain sorted
 *               intervals and class ids
 * @param[in]    num_nodes number of nodes in the itables
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
rfc_build_itrees (policy_t *policy, rfc_itables_t *rfc_itables,
                  uint32_t num_nodes)
{
    rule_t      *rule;
    itable_t    *addr_itable = &rfc_itables->addr_itable;
    itable_t    *port_itable = &rfc_itables->port_itable;
    itable_t    *proto_port_itable = &rfc_itables->proto_port_itable;
    inode_t     *addr_inode, *port_inode, *proto_port_inode;

    /** walk the policy and start building tables */
    SDK_ASSERT(policy->af == IP_AF_IPV4);
    addr_inode = &addr_itable->nodes[0];
    port_inode = &port_itable->nodes[0];
    proto_port_inode = &proto_port_itable->nodes[0];
    for (uint32_t rule_num = 0; rule_num < policy->num_rules; rule_num++) {
        rule = &policy->rules[rule_num];
        itree_add_address_inodes(rule_num, addr_inode, &rule->match.ip_pfx);
        itree_add_port_inodes(rule_num, port_inode,
                              &rule->match.l4_match.sport_range);
        itree_add_proto_port_inodes(rule_num, proto_port_inode,
                                    rule->match.ip_proto,
                                    &rule->match.l4_match.dport_range);
        addr_inode++;
        port_inode++;
        proto_port_inode++;
    }

    return SDK_RET_OK;
}

static inline void
rfc_itables_free (rfc_itables_t *rfc_itables)
{
    if (rfc_itables->addr_itable.nodes) {
        free(rfc_itables->addr_itable.nodes);
    }
    if (rfc_itables->port_itable.nodes) {
        free(rfc_itables->port_itable.nodes);
    }
    if (rfc_itables->proto_port_itable.nodes) {
        free(rfc_itables->proto_port_itable.nodes);
    }
}

static inline sdk_ret_t
rfc_itables_alloc (rfc_itables_t *rfc_itables, uint32_t num_nodes)
{
    rfc_itables->addr_itable.nodes =
        (inode_t *)malloc(sizeof(inode_t) * num_nodes);
    if (rfc_itables->addr_itable.nodes == NULL) {
        return sdk::SDK_RET_OOM;
    }

    rfc_itables->port_itable.nodes =
        (inode_t *)malloc(sizeof(inode_t) * num_nodes);
    if (rfc_itables->port_itable.nodes == NULL) {
        goto cleanup;
    }

    rfc_itables->proto_port_itable.nodes =
        (inode_t *)malloc(sizeof(inode_t) * num_nodes);
    if (rfc_itables->proto_port_itable.nodes == NULL) {
        goto cleanup;
    }

    return SDK_RET_OK;

cleanup:

    rfc_itables_free(rfc_itables);
    return sdk::SDK_RET_OOM;
}

/**
 * @brief    build interval tree based RFC LPM trees and index tables for
 *           subsequent RFC phases, starting at the given memory address
 * @param[in] policy           pointer to the policy
 * @param[in] rfc_tree_root    pointer to the memory address at which tree
 *                             should be built
 * @param[in] rfc_mem_size     RFC memory block size provided (for error
 *                             detection)
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
rfc_create (policy_t *policy, mem_addr_t rfc_tree_root_addr,
            uint32_t rfc_mem_size)
{
    sdk_ret_t        ret;
    uint32_t         num_nodes;
    rfc_itables_t    rfc_itables = { 0 };

    if (unlikely(policy->num_rules == 0)) {
        return sdk::SDK_RET_INVALID_ARG;
    }

    /**< allocate memory for all the RFC itree tables */
    num_nodes = (policy->num_rules << 1) + 1;
    ret = rfc_itables_alloc(&rfc_itables, num_nodes);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    /**< build all the interval trees with the given policy */
    ret = rfc_build_itrees(policy, &rfc_itables, num_nodes);

    if (rfc_itables.addr_itable.nodes) {
        free(rfc_itables.addr_itable.nodes);
    }
    if (rfc_itables.port_itable.nodes) {
        free(rfc_itables.port_itable.nodes);
    }
    if (rfc_itables.proto_port_itable.nodes) {
        free(rfc_itables.proto_port_itable.nodes);
    }
    return ret;
}

}    // namespace rfc
