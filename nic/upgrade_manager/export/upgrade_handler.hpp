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

} // namespace upgrade

#endif // __UPGRAGE_HANDLER_H__
