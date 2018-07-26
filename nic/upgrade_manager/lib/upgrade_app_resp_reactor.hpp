// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_APP_RESP_REACT_H__
#define __UPGRADE_APP_RESP_REACT_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/upgrade/upgrade.delphi.hpp"
#include "upgrade_req_react.hpp"

namespace upgrade {

using namespace std;

class UpgAppRespReact : public delphi::objects::UpgAppRespReactor {
    delphi::SdkPtr     sdk_;
    UpgReqReactPtr      upgMgr_;
public:
    UpgAppRespReact(delphi::SdkPtr sk, UpgReqReactPtr upgmgr) {
        sdk_ = sk;
        upgMgr_ = upgmgr;
    }

    // OnUpgAppRespCreate gets called when UpgAppResp object is created
    virtual delphi::error OnUpgAppRespCreate(delphi::objects::UpgAppRespPtr resp);

    // OnUpgAppRespVal gets called when UpgAppRespVal attribute changes
    virtual delphi::error OnUpgAppRespVal(delphi::objects::UpgAppRespPtr resp);

    // OnUpgAppRespDelete gets called when UpgAppResp object is deleted
    virtual delphi::error OnUpgAppRespDelete(delphi::objects::UpgAppRespPtr resp);

    string UpgStateRespTypeToStr(UpgStateRespType type);
};
typedef std::shared_ptr<UpgAppRespReact> UpgAppRespReactPtr;

} // namespace upgrade 

#endif // __UPGRADE_APP_RESP_REACT_H__
