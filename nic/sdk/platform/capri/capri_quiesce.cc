// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "asic/asic.hpp"
#include "platform/capri/capri_quiesce.hpp"
#include "platform/capri/capri_state.hpp"
#include "third-party/asic/capri/model/cap_top/cap_top_csr.h"
#include "third-party/asic/capri/verif/apis/cap_quiesce_api.h"
#include "third-party/asic/capri/model/cap_ptd/cap_ptd_csr.h"
#include "third-party/asic/capri/model/cap_psp/cap_psp_csr.h"
#include "third-party/asic/capri/verif/apis/cap_ptd_api.h"

namespace sdk {
namespace platform {
namespace capri {

#define MAX_PORT10_FLOW_CTRL_ENTRIES    32
#define MAX_PORT11_FLOW_CTRL_ENTRIES    32
uint32_t port_10_ref_credits[MAX_PORT10_FLOW_CTRL_ENTRIES];
uint32_t port_11_ref_credits[MAX_PORT11_FLOW_CTRL_ENTRIES];

static inline void
config_dump (void)
{
    cap_top_csr_t &top_csr= g_capri_state_pd->cap_top();
    cap_pbc_csr_t &pbc_csr = top_csr.pb.pbc;
    cap_ptd_csr_t &ptd_csr = top_csr.pt.pt.ptd;
    cap_psp_csr_t &psp_csr = top_csr.pt.pt.psp;
    stringstream data;
    uint32_t tmp;

    data << hex << endl;

    for (int i = 0;
         i < top_csr.pb.pbc.port_11.dhs_oq_flow_control.get_depth_entry();
         i++) {
        top_csr.pb.pbc.port_11.dhs_oq_flow_control.entry[i].read();
        tmp = top_csr.pb.pbc.port_11.dhs_oq_flow_control.entry[i].entry().convert_to<uint32_t>();
        SDK_TRACE_DEBUG("Port 11[%d] : 0x%x", i, tmp);
    }

    pbc_csr.port_11.cfg_account_credit_return.read();
    pbc_csr.port_11.cfg_oq_queue.read();
    pbc_csr.cfg_credits_max_growth_11.read();
    pbc_csr.port_11.cfg_oq.read();

    data <<"pbc_csr.port_11.cfg_account_credit_return.all: 0x" <<
        pbc_csr.port_11.cfg_account_credit_return.all() << endl;
    data <<"pbc_csr.port_11.cfg_account_credit_return.enable: 0x" <<
        pbc_csr.port_11.cfg_account_credit_return.enable() << endl;
    data <<"pbc_csr.port_11.cfg_oq_queue.all: 0x" <<
        pbc_csr.port_11.cfg_oq_queue.all() << endl;
    data <<"pbc_csr.port_11.cfg_oq_queue.recirc: 0x" <<
        pbc_csr.port_11.cfg_oq_queue.recirc() << endl;
    data <<"pbc_csr.port_11.cfg_oq_queue.enable: 0x" <<
        pbc_csr.port_11.cfg_oq_queue.enable() << endl;
    data <<"pbc_csr.port_11.cfg_oq_queue.flush: 0x" <<
        pbc_csr.port_11.cfg_oq_queue.flush() << endl;
    data <<"pbc_csr.cfg_credits_max_growth_11.all: 0x" <<
        pbc_csr.cfg_credits_max_growth_11.all() << endl;
    data <<"pbc_csr.cfg_credits_max_growth_11.cells: 0x" <<
        pbc_csr.cfg_credits_max_growth_11.cells() << endl;

    SDK_TRACE_DEBUG("%s", data.str().c_str());
    data.str("");
    data << hex << endl;

    data <<"pbc_csr.port_11.cfg_oq.all: 0x" << pbc_csr.port_11.cfg_oq.all() << endl;
    data <<"pbc_csr.port_11.cfg_oq.enable: 0x" << pbc_csr.port_11.cfg_oq.enable() << endl;
    data <<"pbc_csr.port_11.cfg_oq.ecc_disable_cor: 0x" << pbc_csr.port_11.cfg_oq.ecc_disable_cor() << endl;
    data <<"pbc_csr.port_11.cfg_oq.packing_enable: 0x" << pbc_csr.port_11.cfg_oq.packing_enable() << endl;
    data <<"pbc_csr.port_11.cfg_oq.dhs_eccbypass: 0x" << pbc_csr.port_11.cfg_oq.dhs_eccbypass() << endl;
    data <<"pbc_csr.port_11.cfg_oq.ipg_bytes: 0x" << pbc_csr.port_11.cfg_oq.ipg_bytes() << endl;
    data <<"pbc_csr.port_11.cfg_oq.bist_run: 0x" << pbc_csr.port_11.cfg_oq.bist_run() << endl;
    data <<"pbc_csr.port_11.cfg_oq.flow_control_enable_xoff: 0x" << pbc_csr.port_11.cfg_oq.flow_control_enable_xoff() << endl;
    data <<"pbc_csr.port_11.cfg_oq.ecc_disable_det: 0x" << pbc_csr.port_11.cfg_oq.ecc_disable_det() << endl;
    data <<"pbc_csr.port_11.cfg_oq.packing_msb: 0x" << pbc_csr.port_11.cfg_oq.packing_msb() << endl;
    data <<"pbc_csr.port_11.cfg_oq.flush: 0x" << pbc_csr.port_11.cfg_oq.flush() << endl;
    data <<"pbc_csr.port_11.cfg_oq.flow_control_enable_credits: 0x" << pbc_csr.port_11.cfg_oq.flow_control_enable_credits() << endl;
    data <<"pbc_csr.port_11.cfg_oq.rewrite_enable: 0x" << pbc_csr.port_11.cfg_oq.rewrite_enable() << endl;
    data <<"pbc_csr.port_11.cfg_oq.dhs_selection: 0x" << pbc_csr.port_11.cfg_oq.dhs_selection() << endl;

    SDK_TRACE_DEBUG("%s", data.str().c_str());

    // pt_ptd_sta_fifo
    ptd_csr.sta_fifo.read();
    SDK_TRACE_DEBUG("STA fifo: ");
    SDK_TRACE_DEBUG("lat_ff_full: %d, lat_ff_empty: %d, wdata_ff_full: %d, wdata_ff_empty: %d, \n"
                    "rcv_stg_ff_full: %d, rcv_stg_ff_empty: %d, cmdflit_ff_full: %d, cmdflit_ff_empty: %d, \n"
                    "cmd_ff_full: %d, cmd_ff_empty: %d, pkt_ff_full: %d, pkt_ff_empty: %d, \n"
                    "wr_mem_ff_full: %d, wr_mem_ff_empty: %d, dfence_ff_full: %d, dfence_ff_empty: %d, \n"
                    "ffence_ff_full: %d, ffence_ff_empty: %d, phv_lpbk_in_drdy: %d, phv_lpbk_out_srdy: %d, \n"
                    "ma_srdy: %d, ma_drdy: %d, axi_wr_valid: %d, axi_wr_ready: %d, axi_rd_valid: %d, axi_rd_ready: %d \n",
                    ptd_csr.sta_fifo.lat_ff_full().convert_to<int>(),
                    ptd_csr.sta_fifo.lat_ff_empty().convert_to<int>(),
                    ptd_csr.sta_fifo.wdata_ff_full().convert_to<int>(),
                    ptd_csr.sta_fifo.wdata_ff_empty().convert_to<int>(),
                    ptd_csr.sta_fifo.rcv_stg_ff_full().convert_to<int>(),
                    ptd_csr.sta_fifo.rcv_stg_ff_empty().convert_to<int>(),
                    ptd_csr.sta_fifo.cmdflit_ff_full().convert_to<int>(),
                    ptd_csr.sta_fifo.cmdflit_ff_empty().convert_to<int>(),
                    ptd_csr.sta_fifo.cmd_ff_full().convert_to<int>(),
                    ptd_csr.sta_fifo.cmd_ff_empty().convert_to<int>(),
                    ptd_csr.sta_fifo.pkt_ff_full().convert_to<int>(),
                    ptd_csr.sta_fifo.pkt_ff_empty().convert_to<int>(),
                    ptd_csr.sta_fifo.wr_mem_ff_full().convert_to<int>(),
                    ptd_csr.sta_fifo.wr_mem_ff_empty().convert_to<int>(),
                    ptd_csr.sta_fifo.dfence_ff_full().convert_to<int>(),
                    ptd_csr.sta_fifo.dfence_ff_empty().convert_to<int>(),
                    ptd_csr.sta_fifo.ffence_ff_full().convert_to<int>(),
                    ptd_csr.sta_fifo.ffence_ff_empty().convert_to<int>(),
                    ptd_csr.sta_fifo.phv_lpbk_in_drdy().convert_to<int>(),
                    ptd_csr.sta_fifo.phv_lpbk_out_srdy().convert_to<int>(),
                    ptd_csr.sta_fifo.ma_srdy().convert_to<int>(),
                    ptd_csr.sta_fifo.ma_drdy().convert_to<int>(),
                    ptd_csr.sta_fifo.axi_wr_valid().convert_to<int>(),
                    ptd_csr.sta_fifo.axi_wr_ready().convert_to<int>(),
                    ptd_csr.sta_fifo.axi_rd_valid().convert_to<int>(),
                    ptd_csr.sta_fifo.axi_rd_ready().convert_to<int>());

    // pt_ptd_sta_id
    ptd_csr.sta_id.read();
    SDK_TRACE_DEBUG("STA Id: ");
    SDK_TRACE_DEBUG("wr_pend_cnt: %d, rd_pend_cnt: %d, rd_pend_rsrc_cnt: %d",
                    ptd_csr.sta_id.wr_pend_cnt().convert_to<int>(),
                    ptd_csr.sta_id.rd_pend_cnt().convert_to<int>(),
                    ptd_csr.sta_id.rd_pend_rsrc_cnt().convert_to<int>());
    // pt_ptd_sta_xoff
    ptd_csr.sta_xoff.read();
    SDK_TRACE_DEBUG("STA xoff: ");
    SDK_TRACE_DEBUG("numphv_counter: %d, numphv_xoff: %d",
                    ptd_csr.sta_xoff.numphv_counter().convert_to<int>(),
                    ptd_csr.sta_xoff.numphv_xoff().convert_to<int>());
    // pt_psp_sta_flow
    // cap_psp_csr_t & psp0 = CAP_BLK_REG_MODEL_ACCESS(cap_psp_csr_t, 0, 0);
    psp_csr.sta_flow.read();
    SDK_TRACE_DEBUG("STA flow: ");
    SDK_TRACE_DEBUG("ma_drdy: %d, pb_pbus_drdy: %d, pkt_order_push_drdy: %d, pr_resub_drdy: %d, \n"
                    "pr_resub_pbus_drdy: %d, sv01_pb_pkt_drdy: %d, sv01_lb_phv_drdy: %d, sv02_lb_pkt_drdy: %d, \n"
                    "sv04_lt_ovr_drdy: %d, sv04_lt_rsp_drdy: %d, sv11_in_phv_drdy: %d, sv11_in_pkt_drdy: %d, \n"
                    "sv14_ma_phv_drdy: %d, sv15_pr_pkt_drdy: %d, sv20_ma_phv_drdy: %d, sv21_ma_phv_drdy: %d, \n"
                    "pr_pkt_ff_almost_full: %d, ptd_npv_phv_full: %d \n",
                    psp_csr.sta_flow.ma_drdy().convert_to<int>(),
                    psp_csr.sta_flow.pb_pbus_drdy().convert_to<int>(),
                    psp_csr.sta_flow.pkt_order_push_drdy().convert_to<int>(),
                    psp_csr.sta_flow.pr_resub_drdy().convert_to<int>(),
                    psp_csr.sta_flow.pr_resub_pbus_drdy().convert_to<int>(),
                    psp_csr.sta_flow.sv01_pb_pkt_drdy().convert_to<int>(),
                    psp_csr.sta_flow.sv01_lb_phv_drdy().convert_to<int>(),
                    psp_csr.sta_flow.sv02_lb_pkt_drdy().convert_to<int>(),
                    psp_csr.sta_flow.sv04_lt_ovr_drdy().convert_to<int>(),
                    psp_csr.sta_flow.sv04_lt_rsp_drdy().convert_to<int>(),
                    psp_csr.sta_flow.sv11_in_phv_drdy().convert_to<int>(),
                    psp_csr.sta_flow.sv11_in_pkt_drdy().convert_to<int>(),
                    psp_csr.sta_flow.sv14_ma_phv_drdy().convert_to<int>(),
                    psp_csr.sta_flow.sv15_pr_pkt_drdy().convert_to<int>(),
                    psp_csr.sta_flow.sv20_ma_phv_drdy().convert_to<int>(),
                    psp_csr.sta_flow.sv21_ma_phv_drdy().convert_to<int>(),
                    psp_csr.sta_flow.pr_pkt_ff_almost_full().convert_to<int>(),
                    psp_csr.sta_flow.ptd_npv_phv_full().convert_to<int>());
}

sdk_ret_t
capri_quiesce_start (void)
{
    sdk_ret_t           ret = SDK_RET_OK;
    int                 chip_id = 0;
    int                 ret_val = 0;

   // SDK_TRACE_DEBUG(" %s Start", __FUNCTION__);

    cap_top_quiesce_pb_start(chip_id);
    cap_top_quiesce_txs_start(chip_id);
    cap_top_quiesce_sw_phv_insert(chip_id);

    ret_val = cap_top_quiesce_txs_poll(chip_id, 1000);
    if (ret_val != 0) {
        SDK_TRACE_DEBUG("Failed txs poll");
        ret = SDK_RET_ERR;
    }
    ret_val = cap_top_quiesce_p4p_tx_poll(chip_id, 1000);
    if (ret_val != 0) {
        SDK_TRACE_DEBUG("Failed p4p tx poll");
        ret = SDK_RET_ERR;
    }
    ret_val = cap_top_quiesce_pb_poll(chip_id, port_10_ref_credits,
                                      port_11_ref_credits, 100);
    if (ret_val != 0) {
        SDK_TRACE_DEBUG("Failed pb poll");
        ret = SDK_RET_ERR;
    }
    ret_val = cap_top_quiesce_psp_poll(chip_id, 100);
    if (ret_val != 0) {
        SDK_TRACE_DEBUG("Failed psp poll");
        ret = SDK_RET_ERR;
    }
    ret_val = cap_top_quiesce_p4p_prd_poll(chip_id, 100);
    if (ret_val != 0) {
        SDK_TRACE_DEBUG("Failed prd poll");
        ret = SDK_RET_ERR;
    }

    // even if there an error, stop needs to be called.
    // so no need to check the ret_val
    sdk::asic::set_state(sdk::asic::asic_state_t::ASIC_STATE_QUIESCED);

   // SDK_TRACE_DEBUG(" %s End ret: %d", __FUNCTION__, ret_val);
    return ret;
}

sdk_ret_t
capri_quiesce_stop (void)
{
    sdk_ret_t           ret = SDK_RET_OK;
    int                 chip_id = 0;

    // SDK_TRACE_DEBUG(" %s Start", __FUNCTION__);

    cap_top_quiesce_pb_stop(chip_id);
    cap_top_quiesce_txs_stop(chip_id);

    sdk::asic::set_state(sdk::asic::asic_state_t::ASIC_STATE_RUNNING);

    // SDK_TRACE_DEBUG(" %s End", __FUNCTION__);
    return ret;
}


sdk_ret_t
capri_quiesce_init (void)
{
    sdk_ret_t           ret = SDK_RET_OK;
    cap_top_csr_t       &top_csr= g_capri_state_pd->cap_top();

    //SDK_TRACE_DEBUG("Port10: 0x%x", top_csr.pb.pbc.port_10.dhs_oq_flow_control.get_depth_entry());
    //SDK_TRACE_DEBUG("Port11: 0x%x", top_csr.pb.pbc.port_11.dhs_oq_flow_control.get_depth_entry());
    SDK_ASSERT(top_csr.pb.pbc.port_10.dhs_oq_flow_control.get_depth_entry() ==
               MAX_PORT10_FLOW_CTRL_ENTRIES);
    SDK_ASSERT(top_csr.pb.pbc.port_11.dhs_oq_flow_control.get_depth_entry() ==
               MAX_PORT11_FLOW_CTRL_ENTRIES);

    for (int i = 0;
         i < top_csr.pb.pbc.port_10.dhs_oq_flow_control.get_depth_entry();
         i++) {
        top_csr.pb.pbc.port_10.dhs_oq_flow_control.entry[i].read();
        port_10_ref_credits[i] =
            top_csr.pb.pbc.port_10.dhs_oq_flow_control.entry[i].entry().convert_to<uint32_t>();
        //SDK_TRACE_DEBUG("Port 10[%d] : 0x%x", i, port_10_ref_credits[i]);
    }

    for (int i = 0;
         i < top_csr.pb.pbc.port_11.dhs_oq_flow_control.get_depth_entry();
         i++) {
        top_csr.pb.pbc.port_11.dhs_oq_flow_control.entry[i].read();
        port_11_ref_credits[i] =
            top_csr.pb.pbc.port_11.dhs_oq_flow_control.entry[i].entry().convert_to<uint32_t>();
        //SDK_TRACE_DEBUG("Port 11[%d] : 0x%x", i, port_11_ref_credits[i]);
    }

    //Buffer configs
    top_csr.pb.pbc.port_11.cfg_oq.read();
    top_csr.txs.txs.cfg_sch.read();

    return ret;
}

} // namespace capri
} // namespace platform
} // namespace sdk    // namespace hal
