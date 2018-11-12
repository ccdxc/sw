//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/iris/sysmgr/sysmgr.hpp"
#include "nic/hal/core/heartbeat/heartbeat.hpp"

namespace hal {
namespace sysmgr {

std::shared_ptr<sysmgr_client> create_sysmgr_client(delphi::SdkPtr &sdk)
{
    std::shared_ptr<sysmgr_client> client = std::make_shared<sysmgr_client>(sdk);
    // register for nicmgr up/down notifications
    client->register_for_service("nicmgr");
    return client;
}

void sysmgr_client::register_for_service(string name)
{
    HAL_TRACE_DEBUG("Registering for {} up|down notifications", name);
    this->sysmgr_->register_service_reactor(name, shared_from_this());
}

sysmgr_client::sysmgr_client(delphi::SdkPtr &sdk)
{
    this->sysmgr_ = ::sysmgr::CreateClient(sdk, "hal");
}

void sysmgr_client::init_done(void)
{
    HAL_TRACE_DEBUG("Indicating init_done to sysmgr\n");
    this->sysmgr_->init_done();
}

void sysmgr_client::ServiceUp(string name)
{
    HAL_TRACE_DEBUG("Rcvd {} up ntfn\n", name);
}

void sysmgr_client::ServiceDown(string name)
{
    HAL_TRACE_DEBUG("Rcvd {} down ntfn\n", name);
}

bool sysmgr_client::SkipHeartbeat(void) {
    return false;
}

std::pair<error, string> sysmgr_client::Heartbeat(void) {
    if (!hal::hb::is_hal_healthy()) {
        return std::make_pair(error::OK(), "");
    }
    return std::make_pair(error::New("HAL Unhealthy"), "");
};

}    // namespace sysmgr
}    // namespace hal
