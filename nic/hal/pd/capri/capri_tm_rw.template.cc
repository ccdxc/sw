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
#include "nic/p4/iris/include/defines.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/capri/capri_tm_rw.hpp"

#ifndef HAL_GTEST
#include "nic/asic/capri/model/utils/cap_blk_reg_model.h"
#include "nic/asic/capri/model/cap_top/cap_top_csr.h"
#include "nic/asic/capri/model/cap_pb/cap_pbc_csr.h"
#include "nic/asic/capri/verif/apis/cap_pb_api.h"
#include "nic/asic/capri/model/cap_pb/cap_pbc_decoders.h"
#endif

typedef struct capri_tm_cfg_profile_s {
    uint32_t num_qs[NUM_TM_PORT_TYPES];
    uint32_t jumbo_mtu;
    uint32_t num_active_uplink_ports;
    uint64_t hbm_fifo_base;
    uint32_t hbm_fifo_size;
    bool     sw_init_enabled;
    bool     sw_cfg_write_enabled;
} __PACK__ capri_tm_cfg_profile_t;

typedef struct capri_tm_port_asic_profile_s {
    uint32_t reserved_mtus;
    uint32_t headroom_cells;
    uint32_t recirc_q;
    uint32_t rate_limiter;
    bool     uses_credits;
} __PACK__ capri_tm_asic_port_profile_t;

typedef struct capri_tm_asic_profile_s {
    uint32_t                     cell_alloc_units;
    uint32_t                     hbm_fifo_alloc_units;
    uint32_t                     hbm_fifo_control_scale_factor;
    capri_tm_asic_port_profile_t port[NUM_TM_PORT_TYPES];
    uint32_t                     hbm_fifo_reserved_bytes_per_context[NUM_TM_HBM_FIFO_TYPES];
    uint32_t                     cpu_copy_tail_drop_threshold;
    uint32_t                     span_tail_drop_threshold;
} __PACK__ capri_tm_asic_profile_t;

typedef struct capri_tm_buf_hbm_cfg_s {
    bool valid;
    uint64_t payload_offset;
    uint64_t control_offset;
    uint32_t payload_chunks;
    uint32_t control_chunks;
} __PACK__ capri_tm_buf_hbm_cfg_t;

typedef struct capri_tm_buf_cfgs_s {
    uint32_t               chunks_per_q[NUM_TM_PORT_TYPES];
    capri_tm_buf_hbm_cfg_t hbm_fifo[NUM_TM_HBM_FIFO_TYPES][HAL_TM_MAX_HBM_CONTEXTS];
} __PACK__ capri_tm_buf_cfg_t;

typedef struct capri_tm_ctx_s {
    capri_tm_asic_profile_t asic_profile;
    capri_tm_cfg_profile_t  cfg_profile;
    capri_tm_buf_cfg_t      buf_cfg;
} capri_tm_ctx_t;

capri_tm_ctx_t g_tm_ctx_;
capri_tm_ctx_t *g_tm_ctx;

static void
set_tm_ctx (capri_tm_cfg_profile_t *tm_cfg_profile,
            capri_tm_asic_profile_t *asic_profile)
{
    if (!g_tm_ctx) {
        g_tm_ctx_.cfg_profile = *tm_cfg_profile;
        g_tm_ctx_.asic_profile = *asic_profile;
        g_tm_ctx = &g_tm_ctx_;
    }
}

static capri_tm_ctx_t *
tm_ctx (void)
{
    return g_tm_ctx;
}

static capri_tm_asic_profile_t *
tm_asic_profile (void)
{
    return &tm_ctx()->asic_profile;
}

static capri_tm_cfg_profile_t *
tm_cfg_profile (void)
{
    return &tm_ctx()->cfg_profile;
}

static inline bool
tm_sw_init_enabled (void)
{
    return tm_cfg_profile()->sw_init_enabled;
}

static inline bool
tm_sw_cfg_write_enabled (void)
{
    return tm_cfg_profile()->sw_cfg_write_enabled;
}

static void
populate_asic_profile (capri_tm_asic_profile_t *asic_profile)
{
    // These are values based on performance numbers seen during rtl simulation
    // When reserved_mtus is zero, it indicates that allocate whatever is left
    memset(asic_profile, 0, sizeof(*asic_profile));
    asic_profile->cell_alloc_units = 4;
    asic_profile->hbm_fifo_alloc_units = HAL_TM_HBM_FIFO_ALLOC_SIZE;
    asic_profile->hbm_fifo_control_scale_factor = 50;
    asic_profile->cpu_copy_tail_drop_threshold = 900;
    asic_profile->span_tail_drop_threshold = 900;

    asic_profile->port[TM_PORT_TYPE_UPLINK].reserved_mtus = 0;
    asic_profile->port[TM_PORT_TYPE_UPLINK].headroom_cells = 116;
    asic_profile->port[TM_PORT_TYPE_UPLINK].rate_limiter = 0xf;

    asic_profile->port[TM_PORT_TYPE_P4IG].reserved_mtus = 3;
    asic_profile->port[TM_PORT_TYPE_P4IG].headroom_cells = 0;
    asic_profile->port[TM_PORT_TYPE_P4IG].uses_credits = true;
    asic_profile->port[TM_PORT_TYPE_P4IG].recirc_q = TM_P4_RECIRC_QUEUE;

    asic_profile->port[TM_PORT_TYPE_P4EG].reserved_mtus = 3;
    asic_profile->port[TM_PORT_TYPE_P4EG].headroom_cells = 0;
    asic_profile->port[TM_PORT_TYPE_P4EG].uses_credits = true;
    asic_profile->port[TM_PORT_TYPE_P4EG].recirc_q = TM_P4_RECIRC_QUEUE;

    asic_profile->port[TM_PORT_TYPE_DMA].reserved_mtus = 0;
    asic_profile->port[TM_PORT_TYPE_DMA].headroom_cells = 125;

    asic_profile->hbm_fifo_reserved_bytes_per_context[TM_HBM_FIFO_TYPE_UPLINK] = 3*1024*1024; // 3MB
    asic_profile->hbm_fifo_reserved_bytes_per_context[TM_HBM_FIFO_TYPE_TXDMA] = 9*1024*1024;; // 9MB
}

static inline uint32_t
bytes_to_cells (uint32_t bytes)
{
    return (bytes + HAL_TM_CELL_SIZE - 1)/HAL_TM_CELL_SIZE;
}

static inline uint32_t
cells_to_bytes (uint32_t cells)
{
    return cells * HAL_TM_CELL_SIZE;
}

static inline uint32_t
cells_to_chunks (uint32_t cells)
{
    return (cells + tm_asic_profile()->cell_alloc_units - 1)/
        tm_asic_profile()->cell_alloc_units;
}

static inline uint32_t
chunks_to_cells (uint32_t chunks)
{
    return chunks * tm_asic_profile()->cell_alloc_units;
}

static inline uint32_t
hbm_bytes_to_chunks (uint32_t bytes)
{
    return (bytes + tm_asic_profile()->hbm_fifo_alloc_units - 1)/
        tm_asic_profile()->hbm_fifo_alloc_units;
}

static inline uint32_t
hbm_chunks_to_bytes (uint32_t chunks)
{
    return chunks * tm_asic_profile()->hbm_fifo_alloc_units;
}

static inline uint32_t
capri_tm_get_max_cell_chunks_for_island (uint32_t island)
{
    uint32_t cells = 0;
    HAL_ASSERT(island < HAL_TM_NUM_BUFFER_ISLANDS);
    if (tm_cfg_profile()->num_active_uplink_ports > 2) {
        island = HAL_TM_NUM_BUFFER_ISLANDS - island - 1;
    }
    if (island == 0) {
        cells = HAL_TM_BUFFER_ISLAND_0_CELL_COUNT;
    } else if (island == 1) {
        cells = HAL_TM_BUFFER_ISLAND_1_CELL_COUNT;
    }
    return cells_to_chunks(cells);
}

static inline tm_port_type_e
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
        case TM_PORT_NCSI:
            return TM_PORT_TYPE_UPLINK;
        case TM_PORT_DMA:
            return TM_PORT_TYPE_DMA;
        case TM_PORT_EGRESS:
            return TM_PORT_TYPE_P4EG;
        case TM_PORT_INGRESS:
            return TM_PORT_TYPE_P4IG;
    }
    return NUM_TM_PORT_TYPES;
}

static inline bool
is_active_uplink_port (tm_port_t port)
{
    if (port < tm_cfg_profile()->num_active_uplink_ports) {
        return true;
    }
    return false;
}

static inline bool
is_active_port (tm_port_t port)
{
    tm_port_type_e port_type;

    port_type = capri_tm_get_port_type(port);
    if (port_type == TM_PORT_TYPE_UPLINK) {
        return (is_active_uplink_port(port) ||
                (port == TM_PORT_NCSI));
    }
    return true;
}

static inline uint32_t
capri_tm_max_hbm_contexts_for_fifo (uint32_t fifo_type)
{
    switch (fifo_type) {
        case TM_HBM_FIFO_TYPE_UPLINK:
            return HAL_TM_MAX_HBM_ETH_CONTEXTS;
        case TM_HBM_FIFO_TYPE_TXDMA:
            return HAL_TM_MAX_HBM_DMA_CONTEXTS;
        case NUM_TM_HBM_FIFO_TYPES:
            return 0;
    }
    return 0;
}

static inline bool
port_supports_hbm_contexts (tm_port_t port)
{
    return is_active_uplink_port(port) || (port == TM_PORT_DMA);
}

static inline uint32_t
capri_tm_get_num_iqs_for_port_type (tm_port_type_e port_type)
{
    switch(port_type) {
        case TM_PORT_TYPE_UPLINK:
            return 8;
        case TM_PORT_TYPE_P4IG:
        case TM_PORT_TYPE_P4EG:
            return 32;
        case TM_PORT_TYPE_DMA:
            return 16;
        case NUM_TM_PORT_TYPES:
            return 0;
    }
    return 0;
}

static inline uint32_t
capri_tm_get_island_for_port_type (tm_port_type_e port_type)
{
    switch(port_type) {
        case TM_PORT_TYPE_UPLINK:
        case TM_PORT_TYPE_P4EG:
            return 1;
        case TM_PORT_TYPE_P4IG:
        case TM_PORT_TYPE_DMA:
            return 0;
        case NUM_TM_PORT_TYPES:
            return 0;
    }

    return 0;
}

