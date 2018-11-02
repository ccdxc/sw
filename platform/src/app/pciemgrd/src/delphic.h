/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __DELPHI_H__
#define __DELPHI_H__

#include <memory>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/sysmgr/lib/sysmgr_client.hpp"

using namespace std;

// delphi service for pciemgr
class PciemgrService: public delphi::Service,
		    public enable_shared_from_this<PciemgrService>
{
 private:
  delphi::SdkPtr delphi;
  sysmgr::ClientPtr sysmgr;
  string name;

 public:
  PciemgrService();
  void RegisterService();
  virtual void OnMountComplete();
};

#define SERVICE_NAME "pciemgrd"

int delphi_client_start();

#endif
