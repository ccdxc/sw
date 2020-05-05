/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    lpm_ipv4_route.cc
 *
 * @brief   LPM IPv4 Route implementation
 */

#include <math.h>
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/impl/lpm/lpm.hpp"
#include "lpm_ipv4_route.hpp"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/framework/pipeline_impl_base.hpp"

sdk_ret_t
lpm_ipv4_route_add_key_to_stage (uint8_t *bytes, uint32_t idx,
                                 lpm_inode_t *lpm_inode)
{
    auto table = (txlpm1_actiondata_t *)bytes;

    switch (idx) {
    case 0:
        table->action_u.txlpm1_match1_32b.key0 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 1:
        table->action_u.txlpm1_match1_32b.key1 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 2:
        table->action_u.txlpm1_match1_32b.key2 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 3:
        table->action_u.txlpm1_match1_32b.key3 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 4:
        table->action_u.txlpm1_match1_32b.key4 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 5:
        table->action_u.txlpm1_match1_32b.key5 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 6:
        table->action_u.txlpm1_match1_32b.key6 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 7:
        table->action_u.txlpm1_match1_32b.key7 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 8:
        table->action_u.txlpm1_match1_32b.key8 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 9:
        table->action_u.txlpm1_match1_32b.key9 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 10:
        table->action_u.txlpm1_match1_32b.key10 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 11:
        table->action_u.txlpm1_match1_32b.key11 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 12:
        table->action_u.txlpm1_match1_32b.key12 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 13:
        table->action_u.txlpm1_match1_32b.key13 = lpm_inode->ipaddr.addr.v4_addr;
        break;
    case 14:
        table->action_u.txlpm1_match1_32b.key14 = lpm_inode->ipaddr.addr.v4_addr;
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
    auto table = (txlpm1_actiondata_t *)bytes;

    switch (idx) {
    case 0:
        table->action_u.txlpm1_match1_32b_retrieve.key0 =
                lpm_inode->ipaddr.addr.v4_addr;
        table->action_u.txlpm1_match1_32b_retrieve.data0 =
                lpm_inode->data;
        break;
    case 1:
        table->action_u.txlpm1_match1_32b_retrieve.key1 =
                lpm_inode->ipaddr.addr.v4_addr;
        table->action_u.txlpm1_match1_32b_retrieve.data1 =
                lpm_inode->data;
        break;
    case 2:
        table->action_u.txlpm1_match1_32b_retrieve.key2 =
                lpm_inode->ipaddr.addr.v4_addr;
        table->action_u.txlpm1_match1_32b_retrieve.data2 =
                lpm_inode->data;
        break;
    case 3:
        table->action_u.txlpm1_match1_32b_retrieve.key3 =
                lpm_inode->ipaddr.addr.v4_addr;
        table->action_u.txlpm1_match1_32b_retrieve.data3 =
                lpm_inode->data;
        break;
    case 4:
        table->action_u.txlpm1_match1_32b_retrieve.key4 =
                lpm_inode->ipaddr.addr.v4_addr;
        table->action_u.txlpm1_match1_32b_retrieve.data4 =
                lpm_inode->data;
        break;
    case 5:
        table->action_u.txlpm1_match1_32b_retrieve.key5 =
                lpm_inode->ipaddr.addr.v4_addr;
        table->action_u.txlpm1_match1_32b_retrieve.data5 =
                lpm_inode->data;
        break;
    case 6:
        table->action_u.txlpm1_match1_32b_retrieve.key6 =
                lpm_inode->ipaddr.addr.v4_addr;
        table->action_u.txlpm1_match1_32b_retrieve.data6 =
                lpm_inode->data;
        break;
    default:
        break;
    }

    return SDK_RET_OK;
}

sdk_ret_t
lpm_ipv4_route_set_default_data (uint8_t *bytes, uint32_t default_data)
{
    auto table = (txlpm1_actiondata_t *) bytes;
    table->action_u.txlpm1_match1_32b_retrieve.data_ = default_data;
    return SDK_RET_OK;
}

sdk_ret_t
lpm_ipv4_route_write_stage_table (mem_addr_t addr, uint8_t *bytes)
{
    auto table = (txlpm1_actiondata_t *) bytes;
    table->action_id = TXLPM1_MATCH1_32B_ID;
    PDS_TRACE_VERBOSE("LPM writing HBM stage: 0x%0x, "
                      "0x%0x 0x%0x 0x%0x 0x%0x 0x%0x "
                      "0x%0x 0x%0x 0x%0x 0x%0x 0x%0x "
                      "0x%0x 0x%0x 0x%0x 0x%0x 0x%0x ",
                      addr,
                      table->action_u.txlpm1_match1_32b.key0,
                      table->action_u.txlpm1_match1_32b.key1,
                      table->action_u.txlpm1_match1_32b.key2,
                      table->action_u.txlpm1_match1_32b.key3,
                      table->action_u.txlpm1_match1_32b.key4,
                      table->action_u.txlpm1_match1_32b.key5,
                      table->action_u.txlpm1_match1_32b.key6,
                      table->action_u.txlpm1_match1_32b.key7,
                      table->action_u.txlpm1_match1_32b.key8,
                      table->action_u.txlpm1_match1_32b.key9,
                      table->action_u.txlpm1_match1_32b.key10,
                      table->action_u.txlpm1_match1_32b.key11,
                      table->action_u.txlpm1_match1_32b.key12,
                      table->action_u.txlpm1_match1_32b.key13,
                      table->action_u.txlpm1_match1_32b.key14);
    return impl_base::pipeline_impl()->write_to_txdma_table(addr,
               P4_P4PLUS_TXDMA_TBL_ID_TXLPM1,
               TXLPM1_MATCH1_32B_ID, bytes);
}

sdk_ret_t
lpm_ipv4_route_write_last_stage_table (mem_addr_t addr, uint8_t *bytes)
{
    auto table = (txlpm1_actiondata_t *) bytes;
    table->action_id = TXLPM1_MATCH1_32B_RETRIEVE_ID;
    PDS_TRACE_DEBUG("LPM writing HBM last stage: 0x%0x, "
                    "0x%0x:0x%0x 0x%0x:0x%0x 0x%0x:0x%0x "
                    "0x%0x:0x%0x 0x%0x:0x%0x 0x%0x:0x%0x "
                    "0x%0x:0x%0x 0x%0x ",
                    addr,
                    table->action_u.txlpm1_match1_32b_retrieve.key0,
                    table->action_u.txlpm1_match1_32b_retrieve.data0,
                    table->action_u.txlpm1_match1_32b_retrieve.key1,
                    table->action_u.txlpm1_match1_32b_retrieve.data1,
                    table->action_u.txlpm1_match1_32b_retrieve.key2,
                    table->action_u.txlpm1_match1_32b_retrieve.data2,
                    table->action_u.txlpm1_match1_32b_retrieve.key3,
                    table->action_u.txlpm1_match1_32b_retrieve.data3,
                    table->action_u.txlpm1_match1_32b_retrieve.key4,
                    table->action_u.txlpm1_match1_32b_retrieve.data4,
                    table->action_u.txlpm1_match1_32b_retrieve.key5,
                    table->action_u.txlpm1_match1_32b_retrieve.data5,
                    table->action_u.txlpm1_match1_32b_retrieve.key6,
                    table->action_u.txlpm1_match1_32b_retrieve.data6,
                    table->action_u.txlpm1_match1_32b_retrieve.data_
                    );
    return impl_base::pipeline_impl()->write_to_txdma_table(addr,
               P4_P4PLUS_TXDMA_TBL_ID_TXLPM1,
               TXLPM1_MATCH1_32B_RETRIEVE_ID, bytes);
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