//:: from collections import OrderedDict
//:: import math
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
//::     pinfo["supports_credits"] = False
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
//::        pinfo["supports_credits"] = True
//::     #endif
//::     port_info[p] = pinfo
//:: #endfor
//::
//:: hbm_fifo_info = {
//::                'TM_HBM_FIFO_TYPE_UPLINK': {'reg_name' : 'eth', 'count' : 32 },
//::                'TM_HBM_FIFO_TYPE_TXDMA': {'reg_name' : 'tx', 'count' : 16}
//::                 }
//::
//:: import yaml
//:: def get_reg_instances(regs, types):
//:: import re
//:: instances = []
//:: for reg in sorted(regs):
//::    regn = reg.split('.')[-1]
//::    if types is None or len(set(regn.split('_')) & types):
//::        instances.append(reg)
//::    #endif
//:: #endfor
//:: return instances
//:: #enddef
//::
//:: def parse_block(data, block, type, path_to_here, regs):
//::     global parse_block
//::     path = path_to_here[:]
//::     block_type = data[block]['type']
//::     if block_type == type:
//::        regs.append(path)
//::        return
//::     #endif
//::     if block_type != 'block':
//::         return
//::     #endif
//::     for field, field_d in data[block]['fields'].items():
//::        lpath = path + '.' + field
//::        is_array = False
//::        if field_d['array'] != 1:
//::            is_array = True
//::        #endif
//::        apath = lpath[:]
//::        for i in range(field_d['array']):
//::            if is_array:
//::                apath = lpath + '[%d]' % i
//::            #endif
//::            parse_block(data, field_d['decoder'], type, apath, regs)
//::        #endfor
//::     #endfor
//:: #enddef
//::
//:: def normalize(data):
//::    d = {}
//::    for block_name, block_data in data.items():
//::        d[block_name] = {x.keys()[0]:x.values()[0] for x in block_data}
//::        d[block_name]['fields'] = {x.keys()[0]:x.values()[0] for x in d[block_name]['fields']}
//::        for field_name, field_d in d[block_name]['fields'].items():
//::            d[block_name]['fields'][field_name] = {x.keys()[0]:x.values()[0] for x in field_d}
//::        #endfor
//::    #endfor
//::    return d
//:: #enddef
//::
//:: with open(_context['args']) as data_file:
//::    data = yaml.load(data_file)
//:: #endwith
//:: regs = []
//:: memories = []
//::
//:: data = normalize(data)
//:: parse_block(data, 'cap_pbc_csr', 'register', 'pbc_csr', regs)
//:: parse_block(data, 'cap_pbc_csr', 'memory', 'cap0.pb', memories)
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

bool
capri_tm_port_is_uplink_port (uint32_t port)
{
    return ((port >= TM_UPLINK_PORT_BEGIN) && (port <= TM_UPLINK_PORT_END));
}

bool
capri_tm_port_is_dma_port (uint32_t port)
{
    return ((port >= TM_DMA_PORT_BEGIN) && (port <= TM_DMA_PORT_END));
}

static bool
is_valid_tm_port (uint32_t port)
{
    return ((port >= TM_PORT_UPLINK_0) && (port <= TM_PORT_INGRESS));
}

hal_ret_t
capri_tm_uplink_iq_params_update (tm_port_t port,
                                  tm_q_t iq,
                                  tm_uplink_iq_params_t *iq_params)
{
    /* Do some sanity checks for port and iq */
    if (!capri_tm_port_is_uplink_port(port)) {
        HAL_TRACE_ERR("{} is not a valid TM uplink port",
                      port);
        return HAL_RET_INVALID_ARG;
    }

    if (iq_params->mtu > tm_cfg_profile()->jumbo_mtu) {
        HAL_TRACE_ERR("Invalid mtu {} larger than the jumbo {}",
                      iq_params->mtu, tm_cfg_profile()->jumbo_mtu);
        return HAL_RET_INVALID_ARG;
    }

    if ((iq_params->p4_q < (signed)HAL_TM_P4_UPLINK_IQ_OFFSET) ||
        (iq_params->p4_q >= (signed)capri_tm_get_num_iqs_for_port_type(TM_PORT_TYPE_P4IG))) {
        HAL_TRACE_ERR("Invalid P4 Oq {} for uplink port {}",
                      iq_params->p4_q, port);
        return HAL_RET_INVALID_ARG;
    }

#ifndef HAL_GTEST
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    cap_pbchbm_csr_t &hbm_csr = pbc_csr.hbm;
    cpp_int xoff_val;
    cpp_int xon_val;
    cpp_int oq_map_val;
    cpp_int port_payload_occupancy_val;
    uint32_t payload_occupancy;
    uint32_t payload_occupancy_bytes;
    uint32_t xoff_threshold;
    uint32_t xon_threshold;
    uint32_t hbm_context;
    uint32_t num_hbm_contexts_per_port;
    cap_pbc_oq_map_t oq_map_decoder;
    oq_map_decoder.init();

    num_hbm_contexts_per_port = tm_cfg_profile()->num_qs[TM_PORT_TYPE_UPLINK];
    switch(port) {
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
//::    if pinfo["type"] == "uplink":
        case ${pinfo["enum"]}:
            {
                // P4 oq derivation register
                pbc_csr.cfg_parser${p}.read();
                oq_map_val = pbc_csr.cfg_parser${p}.oq_map();

                pbc_csr.hlp.set_slc(oq_map_val, iq_params->p4_q,
                                    iq * 5,
                                    ((iq + 1) * 5) - 1);

                pbc_csr.cfg_parser${p}.oq_map(oq_map_val);

                oq_map_decoder.all(pbc_csr.cfg_parser${p}.oq_map());
                oq_map_decoder.set_name("cap0.pb.pbc.cfg_parser${p}.decoder");
                if (tm_sw_cfg_write_enabled()) {
                    oq_map_decoder.show();

                    pbc_csr.cfg_parser${p}.show();
                    pbc_csr.cfg_parser${p}.write();
                }

                // MTU
                pbc_csr.port_${p}.cfg_account_mtu_table.read();
                switch (iq) {
//::        for pg in range(port_info[p]["pgs"]):
                    case ${pg}:
                        {
                            /* Update the MTU in the MTU register */
                            pbc_csr.port_${p}.cfg_account_mtu_table.pg${pg}(iq_params->mtu);
                            break;
                        }
//::        #endfor
                    default:
                        return HAL_RET_ERR;
                }

                if (tm_sw_cfg_write_enabled()) {
                    pbc_csr.port_${p}.cfg_account_mtu_table.show();
                    pbc_csr.port_${p}.cfg_account_mtu_table.write();
                }

                if (port_supports_hbm_contexts(port)) {
                    // HBM Thresholds
                    hbm_csr.cfg_hbm_threshold.read();
                    hbm_csr.hbm_port_${p}.cfg_hbm_eth_payload_occupancy.read();

                    port_payload_occupancy_val = hbm_csr.hbm_port_${p}.cfg_hbm_eth_payload_occupancy.threshold();

                    xoff_val = hbm_csr.cfg_hbm_threshold.xoff();
                    xon_val = hbm_csr.cfg_hbm_threshold.xon();
                    hbm_context = iq + (num_hbm_contexts_per_port * ${pinfo["enum"]});

                    payload_occupancy = pbc_csr.hlp.get_slc(
                        port_payload_occupancy_val,
                        iq*19, ((iq + 1) * 19) - 1).convert_to<uint32_t>();

                    payload_occupancy_bytes = payload_occupancy << 10;

                    // xoff threshold is the value from the payload occupancy
                    // threshold.
                    // But in the csr we need to write the value from the base.
                    // So subtract it from the payload occupancy threshold

                    xoff_threshold = iq_params->xoff_threshold;
                    if (payload_occupancy_bytes > xoff_threshold) {
                        xoff_threshold = payload_occupancy_bytes - xoff_threshold;
                    } else {
                        xoff_threshold = 0;
                    }

                    // xoff and xon thresholds are in 512B units in register.
                    // So right shift by 9 (using ceil value for xon and floor
                    // for xoff)
                    xoff_threshold >>= 9;
                    xon_threshold = (iq_params->xon_threshold + (1<<9) - 1) >> 9;

                    // 20 bits per hbm_context
                    pbc_csr.hlp.set_slc(xoff_val, xoff_threshold,
                                        hbm_context * 20, ((hbm_context + 1) * 20) - 1);
                    // 20 bits per hbm_context
                    pbc_csr.hlp.set_slc(xon_val, xon_threshold,
                                        hbm_context * 20, ((hbm_context + 1) * 20) - 1);

                    hbm_csr.cfg_hbm_threshold.xoff(xoff_val);
                    hbm_csr.cfg_hbm_threshold.xon(xon_val);
                    // Write all the registers
                    if (tm_sw_cfg_write_enabled()) {
                        hbm_csr.cfg_hbm_threshold.show();
                        hbm_csr.cfg_hbm_threshold.write();
                    }
                }
                break;
            }
//::    #endif
//:: #endfor
        default:
            return HAL_RET_ERR;
    }

#endif
    HAL_TRACE_DEBUG("Updated the iq {} on port {}",
                    iq, port);

    return HAL_RET_OK;
}

hal_ret_t
capri_tm_uplink_input_map_update (tm_port_t port,
                                 uint32_t dot1q_pcp,
                                 tm_q_t iq)
{
    if (!capri_tm_port_is_uplink_port(port)) {
        HAL_TRACE_ERR("{} is not a valid TM uplink port",
                      port);
        return HAL_RET_INVALID_ARG;
    }
#ifndef HAL_GTEST
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    cap_pbchbm_csr_t &hbm_csr = pbc_csr.hbm;
    cpp_int tc_map_reg_val;
    uint32_t tc;
    uint32_t nbits;

    tc = dot1q_pcp;

    switch(port) {
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
//::    if pinfo["type"] == "uplink":
        case ${pinfo["enum"]}:
            {
                nbits = ${int(math.log(pinfo["pgs"], 2))};
                hbm_csr.hbm_port_${p}.cfg_hbm_tc_to_q.read();
                tc_map_reg_val = hbm_csr.hbm_port_${p}.cfg_hbm_tc_to_q.table();

                pbc_csr.hlp.set_slc(tc_map_reg_val, iq,
                                    tc * nbits,
                                    ((tc+1) * nbits) - 1);

                /* Update and write the tc to PG mapping */
                hbm_csr.hbm_port_${p}.cfg_hbm_tc_to_q.table(tc_map_reg_val);
                if (tm_sw_cfg_write_enabled()) {
                    hbm_csr.hbm_port_${p}.cfg_hbm_tc_to_q.show();
                    hbm_csr.hbm_port_${p}.cfg_hbm_tc_to_q.write();
                }
                break;
            }
//::    #endif
//:: #endfor
        default:
            return HAL_RET_INVALID_ARG;
    }
#endif
    return HAL_RET_OK;
}

