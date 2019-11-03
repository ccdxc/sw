/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    lpm_ipv6_acl.cc
 *
 * @brief   LPM RFC IPv6 implementation
 */

#include <math.h>
#include "lpm_ipv6_acl.hpp"
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd.h"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/framework/pipeline_impl_base.hpp"
#include "nic/sdk/lib/utils/utils.hpp"

sdk_ret_t
lpm_ipv6_acl_add_key_to_stage (uint8_t *bytes, uint32_t idx,
                               lpm_inode_t *lpm_inode)
{
    auto table = (rxlpm1_actiondata_t *)bytes;

    switch (idx) {
    case 0:
        sdk::lib::memrev(table->action_u.rxlpm1_match1_128b.key0,
                         lpm_inode->ipaddr.addr.v6_addr.addr8, IP6_ADDR8_LEN);
        break;
    case 1:
        sdk::lib::memrev(table->action_u.rxlpm1_match1_128b.key1,
                         lpm_inode->ipaddr.addr.v6_addr.addr8, IP6_ADDR8_LEN);
        break;
    case 2:
        sdk::lib::memrev(table->action_u.rxlpm1_match1_128b.key2,
                         lpm_inode->ipaddr.addr.v6_addr.addr8, IP6_ADDR8_LEN);
        break;
    default:
        break;
    }
    return SDK_RET_OK;
}

sdk_ret_t
lpm_ipv6_acl_add_key_to_last_stage (uint8_t *bytes, uint32_t idx,
                                    lpm_inode_t *lpm_inode)
{
    auto table = (rxlpm1_actiondata_t *)bytes;

    switch (idx) {
    case 0:
        sdk::lib::memrev(table->action_u.rxlpm1_match1_128b_retrieve.key0,
                         lpm_inode->ipaddr.addr.v6_addr.addr8, IP6_ADDR8_LEN);
        table->action_u.rxlpm1_match1_128b_retrieve.data0 =
            (uint16_t)lpm_inode->data;
        break;
    default:
        break;
    }
    return SDK_RET_OK;
}

sdk_ret_t
lpm_ipv6_acl_set_default_data (uint8_t *bytes, uint32_t default_data)
{
    auto table = (rxlpm1_actiondata_t *) bytes;
    table->action_u.rxlpm1_match1_128b_retrieve.data_ =
            (uint16_t)default_data;
    return SDK_RET_OK;
}

sdk_ret_t
lpm_ipv6_acl_write_stage_table (mem_addr_t addr, uint8_t *bytes)
{
    return impl_base::pipeline_impl()->write_to_rxdma_table(addr,
               P4_P4PLUS_RXDMA_TBL_ID_RXLPM1,
               RXLPM1_MATCH1_128B_ID, bytes);
}

sdk_ret_t
lpm_ipv6_acl_write_last_stage_table (mem_addr_t addr, uint8_t *bytes)
{
    return impl_base::pipeline_impl()->write_to_rxdma_table(addr,
               P4_P4PLUS_RXDMA_TBL_ID_RXLPM1,
               RXLPM1_MATCH1_128B_RETRIEVE_ID, bytes);
}

/**
 * key size is 16 bytes for IPv6 sacl
 */
uint32_t
lpm_ipv6_acl_key_size (void)
{
    return LPM_IPV6_ACL_KEY_SIZE;
}

/**
 * @brief    compute the number of stages needed for LPM lookup given the
 *           interval table scale
 * @param[in]    num_intrvls   number of intervals in the interval table
 * @return       number of lookup stages (aka. depth of the interval tree)
 *
 * The computation is done as follows for IPv6 Acl (128b key):
 *     The last stage gives an 2-way decision. The other stages each give
 *     a 4-way decision.
 *     #stages = 1 + log4(num_intrvls/2.0)
 *             = 1 + log2(num_intrvls/2.0)/log2(4)
 *             = 1 + log2(num_intrvls/2.0)/2.0
 */
uint32_t
lpm_ipv6_acl_stages (uint32_t num_intrvls)
{
    // 1 * 2-way last stage, plus (n-1) * 4-way stages
    return (1 + ((uint32_t)ceil(log2f((float)(num_intrvls/2.0))/2.0)));
}
