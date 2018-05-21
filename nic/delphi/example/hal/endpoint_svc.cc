// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "endpoint_svc.hpp"

namespace hal {
namespace svc {

using namespace std;
using namespace delphi;

// OnEndpointSpecCreate gets called when EndpointSpec object is created
error EndpointMgr::OnEndpointSpecCreate(delphi::objects::EndpointSpecPtr ep) {
    delphi::objects::EndpointStatusPtr ep_status = make_shared<delphi::objects::EndpointStatus>();

#if 0
    EndpointSpec     spec = *ep.get();
    EndpointResponse resp;
    hal_ret_t   ret;

    // open the db
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);

    // create endpoint in hal
    ret = hal::endpoint_create(spec, &resp);
    if (ret != HAL_RET_OK) {
        LogError("Error creating endpoint. Err {}", ret);
    }

    // close the db
    hal::hal_cfg_db_close();

#endif
    // publish endpoint status
    sdk_->SetObject(ep_status);

    return error::OK();
}

// OnEndpointSpecUpdate gets called when EndpointSpec object is updated
error EndpointMgr::OnEndpointSpecUpdate(delphi::objects::EndpointSpecPtr ep) {
    delphi::objects::EndpointStatusPtr ep_status = make_shared<delphi::objects::EndpointStatus>();

#if 0
    EndpointSpec     spec = *ep.get();
    EndpointResponse resp;
    hal_ret_t   ret;

    // open the db
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);

    // update endpoint in hal
    ret = hal::endpoint_update(spec, &resp);
    if (ret != HAL_RET_OK) {
        LogError("Error creating endpoint. Err {}", ret);
    }

    // close the db
    hal::hal_cfg_db_close();

#endif
    // publish endpoint status
    sdk_->SetObject(ep_status);

    return error::OK();
}

// OnEndpointSpecDelete gets called when EndpointSpec object is deleted
error EndpointMgr::OnEndpointSpecDelete(delphi::objects::EndpointSpecPtr ep) {
    delphi::objects::EndpointStatusPtr ep_status = make_shared<delphi::objects::EndpointStatus>();

#if 0
    EndpointSpec     spec = *ep.get();
    EndpointResponse resp;
    hal_ret_t   ret;

    // open the db
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);

    // delete endpoint in hal
    ret = hal::endpoint_delete(spec, &resp);
    if (ret != HAL_RET_OK) {
        LogError("Error creating endpoint. Err {}", ret);
    }

    // close the db
    hal::hal_cfg_db_close();

#endif
    // publish endpoint status
    sdk_->DeleteObject(ep_status);

    return error::OK();
}

} // namespace svc
} // namespace hal
