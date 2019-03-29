//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#include "nic/apollo/api/include/pds_debug.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/svc/util.hpp"
#include "nic/apollo/agent/svc/debug.hpp"
#include "nic/apollo/agent/trace.hpp"

Status
DebugSvcImpl::ClockFrequencyUpdate(ServerContext *context,
                                   const pds::ClockFrequencyRequest *proto_req,
                                   pds::ClockFrequencyResponse *proto_rsp) {
    sdk_ret_t ret;
    uint32_t freq;
    pds_clock_freq_t clock_freq;

    if (proto_req == NULL) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    freq = proto_req->clockfrequency();
    switch (freq) {
    case 833:
        clock_freq = PDS_CLOCK_FREQUENCY_833;
        break;
    case 900:
        clock_freq = PDS_CLOCK_FREQUENCY_900;
        break;
    case 957:
        clock_freq = PDS_CLOCK_FREQUENCY_957;
        break;
    case 1033:
        clock_freq = PDS_CLOCK_FREQUENCY_1033;
        break;
    case 1100:
        clock_freq = PDS_CLOCK_FREQUENCY_1100;
        break;
    default:
        PDS_TRACE_ERR("Clock-frequency update failed, accepted frequencies are 833, 900, 957, 1033 and 1100");
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::OK;
    }

    ret = debug::pds_clock_frequency_update(clock_freq);
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    return Status::OK;
}

Status
DebugSvcImpl::SystemTemperatureGet(ServerContext *context, const Empty *proto_req,
                                   pds::SystemTemperatureGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_system_temperature_t temp = {0};

    ret = debug::pds_get_system_temperature(&temp);
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    proto_rsp->set_dietemp(temp.dietemp);
    proto_rsp->set_localtemp(temp.localtemp);
    proto_rsp->set_hbmtemp(temp.hbmtemp);
    return Status::OK;
}

Status
DebugSvcImpl::SystemPowerGet(ServerContext *context, const Empty *proto_req,
                             pds::SystemPowerGetResponse *proto_rsp) {
    sdk_ret_t ret;
    pds_system_power_t pow = {0};

    ret = debug::pds_get_system_power(&pow);
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
    proto_rsp->set_pin(pow.pin);
    proto_rsp->set_pout1(pow.pout1);
    proto_rsp->set_pout2(pow.pout2);
    return Status::OK;
}
