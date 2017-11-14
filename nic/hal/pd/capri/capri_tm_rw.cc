/*
 * capri_tm_rw.cc
 * Vasanth Kumar (Pensando Systems)
 */

/* NOTE This is an auto-generated file using pytenjin. DO NOT EDIT!!!!!
 * Template file is at capri_tm_rw.template.cc
 */

#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <map>
#include <cmath>

#include "nic/include/base.h"
#include "nic/p4/nw/include/defines.h"
#include "nic/hal/pd/capri/capri_tm_rw.hpp"

#ifndef HAL_GTEST
#include "nic/asic/capri/model/utils/cap_blk_reg_model.h"
#include "nic/asic/capri/model/cap_top/cap_top_csr.h"
#include "nic/asic/capri/model/cap_pb/cap_pbc_csr.h"
#include "nic/asic/capri/verif/apis/cap_pb_api.h"
#endif


static bool 
is_valid_tm_uplink_port(uint32_t port) 
{
    return ((port >= TM_PORT_UPLINK_0) && (port <= TM_PORT_NCSI));
}

static bool 
is_valid_tm_port(uint32_t port) 
{
    return ((port >= TM_PORT_UPLINK_0) && (port <= TM_PORT_INGRESS));
}

static inline uint32_t 
bytes_to_cells(uint32_t bytes)
{
    return (bytes + HAL_TM_CELL_SIZE - 1)/HAL_TM_CELL_SIZE;
}

