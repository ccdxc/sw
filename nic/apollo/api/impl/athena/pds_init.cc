//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// athena global and per thread routines
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/apollo/api/include/athena/pds_init.h"
#include "nic/apollo/api/include/athena/pds_flow_cache.h"
#include "nic/apollo/api/include/athena/pds_flow_age.h"
#include "nic/sdk/asic/asic.hpp"

using namespace sdk;
using namespace sdk::asic;

extern "C" {
// Function prototypes
sdk_ret_t pds_flow_cache_create(void);
void pds_flow_cache_delete(void);
void pds_flow_cache_set_core_id(uint32_t core_id);
sdk_ret_t pds_dnat_map_create(void);
sdk_ret_t pds_dnat_map_delete(void);
void pds_dnat_map_set_core_id(uint32_t core_id);

pds_ret_t
pds_global_init (pds_cinit_params_t *params)
{
    sdk_ret_t           ret;
    pds_init_params_t   params_cpp;
    asic_init_type_t    asic_init_type = ASIC_INIT_TYPE_HARD;


    if (params == NULL) {
        PDS_TRACE_ERR("params arg is null");
        return PDS_RET_INVALID_ARG;
    }

    memset(&params_cpp, 0, sizeof(params_cpp));
    params_cpp.init_mode = (pds_init_mode_t) params->init_mode;
    params_cpp.trace_cb = (sdk::lib::logger::trace_cb_t) params->trace_cb;
    params_cpp.pipeline = "athena";
    //params_cpp.scale_profile = PDS_SCALE_PROFILE_DEFAULT;
    params_cpp.memory_profile = PDS_MEMORY_PROFILE_DEFAULT;
#ifdef __aarch64__
    params_cpp.cfg_file = "hal_hw.json";
#else
     params_cpp.cfg_file = "hal.json";
#endif
   
    if (params->flags & PDS_FLAG_INIT_TYPE_SOFT) {
        asic_init_type = ASIC_INIT_TYPE_SOFT;
    } 

    sdk::asic::asic_set_init_type(asic_init_type);

    ret = pds_init(&params_cpp);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("PDS init failed with ret %u\n", ret);
        return (pds_ret_t)ret;
    }
    ret = pds_flow_cache_create();
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Flow cache init failed with ret %u\n", ret);
        return (pds_ret_t)ret;
    }
    ret = pds_dnat_map_create();
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("DNAT map init failed with ret %u\n", ret);
        return (pds_ret_t)ret;
    }
    ret = (sdk_ret_t)pds_flow_age_init();
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Flow aging init failed with ret %u\n", ret);
    }
    return (pds_ret_t)ret;
}

pds_ret_t
pds_thread_init (uint32_t core_id)
{
    pds_flow_cache_set_core_id(core_id);
    pds_dnat_map_set_core_id(core_id);
    return PDS_RET_OK;
}

void
pds_global_teardown ()
{
    pds_flow_age_fini();
    pds_dnat_map_delete();
    pds_flow_cache_delete();
    pds_teardown();
    return;
}

}

