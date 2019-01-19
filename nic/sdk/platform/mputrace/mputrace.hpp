//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the captrace implementation for setting trace
/// registers in ASIC
///
//===----------------------------------------------------------------------===//

#ifndef __MPU_TRACE_HPP__
#define __MPU_TRACE_HPP__

#include "nic/asic/capri/model/cap_top/cap_top_csr.h"
#include "include/sdk/types.hpp"
#include "lib/pal/pal.hpp"

#include <stdint.h>
#include <stdlib.h>

namespace sdk {
namespace platform {

#define MPUTRACE_MAX_ARG 3
#define MPUTRACE_PIPELINE_NONE -1
#define MPUTRACE_MAX_MPU 4

#define MPUTRACE_ONE 1

#define MPUTRACE_STR_NAME_LEN 256
#define TRACE_ENTRY_SIZE 64
#define MPUTRACE_FREE(ptr)                                                     \
    if (ptr) {                                                                 \
        free(ptr);                                                             \
        ptr = NULL;                                                            \
    }

/// This order of defn. is needed for mputrace show and conf functionality
typedef enum pipeline_type_s {
    P4IG,
    P4EG,
    RXDMA,
    TXDMA,
    PIPE_CNT
} pipeline_type_t;

typedef enum mputrace_arg_types_s {
    MPUTRACE_NONE = 0,
    MPUTRACE_TRACE_ENABLE,
    MPUTRACE_PHV_DEBUG,
    MPUTRACE_PHV_ERROR,
    MPUTRACE_TBL_KEY_ENABLE,
    MPUTRACE_INSTR_ENABLE,
    MPUTRACE_WRAP,
    MPUTRACE_WATCH_PC,
    MPUTRACE_TRACE_SIZE
} mputrace_arg_types_t;

typedef enum mputrace_op_type_s {
    MPUTRACE_CONFIG,
    MPUTRACE_RESET,
    MPUTRACE_DUMP,
    MPUTRACE_SHOW
} mputrace_op_type_t;

#pragma pack(push, 1)

typedef struct mputrace_instance_s {
    uint8_t pipeline_type;
    uint32_t stage_id;
    uint32_t mpu;
    /// trigger options
    uint8_t enable; // this is a continuous trigger, if this is enabled
                    // all phvs are traced irrespective of the next 4 triggers
    uint8_t trace_enable; // tracing starts if a trace instr is in the program
    uint8_t phv_debug; // trace only for phvs with phv_debug enabled
    uint8_t phv_error; // trace only for phvs with table error enabled
    uint64_t watch_pc; // trace whenever pc == watch_pc
    /// data to be traced
    uint64_t trace_addr;
    uint8_t table_key; // Data and Key pair
    uint8_t instructions; // Instructions

    uint8_t wrap; // Trace data will wrap over if this is enabled
    uint8_t reset;
    uint32_t trace_size;

    uint8_t __pad[27]; // Pad to 64 bytes
} mputrace_instance_t;

static_assert(sizeof(mputrace_instance_t) == 64,
              "mpu trace instance struct should be 64B");

#pragma pack(pop)

extern std::map<int, int> max_stages;

void mputrace_config_trace(int, int, int, mputrace_instance_t *);
void mputrace_disable_trace();
void mputrace_enable_trace();
void mputrace_reset();
void mputrace_dump();
void mputrace_show();

extern char g_MPUTRACE_DUMP_FILE[MPUTRACE_STR_NAME_LEN];
extern uint64_t g_trace_base;
extern uint64_t g_trace_end;

} // end namespace platform
} // end namespace sdk

#endif // __MPU_TRACE_HPP__