hal_ret_t
capri_tm_uplink_input_dscp_map_update(tm_port_t port,
                                      tm_uplink_input_dscp_map_t *dscp_map)
{
    if (!capri_tm_port_is_uplink_port(port)) {
        HAL_TRACE_ERR("{} is not a valid TM uplink port",
                      port);
        return HAL_RET_INVALID_ARG;
    }
#ifndef HAL_GTEST
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    cap_pbchbm_csr_t &hbm_csr = pbc_csr.hbm;
    cpp_int dscp_map_val;
    uint32_t tc;
    int use_ip = 0;

    tc = dscp_map->dot1q_pcp;

    switch(port) {
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
//::    if pinfo["type"] == "uplink":
        case ${pinfo["enum"]}:
            {
                hbm_csr.hbm_port_${p}.cfg_hbm_parser.read();
                dscp_map_val = hbm_csr.hbm_port_${p}.cfg_hbm_parser.dscp_map();

                for (unsigned i = 0; i < HAL_ARRAY_SIZE(dscp_map->ip_dscp); i++) {
                    if (dscp_map->ip_dscp[i]) {
                        pbc_csr.hlp.set_slc(dscp_map_val, tc, i*3, ((i+1)*3)-1);
                    }
                }

                use_ip = (dscp_map_val ? 1 : 0);

                hbm_csr.hbm_port_${p}.cfg_hbm_parser.use_dot1q(1);
                hbm_csr.hbm_port_${p}.cfg_hbm_parser.use_ip(use_ip);
                hbm_csr.hbm_port_${p}.cfg_hbm_parser.dscp_map(dscp_map_val);
                if (tm_sw_cfg_write_enabled()) {
                    hbm_csr.hbm_port_${p}.cfg_hbm_parser.show();
                    hbm_csr.hbm_port_${p}.cfg_hbm_parser.write();
                }
                break;
            }
//::    #endif
//:: #endfor
        default:
            return HAL_RET_INVALID_ARG;
    }
#endif
    return HAL_RET_OK;
}


hal_ret_t
capri_tm_uplink_oq_update(tm_port_t port,
                          tm_q_t oq,
                          bool xoff_enable,
                          uint32_t xoff_cos)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    uint32_t xoff_enable_val;
    cpp_int xoff2oq_map_val;
    cap_pbc_eth_oq_xoff_map_t xoff2oq_map_decoder;
    xoff2oq_map_decoder.init();

    if (!capri_tm_port_is_uplink_port(port)) {
        HAL_TRACE_ERR("{} is not a valid TM uplink port",
                      port);
        return HAL_RET_INVALID_ARG;
    }

    switch(port) {
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
//::    if pinfo["type"] == "uplink":
        case ${pinfo["enum"]}:
            {
                pbc_csr.port_${p}.cfg_oq_xoff2oq.read();
                xoff2oq_map_val = pbc_csr.port_${p}.cfg_oq_xoff2oq.map();

                pbc_csr.hlp.set_slc(xoff2oq_map_val, xoff_cos, oq*3, ((oq+1)*3)-1);

                pbc_csr.port_${p}.cfg_oq_xoff2oq.map(xoff2oq_map_val);

                pbc_csr.port_${p}.cfg_mac_xoff.read();
                xoff_enable_val = pbc_csr.port_${p}.cfg_mac_xoff.enable().convert_to<uint32_t>();
                if (xoff_enable) {
                    xoff_enable_val &= ~(1<<xoff_cos);
                } else {
                    xoff_enable_val |= 1<<xoff_cos;
                }
                pbc_csr.port_${p}.cfg_mac_xoff.enable(xoff_enable_val);

                xoff2oq_map_decoder.all(pbc_csr.port_${p}.cfg_oq_xoff2oq.map());
                xoff2oq_map_decoder.set_name("cap0.pb.pbc.port_${p}.cfg_oq_xoff2oq.decoder");
                if (tm_sw_cfg_write_enabled()) {
                    xoff2oq_map_decoder.show();

                    pbc_csr.port_${p}.cfg_oq_xoff2oq.show();
                    pbc_csr.port_${p}.cfg_oq_xoff2oq.write();

                    pbc_csr.port_${p}.cfg_mac_xoff.show();
                    pbc_csr.port_${p}.cfg_mac_xoff.write();
                }
                break;
            }
//::    #endif
//:: #endfor
        default:
            return HAL_RET_INVALID_ARG;
    }

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

    uint32_t max_nodes = HAL_TM_COUNT_L${level}_NODES;

    if (node >= max_nodes) {
        HAL_TRACE_ERR("node {} exceeds the number of valid level "
                      "${level} nodes in port {}",
                      node, port);
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
                if (tm_sw_cfg_write_enabled()) {
                    pbc_csr.port_${p}.cfg_oq_arb_l${parent_level}_selection.show();
                    pbc_csr.port_${p}.cfg_oq_arb_l${parent_level}_strict.show();

                    pbc_csr.port_${p}.cfg_oq_arb_l${parent_level}_selection.write();
                    pbc_csr.port_${p}.cfg_oq_arb_l${parent_level}_strict.write();
                }
                break;
            }
//::    #endif
//:: #endfor
        default:
            return HAL_RET_ERR;
    }

    pbc_csr.cfg_sched.enable_wrr(1);
    pbc_csr.cfg_sched.dhs_selection( ${level}*2 );
    if (tm_sw_cfg_write_enabled()) {
        pbc_csr.cfg_sched.write();
    }

    pbc_csr.cfg_dhs_mem.address(port*max_nodes + node);
    if (tm_sw_cfg_write_enabled()) {
        pbc_csr.cfg_dhs_mem.write();
    }

    uint32_t quota = (node_params->sched_type == TM_SCHED_TYPE_STRICT ?
                      node_params->strict.rate : node_params->dwrr.weight);

    pbc_csr.dhs_sched.entry[0].command(1);   //1: overwrite quota and credits
    pbc_csr.dhs_sched.entry[0].current_credit(quota);
    pbc_csr.dhs_sched.entry[0].quota(quota);
    if (tm_sw_cfg_write_enabled()) {
        pbc_csr.dhs_sched.entry[0].write();
    }
#endif
    HAL_TRACE_DEBUG("Updated the output queue scheduler on port {} "
                    "level {}, node {}",
                    port, ${level}, node);

    return HAL_RET_OK;
}

//:: #endfor

