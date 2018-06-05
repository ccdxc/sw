//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/nw/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/lif/descriptor_aol.hpp"

namespace hal {

hal_ret_t
descraol_get(DescrAolRequest& request, DescrAolResponseMsg *resp_msg)
{
    pd::pd_descr_aol_t      pd_descr, *pd_descr_ptr;
    hal_ret_t               ret = HAL_RET_OK;
    pd::pd_descriptor_aol_get_args_t args;
    pd::pd_func_args_t          pd_func_args = {0};
    DescrAolSpec *response = resp_msg->add_response();

    pd_descr_ptr = (pd::pd_descr_aol_t*) request.descr_aol_handle();
    if (pd_descr_ptr == 0) {
        return HAL_RET_OK;
    }
    args.src = pd_descr_ptr;
    args.dst = &pd_descr;
    pd_func_args.pd_descriptor_aol_get = &args;
    ret = pd::hal_pd_call(pd::PD_FUNC_ID_DESC_AOL_GET, &pd_func_args);

    response->set_descr_aol_handle(request.descr_aol_handle());
    response->set_address1(pd_descr.a0);
    response->set_offset1(pd_descr.o0);
    response->set_length1(pd_descr.l0);
    response->set_address2(pd_descr.a1);
    response->set_offset2(pd_descr.o1);
    response->set_length2(pd_descr.l1);
    response->set_address3(pd_descr.a2);
    response->set_offset3(pd_descr.o2);
    response->set_length3(pd_descr.l2);

    return ret;
}
}
