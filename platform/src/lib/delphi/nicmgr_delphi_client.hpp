// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __NICMGR_DELPHI_CLIENT_HPP__
#define __NICMGR_DELPHI_CLIENT_HPP__

#include <memory>
#include <string>
#include <iostream>

#include "nicmgr_upgrade.hpp"
#include "nicmgr_sysmgr.hpp"
#include "nicmgr.delphi.hpp"

namespace nicmgr {


using nicmgr::nicmgr_upg_hndlr;
using nicmgr::sysmgr_client;

class NicMgrService : public delphi::Service, public enable_shared_from_this<NicMgrService> {
private:
    string                       svcName_;

public:
    shared_ptr<sysmgr::Client>   sysmgr_;
    UpgSdkPtr                    upgsdk_;
    delphi::SdkPtr               sdk_;
    NicMgrService(delphi::SdkPtr sk);
    NicMgrService(delphi::SdkPtr sk, string name);
 
    virtual string Name() { return svcName_; }
    
    void OnMountComplete(void); 
};

shared_ptr<NicMgrService> nicmgr_svc_;

}    // namespace nicmgr

#endif
