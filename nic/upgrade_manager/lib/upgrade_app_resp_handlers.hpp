// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_APP_RESP_HANDLERS_H__
#define __UPGRADE_APP_RESP_HANDLERS_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/proto/upgrade.delphi.hpp"
#include "upgrade_mgr.hpp"

namespace upgrade {

using namespace std;

class UpgAppRespHdlr : public delphi::objects::UpgAppRespReactor {
    delphi::SdkPtr     sdk_;
    UpgradeMgrPtr      upgMgr_;
public:
    UpgAppRespHdlr(delphi::SdkPtr sk, UpgradeMgrPtr upgmgr) {
        this->sdk_ = sk;
        this->upgMgr_ = upgmgr;
    }

    // OnUpgAppRespCreate gets called when UpgAppResp object is created
    virtual delphi::error OnUpgAppRespCreate(delphi::objects::UpgAppRespPtr resp);

    // OnUpgAppRespVal gets called when UpgAppRespVal attribute changes
    virtual delphi::error OnUpgAppRespVal(delphi::objects::UpgAppRespPtr resp);

    // OnUpgAppRespDelete gets called when UpgAppResp object is deleted
    virtual delphi::error OnUpgAppRespDelete(delphi::objects::UpgAppRespPtr resp);

    string UpgRespStateTypeToStr(UpgRespStateType type);
};
typedef std::shared_ptr<UpgAppRespHdlr> UpgAppRespHdlrPtr;

} // namespace upgrade 

#endif // __UPGRADE_APP_RESP_HANDLERS_H__
