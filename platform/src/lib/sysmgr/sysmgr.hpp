//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NICMGR_SYSMGR_HPP__
#define __NICMGR_SYSMGR_HPP__

#include <memory>
#include <string>

#include "gen/proto/sysmgr.delphi.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/sysmgr/lib/sysmgr_client.hpp"

using namespace std;
using namespace sysmgr;

namespace nicmgr {
class sysmgr_client :  public ::sysmgr::ServiceStatusReactor,
                       public std::enable_shared_from_this<sysmgr_client> {
public:
    sysmgr_client(delphi::SdkPtr &sdk);
    void register_for_service(std::string name);
    virtual void ServiceUp(std::string name);
    virtual void ServiceDown(std::string name);
    void init_done(void);
    bool SkipHeartbeat(void) {
        return false;
    };
private:
    sysmgr::ClientPtr    sysmgr_;
};
std::shared_ptr<sysmgr_client> create_sysmgr_client(delphi::SdkPtr &sdk);

} // namespace nicmgr 
#endif //__NICMGR_SYSMGR_HPP__
