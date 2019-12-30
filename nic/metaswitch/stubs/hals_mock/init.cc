//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Mock stub init 
//---------------------------------------------------------------

#include "nic/metaswitch/stubs/pds_ms_stubs_init.hpp"

namespace pds_ms {
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
