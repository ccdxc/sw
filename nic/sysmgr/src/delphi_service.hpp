#ifndef __DELPHI_SERVICE_HPP__
#define __DELPHI_SERVICE_HPP__

#include <memory>
#include <string>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/client.delphi.hpp"
#include "gen/proto/sysmgr.delphi.hpp"

class DelphiService : public delphi::Service,
		      public delphi::objects::SysmgrServiceStatusReactor,
		      public delphi::objects::DelphiClientStatusReactor,
		      public delphi::objects::SysmgrShutdownReqReactor
{
  private:
    delphi::SdkPtr sdk;
    std::string name;
  public:
    static std::shared_ptr<DelphiService> create(delphi::SdkPtr sdk);
    virtual void OnMountComplete();
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
};
typedef std::shared_ptr<DelphiService> DelphiServicePtr;

#endif //__DELPHI_SERVICE_HPP__
