//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include "nic/apollo/upgrade/svc/upgrade.hpp"
#include "nic/apollo/upgrade/logger.hpp"

namespace upg {

sdk::operd::logger_ptr g_upg_log = sdk::operd::logger::create(UPG_LOG_NAME);

}    // namespace upg

static void inline
print_usage (char **argv)
{
    fprintf(stdout, "Usage : ");
}

int
main (int argc, char **argv)
{
    print_usage(argv);
}

