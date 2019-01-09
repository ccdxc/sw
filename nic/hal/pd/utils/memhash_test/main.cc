//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include <gtest/gtest.h>
#include <stdio.h>
#include "nic/sdk/lib/table/memhash/mem_hash.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "gen/p4gen/p4/include/p4pd.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <chrono>
#include <fstream>
#include "nic/hal/pd/utils/flow/test/jenkins_spooky/spooky.h"
#include "include/sdk/base.hpp"
#include "nic/include/hal_mem.hpp"
#include <arpa/inet.h>

using sdk::table::mem_hash;
using boost::multiprecision::uint512_t;
using boost::multiprecision::uint128_t;
using namespace std::chrono;

static int
memhash_debug_logger (sdk_trace_level_e trace_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    HAL_TRACE_DEBUG_NO_META("{}", logbuf);
    va_end(args);

    return 0;
}

//------------------------------------------------------------------------------
// SDK initiaization
//------------------------------------------------------------------------------
hal_ret_t
memhash_sdk_init (void)
{
    sdk::lib::logger::init(memhash_debug_logger);
    return HAL_RET_OK;
}

int 
main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    std::string logfile;
    logfile = std::string("./hal.log");
    hal::utils::trace_init("hal", 0x3, true,
                           logfile.c_str(),
                           TRACE_FILE_SIZE_DEFAULT,
                           TRACE_NUM_FILES_DEFAULT,
                           ::utils::trace_debug);

    memhash_sdk_init();

    HAL_TRACE_DEBUG("Starting Main ... ");

    p4pd_cfg_t                 p4pd_cfg = {
        .table_map_cfg_file  = "iris/capri_p4_table_map.json",
        .p4pd_pgm_name       = "iris",
        .p4pd_rxdma_pgm_name = "p4plus",
        .p4pd_txdma_pgm_name = "p4plus",
        .cfg_path            = std::getenv("HAL_CONFIG_PATH"),
    };
    p4pd_init(&p4pd_cfg);

    // testing::GTEST_FLAG(filter) = "-*test18*";
    int res = RUN_ALL_TESTS();
    p4pd_cleanup();
    return res;
}
