/*
 *  {C} Copyright 2019 Pensando Systems Inc. All rights reserved.
 */

#include <stdint.h>
#include <cstddef>
#include <cstring>
#include <nic/sdk/include/sdk/table.hpp>
#include <nic/sdk/asic/pd/pd.hpp>
#include <nic/apollo/api/include/pds_init.hpp>
#include <pd_utils.h>
#include <nat.h>

using namespace sdk;
using namespace sdk::table;
using namespace sdk::platform;

extern "C" {

static int
pds_init_logger (sdk_trace_level_e tracel_level, const char *format, ...)
{
    return 0;
}

int
initialize_pds(void)
{
    sdk_ret_t ret;
    pds_init_params_t init_params;
    std::string pipeline = PDS_PIPELINE;
#ifdef __aarch64__
    std::string cfg_file = "hal_hw.json";
#else
    std::string cfg_file = "hal.json";
#endif

    // do soft init
    sdk::asic::asic_set_init_type(sdk::asic::asic_init_type_t::ASIC_INIT_TYPE_SOFT);
    memset(&init_params, 0, sizeof(init_params));
    init_params.init_mode = PDS_INIT_MODE_COLD_START;
    init_params.trace_cb  = pds_init_logger;
    init_params.pipeline  = pipeline;
    init_params.cfg_file  = cfg_file;
    init_params.scale_profile = PDS_SCALE_PROFILE_DEFAULT;
    ret = pds_init(&init_params);
    SDK_ASSERT(ret == SDK_RET_OK);

    // NAT hw init
    pds_nat_init();

    return ret;
}
}

