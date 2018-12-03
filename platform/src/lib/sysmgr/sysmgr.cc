//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <memory>
#include <iostream>
#include <string>

#include "sysmgr.hpp"

#include "platform/src/lib/nicmgr/include/logger.hpp"

using namespace std;

namespace nicmgr {

std::shared_ptr<sysmgr_client> create_sysmgr_client(delphi::SdkPtr &sdk)
{
    std::shared_ptr<sysmgr_client> client = std::make_shared<sysmgr_client>(sdk);
    // register for nicmgr up/down notifications
    client->register_for_service("hal");
    return client;
}

void sysmgr_client::register_for_service(std:: string name)
{
    NIC_LOG_DEBUG("Registering for {} up|down notifications", name);
    this->sysmgr_->register_service_reactor(name, shared_from_this());
}

sysmgr_client::sysmgr_client(delphi::SdkPtr &sdk)
{
    this->sysmgr_ = ::sysmgr::CreateClient(sdk, "nicmgr");
}

void sysmgr_client::init_done(void)
{
    NIC_LOG_DEBUG("Indicating init_done to sysmgr");
    this->sysmgr_->init_done();
}

void sysmgr_client::ServiceUp(std::string name)
{
    NIC_LOG_DEBUG("Rcvd {} up ntfn", name);
}

void sysmgr_client::ServiceDown(std::string name)
{
    NIC_LOG_DEBUG("Rcvd {} down ntfn", name);
}

}    // namespace nicmgr
