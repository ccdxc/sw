#include <memory>
#include <string>

#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/sdk/lib/ipc/ipc_ev.hpp"
#include "nic/sdk/upgrade/include/ev.hpp"
#include "nic/apollo/include/globals.hpp"
#include "../bus_api.hpp"

class PenIPCBus : public SysmgrBus {
public:
    PenIPCBus(bus_api_t *api);
    virtual void Connect(void) override;
    virtual void SystemFault(std::string reason) override;
    virtual void ProcessDied(std::string name, pid_t pid,
                             std::string reason) override;
    void SwitchRoot(void);
    void RespawnProcesses(void);
private:
    bus_api_t *api_;

};
typedef std::shared_ptr<PenIPCBus> PenIPCBusPtr;

SysmgrBusPtr
init_bus (bus_api_t *api)
{
    return std::make_shared<PenIPCBus>(api);
}

#ifndef SIM
static sdk_ret_t
upg_respawn_cb (sdk::upg::upg_ev_params_t *params)
{
    PenIPCBus *bus = (PenIPCBus *)params->svc_ctx;
    bus->RespawnProcesses();

    return SDK_RET_OK;
}

static sdk_ret_t
upg_switchover_cb (sdk::upg::upg_ev_params_t *params)
{
    PenIPCBus *bus = (PenIPCBus *)params->svc_ctx;
    bus->SwitchRoot();

    return SDK_RET_OK;
}
#endif

PenIPCBus::PenIPCBus(bus_api_t *api) {
    this->api_ = api;

    sdk::ipc::ipc_init_ev_default(PDS_IPC_ID_SYSMGR);

    sdk::upg::upg_ev_t upg_ev = { 0 };

    snprintf(upg_ev.svc_name, SDK_MAX_NAME_LEN, "sysmgr");
    upg_ev.svc_ipc_id = PDS_IPC_ID_SYSMGR;
#ifndef SIM
    upg_ev.switchover_hdlr = upg_switchover_cb;
    upg_ev.respawn_hdlr = upg_respawn_cb;
#endif
    upg_ev.svc_ctx = this;
    sdk::upg::upg_ev_hdlr_register(upg_ev);
}

void
PenIPCBus::SwitchRoot(void) {
    this->api_->switchroot();
}

void
PenIPCBus::RespawnProcesses(void) {
    this->api_->respawn_processes();
}

void
PenIPCBus::Connect(void) {
}

void
PenIPCBus::SystemFault(std::string readon) {
}

void
PenIPCBus::ProcessDied(std::string name, pid_t pid, std::string reason) {
}
