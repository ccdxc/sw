//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Mock stub init 
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/pdsa_stubs_init.hpp"

namespace pdsa_stub {
int init() 
{
    return 1;
};

bool hal_init(void)
{
    return true;
}

void hal_deinit(void)
{
    return;
}

}
