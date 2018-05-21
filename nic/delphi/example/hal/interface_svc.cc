// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "interface_svc.hpp"

namespace hal {
namespace svc {

using namespace std;
using namespace delphi;

// OnInterfaceSpecCreate gets called when InterfaceSpec object is created
error InterfaceMgr::OnInterfaceSpecCreate(delphi::objects::InterfaceSpecPtr intf) {
    delphi::objects::InterfaceStatusPtr intf_status = make_shared<delphi::objects::InterfaceStatus>();

#if 0
    InterfaceSpec     spec = *intf.get();
    InterfaceResponse resp;
    hal_ret_t   ret;

    // open the db
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);

    // create interface in hal
    ret = hal::interface_create(spec, &resp);
    if (ret != HAL_RET_OK) {
        LogError("Error creating interface. Err {}", ret);
    }

    // close the db
    hal::hal_cfg_db_close();

#endif
    // publish interface status
    sdk_->SetObject(intf_status);

    return error::OK();
}

// OnInterfaceSpecUpdate gets called when InterfaceSpec object is updated
error InterfaceMgr::OnInterfaceSpecUpdate(delphi::objects::InterfaceSpecPtr intf) {
    delphi::objects::InterfaceStatusPtr intf_status = make_shared<delphi::objects::InterfaceStatus>();

#if 0
    InterfaceSpec     spec = *intf.get();
    InterfaceResponse resp;
    hal_ret_t   ret;

    // open the db
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);

    // update interface in hal
    ret = hal::interface_update(spec, &resp);
    if (ret != HAL_RET_OK) {
        LogError("Error creating interface. Err {}", ret);
    }

    // close the db
    hal::hal_cfg_db_close();

#endif
    // publish interface status
    sdk_->SetObject(intf_status);

    return error::OK();
}

// OnInterfaceSpecDelete gets called when InterfaceSpec object is deleted
error InterfaceMgr::OnInterfaceSpecDelete(delphi::objects::InterfaceSpecPtr intf) {
    delphi::objects::InterfaceStatusPtr intf_status = make_shared<delphi::objects::InterfaceStatus>();

#if 0
    InterfaceSpec     spec = *intf.get();
    InterfaceResponse resp;
    hal_ret_t   ret;

    // open the db
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);

    // delete interface in hal
    ret = hal::interface_delete(spec, &resp);
    if (ret != HAL_RET_OK) {
        LogError("Error creating interface. Err {}", ret);
    }

    // close the db
    hal::hal_cfg_db_close();

#endif
    // publish interface status
    sdk_->DeleteObject(intf_status);

    return error::OK();
}

} // namespace svc
} // namespace hal
