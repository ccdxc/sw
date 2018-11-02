/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include "delphic.h"
#include <memory>
#include <pthread.h>
#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/sysmgr.delphi.hpp"
#include "nic/sysmgr/lib/sysmgr_client.hpp"

using namespace std;

static shared_ptr<PciemgrService> delphic;


int delphi_client_start (void)
{
  printf("Starting delphi thread..\n");

  delphic = make_shared<PciemgrService>();
  delphic->RegisterService();

  return 0;
}

PciemgrService::PciemgrService()
{
  this->delphi = make_shared<delphi::Sdk>();
  this->name = SERVICE_NAME;
  this->sysmgr = sysmgr::CreateClient(this->delphi, SERVICE_NAME);
}

void PciemgrService::RegisterService()
{
  this->delphi->RegisterService(shared_from_this());
  this->delphi->Connect();
}

void PciemgrService::OnMountComplete()
{
    printf("delphi on mount complete got called..\n");
    fflush(stdout);

    // notify sysmgr that init is done
    this->sysmgr->init_done();
}
