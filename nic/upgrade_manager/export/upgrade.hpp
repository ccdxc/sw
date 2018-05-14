// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRAGE_H__
#define __UPGRADE_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/proto/upgrade.delphi.hpp"

namespace upgrade {

using namespace std;

class UpgHandler {
public:
    UpgHandler() {}
    virtual delphi::error UpgReqStatusCreate(delphi::objects::UpgReqStatusPtr req);
    virtual delphi::error UpgReqStatusDelete(delphi::objects::UpgReqStatusPtr req);
    virtual delphi::error HandleStateUpgReqRcvd(delphi::objects::UpgReqStatusPtr req);
    virtual delphi::error HandleStatePreUpgState(delphi::objects::UpgReqStatusPtr req);
    virtual delphi::error HandleStatePostBinRestart(delphi::objects::UpgReqStatusPtr req);
    virtual delphi::error HandleStateProcessesQuiesced(delphi::objects::UpgReqStatusPtr req);
    virtual delphi::error HandleStateDataplaneDowntimeStart(delphi::objects::UpgReqStatusPtr req);
    virtual delphi::error HandleStateCleanup(delphi::objects::UpgReqStatusPtr req);
    virtual delphi::error HandleStateUpgSuccess(delphi::objects::UpgReqStatusPtr req);
    virtual delphi::error HandleStateUpgFailed(delphi::objects::UpgReqStatusPtr req);
    virtual delphi::error HandleStateInvalidUpgState(delphi::objects::UpgReqStatusPtr req);
};
typedef std::shared_ptr<UpgHandler> UpgHandlerPtr;

// UpgReqReactor is the reactor for the UpgReqStatus object
class UpgReqReactor : public delphi::objects::UpgReqStatusReactor {
    delphi::SdkPtr sdk_;
    UpgHandlerPtr upgHdlrPtr_;
public:
    UpgReqReactor(delphi::SdkPtr sk) {
        this->sdk_ = sk;
        upgHdlrPtr_ = make_shared<UpgHandler>();
    }

    UpgReqReactor(delphi::SdkPtr sk, UpgHandlerPtr uh) {
        this->sdk_ = sk;
        this->upgHdlrPtr_ = uh;
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
    UpgHandlerPtr upgHdlrPtr_;
    UpgReqReactorPtr upgReqReactPtr_;
public:
    UpgSdk(delphi::SdkPtr sk) {
        this->sdk_ = sk;
        this->upgHdlrPtr_ = NULL;
        upgReqReactPtr_ = make_shared<UpgReqReactor>(sk);
        delphi::objects::UpgReqStatus::Mount(sdk_, delphi::ReadMode);
        delphi::objects::UpgReqStatus::Watch(sdk_, upgReqReactPtr_);
    }

    UpgSdk(delphi::SdkPtr sk, UpgHandlerPtr uh) {
        this->sdk_ = sk;
        this->upgHdlrPtr_ = uh;
        upgReqReactPtr_ = make_shared<UpgReqReactor>(sk, uh);
        delphi::objects::UpgReqStatus::Mount(sdk_, delphi::ReadMode);
        delphi::objects::UpgReqStatus::Watch(sdk_, upgReqReactPtr_);
    }

    // OnMountComplete gets called when all the objects are mounted
    void OnMountComplete(void);
};
typedef std::shared_ptr<UpgSdk> UpgSdkPtr;

} // namespace upgrade

#endif // __UPGRAGE_H__
