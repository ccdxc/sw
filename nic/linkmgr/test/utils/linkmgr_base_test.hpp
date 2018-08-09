// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <gtest/gtest.h>
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "sdk/pal.hpp"
#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/linkmgr/linkmgr_src.hpp"
#include "nic/linkmgr/linkmgr_state.hpp"

using sdk::SDK_RET_OK;
using boost::property_tree::ptree;
using sdk::types::platform_type_t;

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

hal_ret_t
linkmgr_parse_cfg (const char *cfgfile, linkmgr::linkmgr_cfg_t *linkmgr_cfg)
{
    ptree             pt;
    std::string       sparam;

    if (!cfgfile) {
        return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_DEBUG("cfg file {}",  cfgfile);

    std::ifstream json_cfg(cfgfile);

    read_json(json_cfg, pt);

    try {
		std::string platform_type = pt.get<std::string>("platform_type");

        linkmgr_cfg->platform_type =
            sdk::lib::catalog::catalog_platform_type_to_platform_type(platform_type);

        linkmgr_cfg->grpc_port = pt.get<std::string>("sw.grpc_port");

        if (getenv("HAL_GRPC_PORT")) {
            linkmgr_cfg->grpc_port = getenv("HAL_GRPC_PORT");
            HAL_TRACE_DEBUG("Overriding GRPC Port to {}", linkmgr_cfg->grpc_port);
        }
    } catch (std::exception const& e) {
        std::cerr << e.what() << std::endl;
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
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

    // makeup the full file path
    cfg_path = std::getenv("HAL_CONFIG_PATH");
    if (cfg_path) {
        cfg_file = std::string(cfg_path) + "/" + cfg_file;
    } else {
        HAL_ASSERT(FALSE);
    }

    linkmgr_parse_cfg(cfg_file.c_str(), &linkmgr_cfg);

    catalog =
        sdk::lib::catalog::factory(std::string(cfg_path) + "/catalog.json");

    if (sdk::lib::pal_init(linkmgr_cfg.platform_type) != sdk::lib::PAL_RET_OK) {
        HAL_TRACE_ERR("pal init failed");
        return;
    }

    sdk_cfg.platform_type = linkmgr_cfg.platform_type;
    sdk_cfg.cfg_path = cfg_path;
    sdk_cfg.catalog  = catalog;

    ret_hal = linkmgr::linkmgr_init(&sdk_cfg);
    if (ret_hal != HAL_RET_OK) {
        HAL_TRACE_ERR("linkmgr init failed");
        return;
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
