//// {C} Copyright 2020 Pensando Systems Inc. All rights reserved


#include "platform/elba/elba_quiesce.hpp"
#include "third-party/asic/elba/model/elb_top/elb_top_csr.h"
#include "third-party/asic/elba/verif/apis/elb_quiesce_api.h"

namespace sdk {
namespace platform {
namespace elba {

#define MAX_PORT10_FLOW_CTRL_ENTRIES    32
#define MAX_PORT11_FLOW_CTRL_ENTRIES    32
uint32_t    port_6_ref_credits[MAX_PORT10_FLOW_CTRL_ENTRIES] =
            {0xb4, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0xb4, 0x5a, 0x5a,
             0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
             0x5a, 0x5a, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

uint32_t    port_7_ref_credits[MAX_PORT11_FLOW_CTRL_ENTRIES] =
            {0xb4, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0xb4, 0x5a, 0x5a,
             0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a, 0x5a,
             0x5a, 0x5a, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

sdk_ret_t
elba_quiesce_start (void)
{
    return SDK_RET_OK;
}

sdk_ret_t
elba_quiesce_stop (void)
{
    return SDK_RET_OK;
}

sdk_ret_t
elba_quiesce_init (void)
{
    elb_top_csr_t &top_csr= ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);

    SDK_ASSERT(top_csr.pb.pbc.port_6.dhs_oq_flow_control.get_depth_entry() ==
               MAX_PORT10_FLOW_CTRL_ENTRIES);
    SDK_ASSERT(top_csr.pb.pbc.port_7.dhs_oq_flow_control.get_depth_entry() ==
               MAX_PORT11_FLOW_CTRL_ENTRIES);

    for (int i = 0;
         i < top_csr.pb.pbc.port_6.dhs_oq_flow_control.get_depth_entry(); i++) {
        top_csr.pb.pbc.port_6.dhs_oq_flow_control.entry[i].read();
        port_6_ref_credits[i] =
            top_csr.pb.pbc.port_6.dhs_oq_flow_control.entry[i].entry().convert_to<uint32_t>();
    }

    for (int i = 0;
         i < top_csr.pb.pbc.port_7.dhs_oq_flow_control.get_depth_entry(); i++) {
        top_csr.pb.pbc.port_7.dhs_oq_flow_control.entry[i].read();
        port_7_ref_credits[i] =
            top_csr.pb.pbc.port_7.dhs_oq_flow_control.entry[i].entry().convert_to<uint32_t>();
    }

    return SDK_RET_OK;
}

}    // namespace elba
}    // namespace platform
}    // namespace sdk
