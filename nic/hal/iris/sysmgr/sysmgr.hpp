//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __HAL_SYSMGR_HPP__
#define __HAL_SYSMGR_HPP__

#include <string>
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/sysmgr.delphi.hpp"
#include "nic/sysmgr/lib/sysmgr_client.hpp"

using std::string;
using delphi::error;

namespace hal {
namespace sysmgr {

class sysmgr_client : public ::sysmgr::ServiceStatusReactor,
          public std::enable_shared_from_this<sysmgr_client> {
public:
    sysmgr_client(delphi::SdkPtr &sdk);
    void init_done(void);
    void register_for_service(string name);
    bool SkipHeartbeat(void);
    std::pair<error, string> Heartbeat(void);
    void ServiceUp(string name);
    void ServiceDown(string name);
private:
    ::sysmgr::ClientPtr sysmgr_;
};
std::shared_ptr<sysmgr_client> create_sysmgr_client(delphi::SdkPtr &sdk);

}    // namespace sysmgr
}    // namespace hal

#endif    // __HAL_SYSMGR_HPP__
