// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __LINKMGR_TEST_PORT_UTILS_HPP__
#define __LINKMGR_TEST_PORT_UTILS_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/types.hpp"

int port_create(uint32_t port_id);

int port_update(uint32_t            port_id,
                hal_ret_t           ret_exp,
                port_speed_t        speed,
                port_admin_state_t  admin_state,
                port_fec_type_t     fec_type,
                uint32_t            debounce_time,
                bool                auto_neg_enable,
                uint32_t            num_lanes);

int port_get(uint32_t             port_id,
             hal_ret_t            ret_exp,
             bool                 compare          = false,
             port_oper_status_t   port_oper_status =
                                    port_oper_status_t::PORT_OPER_STATUS_NONE,
             port_type_t          port_type        =
                                    port_type_t::PORT_TYPE_NONE,
             port_speed_t         port_speed       =
                                    port_speed_t::PORT_SPEED_NONE,
             port_admin_state_t   port_admin_state =
                                    port_admin_state_t::PORT_ADMIN_STATE_NONE,
             port_fec_type_t      fec_type         =
                                    port_fec_type_t::PORT_FEC_TYPE_NONE,
             uint32_t             debounce_time    = 100,
             bool                 auto_neg_enable  = false);

int port_delete(uint32_t port_id);
#endif // __LINKMGR_TEST_PORT_UTILS_HPP__
