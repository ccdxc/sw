//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements pds upgrade
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_upgrade.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/upgrade_state.hpp"

sdk_ret_t
pds_upgrade (pds_upg_spec_t *spec)
{
    sdk_ret_t ret;
    upg::upg_state *ustate;

    if (!spec) {
        return SDK_RET_INVALID_ARG;
    }
    ustate = upg::upg_state::get_instance();
    if (!ustate) {
        ret = SDK_RET_ERR;
        goto err_exit;
    }

    switch(spec->stage) {
    case UPG_STAGE_START:
        // stop grpc as no more config commands are expected
        // TODO
        break;
    case UPG_STAGE_PREPARE:
        break;
    default:
        // nothing to do
        break;
    }
    return SDK_RET_OK;

err_exit:

    return ret;
}
