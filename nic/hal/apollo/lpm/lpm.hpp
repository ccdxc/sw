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

typedef struct route_s {
    ip_prefix_t    prefix;    /**< IP prefix */
    uint32_t       nhid;      /**< nexthop index */
} route_t;

typedef struct route_table_s {
    uint8_t     af;            /**< address family */
    uint32_t    num_routes;    /**< number of routes */
    route_t     routes[0];     /**< route list */
} route_table_t;

/**
 * @brief    build interval tree based LPM tree at the given memory address
 * @param[in] route_table      pointer to the route configuration
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
                          mem_addr_t lpm_tree_root_addr,
                          uint32_t lpm_mem_size);

#endif    /** __LPM_HPP__ */
