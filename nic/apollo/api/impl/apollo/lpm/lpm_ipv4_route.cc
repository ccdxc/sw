/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    lpm_ipv4_route.cc
 *
 * @brief   LPM IPv4 Route implementation
 */

#include <math.h>
#include "nic/apollo/api/impl/apollo/lpm/lpm_ipv4_route.hpp"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/framework/pipeline_impl_base.hpp"

sdk_ret_t
lpm_ipv4_route_add_key_to_stage (uint8_t *bytes, uint32_t idx,
                                 lpm_inode_t *lpm_inode)
{
    auto table = (route_actiondata_t *)bytes;

    switch (idx) {
    case 0:
        table->action_u.route_search_ipv4.key0 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 1:
        table->action_u.route_search_ipv4.key1 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 2:
        table->action_u.route_search_ipv4.key2 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 3:
        table->action_u.route_search_ipv4.key3 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 4:
        table->action_u.route_search_ipv4.key4 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 5:
        table->action_u.route_search_ipv4.key5 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 6:
        table->action_u.route_search_ipv4.key6 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 7:
        table->action_u.route_search_ipv4.key7 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 8:
        table->action_u.route_search_ipv4.key8 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 9:
        table->action_u.route_search_ipv4.key9 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 10:
        table->action_u.route_search_ipv4.key10 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 11:
        table->action_u.route_search_ipv4.key11 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 12:
        table->action_u.route_search_ipv4.key12 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 13:
        table->action_u.route_search_ipv4.key13 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 14:
        table->action_u.route_search_ipv4.key14 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    default:
        break;
    }

    return SDK_RET_OK;
}

sdk_ret_t
lpm_ipv4_route_add_key_to_last_stage (uint8_t *bytes, uint32_t idx,
                                      lpm_inode_t *lpm_inode)
{
    auto table = (route_actiondata_t *)bytes;

    switch (idx) {
    case 0:
        table->action_u.route_search_ipv4_retrieve.key0 =
                lpm_inode->ipaddr.addr.v4_addr;
        table->action_u.route_search_ipv4_retrieve.data0 =
                (uint16_t)lpm_inode->data;
        break;
    case 1:
        table->action_u.route_search_ipv4_retrieve.key1 =
                lpm_inode->ipaddr.addr.v4_addr;
        table->action_u.route_search_ipv4_retrieve.data1 =
                (uint16_t)lpm_inode->data;
        break;
    case 2:
        table->action_u.route_search_ipv4_retrieve.key2 =
                lpm_inode->ipaddr.addr.v4_addr;
        table->action_u.route_search_ipv4_retrieve.data2 =
                (uint16_t)lpm_inode->data;
        break;
    case 3:
        table->action_u.route_search_ipv4_retrieve.key3 =
                lpm_inode->ipaddr.addr.v4_addr;
        table->action_u.route_search_ipv4_retrieve.data3 =
                (uint16_t)lpm_inode->data;
        break;
    case 4:
        table->action_u.route_search_ipv4_retrieve.key4 =
                lpm_inode->ipaddr.addr.v4_addr;
        table->action_u.route_search_ipv4_retrieve.data4 =
                (uint16_t)lpm_inode->data;
        break;
    case 5:
        table->action_u.route_search_ipv4_retrieve.key5 =
                lpm_inode->ipaddr.addr.v4_addr;
        table->action_u.route_search_ipv4_retrieve.data5 =
                (uint16_t)lpm_inode->data;
        break;
    case 6:
        table->action_u.route_search_ipv4_retrieve.key6 =
                lpm_inode->ipaddr.addr.v4_addr;
        table->action_u.route_search_ipv4_retrieve.data6 =
                (uint16_t)lpm_inode->data;
        break;
    default:
        break;
    }

    return SDK_RET_OK;
}

sdk_ret_t
lpm_ipv4_route_set_default_data (uint8_t *bytes, uint32_t default_data)
{
    auto table = (route_actiondata_t *) bytes;
    table->action_u.route_search_ipv4_retrieve.data_ =
            (uint16_t)default_data;
    return SDK_RET_OK;
}

sdk_ret_t
lpm_ipv4_route_write_stage_table (mem_addr_t addr, uint8_t *bytes)
{
    auto table = (route_actiondata_t *) bytes;
    table->action_id = ROUTE_SEARCH_IPV4_ID;
    return impl_base::pipeline_impl()->write_to_txdma_table(addr,
               P4_P4PLUS_TXDMA_TBL_ID_ROUTE,
               ROUTE_SEARCH_IPV4_ID, bytes);
}

sdk_ret_t
lpm_ipv4_route_write_last_stage_table (mem_addr_t addr, uint8_t *bytes)
{
    auto table = (route_actiondata_t *) bytes;
    table->action_id = ROUTE_SEARCH_IPV4_RETRIEVE_ID;
    return impl_base::pipeline_impl()->write_to_txdma_table(addr,
               P4_P4PLUS_TXDMA_TBL_ID_ROUTE,
               ROUTE_SEARCH_IPV4_RETRIEVE_ID, bytes);
}

/**
 * key size is 4 bytes for IPv4 route
 */
uint32_t
lpm_ipv4_route_key_size (void)
{
    return LPM_IPV4_ROUTE_KEY_SIZE;
}

/**
 * @brief    compute the number of stages needed for LPM lookup given the
 *           interval table scale
 * @param[in]    num_intrvls   number of intervals in the interval table
 * @return       number of lookup stages (aka. depth of the interval tree)
 *
 * The computation is done as follows for IPv4 :
 *     The last stage gives an 8-way decision. The other stages each give
 *     a 16-way decision.
 *     #stages = 1 + log16(num_intrvls/8.0)
 *             = 1 + log2(num_intrvls/8.0)/log2(16)
 *             = 1 + log2(num_intrvls/8.0)/4.0
 */
uint32_t
lpm_ipv4_route_stages (uint32_t num_intrvls)
{
    // 1 * 8-way last stage, plus (n-1) * 16-way stages
    return (1 + ((uint32_t)ceil(log2f((float)(num_intrvls/8.0))/4.0)));
}
