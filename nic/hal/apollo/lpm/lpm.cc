/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    lpm.hpp
 *
 * @brief   LPM library implementation
 */

#include <math.h>
#include <stack>
#include "nic/hal/apollo/lpm/lpm.hpp"

using std::stack;

#define LPM_NEXTHOP_INVALID            -1
typedef struct lpm_itree_node_s {
    ip_addr_t     ipaddr;
    uint16_t      nhid;
} lpm_itree_node_t;

/**< temporary stack elements maintained while building the interval tree */
typedef struct itree_stack_elem_s {
    lpm_itree_node_t    interval;
    uint16_t            fallback_nhid;
} itree_stack_elem_t;

/**
 * @brief    compute the number of stages needed for LPM lookup given the
 *           route table scale
 * @param[in]    num_routes    number of routes in the route table
 * @return       number of lookup stages (aka. depth of the interval tree)
 *
 * The computation is done as follows for IPv4:
 *     log16(2 * num_routes) = log16(1 << num_routes) =
 *     log2(1 << num_routes)/log2(16) = log2(1 << num_routes)/4
 *     div by 4 is accomplished by right shift as we need to round up at the end
 * and for IPv6:
 *     log4(2 * num_routes)
 */
static inline uint32_t
compute_lpm_stages (uint8_t ip_af, uint32_t num_routes)
{
    if (ip_af == IP_AF_IPV4) {
        return (uint32_t)ceil(log2f((float)(1 << num_routes)/4.0));
    } else {
        return (uint32_t)ceil(log2f((float)(1 << num_routes)/2.0));
    }
}

/**
 * @brief    compare two given routes and return -1, 0 or 1 based on where they
 *           fit on the number line, breaking the tie by comparing prefix
 *           lengths
 * @param[in]    r1    pointer to 1st route
 * @param[in]    r2    pointer to 2nd route
 * @return  -1, 0, 1 based on the comparison result
 */
static int
route_compare_cb (const void *route1, const void *route2)
{
    route_t    *r1 = (route_t *)route1;
    route_t    *r2 = (route_t *)route2;

    if (IPADDR_LT(&r1->prefix.addr, &r2->prefix.addr)) {
        return -1;
    } else if (IPADDR_GT(&r1->prefix.addr, &r2->prefix.addr)) {
        return 1;
    } else if (IPADDR_EQ(&r1->prefix.addr, &r2->prefix.addr)) {
        if (r1->prefix.len < r2->prefix.len) {
            return -1;
        } else if (r1->prefix.len > r2->prefix.len) {
            return 1;
        }
    }
    /**< two exact same prefixes, configuration error */
    SDK_ASSERT(0);
    return 0;
}

/**
 * @brief    given number of routes, compute the number of intervals in the
 *           interval tree
 * @param[in]    num_routes number of routes
 * @return    number of intervale tree nodes needed to support the given route
 *            scale
 */
static inline uint32_t
compute_intervals (uint32_t num_routes)
{
    num_routes--;
    num_routes |= num_routes >> 1;
    num_routes |= num_routes >> 2;
    num_routes |= num_routes >> 4;
    num_routes |= num_routes >> 8;
    num_routes |= num_routes >> 16;
    num_routes++;

    return (1 << num_routes);
}

/**
 * @brief    build an interval table corresponding to the give route table
 * @param[in]    route_table    route table
 * @param[out]   itable         interval node table
 * @return    number of intervals created
 */