hal_ret_t
capri_tm_scheduler_map_update (uint32_t port,
                               tm_queue_node_type_e node_type,
                               uint32_t node,
                               tm_queue_node_params_t *node_params)
{
    hal_ret_t ret = HAL_RET_OK;

    if (!is_valid_tm_port(port)) {
        HAL_TRACE_ERR("{} is not a valid TM port",
                      port);
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
capri_tm_uplink_lif_set (uint32_t port,
                         uint32_t lif)
{
    if (!capri_tm_port_is_uplink_port(port)) {
        HAL_TRACE_ERR("{} is not a valid TM uplink port",
                      port);
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
    pbc_csr.cfg_src_port_to_lif_map.show();
    pbc_csr.cfg_src_port_to_lif_map.write();
#endif

    HAL_TRACE_DEBUG("Set the lif {} on port {}",
                    lif, port);

    return HAL_RET_OK;
}

uint32_t
capri_tm_get_hbm_occupancy(tm_hbm_fifo_type_e fifo_type, uint32_t context)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    cap_pbchbm_csr_t &hbm_csr = pbc_csr.hbm;
    uint32_t occupancy = UINT32_MAX;

    if (context >= capri_tm_max_hbm_contexts_for_fifo(fifo_type)) {
        HAL_TRACE_ERR("Invalid context {} for fifo {}", context, fifo_type);
        return HAL_RET_INVALID_ARG;
    }

    switch(fifo_type) {
//:: for fifo_type, finfo in hbm_fifo_info.items():
        case ${fifo_type}:
            switch (context) {
//::    for context in range(finfo['count']):
                case ${context}:
                    {
                        hbm_csr.sta_hbm_${finfo["reg_name"]}_context_${context}.read();
                        hbm_csr.sta_hbm_${finfo["reg_name"]}_context_${context}.show();
                        occupancy =
                            hbm_csr.sta_hbm_${finfo["reg_name"]}_context_${context}.depth().convert_to<uint32_t>();
                    }
                    break;
//::    #endfor
            }
            break;
//:: #endfor
        default:
            return occupancy;
    }

    return occupancy;
}

static hal_ret_t
capri_tm_drain_uplink_port (tm_port_t port)
{
    bool all_zeroes = false;
    uint32_t tries = 0;
    uint32_t max_tries = 1000;
    uint32_t occupancy;
    uint32_t num_hbm_contexts_per_port;
    uint32_t context;
    uint32_t i;

    num_hbm_contexts_per_port = tm_cfg_profile()->num_qs[TM_PORT_TYPE_UPLINK];
    while (port_supports_hbm_contexts(port) &&
           !all_zeroes && (tries < max_tries)) {
        all_zeroes = true;
        for (i = 0; i < num_hbm_contexts_per_port; i++) {
            context = (port * num_hbm_contexts_per_port) + i;
            occupancy = capri_tm_get_hbm_occupancy(TM_HBM_FIFO_TYPE_UPLINK, context);
            if (occupancy) {
                all_zeroes = false;
            }
        }
        // TODO: Do we need a sleep here ?
        usleep(1000);
        tries++;
    }

    if (!all_zeroes && port_supports_hbm_contexts(port)) {
        HAL_TRACE_ERR("Port {} hbm queues not drained completely after {} tries",
                      port, tries);
        return HAL_RET_RETRY;
    }

    return HAL_RET_OK;
}

hal_ret_t
capri_tm_enable_disable_uplink_port (tm_port_t port, bool enable)
{
    hal_ret_t ret = HAL_RET_OK;
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;

    if (!capri_tm_port_is_uplink_port(port)) {
        HAL_TRACE_ERR("{} is not a valid TM uplink port",
                      port);
        return HAL_RET_INVALID_ARG;
    }

    if (enable) {
        /* Make sure the contexts are free */
        ret = capri_tm_drain_uplink_port(port);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Port {} is not fully drained. Retry later", port);
            return HAL_RET_RETRY;
        }
    }

    switch(port) {
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
//::    if pinfo["type"] == "uplink":
        case ${pinfo["enum"]}:
            {
                pbc_csr.port_${p}.cfg_write_control.read();
                pbc_csr.port_${p}.cfg_oq.read();

                pbc_csr.port_${p}.cfg_write_control.enable(enable ? 1 : 0);
                pbc_csr.port_${p}.cfg_oq.flush(enable ? 0 : 1);

                if (tm_sw_cfg_write_enabled()) {
                    pbc_csr.port_${p}.cfg_write_control.show();
                    pbc_csr.port_${p}.cfg_write_control.write();

                    pbc_csr.port_${p}.cfg_oq.show();
                    pbc_csr.port_${p}.cfg_oq.write();
                }
            }
//::    #endif
//:: #endfor
        default:
            return HAL_RET_ERR;
    }

    /* If we're disabling the port, we need to wait until all HBM contexts are
     * free
     */
    if (!enable) {
        ret = capri_tm_drain_uplink_port(port);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Port {} is not fully drained", port);
            // Ignore the return status and continue
            ret = HAL_RET_OK;
        }
    }

    HAL_TRACE_DEBUG("{}d uplink port {}",
                    enable ? "Enable" : "Disable", port);
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

static hal_ret_t
alloc_cells (tm_port_type_e port_type, uint32_t *pbc_cell_chunks,
             capri_tm_buf_cfg_t *buf_cfg)
{
    hal_ret_t ret = HAL_RET_OK;
    uint32_t reserved_mtus;
    uint32_t reserved_cells;
    uint32_t headroom_cells;
    uint32_t chunks_per_q;
    uint32_t island;
    uint32_t num_qs;
    uint32_t reserved_chunks;
    uint32_t headroom_chunks;
    uint32_t chunks_needed;

    island = capri_tm_get_island_for_port_type(port_type);

    num_qs = tm_cfg_profile()->num_qs[port_type];
    if (port_type == TM_PORT_TYPE_UPLINK) {
        num_qs =
            1 + (tm_cfg_profile()->num_qs[port_type] * tm_cfg_profile()->num_active_uplink_ports);
    }

    reserved_mtus = tm_asic_profile()->port[port_type].reserved_mtus;
    headroom_cells = tm_asic_profile()->port[port_type].headroom_cells;

    headroom_chunks = cells_to_chunks(headroom_cells);

    if (reserved_mtus) {
        // Allocate reserved_mtus and headroom_cells from the given island
        reserved_cells =
            bytes_to_cells(tm_cfg_profile()->jumbo_mtu) * reserved_mtus;

        reserved_chunks = cells_to_chunks(reserved_cells);

        chunks_needed = (reserved_chunks + headroom_chunks) * num_qs;
        if (pbc_cell_chunks[island] < chunks_needed) {
            HAL_TRACE_ERR("Error allocating reserved pbc chunks "
                          "island {} port_type {} num_qs {} reserved_cells {} "
                          "headroom_cells {} available {}",
                          island, port_type, num_qs, reserved_cells,
                          headroom_cells, chunks_to_cells(pbc_cell_chunks[island]));
            return HAL_RET_NO_RESOURCE;
        }
        pbc_cell_chunks[island] -= chunks_needed;
        buf_cfg->chunks_per_q[port_type] = reserved_chunks;
    } else {
        // Allocate the remaining chunks in the island for every queue
        chunks_per_q = pbc_cell_chunks[island]/num_qs;
        if (chunks_per_q < headroom_chunks) {
            HAL_TRACE_ERR("Error allocating remaining pbc chunks "
                          "island {} port_type {} num_qs {} "
                          "headroom_cells {} available {} per_q available {}",
                          island, port_type, num_qs,
                          headroom_cells,
                          chunks_to_cells(pbc_cell_chunks[island]),
                          chunks_to_cells(chunks_per_q));
            return HAL_RET_NO_RESOURCE;
        }
        pbc_cell_chunks[island] -= chunks_per_q * num_qs;
        buf_cfg->chunks_per_q[port_type] = chunks_per_q - headroom_chunks;
    }
    if (chunks_to_cells(buf_cfg->chunks_per_q[port_type]) <
        bytes_to_cells(tm_cfg_profile()->jumbo_mtu)) {
        HAL_TRACE_ERR("Error reserved cells {}/bytes {} is less than the "
                      "jumbo mtu  {}",
                      chunks_to_cells(buf_cfg->chunks_per_q[port_type]),
                      cells_to_bytes(chunks_to_cells(buf_cfg->chunks_per_q[port_type])),
                      tm_cfg_profile()->jumbo_mtu);
        return HAL_RET_NO_RESOURCE;
    }
    HAL_TRACE_DEBUG("allocated cells {} port_type {} headroom {}",
                    chunks_to_cells(buf_cfg->chunks_per_q[port_type]),
                    port_type,
                    chunks_to_cells(headroom_chunks));

    return ret;
}

static hal_ret_t
capri_tm_alloc_pbc_buffers (capri_tm_buf_cfg_t *buf_cfg)
{
    hal_ret_t ret = HAL_RET_OK;
    uint32_t pbc_cell_chunks[HAL_TM_NUM_BUFFER_ISLANDS] = {0};

    for (unsigned i = 0; i < HAL_ARRAY_SIZE(pbc_cell_chunks); i++) {
        pbc_cell_chunks[i] = capri_tm_get_max_cell_chunks_for_island(i);
    }

    /* First allocate buffer cells for the P4 ports and
     * then distribute the remaining equally among the different classes
     */
    ret = alloc_cells(TM_PORT_TYPE_P4IG, pbc_cell_chunks, buf_cfg);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    ret = alloc_cells(TM_PORT_TYPE_P4EG, pbc_cell_chunks, buf_cfg);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    /* Now allocate the remaining uniformly */
    ret = alloc_cells(TM_PORT_TYPE_UPLINK, pbc_cell_chunks, buf_cfg);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    ret = alloc_cells(TM_PORT_TYPE_DMA, pbc_cell_chunks, buf_cfg);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    for (unsigned i = 0; i < HAL_ARRAY_SIZE(pbc_cell_chunks); i++) {
        HAL_TRACE_DEBUG("unallocated cells island {} cells {}",
                        i, chunks_to_cells(pbc_cell_chunks[i]));
    }

    return HAL_RET_OK;
}

static hal_ret_t
capri_tm_alloc_hbm_buffers (capri_tm_buf_cfg_t *buf_cfg)
{
    uint32_t num_hbm_contexts[NUM_TM_HBM_FIFO_TYPES] = {0};
    uint64_t total_hbm_chunks;
    uint32_t fifo_type;

    // HBM allocation
    //
    // Out of the whole available HBM payload and control has to be carved.
    // Control needs to be 1/50th of payload
    num_hbm_contexts[TM_HBM_FIFO_TYPE_UPLINK] =
        tm_cfg_profile()->num_qs[TM_PORT_TYPE_UPLINK] * tm_cfg_profile()->num_active_uplink_ports;
    num_hbm_contexts[TM_HBM_FIFO_TYPE_TXDMA] = tm_cfg_profile()->num_qs[TM_PORT_TYPE_DMA];

    total_hbm_chunks = tm_cfg_profile()->hbm_fifo_size/HAL_TM_HBM_FIFO_ALLOC_SIZE;

    // This calculation involves taking the floor value for both
    // total_hbm_chunks and control_chunks
    uint64_t control_chunks;
    uint64_t payload_chunks;

    control_chunks = total_hbm_chunks/(1 + tm_asic_profile()->hbm_fifo_control_scale_factor);
    payload_chunks = control_chunks * tm_asic_profile()->hbm_fifo_control_scale_factor;
    HAL_ASSERT((payload_chunks + control_chunks) <= total_hbm_chunks);

    HAL_TRACE_DEBUG("Available hbm chunks total {} payload {} control {}",
                    total_hbm_chunks, payload_chunks, control_chunks);

    uint64_t total_reserved_hbm_chunks = 0;
    uint64_t total_hbm_contexts = 0;
    uint64_t reserved_hbm_chunks_per_context[NUM_TM_HBM_FIFO_TYPES] = {0};
    for (fifo_type = 0; fifo_type < NUM_TM_HBM_FIFO_TYPES; fifo_type++) {
        reserved_hbm_chunks_per_context[fifo_type] =
            hbm_bytes_to_chunks(
                tm_asic_profile()->hbm_fifo_reserved_bytes_per_context[fifo_type]);

        if (num_hbm_contexts[fifo_type] > capri_tm_max_hbm_contexts_for_fifo(fifo_type)) {
            HAL_TRACE_ERR("num hbm contexts {} for fifo {} exceeds "
                          "max available {}",
                          num_hbm_contexts[fifo_type],
                          fifo_type,
                          capri_tm_max_hbm_contexts_for_fifo(fifo_type));
            return HAL_RET_INVALID_ARG;
        }
        uint64_t reserved_hbm_chunks =
            reserved_hbm_chunks_per_context[fifo_type] * num_hbm_contexts[fifo_type];

        total_reserved_hbm_chunks += reserved_hbm_chunks;
        total_hbm_contexts += num_hbm_contexts[fifo_type];
    }

    if (payload_chunks < total_reserved_hbm_chunks) {
        HAL_TRACE_ERR("Error allocating hbm fifo . Available chunks {} "
                      "total reserved required {} ",
                      payload_chunks, total_reserved_hbm_chunks);
        return HAL_RET_NO_RESOURCE;
    }
    uint64_t rem_payload_chunks = payload_chunks - total_reserved_hbm_chunks;

    // Allocate the rem_payload_chunks for the total number of queues
    //
    uint64_t payload_chunks_per_context = rem_payload_chunks/total_hbm_contexts;

    HAL_TRACE_DEBUG("HBM fifo allocation total payload {}, reserved {} "
                    "remaining {} total_contexts {} "
                    "payload chunks per context {}",
                    payload_chunks, total_reserved_hbm_chunks,
                    rem_payload_chunks, total_hbm_contexts, payload_chunks_per_context);

    uint64_t offset = 0;
    for (fifo_type = 0; fifo_type < NUM_TM_HBM_FIFO_TYPES; fifo_type++) {
        for (uint32_t context = 0; context < num_hbm_contexts[fifo_type]; context++) {
            uint64_t payload_chunks_needed =
                payload_chunks_per_context + reserved_hbm_chunks_per_context[fifo_type];
            uint64_t control_chunks_needed =
                (payload_chunks_needed + tm_asic_profile()->hbm_fifo_control_scale_factor - 1)/
                tm_asic_profile()->hbm_fifo_control_scale_factor;

            buf_cfg->hbm_fifo[fifo_type][context].valid = true;
            buf_cfg->hbm_fifo[fifo_type][context].payload_offset = offset;
            buf_cfg->hbm_fifo[fifo_type][context].payload_chunks = payload_chunks_needed;
            offset += payload_chunks_needed;
            buf_cfg->hbm_fifo[fifo_type][context].control_offset = offset;
            buf_cfg->hbm_fifo[fifo_type][context].control_chunks = control_chunks_needed;
            offset += control_chunks_needed;
        }
    }
    HAL_ASSERT(offset <= (control_chunks + payload_chunks));
    HAL_TRACE_DEBUG("unallocated hbm chunks {} total {}",
                    (control_chunks + payload_chunks) - offset,
                    total_hbm_chunks);
    return HAL_RET_OK;
}

static bool
iq_disabled (tm_port_t port, uint32_t iq)
{
    tm_port_type_e port_type;
    bool disabled = false;
    uint32_t max_iqs;
    uint32_t num_iqs;
    uint32_t disabled_iq_start, disabled_iq_end;

    port_type = capri_tm_get_port_type(port);
    num_iqs = tm_cfg_profile()->num_qs[port_type];

    switch(port_type) {
        case TM_PORT_TYPE_UPLINK:
            if (port == TM_PORT_NCSI) {
                // On the BMC port, only one pg is supported
                num_iqs = 1;
            } else if (!is_active_uplink_port(port)) {
                disabled = true;
            }
            if (iq >= num_iqs) {
                disabled = true;
            }
            break;
        case TM_PORT_TYPE_P4EG:
        case TM_PORT_TYPE_P4IG:
            max_iqs = capri_tm_get_num_iqs_for_port_type(port_type);
            HAL_ASSERT(max_iqs > num_iqs);
            disabled_iq_start = HAL_TM_P4_UPLINK_IQ_OFFSET - (max_iqs - num_iqs) - 1;
            disabled_iq_end = HAL_TM_P4_UPLINK_IQ_OFFSET - 1;
            if ((iq >= disabled_iq_start) && (iq < disabled_iq_end)) {
                disabled = true;
            }
            break;
        case TM_PORT_TYPE_DMA:
            if (iq >= num_iqs) {
                disabled = true;
            }
            break;
        case NUM_TM_PORT_TYPES:
            disabled = true;
            break;
    }

    if (disabled) {
        HAL_TRACE_DEBUG("iq {} on port {} disabled",
                        iq, port);
    }

    return disabled;
}

static hal_ret_t
capri_tm_program_pbc_buffers (capri_tm_buf_cfg_t *buf_cfg)
{
    hal_ret_t ret = HAL_RET_OK;
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    uint32_t reserved_min;
    uint32_t headroom;
    uint32_t xon_threshold;
    tm_port_type_e port_type;

    // Program the buffers
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
    port_type = capri_tm_get_port_type(${pinfo["enum"]});
//::    for pg in range(port_info[p]["pgs"]):
    reserved_min = 0;
    headroom = 0;
    xon_threshold = 0;
    if (!iq_disabled(${pinfo["enum"]}, ${pg})) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            tm_asic_profile()->port[port_type].recirc_q != ${pg}) {
            reserved_min = buf_cfg->chunks_per_q[port_type] +
                cells_to_chunks(bytes_to_cells(tm_cfg_profile()->jumbo_mtu)) + 2;
            headroom = 0;
        } else {
            reserved_min = buf_cfg->chunks_per_q[port_type];
            headroom = cells_to_chunks(tm_asic_profile()->port[port_type].headroom_cells);
        }
    }

    if (headroom) {
        HAL_ASSERT(chunks_to_cells(reserved_min) >=
                   (bytes_to_cells(tm_cfg_profile()->jumbo_mtu) + 1));
        xon_threshold = chunks_to_cells(reserved_min) -
            (bytes_to_cells(tm_cfg_profile()->jumbo_mtu) + 1);
    }
    pbc_csr.port_${p}.cfg_account_pg_${pg}.read();
    /* Update the PG parameters */
    pbc_csr.port_${p}.cfg_account_pg_${pg}.reserved_min(reserved_min);
    pbc_csr.port_${p}.cfg_account_pg_${pg}.headroom(headroom);
    pbc_csr.port_${p}.cfg_account_pg_${pg}.xon_threshold(xon_threshold);
    pbc_csr.port_${p}.cfg_account_pg_${pg}.low_limit(0);
    pbc_csr.port_${p}.cfg_account_pg_${pg}.alpha(0);

    if (tm_sw_init_enabled()) {
        pbc_csr.port_${p}.cfg_account_pg_${pg}.show();
        pbc_csr.port_${p}.cfg_account_pg_${pg}.write();
    }
