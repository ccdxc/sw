// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>

void
hal_initialize (const char c_file[])
{
    char 			cfg_file[32];
    char 			def_cfg_file[] = "hal.json";
    std::string     ini_file = "hal.ini";
    hal::hal_cfg_t  hal_cfg;

    bzero(&hal_cfg, sizeof(hal_cfg));

    if (strlen(c_file) > 0) {
        strcpy(cfg_file, c_file);
    } else {
        strcpy(cfg_file, def_cfg_file);
    }

    if (hal::hal_parse_cfg(cfg_file, &hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL config file parsing failed, quitting ...\n");
        ASSERT_TRUE(0);
    }
    printf("Parsed cfg json file \n");

    // set the full path of the catalog file
    hal_cfg.catalog_file = hal_cfg.cfg_path + "/catalog.json";

    // make sure catalog file exists
    if (access(hal_cfg.catalog_file.c_str(), R_OK) < 0) {
        fprintf(stderr, "Catalog file %s has no read permissions\n",
                hal_cfg.catalog_file.c_str());
        exit(1);
    }

    // parse the ini
    if (hal::hal_parse_ini(ini_file.c_str(), &hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL ini file parsing failed, quitting ...\n");
        exit(1);
    }

    // disabling async logging
    hal_cfg.sync_mode_logging = true;

    // initialize HAL
    if (hal::hal_init(&hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL initialization failed, quitting ...\n");
        exit(1);
    }
    printf("HAL Initialized\n");
}

void
hal_uninitialize (void)
{
    // uninitialize HAL
    if (hal::hal_destroy() != HAL_RET_OK) {
        fprintf(stderr, "HAL initialization failed, quitting ...\n");
        exit(1);
    }
    printf("HAL UnInitialized \n");
}

class hal_base_test : public ::testing::Test {
protected:
  hal_base_test() {
  }

  virtual ~hal_base_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

  // Will be called at the beginning of all test cases in this class
  static void SetUpTestCase() {
    hal_initialize("");
  }

  static void SetUpTestCase(const char c_file[]) {
    hal_initialize(c_file);
  }
  // Will be called at the end of all test cases in this class
  static void TearDownTestCase() {
    // hal_uninitialize();
  }
};
