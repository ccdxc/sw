// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#ifndef _DELPHI_EXAMPLE_HAL_LIF_SVC_H_
#define _DELPHI_EXAMPLE_HAL_LIF_SVC_H_

#include "nic/delphi/sdk/delphi_sdk.hpp"
#include "nic/delphi/example/hal/proto/interface.delphi.hpp"

namespace hal {
namespace svc {

using namespace std;
using namespace delphi;

// LifMgr is the reactor for the LifSpec object
class LifMgr : public delphi::objects::LifSpecReactor {
public:
    LifMgr(delphi::SdkPtr sk) {
        this->sdk_ = sk;
    }

    // OnLifSpecCreate gets called when LifSpec object is created
    virtual error OnLifSpecCreate(delphi::objects::LifSpecPtr ep);

    // OnLifSpecUpdate gets called when LifSpec object is updated
    virtual error OnLifSpecUpdate(delphi::objects::LifSpecPtr ep);

    // OnLifSpecDelete gets called when LifSpec object is deleted
    virtual error OnLifSpecDelete(delphi::objects::LifSpecPtr ep);

private:
    delphi::SdkPtr sdk_;
};
typedef std::shared_ptr<LifMgr> LifMgrPtr;

} // namespace svc
} // namespace hal

#endif // _DELPHI_EXAMPLE_HAL_LIF_SVC_H_
