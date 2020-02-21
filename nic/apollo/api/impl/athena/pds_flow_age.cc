//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// athena hardware-assisted flow aging implementation
///
//----------------------------------------------------------------------------

#include "nic/apollo/api/include/athena/pds_flow_age.h"
#include "ftl_dev_impl.hpp"
#include "ftl_pollers_client.hpp"

extern "C" {

sdk_ret_t
pds_flow_age_init(void)
{
    return ftl_pollers_client::init();
}

sdk_ret_t
pds_flow_age_hw_scanners_start(void)
{
    return ftl_dev_impl::scanners_start();
}

sdk_ret_t
pds_flow_age_hw_scanners_stop(bool quiesce_check)
{
    return ftl_dev_impl::scanners_stop(quiesce_check);
}

sdk_ret_t
pds_flow_age_sw_pollers_flush(void)
{
    return ftl_dev_impl::pollers_flush();
}

sdk_ret_t
pds_flow_age_sw_pollers_qcount(uint32_t *ret_qcount)
{
    return ftl_dev_impl::pollers_qcount_get(ret_qcount);
}

sdk_ret_t
pds_flow_age_sw_pollers_expiry_fn_dflt(pds_flow_expiry_fn_t *ret_fn_dflt)
{
    return ftl_pollers_client::expiry_fn_dflt(ret_fn_dflt);
}

sdk_ret_t
pds_flow_age_sw_pollers_poll_control(bool user_will_poll,
                                     pds_flow_expiry_fn_t expiry_fn)
{
    return ftl_pollers_client::poll_control(user_will_poll, expiry_fn);
}

sdk_ret_t
pds_flow_age_sw_pollers_poll(uint32_t poller_id,
                             void *user_ctx)
{
    return ftl_pollers_client::poll(poller_id, user_ctx);
}

sdk_ret_t
pds_flow_age_normal_timeouts_set(const pds_flow_age_timeouts_t *norm_age_timeouts)
{
    return ftl_dev_impl::normal_timeouts_set(norm_age_timeouts);
}

sdk_ret_t
pds_flow_age_accel_timeouts_set(const pds_flow_age_timeouts_t *accel_age_timeouts)
{
    return ftl_dev_impl::accel_timeouts_set(accel_age_timeouts);
}

sdk_ret_t
pds_flow_age_accel_control(bool enable_sense)
{
    return ftl_dev_impl::accel_aging_control(enable_sense);
}

}

