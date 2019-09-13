//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// LPM library's common functions across pipelines
///
//----------------------------------------------------------------------------

#include <stack>
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/impl/lpm/lpm.hpp"
#include "nic/apollo/api/impl/lpm/lpm_priv.hpp"
#include "nic/apollo/api/impl/lpm/lpm_impl.hpp"

using std::stack;

/**
 * @brief    compare two given routes and return -1, 0 or 1 based on where they
 *           fit on the number line, breaking the tie by comparing prefix
 *           lengths
 * @param[in]    r1    pointer to 1st route
 * @param[in]    r2    pointer to 2nd route
 * @param[in]    ctxt  opaque context
 * @return  -1, 0, 1 based on the comparison result
 */
static inline int
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

/**
 * @brief    Flushes the partially filled tabled to HW at the end of processing.
 * @param[in] smeta                  meta information for the stages
 * @param[in] nstages                the number of stages
 * @return    SDK_RET_OK on success, failure status code on error
 */
static inline sdk_ret_t
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
static inline sdk_ret_t
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
