#include <iostream>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/upgrade/core/stage.hpp"
#include "nic/apollo/upgrade/core/service.hpp"
#include "nic/apollo/upgrade/core/idl.hpp"
#include "nic/apollo/upgrade/include/pds_upgmgr.hpp"

int
main (int argc, char *argv[])
{
    upg::pds_do_upgrade();
    return 0;
}

