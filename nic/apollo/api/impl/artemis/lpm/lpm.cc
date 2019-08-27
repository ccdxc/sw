/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    lpm.cc
 *
 * @brief   LPM library implementation
 */

#include <math.h>
#include <stack>
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/lpm/lpm.hpp"
#include "nic/apollo/api/impl/artemis/lpm/lpm_sport.hpp"
#include "nic/apollo/api/impl/artemis/lpm/lpm_ipv4_acl.hpp"
#include "nic/apollo/api/impl/artemis/lpm/lpm_ipv6_acl.hpp"
#include "nic/apollo/api/impl/artemis/lpm/lpm_ipv4_tag.hpp"
#include "nic/apollo/api/impl/artemis/lpm/lpm_ipv6_tag.hpp"
#include "nic/apollo/api/impl/artemis/lpm/lpm_ipv4_sip.hpp"
#include "nic/apollo/api/impl/artemis/lpm/lpm_ipv6_sip.hpp"
#include "nic/apollo/api/impl/artemis/lpm/lpm_ipv4_meter.hpp"
#include "nic/apollo/api/impl/artemis/lpm/lpm_ipv6_meter.hpp"
#include "nic/apollo/api/impl/artemis/lpm/lpm_ipv4_route.hpp"
#include "nic/apollo/api/impl/artemis/lpm/lpm_ipv6_route.hpp"
#include "nic/apollo/api/impl/artemis/lpm/lpm_proto_dport.hpp"
#include "nic/apollo/api/impl/artemis/lpm/lpm_ipv4_peer_route.hpp"
#include "nic/apollo/api/impl/artemis/lpm/lpm_ipv6_peer_route.hpp"

using std::stack;

/**< invalid nexthop */
#define LPM_NEXTHOP_INVALID            -1
#define LPM_NEXTHOP_SIZE                2    /**< 2 byte nexthop */

/**< temporary stack elements maintained while building the interval table */
typedef struct itable_stack_elem_s {
    lpm_inode_t    interval;         /**< interval table node (to be emitted) */
    uint32_t       fallback_nhid;    /**< fallback nexthop */
    uint32_t       priority;         /**< Priority */
} itable_stack_elem_t;

/**< each LPM table is 64 bytes (512 bits/flit) */
#define LPM_TABLE_SIZE         (CACHE_LINE_SIZE)

/**
 * key size is 4 bytes for IPv4, 8 bytes (assuming max prefix length of 64)
 * for IPv6, 2 bytes for port tree and 4 bytes for proto-port tree
 */
static inline uint32_t
lpm_entry_key_size (itree_type_t tree_type)
{
    if (tree_type == ITREE_TYPE_IPV4) {
        return lpm_ipv4_route_key_size();
    } else if (tree_type == ITREE_TYPE_IPV6) {
        return lpm_ipv6_route_key_size();
    } else if (tree_type == ITREE_TYPE_PROTO_PORT) {
        return lpm_proto_dport_key_size();
    } else if (tree_type == ITREE_TYPE_PORT) {
        return lpm_sport_key_size();
    } else if (tree_type == ITREE_TYPE_IPV4_DIP_ACL) {
        return lpm_ipv4_acl_key_size();
    } else if (tree_type == ITREE_TYPE_IPV6_DIP_ACL) {
        return lpm_ipv6_acl_key_size();
    } else if (tree_type == ITREE_TYPE_METER_V4) {
        return lpm_ipv4_meter_key_size();
    } else if (tree_type == ITREE_TYPE_METER_V6) {
        return lpm_ipv6_meter_key_size();
    } else if (tree_type == ITREE_TYPE_TAG_V4) {
        return lpm_ipv4_tag_key_size();
    } else if (tree_type == ITREE_TYPE_TAG_V6) {
        return lpm_ipv6_tag_key_size();
    } else if (tree_type == ITREE_TYPE_IPV4_SIP_ACL) {
        return lpm_ipv4_sip_key_size();
    } else if (tree_type == ITREE_TYPE_IPV6_SIP_ACL) {
        return lpm_ipv6_sip_key_size();
    } else if (tree_type == ITREE_TYPE_IPV4_PEER_ROUTE) {
        return lpm_ipv4_peer_route_key_size();
    } else if (tree_type == ITREE_TYPE_IPV6_PEER_ROUTE) {
        return lpm_ipv6_peer_route_key_size();
    }
    return 0;
}

/**
 * number of keys per table is CACHE_LINE_SIZE/(sizeof(ipv4_addr_t) = 16 for
 * IPv4 and CACHE_LINE_SIZE/8 = 8 for IPv6 (assuming max prefix length of 64)
 * NOTE: this doesn't apply to last stage where each node also has 2 byte
 * nexthop
 */
