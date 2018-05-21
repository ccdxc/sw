// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

#include <stdio.h>
#include <iostream>

#include "lif_svc.hpp"

namespace hal {
namespace svc {

using namespace std;
using namespace delphi;


// OnLifSpecCreate gets called when LifSpec object is created
error LifMgr::OnLifSpecCreate(delphi::objects::LifSpecPtr lif) {
    delphi::objects::LifStatusPtr lif_status = make_shared<delphi::objects::LifStatus>();

#if 0
    LifSpec     spec = *lif.get();
    LifResponse resp;
    hal_ret_t   ret;

    // open the db
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);

    // create LIF in hal
    ret = hal::lif_create(spec, &resp);
    if (ret != HAL_RET_OK) {
        LogError("Error creating lif. Err {}", ret);
    }

    // close the db
    hal::hal_cfg_db_close();

#endif
    // publish Lif status
    sdk_->SetObject(lif_status);

    return error::OK();
}

// OnLifSpecUpdate gets called when LifSpec object is updated
error LifMgr::OnLifSpecUpdate(delphi::objects::LifSpecPtr lif) {
    delphi::objects::LifStatusPtr lif_status = make_shared<delphi::objects::LifStatus>();

#if 0
    LifSpec     spec = *lif.get();
    LifResponse resp;
    hal_ret_t   ret;

    // open the db
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);

    // update LIF in hal
    ret = hal::lif_update(spec, &resp);
    if (ret != HAL_RET_OK) {
        LogError("Error creating lif. Err {}", ret);
    }

    // close the db
    hal::hal_cfg_db_close();

#endif
    // publish Lif status
    sdk_->SetObject(lif_status);

    return error::OK();
}

// OnLifSpecDelete gets called when LifSpec object is deleted
error LifMgr::OnLifSpecDelete(delphi::objects::LifSpecPtr lif) {
    delphi::objects::LifStatusPtr lif_status = make_shared<delphi::objects::LifStatus>();

#if 0
    LifSpec     spec = *lif.get();
    LifResponse resp;
    hal_ret_t   ret;

    // open the db
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);

    // delete LIF in hal
    ret = hal::lif_delete(spec, &resp);
    if (ret != HAL_RET_OK) {
        LogError("Error creating lif. Err {}", ret);
    }

    // close the db
    hal::hal_cfg_db_close();

#endif
    // publish Lif status
    sdk_->DeleteObject(lif_status);

    return error::OK();
}

} // namespace svc
} // namespace hal
