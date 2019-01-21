
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <iostream>

#include "nic/sdk/lib/pal/pal.hpp"

int
main(int argc, char *argv[])
{
    // initialize the pal
#ifdef __x86_64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_MOCK) == sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HAPS) == sdk::lib::PAL_RET_OK);
#endif

    std::cout << "QSFP port 1 : " << sdk::lib::pal_is_qsfp_port_present(1) << std::endl;
    std::cout << "QSFP port 2 : " << sdk::lib::pal_is_qsfp_port_present(2) << std::endl;
    return 1;
}
