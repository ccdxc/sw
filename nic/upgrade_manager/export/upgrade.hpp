// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __UPG_MGR_STATUS_H__
#define __UPG_MGR_STATUS_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/proto/upgrade.delphi.hpp"

namespace upgrade {

using namespace std;

// UpgReqStatusMgr is the reactor for the UpgReqStatusMgr object
class UpgReqStatusMgr : public delphi::objects::UpgReqStatusReactor {
    delphi::SdkPtr sdk_;
public:
    UpgReqStatusMgr(delphi::SdkPtr sk) {
        this->sdk_ = sk;
    }

    // OnUpgReqStatusCreate gets called when UpgReqStatus object is created
    virtual delphi::error OnUpgReqStatusCreate(delphi::objects::UpgReqStatusPtr req);

    // OnUpgReqStatusDelete gets called when UpgReqStatus object is deleted
    virtual delphi::error OnUpgReqStatusDelete(delphi::objects::UpgReqStatusPtr req);

    // OnUpgReqState gets called when UpgReqState attribute changes
    virtual delphi::error OnUpgReqState(delphi::objects::UpgReqStatusPtr req);
};
typedef std::shared_ptr<UpgReqStatusMgr> UpgReqStatusMgrPtr;

} // namespace example

#endif // __UPG_MGR_STATUS_H__
