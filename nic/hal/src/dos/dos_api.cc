#include "nic/hal/src/firewall/nwsec.hpp"
#include "nic/include/dos_api.hpp"

namespace hal {

void
dos_set_pd_dos (dos_policy_t *pi_nw, void *pd_nw)
{
    pi_nw->pd = pd_nw;

}

}    // namespace hal
