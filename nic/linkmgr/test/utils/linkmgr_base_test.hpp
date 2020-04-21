// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <gtest/gtest.h>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "nic/sdk/lib/pal/pal.hpp"
#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/linkmgr/linkmgr_src.hpp"
#include "nic/linkmgr/linkmgr_state.hpp"
#ifdef ELBA
#include "nic/sdk/platform/elba/csrint/csr_init.hpp"
#else
#include "nic/sdk/platform/capri/csrint/csr_init.hpp"
#endif

using sdk::SDK_RET_OK;
using boost::property_tree::ptree;
using sdk::platform::platform_type_t;

namespace linkmgr {
    extern hal_ret_t linkmgr_parse_cfg(const char *cfgfile,
                                       linkmgr::linkmgr_cfg_t *linkmgr_cfg);
    extern hal_ret_t linkmgr_thread_init (void);
}

int
sdk_trace_cb (uint32_t mod_id, sdk_trace_level_e trace_level,
              const char *format, ...)
{
    char       logbuf[128];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    HAL_TRACE_DEBUG_NO_META("{}", logbuf);
    va_end(args);

    return 0;
}

void
linkmgr_initialize (const char c_file[])
{
    hal_ret_t                    ret_hal   = HAL_RET_OK;
    std::string                  cfg_file  = "linkmgr.json";
    char                         *cfg_path = NULL;
    sdk::lib::catalog            *catalog;
    sdk::linkmgr::linkmgr_cfg_t  sdk_cfg;
    linkmgr::linkmgr_cfg_t       linkmgr_cfg;

    // Initialize the logger
    hal::utils::trace_init("linkmgr_gtest",
                           sdk::lib::thread::control_cores_mask(),
                           true, "linkmgr_gtest.log", NULL,
                           TRACE_FILE_SIZE_DEFAULT, TRACE_NUM_FILES_DEFAULT,
                           ::utils::trace_debug,
                           ::utils::trace_none);
    sdk::lib::logger::init(sdk_trace_cb);

    sdk::lib::thread::control_cores_mask_set(0x1);

    // makeup the full file path
    cfg_path = std::getenv("HAL_CONFIG_PATH");
    if (cfg_path) {
        cfg_file = std::string(cfg_path) + "/" + cfg_file;
    } else {
        SDK_ASSERT(FALSE);
    }

    linkmgr::linkmgr_parse_cfg(cfg_file.c_str(), &linkmgr_cfg);

    catalog =
        sdk::lib::catalog::factory(std::string(cfg_path), "/catalog.json");

    if (sdk::lib::pal_init(linkmgr_cfg.platform_type) != sdk::lib::PAL_RET_OK) {
        HAL_TRACE_ERR("pal init failed");
        return;
    }

    linkmgr::linkmgr_thread_init();

    memset(&sdk_cfg, 0, sizeof(sdk::linkmgr::linkmgr_cfg_t));

    sdk_cfg.platform_type = linkmgr_cfg.platform_type;
    sdk_cfg.cfg_path = cfg_path;
    sdk_cfg.catalog  = catalog;

#ifdef ELBA
    sdk::platform::elba::csr_init();
#else
    sdk::platform::capri::csr_init();
#endif

    ret_hal = linkmgr::linkmgr_init(&sdk_cfg);
    if (ret_hal != HAL_RET_OK) {
        HAL_TRACE_ERR("linkmgr init failed");
        return;
    }

    printf("LINKMGR Initialized\n");
}

void
linkmgr_uninitialize (void)
{
    printf("LINKMGR UnInitialized \n");
}

class linkmgr_base_test : public ::testing::Test {
protected:
  linkmgr_base_test() {
  }

  virtual ~linkmgr_base_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

  // Will be called at the beginning of all test cases in this class
  static void SetUpTestCase() {
    linkmgr_initialize("");
  }

  static void SetUpTestCase(const char c_file[]) {
    linkmgr_initialize(c_file);
  }
  // Will be called at the end of all test cases in this class
  static void TearDownTestCase() {
    // linkmgr_uninitialize();
  }
};
