//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __NICMGR_SYSMGR_HPP__
#define __NICMGR_SYSMGR_HPP__

#include <memory>
#include <string>

#include "delphi_sdk.hpp"
#include "gen/proto/sysmgr.delphi.hpp"
#include "nic/sysmgr/lib/sysmgr_client.hpp"

using namespace std;
using namespace sysmgr;

namespace nicmgr {
//class sysmgr_client : public delphi::Service, public enable_shared_from_this<SysmgrClient> {
class sysmgr_client : public delphi::Service {
public:
    sysmgr_client(delphi::SdkPtr delphi, std::string name): sysmgr(delphi, name) {
        this->delphi = delphi;
        this->name = name;
    }
    void OnMountComplete() {
        this->sysmgr.init_done();
    }
    bool SkipHeartbeat() {
        return false;
    };

private:
    delphi::SdkPtr delphi;
    string name;
    sysmgr::Client sysmgr;
};

} // namespace nicmgr 
#endif //__NICMGR_SYSMGR_HPP__
