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
#include <cap_blk_reg_model.h>
#include <cap_top_csr.h>
#include <cap_pbc_csr.h>
#endif

//:: from collections import OrderedDict
//:: TM_PORTS = 12
//:: QS = 32
//:: L1_NODES = 16
//:: L2_NODES = 4
//:: e = [
//::    "TM_PORT_UPLINK_0",
//::    "TM_PORT_UPLINK_1",
//::    "TM_PORT_UPLINK_2",
//::    "TM_PORT_UPLINK_3",
//::    "TM_PORT_UPLINK_4",
//::    "TM_PORT_UPLINK_5",
//::    "TM_PORT_UPLINK_6",
//::    "TM_PORT_UPLINK_7",
//::    "TM_PORT_NCSI", 
//::    "TM_PORT_DMA", 
//::    "TM_PORT_EGRESS",
//::    "TM_PORT_INGRESS"]
//:: 
//:: port_info = OrderedDict()
//:: 
//:: for p in range(TM_PORTS):
//::     pinfo = {}
//::     pinfo["enum"] = e[p]
//::     pinfo["qs"] = QS
//::     pinfo["l1_nodes"] = L1_NODES
//::     pinfo["l2_nodes"] = L2_NODES
//::     if p < 9:
//::        pinfo["type"] = "uplink"
//::        pinfo["pgs"] = 8
//::     elif p < 10:
//::        pinfo["type"] = "DMA"
//::        pinfo["pgs"] = 16
//::     else:
//::        pinfo["type"] = "P4"
//::        pinfo["pgs"] = 32
//::     #endif
//::     port_info[p] = pinfo
//:: #endfor
//:: 
//:: 

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
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
        case ${pinfo["enum"]}:
        {
            npgs = ${pinfo["pgs"]};
            break;
        }
//:: #endfor
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
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
        case ${pinfo["enum"]}:
        {
            switch (pool_group) {
//::    for pg in range(port_info[p]["pgs"]):
                case ${pg}:
                {
                    /* Update the PG parameters */
                    pbc_csr.cfg_account_${p}_pg_${pg}.reserved_min(bytes_to_cells(pg_params->reserved_min));
                    pbc_csr.cfg_account_${p}_pg_${pg}.xon_threshold(bytes_to_cells(pg_params->xon_threshold));
                    pbc_csr.cfg_account_${p}_pg_${pg}.headroom(bytes_to_cells(pg_params->headroom));
                    pbc_csr.cfg_account_${p}_pg_${pg}.low_limit(bytes_to_cells(pg_params->low_limit));
                    pbc_csr.cfg_account_${p}_pg_${pg}.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.cfg_account_${p}_mtu_table.pg${pg}(bytes_to_cells(pg_params->mtu));

                    /* Write both registers */
                    pbc_csr.cfg_account_${p}_pg_${pg}.write();
                    pbc_csr.cfg_account_${p}_mtu_table.write();
                    break;
                }
//::    #endfor
                default:
                    return HAL_RET_ERR;
            }
            /* Update and write the cos to PG mapping */
            pbc_csr.cfg_account_${p}_tc_to_pg.table(cos_map_reg_val);
            pbc_csr.cfg_account_${p}_tc_to_pg.write();
            break;
        }
//:: #endfor
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
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
        case ${pinfo["enum"]}:
        {
//::    for l1 in range(pinfo["l1_nodes"]):
            pbc_csr.cfg_oq_${p}_arb_l1_selection.node_${l1}(l1_node_vals[${l1}]);
//::    #endfor

//::    for l2 in range(pinfo["l2_nodes"]):
            pbc_csr.cfg_oq_${p}_arb_l2_selection.node_${l2}(l1_node_vals[${l2}]);
//::    #endfor

            pbc_csr.cfg_oq_${p}_arb_l1_strict.priority(l1_strict_val);
            pbc_csr.cfg_oq_${p}_arb_l2_strict.priority(l2_strict_val);
            /* Write all the L1 and L2 registers */
            pbc_csr.cfg_oq_${p}_arb_l1_selection.write();
            pbc_csr.cfg_oq_${p}_arb_l2_selection.write();
            pbc_csr.cfg_oq_${p}_arb_l1_strict.write();
            pbc_csr.cfg_oq_${p}_arb_l2_strict.write();
            break;
        }
//:: #endfor
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

    /* Update the value in the csr */
    switch(port) {
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
//::    if pinfo["type"] != "uplink":
//::        continue
//::    #endif
        case ${pinfo["enum"]}:
        {
            pbc_csr.cfg_src_port_to_lif_map.entry_${p}(lif);
            break;
        }
//:: #endfor
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

/* Programs the base address in HBM for the replication table */
hal_ret_t
capri_tm_repl_table_base_addr_set(uint32_t addr)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
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
    pbc_csr.cfg_rpl.token_size(num_tokens);
    pbc_csr.cfg_rpl.write();
    return HAL_RET_OK;
}