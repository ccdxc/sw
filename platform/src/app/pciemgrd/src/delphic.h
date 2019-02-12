/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __DELPHI_H__
#define __DELPHI_H__

#include <memory>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/sysmgr/lib/sysmgr_client.hpp"
#include "nic/upgrade_manager/export/upgcsdk/upgrade.hpp"

using namespace std;
using namespace upgrade;

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
};

class PciemgrSvcHandler : public UpgHandler {
public:
    HdlrResp CompatCheckHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        return resp;
    }

    HdlrResp SaveStateHandler(UpgCtx& upgCtx) {
        HdlrResp resp = {.resp=SUCCESS, .errStr=""};
        return resp;
    }

};

#define SERVICE_NAME "pciemgrd"

int delphi_client_start();

#endif
