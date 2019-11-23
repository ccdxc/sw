/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    lpm.hpp
 *
 * @brief   LPM library
 */

#if !defined (__LPM_HPP__)
#define __LPM_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/sdk/include/sdk/ip.hpp"

/**< LPM  interval tree type */
typedef enum itree_type_e {
    ITREE_TYPE_NONE,            /**< invalid tree type */
    ITREE_TYPE_IPV4,            /**< IPv4 route interval tree */
    ITREE_TYPE_IPV6,            /**< IPv6 route interval tree */
    ITREE_TYPE_IPV4_ACL,        /**< IPv4 SIP/DIP ACL interval tree */
    ITREE_TYPE_IPV6_ACL,        /**< IPv6 SIP/DIP ACL interval tree */
    ITREE_TYPE_PORT,            /**< tree with port as key */
    ITREE_TYPE_PROTO_PORT,      /**< tree with protocol + port as key */
    ITREE_TYPE_METER_V4,        /**< IPv4 meter LPM interval tree */
    ITREE_TYPE_METER_V6,        /**< IPv6 meter LPM interval tree */
    ITREE_TYPE_TAG_V4,          /**< IPv4 tag LPM interval tree */
    ITREE_TYPE_TAG_V6,          /**< IPv6 tag LPM interval tree */
    ITREE_TYPE_IPV4_DIP_ACL,    /**< IPv4 DIP ACL interval tree */
    ITREE_TYPE_IPV6_DIP_ACL,    /**< IPv6 DIP ACL interval tree */
    ITREE_TYPE_IPV4_SIP_ACL,    /**< IPv4 SIP ACL interval tree */
    ITREE_TYPE_IPV6_SIP_ACL,    /**< IPv6 SIP ACL interval tree */
    ITREE_TYPE_IPV4_PEER_ROUTE, /**< IPv4 Peer VNET interval tree */
    ITREE_TYPE_IPV6_PEER_ROUTE, /**< IPv6 Peer VNET interval tree */
    ITREE_TYPE_STAG,            /**< source tag interval tree */
    ITREE_TYPE_DTAG,            /**< destination tag interval tree */
} itree_type_t;

typedef struct route_s {
    ip_prefix_t    prefix;    /**< IP prefix */
    uint32_t       nhid;      /**< nexthop index */
    uint32_t       prio;      /**< priority of the route, numerically lower
                                   value is higher priority) */
} route_t;

typedef struct route_table_s {
    uint8_t         af;            /**< address family */
    uint8_t         priority_en;   /**< Priority Routing knob */
    uint32_t        default_nhid;  /**< NH Id for the default route */
    uint32_t        max_routes;    /**< max size of table */
    uint32_t        num_routes;    /**< number of routes */
    route_t         routes[0];     /**< route list */
} route_table_t;

/**< lpm interval tree node */
typedef struct lpm_inode_s {
    union {
        ip_addr_t    ipaddr;
        uint16_t     port;
        uint32_t     key32;
    };
    uint32_t         data;
} lpm_inode_t;

/**< lpm interval node table */
typedef struct lpm_itable_s {
    itree_type_t    tree_type;      /** type of the interval tree */
    uint32_t        num_intervals;  /**< number of entries in interval table */
    lpm_inode_t     *nodes;         /**< interval table nodes */
} lpm_itable_t;

/**
 * @brief    build interval tree based LPM tree at the given memory address
 *           based on the given route table
 * @param[in] route_table      pointer to the route configuration
 * @param[in] type             type of LPM
 * @param[in] lpm_tree_root    pointer to the memory address at which tree
 *                             should be built
 * @param[in] lpm_mem_size     LPM memory block size provided (for error
 *                             detection)
 * @return    SDK_RET_OK on success, failure status code on error
 *
 * NOTE: route_table will be modified internally as the library does
 *       in-place sorting on the given routing table
 */
sdk_ret_t lpm_tree_create(route_table_t *route_table,
                          itree_type_t type,
                          mem_addr_t lpm_tree_root_addr,
                          uint32_t lpm_mem_size);

/**
 * @brief    build interval tree based LPM tree at the given memory address
 *           based on the given interval table
 * @param[in] itable                pointer to the interval table
 * @param[in] default_nh            default nexthop id
 * @param[in] max_routes            max routes supported per route table
 * @param[in] lpm_tree_root_addr    pointer to the memory address at which tree
 *                                  should be built
 * @param[in] lpm_mem_size          LPM memory block size provided (for error
 *                                  detection)
 * @return    SDK_RET_OK on success, failure status code on error
 *
 * NOTE: route_table will be modified internally as the library does
 *       in-place sorting on the given routing table
 */
sdk_ret_t lpm_build_tree(lpm_itable_t *itable, uint32_t default_nh,
                         uint32_t max_routes, mem_addr_t lpm_tree_root_addr,
                         uint32_t lpm_mem_size);

#endif    /** __LPM_HPP__ */
