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

#include "platform/mputrace/mputrace.hpp"
#include "platform/capri/csr/asicrw_if.hpp"
#include "nic/asic/capri/model/utils/cap_csr_py_if.h"
// TODO check if we need to get it from a common location
#include "gen/platform/mem_regions.hpp"

namespace sdk {
namespace platform {

sdk::types::mem_addr_t g_trace_base;
sdk::types::mem_addr_t g_trace_end;
#define roundup(x, y) ((((x) + ((y)-1)) / (y)) * (y)) /* to any y */
#define TRACE_BASE roundup(MEM_REGION_ADDR(MPU_TRACE), 4096)
#define TRACE_END  (MEM_REGION_ADDR(MPU_TRACE) + \
                         MEM_REGION_MPU_TRACE_SIZE )

std::map<std::string, int> pipeline_str_to_id = {
    {"txdma", TXDMA},
    {"rxdma", RXDMA},
    {"p4ig", P4IG},
    {"p4eg", P4EG},
};

std::map<int, int> max_stages = {{TXDMA, 7}, {RXDMA, 7}, {P4IG, 5}, {P4EG, 5}};

std::map<std::string, int> trace_options = {
    {"trace_enable", MPUTRACE_TRACE_ENABLE},
    {"phv_debug", MPUTRACE_PHV_DEBUG},
    {"phv_error", MPUTRACE_PHV_ERROR},
    {"table_key_enable", MPUTRACE_TBL_KEY_ENABLE},
    {"instr_enable", MPUTRACE_INSTR_ENABLE},
    {"wrap", MPUTRACE_WRAP},
    {"watch_pc", MPUTRACE_WATCH_PC},
    {"trace_size", MPUTRACE_TRACE_SIZE},
};

char g_MPUTRACE_DUMP_FILE[MPUTRACE_STR_NAME_LEN] = {};
char *g_line;
int g_line_num = -1;
int g_col_num;
int g_pipeline;
int g_stage;
int g_mpu;
char *g_pipeline_name;
mputrace_instance_t *record;

static void
mputrace_register()
{
    // register hal cpu interface
    auto cpu_if = new cpu_hal_if("cpu", "all");
    cpu::access()->add_if("cpu_if", cpu_if);
    cpu::access()->set_cur_if_name("cpu_if");

    // Register at top level all MRL classes.
    cap_top_csr_t *cap0_ptr = new cap_top_csr_t("cap0");

    cap0_ptr->init(0);
    CAP_BLK_REG_MODEL_REGISTER(cap_top_csr_t, 0, 0, cap0_ptr);
    register_chip_inst("cap0", 0, 0);
}

void
parse_options(mputrace_instance_t *record)
{
    char *option, *value;

    assert(record != NULL);

    record->enable = false;
    record->trace_enable = false;
    record->phv_debug = false;
    record->phv_error = false;
    record->table_key = false;
    record->instructions = false;
    record->wrap = false;
    record->watch_pc = 0;
    record->trace_addr = 0;
    record->trace_size = 4096;

    // Parse and validate options
    while (1) {

        option = strtok(NULL, "=");
        if (option == NULL) {
            break;
        }

        value = strtok(NULL, ",");
        if (value == NULL) {
            break;
        }

        switch (trace_options[std::string((const char *)option)]) {
            case MPUTRACE_NONE:
                printf("Invalid option '%s' ... ignoring!\n", option);
                break;
            case MPUTRACE_TRACE_ENABLE:
                record->trace_enable = atoi(value);
                break;
            case MPUTRACE_PHV_DEBUG:
                record->phv_debug = atoi(value);
                break;
            case MPUTRACE_PHV_ERROR:
                record->phv_error = atoi(value);
                break;
            case MPUTRACE_TBL_KEY_ENABLE:
                record->table_key = atoi(value);
                break;
            case MPUTRACE_INSTR_ENABLE:
                record->instructions = atoi(value);
                break;
            case MPUTRACE_WRAP:
                record->wrap = atoi(value);
                break;
            case MPUTRACE_WATCH_PC:
                record->watch_pc = strtoul(value, NULL, 16);
                break;
            case MPUTRACE_TRACE_SIZE:
                record->trace_size = atoi(value);
                assert(record->trace_size > 0);
                break;
            default:
                printf("Invalid option '%s' ... ignoring!\n", option);
                break;
        }
    }

    uint8_t trace_all = !(record->trace_enable ||
                            record->phv_debug ||
                            record->phv_error ||
                            record->watch_pc);
    if (!trace_all && record->enable) {
        printf("Invalid configuration.\n"
               "enable option overrides these options - "
               "trace_enable, phv_debug, phv_error, watch_pc\n");
    }

    record->enable = trace_all;

    return;
}

/// Parse and validate pipeline
static inline void
mputrace_parse_pipeline()
{
    char *token;

    token = strtok(g_line, ",");
    if (token == NULL) {
        std::cerr << "Truncated line " << g_line_num << " col " << g_col_num
                  << std::endl;
        exit(EXIT_FAILURE);
    } else if (strncmp(token, "*", 1) == 0) {
        g_pipeline_name = token;
        g_pipeline = MPUTRACE_PIPELINE_NONE;
    } else {
        g_pipeline_name = token;
        if (pipeline_str_to_id.find(std::string(
                (const char *)g_pipeline_name)) == pipeline_str_to_id.end()) {
            std::cout << "Invalid pipeline " << g_pipeline_name << " on line "
                      << g_line_num << " col " << g_col_num << std::endl;
            exit(EXIT_FAILURE);
        }
        g_pipeline = pipeline_str_to_id[std::string((const char *)token)];
    }
    g_col_num++;
}

/// Parse and validate stage
static inline void
mputrace_parse_stage()
{
    char *token;

    token = strtok(NULL, ",");
    if (token == NULL) {
        std::cerr << "Truncated line " << g_line_num << " col " << g_col_num
                  << std::endl;
        exit(EXIT_FAILURE);
    } else if (strncmp(token, "*", 1) == 0) {
        g_stage = -1; // all stages
    } else {
        g_stage = atoi(token);
        if (g_stage > max_stages[g_pipeline]) {
            std::cout << "Invalid stage " << g_stage << " for pipeline "
                      << g_pipeline_name << " on line " << g_line_num << " col "
                      << g_col_num << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    g_col_num++;
}

/// Parse and validate mpu
static inline void
mputrace_parse_mpu()
{
    char *token;

    token = strtok(NULL, ",");
    if (token == NULL) {
        std::cerr << "Truncated line " << g_line_num << " col " << g_col_num
                  << std::endl;
        exit(EXIT_FAILURE);
    } else if (strncmp(token, "*", 1) == 0) {
        g_mpu = -1; // all mpus
    } else {
        g_mpu = atoi(token);
        if (g_mpu > MPUTRACE_MAX_MPU) {
            std::cout << "Invalid mpu " << g_mpu << " for stage " << g_stage
                      << " pipeline " << g_pipeline_name << " on line "
                      << g_line_num << " col " << g_col_num << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    g_col_num++;
}

///
/// Parse and validate options. Do a reset first.
/// Call config to program registers for each MPU
///
static inline void
mputrace_parse_options()
{
    g_trace_base = TRACE_BASE;
    g_trace_end = TRACE_END;
    bool enable;
    if (record == NULL) {
        record = (mputrace_instance_t *)malloc(sizeof(mputrace_instance_t));
    }
    memset(record, 0, sizeof(mputrace_instance_t));
    parse_options(record);

    for (int p = 0; p < PIPE_CNT; p++) {
        for (int s = 0; s <= max_stages[p]; s++) {
            for (int m = 0; m < MPUTRACE_MAX_MPU; m++) {
                enable = (g_pipeline == (int)MPUTRACE_PIPELINE_NONE ||
                          p == g_pipeline) &&
                         (g_stage == -1 || s == g_stage) &&
                         (g_mpu == -1 || m == g_mpu);
                if (enable) {
                    mputrace_config_trace(p, s, m, record);
                }
            }
        }
    }
}

static inline void
mputrace_parse_conf_file(const char *cfg_file)
{
    FILE *fp = NULL;
    size_t len = 0;

    fp = fopen(cfg_file, "r");
    if (fp == NULL) {
        std::cerr << "Failed to load config file" << std::endl;
        exit(EXIT_FAILURE);
    }

    while ((getline(&g_line, &len, fp)) != -1) {
        ++g_line_num;
        g_col_num = 0;
        if (g_line[0] == '#') { // comment
            continue;
        }
        mputrace_parse_pipeline();
        mputrace_parse_stage();
        mputrace_parse_mpu();
        mputrace_parse_options();
        g_line_num++;
    }
    fclose(fp);
    if (g_line) {
        MPUTRACE_FREE(g_line);
    }
    if (record) {
        MPUTRACE_FREE(record);
    }
}

void
mpu_trace_config(const char *cfg_file)
{
    mputrace_parse_conf_file(cfg_file);
    printf("mputrace conf success\n");
}

/// Reset the trace buffer
static inline void
mpu_trace_reset()
{
    mputrace_reset();
    printf("mputrace reset success\n");
}

static inline void
mpu_trace_dump(const char *dump_file)
{
    snprintf(g_MPUTRACE_DUMP_FILE, MPUTRACE_STR_NAME_LEN, "%s", dump_file);
    mputrace_dump();
    printf("mputrace dump success\n");
}

static inline void
mpu_trace_show()
{
    printf("%10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s "
           "%10s %10s\n",
           "pipeline", "stage", "mpu", "enable", "wrap", "reset", "trace",
           "phv_debug", "phv_error", "watch_pc", "table_kd", "instr",
           "trace_addr", "trace_nent", "trace_sz");
    mputrace_show();
    printf("mputrace show success\n");
}

void
usage(char *argv[])
{
    std::cerr << "Usage: " << std::endl
              << "       " << argv[0] << " conf <cfg_file>" << std::endl
              << "       " << argv[0] << " dump <out_file>" << std::endl
              << "       " << argv[0] << " show" << std::endl
              << "       " << argv[0] << " reset" << std::endl;
}

static inline int
mputrace_error(char *argv[])
{
    std::cerr << "Invalid operation " << argv[1] << std::endl;
    usage(argv);
    exit(EXIT_FAILURE);
}

static int
mputrace_handle_options(int argc, char *argv[])
{
    int ret = 0;
    std::map<std::string, int> oper = {
        {"conf", MPUTRACE_CONFIG},
        {"dump", MPUTRACE_DUMP},
        {"reset", MPUTRACE_RESET},
        {"show", MPUTRACE_SHOW},
    };

    switch (oper[std::string((const char *)argv[1])]) {
        case MPUTRACE_CONFIG:
            if (argc < MPUTRACE_MAX_ARG) {
                std::cerr << "Specify config filename" << std::endl;
                usage(argv);
                exit(EXIT_FAILURE);
                break;
            }
            mpu_trace_config(argv[2]);
            break;
        case MPUTRACE_DUMP:
            if (argc < MPUTRACE_MAX_ARG) {
                std::cerr << "Specify dump filename" << std::endl;
                usage(argv);
                exit(EXIT_FAILURE);
            }
            mpu_trace_dump(argv[2]);
            break;
        case MPUTRACE_RESET:
            mpu_trace_reset();
            break;
        case MPUTRACE_SHOW:
            mpu_trace_show();
            break;
        default:
            ret = mputrace_error(argv);
    }
    return ret;
}

} // end namespace platform
} // end namespace sdk

int
main(int argc, char *argv[])
{
    int ret = 0;

    if (argc < MPUTRACE_MAX_ARG - 1) {
        sdk::platform::usage(argv);
        exit(EXIT_FAILURE);
    }

#ifdef __x86_64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_SIM) ==
           sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HW) ==
           sdk::lib::PAL_RET_OK);
#endif

    sdk::platform::mputrace_register();
    ret = sdk::platform::mputrace_handle_options(argc, argv);

    return ret;
}
