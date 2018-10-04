//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __HAL_SYSMGR_HPP__
#define __HAL_SYSMGR_HPP__

#include <memory>
#include <string>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/sysmgr/proto/sysmgr.delphi.hpp"
#include "nic/sysmgr/lib/sysmgr_client.hpp"

namespace hal {

class sysmgr_client : public delphi::Service, public enable_shared_from_this<SysmgrClient> {
public:
    sysmgr_client(delphi::SdkPtr delphi, string name): sysmgr(delphi, name) {
        this->delphi = delphi;
        this->name = name;
    }
    void OnMountComplete() {
        this->sysmgr.init_done();
    }
    std::pair<error, string> Heartbeat() {
        return std::make_pair(error::OK(), "");
    };
    bool SkipHeartbeat() {
        return false;
    };

private:
    delphi::SdkPtr delphi;
    string name;
    sysmgr::Client sysmgr;
};

} // namespace hal
