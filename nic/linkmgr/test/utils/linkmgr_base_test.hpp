// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <stdio.h>
#include <stdlib.h>
#include <gtest/gtest.h>
#include "sdk/pal.hpp"
#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/linkmgr/linkmgr_src.hpp"
#include "nic/linkmgr/linkmgr_state.hpp"

using sdk::SDK_RET_OK;
using linkmgr::g_linkmgr_state;

int
sdk_error_logger (const char *format, ...)
{
    char       logbuf[128];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    HAL_TRACE_ERR_NO_META("{}", logbuf);
    va_end(args);

    return 0;
}

int
sdk_debug_logger (const char *format, ...)
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
    sdk_ret_t                     sdk_ret   = SDK_RET_OK;
    std::string                   cfg_file  = "linkmgr.json";
    char                          *cfg_path = NULL;
    sdk::linkmgr::linkmgr_cfg_t   sdk_cfg;
    sdk::lib::catalog             *catalog;
    linkmgr::linkmgr_cfg_t        linkmgr_cfg;

    sdk::lib::thread::control_cores_mask_set(0x1);

    // Initialize the logger
    hal::utils::trace_init("linkmgr", sdk::lib::thread::control_cores_mask(),
                           true, "linkmgr.log", hal::utils::trace_debug);

    sdk::lib::logger::init(sdk_error_logger, sdk_debug_logger);

    // makeup the full file path
    cfg_path = std::getenv("HAL_CONFIG_PATH");
    if (cfg_path) {
        cfg_file = std::string(cfg_path) + "/" + cfg_file;
    } else {
        HAL_ASSERT(FALSE);
    }

    linkmgr::linkmgr_parse_cfg(cfg_file.c_str(), &linkmgr_cfg);

    g_linkmgr_state = linkmgr::linkmgr_state::factory();
    if (g_linkmgr_state == NULL) {
        fprintf(stderr, "%s: Failed to create linkmgr state\n", __FUNCTION__);
        ASSERT_TRUE(0);
    }

    catalog =
        sdk::lib::catalog::factory(std::string(cfg_path) + "/catalog.json");
    if (catalog == NULL) {
        fprintf(stderr, "%s: catalog init failed\n", __FUNCTION__);
        ASSERT_TRUE(0);
    }

    // store the catalog in global hal state
    g_linkmgr_state->set_catalog(catalog);
    sdk_cfg.platform_type = linkmgr_cfg.platform_type;
    sdk_cfg.hw_mock = linkmgr_cfg.hw_mock;

    sdk_ret = sdk::linkmgr::linkmgr_init(&sdk_cfg);
    if (sdk_ret != SDK_RET_OK) {
        fprintf(stderr, "%s: linkmgr init failed", __FUNCTION__);
        ASSERT_TRUE(0);
    }

    if (sdk::lib::pal_init(linkmgr_cfg.platform_type) != sdk::lib::PAL_RET_OK) {
        fprintf(stderr, "%s: pal init failed\n", __FUNCTION__);
        ASSERT_TRUE(0);
    }

    // start the linkmgr control thread
    sdk::linkmgr::linkmgr_event_wait();

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
