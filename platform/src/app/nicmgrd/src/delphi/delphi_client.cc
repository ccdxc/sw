// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream.h>

#include <delphi/delphi_client.hpp>
#include <upgrade/upgrade.hpp>

using namespace std;

namespace nicmgr {

NicMgrService::NicMgrService(delphi::SdkPtr sk, string name) {
    
    sdk_ = sk;
    svcName_ = name;
    upgsdk_ = make_shared<UpgSdk>(sdk_, make_shared<nicmgr_upg_hndlr>(), name, NON_AGENT);

}


}
