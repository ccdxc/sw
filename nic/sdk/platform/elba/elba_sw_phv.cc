//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------


#include "lib/p4/p4_api.hpp"
#include "platform/elba/elba_sw_phv.hpp"

#include "third-party/asic/elba/model/utils/elb_blk_reg_model.h"
#include "third-party/asic/elba/model/elb_top/elb_top_csr.h"
#include "third-party/asic/elba/model/elb_ptd/elb_pt_csr.h"
#include "third-party/asic/elba/model/elb_prd/elb_pr_csr.h"
#include "third-party/asic/elba/model/elb_ppa/elb_ppa_csr.h"
#include "third-party/asic/elba/model/elb_dpa/elb_dpr_csr.h"
#include "third-party/asic/elba/model/elb_dpa/elb_dpp_csr.h"

namespace sdk  {
namespace platform {
namespace elba {

// elba_flit_t is used to access PHV data one flit at a time
typedef struct elba_flit_ {
    uint8_t flit_data[ASIC_FLIT_SIZE];
} elba_flit_t;


// elba_psp_swphv_init Initializes TxDMA(pt)/RxDMA(pr) SW PHV generator profiles
static sdk_ret_t
elba_psp_swphv_init (bool rx)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_psp_csr_t *psp_csr = &elb0.pt.pt.psp;
    if (rx) {
        psp_csr = &elb0.pr.pr.psp;
    }

    int index = 0;
    SDK_TRACE_DEBUG("Initializing PSP Global Config");

    psp_csr->cfg_sw_phv_global.start_enable(1);
    psp_csr->cfg_sw_phv_global.err_enable(0);
    psp_csr->cfg_sw_phv_global.write();

    for (index = 0; index < ASIC_SW_PHV_NUM_MEM_ENTRIES; index++) {
        elb_psp_csr_dhs_sw_phv_mem_entry_t &phv_mem_entry =
            psp_csr->dhs_sw_phv_mem.entry[index];
        phv_mem_entry.data(0);
        phv_mem_entry.write();
    }

    for (index = 0; index < ASIC_SW_PHV_NUM_PROFILES; index++) {
        elb_psp_csr_cfg_sw_phv_control_t &sw_phv_ctrl =
            psp_csr->cfg_sw_phv_control[index];
        sw_phv_ctrl.start_enable(0);
        sw_phv_ctrl.counter_repeat_enable(0);
        sw_phv_ctrl.qid_repeat_enable(0);
        sw_phv_ctrl.localtime_enable(0);
        sw_phv_ctrl.frame_size_enable(0);
        sw_phv_ctrl.packet_len_enable(0);
        sw_phv_ctrl.qid_enable(0);
        sw_phv_ctrl.write();

        elb_psp_csr_cfg_sw_phv_config_t &sw_phv_cfg =
            psp_csr->cfg_sw_phv_config[index];
        sw_phv_cfg.start_addr(index);
        sw_phv_cfg.num_flits(1);
        sw_phv_cfg.insertion_period_clocks(0);
        sw_phv_cfg.counter_max(0);
        sw_phv_cfg.qid_min(0);
        sw_phv_cfg.qid_max(0);
        sw_phv_cfg.write();
    }

    return SDK_RET_OK;
}

// elba_ppa_swphv_init initializes P4 Ingress/Egress sw phv units
static sdk_ret_t
elba_ppa_swphv_init ()
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    int pidx = 0;

