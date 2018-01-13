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
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_tm_rw.hpp"

#ifndef HAL_GTEST
#include "nic/asic/capri/model/utils/cap_blk_reg_model.h"
#include "nic/asic/capri/model/cap_top/cap_top_csr.h"
#include "nic/asic/capri/model/cap_pb/cap_pbc_csr.h"
#include "nic/asic/capri/verif/apis/cap_pb_api.h"
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
//::     pinfo["l1"] = L1_NODES
//::     pinfo["l2"] = L2_NODES
//::     pinfo["l3"] = 0
//::     if p < 9:
//::        pinfo["type"] = "uplink"
//::        pinfo["pgs"] = 8
//::     elif p < 10:
//::        pinfo["type"] = "DMA"
//::        pinfo["pgs"] = 16
//::        pinfo["l3"] = 1
//::     else:
//::        pinfo["type"] = "P4"
//::        pinfo["pgs"] = 32
//::        pinfo["l3"] = 1
//::     #endif
//::     port_info[p] = pinfo
//:: #endfor
//::
//::
//:: import json
//:: def get_reg_instances(regs, types):
//:: import re
//:: instances = []
//:: for reg,data in sorted(regs.items()):
//::    if types is None or len(set(reg.split('_')) & types):
//::        is_array = int(data["is_array"])
//::        inst_name = data["inst_name"][4:]
//::        if is_array:
//::            mg = re.search('\[(\d+)\]$', reg)
//::            array_index = int(mg.groups()[0])
//::            inst_name = inst_name + '[%s]' % array_index
//::        #endif   
//::        instances.append(inst_name)
//::    #endif   
//:: #endfor
//:: return instances
//:: #enddef
//::
//:: with open(_context['args']) as data_file:
//::    data = json.load(data_file)
//:: #endwith
//:: regs = data["cap_pbc"]["registers"]
//:: memories = data["cap_pbc"]["memories"]
//:: 
//:: fns = OrderedDict()
//:: fns["debug"] = set(['cnt', 'sta', 'sat'])
//:: fns["config"] = set(['cfg'])
//:: fns["all"] = None
//::
//:: for fn,types in fns.items():

void
capri_tm_dump_${fn}_regs (void)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
//:: instances = get_reg_instances(regs, types)
//:: for inst_name in instances:
    ${inst_name}.read();
    ${inst_name}.show();
//:: #endfor
}
//:: #endfor

tm_port_type_e
tm_port_type_get (tm_port_t port)
{
    switch(port) {
        case TM_PORT_UPLINK_0:
        case TM_PORT_UPLINK_1:
        case TM_PORT_UPLINK_2:
        case TM_PORT_UPLINK_3:
        case TM_PORT_UPLINK_4:
        case TM_PORT_UPLINK_5:
        case TM_PORT_UPLINK_6:
        case TM_PORT_UPLINK_7:
        case TM_PORT_NCSI:
            return TM_PORT_TYPE_UPLINK;
        case TM_PORT_DMA:
            return TM_PORT_TYPE_DMA;
        case TM_PORT_EGRESS:
        case TM_PORT_INGRESS:
            return TM_PORT_TYPE_P4;
    }
    return TM_PORT_TYPE_UPLINK;
}

bool
tm_port_is_uplink_port(uint32_t port)
{
    return ((port >= TM_UPLINK_PORT_BEGIN) && (port <= TM_UPLINK_PORT_END));
}

bool
tm_port_is_dma_port(uint32_t port)
{
    return ((port >= TM_DMA_PORT_BEGIN) && (port <= TM_DMA_PORT_END));
}

