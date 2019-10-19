#include "delphi_service.hpp"

#include <memory>
#include <string>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/client.delphi.hpp"
#include "gen/proto/sysmgr.delphi.hpp"
#include "gen/proto/eventtypes.pb.h"

#include "../bus_api.hpp"

SysmgrBusPtr
init_bus (bus_api_t *api)
{
    delphi::SdkPtr delphi_sdk = std::make_shared<delphi::Sdk>();

    DelphiServicePtr svc = DelphiService::create(delphi_sdk, api);
    delphi_sdk->RegisterService(svc);

    return svc;
}

DelphiServicePtr DelphiService::create(delphi::SdkPtr sdk, bus_api_t *bus_api)
{
    DelphiServicePtr svc = std::make_shared<DelphiService>();
    svc->bus_api = bus_api;
    svc->sdk = sdk;
    svc->name = "sysmgr";

    delphi::objects::SysmgrProcessStatus::Mount(svc->sdk, delphi::ReadWriteMode);
    delphi::objects::SysmgrSystemStatus::Mount(svc->sdk, delphi::ReadWriteMode);
    delphi::objects::SysmgrServiceStatus::Mount(svc->sdk, delphi::ReadMode);
    delphi::objects::DelphiClientStatus::Mount(svc->sdk, delphi::ReadMode);
    delphi::objects::SysmgrShutdownReq::Mount(svc->sdk, delphi::ReadMode);
    delphi::objects::SysmgrRespawnReq::Mount(svc->sdk, delphi::ReadMode);

    delphi::objects::SysmgrServiceStatus::Watch(svc->sdk, svc);
    delphi::objects::DelphiClientStatus::Watch(svc->sdk, svc);
    delphi::objects::SysmgrShutdownReq::Watch(svc->sdk, svc);
    delphi::objects::SysmgrRespawnReq::Watch(svc->sdk, svc);

    return svc;
}

void
DelphiService::Connect(void) {
    this->sdk->Connect();
}

void
DelphiService::SystemFault(std::string reason) {
    auto obj = std::make_shared<delphi::objects::SysmgrSystemStatus>();

    obj->set_state(::sysmgr::Fault);
    obj->set_reason(reason);

    this->sdk->QueueUpdate(obj);
}

void
DelphiService::ProcessDied(std::string name, pid_t pid, std::string reason) {
    auto obj = std::make_shared<delphi::objects::SysmgrProcessStatus>();

    obj->set_key(name);
    obj->set_pid(pid);
    obj->set_state(::sysmgr::Died);
    obj->set_exitreason(reason);

    this->sdk->QueueUpdate(obj);
}

void DelphiService::OnMountComplete()
{
    this->bus_api->bus_up("delphi");
    auto obj = std::make_shared<delphi::objects::SysmgrSystemStatus>();
    obj->set_state(::sysmgr::Normal);
    this->sdk->QueueUpdate(obj);
}

std::string DelphiService::Name()
{
    return "sysmgr";
}

delphi::error DelphiService::OnSysmgrServiceStatusCreate(
    delphi::objects::SysmgrServiceStatusPtr obj)
{
    this->bus_api->service_started(obj->key());
    return delphi::error::OK();
}

delphi::error DelphiService::OnSysmgrServiceStatusDelete(
    delphi::objects::SysmgrServiceStatusPtr obj)
{
    return delphi::error::OK();
}

delphi::error DelphiService::OnSysmgrServiceStatusUpdate(
    delphi::objects::SysmgrServiceStatusPtr obj)
{
    this->bus_api->service_started(obj->key());
    return delphi::error::OK();
}

delphi::error DelphiService::OnDelphiClientStatusCreate(
    delphi::objects::DelphiClientStatusPtr obj)
{
    this->bus_api->service_heartbeat(obj->key());
    return delphi::error::OK();
}

delphi::error DelphiService::OnDelphiClientStatusDelete(
    delphi::objects::DelphiClientStatusPtr obj)
{
    return delphi::error::OK();
}

delphi::error DelphiService::OnDelphiClientStatusUpdate(
    delphi::objects::DelphiClientStatusPtr obj)
{
    this->bus_api->service_heartbeat(obj->key());
    return delphi::error::OK();
}

delphi::error DelphiService::OnSysmgrShutdownReqCreate(
    delphi::objects::SysmgrShutdownReqPtr obj)
{
    this->bus_api->switchroot();
    return delphi::error::OK();
}

delphi::error DelphiService::OnSysmgrShutdownReqDelete(
    delphi::objects::SysmgrShutdownReqPtr obj)
{
    return delphi::error::OK();
}

delphi::error DelphiService::OnSysmgrShutdownReqUpdate(
    delphi::objects::SysmgrShutdownReqPtr obj)
{
    this->bus_api->switchroot();
    return delphi::error::OK();
}

delphi::error DelphiService::OnSysmgrRespawnReqCreate(
    delphi::objects::SysmgrRespawnReqPtr obj)
{
    this->bus_api->respawn_processes();
    return delphi::error::OK();
}

delphi::error DelphiService::OnSysmgrRespawnReqDelete(
    delphi::objects::SysmgrRespawnReqPtr obj)
{
    return delphi::error::OK();
}

delphi::error DelphiService::OnSysmgrRespawnReqUpdate(
    delphi::objects::SysmgrRespawnReqPtr obj)
{
    // glog->info("Respawning processes");
    // ServiceFactory::getInstance()->respawn_all();
    this->bus_api->respawn_processes();
    return delphi::error::OK();
}
