//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/plugins/sfw/cfg/nwsec.hpp"
#include "nic/hal/src/export/nwsec_api.hpp"

namespace hal {

void
nwsec_set_pd_nwsec (nwsec_profile_t *pi_nw, void *pd_nw)
{
    pi_nw->pd = pd_nw;

}

} // namespace hal
