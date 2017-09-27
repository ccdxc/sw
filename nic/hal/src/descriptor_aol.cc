#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_state.hpp"
// #include "nic/hal/svc/descriptor_aol_svc.hpp"
#include "nic/hal/src/tenant.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/descriptor_aol.hpp"



namespace hal {

hal_ret_t
descriptor_aol_get(DescrAolRequest& request, DescrAolSpec *response)
{
    pd::pd_descr_aol_t      pd_descr, *pd_descr_ptr;
    hal_ret_t               ret = HAL_RET_OK;

    pd_descr_ptr = (pd::pd_descr_aol_t*) request.descr_aol_handle();
    ret = pd::pd_descriptor_aol_get(pd_descr_ptr, &pd_descr);

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
