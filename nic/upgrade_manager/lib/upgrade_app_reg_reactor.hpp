// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_APP_REG_REACTOR_H__
#define __UPGRADE_APP_REG_REACTOR_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/upgrade/upgrade.delphi.hpp"
#include "upgrade_req_react.hpp"

namespace upgrade {

using namespace std;

//UpgAppRegReact class is used to create the object for 
class UpgAppRegReact : public delphi::objects::UpgAppReactor {
    UpgReqReactPtr      upgMgr_;
    delphi::SdkPtr      sdk_;
public:
    UpgAppRegReact(UpgReqReactPtr upgmgr, delphi::SdkPtr sk) {
        upgMgr_ = upgmgr;
        sdk_ = sk;
    }

    // OnUpgApppCreate gets called when UpgApp object is created
    virtual delphi::error OnUpgAppCreate(delphi::objects::UpgAppPtr app);

    virtual void OnMountComplete(void);
};
typedef std::shared_ptr<UpgAppRegReact> UpgAppRegReactPtr;

} // namespace upgrade

#endif // __UPGRADE_APP_REG_REACTOR_H__
