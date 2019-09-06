#ifndef _SYSMOND_DELPHI_H_
#define _SYSMOND_DELPHI_H_

#include "gen/proto/sysmond.delphi.hpp"
#include "gen/proto/sysmgr.delphi.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"

using namespace std;

void *delphi_thread_run(void *ctx);
void event_cb_init(void);

class SysmondService : public delphi::Service,
                       public enable_shared_from_this<SysmondService>,
                       public delphi::objects::SysmgrSystemStatusReactor {
    private:
        delphi::SdkPtr sdk;
        string name;
    public:
        SysmondService(delphi::SdkPtr sdk, string name);
        void init();
        void ChangeAsicFrequency();
        virtual void OnMountComplete();

        virtual delphi::error OnSysmgrSystemStatusCreate(
            delphi::objects::SysmgrSystemStatusPtr obj);

        virtual delphi::error OnSysmgrSystemStatusDelete(
            delphi::objects::SysmgrSystemStatusPtr obj);

        virtual delphi::error OnSysmgrSystemStatusUpdate(
            delphi::objects::SysmgrSystemStatusPtr obj);

        virtual void SocketClosed();

        virtual std::string Name();
};
#endif
