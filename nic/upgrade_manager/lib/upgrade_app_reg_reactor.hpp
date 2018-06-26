// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_APP_REG_REACTOR_H__
#define __UPGRADE_APP_REG_REACTOR_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/upgrade/upgrade.delphi.hpp"
#include "upgrade_mgr.hpp"

namespace upgrade {

using namespace std;

//UpgAppRegReact class is used to create the object for 
class UpgAppRegReact : public delphi::objects::UpgAppReactor {
    UpgradeMgrPtr      upgMgr_;
public:
    UpgAppRegReact(UpgradeMgrPtr upgmgr) {
        upgMgr_ = upgmgr;
    }

    // OnUpgApppCreate gets called when UpgApp object is created
    virtual delphi::error OnUpgAppCreate(delphi::objects::UpgAppPtr app);
};
typedef std::shared_ptr<UpgAppRegReact> UpgAppRegReactPtr;

} // namespace upgrade

#endif // __UPGRADE_APP_REG_REACTOR_H__
