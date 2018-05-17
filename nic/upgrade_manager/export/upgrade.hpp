// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRAGE_H__
#define __UPGRADE_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/proto/upgrade.delphi.hpp"

namespace upgrade {

using namespace std;

typedef enum {
    SUCCESS,
    FAIL,
    INPROGRESS
} HdlrRespCode;

class UpgHandler {
public:
    UpgHandler() {}
    virtual HdlrRespCode UpgReqStatusCreate(delphi::objects::UpgReqStatusPtr req);
    virtual HdlrRespCode UpgReqStatusDelete(delphi::objects::UpgReqStatusPtr req);
    virtual HdlrRespCode HandleStateUpgReqRcvd(delphi::objects::UpgReqStatusPtr req);
    virtual HdlrRespCode HandleStatePreUpgState(delphi::objects::UpgReqStatusPtr req);
    virtual HdlrRespCode HandleStatePostBinRestart(delphi::objects::UpgReqStatusPtr req);
    virtual HdlrRespCode HandleStateProcessesQuiesced(delphi::objects::UpgReqStatusPtr req);
    virtual HdlrRespCode HandleStateDataplaneDowntimeStart(delphi::objects::UpgReqStatusPtr req);
    virtual HdlrRespCode HandleStateCleanup(delphi::objects::UpgReqStatusPtr req);
    virtual HdlrRespCode HandleStateUpgSuccess(delphi::objects::UpgReqStatusPtr req);
    virtual HdlrRespCode HandleStateUpgFailed(delphi::objects::UpgReqStatusPtr req);
    virtual HdlrRespCode HandleStateInvalidUpgState(delphi::objects::UpgReqStatusPtr req);
};
typedef std::shared_ptr<UpgHandler> UpgHandlerPtr;

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

// UpgReqReactor is the reactor for the UpgReqStatus object
class UpgReqReactor : public delphi::objects::UpgReqStatusReactor {
    delphi::SdkPtr sdk_;
    UpgHandlerPtr upgHdlrPtr_;
    UpgAppRespHdlrPtr upgAppRespPtr_;
    delphi::objects::UpgReqStatusPtr upgReqStatus_;
public:
    UpgReqReactor(delphi::SdkPtr sk, string name, UpgAppRespHdlrPtr ptr) {
        this->sdk_ = sk;
        this->upgHdlrPtr_ = make_shared<UpgHandler>();
        this->upgAppRespPtr_ = ptr;
    }

    UpgReqReactor(delphi::SdkPtr sk, UpgHandlerPtr uh, string name, UpgAppRespHdlrPtr ptr) {
        this->sdk_ = sk;
        this->upgHdlrPtr_ = uh;
        this->upgAppRespPtr_ = ptr;
    }

    // OnUpgReqStatusCreate gets called when UpgReqStatus object is created
    virtual delphi::error OnUpgReqStatusCreate(delphi::objects::UpgReqStatusPtr req);

    // OnUpgReqStatusDelete gets called when UpgReqStatus object is deleted
    virtual delphi::error OnUpgReqStatusDelete(delphi::objects::UpgReqStatusPtr req);

    // OnUpgReqState gets called when UpgReqState attribute changes
    virtual delphi::error OnUpgReqState(delphi::objects::UpgReqStatusPtr req);
};
typedef std::shared_ptr<UpgReqReactor> UpgReqReactorPtr;

class UpgSdk : public delphi::Service {
    delphi::SdkPtr sdk_;
    UpgReqReactorPtr upgReqReactPtr_;
    UpgAppRespHdlrPtr upgAppRespPtr_;
public:
    UpgSdk(delphi::SdkPtr sk, string name) {
        sdk_ = sk;
        upgAppRespPtr_ = make_shared<UpgAppRespHdlr>(sk, name);
        upgReqReactPtr_ = make_shared<UpgReqReactor>(sk, name, upgAppRespPtr_);
        delphi::objects::UpgReqStatus::Mount(sdk_, delphi::ReadMode);
        delphi::objects::UpgReqStatus::Watch(sdk_, upgReqReactPtr_);
        delphi::objects::UpgAppResp::Mount(sdk_, delphi::ReadWriteMode);
    }

    UpgSdk(delphi::SdkPtr sk, UpgHandlerPtr uh, string name) {
        sdk_ = sk;
        upgAppRespPtr_ = make_shared<UpgAppRespHdlr>(sk, name);
        upgReqReactPtr_ = make_shared<UpgReqReactor>(sk, uh, name, upgAppRespPtr_);
        delphi::objects::UpgReqStatus::Mount(sdk_, delphi::ReadMode);
        delphi::objects::UpgReqStatus::Watch(sdk_, upgReqReactPtr_);
        delphi::objects::UpgAppResp::Mount(sdk_, delphi::ReadWriteMode);
    }

    // OnMountComplete gets called when all the objects are mounted
    void OnMountComplete(void);

    void SendAppRespSuccess(void);
    void SendAppRespFail(void);
};
typedef std::shared_ptr<UpgSdk> UpgSdkPtr;

} // namespace upgrade

#endif // __UPGRAGE_H__