hal_ret_t 
capri_tm_pg_params_update(uint32_t port,
                          uint32_t pool_group,
                          tm_pg_params_t *pg_params)
{
    /* Do some sanity checks for port and pool_group */
    if (!is_valid_tm_port(port)) {
        HAL_TRACE_ERR("CAPRI-TM::{}: {} is not a valid TM port", 
                      __func__, port);
        return HAL_RET_INVALID_ARG;
    }
#ifndef HAL_GTEST
    uint32_t i;
    uint32_t cos;
    uint32_t npgs;

    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    cpp_int cos_map_reg_val = 0;

    switch(port) {
        case TM_PORT_UPLINK_0:
        {
            npgs = 8;
            break;
        }
        case TM_PORT_UPLINK_1:
        {
            npgs = 8;
            break;
        }
        case TM_PORT_UPLINK_2:
        {
            npgs = 8;
            break;
        }
        case TM_PORT_UPLINK_3:
        {
            npgs = 8;
            break;
        }
        case TM_PORT_UPLINK_4:
        {
            npgs = 8;
            break;
        }
        case TM_PORT_UPLINK_5:
        {
            npgs = 8;
            break;
        }
        case TM_PORT_UPLINK_6:
        {
            npgs = 8;
            break;
        }
        case TM_PORT_UPLINK_7:
        {
            npgs = 8;
            break;
        }
        case TM_PORT_NCSI:
        {
            npgs = 8;
            break;
        }
        case TM_PORT_DMA:
        {
            npgs = 16;
            break;
        }
        case TM_PORT_EGRESS:
        {
            npgs = 32;
            break;
        }
        case TM_PORT_INGRESS:
        {
            npgs = 32;
            break;
        }
        default:
            return HAL_RET_INVALID_ARG;
    }

    if (pool_group >= npgs) {
        HAL_TRACE_ERR("CAPRI-TM::{}: Invalid pool group {} on port {}",
                      __func__, pool_group, port);
        return HAL_RET_INVALID_ARG;
    }

    /* Update the cos to PG mapping */
    for (i = 0; i < pg_params->ncos; i++) {
        cos = pg_params->cos_map[i]; 
        if (cos >= npgs) {
            HAL_TRACE_ERR("CAPRI-TM::{}: Invalid cos value {} on port {} pool group {}",
                          __func__, cos, port, pool_group);
            return HAL_RET_INVALID_ARG;
        }

        cos_map_reg_val |= (pool_group & (npgs-1)) << (cos * (int)std::log2(npgs));
    }

    switch(port) {
        case TM_PORT_UPLINK_0:
        {
            switch (pool_group) {
                case 0:
                {
                    pbc_csr.cfg_account_0_pg_0.read();
                    pbc_csr.cfg_account_0_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_0_pg_0.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_0_pg_0.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_0_pg_0.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_0_pg_0.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_0_pg_0.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_0_mtu_table.pg0(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_0_pg_0.write();
                    pbc_csr.cfg_account_0_mtu_table.write();
                    break;
                }
                case 1:
                {
                    pbc_csr.cfg_account_0_pg_1.read();
                    pbc_csr.cfg_account_0_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_0_pg_1.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_0_pg_1.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_0_pg_1.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_0_pg_1.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_0_pg_1.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_0_mtu_table.pg1(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_0_pg_1.write();
                    pbc_csr.cfg_account_0_mtu_table.write();
                    break;
                }
                case 2:
                {
                    pbc_csr.cfg_account_0_pg_2.read();
                    pbc_csr.cfg_account_0_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_0_pg_2.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_0_pg_2.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_0_pg_2.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_0_pg_2.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_0_pg_2.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_0_mtu_table.pg2(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_0_pg_2.write();
                    pbc_csr.cfg_account_0_mtu_table.write();
                    break;
                }
                case 3:
                {
                    pbc_csr.cfg_account_0_pg_3.read();
                    pbc_csr.cfg_account_0_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_0_pg_3.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_0_pg_3.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_0_pg_3.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_0_pg_3.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_0_pg_3.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_0_mtu_table.pg3(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_0_pg_3.write();
                    pbc_csr.cfg_account_0_mtu_table.write();
                    break;
                }
                case 4:
                {
                    pbc_csr.cfg_account_0_pg_4.read();
                    pbc_csr.cfg_account_0_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_0_pg_4.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_0_pg_4.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_0_pg_4.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_0_pg_4.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_0_pg_4.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_0_mtu_table.pg4(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_0_pg_4.write();
                    pbc_csr.cfg_account_0_mtu_table.write();
                    break;
                }
                case 5:
                {
                    pbc_csr.cfg_account_0_pg_5.read();
                    pbc_csr.cfg_account_0_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_0_pg_5.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_0_pg_5.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_0_pg_5.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_0_pg_5.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_0_pg_5.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_0_mtu_table.pg5(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_0_pg_5.write();
                    pbc_csr.cfg_account_0_mtu_table.write();
                    break;
                }
                case 6:
                {
                    pbc_csr.cfg_account_0_pg_6.read();
                    pbc_csr.cfg_account_0_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_0_pg_6.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_0_pg_6.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_0_pg_6.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_0_pg_6.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_0_pg_6.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_0_mtu_table.pg6(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_0_pg_6.write();
                    pbc_csr.cfg_account_0_mtu_table.write();
                    break;
                }
                case 7:
                {
                    pbc_csr.cfg_account_0_pg_7.read();
                    pbc_csr.cfg_account_0_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_0_pg_7.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_0_pg_7.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_0_pg_7.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_0_pg_7.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_0_pg_7.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_0_mtu_table.pg7(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_0_pg_7.write();
                    pbc_csr.cfg_account_0_mtu_table.write();
                    break;
                }
                default:
                    return HAL_RET_ERR;
            }
            /* Update and write the cos to PG mapping */
            pbc_csr.cfg_account_0_tc_to_pg.read();
            pbc_csr.cfg_account_0_tc_to_pg.table(cos_map_reg_val);
            pbc_csr.cfg_account_0_tc_to_pg.write();
            break;
        }
        case TM_PORT_UPLINK_1:
        {
            switch (pool_group) {
                case 0:
                {
                    pbc_csr.cfg_account_1_pg_0.read();
                    pbc_csr.cfg_account_1_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_1_pg_0.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_1_pg_0.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_1_pg_0.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_1_pg_0.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_1_pg_0.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_1_mtu_table.pg0(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_1_pg_0.write();
                    pbc_csr.cfg_account_1_mtu_table.write();
                    break;
                }
                case 1:
                {
                    pbc_csr.cfg_account_1_pg_1.read();
                    pbc_csr.cfg_account_1_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_1_pg_1.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_1_pg_1.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_1_pg_1.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_1_pg_1.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_1_pg_1.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_1_mtu_table.pg1(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_1_pg_1.write();
                    pbc_csr.cfg_account_1_mtu_table.write();
                    break;
                }
                case 2:
                {
                    pbc_csr.cfg_account_1_pg_2.read();
                    pbc_csr.cfg_account_1_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_1_pg_2.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_1_pg_2.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_1_pg_2.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_1_pg_2.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_1_pg_2.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_1_mtu_table.pg2(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_1_pg_2.write();
                    pbc_csr.cfg_account_1_mtu_table.write();
                    break;
                }
                case 3:
                {
                    pbc_csr.cfg_account_1_pg_3.read();
                    pbc_csr.cfg_account_1_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_1_pg_3.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_1_pg_3.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_1_pg_3.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_1_pg_3.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_1_pg_3.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_1_mtu_table.pg3(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_1_pg_3.write();
                    pbc_csr.cfg_account_1_mtu_table.write();
                    break;
                }
                case 4:
                {
                    pbc_csr.cfg_account_1_pg_4.read();
                    pbc_csr.cfg_account_1_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_1_pg_4.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_1_pg_4.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_1_pg_4.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_1_pg_4.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_1_pg_4.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_1_mtu_table.pg4(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_1_pg_4.write();
                    pbc_csr.cfg_account_1_mtu_table.write();
                    break;
                }
                case 5:
                {
                    pbc_csr.cfg_account_1_pg_5.read();
                    pbc_csr.cfg_account_1_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_1_pg_5.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_1_pg_5.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_1_pg_5.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_1_pg_5.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_1_pg_5.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_1_mtu_table.pg5(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_1_pg_5.write();
                    pbc_csr.cfg_account_1_mtu_table.write();
                    break;
                }
                case 6:
                {
                    pbc_csr.cfg_account_1_pg_6.read();
                    pbc_csr.cfg_account_1_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_1_pg_6.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_1_pg_6.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_1_pg_6.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_1_pg_6.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_1_pg_6.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_1_mtu_table.pg6(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_1_pg_6.write();
                    pbc_csr.cfg_account_1_mtu_table.write();
                    break;
                }
                case 7:
                {
                    pbc_csr.cfg_account_1_pg_7.read();
                    pbc_csr.cfg_account_1_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_1_pg_7.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_1_pg_7.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_1_pg_7.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_1_pg_7.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_1_pg_7.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_1_mtu_table.pg7(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_1_pg_7.write();
                    pbc_csr.cfg_account_1_mtu_table.write();
                    break;
                }
                default:
                    return HAL_RET_ERR;
            }
            /* Update and write the cos to PG mapping */
            pbc_csr.cfg_account_1_tc_to_pg.read();
            pbc_csr.cfg_account_1_tc_to_pg.table(cos_map_reg_val);
            pbc_csr.cfg_account_1_tc_to_pg.write();
            break;
        }
        case TM_PORT_UPLINK_2:
        {
            switch (pool_group) {
                case 0:
                {
                    pbc_csr.cfg_account_2_pg_0.read();
                    pbc_csr.cfg_account_2_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_2_pg_0.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_2_pg_0.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_2_pg_0.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_2_pg_0.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_2_pg_0.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_2_mtu_table.pg0(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_2_pg_0.write();
                    pbc_csr.cfg_account_2_mtu_table.write();
                    break;
                }
                case 1:
                {
                    pbc_csr.cfg_account_2_pg_1.read();
                    pbc_csr.cfg_account_2_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_2_pg_1.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_2_pg_1.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_2_pg_1.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_2_pg_1.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_2_pg_1.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_2_mtu_table.pg1(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_2_pg_1.write();
                    pbc_csr.cfg_account_2_mtu_table.write();
                    break;
                }
                case 2:
                {
                    pbc_csr.cfg_account_2_pg_2.read();
                    pbc_csr.cfg_account_2_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_2_pg_2.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_2_pg_2.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_2_pg_2.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_2_pg_2.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_2_pg_2.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_2_mtu_table.pg2(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_2_pg_2.write();
                    pbc_csr.cfg_account_2_mtu_table.write();
                    break;
                }
                case 3:
                {
                    pbc_csr.cfg_account_2_pg_3.read();
                    pbc_csr.cfg_account_2_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_2_pg_3.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_2_pg_3.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_2_pg_3.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_2_pg_3.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_2_pg_3.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_2_mtu_table.pg3(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_2_pg_3.write();
                    pbc_csr.cfg_account_2_mtu_table.write();
                    break;
                }
                case 4:
                {
                    pbc_csr.cfg_account_2_pg_4.read();
                    pbc_csr.cfg_account_2_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_2_pg_4.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_2_pg_4.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_2_pg_4.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_2_pg_4.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_2_pg_4.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_2_mtu_table.pg4(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_2_pg_4.write();
                    pbc_csr.cfg_account_2_mtu_table.write();
                    break;
                }
                case 5:
                {
                    pbc_csr.cfg_account_2_pg_5.read();
                    pbc_csr.cfg_account_2_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_2_pg_5.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_2_pg_5.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_2_pg_5.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_2_pg_5.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_2_pg_5.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_2_mtu_table.pg5(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_2_pg_5.write();
                    pbc_csr.cfg_account_2_mtu_table.write();
                    break;
                }
                case 6:
                {
                    pbc_csr.cfg_account_2_pg_6.read();
                    pbc_csr.cfg_account_2_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_2_pg_6.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_2_pg_6.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_2_pg_6.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_2_pg_6.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_2_pg_6.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_2_mtu_table.pg6(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_2_pg_6.write();
                    pbc_csr.cfg_account_2_mtu_table.write();
                    break;
                }
                case 7:
                {
                    pbc_csr.cfg_account_2_pg_7.read();
                    pbc_csr.cfg_account_2_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_2_pg_7.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_2_pg_7.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_2_pg_7.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_2_pg_7.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_2_pg_7.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_2_mtu_table.pg7(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_2_pg_7.write();
                    pbc_csr.cfg_account_2_mtu_table.write();
                    break;
                }
                default:
                    return HAL_RET_ERR;
            }
            /* Update and write the cos to PG mapping */
            pbc_csr.cfg_account_2_tc_to_pg.read();
            pbc_csr.cfg_account_2_tc_to_pg.table(cos_map_reg_val);
            pbc_csr.cfg_account_2_tc_to_pg.write();
            break;
        }
        case TM_PORT_UPLINK_3:
        {
            switch (pool_group) {
                case 0:
                {
                    pbc_csr.cfg_account_3_pg_0.read();
                    pbc_csr.cfg_account_3_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_3_pg_0.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_3_pg_0.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_3_pg_0.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_3_pg_0.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_3_pg_0.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_3_mtu_table.pg0(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_3_pg_0.write();
                    pbc_csr.cfg_account_3_mtu_table.write();
                    break;
                }
                case 1:
                {
                    pbc_csr.cfg_account_3_pg_1.read();
                    pbc_csr.cfg_account_3_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_3_pg_1.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_3_pg_1.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_3_pg_1.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_3_pg_1.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_3_pg_1.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_3_mtu_table.pg1(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_3_pg_1.write();
                    pbc_csr.cfg_account_3_mtu_table.write();
                    break;
                }
                case 2:
                {
                    pbc_csr.cfg_account_3_pg_2.read();
                    pbc_csr.cfg_account_3_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_3_pg_2.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_3_pg_2.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_3_pg_2.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_3_pg_2.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_3_pg_2.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_3_mtu_table.pg2(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_3_pg_2.write();
                    pbc_csr.cfg_account_3_mtu_table.write();
                    break;
                }
                case 3:
                {
                    pbc_csr.cfg_account_3_pg_3.read();
                    pbc_csr.cfg_account_3_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_3_pg_3.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_3_pg_3.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_3_pg_3.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_3_pg_3.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_3_pg_3.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_3_mtu_table.pg3(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_3_pg_3.write();
                    pbc_csr.cfg_account_3_mtu_table.write();
                    break;
                }
                case 4:
                {
                    pbc_csr.cfg_account_3_pg_4.read();
                    pbc_csr.cfg_account_3_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_3_pg_4.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_3_pg_4.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_3_pg_4.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_3_pg_4.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_3_pg_4.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_3_mtu_table.pg4(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_3_pg_4.write();
                    pbc_csr.cfg_account_3_mtu_table.write();
                    break;
                }
                case 5:
                {
                    pbc_csr.cfg_account_3_pg_5.read();
                    pbc_csr.cfg_account_3_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_3_pg_5.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_3_pg_5.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_3_pg_5.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_3_pg_5.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_3_pg_5.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_3_mtu_table.pg5(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_3_pg_5.write();
                    pbc_csr.cfg_account_3_mtu_table.write();
                    break;
                }
                case 6:
                {
                    pbc_csr.cfg_account_3_pg_6.read();
                    pbc_csr.cfg_account_3_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_3_pg_6.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_3_pg_6.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_3_pg_6.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_3_pg_6.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_3_pg_6.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_3_mtu_table.pg6(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_3_pg_6.write();
                    pbc_csr.cfg_account_3_mtu_table.write();
                    break;
                }
                case 7:
                {
                    pbc_csr.cfg_account_3_pg_7.read();
                    pbc_csr.cfg_account_3_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_3_pg_7.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_3_pg_7.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_3_pg_7.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_3_pg_7.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_3_pg_7.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_3_mtu_table.pg7(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_3_pg_7.write();
                    pbc_csr.cfg_account_3_mtu_table.write();
                    break;
                }
                default:
                    return HAL_RET_ERR;
            }
            /* Update and write the cos to PG mapping */
            pbc_csr.cfg_account_3_tc_to_pg.read();
            pbc_csr.cfg_account_3_tc_to_pg.table(cos_map_reg_val);
            pbc_csr.cfg_account_3_tc_to_pg.write();
            break;
        }
        case TM_PORT_UPLINK_4:
        {
            switch (pool_group) {
                case 0:
                {
                    pbc_csr.cfg_account_4_pg_0.read();
                    pbc_csr.cfg_account_4_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_4_pg_0.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_4_pg_0.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_4_pg_0.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_4_pg_0.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_4_pg_0.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_4_mtu_table.pg0(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_4_pg_0.write();
                    pbc_csr.cfg_account_4_mtu_table.write();
                    break;
                }
                case 1:
                {
                    pbc_csr.cfg_account_4_pg_1.read();
                    pbc_csr.cfg_account_4_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_4_pg_1.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_4_pg_1.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_4_pg_1.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_4_pg_1.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_4_pg_1.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_4_mtu_table.pg1(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_4_pg_1.write();
                    pbc_csr.cfg_account_4_mtu_table.write();
                    break;
                }
                case 2:
                {
                    pbc_csr.cfg_account_4_pg_2.read();
                    pbc_csr.cfg_account_4_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_4_pg_2.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_4_pg_2.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_4_pg_2.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_4_pg_2.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_4_pg_2.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_4_mtu_table.pg2(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_4_pg_2.write();
                    pbc_csr.cfg_account_4_mtu_table.write();
                    break;
                }
                case 3:
                {
                    pbc_csr.cfg_account_4_pg_3.read();
                    pbc_csr.cfg_account_4_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_4_pg_3.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_4_pg_3.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_4_pg_3.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_4_pg_3.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_4_pg_3.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_4_mtu_table.pg3(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_4_pg_3.write();
                    pbc_csr.cfg_account_4_mtu_table.write();
                    break;
                }
                case 4:
                {
                    pbc_csr.cfg_account_4_pg_4.read();
                    pbc_csr.cfg_account_4_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_4_pg_4.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_4_pg_4.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_4_pg_4.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_4_pg_4.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_4_pg_4.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_4_mtu_table.pg4(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_4_pg_4.write();
                    pbc_csr.cfg_account_4_mtu_table.write();
                    break;
                }
                case 5:
                {
                    pbc_csr.cfg_account_4_pg_5.read();
                    pbc_csr.cfg_account_4_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_4_pg_5.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_4_pg_5.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_4_pg_5.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_4_pg_5.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_4_pg_5.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_4_mtu_table.pg5(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_4_pg_5.write();
                    pbc_csr.cfg_account_4_mtu_table.write();
                    break;
                }
                case 6:
                {
                    pbc_csr.cfg_account_4_pg_6.read();
                    pbc_csr.cfg_account_4_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_4_pg_6.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_4_pg_6.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_4_pg_6.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_4_pg_6.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_4_pg_6.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_4_mtu_table.pg6(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_4_pg_6.write();
                    pbc_csr.cfg_account_4_mtu_table.write();
                    break;
                }
                case 7:
                {
                    pbc_csr.cfg_account_4_pg_7.read();
                    pbc_csr.cfg_account_4_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_4_pg_7.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_4_pg_7.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_4_pg_7.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_4_pg_7.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_4_pg_7.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_4_mtu_table.pg7(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_4_pg_7.write();
                    pbc_csr.cfg_account_4_mtu_table.write();
                    break;
                }
                default:
                    return HAL_RET_ERR;
            }
            /* Update and write the cos to PG mapping */
            pbc_csr.cfg_account_4_tc_to_pg.read();
            pbc_csr.cfg_account_4_tc_to_pg.table(cos_map_reg_val);
            pbc_csr.cfg_account_4_tc_to_pg.write();
            break;
        }
        case TM_PORT_UPLINK_5:
        {
            switch (pool_group) {
                case 0:
                {
                    pbc_csr.cfg_account_5_pg_0.read();
                    pbc_csr.cfg_account_5_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_5_pg_0.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_5_pg_0.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_5_pg_0.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_5_pg_0.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_5_pg_0.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_5_mtu_table.pg0(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_5_pg_0.write();
                    pbc_csr.cfg_account_5_mtu_table.write();
                    break;
                }
                case 1:
                {
                    pbc_csr.cfg_account_5_pg_1.read();
                    pbc_csr.cfg_account_5_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_5_pg_1.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_5_pg_1.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_5_pg_1.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_5_pg_1.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_5_pg_1.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_5_mtu_table.pg1(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_5_pg_1.write();
                    pbc_csr.cfg_account_5_mtu_table.write();
                    break;
                }
                case 2:
                {
                    pbc_csr.cfg_account_5_pg_2.read();
                    pbc_csr.cfg_account_5_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_5_pg_2.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_5_pg_2.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_5_pg_2.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_5_pg_2.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_5_pg_2.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_5_mtu_table.pg2(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_5_pg_2.write();
                    pbc_csr.cfg_account_5_mtu_table.write();
                    break;
                }
                case 3:
                {
                    pbc_csr.cfg_account_5_pg_3.read();
                    pbc_csr.cfg_account_5_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_5_pg_3.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_5_pg_3.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_5_pg_3.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_5_pg_3.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_5_pg_3.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_5_mtu_table.pg3(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_5_pg_3.write();
                    pbc_csr.cfg_account_5_mtu_table.write();
                    break;
                }
                case 4:
                {
                    pbc_csr.cfg_account_5_pg_4.read();
                    pbc_csr.cfg_account_5_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_5_pg_4.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_5_pg_4.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_5_pg_4.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_5_pg_4.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_5_pg_4.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_5_mtu_table.pg4(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_5_pg_4.write();
                    pbc_csr.cfg_account_5_mtu_table.write();
                    break;
                }
                case 5:
                {
                    pbc_csr.cfg_account_5_pg_5.read();
                    pbc_csr.cfg_account_5_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_5_pg_5.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_5_pg_5.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_5_pg_5.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_5_pg_5.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_5_pg_5.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_5_mtu_table.pg5(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_5_pg_5.write();
                    pbc_csr.cfg_account_5_mtu_table.write();
                    break;
                }
                case 6:
                {
                    pbc_csr.cfg_account_5_pg_6.read();
                    pbc_csr.cfg_account_5_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_5_pg_6.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_5_pg_6.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_5_pg_6.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_5_pg_6.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_5_pg_6.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_5_mtu_table.pg6(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_5_pg_6.write();
                    pbc_csr.cfg_account_5_mtu_table.write();
                    break;
                }
                case 7:
                {
                    pbc_csr.cfg_account_5_pg_7.read();
                    pbc_csr.cfg_account_5_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_5_pg_7.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_5_pg_7.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_5_pg_7.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_5_pg_7.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_5_pg_7.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_5_mtu_table.pg7(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_5_pg_7.write();
                    pbc_csr.cfg_account_5_mtu_table.write();
                    break;
                }
                default:
                    return HAL_RET_ERR;
            }
            /* Update and write the cos to PG mapping */
            pbc_csr.cfg_account_5_tc_to_pg.read();
            pbc_csr.cfg_account_5_tc_to_pg.table(cos_map_reg_val);
            pbc_csr.cfg_account_5_tc_to_pg.write();
            break;
        }
        case TM_PORT_UPLINK_6:
        {
            switch (pool_group) {
                case 0:
                {
                    pbc_csr.cfg_account_6_pg_0.read();
                    pbc_csr.cfg_account_6_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_6_pg_0.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_6_pg_0.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_6_pg_0.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_6_pg_0.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_6_pg_0.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_6_mtu_table.pg0(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_6_pg_0.write();
                    pbc_csr.cfg_account_6_mtu_table.write();
                    break;
                }
                case 1:
                {
                    pbc_csr.cfg_account_6_pg_1.read();
                    pbc_csr.cfg_account_6_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_6_pg_1.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_6_pg_1.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_6_pg_1.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_6_pg_1.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_6_pg_1.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_6_mtu_table.pg1(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_6_pg_1.write();
                    pbc_csr.cfg_account_6_mtu_table.write();
                    break;
                }
                case 2:
                {
                    pbc_csr.cfg_account_6_pg_2.read();
                    pbc_csr.cfg_account_6_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_6_pg_2.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_6_pg_2.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_6_pg_2.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_6_pg_2.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_6_pg_2.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_6_mtu_table.pg2(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_6_pg_2.write();
                    pbc_csr.cfg_account_6_mtu_table.write();
                    break;
                }
                case 3:
                {
                    pbc_csr.cfg_account_6_pg_3.read();
                    pbc_csr.cfg_account_6_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_6_pg_3.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_6_pg_3.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_6_pg_3.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_6_pg_3.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_6_pg_3.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_6_mtu_table.pg3(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_6_pg_3.write();
                    pbc_csr.cfg_account_6_mtu_table.write();
                    break;
                }
                case 4:
                {
                    pbc_csr.cfg_account_6_pg_4.read();
                    pbc_csr.cfg_account_6_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_6_pg_4.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_6_pg_4.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_6_pg_4.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_6_pg_4.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_6_pg_4.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_6_mtu_table.pg4(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_6_pg_4.write();
                    pbc_csr.cfg_account_6_mtu_table.write();
                    break;
                }
                case 5:
                {
                    pbc_csr.cfg_account_6_pg_5.read();
                    pbc_csr.cfg_account_6_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_6_pg_5.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_6_pg_5.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_6_pg_5.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_6_pg_5.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_6_pg_5.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_6_mtu_table.pg5(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_6_pg_5.write();
                    pbc_csr.cfg_account_6_mtu_table.write();
                    break;
                }
                case 6:
                {
                    pbc_csr.cfg_account_6_pg_6.read();
                    pbc_csr.cfg_account_6_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_6_pg_6.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_6_pg_6.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_6_pg_6.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_6_pg_6.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_6_pg_6.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_6_mtu_table.pg6(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_6_pg_6.write();
                    pbc_csr.cfg_account_6_mtu_table.write();
                    break;
                }
                case 7:
                {
                    pbc_csr.cfg_account_6_pg_7.read();
                    pbc_csr.cfg_account_6_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_6_pg_7.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_6_pg_7.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_6_pg_7.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_6_pg_7.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_6_pg_7.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_6_mtu_table.pg7(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_6_pg_7.write();
                    pbc_csr.cfg_account_6_mtu_table.write();
                    break;
                }
                default:
                    return HAL_RET_ERR;
            }
            /* Update and write the cos to PG mapping */
            pbc_csr.cfg_account_6_tc_to_pg.read();
            pbc_csr.cfg_account_6_tc_to_pg.table(cos_map_reg_val);
            pbc_csr.cfg_account_6_tc_to_pg.write();
            break;
        }
        case TM_PORT_UPLINK_7:
        {
            switch (pool_group) {
                case 0:
                {
                    pbc_csr.cfg_account_7_pg_0.read();
                    pbc_csr.cfg_account_7_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_7_pg_0.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_7_pg_0.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_7_pg_0.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_7_pg_0.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_7_pg_0.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_7_mtu_table.pg0(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_7_pg_0.write();
                    pbc_csr.cfg_account_7_mtu_table.write();
                    break;
                }
                case 1:
                {
                    pbc_csr.cfg_account_7_pg_1.read();
                    pbc_csr.cfg_account_7_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_7_pg_1.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_7_pg_1.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_7_pg_1.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_7_pg_1.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_7_pg_1.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_7_mtu_table.pg1(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_7_pg_1.write();
                    pbc_csr.cfg_account_7_mtu_table.write();
                    break;
                }
                case 2:
                {
                    pbc_csr.cfg_account_7_pg_2.read();
                    pbc_csr.cfg_account_7_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_7_pg_2.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_7_pg_2.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_7_pg_2.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_7_pg_2.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_7_pg_2.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_7_mtu_table.pg2(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_7_pg_2.write();
                    pbc_csr.cfg_account_7_mtu_table.write();
                    break;
                }
                case 3:
                {
                    pbc_csr.cfg_account_7_pg_3.read();
                    pbc_csr.cfg_account_7_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_7_pg_3.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_7_pg_3.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_7_pg_3.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_7_pg_3.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_7_pg_3.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_7_mtu_table.pg3(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_7_pg_3.write();
                    pbc_csr.cfg_account_7_mtu_table.write();
                    break;
                }
                case 4:
                {
                    pbc_csr.cfg_account_7_pg_4.read();
                    pbc_csr.cfg_account_7_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_7_pg_4.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_7_pg_4.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_7_pg_4.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_7_pg_4.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_7_pg_4.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_7_mtu_table.pg4(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_7_pg_4.write();
                    pbc_csr.cfg_account_7_mtu_table.write();
                    break;
                }
                case 5:
                {
                    pbc_csr.cfg_account_7_pg_5.read();
                    pbc_csr.cfg_account_7_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_7_pg_5.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_7_pg_5.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_7_pg_5.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_7_pg_5.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_7_pg_5.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_7_mtu_table.pg5(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_7_pg_5.write();
                    pbc_csr.cfg_account_7_mtu_table.write();
                    break;
                }
                case 6:
                {
                    pbc_csr.cfg_account_7_pg_6.read();
                    pbc_csr.cfg_account_7_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_7_pg_6.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_7_pg_6.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_7_pg_6.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_7_pg_6.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_7_pg_6.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_7_mtu_table.pg6(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_7_pg_6.write();
                    pbc_csr.cfg_account_7_mtu_table.write();
                    break;
                }
                case 7:
                {
                    pbc_csr.cfg_account_7_pg_7.read();
                    pbc_csr.cfg_account_7_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_7_pg_7.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_7_pg_7.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_7_pg_7.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_7_pg_7.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_7_pg_7.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_7_mtu_table.pg7(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_7_pg_7.write();
                    pbc_csr.cfg_account_7_mtu_table.write();
                    break;
                }
                default:
                    return HAL_RET_ERR;
            }
            /* Update and write the cos to PG mapping */
            pbc_csr.cfg_account_7_tc_to_pg.read();
            pbc_csr.cfg_account_7_tc_to_pg.table(cos_map_reg_val);
            pbc_csr.cfg_account_7_tc_to_pg.write();
            break;
        }
        case TM_PORT_NCSI:
        {
            switch (pool_group) {
                case 0:
                {
                    pbc_csr.cfg_account_8_pg_0.read();
                    pbc_csr.cfg_account_8_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_8_pg_0.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_8_pg_0.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_8_pg_0.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_8_pg_0.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_8_pg_0.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_8_mtu_table.pg0(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_8_pg_0.write();
                    pbc_csr.cfg_account_8_mtu_table.write();
                    break;
                }
                case 1:
                {
                    pbc_csr.cfg_account_8_pg_1.read();
                    pbc_csr.cfg_account_8_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_8_pg_1.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_8_pg_1.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_8_pg_1.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_8_pg_1.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_8_pg_1.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_8_mtu_table.pg1(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_8_pg_1.write();
                    pbc_csr.cfg_account_8_mtu_table.write();
                    break;
                }
                case 2:
                {
                    pbc_csr.cfg_account_8_pg_2.read();
                    pbc_csr.cfg_account_8_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_8_pg_2.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_8_pg_2.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_8_pg_2.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_8_pg_2.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_8_pg_2.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_8_mtu_table.pg2(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_8_pg_2.write();
                    pbc_csr.cfg_account_8_mtu_table.write();
                    break;
                }
                case 3:
                {
                    pbc_csr.cfg_account_8_pg_3.read();
                    pbc_csr.cfg_account_8_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_8_pg_3.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_8_pg_3.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_8_pg_3.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_8_pg_3.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_8_pg_3.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_8_mtu_table.pg3(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_8_pg_3.write();
                    pbc_csr.cfg_account_8_mtu_table.write();
                    break;
                }
                case 4:
                {
                    pbc_csr.cfg_account_8_pg_4.read();
                    pbc_csr.cfg_account_8_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_8_pg_4.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_8_pg_4.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_8_pg_4.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_8_pg_4.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_8_pg_4.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_8_mtu_table.pg4(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_8_pg_4.write();
                    pbc_csr.cfg_account_8_mtu_table.write();
                    break;
                }
                case 5:
                {
                    pbc_csr.cfg_account_8_pg_5.read();
                    pbc_csr.cfg_account_8_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_8_pg_5.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_8_pg_5.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_8_pg_5.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_8_pg_5.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_8_pg_5.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_8_mtu_table.pg5(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_8_pg_5.write();
                    pbc_csr.cfg_account_8_mtu_table.write();
                    break;
                }
                case 6:
                {
                    pbc_csr.cfg_account_8_pg_6.read();
                    pbc_csr.cfg_account_8_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_8_pg_6.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_8_pg_6.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_8_pg_6.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_8_pg_6.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_8_pg_6.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_8_mtu_table.pg6(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_8_pg_6.write();
                    pbc_csr.cfg_account_8_mtu_table.write();
                    break;
                }
                case 7:
                {
                    pbc_csr.cfg_account_8_pg_7.read();
                    pbc_csr.cfg_account_8_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_8_pg_7.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_8_pg_7.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_8_pg_7.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_8_pg_7.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_8_pg_7.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_8_mtu_table.pg7(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_8_pg_7.write();
                    pbc_csr.cfg_account_8_mtu_table.write();
                    break;
                }
                default:
                    return HAL_RET_ERR;
            }
            /* Update and write the cos to PG mapping */
            pbc_csr.cfg_account_8_tc_to_pg.read();
            pbc_csr.cfg_account_8_tc_to_pg.table(cos_map_reg_val);
            pbc_csr.cfg_account_8_tc_to_pg.write();
            break;
        }
        case TM_PORT_DMA:
        {
            switch (pool_group) {
                case 0:
                {
                    pbc_csr.cfg_account_9_pg_0.read();
                    pbc_csr.cfg_account_9_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_9_pg_0.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_9_pg_0.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_9_pg_0.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_9_pg_0.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_9_pg_0.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_9_mtu_table.pg0(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_9_pg_0.write();
                    pbc_csr.cfg_account_9_mtu_table.write();
                    break;
                }
                case 1:
                {
                    pbc_csr.cfg_account_9_pg_1.read();
                    pbc_csr.cfg_account_9_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_9_pg_1.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_9_pg_1.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_9_pg_1.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_9_pg_1.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_9_pg_1.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_9_mtu_table.pg1(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_9_pg_1.write();
                    pbc_csr.cfg_account_9_mtu_table.write();
                    break;
                }
                case 2:
                {
                    pbc_csr.cfg_account_9_pg_2.read();
                    pbc_csr.cfg_account_9_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_9_pg_2.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_9_pg_2.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_9_pg_2.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_9_pg_2.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_9_pg_2.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_9_mtu_table.pg2(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_9_pg_2.write();
                    pbc_csr.cfg_account_9_mtu_table.write();
                    break;
                }
                case 3:
                {
                    pbc_csr.cfg_account_9_pg_3.read();
                    pbc_csr.cfg_account_9_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_9_pg_3.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_9_pg_3.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_9_pg_3.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_9_pg_3.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_9_pg_3.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_9_mtu_table.pg3(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_9_pg_3.write();
                    pbc_csr.cfg_account_9_mtu_table.write();
                    break;
                }
                case 4:
                {
                    pbc_csr.cfg_account_9_pg_4.read();
                    pbc_csr.cfg_account_9_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_9_pg_4.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_9_pg_4.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_9_pg_4.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_9_pg_4.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_9_pg_4.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_9_mtu_table.pg4(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_9_pg_4.write();
                    pbc_csr.cfg_account_9_mtu_table.write();
                    break;
                }
                case 5:
                {
                    pbc_csr.cfg_account_9_pg_5.read();
                    pbc_csr.cfg_account_9_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_9_pg_5.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_9_pg_5.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_9_pg_5.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_9_pg_5.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_9_pg_5.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_9_mtu_table.pg5(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_9_pg_5.write();
                    pbc_csr.cfg_account_9_mtu_table.write();
                    break;
                }
                case 6:
                {
                    pbc_csr.cfg_account_9_pg_6.read();
                    pbc_csr.cfg_account_9_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_9_pg_6.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_9_pg_6.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_9_pg_6.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_9_pg_6.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_9_pg_6.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_9_mtu_table.pg6(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_9_pg_6.write();
                    pbc_csr.cfg_account_9_mtu_table.write();
                    break;
                }
                case 7:
                {
                    pbc_csr.cfg_account_9_pg_7.read();
                    pbc_csr.cfg_account_9_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_9_pg_7.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_9_pg_7.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_9_pg_7.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_9_pg_7.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_9_pg_7.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_9_mtu_table.pg7(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_9_pg_7.write();
                    pbc_csr.cfg_account_9_mtu_table.write();
                    break;
                }
                case 8:
                {
                    pbc_csr.cfg_account_9_pg_8.read();
                    pbc_csr.cfg_account_9_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_9_pg_8.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_9_pg_8.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_9_pg_8.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_9_pg_8.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_9_pg_8.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_9_mtu_table.pg8(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_9_pg_8.write();
                    pbc_csr.cfg_account_9_mtu_table.write();
                    break;
                }
                case 9:
                {
                    pbc_csr.cfg_account_9_pg_9.read();
                    pbc_csr.cfg_account_9_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_9_pg_9.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_9_pg_9.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_9_pg_9.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_9_pg_9.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_9_pg_9.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_9_mtu_table.pg9(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_9_pg_9.write();
                    pbc_csr.cfg_account_9_mtu_table.write();
                    break;
                }
                case 10:
                {
                    pbc_csr.cfg_account_9_pg_10.read();
                    pbc_csr.cfg_account_9_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_9_pg_10.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_9_pg_10.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_9_pg_10.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_9_pg_10.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_9_pg_10.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_9_mtu_table.pg10(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_9_pg_10.write();
                    pbc_csr.cfg_account_9_mtu_table.write();
                    break;
                }
                case 11:
                {
                    pbc_csr.cfg_account_9_pg_11.read();
                    pbc_csr.cfg_account_9_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_9_pg_11.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_9_pg_11.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_9_pg_11.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_9_pg_11.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_9_pg_11.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_9_mtu_table.pg11(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_9_pg_11.write();
                    pbc_csr.cfg_account_9_mtu_table.write();
                    break;
                }
                case 12:
                {
                    pbc_csr.cfg_account_9_pg_12.read();
                    pbc_csr.cfg_account_9_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_9_pg_12.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_9_pg_12.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_9_pg_12.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_9_pg_12.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_9_pg_12.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_9_mtu_table.pg12(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_9_pg_12.write();
                    pbc_csr.cfg_account_9_mtu_table.write();
                    break;
                }
                case 13:
                {
                    pbc_csr.cfg_account_9_pg_13.read();
                    pbc_csr.cfg_account_9_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_9_pg_13.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_9_pg_13.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_9_pg_13.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_9_pg_13.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_9_pg_13.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_9_mtu_table.pg13(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_9_pg_13.write();
                    pbc_csr.cfg_account_9_mtu_table.write();
                    break;
                }
                case 14:
                {
                    pbc_csr.cfg_account_9_pg_14.read();
                    pbc_csr.cfg_account_9_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_9_pg_14.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_9_pg_14.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_9_pg_14.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_9_pg_14.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_9_pg_14.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_9_mtu_table.pg14(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_9_pg_14.write();
                    pbc_csr.cfg_account_9_mtu_table.write();
                    break;
                }
                case 15:
                {
                    pbc_csr.cfg_account_9_pg_15.read();
                    pbc_csr.cfg_account_9_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_9_pg_15.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_9_pg_15.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_9_pg_15.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_9_pg_15.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_9_pg_15.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_9_mtu_table.pg15(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_9_pg_15.write();
                    pbc_csr.cfg_account_9_mtu_table.write();
                    break;
                }
                default:
                    return HAL_RET_ERR;
            }
            /* Update and write the cos to PG mapping */
            pbc_csr.cfg_account_9_tc_to_pg.read();
            pbc_csr.cfg_account_9_tc_to_pg.table(cos_map_reg_val);
            pbc_csr.cfg_account_9_tc_to_pg.write();
            break;
        }
        case TM_PORT_EGRESS:
        {
            switch (pool_group) {
                case 0:
                {
                    pbc_csr.cfg_account_10_pg_0.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_0.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_0.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_0.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_0.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_0.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg0(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_0.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 1:
                {
                    pbc_csr.cfg_account_10_pg_1.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_1.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_1.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_1.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_1.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_1.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg1(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_1.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 2:
                {
                    pbc_csr.cfg_account_10_pg_2.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_2.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_2.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_2.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_2.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_2.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg2(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_2.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 3:
                {
                    pbc_csr.cfg_account_10_pg_3.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_3.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_3.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_3.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_3.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_3.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg3(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_3.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 4:
                {
                    pbc_csr.cfg_account_10_pg_4.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_4.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_4.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_4.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_4.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_4.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg4(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_4.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 5:
                {
                    pbc_csr.cfg_account_10_pg_5.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_5.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_5.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_5.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_5.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_5.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg5(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_5.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 6:
                {
                    pbc_csr.cfg_account_10_pg_6.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_6.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_6.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_6.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_6.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_6.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg6(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_6.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 7:
                {
                    pbc_csr.cfg_account_10_pg_7.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_7.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_7.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_7.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_7.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_7.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg7(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_7.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 8:
                {
                    pbc_csr.cfg_account_10_pg_8.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_8.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_8.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_8.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_8.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_8.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg8(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_8.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 9:
                {
                    pbc_csr.cfg_account_10_pg_9.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_9.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_9.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_9.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_9.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_9.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg9(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_9.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 10:
                {
                    pbc_csr.cfg_account_10_pg_10.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_10.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_10.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_10.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_10.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_10.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg10(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_10.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 11:
                {
                    pbc_csr.cfg_account_10_pg_11.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_11.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_11.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_11.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_11.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_11.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg11(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_11.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 12:
                {
                    pbc_csr.cfg_account_10_pg_12.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_12.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_12.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_12.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_12.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_12.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg12(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_12.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 13:
                {
                    pbc_csr.cfg_account_10_pg_13.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_13.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_13.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_13.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_13.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_13.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg13(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_13.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 14:
                {
                    pbc_csr.cfg_account_10_pg_14.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_14.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_14.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_14.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_14.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_14.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg14(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_14.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 15:
                {
                    pbc_csr.cfg_account_10_pg_15.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_15.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_15.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_15.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_15.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_15.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg15(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_15.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 16:
                {
                    pbc_csr.cfg_account_10_pg_16.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_16.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_16.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_16.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_16.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_16.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg16(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_16.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 17:
                {
                    pbc_csr.cfg_account_10_pg_17.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_17.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_17.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_17.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_17.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_17.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg17(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_17.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 18:
                {
                    pbc_csr.cfg_account_10_pg_18.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_18.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_18.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_18.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_18.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_18.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg18(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_18.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 19:
                {
                    pbc_csr.cfg_account_10_pg_19.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_19.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_19.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_19.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_19.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_19.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg19(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_19.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 20:
                {
                    pbc_csr.cfg_account_10_pg_20.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_20.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_20.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_20.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_20.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_20.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg20(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_20.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 21:
                {
                    pbc_csr.cfg_account_10_pg_21.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_21.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_21.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_21.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_21.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_21.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg21(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_21.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 22:
                {
                    pbc_csr.cfg_account_10_pg_22.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_22.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_22.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_22.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_22.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_22.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg22(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_22.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 23:
                {
                    pbc_csr.cfg_account_10_pg_23.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_23.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_23.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_23.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_23.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_23.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg23(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_23.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 24:
                {
                    pbc_csr.cfg_account_10_pg_24.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_24.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_24.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_24.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_24.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_24.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg24(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_24.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 25:
                {
                    pbc_csr.cfg_account_10_pg_25.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_25.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_25.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_25.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_25.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_25.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg25(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_25.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 26:
                {
                    pbc_csr.cfg_account_10_pg_26.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_26.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_26.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_26.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_26.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_26.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg26(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_26.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 27:
                {
                    pbc_csr.cfg_account_10_pg_27.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_27.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_27.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_27.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_27.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_27.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg27(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_27.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 28:
                {
                    pbc_csr.cfg_account_10_pg_28.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_28.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_28.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_28.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_28.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_28.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg28(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_28.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 29:
                {
                    pbc_csr.cfg_account_10_pg_29.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_29.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_29.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_29.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_29.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_29.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg29(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_29.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 30:
                {
                    pbc_csr.cfg_account_10_pg_30.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_30.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_30.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_30.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_30.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_30.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg30(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_30.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                case 31:
                {
                    pbc_csr.cfg_account_10_pg_31.read();
                    pbc_csr.cfg_account_10_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_10_pg_31.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_10_pg_31.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_10_pg_31.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_10_pg_31.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_10_pg_31.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_10_mtu_table.pg31(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_10_pg_31.write();
                    pbc_csr.cfg_account_10_mtu_table.write();
                    break;
                }
                default:
                    return HAL_RET_ERR;
            }
            /* Update and write the cos to PG mapping */
            pbc_csr.cfg_account_10_tc_to_pg.read();
            pbc_csr.cfg_account_10_tc_to_pg.table(cos_map_reg_val);
            pbc_csr.cfg_account_10_tc_to_pg.write();
            break;
        }
        case TM_PORT_INGRESS:
        {
            switch (pool_group) {
                case 0:
                {
                    pbc_csr.cfg_account_11_pg_0.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_0.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_0.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_0.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_0.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_0.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg0(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_0.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 1:
                {
                    pbc_csr.cfg_account_11_pg_1.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_1.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_1.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_1.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_1.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_1.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg1(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_1.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 2:
                {
                    pbc_csr.cfg_account_11_pg_2.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_2.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_2.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_2.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_2.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_2.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg2(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_2.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 3:
                {
                    pbc_csr.cfg_account_11_pg_3.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_3.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_3.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_3.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_3.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_3.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg3(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_3.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 4:
                {
                    pbc_csr.cfg_account_11_pg_4.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_4.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_4.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_4.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_4.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_4.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg4(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_4.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 5:
                {
                    pbc_csr.cfg_account_11_pg_5.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_5.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_5.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_5.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_5.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_5.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg5(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_5.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 6:
                {
                    pbc_csr.cfg_account_11_pg_6.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_6.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_6.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_6.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_6.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_6.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg6(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_6.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 7:
                {
                    pbc_csr.cfg_account_11_pg_7.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_7.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_7.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_7.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_7.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_7.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg7(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_7.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 8:
                {
                    pbc_csr.cfg_account_11_pg_8.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_8.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_8.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_8.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_8.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_8.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg8(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_8.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 9:
                {
                    pbc_csr.cfg_account_11_pg_9.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_9.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_9.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_9.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_9.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_9.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg9(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_9.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 10:
                {
                    pbc_csr.cfg_account_11_pg_10.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_10.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_10.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_10.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_10.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_10.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg10(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_10.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 11:
                {
                    pbc_csr.cfg_account_11_pg_11.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_11.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_11.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_11.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_11.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_11.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg11(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_11.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 12:
                {
                    pbc_csr.cfg_account_11_pg_12.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_12.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_12.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_12.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_12.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_12.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg12(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_12.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 13:
                {
                    pbc_csr.cfg_account_11_pg_13.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_13.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_13.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_13.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_13.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_13.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg13(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_13.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 14:
                {
                    pbc_csr.cfg_account_11_pg_14.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_14.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_14.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_14.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_14.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_14.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg14(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_14.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 15:
                {
                    pbc_csr.cfg_account_11_pg_15.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_15.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_15.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_15.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_15.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_15.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg15(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_15.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 16:
                {
                    pbc_csr.cfg_account_11_pg_16.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_16.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_16.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_16.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_16.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_16.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg16(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_16.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 17:
                {
                    pbc_csr.cfg_account_11_pg_17.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_17.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_17.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_17.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_17.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_17.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg17(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_17.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 18:
                {
                    pbc_csr.cfg_account_11_pg_18.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_18.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_18.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_18.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_18.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_18.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg18(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_18.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 19:
                {
                    pbc_csr.cfg_account_11_pg_19.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_19.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_19.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_19.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_19.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_19.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg19(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_19.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 20:
                {
                    pbc_csr.cfg_account_11_pg_20.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_20.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_20.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_20.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_20.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_20.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg20(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_20.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 21:
                {
                    pbc_csr.cfg_account_11_pg_21.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_21.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_21.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_21.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_21.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_21.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg21(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_21.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 22:
                {
                    pbc_csr.cfg_account_11_pg_22.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_22.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_22.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_22.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_22.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_22.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg22(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_22.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 23:
                {
                    pbc_csr.cfg_account_11_pg_23.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_23.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_23.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_23.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_23.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_23.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg23(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_23.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 24:
                {
                    pbc_csr.cfg_account_11_pg_24.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_24.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_24.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_24.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_24.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_24.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg24(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_24.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 25:
                {
                    pbc_csr.cfg_account_11_pg_25.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_25.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_25.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_25.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_25.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_25.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg25(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_25.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 26:
                {
                    pbc_csr.cfg_account_11_pg_26.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_26.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_26.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_26.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_26.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_26.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg26(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_26.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 27:
                {
                    pbc_csr.cfg_account_11_pg_27.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_27.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_27.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_27.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_27.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_27.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg27(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_27.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 28:
                {
                    pbc_csr.cfg_account_11_pg_28.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_28.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_28.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_28.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_28.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_28.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg28(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_28.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 29:
                {
                    pbc_csr.cfg_account_11_pg_29.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_29.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_29.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_29.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_29.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_29.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg29(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_29.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 30:
                {
                    pbc_csr.cfg_account_11_pg_30.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_30.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_30.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_30.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_30.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_30.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg30(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_30.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                case 31:
                {
                    pbc_csr.cfg_account_11_pg_31.read();
                    pbc_csr.cfg_account_11_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.cfg_account_11_pg_31.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_11_pg_31.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_11_pg_31.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_11_pg_31.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_11_pg_31.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_11_mtu_table.pg31(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_11_pg_31.write();
                    pbc_csr.cfg_account_11_mtu_table.write();
                    break;
                }
                default:
                    return HAL_RET_ERR;
            }
            /* Update and write the cos to PG mapping */
            pbc_csr.cfg_account_11_tc_to_pg.read();
            pbc_csr.cfg_account_11_tc_to_pg.table(cos_map_reg_val);
            pbc_csr.cfg_account_11_tc_to_pg.write();
            break;
        }
        default:
            return HAL_RET_ERR;
    }

#endif
    HAL_TRACE_DEBUG("CAPRI-TM::{}: Updated the pool group {} on port {}",
                    __func__, pool_group, port);

    return HAL_RET_OK;
}

hal_ret_t 
capri_tm_scheduler_map_update(uint32_t port,
                              tm_scheduler_map_t *sch_map) 
{
    if (!is_valid_tm_port(port)) {
        HAL_TRACE_ERR("CAPRI-TM::{}: {} is not a valid TM port", 
                      __func__, port);
        return HAL_RET_INVALID_ARG;
    }
#ifndef HAL_GTEST
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    uint32_t i;

    cpp_int l1_node_vals[HAL_TM_COUNT_L1_NODES];
    cpp_int l2_node_vals[HAL_TM_COUNT_L2_NODES];
    cpp_int l1_strict_val;
    cpp_int l2_strict_val;

    tm_queue_node_t *l0_node, *l1_node;

    for (i = 0; i < HAL_TM_COUNT_L0_NODES; i++) {
        l0_node = &sch_map->l0_nodes[i];
        if (!l0_node->in_use) {
            continue;
        }

        HAL_ASSERT(l0_node->parent_node < HAL_TM_COUNT_L1_NODES);
        if (sch_map->l1_nodes[l0_node->parent_node].in_use == false) {
            HAL_TRACE_ERR("CAPRI-TM::{}: L1 node {} referred by Queue {} is not in use"
                          " on port {}", 
                          __func__, l0_node->parent_node, i, port);
            return HAL_RET_INVALID_ARG;
        }

        l1_node_vals[l0_node->parent_node] |= (1<<i);
        if (l0_node->sched_type == TM_SCHED_TYPE_STRICT) {
            l1_strict_val |= (1<<i);
        }
    }

    for (i = 0; i < HAL_TM_COUNT_L1_NODES; i++) {
        l1_node = &sch_map->l1_nodes[i];
        if (!l1_node->in_use) {
            continue;
        }

        HAL_ASSERT(l1_node->parent_node < HAL_TM_COUNT_L2_NODES);
        l2_node_vals[l1_node->parent_node] |= (1<<i);
        if (l1_node->sched_type == TM_SCHED_TYPE_STRICT) {
            l2_strict_val |= (1<<i);
        }
    }

    switch(port) {
        case TM_PORT_UPLINK_0:
        {
            pbc_csr.cfg_oq_0_arb_l1_selection.read();
            pbc_csr.cfg_oq_0_arb_l2_selection.read();
            pbc_csr.cfg_oq_0_arb_l1_strict.read();
            pbc_csr.cfg_oq_0_arb_l2_strict.read();
            pbc_csr.cfg_oq_0_arb_l1_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_0_arb_l1_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_0_arb_l1_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_0_arb_l1_selection.node_3(l1_node_vals[3]);
            pbc_csr.cfg_oq_0_arb_l1_selection.node_4(l1_node_vals[4]);
            pbc_csr.cfg_oq_0_arb_l1_selection.node_5(l1_node_vals[5]);
            pbc_csr.cfg_oq_0_arb_l1_selection.node_6(l1_node_vals[6]);
            pbc_csr.cfg_oq_0_arb_l1_selection.node_7(l1_node_vals[7]);
            pbc_csr.cfg_oq_0_arb_l1_selection.node_8(l1_node_vals[8]);
            pbc_csr.cfg_oq_0_arb_l1_selection.node_9(l1_node_vals[9]);
            pbc_csr.cfg_oq_0_arb_l1_selection.node_10(l1_node_vals[10]);
            pbc_csr.cfg_oq_0_arb_l1_selection.node_11(l1_node_vals[11]);
            pbc_csr.cfg_oq_0_arb_l1_selection.node_12(l1_node_vals[12]);
            pbc_csr.cfg_oq_0_arb_l1_selection.node_13(l1_node_vals[13]);
            pbc_csr.cfg_oq_0_arb_l1_selection.node_14(l1_node_vals[14]);
            pbc_csr.cfg_oq_0_arb_l1_selection.node_15(l1_node_vals[15]);

            pbc_csr.cfg_oq_0_arb_l2_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_0_arb_l2_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_0_arb_l2_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_0_arb_l2_selection.node_3(l1_node_vals[3]);

            pbc_csr.cfg_oq_0_arb_l1_strict.priority(l1_strict_val);
            pbc_csr.cfg_oq_0_arb_l2_strict.priority(l2_strict_val);
            /* Write all the L1 and L2 registers */
            pbc_csr.cfg_oq_0_arb_l1_selection.write();
            pbc_csr.cfg_oq_0_arb_l2_selection.write();
            pbc_csr.cfg_oq_0_arb_l1_strict.write();
            pbc_csr.cfg_oq_0_arb_l2_strict.write();
            break;
        }
        case TM_PORT_UPLINK_1:
        {
            pbc_csr.cfg_oq_1_arb_l1_selection.read();
            pbc_csr.cfg_oq_1_arb_l2_selection.read();
            pbc_csr.cfg_oq_1_arb_l1_strict.read();
            pbc_csr.cfg_oq_1_arb_l2_strict.read();
            pbc_csr.cfg_oq_1_arb_l1_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_1_arb_l1_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_1_arb_l1_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_1_arb_l1_selection.node_3(l1_node_vals[3]);
            pbc_csr.cfg_oq_1_arb_l1_selection.node_4(l1_node_vals[4]);
            pbc_csr.cfg_oq_1_arb_l1_selection.node_5(l1_node_vals[5]);
            pbc_csr.cfg_oq_1_arb_l1_selection.node_6(l1_node_vals[6]);
            pbc_csr.cfg_oq_1_arb_l1_selection.node_7(l1_node_vals[7]);
            pbc_csr.cfg_oq_1_arb_l1_selection.node_8(l1_node_vals[8]);
            pbc_csr.cfg_oq_1_arb_l1_selection.node_9(l1_node_vals[9]);
            pbc_csr.cfg_oq_1_arb_l1_selection.node_10(l1_node_vals[10]);
            pbc_csr.cfg_oq_1_arb_l1_selection.node_11(l1_node_vals[11]);
            pbc_csr.cfg_oq_1_arb_l1_selection.node_12(l1_node_vals[12]);
            pbc_csr.cfg_oq_1_arb_l1_selection.node_13(l1_node_vals[13]);
            pbc_csr.cfg_oq_1_arb_l1_selection.node_14(l1_node_vals[14]);
            pbc_csr.cfg_oq_1_arb_l1_selection.node_15(l1_node_vals[15]);

            pbc_csr.cfg_oq_1_arb_l2_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_1_arb_l2_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_1_arb_l2_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_1_arb_l2_selection.node_3(l1_node_vals[3]);

            pbc_csr.cfg_oq_1_arb_l1_strict.priority(l1_strict_val);
            pbc_csr.cfg_oq_1_arb_l2_strict.priority(l2_strict_val);
            /* Write all the L1 and L2 registers */
            pbc_csr.cfg_oq_1_arb_l1_selection.write();
            pbc_csr.cfg_oq_1_arb_l2_selection.write();
            pbc_csr.cfg_oq_1_arb_l1_strict.write();
            pbc_csr.cfg_oq_1_arb_l2_strict.write();
            break;
        }
        case TM_PORT_UPLINK_2:
        {
            pbc_csr.cfg_oq_2_arb_l1_selection.read();
            pbc_csr.cfg_oq_2_arb_l2_selection.read();
            pbc_csr.cfg_oq_2_arb_l1_strict.read();
            pbc_csr.cfg_oq_2_arb_l2_strict.read();
            pbc_csr.cfg_oq_2_arb_l1_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_2_arb_l1_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_2_arb_l1_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_2_arb_l1_selection.node_3(l1_node_vals[3]);
            pbc_csr.cfg_oq_2_arb_l1_selection.node_4(l1_node_vals[4]);
            pbc_csr.cfg_oq_2_arb_l1_selection.node_5(l1_node_vals[5]);
            pbc_csr.cfg_oq_2_arb_l1_selection.node_6(l1_node_vals[6]);
            pbc_csr.cfg_oq_2_arb_l1_selection.node_7(l1_node_vals[7]);
            pbc_csr.cfg_oq_2_arb_l1_selection.node_8(l1_node_vals[8]);
            pbc_csr.cfg_oq_2_arb_l1_selection.node_9(l1_node_vals[9]);
            pbc_csr.cfg_oq_2_arb_l1_selection.node_10(l1_node_vals[10]);
            pbc_csr.cfg_oq_2_arb_l1_selection.node_11(l1_node_vals[11]);
            pbc_csr.cfg_oq_2_arb_l1_selection.node_12(l1_node_vals[12]);
            pbc_csr.cfg_oq_2_arb_l1_selection.node_13(l1_node_vals[13]);
            pbc_csr.cfg_oq_2_arb_l1_selection.node_14(l1_node_vals[14]);
            pbc_csr.cfg_oq_2_arb_l1_selection.node_15(l1_node_vals[15]);

            pbc_csr.cfg_oq_2_arb_l2_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_2_arb_l2_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_2_arb_l2_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_2_arb_l2_selection.node_3(l1_node_vals[3]);

            pbc_csr.cfg_oq_2_arb_l1_strict.priority(l1_strict_val);
            pbc_csr.cfg_oq_2_arb_l2_strict.priority(l2_strict_val);
            /* Write all the L1 and L2 registers */
            pbc_csr.cfg_oq_2_arb_l1_selection.write();
            pbc_csr.cfg_oq_2_arb_l2_selection.write();
            pbc_csr.cfg_oq_2_arb_l1_strict.write();
            pbc_csr.cfg_oq_2_arb_l2_strict.write();
            break;
        }
        case TM_PORT_UPLINK_3:
        {
            pbc_csr.cfg_oq_3_arb_l1_selection.read();
            pbc_csr.cfg_oq_3_arb_l2_selection.read();
            pbc_csr.cfg_oq_3_arb_l1_strict.read();
            pbc_csr.cfg_oq_3_arb_l2_strict.read();
            pbc_csr.cfg_oq_3_arb_l1_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_3_arb_l1_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_3_arb_l1_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_3_arb_l1_selection.node_3(l1_node_vals[3]);
            pbc_csr.cfg_oq_3_arb_l1_selection.node_4(l1_node_vals[4]);
            pbc_csr.cfg_oq_3_arb_l1_selection.node_5(l1_node_vals[5]);
            pbc_csr.cfg_oq_3_arb_l1_selection.node_6(l1_node_vals[6]);
            pbc_csr.cfg_oq_3_arb_l1_selection.node_7(l1_node_vals[7]);
            pbc_csr.cfg_oq_3_arb_l1_selection.node_8(l1_node_vals[8]);
            pbc_csr.cfg_oq_3_arb_l1_selection.node_9(l1_node_vals[9]);
            pbc_csr.cfg_oq_3_arb_l1_selection.node_10(l1_node_vals[10]);
            pbc_csr.cfg_oq_3_arb_l1_selection.node_11(l1_node_vals[11]);
            pbc_csr.cfg_oq_3_arb_l1_selection.node_12(l1_node_vals[12]);
            pbc_csr.cfg_oq_3_arb_l1_selection.node_13(l1_node_vals[13]);
            pbc_csr.cfg_oq_3_arb_l1_selection.node_14(l1_node_vals[14]);
            pbc_csr.cfg_oq_3_arb_l1_selection.node_15(l1_node_vals[15]);

            pbc_csr.cfg_oq_3_arb_l2_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_3_arb_l2_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_3_arb_l2_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_3_arb_l2_selection.node_3(l1_node_vals[3]);

            pbc_csr.cfg_oq_3_arb_l1_strict.priority(l1_strict_val);
            pbc_csr.cfg_oq_3_arb_l2_strict.priority(l2_strict_val);
            /* Write all the L1 and L2 registers */
            pbc_csr.cfg_oq_3_arb_l1_selection.write();
            pbc_csr.cfg_oq_3_arb_l2_selection.write();
            pbc_csr.cfg_oq_3_arb_l1_strict.write();
            pbc_csr.cfg_oq_3_arb_l2_strict.write();
            break;
        }
        case TM_PORT_UPLINK_4:
        {
            pbc_csr.cfg_oq_4_arb_l1_selection.read();
            pbc_csr.cfg_oq_4_arb_l2_selection.read();
            pbc_csr.cfg_oq_4_arb_l1_strict.read();
            pbc_csr.cfg_oq_4_arb_l2_strict.read();
            pbc_csr.cfg_oq_4_arb_l1_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_4_arb_l1_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_4_arb_l1_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_4_arb_l1_selection.node_3(l1_node_vals[3]);
            pbc_csr.cfg_oq_4_arb_l1_selection.node_4(l1_node_vals[4]);
            pbc_csr.cfg_oq_4_arb_l1_selection.node_5(l1_node_vals[5]);
            pbc_csr.cfg_oq_4_arb_l1_selection.node_6(l1_node_vals[6]);
            pbc_csr.cfg_oq_4_arb_l1_selection.node_7(l1_node_vals[7]);
            pbc_csr.cfg_oq_4_arb_l1_selection.node_8(l1_node_vals[8]);
            pbc_csr.cfg_oq_4_arb_l1_selection.node_9(l1_node_vals[9]);
            pbc_csr.cfg_oq_4_arb_l1_selection.node_10(l1_node_vals[10]);
            pbc_csr.cfg_oq_4_arb_l1_selection.node_11(l1_node_vals[11]);
            pbc_csr.cfg_oq_4_arb_l1_selection.node_12(l1_node_vals[12]);
            pbc_csr.cfg_oq_4_arb_l1_selection.node_13(l1_node_vals[13]);
            pbc_csr.cfg_oq_4_arb_l1_selection.node_14(l1_node_vals[14]);
            pbc_csr.cfg_oq_4_arb_l1_selection.node_15(l1_node_vals[15]);

            pbc_csr.cfg_oq_4_arb_l2_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_4_arb_l2_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_4_arb_l2_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_4_arb_l2_selection.node_3(l1_node_vals[3]);

            pbc_csr.cfg_oq_4_arb_l1_strict.priority(l1_strict_val);
            pbc_csr.cfg_oq_4_arb_l2_strict.priority(l2_strict_val);
            /* Write all the L1 and L2 registers */
            pbc_csr.cfg_oq_4_arb_l1_selection.write();
            pbc_csr.cfg_oq_4_arb_l2_selection.write();
            pbc_csr.cfg_oq_4_arb_l1_strict.write();
            pbc_csr.cfg_oq_4_arb_l2_strict.write();
            break;
        }
        case TM_PORT_UPLINK_5:
        {
            pbc_csr.cfg_oq_5_arb_l1_selection.read();
            pbc_csr.cfg_oq_5_arb_l2_selection.read();
            pbc_csr.cfg_oq_5_arb_l1_strict.read();
            pbc_csr.cfg_oq_5_arb_l2_strict.read();
            pbc_csr.cfg_oq_5_arb_l1_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_5_arb_l1_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_5_arb_l1_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_5_arb_l1_selection.node_3(l1_node_vals[3]);
            pbc_csr.cfg_oq_5_arb_l1_selection.node_4(l1_node_vals[4]);
            pbc_csr.cfg_oq_5_arb_l1_selection.node_5(l1_node_vals[5]);
            pbc_csr.cfg_oq_5_arb_l1_selection.node_6(l1_node_vals[6]);
            pbc_csr.cfg_oq_5_arb_l1_selection.node_7(l1_node_vals[7]);
            pbc_csr.cfg_oq_5_arb_l1_selection.node_8(l1_node_vals[8]);
            pbc_csr.cfg_oq_5_arb_l1_selection.node_9(l1_node_vals[9]);
            pbc_csr.cfg_oq_5_arb_l1_selection.node_10(l1_node_vals[10]);
            pbc_csr.cfg_oq_5_arb_l1_selection.node_11(l1_node_vals[11]);
            pbc_csr.cfg_oq_5_arb_l1_selection.node_12(l1_node_vals[12]);
            pbc_csr.cfg_oq_5_arb_l1_selection.node_13(l1_node_vals[13]);
            pbc_csr.cfg_oq_5_arb_l1_selection.node_14(l1_node_vals[14]);
            pbc_csr.cfg_oq_5_arb_l1_selection.node_15(l1_node_vals[15]);

            pbc_csr.cfg_oq_5_arb_l2_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_5_arb_l2_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_5_arb_l2_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_5_arb_l2_selection.node_3(l1_node_vals[3]);

            pbc_csr.cfg_oq_5_arb_l1_strict.priority(l1_strict_val);
            pbc_csr.cfg_oq_5_arb_l2_strict.priority(l2_strict_val);
            /* Write all the L1 and L2 registers */
            pbc_csr.cfg_oq_5_arb_l1_selection.write();
            pbc_csr.cfg_oq_5_arb_l2_selection.write();
            pbc_csr.cfg_oq_5_arb_l1_strict.write();
            pbc_csr.cfg_oq_5_arb_l2_strict.write();
            break;
        }
        case TM_PORT_UPLINK_6:
        {
            pbc_csr.cfg_oq_6_arb_l1_selection.read();
            pbc_csr.cfg_oq_6_arb_l2_selection.read();
            pbc_csr.cfg_oq_6_arb_l1_strict.read();
            pbc_csr.cfg_oq_6_arb_l2_strict.read();
            pbc_csr.cfg_oq_6_arb_l1_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_6_arb_l1_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_6_arb_l1_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_6_arb_l1_selection.node_3(l1_node_vals[3]);
            pbc_csr.cfg_oq_6_arb_l1_selection.node_4(l1_node_vals[4]);
            pbc_csr.cfg_oq_6_arb_l1_selection.node_5(l1_node_vals[5]);
            pbc_csr.cfg_oq_6_arb_l1_selection.node_6(l1_node_vals[6]);
            pbc_csr.cfg_oq_6_arb_l1_selection.node_7(l1_node_vals[7]);
            pbc_csr.cfg_oq_6_arb_l1_selection.node_8(l1_node_vals[8]);
            pbc_csr.cfg_oq_6_arb_l1_selection.node_9(l1_node_vals[9]);
            pbc_csr.cfg_oq_6_arb_l1_selection.node_10(l1_node_vals[10]);
            pbc_csr.cfg_oq_6_arb_l1_selection.node_11(l1_node_vals[11]);
            pbc_csr.cfg_oq_6_arb_l1_selection.node_12(l1_node_vals[12]);
            pbc_csr.cfg_oq_6_arb_l1_selection.node_13(l1_node_vals[13]);
            pbc_csr.cfg_oq_6_arb_l1_selection.node_14(l1_node_vals[14]);
            pbc_csr.cfg_oq_6_arb_l1_selection.node_15(l1_node_vals[15]);

            pbc_csr.cfg_oq_6_arb_l2_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_6_arb_l2_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_6_arb_l2_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_6_arb_l2_selection.node_3(l1_node_vals[3]);

            pbc_csr.cfg_oq_6_arb_l1_strict.priority(l1_strict_val);
            pbc_csr.cfg_oq_6_arb_l2_strict.priority(l2_strict_val);
            /* Write all the L1 and L2 registers */
            pbc_csr.cfg_oq_6_arb_l1_selection.write();
            pbc_csr.cfg_oq_6_arb_l2_selection.write();
            pbc_csr.cfg_oq_6_arb_l1_strict.write();
            pbc_csr.cfg_oq_6_arb_l2_strict.write();
            break;
        }
        case TM_PORT_UPLINK_7:
        {
            pbc_csr.cfg_oq_7_arb_l1_selection.read();
            pbc_csr.cfg_oq_7_arb_l2_selection.read();
            pbc_csr.cfg_oq_7_arb_l1_strict.read();
            pbc_csr.cfg_oq_7_arb_l2_strict.read();
            pbc_csr.cfg_oq_7_arb_l1_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_7_arb_l1_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_7_arb_l1_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_7_arb_l1_selection.node_3(l1_node_vals[3]);
            pbc_csr.cfg_oq_7_arb_l1_selection.node_4(l1_node_vals[4]);
            pbc_csr.cfg_oq_7_arb_l1_selection.node_5(l1_node_vals[5]);
            pbc_csr.cfg_oq_7_arb_l1_selection.node_6(l1_node_vals[6]);
            pbc_csr.cfg_oq_7_arb_l1_selection.node_7(l1_node_vals[7]);
            pbc_csr.cfg_oq_7_arb_l1_selection.node_8(l1_node_vals[8]);
            pbc_csr.cfg_oq_7_arb_l1_selection.node_9(l1_node_vals[9]);
            pbc_csr.cfg_oq_7_arb_l1_selection.node_10(l1_node_vals[10]);
            pbc_csr.cfg_oq_7_arb_l1_selection.node_11(l1_node_vals[11]);
            pbc_csr.cfg_oq_7_arb_l1_selection.node_12(l1_node_vals[12]);
            pbc_csr.cfg_oq_7_arb_l1_selection.node_13(l1_node_vals[13]);
            pbc_csr.cfg_oq_7_arb_l1_selection.node_14(l1_node_vals[14]);
            pbc_csr.cfg_oq_7_arb_l1_selection.node_15(l1_node_vals[15]);

            pbc_csr.cfg_oq_7_arb_l2_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_7_arb_l2_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_7_arb_l2_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_7_arb_l2_selection.node_3(l1_node_vals[3]);

            pbc_csr.cfg_oq_7_arb_l1_strict.priority(l1_strict_val);
            pbc_csr.cfg_oq_7_arb_l2_strict.priority(l2_strict_val);
            /* Write all the L1 and L2 registers */
            pbc_csr.cfg_oq_7_arb_l1_selection.write();
            pbc_csr.cfg_oq_7_arb_l2_selection.write();
            pbc_csr.cfg_oq_7_arb_l1_strict.write();
            pbc_csr.cfg_oq_7_arb_l2_strict.write();
            break;
        }
        case TM_PORT_NCSI:
        {
            pbc_csr.cfg_oq_8_arb_l1_selection.read();
            pbc_csr.cfg_oq_8_arb_l2_selection.read();
            pbc_csr.cfg_oq_8_arb_l1_strict.read();
            pbc_csr.cfg_oq_8_arb_l2_strict.read();
            pbc_csr.cfg_oq_8_arb_l1_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_8_arb_l1_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_8_arb_l1_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_8_arb_l1_selection.node_3(l1_node_vals[3]);
            pbc_csr.cfg_oq_8_arb_l1_selection.node_4(l1_node_vals[4]);
            pbc_csr.cfg_oq_8_arb_l1_selection.node_5(l1_node_vals[5]);
            pbc_csr.cfg_oq_8_arb_l1_selection.node_6(l1_node_vals[6]);
            pbc_csr.cfg_oq_8_arb_l1_selection.node_7(l1_node_vals[7]);
            pbc_csr.cfg_oq_8_arb_l1_selection.node_8(l1_node_vals[8]);
            pbc_csr.cfg_oq_8_arb_l1_selection.node_9(l1_node_vals[9]);
            pbc_csr.cfg_oq_8_arb_l1_selection.node_10(l1_node_vals[10]);
            pbc_csr.cfg_oq_8_arb_l1_selection.node_11(l1_node_vals[11]);
            pbc_csr.cfg_oq_8_arb_l1_selection.node_12(l1_node_vals[12]);
            pbc_csr.cfg_oq_8_arb_l1_selection.node_13(l1_node_vals[13]);
            pbc_csr.cfg_oq_8_arb_l1_selection.node_14(l1_node_vals[14]);
            pbc_csr.cfg_oq_8_arb_l1_selection.node_15(l1_node_vals[15]);

            pbc_csr.cfg_oq_8_arb_l2_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_8_arb_l2_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_8_arb_l2_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_8_arb_l2_selection.node_3(l1_node_vals[3]);

            pbc_csr.cfg_oq_8_arb_l1_strict.priority(l1_strict_val);
            pbc_csr.cfg_oq_8_arb_l2_strict.priority(l2_strict_val);
            /* Write all the L1 and L2 registers */
            pbc_csr.cfg_oq_8_arb_l1_selection.write();
            pbc_csr.cfg_oq_8_arb_l2_selection.write();
            pbc_csr.cfg_oq_8_arb_l1_strict.write();
            pbc_csr.cfg_oq_8_arb_l2_strict.write();
            break;
        }
        case TM_PORT_DMA:
        {
            pbc_csr.cfg_oq_9_arb_l1_selection.read();
            pbc_csr.cfg_oq_9_arb_l2_selection.read();
            pbc_csr.cfg_oq_9_arb_l1_strict.read();
            pbc_csr.cfg_oq_9_arb_l2_strict.read();
            pbc_csr.cfg_oq_9_arb_l1_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_9_arb_l1_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_9_arb_l1_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_9_arb_l1_selection.node_3(l1_node_vals[3]);
            pbc_csr.cfg_oq_9_arb_l1_selection.node_4(l1_node_vals[4]);
            pbc_csr.cfg_oq_9_arb_l1_selection.node_5(l1_node_vals[5]);
            pbc_csr.cfg_oq_9_arb_l1_selection.node_6(l1_node_vals[6]);
            pbc_csr.cfg_oq_9_arb_l1_selection.node_7(l1_node_vals[7]);
            pbc_csr.cfg_oq_9_arb_l1_selection.node_8(l1_node_vals[8]);
            pbc_csr.cfg_oq_9_arb_l1_selection.node_9(l1_node_vals[9]);
            pbc_csr.cfg_oq_9_arb_l1_selection.node_10(l1_node_vals[10]);
            pbc_csr.cfg_oq_9_arb_l1_selection.node_11(l1_node_vals[11]);
            pbc_csr.cfg_oq_9_arb_l1_selection.node_12(l1_node_vals[12]);
            pbc_csr.cfg_oq_9_arb_l1_selection.node_13(l1_node_vals[13]);
            pbc_csr.cfg_oq_9_arb_l1_selection.node_14(l1_node_vals[14]);
            pbc_csr.cfg_oq_9_arb_l1_selection.node_15(l1_node_vals[15]);

            pbc_csr.cfg_oq_9_arb_l2_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_9_arb_l2_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_9_arb_l2_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_9_arb_l2_selection.node_3(l1_node_vals[3]);

            pbc_csr.cfg_oq_9_arb_l1_strict.priority(l1_strict_val);
            pbc_csr.cfg_oq_9_arb_l2_strict.priority(l2_strict_val);
            /* Write all the L1 and L2 registers */
            pbc_csr.cfg_oq_9_arb_l1_selection.write();
            pbc_csr.cfg_oq_9_arb_l2_selection.write();
            pbc_csr.cfg_oq_9_arb_l1_strict.write();
            pbc_csr.cfg_oq_9_arb_l2_strict.write();
            break;
        }
        case TM_PORT_EGRESS:
        {
            pbc_csr.cfg_oq_10_arb_l1_selection.read();
            pbc_csr.cfg_oq_10_arb_l2_selection.read();
            pbc_csr.cfg_oq_10_arb_l1_strict.read();
            pbc_csr.cfg_oq_10_arb_l2_strict.read();
            pbc_csr.cfg_oq_10_arb_l1_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_10_arb_l1_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_10_arb_l1_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_10_arb_l1_selection.node_3(l1_node_vals[3]);
            pbc_csr.cfg_oq_10_arb_l1_selection.node_4(l1_node_vals[4]);
            pbc_csr.cfg_oq_10_arb_l1_selection.node_5(l1_node_vals[5]);
            pbc_csr.cfg_oq_10_arb_l1_selection.node_6(l1_node_vals[6]);
            pbc_csr.cfg_oq_10_arb_l1_selection.node_7(l1_node_vals[7]);
            pbc_csr.cfg_oq_10_arb_l1_selection.node_8(l1_node_vals[8]);
            pbc_csr.cfg_oq_10_arb_l1_selection.node_9(l1_node_vals[9]);
            pbc_csr.cfg_oq_10_arb_l1_selection.node_10(l1_node_vals[10]);
            pbc_csr.cfg_oq_10_arb_l1_selection.node_11(l1_node_vals[11]);
            pbc_csr.cfg_oq_10_arb_l1_selection.node_12(l1_node_vals[12]);
            pbc_csr.cfg_oq_10_arb_l1_selection.node_13(l1_node_vals[13]);
            pbc_csr.cfg_oq_10_arb_l1_selection.node_14(l1_node_vals[14]);
            pbc_csr.cfg_oq_10_arb_l1_selection.node_15(l1_node_vals[15]);

            pbc_csr.cfg_oq_10_arb_l2_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_10_arb_l2_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_10_arb_l2_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_10_arb_l2_selection.node_3(l1_node_vals[3]);

            pbc_csr.cfg_oq_10_arb_l1_strict.priority(l1_strict_val);
            pbc_csr.cfg_oq_10_arb_l2_strict.priority(l2_strict_val);
            /* Write all the L1 and L2 registers */
            pbc_csr.cfg_oq_10_arb_l1_selection.write();
            pbc_csr.cfg_oq_10_arb_l2_selection.write();
            pbc_csr.cfg_oq_10_arb_l1_strict.write();
            pbc_csr.cfg_oq_10_arb_l2_strict.write();
            break;
        }
        case TM_PORT_INGRESS:
        {
            pbc_csr.cfg_oq_11_arb_l1_selection.read();
            pbc_csr.cfg_oq_11_arb_l2_selection.read();
            pbc_csr.cfg_oq_11_arb_l1_strict.read();
            pbc_csr.cfg_oq_11_arb_l2_strict.read();
            pbc_csr.cfg_oq_11_arb_l1_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_11_arb_l1_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_11_arb_l1_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_11_arb_l1_selection.node_3(l1_node_vals[3]);
            pbc_csr.cfg_oq_11_arb_l1_selection.node_4(l1_node_vals[4]);
            pbc_csr.cfg_oq_11_arb_l1_selection.node_5(l1_node_vals[5]);
            pbc_csr.cfg_oq_11_arb_l1_selection.node_6(l1_node_vals[6]);
            pbc_csr.cfg_oq_11_arb_l1_selection.node_7(l1_node_vals[7]);
            pbc_csr.cfg_oq_11_arb_l1_selection.node_8(l1_node_vals[8]);
            pbc_csr.cfg_oq_11_arb_l1_selection.node_9(l1_node_vals[9]);
            pbc_csr.cfg_oq_11_arb_l1_selection.node_10(l1_node_vals[10]);
            pbc_csr.cfg_oq_11_arb_l1_selection.node_11(l1_node_vals[11]);
            pbc_csr.cfg_oq_11_arb_l1_selection.node_12(l1_node_vals[12]);
            pbc_csr.cfg_oq_11_arb_l1_selection.node_13(l1_node_vals[13]);
            pbc_csr.cfg_oq_11_arb_l1_selection.node_14(l1_node_vals[14]);
            pbc_csr.cfg_oq_11_arb_l1_selection.node_15(l1_node_vals[15]);

            pbc_csr.cfg_oq_11_arb_l2_selection.node_0(l1_node_vals[0]);
            pbc_csr.cfg_oq_11_arb_l2_selection.node_1(l1_node_vals[1]);
            pbc_csr.cfg_oq_11_arb_l2_selection.node_2(l1_node_vals[2]);
            pbc_csr.cfg_oq_11_arb_l2_selection.node_3(l1_node_vals[3]);

            pbc_csr.cfg_oq_11_arb_l1_strict.priority(l1_strict_val);
            pbc_csr.cfg_oq_11_arb_l2_strict.priority(l2_strict_val);
            /* Write all the L1 and L2 registers */
            pbc_csr.cfg_oq_11_arb_l1_selection.write();
            pbc_csr.cfg_oq_11_arb_l2_selection.write();
            pbc_csr.cfg_oq_11_arb_l1_strict.write();
            pbc_csr.cfg_oq_11_arb_l2_strict.write();
            break;
        }
        default:
            return HAL_RET_ERR;
    }

#endif
    HAL_TRACE_DEBUG("CAPRI-TM::{}: Updated the output queue scheduler on port {}",
                    __func__, port);

    return HAL_RET_OK;
}

/* Program the lif value on an uplink port */
hal_ret_t 
capri_tm_uplink_lif_set(uint32_t port,
                        uint32_t lif)
{
    if (!is_valid_tm_uplink_port(port)) {
        HAL_TRACE_ERR("CAPRI-TM::{}: {} is not a valid TM uplink port", 
                      __func__, port);
        return HAL_RET_INVALID_ARG;
    }

#ifndef HAL_GTEST
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;

    pbc_csr.cfg_src_port_to_lif_map.read();
    /* Update the value in the csr */
    switch(port) {
        case TM_PORT_UPLINK_0:
        {
            pbc_csr.cfg_src_port_to_lif_map.entry_0(lif);
            break;
        }
        case TM_PORT_UPLINK_1:
        {
            pbc_csr.cfg_src_port_to_lif_map.entry_1(lif);
            break;
        }
        case TM_PORT_UPLINK_2:
        {
            pbc_csr.cfg_src_port_to_lif_map.entry_2(lif);
            break;
        }
        case TM_PORT_UPLINK_3:
        {
            pbc_csr.cfg_src_port_to_lif_map.entry_3(lif);
            break;
        }
        case TM_PORT_UPLINK_4:
        {
            pbc_csr.cfg_src_port_to_lif_map.entry_4(lif);
            break;
        }
        case TM_PORT_UPLINK_5:
        {
            pbc_csr.cfg_src_port_to_lif_map.entry_5(lif);
            break;
        }
        case TM_PORT_UPLINK_6:
        {
            pbc_csr.cfg_src_port_to_lif_map.entry_6(lif);
            break;
        }
        case TM_PORT_UPLINK_7:
        {
            pbc_csr.cfg_src_port_to_lif_map.entry_7(lif);
            break;
        }
        case TM_PORT_NCSI:
        {
            pbc_csr.cfg_src_port_to_lif_map.entry_8(lif);
            break;
        }
        default:
            return HAL_RET_ERR;
    }

    /* Write the csr */
    pbc_csr.cfg_src_port_to_lif_map.write();
#endif

    HAL_TRACE_DEBUG("CAPRI-TM::{}: Set the lif {} on port {}",
                    __func__, lif, port);

    return HAL_RET_OK;
}

hal_ret_t 
capri_tm_hw_config_load_poll (int phase)
{
    if (phase == 0) {
//        cap_pb_init_done(0,0);
    }
    return HAL_RET_OK;
}

hal_ret_t
capri_tm_asic_init (void) 
{
    cap_pb_init_start(0,0);
    cap_pb_init_done(0,0);
    return HAL_RET_OK;
}

hal_ret_t 
capri_tm_init (void)
{
#ifndef HAL_GTEST
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;

    // TM_PORT_UPLINK_0
    pbc_csr.cfg_oq_0.read();
    pbc_csr.cfg_oq_0.num_hdr_bytes(
        CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_P4_INTRINSIC_HDR_SZ);
    pbc_csr.cfg_oq_0.write();

    // TM_PORT_UPLINK_1
    pbc_csr.cfg_oq_1.read();
    pbc_csr.cfg_oq_1.num_hdr_bytes(
        CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_P4_INTRINSIC_HDR_SZ);
    pbc_csr.cfg_oq_1.write();

    // TM_PORT_UPLINK_2
    pbc_csr.cfg_oq_2.read();
    pbc_csr.cfg_oq_2.num_hdr_bytes(
        CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_P4_INTRINSIC_HDR_SZ);
    pbc_csr.cfg_oq_2.write();

    // TM_PORT_UPLINK_3
    pbc_csr.cfg_oq_3.read();
    pbc_csr.cfg_oq_3.num_hdr_bytes(
        CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_P4_INTRINSIC_HDR_SZ);
    pbc_csr.cfg_oq_3.write();

    // TM_PORT_UPLINK_4
    pbc_csr.cfg_oq_4.read();
    pbc_csr.cfg_oq_4.num_hdr_bytes(
        CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_P4_INTRINSIC_HDR_SZ);
    pbc_csr.cfg_oq_4.write();

    // TM_PORT_UPLINK_5
    pbc_csr.cfg_oq_5.read();
    pbc_csr.cfg_oq_5.num_hdr_bytes(
        CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_P4_INTRINSIC_HDR_SZ);
    pbc_csr.cfg_oq_5.write();

    // TM_PORT_UPLINK_6
    pbc_csr.cfg_oq_6.read();
    pbc_csr.cfg_oq_6.num_hdr_bytes(
        CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_P4_INTRINSIC_HDR_SZ);
    pbc_csr.cfg_oq_6.write();

    // TM_PORT_UPLINK_7
    pbc_csr.cfg_oq_7.read();
    pbc_csr.cfg_oq_7.num_hdr_bytes(
        CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_P4_INTRINSIC_HDR_SZ);
    pbc_csr.cfg_oq_7.write();

    // TM_PORT_NCSI
    pbc_csr.cfg_oq_8.read();
    pbc_csr.cfg_oq_8.num_hdr_bytes(
        CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_P4_INTRINSIC_HDR_SZ);
    pbc_csr.cfg_oq_8.write();

#endif
    HAL_TRACE_DEBUG("CAPRI-TM::{}: Init completed",
                    __func__);

    return HAL_RET_OK;
}

/* Programs the base address in HBM for the replication table */
hal_ret_t
capri_tm_repl_table_base_addr_set(uint32_t addr)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    pbc_csr.cfg_rpl.read();
    pbc_csr.cfg_rpl.base(addr);
    pbc_csr.cfg_rpl.write();
    return HAL_RET_OK;
}

/* Programs the # of tokens per replication table entry */
hal_ret_t
capri_tm_repl_table_num_tokens_set(uint32_t num_tokens)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    pbc_csr.cfg_rpl.read();
    pbc_csr.cfg_rpl.token_size(num_tokens);
    pbc_csr.cfg_rpl.write();
    return HAL_RET_OK;
}
