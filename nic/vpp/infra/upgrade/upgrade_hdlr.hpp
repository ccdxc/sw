//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//

#ifndef __VPP_INFRA_IPC_UPGRADE_HDLR_HPP__
#define __VPP_INFRA_IPC_UPGRADE_HDLR_HPP__

#include <stdint.h>
#include <string.h>

#include <nic/sdk/include/sdk/base.hpp>
#include <nic/sdk/upgrade/include/ev.hpp>

sdk_ret_t vpp_upg_ev_hdlr(sdk::upg::upg_ev_params_t *params);
void vpp_upg_init();

#endif /* __VPP_INFRA_IPC_UPGRADE_HDLR_HPP__ */

