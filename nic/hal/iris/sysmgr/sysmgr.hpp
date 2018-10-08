//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __HAL_SYSMGR_HPP__
#define __HAL_SYSMGR_HPP__

//#include <memory>
#include <string>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/sysmgr/proto/sysmgr.delphi.hpp"
#include "nic/sysmgr/lib/sysmgr_client.hpp"

namespace hal {
namespace sysmgr {

class sysmgr_client : public delphi::Service {
public:
    sysmgr_client(delphi::SdkPtr sdk) : sysmgr(sdk, "hal") {}
    void OnMountComplete(void) { this->sysmgr.init_done(); }
    bool SkipHeartbeat(void) { return false; }
    std::pair<deplhi::error, string> Heartbeat(void);

private:
    sysmgr::Client    sysmgr;
};

}    // namespace sysmgr
}    // namespace hal

#endif    // __HAL_SYSMGR_HPP__