static bool
is_valid_tm_port(uint32_t port)
{
    return ((port >= TM_PORT_UPLINK_0) && (port <= TM_PORT_INGRESS));
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
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;

    switch(port) {
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
        case ${pinfo["enum"]}:
        {
            switch (pool_group) {
//::    for pg in range(port_info[p]["pgs"]):
                case ${pg}:
                {
                    pbc_csr.port_${p}.cfg_account_pg_${pg}.read();
                    pbc_csr.port_${p}.cfg_account_mtu_table.read();

                    /* Update the PG parameters */
                    pbc_csr.port_${p}.cfg_account_pg_${pg}.reserved_min(pg_params->reserved_min);
                    pbc_csr.port_${p}.cfg_account_pg_${pg}.xon_threshold(pg_params->xon_threshold);
                    pbc_csr.port_${p}.cfg_account_pg_${pg}.headroom(pg_params->headroom);
                    pbc_csr.port_${p}.cfg_account_pg_${pg}.low_limit(pg_params->low_limit);
                    pbc_csr.port_${p}.cfg_account_pg_${pg}.alpha(pg_params->alpha);

                    /* Update the MTU in the MTU register */
                    pbc_csr.port_${p}.cfg_account_mtu_table.pg${pg}(pg_params->mtu);

                    /* Write both registers */
#ifdef CAPRI_TM_REG_WRITE_ENABLE
                    pbc_csr.port_${p}.cfg_account_pg_${pg}.write();
                    pbc_csr.port_${p}.cfg_account_mtu_table.write();
#endif
                    break;
                }
//::    #endfor
                default:
                    return HAL_RET_ERR;
            }
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
capri_tm_tc_map_update(uint32_t port,
                       uint32_t count,
                       tm_tc_to_pg_map_t *tc_map)
{
    /* Do some sanity checks for port */
    if (!is_valid_tm_port(port)) {
        HAL_TRACE_ERR("CAPRI-TM::{}: {} is not a valid TM port",
                      __func__, port);
        return HAL_RET_INVALID_ARG;
    }
#ifndef HAL_GTEST
    uint32_t i;
    uint32_t tc;
    uint32_t npgs;
    cpp_int tc_map_reg_val = 0;
    uint32_t pool_group;

    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    switch(port) {
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
        case ${pinfo["enum"]}:
        {
            npgs = ${pinfo["pgs"]};
            pbc_csr.port_${p}.cfg_account_tc_to_pg.read();
            tc_map_reg_val = pbc_csr.port_${p}.cfg_account_tc_to_pg.table();
            break;
        }
//:: #endfor
        default:
            return HAL_RET_INVALID_ARG;
    }

    for (i = 0; i < count; i++) {
        tc = tc_map[i].tc;
        pool_group = tc_map[i].pg;
        if (tc >= npgs) {
            HAL_TRACE_ERR("CAPRI-TM::{}: Invalid tc value {} on port {} pool group {}",
                          __func__, tc, port, pool_group);
            return HAL_RET_INVALID_ARG;
        }
        if (pool_group >= npgs) {
            HAL_TRACE_ERR("CAPRI-TM::{}: Invalid pool group {} on port {}",
                          __func__, pool_group, port);
            return HAL_RET_INVALID_ARG;
        }

        tc_map_reg_val &= ~((npgs-1) << (tc * (int)std::log2(npgs)));
        tc_map_reg_val |= (pool_group & (npgs-1)) << (tc * (int)std::log2(npgs));
    }

    switch(port) {
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
        case ${pinfo["enum"]}:
        {
            /* Update and write the tc to PG mapping */
            pbc_csr.port_${p}.cfg_account_tc_to_pg.table(tc_map_reg_val);
#ifdef CAPRI_TM_REG_WRITE_ENABLE
            pbc_csr.port_${p}.cfg_account_tc_to_pg.write();
#endif
            break;
        }
//:: #endfor
        default:
            return HAL_RET_INVALID_ARG;
    }
#endif

    return HAL_RET_OK;
}

hal_ret_t
capri_tm_uplink_input_map_update (tm_port_t port,
                                  tm_uplink_input_map_t *imap)
{

    if (!tm_port_is_uplink_port(port)) {
        HAL_TRACE_ERR("CAPRI-TM::{}: {} is not a valid TM uplink port",
                      __func__, port);
        return HAL_RET_INVALID_ARG;
    }
#ifndef HAL_GTEST
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    cap_pbchbm_csr_t &hbm_csr = pbc_csr.hbm;
    cpp_int dscp_map_val = 0;
    cpp_int oq_map_val = 0;
    cpp_int use_ip = 0;

    if ((imap->p4_oq < (signed)HAL_TM_P4_UPLINK_IQ_OFFSET) ||
        (imap->p4_oq >= (signed)tm_get_num_iqs_for_port_type(TM_PORT_TYPE_P4))) {
        HAL_TRACE_ERR("CAPRI-TM::{}:{} Invalid P4 Oq {} for uplink port {}",
                      __func__, __LINE__,
                      imap->p4_oq, port);
        return HAL_RET_INVALID_ARG;
    }

    switch(port) {
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
//::    if pinfo["type"] != "uplink":
//::        continue
//::    #endif
        case ${pinfo["enum"]}:
        {
            hbm_csr.hbm_port_${p}.cfg_hbm_parser.read();
            dscp_map_val = hbm_csr.hbm_port_${p}.cfg_hbm_parser.dscp_map();

            pbc_csr.cfg_parser${p}.read();
            oq_map_val = pbc_csr.cfg_parser${p}.oq_map();
            break;
        }
//:: #endfor
        default:
            return HAL_RET_INVALID_ARG;
    }

    oq_map_val = ~(((1<<5)-1) << (imap->tc * 5));
    oq_map_val = imap->p4_oq << (imap->tc * 5);

    for (unsigned i = 0; i < HAL_ARRAY_SIZE(imap->ip_dscp); i++) {
        if (imap->ip_dscp[i]) {
            dscp_map_val &= ~(((1<<3)-1) << (i*3));
            dscp_map_val |= imap->tc << (i * 3);
        }
    }

    if (dscp_map_val == 0) {
        use_ip = 0;
    } else {
        use_ip = 1;
    }

    switch(port) {
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
//::    if pinfo["type"] != "uplink":
//::        continue
//::    #endif
        case ${pinfo["enum"]}:
        {
            hbm_csr.hbm_port_${p}.cfg_hbm_parser.use_dot1q(1);
            hbm_csr.hbm_port_${p}.cfg_hbm_parser.use_ip(use_ip);
            hbm_csr.hbm_port_${p}.cfg_hbm_parser.dscp_map(dscp_map_val);
#ifdef CAPRI_TM_REG_WRITE_ENABLE
            hbm_csr.hbm_port_${p}.cfg_hbm_parser.write();
#endif

            pbc_csr.cfg_parser${p}.oq_map(oq_map_val);
#ifdef CAPRI_TM_REG_WRITE_ENABLE
            pbc_csr.cfg_parser${p}.write();
#endif
            break;
        }
//:: #endfor
        default:
            return HAL_RET_INVALID_ARG;
    }
#endif
    return HAL_RET_OK;
}

//:: for level in range(3):
//::    parent_level = level+1
hal_ret_t
capri_tm_scheduler_map_update_l${level} (uint32_t port,
                                  uint32_t node,
                                  tm_queue_node_params_t *node_params)
{
#ifndef HAL_GTEST
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    (void)pbc_csr;
    cpp_int node_val;
    cpp_int strict_val;

    if (node >= HAL_TM_COUNT_L${level}_NODES) {
        HAL_TRACE_ERR("CAPRI-TM::{}: node {} exceeds the number of valid level "
                      "${level} nodes in port {}",
                      __func__, node, port);
        return HAL_RET_INVALID_ARG;
    }

    switch(port) {
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
//::    if pinfo["l"+str(parent_level)]:
        case ${pinfo["enum"]}:
        {
            pbc_csr.port_${p}.cfg_oq_arb_l${parent_level}_selection.read();
            pbc_csr.port_${p}.cfg_oq_arb_l${parent_level}_strict.read();

            strict_val = pbc_csr.port_${p}.cfg_oq_arb_l${parent_level}_strict.priority();
            if (node_params->sched_type == TM_SCHED_TYPE_STRICT) {
                strict_val |= 1<<node;
            } else {
                strict_val &= ~(1<<node);
            }

            // Reset the current node's association in all the parent level
            // nodes
//::        for parent_node in range(pinfo["l"+str(parent_level)]):
            // ${parent_node}
            node_val = pbc_csr.port_${p}.cfg_oq_arb_l${parent_level}_selection.node_${parent_node}();
            if (node_params->parent_node == ${parent_node}) {
                // Associate the current node with the parent node
                node_val |= 1<<node;
            } else {
                node_val &= ~(1<<node);
            }
            pbc_csr.port_${p}.cfg_oq_arb_l${parent_level}_selection.node_${parent_node}(node_val);
//::        #endfor
            pbc_csr.port_${p}.cfg_oq_arb_l${parent_level}_strict.priority(strict_val);

            /* Write the registers */
#ifdef CAPRI_TM_REG_WRITE_ENABLE
            pbc_csr.port_${p}.cfg_oq_arb_l${parent_level}_selection.write();
            pbc_csr.port_${p}.cfg_oq_arb_l${parent_level}_strict.write();
#endif
            break;
        }
//::    #endif
//:: #endfor
        default:
            return HAL_RET_ERR;
    }

#endif
    HAL_TRACE_DEBUG("CAPRI-TM::{}: Updated the output queue scheduler on port {} "
                    "level {}, node {}",
                    __func__, port, ${level}, node);

    return HAL_RET_OK;
}

//:: #endfor

hal_ret_t
capri_tm_scheduler_map_update(uint32_t port,
                              tm_queue_node_type_e node_type,
                              uint32_t node,
                              tm_queue_node_params_t *node_params)
{
    hal_ret_t ret = HAL_RET_OK;

    if (!is_valid_tm_port(port)) {
        HAL_TRACE_ERR("CAPRI-TM::{}: {} is not a valid TM port",
                      __func__, port);
        return HAL_RET_INVALID_ARG;
    }
    switch(node_type) {
        case TM_QUEUE_NODE_TYPE_LEVEL_0:
            ret = capri_tm_scheduler_map_update_l0(port, node, node_params);
            break;
        case TM_QUEUE_NODE_TYPE_LEVEL_1:
            ret = capri_tm_scheduler_map_update_l1(port, node, node_params);
            break;
        case TM_QUEUE_NODE_TYPE_LEVEL_2:
            ret = capri_tm_scheduler_map_update_l2(port, node, node_params);
            break;
    }
    return ret;
}

/* Program the lif value on an uplink port */
hal_ret_t
capri_tm_uplink_lif_set(uint32_t port,
                        uint32_t lif)
{
    if (!tm_port_is_uplink_port(port)) {
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

hal_ret_t
capri_tm_hw_config_load_poll (int phase)
{
    if (phase == 0) {
        cap_pb_init_done(0,0);
    }
    return HAL_RET_OK;
}

hal_ret_t
capri_tm_asic_init (void)
{
//    cap_pb_init_start(0,0);
//    cap_pb_init_done(0,0);
    return HAL_RET_OK;
}

hal_ret_t
capri_tm_init (void)
{
#ifndef HAL_GTEST
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;

//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
//::    if pinfo["type"] == "uplink":
    // ${pinfo["enum"]}
    pbc_csr.port_${p}.cfg_oq.read();
    pbc_csr.port_${p}.cfg_oq.num_hdr_bytes(
        CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_P4_INTRINSIC_HDR_SZ);
    pbc_csr.port_${p}.cfg_oq.write();

//::    #endif
//:: #endfor
    // AXI Base Address
    pbc_csr.cfg_axi.read();
    pbc_csr.cfg_axi.base_addr(get_hbm_base());
    pbc_csr.cfg_axi.write();

#endif
    HAL_TRACE_DEBUG("CAPRI-TM::{}: Init completed",
                    __func__);

    return HAL_RET_OK;
}

/* Programs the base address in HBM for the replication table */
hal_ret_t
capri_tm_repl_table_base_addr_set(uint64_t addr)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    pbc_csr.cfg_rpl.read();
    pbc_csr.cfg_rpl.base(addr);
    pbc_csr.cfg_rpl.write();
    return HAL_RET_OK;
}

/* Programs the replication table token size */
hal_ret_t
capri_tm_repl_table_token_size_set(uint32_t size_in_bits)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    pbc_csr.cfg_rpl.read();

    // "Size of token in nodes. 0: 32 bits, 1: 48 bits, 2: 64 bits"
    if (size_in_bits == 64) {
        pbc_csr.cfg_rpl.token_size(2);
    } else if (size_in_bits == 48) {
        pbc_csr.cfg_rpl.token_size(1);
    } else if (size_in_bits == 32) {
        pbc_csr.cfg_rpl.token_size(0);
    } else {
        return HAL_RET_INVALID_ARG;
    }

    pbc_csr.cfg_rpl.write();
    return HAL_RET_OK;
}
