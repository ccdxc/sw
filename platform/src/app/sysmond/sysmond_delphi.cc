#include "sysmond_delphi.hpp"

using namespace std;

void SysmondService::init() {
    delphi::objects::SysmgrSystemStatus::Mount(this->sdk, delphi::ReadMode);
    delphi::objects::SysmgrSystemStatus::Watch(this->sdk, shared_from_this());
}

delphi::error SysmondService::OnSysmgrSystemStatusCreate(delphi::objects::SysmgrSystemStatusPtr obj) {
    systemled_t led;
    if (obj->state() == ::sysmgr::Fault) {
        led.event = NON_CRITICAL_EVENT;
        sysmgrsystemled(led);
    }

    return delphi::error::OK();
}

delphi::error SysmondService::OnSysmgrSystemStatusDelete(delphi::objects::SysmgrSystemStatusPtr obj) {
    return delphi::error::OK();
}

delphi::error SysmondService::OnSysmgrSystemStatusUpdate(delphi::objects::SysmgrSystemStatusPtr obj) {
    systemled_t led;
    if (obj->state() == ::sysmgr::Fault) {
        led.event = NON_CRITICAL_EVENT;
        sysmgrsystemled(led);
    }

    return delphi::error::OK();
}

SysmondService::SysmondService(delphi::SdkPtr sdk, string name) {
    this->sdk = sdk;
    this->name = name;
}

void SysmondService::OnMountComplete(){
}

std::string SysmondService::Name() {
    return "sysmond";
}

void SysmondService::SocketClosed() {
    TRACE_INFO(GetLogger(), "SysmondService::Delphi Crashed");
    systemled_t led;
        led.event = CRITICAL_EVENT;
        sysmgrsystemled(led);
}

void *delphi_thread_run(void *ctx)
{
    delphi::SdkPtr sdk = *reinterpret_cast<delphi::SdkPtr*>(ctx);

    TRACE_INFO(GetLogger(), "Starting Delphi Mainloop");
    sdk->MainLoop();
    TRACE_INFO(GetLogger(), "Delphi main loop exited");

    return NULL;
}

