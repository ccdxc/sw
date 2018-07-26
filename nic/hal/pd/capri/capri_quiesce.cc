#include "nic/hal/pd/pd_api.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/asic/capri/model/cap_top/cap_top_csr.h"
#include "nic/asic/capri/verif/apis/cap_quiesce_api.h"

namespace hal {
namespace pd {

#define MAX_PORT10_FLOW_CTRL_ENTRIES    32
#define MAX_PORT11_FLOW_CTRL_ENTRIES    32
uint32_t    port_10_ref_credits[MAX_PORT10_FLOW_CTRL_ENTRIES] =
{0xb4, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0xb4, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
uint32_t    port_11_ref_credits[MAX_PORT11_FLOW_CTRL_ENTRIES] =
{0xb4, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0xb4, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};


hal_ret_t
capri_quiesce_start(void) {
    hal_ret_t           ret = HAL_RET_OK;
    int                 chip_id = 0;

    HAL_TRACE_DEBUG("{}: Start", __FUNCTION__);

	cap_top_quiesce_pb_start(chip_id);
	cap_top_quiesce_txs_start(chip_id);
	cap_top_quiesce_sw_phv_insert(chip_id);

	cap_top_quiesce_txs_poll(chip_id, 100);
	cap_top_quiesce_p4p_tx_poll(chip_id, 100);
	cap_top_quiesce_pb_poll(chip_id, port_10_ref_credits, port_11_ref_credits, 100);
	cap_top_quiesce_psp_poll(chip_id, 100);
	cap_top_quiesce_p4p_prd_poll(chip_id, 100);

    HAL_TRACE_DEBUG("{}: End", __FUNCTION__);
    return ret;
}


hal_ret_t
capri_quiesce_stop(void) {
    hal_ret_t           ret = HAL_RET_OK;
    int                 chip_id = 0;

    HAL_TRACE_DEBUG("{}: Start", __FUNCTION__);

	cap_top_quiesce_pb_stop(chip_id);
	cap_top_quiesce_txs_stop(chip_id);

    HAL_TRACE_DEBUG("{}: End", __FUNCTION__);
    return ret;
}


hal_ret_t
capri_quiesce_init(void) {
    hal_ret_t           ret = HAL_RET_OK;
	cap_top_csr_t       &top_csr= CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

    HAL_TRACE_DEBUG("{}: Start", __FUNCTION__);
    HAL_TRACE_DEBUG("{}: Port10: {:x}", __FUNCTION__, top_csr.pb.pbc.port_10.dhs_oq_flow_control.get_depth_entry());
    HAL_TRACE_DEBUG("{}: Port11: {:x}", __FUNCTION__, top_csr.pb.pbc.port_11.dhs_oq_flow_control.get_depth_entry());
    HAL_ASSERT(top_csr.pb.pbc.port_10.dhs_oq_flow_control.get_depth_entry() == MAX_PORT10_FLOW_CTRL_ENTRIES);
    HAL_ASSERT(top_csr.pb.pbc.port_11.dhs_oq_flow_control.get_depth_entry() == MAX_PORT11_FLOW_CTRL_ENTRIES);

	for (int i=0; i < top_csr.pb.pbc.port_10.dhs_oq_flow_control.get_depth_entry(); i++) {
        top_csr.pb.pbc.port_10.dhs_oq_flow_control.entry[i].read();
		port_10_ref_credits[i] = top_csr.pb.pbc.port_10.dhs_oq_flow_control.entry[i].entry().convert_to<uint32_t>();
        HAL_TRACE_DEBUG("{}: Port 10[{}] : {:x}", __FUNCTION__, i, port_10_ref_credits[i]);
	}

	for (int i=0; i < top_csr.pb.pbc.port_11.dhs_oq_flow_control.get_depth_entry(); i++) {
        top_csr.pb.pbc.port_11.dhs_oq_flow_control.entry[i].read();
		port_11_ref_credits[i] = top_csr.pb.pbc.port_11.dhs_oq_flow_control.entry[i].entry().convert_to<uint32_t>();
        HAL_TRACE_DEBUG("{}: Port 11[{}] : {:x}", __FUNCTION__, i, port_11_ref_credits[i]);
	}
    HAL_TRACE_DEBUG("{}: End", __FUNCTION__);

    return ret;
}


}    // namespace pd
}    // namespace hal
