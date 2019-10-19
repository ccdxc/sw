#ifndef __DELPHI_SERVICE_HPP__
#define __DELPHI_SERVICE_HPP__

#include <memory>
#include <string>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/client.delphi.hpp"
#include "gen/proto/sysmgr.delphi.hpp"

#include "../bus_api.hpp"

class DelphiService : public delphi::Service,
                      public SysmgrBus,
                      public delphi::objects::SysmgrServiceStatusReactor,
                      public delphi::objects::DelphiClientStatusReactor,
                      public delphi::objects::SysmgrShutdownReqReactor,
                      public delphi::objects::SysmgrRespawnReqReactor
{
  private:
    delphi::SdkPtr sdk;
    std::string    name;
    bus_api_t      *bus_api;
  public:
    static std::shared_ptr<DelphiService> create(delphi::SdkPtr sdk,
                                                 bus_api_t *api);
    virtual void Connect(void);
    virtual void SystemFault(std::string reason);
    virtual void ProcessDied(std::string name, pid_t pid, std::string reason);
    virtual void OnMountComplete();
    virtual std::string Name();
    // SysmgrServiceStatusReactor
    virtual delphi::error OnSysmgrServiceStatusCreate(
        delphi::objects::SysmgrServiceStatusPtr obj);
    virtual delphi::error OnSysmgrServiceStatusDelete(
        delphi::objects::SysmgrServiceStatusPtr obj);        
    virtual delphi::error OnSysmgrServiceStatusUpdate(
        delphi::objects::SysmgrServiceStatusPtr obj);
    // DelphiClientStatusReactor
    virtual delphi::error OnDelphiClientStatusCreate(
        delphi::objects::DelphiClientStatusPtr obj);
    virtual delphi::error OnDelphiClientStatusDelete(
        delphi::objects::DelphiClientStatusPtr obj);
    virtual delphi::error OnDelphiClientStatusUpdate(
        delphi::objects::DelphiClientStatusPtr obj);
    // SysmgrShutdownReqReactor
    virtual delphi::error OnSysmgrShutdownReqCreate(
        delphi::objects::SysmgrShutdownReqPtr obj);
    virtual delphi::error OnSysmgrShutdownReqDelete(
        delphi::objects::SysmgrShutdownReqPtr obj);
    virtual delphi::error OnSysmgrShutdownReqUpdate(
        delphi::objects::SysmgrShutdownReqPtr obj);
    // SysmgrRespawnReqReactor
    virtual delphi::error OnSysmgrRespawnReqCreate(
        delphi::objects::SysmgrRespawnReqPtr obj);
    virtual delphi::error OnSysmgrRespawnReqDelete(
        delphi::objects::SysmgrRespawnReqPtr obj);
    virtual delphi::error OnSysmgrRespawnReqUpdate(
        delphi::objects::SysmgrRespawnReqPtr obj);
};
typedef std::shared_ptr<DelphiService> DelphiServicePtr;

#endif //__DELPHI_SERVICE_HPP__
