//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/src/debug/table.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/include/pd_api.hpp"

using sdk::lib::slab;

namespace hal {

//------------------------------------------------------------------------------
// process a table metadata get
//------------------------------------------------------------------------------
hal_ret_t
table_metadata_get (table::TableMetadataResponseMsg *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;
    hal::pd::pd_table_metadata_get_args_t args;
    pd::pd_func_args_t pd_func_args = {0};

    // Optimistically set the api status to OK. If failure is seen in the guts,
    // then respective function will set the correct api status.
    rsp->set_api_status(types::API_STATUS_OK);

    args.rsp = rsp;

    pd_func_args.pd_table_metadata_get = &args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_TABLE_METADATA_GET,
                               &pd_func_args);
    return ret;
}

//------------------------------------------------------------------------------
// process a table get
//------------------------------------------------------------------------------
hal_ret_t
table_get (TableSpec& spec, TableResponseMsg *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;
    hal::pd::pd_table_get_args_t args;
    pd::pd_func_args_t pd_func_args = {0};

    args.spec = &spec;
    args.rsp = rsp;

    pd_func_args.pd_table_get = &args;
    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_TABLE_GET, &pd_func_args);

    return ret;
}


}    // namespace hal