    for (pidx = 0; pidx < ASIC_NUM_PPA; pidx++) {
        elb_ppa_csr_t &ppa_csr = elb0.ppa.ppa[pidx];
        elb_dpr_csr_t &dpr_csr = elb0.dpr.dpr[pidx];
        elb_dpp_csr_t &dpp_csr = elb0.dpp.dpp[pidx];
        int index = 0;

        SDK_TRACE_DEBUG("Initializing PPA Global Config");

        // enable sw phv
        ppa_csr.cfg_sw_phv_global.start_enable(1);
        ppa_csr.cfg_sw_phv_global.err_enable(0);
        ppa_csr.cfg_sw_phv_global.write();

        // enable drop phvs
        dpr_csr.cfg_global_2.dump_drop_no_data_phv(1);
        dpr_csr.cfg_global_2.write();
        dpp_csr.cfg_global_2.dump_drop_no_data_phv_0(1);
        dpp_csr.cfg_global_2.dump_drop_no_data_phv_1(1);
        dpp_csr.cfg_global_2.write();

        // init phv entries
        for (index = 0; index < ASIC_SW_PHV_NUM_MEM_ENTRIES; index++) {
            elb_ppa_csr_dhs_sw_phv_mem_entry_t &phv_mem_entry =
                ppa_csr.dhs_sw_phv_mem.entry[index];
            phv_mem_entry.data(0);
        }

        for (index = 0; index < ASIC_SW_PHV_NUM_PROFILES; index++) {
            elb_ppa_csr_cfg_sw_phv_control_t &sw_phv_ctrl =
                ppa_csr.cfg_sw_phv_control[index];

            sw_phv_ctrl.start_enable(0);
            sw_phv_ctrl.counter_repeat_enable(0);
            sw_phv_ctrl.qid_repeat_enable(0);
            sw_phv_ctrl.localtime_enable(0);
            sw_phv_ctrl.frame_size_enable(0);
            sw_phv_ctrl.packet_len_enable(0);
            sw_phv_ctrl.qid_enable(0);
            sw_phv_ctrl.write();

            elb_ppa_csr_cfg_sw_phv_config_t &sw_phv_cfg =
                ppa_csr.cfg_sw_phv_config[index];

            sw_phv_cfg.start_addr(index);
            sw_phv_cfg.num_flits(1);
            sw_phv_cfg.insertion_period_clocks(0);
            sw_phv_cfg.counter_max(0);
            sw_phv_cfg.qid_min(0);
            sw_phv_cfg.qid_max(0);
            sw_phv_cfg.write();
        }
    }

    return SDK_RET_OK;
}

// elba_sw_phv_init initializes Software PHV modules
sdk_ret_t
elba_sw_phv_init (void)
{
    sdk_ret_t ret = SDK_RET_OK;

    SDK_TRACE_DEBUG("Asic Sw PHV Init ");

    ret = elba_ppa_swphv_init();

    if (ret == SDK_RET_OK) {
        ret = elba_psp_swphv_init(false);
    }
    if (ret == SDK_RET_OK) {
        ret = elba_psp_swphv_init(true);
    }

    return ret;
}

// elba_psp_sw_phv_inject injects a PHV into Rx/Tx DMA pipeline
static sdk_ret_t
elba_pr_psp_sw_phv_inject (uint8_t prof_num, uint8_t start_idx,
                           uint8_t num_flits, void *data)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_psp_csr_t &pr_psp_csr = elb0.pr.pr.psp;
    pu_cpp_int< 512 > flit_data;

    SDK_TRACE_DEBUG("Injecting Software PHV.");

    int index = 0;
    elba_flit_t *curr_flit_ptr = (elba_flit_t *)data;

    // write flit data
    for (index = 0; index < num_flits; index++) {
        elba_flit_t rdata;
        // swap the bytes in data
        int i;
        for (i = 0; i < ASIC_FLIT_SIZE; i++) {
            rdata.flit_data[ASIC_FLIT_SIZE-1-i] = curr_flit_ptr->flit_data[i];
        }
        printf("\n");
        for (i = 0; i < ASIC_FLIT_SIZE; i++) {
            printf("%02x ", rdata.flit_data[i]);
        }
        printf("\n");

        elb_psp_csr_dhs_sw_phv_mem_entry_t &phv_mem_entry =
            pr_psp_csr.dhs_sw_phv_mem.entry[index];
        cpp_int_helper::s_cpp_int_from_array(flit_data, 0,
                                             (ASIC_FLIT_SIZE-1),
                                             (uint8_t *)curr_flit_ptr);
        phv_mem_entry.data(flit_data);
        phv_mem_entry.write();
        curr_flit_ptr++;
    }

    elb_psp_csr_cfg_sw_phv_config_t &sw_phv_cfg =
        pr_psp_csr.cfg_sw_phv_config[prof_num];
    sw_phv_cfg.start_addr(start_idx);
    sw_phv_cfg.num_flits(num_flits-1);
    sw_phv_cfg.insertion_period_clocks(0);
    sw_phv_cfg.counter_max(0);
    sw_phv_cfg.qid_min(0);
    sw_phv_cfg.qid_max(0);
    sw_phv_cfg.write();

