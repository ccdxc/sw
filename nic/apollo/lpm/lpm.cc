/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    lpm.cc
 *
 * @brief   LPM library implementation
 */

#include <math.h>
#include <stack>
#include "nic/apollo/lpm/lpm.hpp"
#include "nic/apollo/lpm/lpm_sport.hpp"
#include "nic/apollo/lpm/lpm_ipv4_acl.hpp"
#include "nic/apollo/lpm/lpm_ipv4_route.hpp"
#include "nic/apollo/lpm/lpm_proto_dport.hpp"

using std::stack;

/**< invalid nexthop */
#define LPM_NEXTHOP_INVALID            -1
#define LPM_NEXTHOP_SIZE                2    /**< 2 byte nexthop */

/**< temporary stack elements maintained while building the interval table */
typedef struct itable_stack_elem_s {
    lpm_inode_t    interval;         /**< interval table node (to be emitted) */
    uint32_t       fallback_nhid;    /**< fallback nexthop */
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
    } else if (tree_type == ITREE_TYPE_PROTO_PORT) {
        return lpm_proto_dport_key_size();
    } else if (tree_type == ITREE_TYPE_PORT) {
        return lpm_sport_key_size();
    } else if (tree_type == ITREE_TYPE_IPV4_ACL) {
        return lpm_ipv4_acl_key_size();
    } else if (tree_type == ITREE_TYPE_IPV6) {
        return 8;
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
    } else if (tree_type == ITREE_TYPE_PROTO_PORT) {
        return ((LPM_TABLE_SIZE / lpm_proto_dport_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_PORT) {
        return ((LPM_TABLE_SIZE / lpm_sport_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_IPV4_ACL) {
        return ((LPM_TABLE_SIZE / lpm_ipv4_acl_key_size()) - 1);
    } else if (tree_type == ITREE_TYPE_IPV6) {
        return 7;
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
    } else if (tree_type == ITREE_TYPE_PROTO_PORT) {
        return lpm_proto_dport_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_PORT) {
        return lpm_sport_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_IPV4_ACL) {
        return lpm_ipv4_acl_stages(num_intrvls);
    } else if (tree_type == ITREE_TYPE_IPV6) {
        // 1 * 4-way last stage, plus (n-1) * 8-way stages
        return (1 + ((uint32_t)ceil(log2f((float)(num_intrvls/4.0))/3.0)));
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
 * @param[in]   itable         interval node table that will be filled with
 *                             intervals and nexthop ids
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
lpm_build_interval_table (route_table_t *route_table, lpm_itable_t *itable)
{
    stack<itable_stack_elem_t>    s;
    itable_stack_elem_t           elem, end;
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
    /**< push this elem to the stack */
    s.push(end);

    for (uint32_t i = 0; i < route_table->num_routes; i++) {
        /**< add itree node corresponding to the start of the route prefix */
        ip_prefix_ip_low(&route_table->routes[i].prefix, &elem.interval.ipaddr);
        elem.interval.data = route_table->routes[i].nhid;
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
            itable->nodes[num_intervals++] = s.top().interval;
            s.pop();
        }

        /**< start address of prefix is always emitted to the interval table */
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

        /**< add node corresponding to the IP after the end of route prefix */
        ip_prefix_ip_next(&route_table->routes[i].prefix,
                          &elem.interval.ipaddr);
        /**< nexthop id for the IP beyond this prefix is the fallback nexthop */
        elem.interval.data = s.top().fallback_nhid;
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
        SDK_ASSERT(s.empty() == false);
        if (IPADDR_EQ(&(s.top().interval.ipaddr), &elem.interval.ipaddr)) {
            s.pop();
        }

        /**< push this interval's end node to stack */
        s.push(elem);
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
    case ITREE_TYPE_IPV4_ACL:
        lpm_ipv4_acl_add_key_to_stage(stage->curr_table,
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
    case ITREE_TYPE_PORT:
        lpm_sport_write_stage_table(stage->curr_table_addr,
                                    stage->curr_table);
        break;
    case ITREE_TYPE_PROTO_PORT:
        lpm_proto_dport_write_stage_table(stage->curr_table_addr,
                                          stage->curr_table);
        break;
    case ITREE_TYPE_IPV4_ACL:
        lpm_ipv4_acl_write_stage_table(stage->curr_table_addr,
                                       stage->curr_table);
        break;
    default:
        SDK_ASSERT(0);
        break;
    }
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
    case ITREE_TYPE_IPV4_ACL:
        lpm_ipv4_acl_add_key_to_last_stage(stage->curr_table,
                                           stage->curr_index,
                                           lpm_inode);
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
    case ITREE_TYPE_PORT:
        lpm_sport_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_PROTO_PORT:
        lpm_proto_dport_set_default_data(stage->curr_table, default_data);
        break;
    case ITREE_TYPE_IPV4_ACL:
        lpm_ipv4_acl_set_default_data(stage->curr_table, default_data);
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
    case ITREE_TYPE_PORT:
        lpm_sport_write_last_stage_table(stage->curr_table_addr,
                                         stage->curr_table);
        break;
    case ITREE_TYPE_PROTO_PORT:
        lpm_proto_dport_write_last_stage_table(stage->curr_table_addr,
                                               stage->curr_table);
        break;
    case ITREE_TYPE_IPV4_ACL:
        lpm_ipv4_acl_write_last_stage_table(stage->curr_table_addr,
                                            stage->curr_table);
        break;
    default:
        SDK_ASSERT(0);
        break;
    }
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
lpm_flush_partial_tables (lpm_stage_meta_t *smeta, uint32_t nstages,
                          uint32_t curr_default)
{
    for (uint32_t i = 0; i < (nstages-1); i++) {
        lpm_write_stage_table(smeta->tree_type, &smeta->stage_info[i]);
    }

    if (nstages > 0) {
        /**< table is ready to write. set the default data */
        lpm_set_default_data(smeta->tree_type, &smeta->stage_info[nstages - 1],
                             curr_default);
        /**< Write the table to HW memory */
        lpm_write_last_stage_table(smeta->tree_type,
                                   &smeta->stage_info[nstages - 1]);

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
        /**< flush current table to hw since its full */
        ret = lpm_write_stage_table(smeta->tree_type, curr_stage);
        SDK_ASSERT(ret == SDK_RET_OK);
        /**< time to promote this to previous stage */
        lpm_promote_route(inode, stage - 1, smeta);
        /**< update this stage meta for next time */
        curr_stage->curr_index = 0;
        curr_stage->curr_table_addr += LPM_TABLE_SIZE;
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
    uint32_t            curr_default = default_nh;

    /**< compute the # of stages, required including the def route */
    nstages = lpm_stages(itable->tree_type, ((max_routes+1)<<1));
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
        ntables *= (smeta.keys_per_table + 1);
        memset(smeta.stage_info[i].curr_table, 0xFF, LPM_TABLE_SIZE);
    }
    SDK_ASSERT(addr <= lpm_tree_root_addr + lpm_mem_size);

    /**
     * walk all the interval tree nodes & from the interval tree bottom up,
     * last stage has half as many keys as other stages, so divide
     * lpm_keys_per_table() by 2
     */
    nkeys_per_table = lpm_keys_per_table(itable->tree_type) << 1;
    last_stage_info = &smeta.stage_info[nstages-1];
    for (uint32_t i = 0; i < itable->num_intervals; i++) {
        lpm_add_key_to_last_stage(smeta.tree_type, last_stage_info,
                                  &itable->nodes[i]);

        if (last_stage_info->curr_index == nkeys_per_table) {
            /**< table is ready to write. set the default data */
            lpm_set_default_data(smeta.tree_type, last_stage_info,
                                 curr_default);
            /**< Write the table to HW memory */
            lpm_write_last_stage_table(smeta.tree_type, last_stage_info);

            if (++i < itable->num_intervals) {
                /**< propogate this node to one of the previous stages */
                lpm_promote_route(&itable->nodes[i], nstages - 2, &smeta);
                /**< default data comes from the most recently promoted entry */
                curr_default = itable->nodes[i].data;
                /**< update the table address  */
                last_stage_info->curr_index = 0;
                last_stage_info->curr_table_addr += LPM_TABLE_SIZE;
            }
        }
    }
    ret = lpm_flush_partial_tables(&smeta, nstages, curr_default);
    SDK_ASSERT(ret == SDK_RET_OK);
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
    sdk_ret_t       ret;
    lpm_itable_t    itable;

    if (unlikely((route_table->num_routes == 0) ||
                 ((route_table->af != IP_AF_IPV4) &&
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
    if (route_table->af == IP_AF_IPV4) {
        itable.tree_type = ITREE_TYPE_IPV4;
    } else {
        itable.tree_type = ITREE_TYPE_IPV6;
    }
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









#if 0
/* Move the following in to their appropriate SLACL file */

inline sdk_ret_t
slacl_p2_pack (uint8_t *bytes, uint32_t idx, uint8_t data)
{
    auto table = (slacl_p2_actiondata_t *)bytes;

    switch (idx) {
    case 0:
        table->action_u.slacl_p2_slacl_p2.id000 = data;
        break;
    case 1:
        table->action_u.slacl_p2_slacl_p2.id001 = data;
        break;
    case 2:
        table->action_u.slacl_p2_slacl_p2.id002 = data;
        break;
    case 3:
        table->action_u.slacl_p2_slacl_p2.id003 = data;
        break;
    case 4:
        table->action_u.slacl_p2_slacl_p2.id004 = data;
        break;
    case 5:
        table->action_u.slacl_p2_slacl_p2.id005 = data;
        break;
    case 6:
        table->action_u.slacl_p2_slacl_p2.id006 = data;
        break;
    case 7:
        table->action_u.slacl_p2_slacl_p2.id007 = data;
        break;
    case 8:
        table->action_u.slacl_p2_slacl_p2.id008 = data;
        break;
    case 9:
        table->action_u.slacl_p2_slacl_p2.id009 = data;
        break;
    case 10:
        table->action_u.slacl_p2_slacl_p2.id010 = data;
        break;
    case 11:
        table->action_u.slacl_p2_slacl_p2.id011 = data;
        break;
    case 12:
        table->action_u.slacl_p2_slacl_p2.id012 = data;
        break;
    case 13:
        table->action_u.slacl_p2_slacl_p2.id013 = data;
        break;
    case 14:
        table->action_u.slacl_p2_slacl_p2.id014 = data;
        break;
    case 15:
        table->action_u.slacl_p2_slacl_p2.id015 = data;
        break;
    case 16:
        table->action_u.slacl_p2_slacl_p2.id016 = data;
        break;
    case 17:
        table->action_u.slacl_p2_slacl_p2.id017 = data;
        break;
    case 18:
        table->action_u.slacl_p2_slacl_p2.id018 = data;
        break;
    case 19:
        table->action_u.slacl_p2_slacl_p2.id019 = data;
        break;
    case 20:
        table->action_u.slacl_p2_slacl_p2.id020 = data;
        break;
    case 21:
        table->action_u.slacl_p2_slacl_p2.id021 = data;
        break;
    case 22:
        table->action_u.slacl_p2_slacl_p2.id022 = data;
        break;
    case 23:
        table->action_u.slacl_p2_slacl_p2.id023 = data;
        break;
    case 24:
        table->action_u.slacl_p2_slacl_p2.id024 = data;
        break;
    case 25:
        table->action_u.slacl_p2_slacl_p2.id025 = data;
        break;
    case 26:
        table->action_u.slacl_p2_slacl_p2.id026 = data;
        break;
    case 27:
        table->action_u.slacl_p2_slacl_p2.id027 = data;
        break;
    case 28:
        table->action_u.slacl_p2_slacl_p2.id028 = data;
        break;
    case 29:
        table->action_u.slacl_p2_slacl_p2.id029 = data;
        break;
    case 30:
        table->action_u.slacl_p2_slacl_p2.id030 = data;
        break;
    case 31:
        table->action_u.slacl_p2_slacl_p2.id031 = data;
        break;
    case 32:
        table->action_u.slacl_p2_slacl_p2.id032 = data;
        break;
    case 33:
        table->action_u.slacl_p2_slacl_p2.id033 = data;
        break;
    case 34:
        table->action_u.slacl_p2_slacl_p2.id034 = data;
        break;
    case 35:
        table->action_u.slacl_p2_slacl_p2.id035 = data;
        break;
    case 36:
        table->action_u.slacl_p2_slacl_p2.id036 = data;
        break;
    case 37:
        table->action_u.slacl_p2_slacl_p2.id037 = data;
        break;
    case 38:
        table->action_u.slacl_p2_slacl_p2.id038 = data;
        break;
    case 39:
        table->action_u.slacl_p2_slacl_p2.id039 = data;
        break;
    case 40:
        table->action_u.slacl_p2_slacl_p2.id040 = data;
        break;
    case 41:
        table->action_u.slacl_p2_slacl_p2.id041 = data;
        break;
    case 42:
        table->action_u.slacl_p2_slacl_p2.id042 = data;
        break;
    case 43:
        table->action_u.slacl_p2_slacl_p2.id043 = data;
        break;
    case 44:
        table->action_u.slacl_p2_slacl_p2.id044 = data;
        break;
    case 45:
        table->action_u.slacl_p2_slacl_p2.id045 = data;
        break;
    case 46:
        table->action_u.slacl_p2_slacl_p2.id046 = data;
        break;
    case 47:
        table->action_u.slacl_p2_slacl_p2.id047 = data;
        break;
    case 48:
        table->action_u.slacl_p2_slacl_p2.id048 = data;
        break;
    case 49:
        table->action_u.slacl_p2_slacl_p2.id049 = data;
        break;
    case 50:
        table->action_u.slacl_p2_slacl_p2.id050 = data;
        break;
    case 51:
        table->action_u.slacl_p2_slacl_p2.id051 = data;
        break;
    case 52:
        table->action_u.slacl_p2_slacl_p2.id052 = data;
        break;
    case 53:
        table->action_u.slacl_p2_slacl_p2.id053 = data;
        break;
    case 54:
        table->action_u.slacl_p2_slacl_p2.id054 = data;
        break;
    case 55:
        table->action_u.slacl_p2_slacl_p2.id055 = data;
        break;
    case 56:
        table->action_u.slacl_p2_slacl_p2.id056 = data;
        break;
    case 57:
        table->action_u.slacl_p2_slacl_p2.id057 = data;
        break;
    case 58:
        table->action_u.slacl_p2_slacl_p2.id058 = data;
        break;
    case 59:
        table->action_u.slacl_p2_slacl_p2.id059 = data;
        break;
    case 60:
        table->action_u.slacl_p2_slacl_p2.id060 = data;
        break;
    case 61:
        table->action_u.slacl_p2_slacl_p2.id061 = data;
        break;
    case 62:
        table->action_u.slacl_p2_slacl_p2.id062 = data;
        break;
    case 63:
        table->action_u.slacl_p2_slacl_p2.id063 = data;
        break;
    case 64:
        table->action_u.slacl_p2_slacl_p2.id064 = data;
        break;
    case 65:
        table->action_u.slacl_p2_slacl_p2.id065 = data;
        break;
    case 66:
        table->action_u.slacl_p2_slacl_p2.id066 = data;
        break;
    case 67:
        table->action_u.slacl_p2_slacl_p2.id067 = data;
        break;
    case 68:
        table->action_u.slacl_p2_slacl_p2.id068 = data;
        break;
    case 69:
        table->action_u.slacl_p2_slacl_p2.id069 = data;
        break;
    case 70:
        table->action_u.slacl_p2_slacl_p2.id070 = data;
        break;
    case 71:
        table->action_u.slacl_p2_slacl_p2.id071 = data;
        break;
    case 72:
        table->action_u.slacl_p2_slacl_p2.id072 = data;
        break;
    case 73:
        table->action_u.slacl_p2_slacl_p2.id073 = data;
        break;
    case 74:
        table->action_u.slacl_p2_slacl_p2.id074 = data;
        break;
    case 75:
        table->action_u.slacl_p2_slacl_p2.id075 = data;
        break;
    case 76:
        table->action_u.slacl_p2_slacl_p2.id076 = data;
        break;
    case 77:
        table->action_u.slacl_p2_slacl_p2.id077 = data;
        break;
    case 78:
        table->action_u.slacl_p2_slacl_p2.id078 = data;
        break;
    case 79:
        table->action_u.slacl_p2_slacl_p2.id079 = data;
        break;
    case 80:
        table->action_u.slacl_p2_slacl_p2.id080 = data;
        break;
    case 81:
        table->action_u.slacl_p2_slacl_p2.id081 = data;
        break;
    case 82:
        table->action_u.slacl_p2_slacl_p2.id082 = data;
        break;
    case 83:
        table->action_u.slacl_p2_slacl_p2.id083 = data;
        break;
    case 84:
        table->action_u.slacl_p2_slacl_p2.id084 = data;
        break;
    case 85:
        table->action_u.slacl_p2_slacl_p2.id085 = data;
        break;
    case 86:
        table->action_u.slacl_p2_slacl_p2.id086 = data;
        break;
    case 87:
        table->action_u.slacl_p2_slacl_p2.id087 = data;
        break;
    case 88:
        table->action_u.slacl_p2_slacl_p2.id088 = data;
        break;
    case 89:
        table->action_u.slacl_p2_slacl_p2.id089 = data;
        break;
    case 90:
        table->action_u.slacl_p2_slacl_p2.id090 = data;
        break;
    case 91:
        table->action_u.slacl_p2_slacl_p2.id091 = data;
        break;
    case 92:
        table->action_u.slacl_p2_slacl_p2.id092 = data;
        break;
    case 93:
        table->action_u.slacl_p2_slacl_p2.id093 = data;
        break;
    case 94:
        table->action_u.slacl_p2_slacl_p2.id094 = data;
        break;
    case 95:
        table->action_u.slacl_p2_slacl_p2.id095 = data;
        break;
    case 96:
        table->action_u.slacl_p2_slacl_p2.id096 = data;
        break;
    case 97:
        table->action_u.slacl_p2_slacl_p2.id097 = data;
        break;
    case 98:
        table->action_u.slacl_p2_slacl_p2.id098 = data;
        break;
    case 99:
        table->action_u.slacl_p2_slacl_p2.id099 = data;
        break;
    case 100:
        table->action_u.slacl_p2_slacl_p2.id100 = data;
        break;
    case 101:
        table->action_u.slacl_p2_slacl_p2.id101 = data;
        break;
    case 102:
        table->action_u.slacl_p2_slacl_p2.id102 = data;
        break;
    case 103:
        table->action_u.slacl_p2_slacl_p2.id103 = data;
        break;
    case 104:
        table->action_u.slacl_p2_slacl_p2.id104 = data;
        break;
    case 105:
        table->action_u.slacl_p2_slacl_p2.id105 = data;
        break;
    case 106:
        table->action_u.slacl_p2_slacl_p2.id106 = data;
        break;
    case 107:
        table->action_u.slacl_p2_slacl_p2.id107 = data;
        break;
    case 108:
        table->action_u.slacl_p2_slacl_p2.id108 = data;
        break;
    case 109:
        table->action_u.slacl_p2_slacl_p2.id109 = data;
        break;
    case 110:
        table->action_u.slacl_p2_slacl_p2.id110 = data;
        break;
    case 111:
        table->action_u.slacl_p2_slacl_p2.id111 = data;
        break;
    case 112:
        table->action_u.slacl_p2_slacl_p2.id112 = data;
        break;
    case 113:
        table->action_u.slacl_p2_slacl_p2.id113 = data;
        break;
    case 114:
        table->action_u.slacl_p2_slacl_p2.id114 = data;
        break;
    case 115:
        table->action_u.slacl_p2_slacl_p2.id115 = data;
        break;
    case 116:
        table->action_u.slacl_p2_slacl_p2.id116 = data;
        break;
    case 117:
        table->action_u.slacl_p2_slacl_p2.id117 = data;
        break;
    case 118:
        table->action_u.slacl_p2_slacl_p2.id118 = data;
        break;
    case 119:
        table->action_u.slacl_p2_slacl_p2.id119 = data;
        break;
    case 120:
        table->action_u.slacl_p2_slacl_p2.id120 = data;
        break;
    case 121:
        table->action_u.slacl_p2_slacl_p2.id121 = data;
        break;
    case 122:
        table->action_u.slacl_p2_slacl_p2.id122 = data;
        break;
    case 123:
        table->action_u.slacl_p2_slacl_p2.id123 = data;
        break;
    case 124:
        table->action_u.slacl_p2_slacl_p2.id124 = data;
        break;
    case 125:
        table->action_u.slacl_p2_slacl_p2.id125 = data;
        break;
    case 126:
        table->action_u.slacl_p2_slacl_p2.id126 = data;
        break;
    case 127:
        table->action_u.slacl_p2_slacl_p2.id127 = data;
        break;
    case 128:
        table->action_u.slacl_p2_slacl_p2.id128 = data;
        break;
    case 129:
        table->action_u.slacl_p2_slacl_p2.id129 = data;
        break;
    case 130:
        table->action_u.slacl_p2_slacl_p2.id130 = data;
        break;
    case 131:
        table->action_u.slacl_p2_slacl_p2.id131 = data;
        break;
    case 132:
        table->action_u.slacl_p2_slacl_p2.id132 = data;
        break;
    case 133:
        table->action_u.slacl_p2_slacl_p2.id133 = data;
        break;
    case 134:
        table->action_u.slacl_p2_slacl_p2.id134 = data;
        break;
    case 135:
        table->action_u.slacl_p2_slacl_p2.id135 = data;
        break;
    case 136:
        table->action_u.slacl_p2_slacl_p2.id136 = data;
        break;
    case 137:
        table->action_u.slacl_p2_slacl_p2.id137 = data;
        break;
    case 138:
        table->action_u.slacl_p2_slacl_p2.id138 = data;
        break;
    case 139:
        table->action_u.slacl_p2_slacl_p2.id139 = data;
        break;
    case 140:
        table->action_u.slacl_p2_slacl_p2.id140 = data;
        break;
    case 141:
        table->action_u.slacl_p2_slacl_p2.id141 = data;
        break;
    case 142:
        table->action_u.slacl_p2_slacl_p2.id142 = data;
        break;
    case 143:
        table->action_u.slacl_p2_slacl_p2.id143 = data;
        break;
    case 144:
        table->action_u.slacl_p2_slacl_p2.id144 = data;
        break;
    case 145:
        table->action_u.slacl_p2_slacl_p2.id145 = data;
        break;
    case 146:
        table->action_u.slacl_p2_slacl_p2.id146 = data;
        break;
    case 147:
        table->action_u.slacl_p2_slacl_p2.id147 = data;
        break;
    case 148:
        table->action_u.slacl_p2_slacl_p2.id148 = data;
        break;
    case 149:
        table->action_u.slacl_p2_slacl_p2.id149 = data;
        break;
    case 150:
        table->action_u.slacl_p2_slacl_p2.id150 = data;
        break;
    case 151:
        table->action_u.slacl_p2_slacl_p2.id151 = data;
        break;
    case 152:
        table->action_u.slacl_p2_slacl_p2.id152 = data;
        break;
    case 153:
        table->action_u.slacl_p2_slacl_p2.id153 = data;
        break;
    case 154:
        table->action_u.slacl_p2_slacl_p2.id154 = data;
        break;
    case 155:
        table->action_u.slacl_p2_slacl_p2.id155 = data;
        break;
    case 156:
        table->action_u.slacl_p2_slacl_p2.id156 = data;
        break;
    case 157:
        table->action_u.slacl_p2_slacl_p2.id157 = data;
        break;
    case 158:
        table->action_u.slacl_p2_slacl_p2.id158 = data;
        break;
    case 159:
        table->action_u.slacl_p2_slacl_p2.id159 = data;
        break;
    case 160:
        table->action_u.slacl_p2_slacl_p2.id160 = data;
        break;
    case 161:
        table->action_u.slacl_p2_slacl_p2.id161 = data;
        break;
    case 162:
        table->action_u.slacl_p2_slacl_p2.id162 = data;
        break;
    case 163:
        table->action_u.slacl_p2_slacl_p2.id163 = data;
        break;
    case 164:
        table->action_u.slacl_p2_slacl_p2.id164 = data;
        break;
    case 165:
        table->action_u.slacl_p2_slacl_p2.id165 = data;
        break;
    case 166:
        table->action_u.slacl_p2_slacl_p2.id166 = data;
        break;
    case 167:
        table->action_u.slacl_p2_slacl_p2.id167 = data;
        break;
    case 168:
        table->action_u.slacl_p2_slacl_p2.id168 = data;
        break;
    case 169:
        table->action_u.slacl_p2_slacl_p2.id169 = data;
        break;
    case 170:
        table->action_u.slacl_p2_slacl_p2.id170 = data;
        break;
    case 171:
        table->action_u.slacl_p2_slacl_p2.id171 = data;
        break;
    case 172:
        table->action_u.slacl_p2_slacl_p2.id172 = data;
        break;
    case 173:
        table->action_u.slacl_p2_slacl_p2.id173 = data;
        break;
    case 174:
        table->action_u.slacl_p2_slacl_p2.id174 = data;
        break;
    case 175:
        table->action_u.slacl_p2_slacl_p2.id175 = data;
        break;
    case 176:
        table->action_u.slacl_p2_slacl_p2.id176 = data;
        break;
    case 177:
        table->action_u.slacl_p2_slacl_p2.id177 = data;
        break;
    case 178:
        table->action_u.slacl_p2_slacl_p2.id178 = data;
        break;
    case 179:
        table->action_u.slacl_p2_slacl_p2.id179 = data;
        break;
    case 180:
        table->action_u.slacl_p2_slacl_p2.id180 = data;
        break;
    case 181:
        table->action_u.slacl_p2_slacl_p2.id181 = data;
        break;
    case 182:
        table->action_u.slacl_p2_slacl_p2.id182 = data;
        break;
    case 183:
        table->action_u.slacl_p2_slacl_p2.id183 = data;
        break;
    case 184:
        table->action_u.slacl_p2_slacl_p2.id184 = data;
        break;
    case 185:
        table->action_u.slacl_p2_slacl_p2.id185 = data;
        break;
    case 186:
        table->action_u.slacl_p2_slacl_p2.id186 = data;
        break;
    case 187:
        table->action_u.slacl_p2_slacl_p2.id187 = data;
        break;
    case 188:
        table->action_u.slacl_p2_slacl_p2.id188 = data;
        break;
    case 189:
        table->action_u.slacl_p2_slacl_p2.id189 = data;
        break;
    case 190:
        table->action_u.slacl_p2_slacl_p2.id190 = data;
        break;
    case 191:
        table->action_u.slacl_p2_slacl_p2.id191 = data;
        break;
    case 192:
        table->action_u.slacl_p2_slacl_p2.id192 = data;
        break;
    case 193:
        table->action_u.slacl_p2_slacl_p2.id193 = data;
        break;
    case 194:
        table->action_u.slacl_p2_slacl_p2.id194 = data;
        break;
    case 195:
        table->action_u.slacl_p2_slacl_p2.id195 = data;
        break;
    case 196:
        table->action_u.slacl_p2_slacl_p2.id196 = data;
        break;
    case 197:
        table->action_u.slacl_p2_slacl_p2.id197 = data;
        break;
    case 198:
        table->action_u.slacl_p2_slacl_p2.id198 = data;
        break;
    case 199:
        table->action_u.slacl_p2_slacl_p2.id199 = data;
        break;
    case 200:
        table->action_u.slacl_p2_slacl_p2.id200 = data;
        break;
    case 201:
        table->action_u.slacl_p2_slacl_p2.id201 = data;
        break;
    case 202:
        table->action_u.slacl_p2_slacl_p2.id202 = data;
        break;
    case 203:
        table->action_u.slacl_p2_slacl_p2.id203 = data;
        break;
    case 204:
        table->action_u.slacl_p2_slacl_p2.id204 = data;
        break;
    case 205:
        table->action_u.slacl_p2_slacl_p2.id205 = data;
        break;
    case 206:
        table->action_u.slacl_p2_slacl_p2.id206 = data;
        break;
    case 207:
        table->action_u.slacl_p2_slacl_p2.id207 = data;
        break;
    case 208:
        table->action_u.slacl_p2_slacl_p2.id208 = data;
        break;
    case 209:
        table->action_u.slacl_p2_slacl_p2.id209 = data;
        break;
    case 210:
        table->action_u.slacl_p2_slacl_p2.id210 = data;
        break;
    case 211:
        table->action_u.slacl_p2_slacl_p2.id211 = data;
        break;
    case 212:
        table->action_u.slacl_p2_slacl_p2.id212 = data;
        break;
    case 213:
        table->action_u.slacl_p2_slacl_p2.id213 = data;
        break;
    case 214:
        table->action_u.slacl_p2_slacl_p2.id214 = data;
        break;
    case 215:
        table->action_u.slacl_p2_slacl_p2.id215 = data;
        break;
    case 216:
        table->action_u.slacl_p2_slacl_p2.id216 = data;
        break;
    case 217:
        table->action_u.slacl_p2_slacl_p2.id217 = data;
        break;
    case 218:
        table->action_u.slacl_p2_slacl_p2.id218 = data;
        break;
    case 219:
        table->action_u.slacl_p2_slacl_p2.id219 = data;
        break;
    case 220:
        table->action_u.slacl_p2_slacl_p2.id220 = data;
        break;
    case 221:
        table->action_u.slacl_p2_slacl_p2.id221 = data;
        break;
    case 222:
        table->action_u.slacl_p2_slacl_p2.id222 = data;
        break;
    case 223:
        table->action_u.slacl_p2_slacl_p2.id223 = data;
        break;
    case 224:
        table->action_u.slacl_p2_slacl_p2.id224 = data;
        break;
    case 225:
        table->action_u.slacl_p2_slacl_p2.id225 = data;
        break;
    case 226:
        table->action_u.slacl_p2_slacl_p2.id226 = data;
        break;
    case 227:
        table->action_u.slacl_p2_slacl_p2.id227 = data;
        break;
    case 228:
        table->action_u.slacl_p2_slacl_p2.id228 = data;
        break;
    case 229:
        table->action_u.slacl_p2_slacl_p2.id229 = data;
        break;
    case 230:
        table->action_u.slacl_p2_slacl_p2.id230 = data;
        break;
    case 231:
        table->action_u.slacl_p2_slacl_p2.id231 = data;
        break;
    case 232:
        table->action_u.slacl_p2_slacl_p2.id232 = data;
        break;
    case 233:
        table->action_u.slacl_p2_slacl_p2.id233 = data;
        break;
    case 234:
        table->action_u.slacl_p2_slacl_p2.id234 = data;
        break;
    case 235:
        table->action_u.slacl_p2_slacl_p2.id235 = data;
        break;
    case 236:
        table->action_u.slacl_p2_slacl_p2.id236 = data;
        break;
    case 237:
        table->action_u.slacl_p2_slacl_p2.id237 = data;
        break;
    case 238:
        table->action_u.slacl_p2_slacl_p2.id238 = data;
        break;
    case 239:
        table->action_u.slacl_p2_slacl_p2.id239 = data;
        break;
    case 240:
        table->action_u.slacl_p2_slacl_p2.id240 = data;
        break;
    case 241:
        table->action_u.slacl_p2_slacl_p2.id241 = data;
        break;
    case 242:
        table->action_u.slacl_p2_slacl_p2.id242 = data;
        break;
    case 243:
        table->action_u.slacl_p2_slacl_p2.id243 = data;
        break;
    case 244:
        table->action_u.slacl_p2_slacl_p2.id244 = data;
        break;
    case 245:
        table->action_u.slacl_p2_slacl_p2.id245 = data;
        break;
    case 246:
        table->action_u.slacl_p2_slacl_p2.id246 = data;
        break;
    case 247:
        table->action_u.slacl_p2_slacl_p2.id247 = data;
        break;
    case 248:
        table->action_u.slacl_p2_slacl_p2.id248 = data;
        break;
    case 249:
        table->action_u.slacl_p2_slacl_p2.id249 = data;
        break;
    case 250:
        table->action_u.slacl_p2_slacl_p2.id250 = data;
        break;
    case 251:
        table->action_u.slacl_p2_slacl_p2.id251 = data;
        break;
    case 252:
        table->action_u.slacl_p2_slacl_p2.id252 = data;
        break;
    case 253:
        table->action_u.slacl_p2_slacl_p2.id253 = data;
        break;
    case 254:
        table->action_u.slacl_p2_slacl_p2.id254 = data;
        break;
    case 255:
        table->action_u.slacl_p2_slacl_p2.id255 = data;
        break;

    default:
        break;
    }

    return SDK_RET_OK;
}

inline sdk_ret_t
slacl_p2_write_stage_table (mem_addr_t addr, uint8_t *bytes)
{
    return lpm_write_rxdma_table(addr,
                                 P4_APOLLO_RXDMA_TBL_ID_SLACL_P2,
                                 SLACL_P2_SLACL_P2_ID,
                                 bytes);
}
#endif
