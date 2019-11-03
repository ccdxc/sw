/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    lpm_ipv6_peer_route.cc
 *
 * @brief   LPM IPv6 Peer Peer_route implementation
 */

#include <math.h>
#include "nic/apollo/api/impl/lpm/lpm.hpp"
#include "lpm_ipv6_peer_route.hpp"
#include "gen/p4gen/p4plus_txdma/include/p4plus_txdma_p4pd.h"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/framework/pipeline_impl_base.hpp"
#include "nic/sdk/lib/utils/utils.hpp"

sdk_ret_t
lpm_ipv6_peer_route_add_key_to_stage (uint8_t *bytes, uint32_t idx,
                                      lpm_inode_t *lpm_inode)
{
    auto table = (txlpm1_actiondata_t *)bytes;

    switch (idx) {
        case 0:
            sdk::lib::memrev(table->action_u.txlpm1_match1_64b.key0,
                             &lpm_inode->ipaddr.addr.v6_addr.addr8[IP6_ADDR8_LEN/2],
                             IP6_ADDR8_LEN/2);
            break;
        case 1:
            sdk::lib::memrev(table->action_u.txlpm1_match1_64b.key1,
                             &lpm_inode->ipaddr.addr.v6_addr.addr8[IP6_ADDR8_LEN/2],
                             IP6_ADDR8_LEN/2);
            break;
        case 2:
            sdk::lib::memrev(table->action_u.txlpm1_match1_64b.key2,
                             &lpm_inode->ipaddr.addr.v6_addr.addr8[IP6_ADDR8_LEN/2],
                             IP6_ADDR8_LEN/2);
            break;
        case 3:
            sdk::lib::memrev(table->action_u.txlpm1_match1_64b.key3,
                             &lpm_inode->ipaddr.addr.v6_addr.addr8[IP6_ADDR8_LEN/2],
                             IP6_ADDR8_LEN/2);
            break;
        case 4:
            sdk::lib::memrev(table->action_u.txlpm1_match1_64b.key4,
                             &lpm_inode->ipaddr.addr.v6_addr.addr8[IP6_ADDR8_LEN/2],
                             IP6_ADDR8_LEN/2);
            break;
        case 5:
            sdk::lib::memrev(table->action_u.txlpm1_match1_64b.key5,
                             &lpm_inode->ipaddr.addr.v6_addr.addr8[IP6_ADDR8_LEN/2],
                             IP6_ADDR8_LEN/2);
            break;
        case 6:
            sdk::lib::memrev(table->action_u.txlpm1_match1_64b.key6,
                             &lpm_inode->ipaddr.addr.v6_addr.addr8[IP6_ADDR8_LEN/2],
                             IP6_ADDR8_LEN/2);
            break;
        default:
            break;
    }

    return SDK_RET_OK;
}

sdk_ret_t
lpm_ipv6_peer_route_add_key_to_last_stage (uint8_t *bytes, uint32_t idx,
                                           lpm_inode_t *lpm_inode)
{
    auto table = (txlpm1_actiondata_t *)bytes;

    switch (idx) {
        case 0:
            sdk::lib::memrev(table->action_u.txlpm1_match1_64b_retrieve.key0,
                             &lpm_inode->ipaddr.addr.v6_addr.addr8[IP6_ADDR8_LEN/2],
                             IP6_ADDR8_LEN/2);
            table->action_u.txlpm1_match1_64b_retrieve.data0 = lpm_inode->data;
            break;
        case 1:
            sdk::lib::memrev(table->action_u.txlpm1_match1_64b_retrieve.key1,
                             &lpm_inode->ipaddr.addr.v6_addr.addr8[IP6_ADDR8_LEN/2],
                             IP6_ADDR8_LEN/2);
            table->action_u.txlpm1_match1_64b_retrieve.data1 = lpm_inode->data;
            break;
        case 2:
            sdk::lib::memrev(table->action_u.txlpm1_match1_64b_retrieve.key2,
                             &lpm_inode->ipaddr.addr.v6_addr.addr8[IP6_ADDR8_LEN/2],
                             IP6_ADDR8_LEN/2);
            table->action_u.txlpm1_match1_64b_retrieve.data2 = lpm_inode->data;
            break;
        default:
            break;
    }

    return SDK_RET_OK;
}

sdk_ret_t
lpm_ipv6_peer_route_set_default_data (uint8_t *bytes, uint32_t default_data)
{
    auto table = (txlpm1_actiondata_t *) bytes;
    table->action_u.txlpm1_match1_64b_retrieve.data_ = default_data;
    return SDK_RET_OK;
}

sdk_ret_t
lpm_ipv6_peer_route_write_stage_table (mem_addr_t addr, uint8_t *bytes)
{
    return impl_base::pipeline_impl()->write_to_txdma_table(addr,
                                    P4_P4PLUS_TXDMA_TBL_ID_TXLPM1,
                                    TXLPM1_MATCH1_64B_ID, bytes);
}

sdk_ret_t
lpm_ipv6_peer_route_write_last_stage_table (mem_addr_t addr, uint8_t *bytes)
{
    return impl_base::pipeline_impl()->write_to_txdma_table(addr,
                                    P4_P4PLUS_TXDMA_TBL_ID_TXLPM1,
                                    TXLPM1_MATCH1_64B_RETRIEVE_ID, bytes);
}

/**
 * key size is 4 bytes for IPv6 Peer Route
 */
uint32_t
lpm_ipv6_peer_route_key_size ()
{
    return LPM_IPV6_PEER_ROUTE_KEY_SIZE;
}

/**
 * @brief    compute the number of stages needed for LPM lookup given the
 *           interval table scale
 * @param[in]    num_intrvls   number of intervals in the interval table
 * @return       number of lookup stages (aka. depth of the interval tree)
 *
 * The computation is done as follows for IPv6 :
 *     The last stage gives an 4-way decision. The other stages each give
 *     a 8-way decision.
 *     #stages = 1 + log8(num_intrvls/4.0)
 *             = 1 + log2(num_intrvls/4.0)/log2(8)
 *             = 1 + log2(num_intrvls/4.0)/3.0
 */
uint32_t
lpm_ipv6_peer_route_stages (uint32_t num_intrvls)
{
    // 1 * 4-way last stage, plus (n-1) * 8-way stages
    return (1 + ((uint32_t)ceil(log2f((float)(num_intrvls/4.0))/3.0)));
}
