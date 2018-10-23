//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <iostream>
#include "nic/include/base.hpp"
#include "nic/include/trace.hpp"
#include "nic/include/hal.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/iris/delphi/if_svc.hpp"

namespace hal {
namespace svc {

using delphi::error;

// OnPortCreate gets called when InterfaceSpec object is created
error
if_svc::OnInterfaceSpecCreate(dobj::InterfaceSpecPtr spec)
{
    InterfaceResponse           response;
    dobj::InterfaceStatusPtr    status;
    hal_ret_t                   ret;

    // call the HAL handler
    ret = hal::interface_create(*spec.get(), &response);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error creating interface. Err: {}", ret)
        return error::New("Interface create error");
    }

    // update interface status based on response
    status = std::make_shared<dobj::InterfaceStatus>(response.status());
    sdk_->SetObject(status);

    return error::OK();
}

// OnPortUpdate gets called when InterfaceSpec object is updated
error
if_svc::OnInterfaceSpecUpdate(dobj::InterfaceSpecPtr spec)
{
    InterfaceResponse    response;
    dobj::InterfaceStatusPtr status;
    hal_ret_t            ret;

    // call the HAL handler
    ret = hal::interface_update(*spec.get(), &response);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error creating interface. Err: {}", ret)
        return error::New("Interface create error");
    }

    // update interface status based on response
    status = std::make_shared<dobj::InterfaceStatus>(response.status());
    sdk_->SetObject(status);

    return error::OK();
}

// OnPortDelete gets called when InterfaceSpec object is deleted
error if_svc::OnInterfaceSpecDelete(dobj::InterfaceSpecPtr spec) {
    InterfaceDeleteRequest     req;
    InterfaceDeleteResponse    response;
    dobj::InterfaceStatusPtr status;
    hal_ret_t            ret;

    // call the HAL handler
    *(req.mutable_key_or_handle()) = spec->key_or_handle();
    ret = hal::interface_delete(req, &response);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error creating interface. Err: {}", ret)
        return error::New("Interface create error");
    }

    // delete interface status from delphi
    status = std::make_shared<dobj::InterfaceStatus>();
    status->mutable_key_or_handle()->set_interface_id(spec->key_or_handle().interface_id());
    sdk_->DeleteObject(status);

    return error::OK();
}

}    // namespace svc
}    // namespace hal
