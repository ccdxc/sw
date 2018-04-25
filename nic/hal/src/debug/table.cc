//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/hal/src/debug/table.hpp"
#include "nic/include/hal_state.hpp"
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

    args.rsp = rsp;

    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_TABLE_METADATA_GET,
                               (void *)&args);
    rsp->set_api_status(types::API_STATUS_OK);

    return ret;
}

//------------------------------------------------------------------------------
// process a table get
//------------------------------------------------------------------------------
hal_ret_t
table_get (TableSpec& spec, TableResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;
    hal::pd::pd_table_get_args_t args;

    args.spec = &spec;
    args.rsp = rsp;

    ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_TABLE_GET, (void *)&args);
    rsp->set_api_status(types::API_STATUS_OK);


    return ret;
}


}    // namespace hal
