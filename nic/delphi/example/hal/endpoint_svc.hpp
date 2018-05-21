// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_EXAMPLE_HAL_ENDPOINT_SVC_H_
#define _DELPHI_EXAMPLE_HAL_ENDPOINT_SVC_H_

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/delphi/example/hal/proto/endpoint.delphi.hpp"

namespace hal {
namespace svc {

using namespace std;
using namespace delphi;

// EndpointMgr is the reactor for the EndpointSpec object
class EndpointMgr : public delphi::objects::EndpointSpecReactor {
public:
    EndpointMgr(delphi::SdkPtr sk) {
        this->sdk_ = sk;
    }

    // OnEndpointSpecCreate gets called when EndpointSpec object is created
    virtual error OnEndpointSpecCreate(delphi::objects::EndpointSpecPtr ep);

    // OnEndpointSpecUpdate gets called when EndpointSpec object is updated
    virtual error OnEndpointSpecUpdate(delphi::objects::EndpointSpecPtr ep);

    // OnEndpointSpecDelete gets called when EndpointSpec object is deleted
    virtual error OnEndpointSpecDelete(delphi::objects::EndpointSpecPtr ep);

private:
    delphi::SdkPtr sdk_;
};
typedef std::shared_ptr<EndpointMgr> EndpointMgrPtr;

} // namespace svc
} // namespace hal

#endif // _DELPHI_EXAMPLE_HAL_ENDPOINT_SVC_H_
