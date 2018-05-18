// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRADE_APP_RESP_HDLR_H__
#define __UPGRADE_APP_RESP_HDLR_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/proto/upgrade.delphi.hpp"

namespace upgrade {

using namespace std;

//UpgAppRespHdlr class is used to create the object for 
class UpgAppRespHdlr {
    string appName_;
    delphi::SdkPtr sdk_;
public:
    UpgAppRespHdlr(delphi::SdkPtr sk, string name) {
        this->appName_ = name;
        this->sdk_ = sk;
    }

    //CreateUpgAppResp creates the object UpgAppResp
    delphi::error CreateUpgAppResp(delphi::objects::UpgReqStatusPtr ptr);

    UpgRespStateType GetUpgAppRespNextPass(UpgReqStateType reqType);
    UpgRespStateType GetUpgAppRespNextFail(UpgReqStateType reqType);
    UpgRespStateType GetUpgAppRespNext(UpgReqStateType reqType, bool isReqSuccess);

    //CreateOrUpdateUpgAppResp creates the response for upgrade_manager
    delphi::error UpdateUpgAppResp(UpgRespStateType type);

    //findUpgAppResp returns the UpgAppResp object for this application
    delphi::objects::UpgAppRespPtr findUpgAppResp(string name);

    //GetUpgReqStatusPtr will return the pointer for UpgReqStatus object
    delphi::objects::UpgReqStatusPtr GetUpgReqStatusPtr(void) {
        delphi::objects::UpgReqStatusPtr ret;
        vector<delphi::objects::UpgReqStatusPtr> upgReqStatuslist = delphi::objects::UpgReqStatus::List(sdk_);
        for (vector<delphi::objects::UpgReqStatusPtr>::iterator reqStatus=upgReqStatuslist.begin(); reqStatus != upgReqStatuslist.end(); ++reqStatus) {
            ret = *reqStatus;
         }
        return ret;
    }
};
typedef std::shared_ptr<UpgAppRespHdlr> UpgAppRespHdlrPtr;

} // namespace upgrade

#endif // __UPGRADE_APP_RESP_HDLR_H__