//::    #endfor
//:: #endfor
    return ret;
}

static hal_ret_t
capri_tm_program_p4_credits (capri_tm_buf_cfg_t *buf_cfg)
{
    hal_ret_t ret = HAL_RET_OK;
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    tm_port_type_e port_type;
    uint32_t credit_enable;
    uint32_t credits;
    uint32_t qs_to_flush;
    uint32_t recirc_q_val;
    cpp_int max_growth;
    cap_pbc_max_growth_map_t max_growth_decoder;
    max_growth_decoder.init();

    // Program the buffers
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
//::    if pinfo["supports_credits"]:
    port_type = capri_tm_get_port_type(${pinfo["enum"]});
    credit_enable = 0;
    qs_to_flush = 0;
    max_growth = 0;
    recirc_q_val = 1<<tm_asic_profile()->port[port_type].recirc_q;
//::        for pg in range(port_info[p]["pgs"]):
    credits = 0;
    if (!iq_disabled(${pinfo["enum"]}, ${pg})) {
        if (tm_asic_profile()->port[port_type].uses_credits &&
            (tm_asic_profile()->port[port_type].recirc_q != ${pg})) {
            credits = chunks_to_cells(buf_cfg->chunks_per_q[port_type]);
            credit_enable |= 1<<${pg};

            pbc_csr.hlp.set_slc(max_growth, 1, ${pg} * 5, ((${pg}+1)*5)-1);

            // Program credits
            // pbc_csr.port_${p}.dhs_oq_flow_control.entry[${pg}].read();
            pbc_csr.port_${p}.dhs_oq_flow_control.entry[${pg}].entry(credits);
            if (tm_sw_init_enabled()) {
                pbc_csr.port_${p}.dhs_oq_flow_control.entry[${pg}].show();
                pbc_csr.port_${p}.dhs_oq_flow_control.entry[${pg}].write();
            }
        }
    } else {
        qs_to_flush |= 1<<${pg};
    }
//::        #endfor
    // Program the credit_enable
    if (credit_enable) {
        pbc_csr.port_${p}.cfg_account_credit_return.read();
        pbc_csr.port_${p}.cfg_oq_queue.read();
        pbc_csr.cfg_credits_max_growth_${p}.read();

        pbc_csr.port_${p}.cfg_account_credit_return.enable(credit_enable);
        pbc_csr.port_${p}.cfg_oq_queue.recirc(recirc_q_val);
        pbc_csr.port_${p}.cfg_oq_queue.flush(qs_to_flush);
        pbc_csr.cfg_credits_max_growth_${p}.cells(max_growth);

        max_growth_decoder.all(pbc_csr.cfg_credits_max_growth_${p}.cells());
        max_growth_decoder.set_name("cap0.pb.pbc.cfg_credits_max_growth_${p}.decoder");
        if (tm_sw_init_enabled()) {
            max_growth_decoder.show();

            pbc_csr.port_${p}.cfg_account_credit_return.show();
            pbc_csr.port_${p}.cfg_oq_queue.show();
            pbc_csr.cfg_credits_max_growth_${p}.show();

            pbc_csr.port_${p}.cfg_account_credit_return.write();
            pbc_csr.port_${p}.cfg_oq_queue.write();
            pbc_csr.cfg_credits_max_growth_${p}.write();
        }
    }
//::    #endif
//:: #endfor

    return ret;
}

