#include "delphi_service.hpp"

#include <memory>
#include <string>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/client.delphi.hpp"
#include "gen/proto/sysmgr.delphi.hpp"
#include "gen/proto/eventtypes.pb.h"

#include "eventlogger.hpp"
#include "service_watcher.hpp"
#include "utils.hpp"

DelphiServicePtr DelphiService::create(delphi::SdkPtr sdk)
{
    DelphiServicePtr svc = std::make_shared<DelphiService>();
    svc->sdk = sdk;
    svc->name = "sysmgr";

    delphi::objects::SysmgrProcessStatus::Mount(svc->sdk, delphi::ReadWriteMode);
    delphi::objects::SysmgrSystemStatus::Mount(svc->sdk, delphi::ReadWriteMode);
    delphi::objects::SysmgrServiceStatus::Mount(svc->sdk, delphi::ReadMode);
    delphi::objects::DelphiClientStatus::Mount(svc->sdk, delphi::ReadMode);
    delphi::objects::SysmgrShutdownReq::Mount(svc->sdk, delphi::ReadMode);

    delphi::objects::SysmgrServiceStatus::Watch(svc->sdk, svc);
    delphi::objects::DelphiClientStatus::Watch(svc->sdk, svc);
    delphi::objects::SysmgrShutdownReq::Watch(svc->sdk, svc);

    return svc;
}

void DelphiService::OnMountComplete()
{
    logger->debug("Service delphi started");
    ServiceLoop::getInstance()->queue_event(
	ServiceEvent::create("delphi", SERVICE_EVENT_START));
    EventLogger::getInstance()->LogSystemEvent(
	eventtypes::SYSTEM_COLDBOOT, "System booted");
}

std::string DelphiService::Name()
{
    return "sysmgr";
}

delphi::error DelphiService::OnSysmgrServiceStatusCreate(
    delphi::objects::SysmgrServiceStatusPtr obj)
{
    logger->debug("Service {} started", obj->key());
    ServiceLoop::getInstance()->queue_event(
	ServiceEvent::create(obj->key(), SERVICE_EVENT_START));
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
    logger->debug("Service {} started", obj->key());
    ServiceLoop::getInstance()->queue_event(
	ServiceEvent::create(obj->key(), SERVICE_EVENT_START));
    return delphi::error::OK();
}

delphi::error DelphiService::OnDelphiClientStatusCreate(
    delphi::objects::DelphiClientStatusPtr obj)
{
    logger->debug("Service {} heartbeat", obj->key());
    ServiceLoop::getInstance()->queue_event(
	ServiceEvent::create(obj->key(), SERVICE_EVENT_HEARTBEAT));
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
    logger->debug("Service {} heartbeat", obj->key());
    ServiceLoop::getInstance()->queue_event(
	ServiceEvent::create(obj->key(), SERVICE_EVENT_HEARTBEAT));
    return delphi::error::OK();
}

delphi::error DelphiService::OnSysmgrShutdownReqCreate(
    delphi::objects::SysmgrShutdownReqPtr obj)
{
    logger->info("Switching root");
    switch_root();
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
    logger->info("Switching root");
    switch_root();
    return delphi::error::OK();
}
