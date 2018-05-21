// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_EXAMPLE_HAL_HAL_SVC_H_
#define _DELPHI_EXAMPLE_HAL_HAL_SVC_H_

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "endpoint_svc.hpp"

namespace hal {
namespace svc {

using namespace std;

class HalService : public delphi::Service, public enable_shared_from_this<HalService> {
public:
    HalService(delphi::SdkPtr sk);
    void OnMountComplete();
private:
    delphi::SdkPtr     sdk_;
    EndpointMgrPtr     epmgr_;
    LifMgrPtr          lifmgr_;
    InterfaceMgrPtr    ifmgr_;
};
typedef std::shared_ptr<HalService> HalServicePtr;

} // namespace svc
} // namespace hal

#endif // _DELPHI_EXAMPLE_HAL_HAL_SVC_H_