    elb_psp_csr_cfg_sw_phv_control_t &sw_phv_ctrl =
        pr_psp_csr.cfg_sw_phv_control[prof_num];
    sw_phv_ctrl.start_enable(1);
    sw_phv_ctrl.counter_repeat_enable(0);
    sw_phv_ctrl.qid_repeat_enable(0);
    sw_phv_ctrl.localtime_enable(0);
    sw_phv_ctrl.frame_size_enable(0);
    sw_phv_ctrl.packet_len_enable(0);
    sw_phv_ctrl.qid_enable(0);
    sw_phv_ctrl.write();

    SDK_TRACE_DEBUG("Software PHV injected. done");
    return SDK_RET_OK;
}

// elba_psp_sw_phv_inject injects a PHV into Rx/Tx DMA pipeline
static sdk_ret_t
elba_pt_psp_sw_phv_inject (uint8_t prof_num, uint8_t start_idx,
                           uint8_t num_flits, void *data)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_psp_csr_t &pt_psp_csr = elb0.pt.pt.psp;
    pu_cpp_int< 512 > flit_data;

    SDK_TRACE_DEBUG("Injecting Software PHV.");

    int index = 0;
    elba_flit_t *curr_flit_ptr = (elba_flit_t *)data;

    // write flit data
    for (index = 0; index < num_flits; index++) {
        elba_flit_t rdata;
        // swap the bytes in data
        int i;
        for (i = 0; i < ASIC_FLIT_SIZE; i++) {
            rdata.flit_data[ASIC_FLIT_SIZE-1-i] = curr_flit_ptr->flit_data[i];
        }
        printf("\n");
        for (i = 0; i < ASIC_FLIT_SIZE; i++) {
            printf("%02x ", rdata.flit_data[i]);
        }
        printf("\n");

        elb_psp_csr_dhs_sw_phv_mem_entry_t &phv_mem_entry =
            pt_psp_csr.dhs_sw_phv_mem.entry[index];
        cpp_int_helper::s_cpp_int_from_array(flit_data, 0,
                                             (ASIC_FLIT_SIZE-1),
                                             curr_flit_ptr->flit_data);
        phv_mem_entry.data(flit_data);
        phv_mem_entry.write();
        curr_flit_ptr++;
    }

    elb_psp_csr_cfg_sw_phv_config_t &sw_phv_cfg =
        pt_psp_csr.cfg_sw_phv_config[prof_num];
    sw_phv_cfg.start_addr(start_idx);
    sw_phv_cfg.num_flits(num_flits-1);
    sw_phv_cfg.insertion_period_clocks(0);
    sw_phv_cfg.counter_max(0);
    sw_phv_cfg.qid_min(0);
    sw_phv_cfg.qid_max(0);
    sw_phv_cfg.write();

    elb_psp_csr_cfg_sw_phv_control_t &sw_phv_ctrl =
        pt_psp_csr.cfg_sw_phv_control[prof_num];
    sw_phv_ctrl.start_enable(1);
    sw_phv_ctrl.counter_repeat_enable(0);
    sw_phv_ctrl.qid_repeat_enable(0);
    sw_phv_ctrl.localtime_enable(0);
    sw_phv_ctrl.frame_size_enable(0);
    sw_phv_ctrl.packet_len_enable(0);
    sw_phv_ctrl.qid_enable(0);
    sw_phv_ctrl.write();

    SDK_TRACE_DEBUG("Software PHV injected. done");
    return SDK_RET_OK;
}

