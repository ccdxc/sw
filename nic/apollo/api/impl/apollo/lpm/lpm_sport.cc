/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    lpm_sport.cc
 *
 * @brief   LPM Source Port implementation
 */

#include <math.h>
#include "nic/apollo/lpm/lpm.hpp"
#include "nic/apollo/lpm/lpm_sport.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/framework/pipeline_impl_base.hpp"
#include "gen/p4gen/apollo_rxdma/include/apollo_rxdma_p4pd.h"

sdk_ret_t
lpm_sport_add_key_to_stage (uint8_t *bytes, uint32_t idx,
                            lpm_inode_t *lpm_inode)
{
    auto table = (sacl_sport_lpm_s0_actiondata_t *)bytes;

    switch (idx) {
    case 0:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key0 =
            lpm_inode->port;
        break;
    case 1:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key1 =
            lpm_inode->port;
        break;
    case 2:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key2 =
            lpm_inode->port;
        break;
    case 3:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key3 =
            lpm_inode->port;
        break;
    case 4:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key4 =
            lpm_inode->port;
        break;
    case 5:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key5 =
            lpm_inode->port;
        break;
    case 6:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key6 =
            lpm_inode->port;
        break;
    case 7:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key7 =
            lpm_inode->port;
        break;
    case 8:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key8 =
            lpm_inode->port;
        break;
    case 9:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key9 =
            lpm_inode->port;
        break;
    case 10:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key10 =
            lpm_inode->port;
        break;
    case 11:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key11 =
            lpm_inode->port;
        break;
    case 12:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key12 =
            lpm_inode->port;
        break;
    case 13:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key13 =
            lpm_inode->port;
        break;
    case 14:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key14 =
            lpm_inode->port;
        break;
    case 15:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key15 =
            lpm_inode->port;
        break;
    case 16:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key16 =
            lpm_inode->port;
        break;
    case 17:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key17 =
            lpm_inode->port;
        break;
    case 18:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key18 =
            lpm_inode->port;
        break;
    case 19:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key19 =
            lpm_inode->port;
        break;
    case 20:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key20 =
            lpm_inode->port;
        break;
    case 21:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key21 =
            lpm_inode->port;
        break;
    case 22:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key22 =
            lpm_inode->port;
        break;
    case 23:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key23 =
            lpm_inode->port;
        break;
    case 24:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key24 =
            lpm_inode->port;
        break;
    case 25:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key25 =
            lpm_inode->port;
        break;
    case 26:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key26 =
            lpm_inode->port;
        break;
    case 27:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key27 =
            lpm_inode->port;
        break;
    case 28:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key28 =
            lpm_inode->port;
        break;
    case 29:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key29 =
            lpm_inode->port;
        break;
    case 30:
        table->action_u.sacl_sport_lpm_s0_sacl_sport_lpm_s0.key30 =
            lpm_inode->port;
        break;
    default:
        break;
    }
    return SDK_RET_OK;
}

sdk_ret_t
lpm_sport_add_key_to_last_stage (uint8_t *bytes, uint32_t idx,
                                 lpm_inode_t *lpm_inode)
{
    auto table = (sacl_sport_lpm_s1_actiondata_t *)bytes;

    switch (idx) {
    case 0:
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.key0 =
            lpm_inode->port;
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.data1 =
            (uint16_t)lpm_inode->data;
        break;
    case 1:
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.key1 =
            lpm_inode->port;
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.data2 =
            (uint16_t)lpm_inode->data;
        break;
    case 2:
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.key2 =
            lpm_inode->port;
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.data3 =
            (uint16_t)lpm_inode->data;
        break;
    case 3:
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.key3 =
            lpm_inode->port;
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.data4 =
            (uint16_t)lpm_inode->data;
        break;
    case 4:
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.key4 =
            lpm_inode->port;
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.data5 =
            (uint16_t)lpm_inode->data;
        break;
    case 5:
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.key5 =
            lpm_inode->port;
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.data6 =
            (uint16_t)lpm_inode->data;
        break;
    case 6:
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.key6 =
            lpm_inode->port;
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.data7 =
            (uint16_t)lpm_inode->data;
        break;
    case 7:
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.key7 =
            lpm_inode->port;
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.data8 =
            (uint16_t)lpm_inode->data;
        break;
    case 8:
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.key8 =
            lpm_inode->port;
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.data9 =
            (uint16_t)lpm_inode->data;
        break;
    case 9:
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.key9 =
            lpm_inode->port;
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.data10 =
            (uint16_t)lpm_inode->data;
        break;
    case 10:
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.key10 =
            lpm_inode->port;
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.data11 =
            (uint16_t)lpm_inode->data;
        break;
    case 11:
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.key11 =
            lpm_inode->port;
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.data12 =
            (uint16_t)lpm_inode->data;
        break;
    case 12:
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.key12 =
            lpm_inode->port;
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.data13 =
            (uint16_t)lpm_inode->data;
        break;
    case 13:
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.key13 =
            lpm_inode->port;
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.data14 =
            (uint16_t)lpm_inode->data;
        break;
    case 14:
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.key14 =
            lpm_inode->port;
        table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.data15 =
            (uint16_t)lpm_inode->data;
        break;
    default:
        break;
    }

    return SDK_RET_OK;
}

sdk_ret_t
lpm_sport_set_default_data (uint8_t *bytes, uint32_t default_data)
{
    auto table = (sacl_sport_lpm_s1_actiondata_t *) bytes;
    table->action_u.sacl_sport_lpm_s1_sacl_sport_lpm_s1.data0 = (uint16_t)default_data;
    return SDK_RET_OK;
}

sdk_ret_t
lpm_sport_write_stage_table (mem_addr_t addr, uint8_t *bytes)
{
    return impl_base::pipeline_impl()->write_to_rxdma_table(addr,
               P4_APOLLO_RXDMA_TBL_ID_SACL_SPORT_LPM_S0,
               SACL_SPORT_LPM_S0_SACL_SPORT_LPM_S0_ID, bytes);
}

sdk_ret_t
lpm_sport_write_last_stage_table (mem_addr_t addr, uint8_t *bytes)
{
    return impl_base::pipeline_impl()->write_to_rxdma_table(addr,
               P4_APOLLO_RXDMA_TBL_ID_SACL_SPORT_LPM_S1,
               SACL_SPORT_LPM_S1_SACL_SPORT_LPM_S1_ID, bytes);
}

/**
 * key size is 2 bytes for Src Port
 */
uint32_t
lpm_sport_key_size (void)
{
    return LPM_SPORT_KEY_SIZE;
}

/**
 * @brief    compute the number of stages needed for LPM lookup given the
 *           interval table scale
 * @param[in]    num_intrvls   number of intervals in the interval table
 * @return       number of lookup stages (aka. depth of the interval tree)
 *
 * The computation is done as follows for Src Port :
 *     The last stage gives an 16-way decision. The other stages each give
 *     a 32-way decision.
 *     #stages = 1 + log32(num_intrvls/16.0)
 *             = 1 + log2(num_intrvls/16.0)/log2(32)
 *             = 1 + log2(num_intrvls/16.0)/5.0
 */
uint32_t
lpm_sport_stages (uint32_t num_intrvls)
{
    // 1 * 16-way last stage, plus (n-1) * 32-way stages
    return (1 + ((uint32_t)ceil(log2f((float)(num_intrvls/16.0))/5.0)));
}