static inline uint32_t
lpm_build_interval_table (route_table_t *route_table, lpm_itree_node_t *itable)
{
    stack<itree_stack_elem_t>    s;
    itree_stack_elem_t           elem, end;
    uint32_t                     num_intervals = 0;

    /**< initialize the stack with highest prefix & invalid nh at the bottom */
    end.fallback_nhid = LPM_NEXTHOP_INVALID;
    end.interval.nhid = LPM_NEXTHOP_INVALID;
    end.interval.ipaddr.af = route_table->af;
    if (route_table->af == IP_AF_IPV4) {
        end.interval.ipaddr.addr.v4_addr = 0xFFFFFFFF;
    } else if (route_table->af == IP_AF_IPV6) {
        for (uint32_t i = 0; i < IP6_ADDR32_LEN; i++) {
            end.interval.ipaddr.addr.v6_addr.addr32[i] = 0xFFFFFF;
        }
    }
    s.push(end);

    for (uint32_t i = 0; i < route_table->num_routes; i++) {
        /**< add itree node corresponding to the start of the route prefix */
        ip_prefix_ip_low(&route_table->routes[i].prefix, &elem.interval.ipaddr);
        elem.interval.nhid = route_table->routes[i].nhid;
        /**
         * if there is anything to pop from stack before pushing the new
         * interval node corresponding to this route, pop those nodes first
         */
        while (IPADDR_LT(&s.top().interval.ipaddr, &elem.interval.ipaddr)) {
            /**
             * we hit this case when this route prefix is outside previous one
             * e.g. route1 = 10.10.0.0/16 and route2 = 11.0.0.0/8
             * while processing 11.0.0.0/8, top of stack would be 10.11.0.0
             * with fallback nexthop
             * pop the stack and emit the interval
             */
            itable[num_intervals++] = s.top().interval;
            s.pop();
        }

        /**< start address of prefix is always emitted to the interval table */
        if ((num_intervals > 0) &&
            IPADDR_EQ(&itable[num_intervals-1].ipaddr, &elem.interval.ipaddr)) {
            /**
             * overwrite the previous itable entry with this more specific
             * route, we hit this case when the current route is starting
             * exactly at the previous route
             * e.g. route1 = 10.10.0.0/16 and route2 = 10.10.0.0/24
             * while processing route2, route1 is already emitted to interval
             * table, but we should overwrite that with more specific route2's
             * nexthop
             */
            itable[num_intervals-1] = elem.interval;
        } else {
            itable[num_intervals++] = elem.interval;
        }

        /**
         * if stack top is same as current, pop it since we already emitted this
         * interval. we hit this case, when this route is adjacent to the one on
         * the stack
         * e.g. route1 = 10.10.0.0/16 and route2 = 10.11.0.0/16
         * while processing route1, 10.11.0.0 is pushed to top of the stack
         * but when 10.11.0.0/16 is seen, we don't need that interval as we have
         * actual user configured route for it
         */
        if (IPADDR_EQ(&s.top().interval.ipaddr, &elem.interval.ipaddr)) {
            s.pop();
        }

        /**< add node corresponding to the IP after the end of route prefix */
        ip_prefix_ip_next(&route_table->routes[i].prefix, &elem.interval.ipaddr);
        /**< nexthop id for the IP beyond this prefix is the fallback nexthop */
        elem.interval.nhid = s.top().fallback_nhid;
        /**
         * fallback nexthop for anyroute within the current prefix is this
         * route's nexthop
         */
        elem.fallback_nhid = route_table->routes[i].nhid;

        /**
         * if stack top is same as the interval's end node, pop it as we will
         * push more specific node now, we hit this case, when one route is
         * overlpping with the other towards right edge of the larger prefix
         * e.g. route1 = 11.11.0.0/16 and route2 = 11.11.255.0/24
         * while processing route2, top of the stack wil have 11.12.0.0 with
         * a fallback nexthop that is not relevant anymore once we saw
         * 11.11.255.0, so we should pop it
         */
        if (IPADDR_EQ(&s.top().interval.ipaddr, &elem.interval.ipaddr)) {
            s.pop();
        }

        /**< push this interval's end node to stack */
        s.push(elem);
    }

    /**< emit whatever is remaining on the stack now */
    while (IPADDR_LT(&s.top().interval.ipaddr, &end.interval.ipaddr)) {
        itable[num_intervals++] = s.top().interval;
        s.pop();
    }
    return num_intervals;
}

/**
 * @brief    build LPM tree based on the interval node table
 * @param[in] itable                interval node table
 * @param[in] num_intervals         number of intervals in the interval table
 * @param[in] lpm_tree_root_addr    pointer to the memory address at which tree
 *                                  should be built
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
lpm_build_tree (lpm_itree_node_t *itable, uint32_t num_intervals,
                mem_addr_t lpm_tree_root_addr)
{
    return SDK_RET_OK;
}

/**
 * @brief    build interval tree based LPM tree at the given memory address
 * @param[in] route_table           pointer to the route configuration
 * @param[in] lpm_tree_root_addr    pointer to the memory address at which tree
 *                                  should be built
 * @param[in] lpm_mem_size          LPM memory block size provided (for error
 *                                  detection)
 * @return    SDK_RET_OK on success, failure status code on error
 *
 * NOTE: route_table will be modified internally as the library does
 *       in-place sorting on the given routing table
 */
sdk_ret_t
lpm_tree_create (route_table_t *route_table,
                 mem_addr_t lpm_tree_root_addr, uint32_t lpm_mem_size)
{
    uint32_t            num_intervals;
    lpm_itree_node_t    *itable;

    if (route_table->num_routes == 0) {
        return sdk::SDK_RET_INVALID_ARG;
    }
    /**< sort the given route table */
    qsort(route_table->routes, route_table->num_routes,
          sizeof(route_t), route_compare_cb);
    /**< allocate memory for creating all interval tree nodes */
    //num_intervals = compute_intervals(route_table->num_routes);
    itable = (lpm_itree_node_t *)
                 malloc(sizeof(lpm_itree_node_t) *
                        (1 << route_table->num_routes));
    if (itable == NULL) {
        return sdk::SDK_RET_OOM;
    }
    num_intervals = lpm_build_interval_table(route_table, itable);
    lpm_build_tree(itable, num_intervals, lpm_tree_root_addr);
    free(itable);

    return SDK_RET_OK;
}
