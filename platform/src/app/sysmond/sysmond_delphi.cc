#include "sysmond_delphi.hpp"

using namespace std;

void SysmondService::init() {
    delphi::objects::SysmgrSystemStatus::Mount(this->sdk, delphi::ReadMode);
    delphi::objects::SysmgrSystemStatus::Watch(this->sdk, shared_from_this());
}

delphi::error SysmondService::OnSysmgrSystemStatusCreate(delphi::objects::SysmgrSystemStatusPtr obj) {
    if (obj->state() == ::sysmgr::Fault) {
        pal_system_set_led(LED_COLOR_YELLOW, LED_FREQUENCY_0HZ);
    }

    return delphi::error::OK();
}

delphi::error SysmondService::OnSysmgrSystemStatusDelete(delphi::objects::SysmgrSystemStatusPtr obj) {
    return delphi::error::OK();
}

delphi::error SysmondService::OnSysmgrSystemStatusUpdate(delphi::objects::SysmgrSystemStatusPtr obj) {
    if (obj->state() == ::sysmgr::Fault) {
        pal_system_set_led(LED_COLOR_YELLOW, LED_FREQUENCY_0HZ);
    }

    return delphi::error::OK();
}

SysmondService::SysmondService(delphi::SdkPtr sdk, string name) {
    this->sdk = sdk;
    this->name = name;
}

void SysmondService::OnMountComplete(){
}

void *delphi_thread_run(void *ctx)
{
    delphi::SdkPtr sdk = *reinterpret_cast<delphi::SdkPtr*>(ctx);

    TRACE_INFO(GetLogger(), "Starting Delphi Mainloop");
    sdk->MainLoop();
    TRACE_INFO(GetLogger(), "Delphi main loop exited");

    return NULL;
}
