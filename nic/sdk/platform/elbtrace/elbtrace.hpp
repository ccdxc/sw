//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the mputrace implementation for setting trace
/// registers in ASIC
///
//===----------------------------------------------------------------------===//

#ifndef __MPU_TRACE_HPP__
#define __MPU_TRACE_HPP__

#include <stdint.h>
#include <stdlib.h>
#include "third-party/asic/elba/model/elb_top/elb_top_csr.h"
#include "include/sdk/types.hpp"
#include "lib/pal/pal.hpp"

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

typedef struct mputrace_global_state_s {
    int pipeline_index;
    char MPUTRACE_DUMP_FILE[MPUTRACE_STR_NAME_LEN];
    mem_addr_t trace_base;
    mem_addr_t trace_end;
} mputrace_global_state_t;

extern mputrace_global_state_t g_state;

// This order of defn. is needed for mputrace show and conf functionality
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
    SDPTRACE_CONFIG,
    SDPTRACE_RESET,
    SDPTRACE_DUMP,
    SDPTRACE_SHOW,
    MPUTRACE_CONFIG,
    MPUTRACE_RESET,
    MPUTRACE_DUMP,
    MPUTRACE_SHOW
} mputrace_op_type_t;

// watch_pc options
typedef struct mputrace_cfg_inst_watch_pc_s {
    uint8_t trace;
    uint8_t count;
    uint8_t intr;
    uint8_t stop;
    uint8_t exception;
    uint64_t addr_lo;
    uint64_t addr_hi;
} mputrace_cfg_inst_watch_pc_t;

// watch_data options
typedef struct mputrace_cfg_inst_watch_data_s {
    uint8_t rtrace;
    uint8_t wtrace;
    uint8_t rintr;
    uint8_t wintr;
    uint8_t rstop;
    uint8_t wstop;
    uint8_t rexception;
    uint8_t wexception;
    uint64_t addr_lo;
    uint64_t addr_hi;
} mputrace_cfg_inst_watch_data_t;

// ctrl options
typedef struct mputrace_cfg_inst_ctrl_s {
    uint8_t
        trace_enable;     // tracing starts if a trace instr is in the program
    uint8_t phv_debug;    // trace only for phvs with phv_debug enabled
    uint8_t phv_error;    // trace only for phvs with table error enabled
  //    uint64_t watch_pc;    // trace whenever pc == watch_pc
    mputrace_cfg_inst_watch_data_t watch_data;
    mputrace_cfg_inst_watch_pc_t watch_pc;
} mputrace_cfg_inst_ctrl_t;


// data to be traced
typedef struct mputrace_cfg_inst_capture_s {
    uint8_t table_key;       // Data and Key pair
    uint8_t instructions;    // Instructions
} mputrace_cfg_inst_capture_t;

typedef struct mputrace_cfg_inst_settings_s {
    uint8_t wrap;    // Trace data will wrap over if this is enabled
    uint8_t reset;
    uint32_t trace_size;
    uint64_t trace_addr;
} mputrace_cfg_inst_settings_t;

typedef struct mputrace_cfg_inst_s {
    std::string pipeline_str;
    std::string stage_str;
    std::string mpu_str;
    mputrace_cfg_inst_ctrl_t ctrl;
    mputrace_cfg_inst_capture_t capture;
    mputrace_cfg_inst_settings_t settings;
} mputrace_cfg_inst_t;

#pragma pack(push, 1)

  //Justina: original one doesn't seem to be 64Bytes. Check with Neel
  // current one is 74B. Why is there a restriction for 64B???
typedef struct mputrace_trace_hdr_s {
    uint8_t pipeline_num;
    uint32_t stage_num;
    uint32_t mpu_num;
    // control options
    uint8_t
        enable;    // this is a continuous trigger, if this is enabled
                   // all phvs are traced irrespective of the next 4 triggers
    uint8_t
        trace_enable;     // tracing starts if a trace instr is in the program
    uint8_t phv_debug;    // trace only for phvs with phv_debug enabled
    uint8_t phv_error;    // trace only for phvs with table error enabled
  //    uint64_t watch_pc;    // trace whenever pc == watch_pc
    // data to be traced
    uint64_t trace_addr;
    uint8_t table_key;       // Data and Key pair
    uint8_t instructions;    // Instructions

    uint8_t wrap;    // Trace data will wrap over if this is enabled
    uint8_t reset;
    uint32_t trace_size;

  //watch_pc
  uint8_t   wpc_trace;
  uint8_t   wpc_count;
  uint8_t   wpc_intr;
  uint8_t   wpc_stop;
  uint8_t   wpc_exception;
  uint64_t  wpc_addr_lo;
  uint64_t  wpc_addr_hi;

  //watch_data
  uint8_t  wdata_rtrace;
  uint8_t  wdata_wtrace;
  uint8_t  wdata_rintr;
  uint8_t  wdata_wintr;
  uint8_t  wdata_rstop;
  uint8_t  wdata_wstop;
  uint8_t  wdata_rexception;
  uint8_t  wdata_wexception;
  uint64_t wdata_addr_lo;
  uint64_t wdata_addr_hi;

  //    uint8_t __pad[27];    // Pad to 64 bytes
} mputrace_trace_hdr_t;
  //justina: add watch_pc and watch_data above. it doesn't seem 64 now. check.

  //justina: changed assert to 74B from 64B. Is that ok? Why 64B??? Neel??
static_assert(sizeof(mputrace_trace_hdr_t) == 74,
              "mpu trace instance struct should be 74B");

#pragma pack(pop)

extern std::map<int, int> max_stages;

std::string mputrace_pipeline_str_get(int);
void mputrace_cfg_trace(int, int, int, mputrace_cfg_inst_t *);

/// Does cfg of mputrace registers as specified in the json file
///
/// param[in] cfg_file Json file containing the config for the trace registers
void mputrace_cfg(const char *cfg_file);

/// Does show of enabled mputrace registers in a tabular format.
void mputrace_show(void);

/// Dumps the contents of trace info from HBM into the specified dump file
///
/// param[in] dump_file File name to dump the trace info from HBM
void mputrace_dump(const char *dump_file);

/// Does reset of mputrace registers.
void mputrace_reset(void);

}    // end namespace platform
}    // end namespace sdk

#endif    // __MPU_TRACE_HPP__
