//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Initialize Stubs that drive the dataplane
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/hals/pdsa_hal_init.hpp"
#include "nic/metaswitch/stubs/hals/pdsa_li.hpp"
#include "nic/metaswitch/stubs/hals/pdsa_l2f.hpp"

namespace pdsa_stub {

bool hal_init(void)
{
    static pdsa_stub::LiIntegSubcomponent g_pdsa_li;
    /* LI stub implementation instance needs to be registered with Metaswitch*/
    li_pen::is_initialize(&g_pdsa_li);
    return true;
}

void hal_deinit(void)
{
}

}
