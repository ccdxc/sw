#ifndef __SYSMGR_CLIENT_HPP__
#define __SYSMGR_CLIENT_HPP__

#include <memory>
#include <string>
#include <vector>

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "gen/proto/sysmgr.delphi.hpp"

namespace sysmgr
{
    class ServiceStatusReactor {
    public:
        virtual void ServiceUp(std::string name) = 0;
        virtual void ServiceDown(std::string name) = 0;
    };
    typedef std::shared_ptr<ServiceStatusReactor> ServiceStatusReactorPtr;
  
    class Client : public std::enable_shared_from_this<Client>,
                   public delphi::objects::SysmgrServiceStatusReactor {
    public:
        Client(delphi::SdkPtr delphi, std::string name);
        virtual void OnMountComplete();
        void register_mounts();
        void init_done();
        void test_done();
        void register_service_reactor(std::string name, ServiceStatusReactorPtr rctor);
        void restart_system();
        void respawn_processes();
        virtual delphi::error OnSysmgrServiceStatusCreate(delphi::objects::SysmgrServiceStatusPtr obj);
        virtual delphi::error OnSysmgrServiceStatusUpdate(delphi::objects::SysmgrServiceStatusPtr obj);
    private:
        delphi::SdkPtr delphi;
        const std::string name;
        std::map<std::string, ServiceStatusReactorPtr> reactors;
        void service_up(std::string name);
    };
    typedef std::shared_ptr<Client> ClientPtr;

    ClientPtr CreateClient(delphi::SdkPtr delphi, std::string name);
} // sysmgr


#endif