// elba_ppa_sw_phv_inject injects a PHV into P4 Ingress/Egress pipeline
static sdk_ret_t
elba_ppa_sw_phv_inject (uint8_t pidx, uint8_t prof_num, uint8_t start_idx,
                        uint8_t num_flits, void *data)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_ppa_csr_t &ppa_csr = elb0.ppa.ppa[pidx];
    pu_cpp_int< 512 > flit_data;

    SDK_TRACE_DEBUG("Injecting PHV into PPA {}", __func__, pidx);

    int index = 0;
    elba_flit_t *curr_flit_ptr = (elba_flit_t *)data;

    // write flit data
    for (index = 0; index < num_flits; index++) {
        elba_flit_t rdata;
        // swap the bytes in data
        int i;
        for (i = 0; i < ASIC_FLIT_SIZE; i++) {
            rdata.flit_data[ASIC_FLIT_SIZE-1-i] = curr_flit_ptr->flit_data[i];
        }
        printf("\n");
        for (i = 0; i < ASIC_FLIT_SIZE; i++) {
            printf("%02x ", rdata.flit_data[i]);
        }
        printf("\n");

        elb_ppa_csr_dhs_sw_phv_mem_entry_t &phv_mem_entry =
            ppa_csr.dhs_sw_phv_mem.entry[index];
        cpp_int_helper::s_cpp_int_from_array(flit_data, 0,
                                             (ASIC_FLIT_SIZE-1),
                                             (uint8_t *)curr_flit_ptr);
        phv_mem_entry.data(flit_data);
        phv_mem_entry.write();
        curr_flit_ptr++;
    }

    elb_ppa_csr_cfg_sw_phv_config_t &sw_phv_cfg =
        ppa_csr.cfg_sw_phv_config[prof_num];
    sw_phv_cfg.start_addr(start_idx);
    sw_phv_cfg.num_flits(num_flits);
    sw_phv_cfg.insertion_period_clocks(0);
    sw_phv_cfg.counter_max(0);
    sw_phv_cfg.qid_min(0);
    sw_phv_cfg.qid_max(0);
    sw_phv_cfg.write();

    elb_ppa_csr_cfg_sw_phv_control_t &sw_phv_ctrl =
        ppa_csr.cfg_sw_phv_control[prof_num];
    sw_phv_ctrl.start_enable(true);
    sw_phv_ctrl.counter_repeat_enable(0);
    sw_phv_ctrl.qid_repeat_enable(0);
    sw_phv_ctrl.localtime_enable(0);
    sw_phv_ctrl.frame_size_enable(0);
    sw_phv_ctrl.packet_len_enable(0);
    sw_phv_ctrl.qid_enable(0);
    sw_phv_ctrl.write();

    return SDK_RET_OK;
}

// elba_sw_phv_inject injects a software PHV into a pipeline
sdk_ret_t
elba_sw_phv_inject (asic_swphv_type_t type, uint8_t prof_num,
                    uint8_t start_idx, uint8_t num_flits, void *data)
{
    sdk_ret_t   ret = SDK_RET_OK;

    SDK_TRACE_DEBUG("Injecting Software PHV type {}", type);

    // switch based on pipeline type
    switch(type) {
    case ASIC_SWPHV_TYPE_RXDMA:
        ret = elba_pr_psp_sw_phv_inject(prof_num, start_idx, num_flits, data);
        break;
    case ASIC_SWPHV_TYPE_TXDMA:
        ret = elba_pt_psp_sw_phv_inject(prof_num, start_idx, num_flits, data);
        break;
    case ASIC_SWPHV_TYPE_INGRESS:
        ret = elba_ppa_sw_phv_inject(1, prof_num, start_idx, num_flits, data);
        break;
    case ASIC_SWPHV_TYPE_EGRESS:
        ret = elba_ppa_sw_phv_inject(0, prof_num, start_idx, num_flits, data);
        break;
    }

    return ret;
}

// elba_psp_sw_phv_state gets Rx/Tx DMA pipeline PHV state
static sdk_ret_t
elba_pr_psp_sw_phv_state (uint8_t prof_num, asic_sw_phv_state_t *state)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_psp_csr_t &pr_psp_csr = elb0.pr.pr.psp;

    SDK_TRACE_DEBUG("Getting Software PHV.");

    elb_psp_csr_sta_sw_phv_state_t &sw_phv_state =
        pr_psp_csr.sta_sw_phv_state[prof_num];
    sw_phv_state.read();
    elb_psp_csr_cfg_sw_phv_global_t &phv_global =
        pr_psp_csr.cfg_sw_phv_global;
    phv_global.read();

    elb_prd_csr_t &prd_csr = elb0.pr.pr.prd;
    elb_prd_csr_CNT_phv_t &prd_cnt = prd_csr.CNT_phv;
    prd_cnt.read();

    // set the status
    state->enabled = (bool)phv_global.start_enable();
    state->done = (bool)sw_phv_state.done();
    state->current_cntr = (uint32_t)sw_phv_state.current_counter();
    state->no_data_cntr = (uint32_t)prd_cnt.no_data();
    state->drop_no_data_cntr = (uint32_t)prd_cnt.drop();

    return SDK_RET_OK;
}

