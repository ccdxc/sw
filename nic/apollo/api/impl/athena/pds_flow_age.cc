//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// athena flow aging implementation
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/athena/pds_flow_age.h"
#include "nic/apollo/fte/impl/athena/fte_ftl_dev.hpp"

extern "C" {

sdk_ret_t
pds_flow_age_hw_scanners_start(void)
{
    return fte_ftl_dev::scanners_start();
}

sdk_ret_t
pds_flow_age_hw_scanners_stop(bool quiesce_check)
{
    return fte_ftl_dev::scanners_stop(quiesce_check);
}

sdk_ret_t
pds_flow_age_sw_pollers_flush(void)
{
    return fte_ftl_dev::pollers_flush();
}

sdk_ret_t
pds_flow_age_normal_timeouts_set(const pds_flow_age_timeouts_t *norm_age_timeouts)
{
    return fte_ftl_dev::normal_timeouts_set(norm_age_timeouts);
}

sdk_ret_t
pds_flow_age_accel_timeouts_set(const pds_flow_age_timeouts_t *accel_age_timeouts)
{
    return fte_ftl_dev::accel_timeouts_set(accel_age_timeouts);
}

sdk_ret_t
pds_flow_age_accel_control(bool enable_sense)
{
    return fte_ftl_dev::accel_aging_control(enable_sense);
}

sdk_ret_t
pds_flow_age_expiry_user_callback_set(void (*user_cb)(
                                            uint32_t expiry_id,
                                            pds_flow_age_expiry_type_t expiry_type,
                                            const pds_flow_age_user_directives_t *user_directives))
{
    return SDK_RET_OK;
}

}

