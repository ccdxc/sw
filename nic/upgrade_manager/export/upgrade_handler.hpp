// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPGRAGE_HANDLER_H__
#define __UPGRAGE_HANDLER_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/proto/upgrade.delphi.hpp"

namespace upgrade {

using namespace std;

typedef enum {
    SUCCESS,
    FAIL,
    INPROGRESS
} HdlrRespCode;

typedef struct HdlrResp_ {
    HdlrRespCode     resp;
    string           errStr;
} HdlrResp;

class UpgHandler {
public:
    UpgHandler() {}
    virtual HdlrResp UpgStateReqCreate(delphi::objects::UpgStateReqPtr req);
    virtual HdlrResp UpgStateReqDelete(delphi::objects::UpgStateReqPtr req);
    virtual HdlrResp HandleStateUpgReqRcvd(delphi::objects::UpgStateReqPtr req);
    virtual HdlrResp HandleStatePreUpgState(delphi::objects::UpgStateReqPtr req);
    virtual HdlrResp HandleStatePostBinRestart(delphi::objects::UpgStateReqPtr req);
    virtual HdlrResp HandleStateProcessesQuiesced(delphi::objects::UpgStateReqPtr req);
    virtual HdlrResp HandleStateDataplaneDowntimeStart(delphi::objects::UpgStateReqPtr req);
    virtual HdlrResp HandleStateCleanup(delphi::objects::UpgStateReqPtr req);
    virtual HdlrResp HandleStateUpgSuccess(delphi::objects::UpgStateReqPtr req);
    virtual HdlrResp HandleStateUpgFailed(delphi::objects::UpgStateReqPtr req);
};
typedef std::shared_ptr<UpgHandler> UpgHandlerPtr;

} // namespace upgrade

#endif // __UPGRAGE_HANDLER_H__
