/*
 * capri_tm_rw.cc
 * Vasanth Kumar (Pensando Systems)
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
//::     pinfo["has_hbm"] = False
//::     if p < 9:
//::        pinfo["type"] = "uplink"
//::        pinfo["pgs"] = 8
//::        if e[p] != "TM_PORT_NCSI":
//::            pinfo["has_hbm"] = True
//::        #endif
//::     elif p < 10:
//::        pinfo["type"] = "dma"
//::        pinfo["pgs"] = 16
//::        pinfo["l3"] = 1
//::        pinfo["has_hbm"] = True 
//::     else:
//::        pinfo["type"] = "p4"
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
capri_tm_get_port_type (tm_port_t port)
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
            return TM_PORT_TYPE_UPLINK;
        case TM_PORT_NCSI:
            return TM_PORT_TYPE_BMC;
        case TM_PORT_DMA:
            return TM_PORT_TYPE_DMA;
        case TM_PORT_EGRESS:
            return TM_PORT_TYPE_P4_EG;
        case TM_PORT_INGRESS:
            return TM_PORT_TYPE_P4_IG;
    }
    return TM_PORT_TYPE_UPLINK;
}

uint32_t
capri_tm_num_active_uplink_ports (void)
{
    // TODO: Set this to right value 
    return TM_NUM_UPLINK_PORTS - 1;
}

bool
capri_tm_port_is_uplink_port(uint32_t port)
{
    return ((port >= TM_UPLINK_PORT_BEGIN) && (port <= TM_UPLINK_PORT_END));
}

bool
capri_tm_port_is_dma_port(uint32_t port)
{
    return ((port >= TM_DMA_PORT_BEGIN) && (port <= TM_DMA_PORT_END));
}

static bool
is_valid_tm_port(uint32_t port)
{
    return ((port >= TM_PORT_UPLINK_0) && (port <= TM_PORT_INGRESS));
}

static uint32_t
capri_tm_get_num_hbm_qs_per_uplink_port (void)
{
    uint32_t num_active_uplink_ports;
    uint32_t num_hbm_qs_per_port;

    num_active_uplink_ports = capri_tm_num_active_uplink_ports();
    num_hbm_qs_per_port = HAL_TM_MAX_HBM_ETH_QS/num_active_uplink_ports;
    if (num_hbm_qs_per_port > 8) {
        num_hbm_qs_per_port = 8;
    }
    return num_hbm_qs_per_port;
}

hal_ret_t
capri_tm_pg_params_update(uint32_t port,
                          uint32_t pool_group,
                          tm_pg_params_t *pg_params,
                          tm_hbm_fifo_params_t *hbm_params)
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
    cap_pbchbm_csr_t &hbm_csr = pbc_csr.hbm;
    cpp_int payload_base_val;
    cpp_int payload_size_val;
    cpp_int control_base_val;
    cpp_int control_size_val;
    cpp_int xoff_val;
    cpp_int xon_val;
    uint32_t xoff_threshold;
    uint32_t xon_threshold;
    uint32_t hbm_q;
    uint32_t num_hbm_qs_per_port;

    num_hbm_qs_per_port = capri_tm_get_num_hbm_qs_per_uplink_port();
    switch(port) {
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
        case ${pinfo["enum"]}:
        {
//::    if pinfo["has_hbm"]:
//::        pass
            if (hbm_params == NULL) {
                HAL_TRACE_ERR("CAPRI-TM::{}:{} HBM parameters not passed for "
                              "port {} pool_group {}",
                              __func__, __LINE__,
                              port, pool_group);
                return HAL_RET_INVALID_ARG;
            }
//::    else:
//::        pass
            if (hbm_params != NULL) {
                HAL_TRACE_ERR("CAPRI-TM::{}:{} HBM parameters passed for "
                              "port {} pool_group {} and HBM not supported",
                              __func__, __LINE__,
                              port, pool_group);
                return HAL_RET_INVALID_ARG;
            }
//::    #endif
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
//::    if pinfo["has_hbm"]:
//::        if pinfo["type"] == "uplink":
            hbm_csr.cfg_hbm_eth_payload.read();
            hbm_csr.cfg_hbm_eth_ctrl.read();
            hbm_csr.cfg_hbm_threshold.read();

            payload_base_val = hbm_csr.cfg_hbm_eth_payload.base();
            payload_size_val = hbm_csr.cfg_hbm_eth_payload.mem_sz();
            control_base_val = hbm_csr.cfg_hbm_eth_ctrl.base();
            control_size_val = hbm_csr.cfg_hbm_eth_ctrl.mem_sz();
            xoff_val = hbm_csr.cfg_hbm_threshold.xoff();
            xon_val = hbm_csr.cfg_hbm_threshold.xon();
            hbm_q = pool_group + (num_hbm_qs_per_port * ${pinfo["enum"]});
//::        else:
            hbm_csr.cfg_hbm_tx_payload.read();
            hbm_csr.cfg_hbm_tx_ctrl.read();

            payload_base_val = hbm_csr.cfg_hbm_tx_payload.base();
            payload_size_val = hbm_csr.cfg_hbm_tx_payload.mem_sz();
            control_base_val = hbm_csr.cfg_hbm_tx_ctrl.base();
            control_size_val = hbm_csr.cfg_hbm_tx_ctrl.mem_sz();
            hbm_q = pool_group;
//::        #endif
            // 27 bits per hbm_q
            pbc_csr.hlp.set_slc(payload_base_val, hbm_params->payload_offset,
                                hbm_q * 27, ((hbm_q + 1) * 27) - 1);
            // 23 bits per hbm_q
            pbc_csr.hlp.set_slc(payload_size_val, hbm_params->payload_size,
                                hbm_q * 23, ((hbm_q + 1) * 23) - 1);
            // 27 bits per hbm_q
            pbc_csr.hlp.set_slc(control_base_val, hbm_params->control_offset,
                                hbm_q * 27, ((hbm_q + 1) * 27) - 1);
            // 23 bits per hbm_q
            pbc_csr.hlp.set_slc(control_size_val, hbm_params->control_size,
                                hbm_q * 23, ((hbm_q + 1) * 23) - 1);
//::        if pinfo["type"] == "uplink":
            // xoff and xon thresholds are in 512B units in register. Input
            // is in 64B unit. So right shift by 3
            xoff_threshold = (hbm_params->xoff_threshold + (1<<3) - 1) >> 3;
            xon_threshold = (hbm_params->xon_threshold + (1<<3) - 1) >> 3;
            // 20 bits per hbm_q
            pbc_csr.hlp.set_slc(xoff_val, xoff_threshold,
                                hbm_q * 20, ((hbm_q + 1) * 20) - 1);
            // 20 bits per hbm_q
            pbc_csr.hlp.set_slc(xon_val, xon_threshold,
                                hbm_q * 20, ((hbm_q + 1) * 20) - 1);

            hbm_csr.cfg_hbm_eth_payload.base(payload_base_val);
            hbm_csr.cfg_hbm_eth_payload.mem_sz(payload_size_val);
            hbm_csr.cfg_hbm_eth_ctrl.base(control_base_val);
            hbm_csr.cfg_hbm_eth_ctrl.mem_sz(control_size_val);
            hbm_csr.cfg_hbm_threshold.xoff(xoff_val);
            hbm_csr.cfg_hbm_threshold.xon(xon_val);
            // Write all the registers
#ifdef CAPRI_TM_REG_WRITE_ENABLE
            hbm_csr.cfg_hbm_eth_payload.write();
            hbm_csr.cfg_hbm_eth_ctrl.write();
            hbm_csr.cfg_hbm_threshold.write();
#endif
//::        else:
            hbm_csr.cfg_hbm_tx_payload.base(payload_base_val);
            hbm_csr.cfg_hbm_tx_payload.mem_sz(payload_size_val);
            hbm_csr.cfg_hbm_tx_ctrl.base(control_base_val);
            hbm_csr.cfg_hbm_tx_ctrl.mem_sz(control_size_val);
            // Write all the registers
#ifdef CAPRI_TM_REG_WRITE_ENABLE
            hbm_csr.cfg_hbm_tx_payload.write();
            hbm_csr.cfg_hbm_tx_ctrl.write();
#endif
//::        #endif
//::    #endif
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
    cpp_int  tc_map_reg_val = 0;
    uint32_t pool_group;

    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    cap_pbchbm_csr_t &hbm_csr = pbc_csr.hbm;
    switch(port) {
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
        case ${pinfo["enum"]}:
        {
            npgs = ${pinfo["pgs"]};
            pbc_csr.port_${p}.cfg_account_tc_to_pg.read();
            tc_map_reg_val = pbc_csr.port_${p}.cfg_account_tc_to_pg.table();
//::    if pinfo["has_hbm"]:
            hbm_csr.hbm_port_${p}.cfg_hbm_tc_to_q.read();
//::    #endif            
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

        pbc_csr.hlp.set_slc(tc_map_reg_val, pool_group, 
                            tc * (int)std::log2(npgs), 
                            ((tc+1) * (int)std::log2(npgs)) - 1);
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
//::    if pinfo["has_hbm"]:
            hbm_csr.hbm_port_${p}.cfg_hbm_tc_to_q.table(tc_map_reg_val);
#ifdef CAPRI_TM_REG_WRITE_ENABLE
            hbm_csr.hbm_port_${p}.cfg_hbm_tc_to_q.write();
#endif
//::    #endif            
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

    if (!capri_tm_port_is_uplink_port(port)) {
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
    int use_ip = 0;

    if ((imap->p4_oq < (signed)HAL_TM_P4_UPLINK_IQ_OFFSET) ||
        (imap->p4_oq >= (signed)capri_tm_get_num_iqs_for_port_type(TM_PORT_TYPE_P4_IG))) {
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

    pbc_csr.hlp.set_slc(oq_map_val, imap->p4_oq,
                        imap->tc * 5,
                        ((imap->tc + 1) * 5) - 1);

    for (unsigned i = 0; i < HAL_ARRAY_SIZE(imap->ip_dscp); i++) {
        if (imap->ip_dscp[i]) {
            pbc_csr.hlp.set_slc(dscp_map_val, imap->tc, i*3, ((i+1)*3)-1);
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
            pbc_csr.hlp.set_slc(strict_val, 
                                node_params->sched_type == TM_SCHED_TYPE_STRICT ? 1 : 0,
                                node,
                                node);

            // Reset the current node's association in all the parent level
            // nodes
//::        for parent_node in range(pinfo["l"+str(parent_level)]):
            // ${parent_node}
            node_val = pbc_csr.port_${p}.cfg_oq_arb_l${parent_level}_selection.node_${parent_node}();
            // Associate/disassociate the current node with the parent node
            pbc_csr.hlp.set_slc(node_val, 
                                node_params->parent_node == ${parent_node} ? 1 : 0,
                                node,
                                node);
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
    if (!capri_tm_port_is_uplink_port(port)) {
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
    cap_pbchbm_csr_t &hbm_csr = pbc_csr.hbm;
    uint32_t num_active_uplink_ports;
    uint32_t num_hbm_qs_per_port;
    uint32_t num_qs;
    uint32_t base_offset;

    num_active_uplink_ports = capri_tm_num_active_uplink_ports();
    num_hbm_qs_per_port = capri_tm_get_num_hbm_qs_per_uplink_port();

    // On uplink ports, set the number of header bytes to remove
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

    // On active uplink ports and the DMA port, setup the HBM queues
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]

//::    if pinfo["type"] == "uplink":
    // ${pinfo["enum"]}
    if (${p} < num_active_uplink_ports) {
        // If we are an active port, then setup the HBM fifo contexts
        num_qs = num_hbm_qs_per_port; 
    } else {
        num_qs = 0;
    }
    base_offset = ${p};
//::    elif pinfo["type"] == "dma":
    // ${pinfo["enum"]}
    num_qs = HAL_TM_MAX_HBM_DMA_QS; 
    base_offset = 0;
//::    #endif

//::    if pinfo["has_hbm"]:
    hbm_csr.hbm_port_${p}.cfg_hbm_context.read();
    hbm_csr.hbm_port_${p}.cfg_hbm_context.enable((1<<num_qs)-1);
    hbm_csr.hbm_port_${p}.cfg_hbm_context.base(base_offset * num_qs);
#ifdef CAPRI_TM_REG_WRITE_ENABLE
    hbm_csr.hbm_port_${p}.cfg_hbm_context.write();
#endif
//::    #endif
//:: #endfor

    // AXI Base Address
    pbc_csr.cfg_axi.read();
    pbc_csr.cfg_axi.base_addr(get_hbm_base());
    pbc_csr.cfg_axi.write();

    // AXI Base for buffer FIFOs
    pbc_csr.hbm.cfg_hbm_axi_base.read();
    pbc_csr.hbm.cfg_hbm_axi_base.addr(get_start_offset(CAPRI_HBM_REG_QOS_HBM_FIFO));
    pbc_csr.hbm.cfg_hbm_axi_base.write();

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

/* Get hw clock */
hal_ret_t
capri_tm_get_clock_tick(uint64_t *tick)
{
#ifndef HAL_GTEST
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    cap_pbchbm_csr_t &hbm_csr = pbc_csr.hbm;

    hbm_csr.sta_hbm_timestamp.read();
    *tick = (uint64_t)hbm_csr.sta_hbm_timestamp.value().convert_to<uint64_t>();
#endif 
    return HAL_RET_OK;
}
