//----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//----------------------------------------------------------------------------
#include "hal_base_test.hpp"

hal::hal_cfg_t  hal::g_hal_cfg;

static void inline
hal_initialize (const char c_file[], bool disable_fte=true)
{
    char                        cfg_file[32];
    char                        def_cfg_file[] = "hal.json";
    std::string                 ini_file = "hal.ini";

    if (disable_fte)
        setenv("DISABLE_FTE", "true", 1);

    bzero(&hal::g_hal_cfg, sizeof(hal::g_hal_cfg));

    if (strlen(c_file) > 0) {
        strcpy(cfg_file, c_file);
    } else {
        strcpy(cfg_file, def_cfg_file);
    }

    if (hal::hal_parse_cfg(cfg_file, &hal::g_hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL config file parsing failed, quitting ...\n");
        ASSERT_TRUE(0);
    }
    printf("Parsed cfg json file \n");

    // set the full path of the catalog file
    hal::g_hal_cfg.catalog_file = hal::g_hal_cfg.cfg_path + "/catalog.json";

    // make sure catalog file exists
    if (access(hal::g_hal_cfg.catalog_file.c_str(), R_OK) < 0) {
        fprintf(stderr, "Catalog file %s has no read permissions\n",
                hal::g_hal_cfg.catalog_file.c_str());
        exit(1);
    }

    // parse the ini
    if (hal::hal_parse_ini(ini_file.c_str(), &hal::g_hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL ini file parsing failed, quitting ...\n");
        exit(1);
    }

    // disabling async logging
    hal::g_hal_cfg.sync_mode_logging = true;

    // initialize HAL
    if (hal::hal_init(&hal::g_hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL initialization failed, quitting ...\n");
        exit(1);
    }
    printf("HAL Initialized\n");
}

static void inline
hal_uninitialize (void)
{
    // uninitialize HAL
    if (hal::hal_destroy() != HAL_RET_OK) {
        fprintf(stderr, "HAL initialization failed, quitting ...\n");
        exit(1);
    }
    printf("HAL UnInitialized \n");
}

void
hal_base_test::SetUpTestCase (void)
{
    hal_initialize("");
}

void
hal_base_test::SetUpTestCase (bool disable_fte)
{
    hal_initialize("", disable_fte);
}

void
hal_base_test::SetUpTestCase (const char c_file[])
{
    hal_initialize(c_file);
}

void
hal_base_test::TearDownTestCase()
{
    hal_uninitialize();
}