static hal_ret_t
capri_tm_program_hbm_buffers (capri_tm_buf_cfg_t *buf_cfg)
{
    hal_ret_t ret = HAL_RET_OK;
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    cap_pbchbm_csr_t &hbm_csr = pbc_csr.hbm;
    uint32_t num_hbm_contexts_per_port;
    uint32_t context;
    uint32_t num_contexts;
    uint32_t base_offset;
    cpp_int port_payload_base_val;
    cpp_int port_payload_size_val;
    cpp_int port_payload_occupancy_val;
    cpp_int port_control_base_val;
    cpp_int port_control_size_val;

    cpp_int payload_base_val[NUM_TM_HBM_FIFO_TYPES];
    cpp_int payload_size_val[NUM_TM_HBM_FIFO_TYPES];
    cpp_int control_base_val[NUM_TM_HBM_FIFO_TYPES];
    cpp_int control_size_val[NUM_TM_HBM_FIFO_TYPES];
    cpp_int eth_xoff_val;
    cpp_int eth_xon_val;
    uint64_t payload_offset;
    uint64_t payload_size;
    uint64_t payload_occupancy;
    uint64_t control_offset;
    uint64_t control_size;

    uint32_t fifo_type;

    // On active uplink ports and the DMA port, setup the HBM queues
    num_hbm_contexts_per_port = tm_cfg_profile()->num_qs[TM_PORT_TYPE_UPLINK];

//:: fifo_types = {"uplink": "TM_HBM_FIFO_TYPE_UPLINK" ,
//::               "dma" : "TM_HBM_FIFO_TYPE_TXDMA" }
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
//::    if pinfo["has_hbm"]:
    // ${pinfo["enum"]}
//::        fifo_type = fifo_types[pinfo["type"]]
//::        reg_name = hbm_fifo_info[fifo_type]["reg_name"]
//::        if pinfo["type"] == "uplink":
    if (is_active_uplink_port(${pinfo["enum"]})) {
        // If we are an active port, then setup the HBM fifo contexts
        num_contexts = num_hbm_contexts_per_port;
    } else {
        num_contexts = 0;
    }
    base_offset = ${p} * num_contexts;
//::        elif pinfo["type"] == "dma":
    num_contexts = capri_tm_max_hbm_contexts_for_fifo(TM_HBM_FIFO_TYPE_TXDMA);
    base_offset = 0;
//::        #endif

    fifo_type = ${fifo_type};
    port_payload_base_val = 0;
    port_payload_size_val = 0;
    port_payload_occupancy_val = 0;
    port_control_base_val = 0;
    port_control_size_val = 0;

    for (unsigned q = 0; q < num_contexts; q++) {
        context = base_offset + q;

        capri_tm_buf_hbm_cfg_t *hbm_cfg = &buf_cfg->hbm_fifo[fifo_type][context];
        if (hbm_cfg->valid) {
            payload_offset = hbm_cfg->payload_offset;
            payload_size = hbm_cfg->payload_chunks;
            // payload occupancy is in units of 1024 bytes
            // program it to drop when 1 jumbo-mtu worth of bytes still free
            payload_occupancy =
                (hbm_chunks_to_bytes(payload_size) - tm_cfg_profile()->jumbo_mtu) >> 10;
            control_offset = hbm_cfg->control_offset;
            control_size = hbm_cfg->control_chunks;
        } else {
            payload_offset = 0;
            payload_size = 0;
            payload_occupancy = 0;
            control_offset = 0;
            control_size = 0;
        }

        // Per port registers
        // 27 bits per hbm_q
        pbc_csr.hlp.set_slc(port_payload_base_val, payload_offset,
                            q * 27, ((q + 1) * 27) - 1);
        // 23 bits per q
        pbc_csr.hlp.set_slc(port_payload_size_val, payload_size,
                            q * 23, ((q + 1) * 23) - 1);
        // 19 bits per q
        pbc_csr.hlp.set_slc(port_payload_occupancy_val, payload_occupancy,
                            q * 19, ((q + 1) * 19) - 1);
        // 27 bits per q
        pbc_csr.hlp.set_slc(port_control_base_val, control_offset,
                            q * 27, ((q + 1) * 27) - 1);
        // 23 bits per q
        pbc_csr.hlp.set_slc(port_control_size_val, control_size,
                            q * 23, ((q + 1) * 23) - 1);

        // Global registers

        // 27 bits per hbm_q
        pbc_csr.hlp.set_slc(payload_base_val[fifo_type], payload_offset,
                            context * 27, ((context + 1) * 27) - 1);
        // 23 bits per context
        pbc_csr.hlp.set_slc(payload_size_val[fifo_type], payload_size,
                            context * 23, ((context + 1) * 23) - 1);
        // 27 bits per context
        pbc_csr.hlp.set_slc(control_base_val[fifo_type], control_offset,
                            context * 27, ((context + 1) * 27) - 1);
        // 23 bits per context
        pbc_csr.hlp.set_slc(control_size_val[fifo_type], control_size,
                            context * 23, ((context + 1) * 23) - 1);
    }

    hbm_csr.hbm_port_${p}.cfg_hbm_${reg_name}_payload.base(port_payload_base_val);
    hbm_csr.hbm_port_${p}.cfg_hbm_${reg_name}_payload.mem_sz(port_payload_size_val);
    hbm_csr.hbm_port_${p}.cfg_hbm_${reg_name}_payload_occupancy.threshold(port_payload_occupancy_val);

//::        if pinfo["type"] == "uplink":
    hbm_csr.hbm_port_${p}.cfg_hbm_${reg_name}_ctrl.base(port_control_base_val);
    hbm_csr.hbm_port_${p}.cfg_hbm_${reg_name}_ctrl.mem_sz(port_control_size_val);
//::        #endif


    hbm_csr.hbm_port_${p}.cfg_hbm_context.read();
    hbm_csr.hbm_port_${p}.cfg_hbm_context.enable((1ull<<num_contexts)-1);
    hbm_csr.hbm_port_${p}.cfg_hbm_context.base(base_offset);
    if (tm_sw_init_enabled()) {

        hbm_csr.hbm_port_${p}.cfg_hbm_${reg_name}_payload.show();
        hbm_csr.hbm_port_${p}.cfg_hbm_${reg_name}_payload.write();

        hbm_csr.hbm_port_${p}.cfg_hbm_${reg_name}_payload_occupancy.show();
        hbm_csr.hbm_port_${p}.cfg_hbm_${reg_name}_payload_occupancy.write();

//::        if pinfo["type"] == "uplink":
        hbm_csr.hbm_port_${p}.cfg_hbm_${reg_name}_ctrl.show();
        hbm_csr.hbm_port_${p}.cfg_hbm_${reg_name}_ctrl.write();
//::        #endif

        hbm_csr.hbm_port_${p}.cfg_hbm_context.show();
        hbm_csr.hbm_port_${p}.cfg_hbm_context.write();
    }
//::    #endif
//:: #endfor

//:: for fifo_type in fifo_types.values():
//::    reg_name = hbm_fifo_info[fifo_type]["reg_name"]

    hbm_csr.cfg_hbm_${reg_name}_payload.base(payload_base_val[${fifo_type}]);
    hbm_csr.cfg_hbm_${reg_name}_payload.mem_sz(payload_size_val[${fifo_type}]);
    hbm_csr.cfg_hbm_${reg_name}_ctrl.base(control_base_val[${fifo_type}]);
    hbm_csr.cfg_hbm_${reg_name}_ctrl.mem_sz(control_size_val[${fifo_type}]);

    // Write all the registers
    {
        cap_pbc_hbm_${reg_name}_ctl_t hbm_ctl_decoder;

        hbm_ctl_decoder.init();
        hbm_ctl_decoder.all(hbm_csr.cfg_hbm_${reg_name}_payload.all());
        hbm_ctl_decoder.set_name("cap0.pb.pbc.hbm.cfg_hbm_${reg_name}_payload.decoder");
//        hbm_ctl_decoder.show();

        hbm_ctl_decoder.init();
        hbm_ctl_decoder.all(hbm_csr.cfg_hbm_${reg_name}_ctrl.all());
        hbm_ctl_decoder.set_name("cap0.pb.pbc.hbm.cfg_hbm_${reg_name}_ctrl.decoder");
//        hbm_ctl_decoder.show();

    }

    if (tm_sw_init_enabled()) {
        hbm_csr.cfg_hbm_${reg_name}_payload.show();
        hbm_csr.cfg_hbm_${reg_name}_ctrl.show();

        hbm_csr.cfg_hbm_${reg_name}_payload.write();
        hbm_csr.cfg_hbm_${reg_name}_ctrl.write();
    }
//:: #endfor

    return ret;
}

static hal_ret_t
capri_tm_program_buffers (capri_tm_buf_cfg_t *buf_cfg)
{
    hal_ret_t ret = HAL_RET_OK;

    ret = capri_tm_program_pbc_buffers(buf_cfg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error programming pbc buffers ret {}",
                      ret);
        return ret;
    }

    ret = capri_tm_program_p4_credits(buf_cfg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error programming hbm buffers ret {}",
                      ret);
        return ret;
    }
    return ret;
}

static hal_ret_t
capri_tm_port_program_defaults (void)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    cpp_int tc_to_pg_val;
    cpp_int xoff2oq_map_val;
    tm_port_type_e port_type;
    uint32_t mtu_cells;

    // For every port, program the tc_to_pg mapping
    // and mtu

//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
    // ${pinfo["enum"]}
    port_type = capri_tm_get_port_type(${pinfo["enum"]});
    tc_to_pg_val = 0;

    pbc_csr.port_${p}.cfg_account_mtu_table.read();
    pbc_csr.port_${p}.cfg_account_tc_to_pg.read();

    mtu_cells = bytes_to_cells(tm_cfg_profile()->jumbo_mtu);
    if (port_type == TM_PORT_TYPE_UPLINK) {
        mtu_cells--;
    }
//::    nbits = int(math.log(pinfo["pgs"], 2))
//::    for pg in range(pinfo["pgs"]):
    pbc_csr.hlp.set_slc(tc_to_pg_val, ${pg}, ${pg} * ${nbits}, ((${pg}+1) * ${nbits}) - 1);
    pbc_csr.port_${p}.cfg_account_mtu_table.pg${pg}(mtu_cells);
//::    #endfor
    pbc_csr.port_${p}.cfg_account_tc_to_pg.table(tc_to_pg_val);

    if (tm_sw_init_enabled()) {
        cap_pbc_pg${pinfo["pgs"]}_map_t pg_map_decoder;
        pg_map_decoder.init();
        pg_map_decoder.all(pbc_csr.port_${p}.cfg_account_tc_to_pg.table());
        pg_map_decoder.set_name("cap0.pb.pbc.port_${p}.cfg_account_tc_to_pg.decoder");
        pg_map_decoder.show();

        pbc_csr.port_${p}.cfg_account_mtu_table.show();
        pbc_csr.port_${p}.cfg_account_tc_to_pg.show();

        pbc_csr.port_${p}.cfg_account_mtu_table.write();
        pbc_csr.port_${p}.cfg_account_tc_to_pg.write();
    }
//:: #endfor

    // On dma port, set the xoff to oq for flow control in RxDMA
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
//::    if pinfo["type"] == "dma":
    // ${pinfo["enum"]}
    xoff2oq_map_val = 0;
//::        for q in range(pinfo["qs"]):
    pbc_csr.hlp.set_slc(xoff2oq_map_val, ${q}, ${q}*5, ((${q}+1)*5)-1);
//::        #endfor

    pbc_csr.port_${p}.cfg_oq_xoff2oq.read();
    pbc_csr.port_${p}.cfg_oq_xoff2oq.map(xoff2oq_map_val);
    if (tm_sw_init_enabled()) {
        pbc_csr.port_${p}.cfg_oq_xoff2oq.show();

        pbc_csr.port_${p}.cfg_oq_xoff2oq.write();
    }
//::    #endif
//:: #endfor

    // On uplink ports, disable xoff on all oqs
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
//::    if pinfo["type"] == "uplink":
    // ${pinfo["enum"]}
    pbc_csr.port_${p}.cfg_mac_xoff.read();
    pbc_csr.port_${p}.cfg_mac_xoff.enable(0);
    if (tm_sw_init_enabled()) {
        pbc_csr.port_${p}.cfg_mac_xoff.show();
        pbc_csr.port_${p}.cfg_mac_xoff.write();
    }
//::    #endif
//:: #endfor

    // On uplink ports, set the number of header bytes to remove
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
    // ${pinfo["enum"]}
    port_type = capri_tm_get_port_type(${pinfo["enum"]});
    pbc_csr.port_${p}.cfg_oq.read();