// elba_psp_sw_phv_state gets Rx/Tx DMA pipeline PHV state
static sdk_ret_t
elba_pt_psp_sw_phv_state (uint8_t prof_num, asic_sw_phv_state_t *state)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_psp_csr_t &pt_psp_csr = elb0.pt.pt.psp;

    SDK_TRACE_DEBUG("Getting Software PHV state.");

    // read the status registers
    elb_psp_csr_sta_sw_phv_state_t &sw_phv_state =
        pt_psp_csr.sta_sw_phv_state[prof_num];
    sw_phv_state.read();
    elb_psp_csr_cfg_sw_phv_global_t &phv_global =
        pt_psp_csr.cfg_sw_phv_global;
    phv_global.read();

    elb_ptd_csr_t &ptd_csr = elb0.pt.pt.ptd;
    elb_ptd_csr_CNT_phv_t &ptd_cnt = ptd_csr.CNT_phv;
    ptd_cnt.read();

    // set the status
    state->done = (bool)sw_phv_state.done();
    state->current_cntr = (uint32_t)sw_phv_state.current_counter();
    state->enabled = (bool)phv_global.start_enable();
    state->no_data_cntr = (uint32_t)ptd_cnt.no_data();
    state->drop_no_data_cntr = (uint32_t)ptd_cnt.drop();

    return SDK_RET_OK;
}

// elba_ppa_sw_phv_state gets P4 Ingress/Egress pipeline PHV state
static sdk_ret_t
elba_ppa_sw_phv_state (uint8_t pidx, uint8_t prof_num,
                       asic_sw_phv_state_t *state)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    elb_ppa_csr_t &ppa_csr = elb0.ppa.ppa[pidx];

    SDK_TRACE_DEBUG("Getting Software PHV state");

    // read the status registers
    elb_ppa_csr_sta_sw_phv_state_t &sw_phv_state =
        ppa_csr.sta_sw_phv_state[prof_num];
    sw_phv_state.read();
    elb_ppa_csr_cfg_sw_phv_global_t &phv_global = ppa_csr.cfg_sw_phv_global;
    phv_global.read();

    elb_dpr_csr_t &dpr_csr = elb0.dpr.dpr[pidx];
    elb_dprstats_csr_t &dpr_stats = dpr_csr.stats;
    dpr_stats.CNT_dpr_phv_no_data.read();
    dpr_stats.CNT_dpr_phv_drop_no_data.read();
    dpr_stats.CNT_dpr_phv_drop_no_data_drop.read();

    // set the state
    state->enabled = (bool)phv_global.start_enable();
    state->done = (bool)sw_phv_state.done();
    state->current_cntr = (uint32_t)sw_phv_state.current_counter();
    state->no_data_cntr = (uint32_t)dpr_stats.CNT_dpr_phv_no_data.all();
    state->drop_no_data_cntr =
        (uint32_t)dpr_stats.CNT_dpr_phv_drop_no_data.all();

    return SDK_RET_OK;
}

// elba_sw_phv_get gets the current state of the PHV
sdk_ret_t
elba_sw_phv_get (asic_swphv_type_t type, uint8_t prof_num,
                 asic_sw_phv_state_t *state)
{
    sdk_ret_t   ret = SDK_RET_OK;

    SDK_TRACE_DEBUG("Getting Software PHV state for type {}", type);

    // switch based on pipeline type
    switch(type) {
    case ASIC_SWPHV_TYPE_RXDMA:
        ret = elba_pr_psp_sw_phv_state(prof_num, state);
        break;
    case ASIC_SWPHV_TYPE_TXDMA:
        ret = elba_pt_psp_sw_phv_state(prof_num, state);
        break;
    case ASIC_SWPHV_TYPE_INGRESS:
        ret = elba_ppa_sw_phv_state(1, prof_num, state);
        break;
    case ASIC_SWPHV_TYPE_EGRESS:
        ret = elba_ppa_sw_phv_state(0, prof_num, state);
        break;
    }
    return ret;
}

}    // namespace elba
}    // namespace platform
}    // namespace sdk
