//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __API_UPGRADE_HPP__
#define __API_UPGRADE_HPP__

#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/apollo/api/include/pds_upgrade.hpp"
#include "nic/apollo/core/event.hpp"

namespace api {

sdk_ret_t upg_init(pds_init_params_t *params);
sdk_ret_t upg_graceful_init(pds_init_params_t *params);
sdk_ret_t upg_hitless_init(pds_init_params_t *params);
void upg_event_response_cb(sdk_ret_t ret, event_id_t ev_id);

}    // namespace api
#endif    // __API_UPGRADE_HPP__
