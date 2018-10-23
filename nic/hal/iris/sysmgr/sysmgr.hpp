//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __HAL_SYSMGR_HPP__
#define __HAL_SYSMGR_HPP__

#include <string>
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/sysmgr.delphi.hpp"
#include "nic/sysmgr/lib/sysmgr_client.hpp"

namespace hal {
namespace sysmgr {

class sysmgr_client : public delphi::Service {
public:
    sysmgr_client(delphi::SdkPtr sdk) : sysmgr_(sdk, "hal") {}
    void OnMountComplete(void) { this->sysmgr_.init_done(); }
    bool SkipHeartbeat(void) { return false; }
    std::pair<delphi::error, std::string> Heartbeat(void);

private:
    ::sysmgr::Client    sysmgr_;
};

}    // namespace sysmgr
}    // namespace hal

#endif    // __HAL_SYSMGR_HPP__
