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
    MPUTRACE_ENABLE,
    MPUTRACE_DISABLE,
    MPUTRACE_RESET,
    MPUTRACE_DUMP,
    MPUTRACE_SHOW
} mputrace_op_type_t;

#pragma pack(push, 1)

typedef struct mpu_trace_record_s {
    uint8_t pipeline_type;
    uint32_t stage_id;
    uint32_t mpu;
    uint8_t enable;
    uint8_t trace_enable;
    uint8_t phv_debug;
    uint8_t phv_error;
    uint64_t watch_pc;
    uint64_t trace_addr;
    uint8_t table_key;
    uint8_t instructions;
    uint8_t wrap;
    uint8_t reset;
    uint32_t trace_size;
    uint8_t __pad[27]; // Pad to 64 bytes
} mpu_trace_record_t;

static_assert(sizeof(mpu_trace_record_t) == 64,
              "mpu trace record struct should be 64B");

#pragma pack(pop)

extern std::map<int, int> max_stages;

void mputrace_config_trace(int, int, int, mpu_trace_record_t *);
void mputrace_disable_trace();
void mputrace_enable_trace();
void mputrace_reset();
void mputrace_dump();
void mputrace_show();

extern char g_MPUTRACE_DUMP_FILE[MPUTRACE_STR_NAME_LEN];
extern uint64_t g_trace_base;

} // end namespace platform
} // end namespace sdk

#endif // __MPU_TRACE_HPP__
