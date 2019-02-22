#include "sysmond.h"
#include "gen/proto/sysmgr.delphi.hpp"
#include "nic/delphi/sdk/delphi_sdk.hpp"

using namespace std;

void *delphi_thread_run(void *ctx);

class SysmondService : public delphi::Service,
                       public enable_shared_from_this<SysmondService>,
                       public delphi::objects::SysmgrSystemStatusReactor {
    private:
        delphi::SdkPtr sdk;
        string name;
    public:
        SysmondService(delphi::SdkPtr sdk, string name);
        void init();
        virtual void OnMountComplete();

        virtual delphi::error OnSysmgrSystemStatusCreate(  
            delphi::objects::SysmgrSystemStatusPtr obj);

        virtual delphi::error OnSysmgrSystemStatusDelete(
            delphi::objects::SysmgrSystemStatusPtr obj);

        virtual delphi::error OnSysmgrSystemStatusUpdate(
            delphi::objects::SysmgrSystemStatusPtr obj);
};
