//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the mputrace implementation for setting trace
/// registers in ASIC
///
//===----------------------------------------------------------------------===//

#include "third-party/asic/capri/model/utils/cap_csr_py_if.h"
#include "platform/utils/mpart_rsvd.hpp"
#include "platform/capri/csrint/csr_init.hpp"
#include "platform/csr/asicrw_if.hpp"
#include "platform/mputrace/mputrace.hpp"

namespace sdk {
namespace platform {

#define roundup(x, y) ((((x) + ((y)-1)) / (y)) * (y)) /* to any y */
// TODO. Need to read it from memory or file. Temporary fix for SDK compilation
#define MREGION_MPU_TRACE_ADDR                                                 \
    (MREGION_BASE_ADDR + MREGION_MPU_TRACE_START_OFFSET)
#define TRACE_BASE roundup(MREGION_MPU_TRACE_ADDR, 4096)
#define TRACE_END (MREGION_MPU_TRACE_ADDR + MREGION_MPU_TRACE_SIZE)

// Placeholder for the global state in mputrace app
mputrace_global_state_t g_state = {};

void
usage (char *argv[])
{
    std::cerr << "Usage: " << std::endl
              << "       " << argv[0] << " conf <cfg_file>" << std::endl
              << "       " << argv[0] << " dump <out_file>" << std::endl
              << "       " << argv[0] << " show" << std::endl
              << "       " << argv[0] << " reset" << std::endl;
}

static inline int
mputrace_error (char *argv[])
{
    std::cerr << "Invalid operation " << argv[1] << std::endl;
    usage(argv);
    exit(EXIT_FAILURE);
}

static inline void
mputrace_capri_init()
{
#ifdef __x86_64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_SIM) ==
           sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HW) ==
           sdk::lib::PAL_RET_OK);
#endif

    sdk::platform::capri::csr_init();
}

static int
mputrace_handle_options (int argc, char *argv[])
{
    int ret = 0;
    std::map<std::string, int> oper = {
        {"conf", MPUTRACE_CONFIG},
        {"dump", MPUTRACE_DUMP},
        {"reset", MPUTRACE_RESET},
        {"show", MPUTRACE_SHOW},
    };

    g_state.trace_base = TRACE_BASE;
    g_state.trace_end = TRACE_END;
    switch (oper[std::string((const char *)argv[1])]) {
    case MPUTRACE_CONFIG:
        if (argc < MPUTRACE_MAX_ARG) {
            std::cerr << "Specify config filename" << std::endl;
            usage(argv);
            exit(EXIT_FAILURE);
            break;
        }
        mputrace_capri_init();
        mputrace_cfg(argv[2]);
        break;
    case MPUTRACE_DUMP:
        if (argc < MPUTRACE_MAX_ARG) {
            std::cerr << "Specify dump filename" << std::endl;
            usage(argv);
            exit(EXIT_FAILURE);
        }
        mputrace_capri_init();
        mputrace_dump(argv[2]);
        break;
    case MPUTRACE_RESET:
        mputrace_capri_init();
        mputrace_reset();
        break;
    case MPUTRACE_SHOW:
        mputrace_capri_init();
        mputrace_show();
        break;
    default:
        ret = mputrace_error(argv);
    }
    return ret;
}

}    //    end namespace platform
}    //    end namespace sdk

int
main (int argc, char *argv[])
{
    int ret = 0;

    if (argc < MPUTRACE_MAX_ARG - 1) {
        sdk::platform::usage(argv);
        exit(EXIT_FAILURE);
    }

    ret = sdk::platform::mputrace_handle_options(argc, argv);

    return ret;
}
