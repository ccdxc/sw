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

using namespace sdk;

extern "C" {
// Function prototypes
sdk_ret_t pds_flow_cache_create(void);
void pds_flow_cache_delete(void);
void pds_flow_cache_set_core_id(uint32_t core_id);
sdk_ret_t pds_dnat_map_create(void);
sdk_ret_t pds_dnat_map_delete(void);
void pds_dnat_map_set_core_id(uint32_t core_id);

sdk_ret_t
pds_global_init (pds_init_params_t *params)
{
    sdk_ret_t ret;

    if (params == NULL) {
        PDS_TRACE_ERR("params arg is null");
        return SDK_RET_INVALID_ARG;
    }

    ret = pds_init(params);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("PDS init failed with ret %u\n", ret);
        return ret;
    }
    ret = pds_flow_cache_create();
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Flow cache init failed with ret %u\n", ret);
        return ret;
    }
    ret = pds_dnat_map_create();
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("DNAT map init failed with ret %u\n", ret);
        return ret;
    }
    return ret;
}

sdk_ret_t
pds_thread_init (uint32_t core_id)
{
    pds_flow_cache_set_core_id(core_id);
    pds_dnat_map_set_core_id(core_id);
    return SDK_RET_OK;
}

void
pds_global_teardown ()
{
    pds_dnat_map_delete();
    pds_flow_cache_delete();
    pds_teardown();
    return;
}

}

