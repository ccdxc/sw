/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <cinttypes>

#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#include "nic/sdk/platform/pcieport/include/pcieport.h"
#include "nic/sdk/platform/pciemgrd/pciemgrd_impl.hpp"
#include "delphic.h"

static shared_ptr<PciemgrService> delphic;

int delphi_client_start (void)
{
    printf("Starting delphi thread..\n");
    delphic = make_shared<PciemgrService>();
    delphic->RegisterService();
    return 0;
}

void update_pcie_port_status(
    const int port,
    const pciemgr_port_status_t status,
    const int gen,
    const int width,
    const int reversed,
    const char *faultstr)
{
    enum pciemgr::PciePortOperStatus dstatus;

    // map pciemgr status to delphi status type
    switch (status) {
    case PCIEMGR_UP:    dstatus = pciemgr::Up; break;
    case PCIEMGR_DOWN:  dstatus = pciemgr::Down; break;
    case PCIEMGR_FAULT: dstatus = pciemgr::Fault; break;
    default:            dstatus = pciemgr::Fault; break;
    }

    if (delphic) {
        delphic->UpdatePciePortStatus(port, dstatus,
                                      gen, width, reversed, faultstr);
    }
}

static void delphi_update_pciemgr_metrics(const int port)
{
    static delphi::objects::pciemgrmetrics_t m;
    pciemgr_stats_t *s = pciehw_stats_get(port);

    if (delphic == NULL) return;
    if (s == NULL) return;

#define PCIEMGR_STATS_DEF(S) \
    m.S = s->S;
#include "nic/sdk/platform/pciemgr/include/pciemgr_stats_defs.h"

    delphi::objects::PcieMgrMetrics::Publish(port, &m);
}

static void delphi_update_pcie_port_metrics(const int port)
{
    static delphi::objects::pcieportmetrics_t m;
    pcieport_stats_t *s = pcieport_stats_get(port);

    if (delphic == NULL) return;
    if (s == NULL) return;

#define PCIEPORT_STATS_DEF(S) \
    m.S = s->S;
#include "nic/sdk/platform/pcieport/include/pcieport_stats_defs.h"

    delphi::objects::PciePortMetrics::Publish(port, &m);
}

void update_pcie_metrics(const int port)
{
    static int created[PCIEPORT_NPORTS];

    if (delphic == NULL) return;
    if (!created[port]) {
        delphi::objects::PcieMgrMetrics::CreateTable();
        delphi::objects::PciePortMetrics::CreateTable();
        created[port] = 1;
    }
    delphi_update_pciemgr_metrics(port);
    delphi_update_pcie_port_metrics(port);
}

PciemgrService::PciemgrService()
{
    this->delphi = make_shared<delphi::Sdk>();
    this->name = SERVICE_NAME;
    this->sysmgr = sysmgr::CreateClient(this->delphi, SERVICE_NAME);
    this->upgsdk = make_shared<UpgSdk>(this->delphi,
                                       make_shared<PciemgrSvcHandler>(),
                                       this->name, NON_AGENT, nullptr);

    PciePortStatus::Mount(this->delphi, delphi::ReadWriteMode);
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

std::string PciemgrService::Name()
{
    return "pciemgrd";
}

void PciemgrService::UpdatePciePortStatus(
    const int port,
    const enum pciemgr::PciePortOperStatus status,
    const int gen,
    const int width,
    const int reversed,
    const string faultstr)
{
    PciePortStatusPtr s = make_shared<PciePortStatus>();

    s->set_key(port);
    s->set_status(status);
    s->set_gen(gen);
    s->set_width(width);
    s->set_lanes_reversed(reversed);
    s->set_fault_reason(faultstr);

    this->delphi->QueueUpdate(s);
}