//::    if pinfo["type"] == "uplink":
    pbc_csr.port_${p}.cfg_oq.num_hdr_bytes(
        CAPRI_GLOBAL_INTRINSIC_HDR_SZ + CAPRI_P4_INTRINSIC_HDR_SZ);
//::    #endif

    if (tm_sw_init_enabled()) {
        pbc_csr.port_${p}.cfg_oq.enable(1);
        pbc_csr.port_${p}.cfg_oq.rewrite_enable(1);
//::    if pinfo["supports_credits"]:
        pbc_csr.port_${p}.cfg_oq.flow_control_enable_credits(
            tm_asic_profile()->port[port_type].uses_credits ? 1 : 0);
//::    #endif
//::    if pinfo["enum"] == "TM_PORT_INGRESS":
        pbc_csr.port_${p}.cfg_oq.packing_msb(
            capri_tm_get_max_cell_chunks_for_island(0) >
            capri_tm_get_max_cell_chunks_for_island(1) ? 1 : 0) ;
//::    #endif
//::    if pinfo["type"] == "dma" or pinfo["type"] == "uplink":
        pbc_csr.port_${p}.cfg_oq.flow_control_enable_xoff(1);
//::    #endif
    }
    pbc_csr.port_${p}.cfg_oq.show();
    pbc_csr.port_${p}.cfg_oq.write();
//:: #endfor

    return HAL_RET_OK;
}

static hal_ret_t
capri_tm_init_pbc (capri_tm_buf_cfg_t *buf_cfg)
{
    hal_ret_t ret = HAL_RET_OK;

    ret = capri_tm_alloc_pbc_buffers(buf_cfg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error allocating buffer configs {}", ret);
        return ret;
    }

    ret = capri_tm_port_program_defaults();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error programming port defaults {}", ret);
        return ret;
    }

    ret = capri_tm_program_buffers(buf_cfg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error allocating buffer configs {}", ret);
        return ret;
    }

    return HAL_RET_OK;
}

static hal_ret_t
capri_tm_init_hbm_q_map (void)
{
    hal_ret_t ret = HAL_RET_OK;
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    cap_pbchbm_csr_t &hbm_csr = pbc_csr.hbm;
    tm_port_type_e port_type;
    uint32_t p4_oq;
    cpp_int oq_map_val;
    cpp_int hbm_tc_to_q_val;
    cap_pbc_oq_map_t oq_map_decoder;
    oq_map_decoder.init();

    // Map traffic to the contexts
    // On uplink by default, map everything to context 0
    // On DMA port, map each iq to each context
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
//::    if pinfo["has_hbm"] or pinfo["enum"] == "TM_PORT_NCSI":
    // ${pinfo["enum"]}
    hbm_tc_to_q_val = 0;
    port_type = capri_tm_get_port_type(${pinfo["enum"]});
//::        if pinfo["type"] == "dma":
    for (unsigned tc = 0; tc < tm_cfg_profile()->num_qs[port_type]; tc++) {
        pbc_csr.hlp.set_slc(hbm_tc_to_q_val, tc, tc*4, ((tc+1)*4)-1);
    }
//::        #endif
    hbm_csr.hbm_port_${p}.cfg_hbm_tc_to_q.read();
    hbm_csr.hbm_port_${p}.cfg_hbm_tc_to_q.table(hbm_tc_to_q_val);

    if (tm_sw_init_enabled()) {
        cap_pbc_pg${pinfo["pgs"]}_map_t pg_map_decoder;
        pg_map_decoder.init();
        pg_map_decoder.all(hbm_csr.hbm_port_${p}.cfg_hbm_tc_to_q.table());
        pg_map_decoder.set_name("cap0.pb.pbc.hbm.hbm_port_${p}.cfg_hbm_tc_to_q.decoder");
        pg_map_decoder.show();

        hbm_csr.hbm_port_${p}.cfg_hbm_tc_to_q.show();

        hbm_csr.hbm_port_${p}.cfg_hbm_tc_to_q.write();
    }
//::    #endif
//:: #endfor

    // Configure the parsers
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
//::    if pinfo["type"] == "uplink":
    // ${pinfo["enum"]}
    oq_map_val = 0;
    p4_oq = HAL_TM_P4_UPLINK_IQ_OFFSET;
    port_type = capri_tm_get_port_type(${pinfo["enum"]});

    hbm_csr.hbm_port_${p}.cfg_hbm_parser.read();
    hbm_csr.hbm_port_${p}.cfg_hbm_parser.use_dot1q(1);
    hbm_csr.hbm_port_${p}.cfg_hbm_parser.use_ip(0);
    hbm_csr.hbm_port_${p}.cfg_hbm_parser.default_cos(0);
    hbm_csr.hbm_port_${p}.cfg_hbm_parser.dscp_map(0);

    for (unsigned tc = 0; tc < tm_cfg_profile()->num_qs[port_type]; tc++) {
        pbc_csr.hlp.set_slc(oq_map_val, p4_oq, tc * 5, ((tc+1)*5)-1);
    }

    pbc_csr.cfg_parser${p}.default_cos(0);
    pbc_csr.cfg_parser${p}.oq_map(oq_map_val);

    oq_map_decoder.all(pbc_csr.cfg_parser${p}.oq_map());
    oq_map_decoder.set_name("cap0.pb.pbc.cfg_parser${p}.decoder");

    if (tm_sw_init_enabled()) {
        oq_map_decoder.show();

        hbm_csr.hbm_port_${p}.cfg_hbm_parser.show();
        pbc_csr.cfg_parser${p}.show();

        hbm_csr.hbm_port_${p}.cfg_hbm_parser.write();
        pbc_csr.cfg_parser${p}.write();
    }
//::    #endif
//:: #endfor

    return ret;
}

static hal_ret_t
capri_tm_init_hbm (capri_tm_buf_cfg_t *buf_cfg)
{
    hal_ret_t ret = HAL_RET_OK;
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    cap_pbchbm_csr_t &hbm_csr = pbc_csr.hbm;

    ret = capri_tm_alloc_hbm_buffers(buf_cfg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error allocating hbm buffers ret {}",
                      ret);
        return ret;
    }

    // Program the HBM buffers
    ret = capri_tm_program_hbm_buffers(buf_cfg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error programming hbm buffers ret {}",
                      ret);
        return ret;
    }

    ret = capri_tm_init_hbm_q_map();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error programming hbm q mapping ret {}",
                      ret);
        return ret;
    }

    // On Eth ports, configure rate-limiter from HBM
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
//::    if pinfo["type"] == "uplink" and pinfo["has_hbm"]:
    // ${pinfo["enum"]}
    hbm_csr.hbm_port_${p}.cfg_hbm.read();
    hbm_csr.hbm_port_${p}.cfg_hbm.rate_limiter(
        tm_asic_profile()->port[TM_PORT_TYPE_UPLINK].rate_limiter);
    if (tm_sw_init_enabled()) {
        hbm_csr.hbm_port_${p}.cfg_hbm.show();
        hbm_csr.hbm_port_${p}.cfg_hbm.write();
    }
//::    #endif
//:: #endfor

    // AXI Base Address
    pbc_csr.cfg_axi.read();
    pbc_csr.cfg_axi.base_addr(get_hbm_base());
    pbc_csr.cfg_axi.show();
    pbc_csr.cfg_axi.write();

    // AXI Base for buffer FIFOs
    pbc_csr.hbm.cfg_hbm_axi_base.read();
    pbc_csr.hbm.cfg_hbm_axi_base.addr(tm_cfg_profile()->hbm_fifo_base);
    pbc_csr.hbm.cfg_hbm_axi_base.show();
    pbc_csr.hbm.cfg_hbm_axi_base.write();

    return HAL_RET_OK;
}

static hal_ret_t
capri_tm_init_ports (void)
{
    hal_ret_t ret = HAL_RET_OK;
    capri_tm_buf_cfg_t buf_cfg = {0};

    ret = capri_tm_init_pbc(&buf_cfg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error initing pbc {}", ret);
        return ret;
    }

    ret = capri_tm_init_hbm(&buf_cfg);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error initing hbm {}", ret);
        return ret;
    }

    tm_ctx()->buf_cfg = buf_cfg;

    return ret;
}

static hal_ret_t
capri_tm_global_init (void)
{
    // Init the FC mgr and RC
    //
    // There are only 2 combinations valid with min_cell, max_row as either
    // 0, 4095 or 4096, 2559 . Choose one of them based on the island which
    // needs more cells
    uint32_t min_cells[] = { 0, 4096};
    uint32_t max_row[] = {4095, 2559};
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    cap_pbchbm_csr_t &hbm_csr = pbc_csr.hbm;

//:: for fifo_type, finfo in hbm_fifo_info.items():
//::    reg_name = finfo["reg_name"]
    hbm_csr.cfg_hbm_${reg_name}_ctrl_init.head_start(1);
    hbm_csr.cfg_hbm_${reg_name}_ctrl_init.tail_start(1);
    if (tm_sw_init_enabled()) {
        hbm_csr.cfg_hbm_${reg_name}_ctrl_init.write();
    }
//:: #endfor

//:: for fifo_type, finfo in hbm_fifo_info.items():
//::    reg_name = finfo["reg_name"]
    hbm_csr.cfg_hbm_${reg_name}_ctrl_init.head_start(0);
    hbm_csr.cfg_hbm_${reg_name}_ctrl_init.tail_start(0);
    if (tm_sw_init_enabled()) {
        hbm_csr.cfg_hbm_${reg_name}_ctrl_init.write();
    }
//:: #endfor


#if 0
    pbc_csr.cfg_fc_mgr_0.read();
    pbc_csr.cfg_fc_mgr_1.read();
    pbc_csr.cfg_island_control.read();
    pbc_csr.cfg_rc.read();
    pbc_csr.cfg_tail_drop.read();
#endif

    pbc_csr.cfg_fc_mgr_0.init_start(1);
    pbc_csr.cfg_fc_mgr_0.init_reset(0);
    pbc_csr.cfg_fc_mgr_1.init_start(1);
    pbc_csr.cfg_fc_mgr_1.init_reset(0);
    if (capri_tm_get_max_cell_chunks_for_island(0) >
        capri_tm_get_max_cell_chunks_for_island(1)) {
        pbc_csr.cfg_fc_mgr_0.max_row(max_row[0]);
        pbc_csr.cfg_fc_mgr_0.min_cell(min_cells[0]);
        pbc_csr.cfg_fc_mgr_1.max_row(max_row[1]);
        pbc_csr.cfg_fc_mgr_1.min_cell(min_cells[1]);
        pbc_csr.cfg_island_control.map(0);
    } else {
        pbc_csr.cfg_fc_mgr_0.max_row(max_row[1]);
        pbc_csr.cfg_fc_mgr_0.min_cell(min_cells[1]);
        pbc_csr.cfg_fc_mgr_1.max_row(max_row[0]);
        pbc_csr.cfg_fc_mgr_1.min_cell(min_cells[0]);
        pbc_csr.cfg_island_control.map(1);
    }

    pbc_csr.cfg_rc.init_start(1);
    pbc_csr.cfg_rc.init_reset(0);

    pbc_csr.cfg_tail_drop.cpu_threshold(tm_asic_profile()->cpu_copy_tail_drop_threshold);
    pbc_csr.cfg_tail_drop.span_threshold(tm_asic_profile()->span_tail_drop_threshold);

    // Write all the registers
    if (tm_sw_init_enabled()) {
        pbc_csr.cfg_fc_mgr_0.show();
        pbc_csr.cfg_fc_mgr_1.show();
        pbc_csr.cfg_island_control.show();
        pbc_csr.cfg_rc.show();
        pbc_csr.cfg_tail_drop.show();

        pbc_csr.cfg_tail_drop.write();
        pbc_csr.cfg_fc_mgr_0.write();
        pbc_csr.cfg_fc_mgr_1.write();
        pbc_csr.cfg_island_control.write();
        pbc_csr.cfg_rc.write();
        HAL_TRACE_DEBUG("TM global init done");
    }
    return HAL_RET_OK;
}

