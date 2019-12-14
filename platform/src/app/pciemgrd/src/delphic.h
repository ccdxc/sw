/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
 */

#ifndef __DELPHIC_H__
#define __DELPHIC_H__

#include <memory>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/sysmgr/lib/sysmgr_client.hpp"
#include "nic/upgrade_manager/export/upgcsdk/upgrade.hpp"
#include "gen/proto/sysmgr.delphi.hpp"
#include "gen/proto/pciemgr/pciemgr.delphi.hpp"

using namespace std;
using namespace upgrade;
using delphi::objects::PciePortStatus;
using delphi::objects::PciePortStatusPtr;

// delphi service for pciemgr
class PciemgrService: public delphi::Service,
                      public enable_shared_from_this<PciemgrService>
{
private:
    delphi::SdkPtr      delphi;
    sysmgr::ClientPtr   sysmgr;
    string              name;
    UpgSdkPtr           upgsdk;

public:
    PciemgrService();
    void RegisterService();
    virtual void OnMountComplete();
    virtual std::string Name();
    void UpdatePciePortStatus(const int port,
                              const enum pciemgr::PciePortOperStatus status,
                              const int gen = 0,
                              const int width = 0,
                              const int reversed = 0,
                              const string faultstr = "");
};

class PciemgrSvcHandler : public UpgHandler {
public:
    HdlrResp CompatCheckHandler(UpgCtx& upgCtx);
    HdlrResp SaveStateHandler(UpgCtx& upgCtx);
    void SuccessHandler(UpgCtx& upgCtx);
    HdlrResp FailedHandler(UpgCtx& upgCtx);
};

#define SERVICE_NAME "pciemgrd"

#endif /* __DELPHIC_H__ */
