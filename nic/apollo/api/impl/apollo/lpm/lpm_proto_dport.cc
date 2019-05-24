/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    lpm_proto_dport.cc
 *
 * @brief   LPM Protocol + Dest Port implementation
 */

#include <math.h>
#include "nic/apollo/lpm/lpm.hpp"
#include "nic/apollo/lpm/lpm_proto_dport.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/framework/pipeline_impl_base.hpp"
#include "gen/p4gen/apollo_rxdma/include/apollo_rxdma_p4pd.h"

sdk_ret_t
lpm_proto_dport_add_key_to_stage (uint8_t *bytes, uint32_t idx,
                                  lpm_inode_t *lpm_inode)
{
    auto table = (sacl_proto_dport_keys_actiondata_t *)bytes;

    switch (idx) {
    case 0:
        table->action_u.sacl_proto_dport_keys_match_proto_dport.key0 =
            lpm_inode->key32;
        break;
    case 1:
        table->action_u.sacl_proto_dport_keys_match_proto_dport.key1 =
            lpm_inode->key32;
        break;
    case 2:
        table->action_u.sacl_proto_dport_keys_match_proto_dport.key2 =
            lpm_inode->key32;
        break;
    case 3:
        table->action_u.sacl_proto_dport_keys_match_proto_dport.key3 =
            lpm_inode->key32;
        break;
    case 4:
        table->action_u.sacl_proto_dport_keys_match_proto_dport.key4 =
            lpm_inode->key32;
        break;
    case 5:
        table->action_u.sacl_proto_dport_keys_match_proto_dport.key5 =
            lpm_inode->key32;
        break;
    case 6:
        table->action_u.sacl_proto_dport_keys_match_proto_dport.key6 =
            lpm_inode->key32;
        break;
    case 7:
        table->action_u.sacl_proto_dport_keys_match_proto_dport.key7 =
            lpm_inode->key32;
        break;
    case 8:
        table->action_u.sacl_proto_dport_keys_match_proto_dport.key8 =
            lpm_inode->key32;
        break;
    case 9:
        table->action_u.sacl_proto_dport_keys_match_proto_dport.key9 =
            lpm_inode->key32;
        break;
    case 10:
        table->action_u.sacl_proto_dport_keys_match_proto_dport.key10 =
            lpm_inode->key32;
        break;
    case 11:
        table->action_u.sacl_proto_dport_keys_match_proto_dport.key11 =
            lpm_inode->key32;
        break;
    case 12:
        table->action_u.sacl_proto_dport_keys_match_proto_dport.key12 =
            lpm_inode->key32;
        break;
    case 13:
        table->action_u.sacl_proto_dport_keys_match_proto_dport.key13 =
            lpm_inode->key32;
        break;
    case 14:
        table->action_u.sacl_proto_dport_keys_match_proto_dport.key14 =
            lpm_inode->key32;
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
    auto table = (sacl_proto_dport_data_actiondata_t *)bytes;

    switch (idx) {
    case 0:
        table->action_u.sacl_proto_dport_data_match_proto_dport_retrieve.key0 =
            lpm_inode->key32;
        table->action_u.sacl_proto_dport_data_match_proto_dport_retrieve.data0 =
            (uint16_t)lpm_inode->data;
        break;
    case 1:
        table->action_u.sacl_proto_dport_data_match_proto_dport_retrieve.key1 =
            lpm_inode->key32;
        table->action_u.sacl_proto_dport_data_match_proto_dport_retrieve.data1 =
            (uint16_t)lpm_inode->data;
        break;
    case 2:
        table->action_u.sacl_proto_dport_data_match_proto_dport_retrieve.key2 =
            lpm_inode->key32;
        table->action_u.sacl_proto_dport_data_match_proto_dport_retrieve.data2 =
            (uint16_t)lpm_inode->data;
        break;
    case 3:
        table->action_u.sacl_proto_dport_data_match_proto_dport_retrieve.key3 =
            lpm_inode->key32;
        table->action_u.sacl_proto_dport_data_match_proto_dport_retrieve.data3 =
            (uint16_t)lpm_inode->data;
        break;
    case 4:
        table->action_u.sacl_proto_dport_data_match_proto_dport_retrieve.key4 =
            lpm_inode->key32;
        table->action_u.sacl_proto_dport_data_match_proto_dport_retrieve.data4 =
            (uint16_t)lpm_inode->data;
        break;
    case 5:
        table->action_u.sacl_proto_dport_data_match_proto_dport_retrieve.key5 =
            lpm_inode->key32;
        table->action_u.sacl_proto_dport_data_match_proto_dport_retrieve.data5 =
            (uint16_t)lpm_inode->data;
        break;
    case 6:
        table->action_u.sacl_proto_dport_data_match_proto_dport_retrieve.key6 =
            lpm_inode->key32;
        table->action_u.sacl_proto_dport_data_match_proto_dport_retrieve.data6 =
            (uint16_t)lpm_inode->data;
        break;
    default:
        break;
    }

    return SDK_RET_OK;
}

sdk_ret_t
lpm_proto_dport_set_default_data (uint8_t *bytes, uint32_t default_data)
{
    auto table = (sacl_proto_dport_data_actiondata_t *) bytes;
    table->action_u.sacl_proto_dport_data_match_proto_dport_retrieve.data_ =
            (uint16_t)default_data;
    return SDK_RET_OK;
}

sdk_ret_t
lpm_proto_dport_write_stage_table (mem_addr_t addr, uint8_t *bytes)
{
    return impl_base::pipeline_impl()->write_to_rxdma_table(addr,
               P4_APOLLO_RXDMA_TBL_ID_SACL_PROTO_DPORT_KEYS,
               SACL_PROTO_DPORT_KEYS_MATCH_PROTO_DPORT_ID, bytes);
}

sdk_ret_t
lpm_proto_dport_write_last_stage_table (mem_addr_t addr, uint8_t *bytes)
{
    return impl_base::pipeline_impl()->write_to_rxdma_table(addr,
               P4_APOLLO_RXDMA_TBL_ID_SACL_PROTO_DPORT_DATA,
               SACL_PROTO_DPORT_DATA_MATCH_PROTO_DPORT_RETRIEVE_ID, bytes);
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
