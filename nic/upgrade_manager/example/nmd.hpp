// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef __NMD_SERVICE_H__
#define __NMD_SERVICE_H__

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/upgrade_manager/proto/upgrade.delphi.hpp"

namespace nmd {

using namespace std;

class NMDUpgAppRespHdlr : public delphi::objects::UpgAppRespReactor {
    delphi::SdkPtr     sdk_;
public:
    NMDUpgAppRespHdlr() {}

    NMDUpgAppRespHdlr(delphi::SdkPtr sk) {
        this->sdk_ = sk;
    }
    NMDUpgAppRespHdlr(delphi::SdkPtr sk, string name) {
        this->sdk_ = sk;
    }

    // OnUpgAppRespCreate gets called when UpgAppResp object is created
    virtual delphi::error OnUpgAppRespCreate(delphi::objects::UpgAppRespPtr resp);
    
    // OnUpgAppRespVal gets called when UpgAppRespVal attribute changes
    virtual delphi::error OnUpgAppRespVal(delphi::objects::UpgAppRespPtr resp);
};
typedef std::shared_ptr<NMDUpgAppRespHdlr> NMDUpgAppRespHdlrPtr;

// NMDService is the service object for NMD manager 
class NMDService : public delphi::Service, public enable_shared_from_this<NMDService> {
private:
    delphi::SdkPtr        sdk_;
    string                svcName_;
    NMDUpgAppRespHdlrPtr  nmdUpgAppRespHdlr_;
public:
    // NMDService constructor
    NMDService(delphi::SdkPtr sk);
    NMDService(delphi::SdkPtr sk, string name);

    // createUpgReqSpec creates a dummy Upgrade Request
    void createUpgReqSpec();
    void updateUpgReqSpec();
    // override service name method
    virtual string Name() { return svcName_; }
    void OnMountComplete(void);

    // timer for creating a dummy object
    ev::timer          createTimer;
    void createTimerHandler(ev::timer &watcher, int revents);
    void createTimerUpdHandler(ev::timer &watcher, int revents);
};
typedef std::shared_ptr<NMDService> NMDServicePtr;

} // namespace example

#endif // __NMD_SERVICE_H__