static hal_ret_t
capri_tm_init_enable_ports (void)
{
    hal_ret_t ret = HAL_RET_OK;
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;

    tm_port_t port;
    tm_port_type_e port_type;
    uint32_t enable;
    uint32_t rate_limiter;
    uint32_t cut_thru;
    uint32_t recirc_q;
    bool set_rate_limiter;
    bool set_cut_thru;

//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
    // ${pinfo["enum"]}
    port = ${pinfo["enum"]};
    port_type = capri_tm_get_port_type(port);
    enable = 0;
    rate_limiter = tm_asic_profile()->port[port_type].rate_limiter;
    cut_thru = 0;
    recirc_q = 0;
    set_rate_limiter = false;
    set_cut_thru = false;

    if (is_active_port(${pinfo["enum"]})) {
        enable = 1;
    }

    switch(port_type) {
        case TM_PORT_TYPE_UPLINK:
            if (port != TM_PORT_NCSI) {
                set_rate_limiter = true;
            }
            break;
        case TM_PORT_TYPE_P4IG:
        case TM_PORT_TYPE_P4EG:
            recirc_q = tm_asic_profile()->port[port_type].recirc_q;
            set_cut_thru = true;
            cut_thru = 0;
            break;
        case TM_PORT_TYPE_DMA:
            set_cut_thru = true;
            cut_thru = 0;
            break;
        case NUM_TM_PORT_TYPES:
            break;
    }

    pbc_csr.port_${p}.cfg_write_control.read();

    pbc_csr.port_${p}.cfg_write_control.enable(enable);

    if (set_rate_limiter) {
        pbc_csr.port_${p}.cfg_write_control.rate_limiter(rate_limiter);
    }

    if (set_cut_thru) {
        pbc_csr.port_${p}.cfg_write_control.cut_thru(cut_thru);
    }

//::    if pinfo["type"] == "p4":
    pbc_csr.port_${p}.cfg_write_control.recirc_enable(1);
    pbc_csr.port_${p}.cfg_write_control.recirc_oq(recirc_q);
//::    #endif

    if (tm_sw_init_enabled()) {
        pbc_csr.port_${p}.cfg_write_control.show();
        pbc_csr.port_${p}.cfg_write_control.write();
    }
//:: #endfor

    return ret;
}

static hal_ret_t
populate_cfg_profile (capri_tm_cfg_profile_t *tm_cfg_profile,
                      sdk::lib::catalog *catalog)
{
    const sdk::lib::qos_profile_t *qos_profile;

    qos_profile = catalog->qos_profile();
    if (!qos_profile) {
        HAL_TRACE_ERR("QOS asic profile does not exist in the catalog");
        return HAL_RET_INVALID_ARG;
    }

    tm_cfg_profile->sw_init_enabled = qos_profile->sw_init_enable;
    tm_cfg_profile->sw_cfg_write_enabled = qos_profile->sw_cfg_write_enable;
    tm_cfg_profile->hbm_fifo_base = get_start_offset(CAPRI_HBM_REG_QOS_HBM_FIFO);
    tm_cfg_profile->hbm_fifo_size = get_size_kb(CAPRI_HBM_REG_QOS_HBM_FIFO) << 10;
    tm_cfg_profile->num_active_uplink_ports = catalog->num_uplink_ports();
    tm_cfg_profile->jumbo_mtu = qos_profile->jumbo_mtu;

    tm_cfg_profile->num_qs[TM_PORT_TYPE_UPLINK] = qos_profile->num_uplink_qs;
    tm_cfg_profile->num_qs[TM_PORT_TYPE_P4IG] = qos_profile->num_p4ig_qs;
    tm_cfg_profile->num_qs[TM_PORT_TYPE_P4EG] = qos_profile->num_p4eg_qs;
    tm_cfg_profile->num_qs[TM_PORT_TYPE_DMA] = qos_profile->num_dma_qs;


    if ((tm_cfg_profile->num_qs[TM_PORT_TYPE_UPLINK] *
         tm_cfg_profile->num_active_uplink_ports) > HAL_TM_MAX_HBM_ETH_CONTEXTS) {
        HAL_TRACE_WARN("num_uplink_ports {} with {} qs cannot be supported"
                       " reducing the num of qs to {}",
                       tm_cfg_profile->num_active_uplink_ports,
                       tm_cfg_profile->num_qs[TM_PORT_TYPE_UPLINK],
                       HAL_TM_MAX_HBM_ETH_CONTEXTS/tm_cfg_profile->num_active_uplink_ports);
        tm_cfg_profile->num_qs[TM_PORT_TYPE_UPLINK] =
            HAL_TM_MAX_HBM_ETH_CONTEXTS/tm_cfg_profile->num_active_uplink_ports;
    }

    if (!tm_cfg_profile->hbm_fifo_size) {
        HAL_TRACE_ERR("HBM allocation for QOS overflow fifo not available");
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

static hal_ret_t
capri_tm_update_perf_run_config (void)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    cap_pbc_csr_t &pbc_csr = cap0.pb.pbc;
    cap_pbchbm_csr_t &hbm_csr = pbc_csr.hbm;
//:: for p in range(TM_PORTS):
//::    pinfo = port_info[p]
//::    if pinfo["type"] == "uplink":
    // ${pinfo["enum"]}
    hbm_csr.hbm_port_${p}.cfg_hbm_parser.read();
    hbm_csr.hbm_port_${p}.cfg_hbm_parser.use_dot1q(0);
    hbm_csr.hbm_port_${p}.cfg_hbm_parser.use_ip(0);
    hbm_csr.hbm_port_${p}.cfg_hbm_parser.default_cos(0);
    hbm_csr.hbm_port_${p}.cfg_hbm_parser.dscp_map(0);
    hbm_csr.hbm_port_${p}.cfg_hbm_parser.show();
    hbm_csr.hbm_port_${p}.cfg_hbm_parser.write();
//::    #endif
//:: #endfor
    return HAL_RET_OK;
}

hal_ret_t
capri_tm_init (sdk::lib::catalog* catalog)
{
#ifndef HAL_GTEST
    hal_ret_t ret = HAL_RET_OK;
    capri_tm_cfg_profile_t tm_cfg_profile_;
    capri_tm_cfg_profile_t *tm_cfg_profile = &tm_cfg_profile_;

    capri_tm_asic_profile_t asic_profile;

    populate_asic_profile(&asic_profile);
    ret = populate_cfg_profile(tm_cfg_profile, catalog);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error populating config profile from the catalog ret {}",
                      ret);
        return ret;
    }

    HAL_TRACE_DEBUG("cfg-profile sw_init_en {} sw_cfg_write_en {} jumbo_mtu {} "
                    "num_active_uplink_ports {} hbm_fifo_size {} hbm_fifo_base {:#x}",
                    tm_cfg_profile->sw_init_enabled,
                    tm_cfg_profile->sw_cfg_write_enabled,
                    tm_cfg_profile->jumbo_mtu,
                    tm_cfg_profile->num_active_uplink_ports,
                    tm_cfg_profile->hbm_fifo_size,
                    tm_cfg_profile->hbm_fifo_base);
    for (unsigned port_type = 0; port_type < NUM_TM_PORT_TYPES; port_type++) {
        if (tm_cfg_profile->num_qs[port_type] >
            capri_tm_get_num_iqs_for_port_type((tm_port_type_e)port_type)) {
            HAL_TRACE_ERR("Error cfg-profile port_type {} num_qs {} "
                          "supported {}",
                          (tm_port_type_e)port_type, tm_cfg_profile->num_qs[port_type],
                          capri_tm_get_num_iqs_for_port_type((tm_port_type_e)port_type));
            return HAL_RET_INVALID_ARG;
        }
        HAL_TRACE_DEBUG("cfg-profile port_type {} num_qs {} ",
                        (tm_port_type_e)port_type, tm_cfg_profile->num_qs[port_type]);
    }

    set_tm_ctx(tm_cfg_profile, &asic_profile);

    ret = capri_tm_global_init();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error initing global config ret {}", ret);
        return ret;
    }
    if (tm_sw_init_enabled()) {
        // Poll for the completion of the inits
        cap_pb_init_done(0,0);
    }

    ret = capri_tm_init_ports();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error initing ports ret {}", ret);
        return ret;
    }

    ret = capri_tm_init_enable_ports();
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error enabling ports ret {}", ret);
        return ret;
    }

#endif

    char *perf_run = getenv("PERF_RUN");
    if (perf_run) {
        HAL_TRACE_DEBUG("perf-run env {}", perf_run);
        if (!strcmp(perf_run, "true")) {
            capri_tm_update_perf_run_config();
        }
    }
    HAL_TRACE_DEBUG("Init completed");

    return ret;
}

/* Programs the base address in HBM for the replication table */
hal_ret_t
capri_tm_repl_table_base_addr_set (uint64_t addr)
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
capri_tm_repl_table_token_size_set (uint32_t size_in_bits)
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
capri_tm_get_clock_tick (uint64_t *tick)
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
