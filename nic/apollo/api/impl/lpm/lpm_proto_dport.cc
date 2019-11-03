/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    lpm_proto_dport.cc
 *
 * @brief   LPM Protocol + Dest Port implementation
 */

#include <math.h>
#include "lpm_proto_dport.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/framework/pipeline_impl_base.hpp"
#include "gen/p4gen/p4plus_rxdma/include/p4plus_rxdma_p4pd.h"

sdk_ret_t
lpm_proto_dport_add_key_to_stage (uint8_t *bytes, uint32_t idx,
                                  lpm_inode_t *lpm_inode)
{
    auto table = (rxlpm2_actiondata_t *)bytes;

    switch (idx) {
    case 0:
        table->action_u.rxlpm2_match2_32b.key0 = lpm_inode->key32;
        break;
    case 1:
        table->action_u.rxlpm2_match2_32b.key1 = lpm_inode->key32;
        break;
    case 2:
        table->action_u.rxlpm2_match2_32b.key2 =  lpm_inode->key32;
        break;
    case 3:
        table->action_u.rxlpm2_match2_32b.key3 = lpm_inode->key32;
        break;
    case 4:
        table->action_u.rxlpm2_match2_32b.key4 = lpm_inode->key32;
        break;
    case 5:
        table->action_u.rxlpm2_match2_32b.key5 = lpm_inode->key32;
        break;
    case 6:
        table->action_u.rxlpm2_match2_32b.key6 = lpm_inode->key32;
        break;
    case 7:
        table->action_u.rxlpm2_match2_32b.key7 = lpm_inode->key32;
        break;
    case 8:
        table->action_u.rxlpm2_match2_32b.key8 = lpm_inode->key32;
        break;
    case 9:
        table->action_u.rxlpm2_match2_32b.key9 = lpm_inode->key32;
        break;
    case 10:
        table->action_u.rxlpm2_match2_32b.key10 = lpm_inode->key32;
        break;
    case 11:
        table->action_u.rxlpm2_match2_32b.key11 = lpm_inode->key32;
        break;
    case 12:
        table->action_u.rxlpm2_match2_32b.key12 = lpm_inode->key32;
        break;
    case 13:
        table->action_u.rxlpm2_match2_32b.key13 = lpm_inode->key32;
        break;
    case 14:
        table->action_u.rxlpm2_match2_32b.key14 = lpm_inode->key32;
            break;
    default:
        break;
    }
    return SDK_RET_OK;
}

sdk_ret_t
lpm_proto_dport_add_key_to_last_stage (uint8_t *bytes, uint32_t idx,
                                       lpm_inode_t *lpm_inode)
{
    auto table = (rxlpm2_actiondata_t *)bytes;

    switch (idx) {
    case 0:
        table->action_u.rxlpm2_match2_32b_retrieve.key0 = lpm_inode->key32;
        table->action_u.rxlpm2_match2_32b_retrieve.data0 = (uint16_t)lpm_inode->data;
        break;
    case 1:
        table->action_u.rxlpm2_match2_32b_retrieve.key1 = lpm_inode->key32;
        table->action_u.rxlpm2_match2_32b_retrieve.data1 = (uint16_t)lpm_inode->data;
        break;
    case 2:
        table->action_u.rxlpm2_match2_32b_retrieve.key2 = lpm_inode->key32;
        table->action_u.rxlpm2_match2_32b_retrieve.data2 = (uint16_t)lpm_inode->data;
        break;
    case 3:
        table->action_u.rxlpm2_match2_32b_retrieve.key3 = lpm_inode->key32;
        table->action_u.rxlpm2_match2_32b_retrieve.data3 = (uint16_t)lpm_inode->data;
        break;
    case 4:
        table->action_u.rxlpm2_match2_32b_retrieve.key4 = lpm_inode->key32;
        table->action_u.rxlpm2_match2_32b_retrieve.data4 = (uint16_t)lpm_inode->data;
        break;
    case 5:
        table->action_u.rxlpm2_match2_32b_retrieve.key5 = lpm_inode->key32;
        table->action_u.rxlpm2_match2_32b_retrieve.data5 = (uint16_t)lpm_inode->data;
        break;
    case 6:
        table->action_u.rxlpm2_match2_32b_retrieve.key6 = lpm_inode->key32;
        table->action_u.rxlpm2_match2_32b_retrieve.data6 = (uint16_t)lpm_inode->data;
        break;
    default:
        break;
    }

    return SDK_RET_OK;
}

sdk_ret_t
lpm_proto_dport_set_default_data (uint8_t *bytes, uint32_t default_data)
{
    auto table = (rxlpm2_actiondata_t *) bytes;
    table->action_u.rxlpm2_match2_32b_retrieve.data_ =
            (uint16_t)default_data;
    return SDK_RET_OK;
}

sdk_ret_t
lpm_proto_dport_write_stage_table (mem_addr_t addr, uint8_t *bytes)
{
    return impl_base::pipeline_impl()->write_to_rxdma_table(addr,
               P4_P4PLUS_RXDMA_TBL_ID_RXLPM2,
               RXLPM2_MATCH2_32B_ID, bytes);
}

sdk_ret_t
lpm_proto_dport_write_last_stage_table (mem_addr_t addr, uint8_t *bytes)
{
    return impl_base::pipeline_impl()->write_to_rxdma_table(addr,
               P4_P4PLUS_RXDMA_TBL_ID_RXLPM2,
               RXLPM2_MATCH2_32B_RETRIEVE_ID, bytes);
}

/**
 * key size is 4 bytes for PROTO + Dest Port
 */
uint32_t
lpm_proto_dport_key_size (void)
{
    return LPM_PROTO_DPORT_KEY_SIZE;
}

/**
 * @brief    compute the number of stages needed for LPM lookup given the
 *           interval table scale
 * @param[in]    num_intrvls   number of intervals in the interval table
 * @return       number of lookup stages (aka. depth of the interval tree)
 *
 * The computation is done as follows for PROTO + Dest Port :
 *     The last stage gives an 8-way decision. The other stages each give
 *     a 16-way decision.
 *     #stages = 1 + log16(num_intrvls/8.0)
 *             = 1 + log2(num_intrvls/8.0)/log2(16)
 *             = 1 + log2(num_intrvls/8.0)/4.0
 */
uint32_t
lpm_proto_dport_stages (uint32_t num_intrvls)
{
    // 1 * 8-way last stage, plus (n-1) * 16-way stages
    return (1 + ((uint32_t)ceil(log2f((float)(num_intrvls/8.0))/4.0)));
}