static inline uint32_t
lpm_keys_per_table (itree_type_t tree_type)
{
    if (tree_type == ITREE_TYPE_IPV4) {
        return ((LPM_TABLE_SIZE / lpm_ipv4_route_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_IPV6) {
        return ((LPM_TABLE_SIZE / lpm_ipv6_route_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_PROTO_PORT) {
        return ((LPM_TABLE_SIZE / lpm_proto_dport_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_PORT) {
        return ((LPM_TABLE_SIZE / lpm_sport_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_IPV4_DIP_ACL) {
        return ((LPM_TABLE_SIZE / lpm_ipv4_acl_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_IPV6_DIP_ACL) {
        return ((LPM_TABLE_SIZE / lpm_ipv6_acl_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_METER_V4) {
        return ((LPM_TABLE_SIZE / lpm_ipv4_meter_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_METER_V6) {
        return ((LPM_TABLE_SIZE / lpm_ipv6_meter_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_TAG_V4) {
        return ((LPM_TABLE_SIZE / lpm_ipv4_tag_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_TAG_V6) {
        return ((LPM_TABLE_SIZE / lpm_ipv6_tag_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_IPV4_SIP_ACL) {
        return ((LPM_TABLE_SIZE / lpm_ipv4_sip_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_IPV6_SIP_ACL) {
        return ((LPM_TABLE_SIZE / lpm_ipv6_sip_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_IPV4_PEER_ROUTE) {
        return ((LPM_TABLE_SIZE / lpm_ipv4_peer_route_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_IPV6_PEER_ROUTE) {
        return ((LPM_TABLE_SIZE / lpm_ipv6_peer_route_key_size()) - 1);
    }
    return 0;
}

#define LPM_MAX_STAGES    8    /**< 8 stages can serve more than 1M v6 routes */

/**
 * each LPM stage consists of a set of LPM index tables and this stage meta
 * contains temporary state we maintain while building the table
 */
typedef struct lpm_stage_info_s {
    mem_addr_t    curr_table_addr;            /**< current table address */
    mem_addr_t    end_marker_addr;            /**< end of current stage address */
    uint32_t      curr_index;                 /**< current entry (key/key+nh) being populated */
    uint8_t       curr_table[LPM_TABLE_SIZE]; /**< current table being populated */
} lpm_stage_info_t;

typedef struct lpm_stage_meta_s {
    itree_type_t        tree_type;       /**< type of the tree being built */
    uint32_t            key_sz;          /**< key size for the tree */
    uint32_t            keys_per_table;  /**< no. of keys per table is same in
                                              all stages except last stage */
    /**< per stage info */
    lpm_stage_info_t    stage_info[LPM_MAX_STAGES];
} lpm_stage_meta_t;

/**
 * @brief    compute the number of stages needed for LPM lookup given the
 *           interval table scale
 * @param[in]    tree_type     type of the tree being built
 * @param[in]    num_intrvls   number of intervals in the interval table
 * @return       number of lookup stages (aka. depth of the interval tree)
 *
 * for IPv6:
 *     The last stage gives an 4-way decision. The other stages each give
 *     a 8-way decision.
 *     #stages = 1 + log8(num_intrvls/4.0)
 *             = 1 + log2(num_intrvls/4.0)/log2(8)
 *             = 1 + log2(num_intrvls/4.0)/3.0
 */
static inline uint32_t
lpm_stages (itree_type_t tree_type, uint32_t num_intrvls)
{
    if (tree_type == ITREE_TYPE_IPV4) {
        return lpm_ipv4_route_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_IPV6) {
        return lpm_ipv6_route_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_PROTO_PORT) {
        return lpm_proto_dport_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_PORT) {
        return lpm_sport_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_IPV4_DIP_ACL) {
        return lpm_ipv4_acl_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_IPV6_DIP_ACL) {
        return lpm_ipv6_acl_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_METER_V4) {
        return lpm_ipv4_meter_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_METER_V6) {
        return lpm_ipv6_meter_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_TAG_V4) {
        return lpm_ipv4_tag_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_TAG_V6) {
        return lpm_ipv6_tag_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_IPV4_SIP_ACL) {
        return lpm_ipv4_sip_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_IPV6_SIP_ACL) {
        return lpm_ipv6_sip_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_IPV4_PEER_ROUTE) {
        return lpm_ipv4_peer_route_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_IPV6_PEER_ROUTE) {
        return lpm_ipv6_peer_route_stages(num_intrvls);
    }

    return 0;
}

/**
 * @brief    compare two given routes and return -1, 0 or 1 based on where they
 *           fit on the number line, breaking the tie by comparing prefix
 *           lengths
 * @param[in]    r1    pointer to 1st route
 * @param[in]    r2    pointer to 2nd route
 * @param[in]    ctxt  opaque context
 * @return  -1, 0, 1 based on the comparison result
 */
static int
route_compare_cb (const void *route1, const void *route2, void *ctxt)
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
 * @brief    build an interval table corresponding to the give route table
 * @param[in]    route_table    route table
 * @param[in]   itable         interval node table that will be filled with
 *                             intervals and nexthop ids
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
lpm_build_interval_table (route_table_t *route_table, lpm_itable_t *itable)
{
    stack<itable_stack_elem_t>    s;
    itable_stack_elem_t           elem = {}, end = {};
    uint32_t                      num_intervals = 0;

    /**< create a stack elem with the highest prefix & invalid nh */
    end.interval.data = LPM_NEXTHOP_INVALID;
    end.interval.ipaddr.af = route_table->af;
    if (route_table->af == IP_AF_IPV4) {
        end.interval.ipaddr.addr.v4_addr = 0xFFFFFFFF;
    } else if (route_table->af == IP_AF_IPV6) {
        for (uint32_t i = 0; i < IP6_ADDR32_LEN; i++) {
            end.interval.ipaddr.addr.v6_addr.addr32[i] = 0xFFFFFF;
        }
    }

    /**< this elem would provide the catch all nh id */
    end.fallback_nhid = route_table->default_nhid;
    /**< which will have the lowest priority */
    end.priority = 0xFFFFFFFF;
    /**< push this elem to the stack */
    s.push(end);

    for (uint32_t i = 0; i < route_table->num_routes; i++) {
        /**< create the interval corresponding to the start of the prefix */
        elem.priority = route_table->routes[i].prio;
        elem.interval.data = route_table->routes[i].nhid;
        ip_prefix_ip_low(&route_table->routes[i].prefix, &elem.interval.ipaddr);
        /**
         * Pop and emit all the elements of the stack that are numerically lower
         * than the current interval.
         */
        while (IPADDR_LT(&s.top().interval.ipaddr, &elem.interval.ipaddr)) {
            /**
             * we hit this case when this route prefix is outside previous one
             * e.g. route1 = 10.10.0.0/16 and route2 = 11.0.0.0/8
             * while processing 11.0.0.0/8, top of stack would be 10.11.0.0
             * with fallback nexthop
             * pop the stack and emit the interval
             */
            itable->nodes[num_intervals++] = s.top().interval;
            s.pop();
        }

        /** If the priority of this prefix is lower than that of the stack
         * top, skip this prefix because this prefix is shadowed by a shorter
         * prefix with a higher priority */
        if (s.top().priority < route_table->routes[i].prio) {
            continue;
        }

        /**< Emit the current interval to the interval table */
        if ((num_intervals > 0) &&
            IPADDR_EQ(&itable->nodes[num_intervals-1].ipaddr,
                      &elem.interval.ipaddr)) {
            /**
             * overwrite the previous itable entry with this more specific
             * route, we hit this case when the current route is starting
             * exactly at the previous route
             * e.g. route1 = 10.10.0.0/16 and route2 = 10.10.0.0/24
             * while processing route2, route1 is already emitted to interval
             * table, but we should overwrite that with more specific route2's
             * nexthop
             */
            itable->nodes[num_intervals-1] = elem.interval;
        } else {
            itable->nodes[num_intervals++] = elem.interval;
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
        SDK_ASSERT(s.empty() == false);
        if (IPADDR_EQ(&(s.top().interval.ipaddr), &elem.interval.ipaddr)) {
            s.pop();
        }

        /** create the interval corresponding to the IP after the end of the
         * prefix */
        ip_prefix_ip_next(&route_table->routes[i].prefix, &elem.interval.ipaddr);
        /**< nexthop for the IPs beyond this prefix is the fallback nexthop */
        elem.interval.data = s.top().fallback_nhid;
        /**< priority within the current prefix is this route's priority */
        elem.priority = route_table->routes[i].prio;
        /**< fallback nexthop within the current prefix is this route's nexthop */
        elem.fallback_nhid = route_table->routes[i].nhid;

        /**
         * if stack top is same as the interval's end node, update its fallback
         * nhid to reflect the current route's nhid. We hit this case, when one
         * route is overlapping with the other towards right edge of the larger
         * prefix. E.g. route1 = 11.11.0.0/16 and route2 = 11.11.255.0/24
         * while processing route2, top of the stack wil have 11.12.0.0 with a
         * fallback nexthop pointing to 11.11.0.0/16. We need to update this to
         * point to 11.11.255.0/24. If the stack top is different from the
         * interval's end node, push the end node to the stack.
         */
        SDK_ASSERT(s.empty() == false);
        if (IPADDR_EQ(&(s.top().interval.ipaddr), &elem.interval.ipaddr)) {
            s.top().fallback_nhid = route_table->routes[i].nhid;
            s.top().priority = route_table->routes[i].prio;
        } else {
            /**< push this interval's end node to stack */
            s.push(elem);
        }
    }

    /**< emit whatever is remaining on the stack now */
    SDK_ASSERT(s.empty() == false);
    while (IPADDR_LT(&(s.top().interval.ipaddr), &end.interval.ipaddr)) {
        itable->nodes[num_intervals++] = s.top().interval;
        s.pop();
    }
    itable->num_intervals = num_intervals;
    return SDK_RET_OK;
}

static inline sdk_ret_t
lpm_add_key_to_stage (itree_type_t tree_type, lpm_stage_info_t *stage,
                      lpm_inode_t *lpm_inode)
{
    switch (tree_type) {
    case ITREE_TYPE_IPV4:
        lpm_ipv4_route_add_key_to_stage(stage->curr_table,
                                        stage->curr_index,
                                        lpm_inode);
        break;
    case ITREE_TYPE_IPV6:
        lpm_ipv6_route_add_key_to_stage(stage->curr_table,
                                        stage->curr_index,
                                        lpm_inode);
        break;
    case ITREE_TYPE_PORT:
        lpm_sport_add_key_to_stage(stage->curr_table,
                                   stage->curr_index,
                                   lpm_inode);
        break;
    case ITREE_TYPE_PROTO_PORT:
        lpm_proto_dport_add_key_to_stage(stage->curr_table,
                                         stage->curr_index,
                                         lpm_inode);
        break;
    case ITREE_TYPE_IPV4_DIP_ACL:
        lpm_ipv4_acl_add_key_to_stage(stage->curr_table,
                                      stage->curr_index,
                                      lpm_inode);
        break;
    case ITREE_TYPE_IPV6_DIP_ACL:
        lpm_ipv6_acl_add_key_to_stage(stage->curr_table,
                                      stage->curr_index,
                                      lpm_inode);
        break;
    case ITREE_TYPE_METER_V4:
        lpm_ipv4_meter_add_key_to_stage(stage->curr_table,
                                        stage->curr_index,
                                        lpm_inode);
        break;
    case ITREE_TYPE_METER_V6:
        lpm_ipv6_meter_add_key_to_stage(stage->curr_table,
                                        stage->curr_index,
                                        lpm_inode);
        break;
    case ITREE_TYPE_TAG_V4:
        lpm_ipv4_tag_add_key_to_stage(stage->curr_table,
                                      stage->curr_index,
                                      lpm_inode);
        break;
    case ITREE_TYPE_TAG_V6:
        lpm_ipv6_tag_add_key_to_stage(stage->curr_table,
                                      stage->curr_index,
                                      lpm_inode);
        break;
    case ITREE_TYPE_IPV4_SIP_ACL:
        lpm_ipv4_sip_add_key_to_stage(stage->curr_table,
                                      stage->curr_index,
                                      lpm_inode);
        break;
    case ITREE_TYPE_IPV6_SIP_ACL:
        lpm_ipv6_sip_add_key_to_stage(stage->curr_table,
                                      stage->curr_index,
                                      lpm_inode);
        break;
    case ITREE_TYPE_IPV4_PEER_ROUTE:
        lpm_ipv4_peer_route_add_key_to_stage(stage->curr_table,
                                             stage->curr_index,
                                             lpm_inode);
        break;
    case ITREE_TYPE_IPV6_PEER_ROUTE:
        lpm_ipv6_peer_route_add_key_to_stage(stage->curr_table,
                                             stage->curr_index,
                                             lpm_inode);
        break;
    default:
        SDK_ASSERT(0);
        break;
    }

    stage->curr_index++;
    return SDK_RET_OK;
}

static inline sdk_ret_t
lpm_write_stage_table (itree_type_t tree_type, lpm_stage_info_t *stage)
{
    switch (tree_type) {
    case ITREE_TYPE_IPV4:
        lpm_ipv4_route_write_stage_table(stage->curr_table_addr,
                                         stage->curr_table);
        break;
    case ITREE_TYPE_IPV6:
        lpm_ipv6_route_write_stage_table(stage->curr_table_addr,
                                         stage->curr_table);
        break;
    case ITREE_TYPE_PORT:
        lpm_sport_write_stage_table(stage->curr_table_addr,
                                    stage->curr_table);
        break;
    case ITREE_TYPE_PROTO_PORT:
        lpm_proto_dport_write_stage_table(stage->curr_table_addr,
                                          stage->curr_table);
        break;
    case ITREE_TYPE_IPV4_DIP_ACL:
        lpm_ipv4_acl_write_stage_table(stage->curr_table_addr,
                                       stage->curr_table);
        break;
    case ITREE_TYPE_IPV6_DIP_ACL:
        lpm_ipv6_acl_write_stage_table(stage->curr_table_addr,
                                       stage->curr_table);
        break;
    case ITREE_TYPE_METER_V4:
        lpm_ipv4_meter_write_stage_table(stage->curr_table_addr,
                                         stage->curr_table);
        break;
    case ITREE_TYPE_METER_V6:
        lpm_ipv6_meter_write_stage_table(stage->curr_table_addr,
                                         stage->curr_table);
        break;
    case ITREE_TYPE_TAG_V4:
        lpm_ipv4_tag_write_stage_table(stage->curr_table_addr,
                                       stage->curr_table);
        break;
    case ITREE_TYPE_TAG_V6:
        lpm_ipv6_tag_write_stage_table(stage->curr_table_addr,
                                       stage->curr_table);
        break;
    case ITREE_TYPE_IPV4_SIP_ACL:
        lpm_ipv4_sip_write_stage_table(stage->curr_table_addr,
                                       stage->curr_table);
        break;
    case ITREE_TYPE_IPV6_SIP_ACL:
        lpm_ipv6_sip_write_stage_table(stage->curr_table_addr,
                                       stage->curr_table);
        break;
    case ITREE_TYPE_IPV4_PEER_ROUTE:
        lpm_ipv4_peer_route_write_stage_table(stage->curr_table_addr,
                                              stage->curr_table);
        break;
    case ITREE_TYPE_IPV6_PEER_ROUTE:
        lpm_ipv6_peer_route_write_stage_table(stage->curr_table_addr,
                                              stage->curr_table);
        break;
    default:
        SDK_ASSERT(0);
        break;
    }
    /**< update this stage meta for next time */
    stage->curr_index = 0;
    stage->curr_table_addr += LPM_TABLE_SIZE;
    memset(stage->curr_table, 0xFF, LPM_TABLE_SIZE);
    return SDK_RET_OK;
}

static inline sdk_ret_t
lpm_add_key_to_last_stage (itree_type_t tree_type, lpm_stage_info_t *stage,
                           lpm_inode_t *lpm_inode)
{
    switch (tree_type) {
    case ITREE_TYPE_IPV4:
        lpm_ipv4_route_add_key_to_last_stage(stage->curr_table,
                                       stage->curr_index,
                                       lpm_inode);
        break;
    case ITREE_TYPE_IPV6:
        lpm_ipv6_route_add_key_to_last_stage(stage->curr_table,
                                             stage->curr_index,
                                             lpm_inode);
        break;
    case ITREE_TYPE_PORT:
        lpm_sport_add_key_to_last_stage(stage->curr_table,
                                        stage->curr_index,
                                        lpm_inode);
        break;
    case ITREE_TYPE_PROTO_PORT:
        lpm_proto_dport_add_key_to_last_stage(stage->curr_table,
                                              stage->curr_index,
                                              lpm_inode);
        break;
    case ITREE_TYPE_IPV4_DIP_ACL:
        lpm_ipv4_acl_add_key_to_last_stage(stage->curr_table,
                                           stage->curr_index,
                                           lpm_inode);
        break;
    case ITREE_TYPE_IPV6_DIP_ACL:
        lpm_ipv6_acl_add_key_to_last_stage(stage->curr_table,
                                           stage->curr_index,
                                           lpm_inode);
        break;
    case ITREE_TYPE_METER_V4:
        lpm_ipv4_meter_add_key_to_last_stage(stage->curr_table,
                                             stage->curr_index,
                                             lpm_inode);
        break;
    case ITREE_TYPE_METER_V6:
        lpm_ipv6_meter_add_key_to_last_stage(stage->curr_table,
                                             stage->curr_index,
                                             lpm_inode);
        break;
    case ITREE_TYPE_TAG_V4:
        lpm_ipv4_tag_add_key_to_last_stage(stage->curr_table,
                                           stage->curr_index,
                                           lpm_inode);
        break;
    case ITREE_TYPE_TAG_V6:
        lpm_ipv6_tag_add_key_to_last_stage(stage->curr_table,
                                           stage->curr_index,
                                           lpm_inode);
        break;
    case ITREE_TYPE_IPV4_SIP_ACL:
        lpm_ipv4_sip_add_key_to_last_stage(stage->curr_table,
                                           stage->curr_index,
                                           lpm_inode);
        break;
    case ITREE_TYPE_IPV6_SIP_ACL:
        lpm_ipv6_sip_add_key_to_last_stage(stage->curr_table,
                                           stage->curr_index,
                                           lpm_inode);
        break;
    case ITREE_TYPE_IPV4_PEER_ROUTE:
        lpm_ipv4_peer_route_add_key_to_last_stage(stage->curr_table,
                                                  stage->curr_index,
                                                  lpm_inode);
        break;
    case ITREE_TYPE_IPV6_PEER_ROUTE:
        lpm_ipv6_peer_route_add_key_to_last_stage(stage->curr_table,
                                                  stage->curr_index,
                                                  lpm_inode);
        break;
    default:
        SDK_ASSERT(0);
        break;
    }

    stage->curr_index++;
    return SDK_RET_OK;
}

static inline sdk_ret_t
lpm_set_default_data (itree_type_t tree_type, lpm_stage_info_t *stage,
                      uint32_t default_data)
{
    switch (tree_type) {
    case ITREE_TYPE_IPV4:
        lpm_ipv4_route_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_IPV6:
        lpm_ipv6_route_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_PORT:
        lpm_sport_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_PROTO_PORT:
        lpm_proto_dport_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_IPV4_DIP_ACL:
        lpm_ipv4_acl_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_IPV6_DIP_ACL:
        lpm_ipv6_acl_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_METER_V4:
        lpm_ipv4_meter_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_METER_V6:
        lpm_ipv6_meter_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_TAG_V4:
        lpm_ipv4_tag_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_TAG_V6:
        lpm_ipv6_tag_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_IPV4_SIP_ACL:
        lpm_ipv4_sip_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_IPV6_SIP_ACL:
        lpm_ipv6_sip_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_IPV4_PEER_ROUTE:
        lpm_ipv4_peer_route_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_IPV6_PEER_ROUTE:
        lpm_ipv6_peer_route_set_default_data(stage->curr_table, default_data);
        break;
    default:
        SDK_ASSERT(0);
        break;
    }

    return SDK_RET_OK;
}

static inline sdk_ret_t
lpm_write_last_stage_table (itree_type_t tree_type, lpm_stage_info_t *stage)
{
    switch (tree_type) {
    case ITREE_TYPE_IPV4:
        lpm_ipv4_route_write_last_stage_table(stage->curr_table_addr,
                                              stage->curr_table);
        break;
    case ITREE_TYPE_IPV6:
        lpm_ipv6_route_write_last_stage_table(stage->curr_table_addr,
                                              stage->curr_table);
        break;
    case ITREE_TYPE_PORT:
        lpm_sport_write_last_stage_table(stage->curr_table_addr,
                                         stage->curr_table);
        break;
    case ITREE_TYPE_PROTO_PORT:
        lpm_proto_dport_write_last_stage_table(stage->curr_table_addr,
                                               stage->curr_table);
        break;
    case ITREE_TYPE_IPV4_DIP_ACL:
        lpm_ipv4_acl_write_last_stage_table(stage->curr_table_addr,
                                            stage->curr_table);
        break;
    case ITREE_TYPE_IPV6_DIP_ACL:
        lpm_ipv6_acl_write_last_stage_table(stage->curr_table_addr,
                                            stage->curr_table);
        break;
    case ITREE_TYPE_METER_V4:
        lpm_ipv4_meter_write_last_stage_table(stage->curr_table_addr,
                                              stage->curr_table);
        break;
    case ITREE_TYPE_METER_V6:
        lpm_ipv6_meter_write_last_stage_table(stage->curr_table_addr,
                                              stage->curr_table);
        break;
    case ITREE_TYPE_TAG_V4:
        lpm_ipv4_tag_write_last_stage_table(stage->curr_table_addr,
                                            stage->curr_table);
        break;
    case ITREE_TYPE_TAG_V6:
        lpm_ipv6_tag_write_last_stage_table(stage->curr_table_addr,
                                            stage->curr_table);
        break;
    case ITREE_TYPE_IPV4_SIP_ACL:
        lpm_ipv4_sip_write_last_stage_table(stage->curr_table_addr,
                                            stage->curr_table);
        break;
    case ITREE_TYPE_IPV6_SIP_ACL:
        lpm_ipv6_sip_write_last_stage_table(stage->curr_table_addr,
                                            stage->curr_table);
        break;
    case ITREE_TYPE_IPV4_PEER_ROUTE:
        lpm_ipv4_peer_route_write_last_stage_table(stage->curr_table_addr,
                                                   stage->curr_table);
        break;
    case ITREE_TYPE_IPV6_PEER_ROUTE:
        lpm_ipv6_peer_route_write_last_stage_table(stage->curr_table_addr,
                                                   stage->curr_table);
        break;
    default:
        SDK_ASSERT(0);
        break;
    }
    /**< update last stage meta for next time */
    stage->curr_index = 0;
    stage->curr_table_addr += LPM_TABLE_SIZE;
    memset(stage->curr_table, 0xFF, LPM_TABLE_SIZE);
    return SDK_RET_OK;
}

/**
 * @brief    Flushes the partially filled tabled to HW at the end of processing.
 * @param[in] smeta                  meta information for the stages
 * @param[in] nstages                the number of stages
 * @return    SDK_RET_OK on success, failure status code on error
 */
static sdk_ret_t
lpm_flush_tables (lpm_stage_meta_t *smeta, uint32_t nstages,
                  lpm_itable_t *itable)
{
    for (uint32_t i = 0; i < (nstages - 1); i++) {
        lpm_write_stage_table(smeta->tree_type, &smeta->stage_info[i]);
    }

    if (nstages > 0) {
        if (itable->num_intervals > 0) {
            smeta->stage_info[nstages - 1].curr_index =
                    (smeta->keys_per_table >> 1) - 1;
            lpm_add_key_to_last_stage(smeta->tree_type,
                                      &smeta->stage_info[nstages - 1],
                                      &itable->nodes[itable->num_intervals - 1]);
        }
        lpm_write_last_stage_table(smeta->tree_type,
                                   &smeta->stage_info[nstages - 1]);
    }

    return SDK_RET_OK;
}

/**
 * @brief    Finalizes the stages by programming the last table of each stage
 *           to the hardware to make sure that the key with all 1's is taken
 *           care of correctly.
 * @param[in] smeta                  meta information for the stages
 * @param[in] nstages                the number of stages
 * @param[in] itable                 the interval table
 * @return    SDK_RET_OK on success, failure status code on error
 */
static sdk_ret_t
lpm_finalize_stages (lpm_stage_meta_t *smeta, uint32_t nstages,
                     lpm_itable_t *itable)
{
    for (uint32_t i = 0; i < (nstages - 1); i++) {
        /**< Has the last table in this stage programmed? */
        if (smeta->stage_info[i].curr_table_addr !=
            smeta->stage_info[i].end_marker_addr) {
            /**< If not, program it with all FF's */
            smeta->stage_info[i].curr_table_addr =
            smeta->stage_info[i].end_marker_addr - LPM_TABLE_SIZE;
            lpm_write_stage_table(smeta->tree_type, &smeta->stage_info[i]);
        }
    }

    if (nstages > 0 && itable->num_intervals > 0) {
        /**< Has the last table in this stage programmed? */
        if (smeta->stage_info[nstages - 1].curr_table_addr !=
            smeta->stage_info[nstages - 1].end_marker_addr) {
            /**< If not, program it with all FF's, with the data of the last
             * key being the data of the last node in the interval table */

            smeta->stage_info[nstages - 1].curr_table_addr =
            smeta->stage_info[nstages - 1].end_marker_addr - LPM_TABLE_SIZE;
            smeta->stage_info[nstages - 1].curr_index =
                    (smeta->keys_per_table >> 1) - 1;

            lpm_add_key_to_last_stage(smeta->tree_type,
                                      &smeta->stage_info[nstages - 1],
                                      &itable->nodes[itable->num_intervals - 1]);
            lpm_write_last_stage_table(smeta->tree_type,
                                       &smeta->stage_info[nstages - 1]);
        }
    }

    return SDK_RET_OK;
}

static inline void
lpm_promote_route (lpm_inode_t *inode, uint32_t stage, lpm_stage_meta_t *smeta)
{
    sdk_ret_t           ret;
    lpm_stage_info_t    *curr_stage;

    SDK_ASSERT(0 <= stage && stage < LPM_MAX_STAGES);
    curr_stage = &smeta->stage_info[stage];
    if (curr_stage->curr_index == smeta->keys_per_table) {
        /**< time to promote this to previous stage */
        lpm_promote_route(inode, stage - 1, smeta);
        /**< flush current table to hw since its full */
        ret = lpm_write_stage_table(smeta->tree_type, curr_stage);
        SDK_ASSERT(ret == SDK_RET_OK);
        return;
    }

    /**< we need to save this node in this stage */
    ret = lpm_add_key_to_stage(smeta->tree_type, curr_stage, inode);
    SDK_ASSERT(ret == SDK_RET_OK);
}

/**
 * @brief    build LPM tree based on the interval node table
 * @param[in] itable                interval node table
 * @param[in] default_nh            default nexthop id
 * @param[in] max_routes            max routes supported per route table
 * @param[in] lpm_tree_root_addr    pointer to the memory address at which tree
 *                                  should be built
 * @param[in] lpm_mem_size          size of LPM memory block
 * @return    SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t
lpm_build_tree (lpm_itable_t *itable, uint32_t default_nh, uint32_t max_routes,
                mem_addr_t lpm_tree_root_addr, uint32_t lpm_mem_size)
{
    sdk_ret_t           ret;
    mem_addr_t          addr;
    lpm_stage_info_t    *last_stage_info;
    lpm_stage_meta_t    smeta;
    uint32_t            nstages, nkeys_per_table;

    PDS_TRACE_DEBUG("Building LPM tree type %u, interval count %u, "
                    "default nh %u, max routes %u, root addr 0x%llx, "
                    "LPM block size %u", itable->tree_type,
                    itable->num_intervals, default_nh, max_routes,
                    lpm_tree_root_addr, lpm_mem_size);

    /**< compute the # of stages, required including the def route */
    nstages = lpm_stages(itable->tree_type, ((max_routes + 1) << 1));
    SDK_ASSERT(nstages <= LPM_MAX_STAGES);

    /**< initialize all the stage meta */
    memset(&smeta, 0, sizeof(smeta));
    smeta.tree_type = itable->tree_type;
    smeta.key_sz = lpm_entry_key_size(itable->tree_type);
    smeta.keys_per_table = lpm_keys_per_table(itable->tree_type);
    addr = lpm_tree_root_addr;
    for (uint32_t i = 0, ntables = 1; i < nstages; i++) {
        smeta.stage_info[i].curr_table_addr = addr;
        addr += ntables * LPM_TABLE_SIZE;
        smeta.stage_info[i].end_marker_addr = addr;
        ntables *= (smeta.keys_per_table + 1);
        memset(smeta.stage_info[i].curr_table, 0xFF, LPM_TABLE_SIZE);
    }
    SDK_ASSERT(addr <= lpm_tree_root_addr + lpm_mem_size);

    /**
     * walk all the interval tree nodes & from the interval tree bottom up,
     * last stage has half as many keys as other stages, so divide
     * lpm_keys_per_table() by 2
     */
    nkeys_per_table = smeta.keys_per_table >> 1;
    last_stage_info = &smeta.stage_info[nstages-1];
    /**< set the default data for the first table */
    lpm_set_default_data(smeta.tree_type, last_stage_info, default_nh);
    for (uint32_t i = 0; i < itable->num_intervals; i++) {
        lpm_add_key_to_last_stage(smeta.tree_type, last_stage_info,
                                  &itable->nodes[i]);
        if (last_stage_info->curr_index == nkeys_per_table) {
            /**< current table is full and ready to write */
            /**< write the table here only if there are more nodes */
            /**< otherwise, lpm_flush_tables() will write it */
            if (++i < itable->num_intervals) {
                /**< promote the next node, if any, to the previous stage(s) */
                lpm_promote_route(&itable->nodes[i], nstages - 2, &smeta);
                /**< Write the current table to HW memory */
                lpm_write_last_stage_table(smeta.tree_type, last_stage_info);
                /**< set the default data for the next table */
                /**< it comes from the most recently promoted entry */
                lpm_set_default_data(smeta.tree_type, last_stage_info,
                                     itable->nodes[i].data);
            }
        }
    }
    ret = lpm_flush_tables(&smeta, nstages, itable);
    SDK_ASSERT(ret == SDK_RET_OK);
    ret = lpm_finalize_stages(&smeta, nstages, itable);
    SDK_ASSERT(ret == SDK_RET_OK);
    return SDK_RET_OK;
}

/**
 * @brief    build interval tree based LPM tree at the given memory address
 * @param[in] route_table           pointer to the route configuration
 * @param[in] type                  type of LPM
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
lpm_tree_create (route_table_t *route_table, itree_type_t type,
                 mem_addr_t lpm_tree_root_addr, uint32_t lpm_mem_size)
{
    sdk_ret_t       ret;
    lpm_itable_t    itable;

    if (unlikely(((route_table->af != IP_AF_IPV4) &&
                  (route_table->af != IP_AF_IPV6)))) {
        return sdk::SDK_RET_INVALID_ARG;
    }
    SDK_ASSERT(route_table->num_routes <= route_table->max_routes);

    /**< sort the given route table */
    qsort_r(route_table->routes, route_table->num_routes,
            sizeof(route_t), route_compare_cb, route_table);

    /**< allocate memory for creating all interval tree nodes */
    memset(&itable, 0, sizeof(itable));
    itable.nodes =
            (lpm_inode_t *)malloc(sizeof(lpm_inode_t) *
                                  (route_table->num_routes << 1));
    if (itable.nodes == NULL) {
        return sdk::SDK_RET_OOM;
    }
    itable.tree_type = type;
    ret = lpm_build_interval_table(route_table, &itable);
    if (unlikely(ret != SDK_RET_OK)) {
        goto cleanup;
    }
    ret = lpm_build_tree(&itable, route_table->default_nhid,
                         route_table->max_routes, lpm_tree_root_addr,
                         lpm_mem_size);

cleanup:

    free(itable.nodes);
    return ret;
}
