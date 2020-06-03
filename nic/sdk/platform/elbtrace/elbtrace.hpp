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
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "third-party/asic/elba/model/elb_top/elb_top_csr.h"
#include "include/sdk/types.hpp"
#include "lib/pal/pal.hpp"

using boost::property_tree::ptree;

namespace sdk {
namespace platform {

#define MPUTRACE_MAX_ARG 3
#define MPUTRACE_PIPELINE_NONE -1
#define MPUTRACE_MAX_MPU 4

#define ELBTRACE_ONE 1

#define ELBTRACE_STR_NAME_LEN 256
#define TRACE_ENTRY_SIZE 64
#define SDPTRACE_PHV_ENTRY_SIZE 64  //todo - justina
#define SDPTRACE_CTL_ENTRY_SIZE 64  //todo - justina
#define DMATRACE_ENTRY_SIZE 64  //todo - justina
#define MPUTRACE_FREE(ptr)                                                     \
    if (ptr) {                                                                 \
        free(ptr);                                                             \
        ptr = NULL;                                                            \
    }

typedef struct mputrace_global_state_s {
    int pipeline_index;
    char ELBTRACE_DUMP_FILE[ELBTRACE_STR_NAME_LEN];
    mem_addr_t trace_base;
    mem_addr_t trace_end;
    mem_addr_t sdptrace_phv_base;
    mem_addr_t sdptrace_phv_end;
    mem_addr_t sdptrace_ctl_base;
    mem_addr_t sdptrace_ctl_end;
    mem_addr_t dmatrace_base;
    mem_addr_t dmatrace_end;
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

typedef enum dma_pipeline_type_s {
    RXPDMA,
    TXPDMA,
    DMA_PIPE_CNT
} dma_pipeline_type_t;

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
    DMATRACE_CONFIG,
    DMATRACE_RESET,
    DMATRACE_DUMP,
    DMATRACE_SHOW,
    SDPTRACE_CONFIG,
    SDPTRACE_RESET,
    SDPTRACE_DUMP,
    SDPTRACE_SHOW,
    MPUTRACE_CONFIG,
    MPUTRACE_RESET,
    MPUTRACE_DUMP,
    MPUTRACE_SHOW
} mputrace_op_type_t;

  ///temp



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


  /////////
  // SDP //
  /////////
  
  // ctrl options
typedef struct sdptrace_cfg_inst_ctrl_s {
    uint8_t enable;     
    uint8_t trace_trigger_enable;     
} sdptrace_cfg_inst_ctrl_t;


  // capture
typedef struct sdptrace_cfg_inst_capture_s {
  cpp_int trigger_data;  // PHV SOP data
  cpp_int trigger_mask;  // PHV SOP mask
  //  uint64_t trigger_data[8];    
  //uint64_t trigger_mask[8];    
} sdptrace_cfg_inst_capture_t;

  // settings
typedef struct sdptrace_cfg_inst_settings_s {
    uint8_t sw_reset_enable;  //elbtrace reset_sdp will take care of this   
    uint8_t no_trace_when_full;
    uint8_t stop_when_full;   
    uint32_t ring_size;
    uint64_t phv_base_addr;
    uint64_t ctl_base_addr;
} sdptrace_cfg_inst_settings_t;

typedef struct sdptrace_cfg_inst_s {
    std::string pipeline_str;
    std::string stage_str;
    sdptrace_cfg_inst_ctrl_t ctrl;
    sdptrace_cfg_inst_capture_t capture;
    sdptrace_cfg_inst_settings_t settings;
} sdptrace_cfg_inst_t;

  /////////
  // DMA //
  /////////
  
  // ctrl options
typedef struct dmatrace_cfg_inst_ctrl_s {
    uint8_t enable;     
    uint8_t phv_enable;     
    uint8_t capture_all;     
    uint8_t axi_err_enable;     
    uint8_t pkt_phv_sync_err_enable;     
} dmatrace_cfg_inst_ctrl_t;

  // settings
typedef struct dmatrace_cfg_inst_settings_s {
  uint8_t wrap;  
  uint8_t reset;  //elbtrace reset_dma will take care of this   
  uint32_t buf_size;
  uint64_t base_addr;
} dmatrace_cfg_inst_settings_t;

typedef struct dmatrace_cfg_inst_s {
    std::string pipeline_str;
    dmatrace_cfg_inst_ctrl_t ctrl;
    dmatrace_cfg_inst_settings_t settings;
} dmatrace_cfg_inst_t;


#pragma pack(push, 1)

  //todo: Justina: original one doesn't seem to be 64Bytes. Check with Neel
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

  uint8_t __pad[54];    // Pad to 64 bytes boundary
} mputrace_trace_hdr_t;

  //todo: changed assert to 74B from 64B. Is that ok? Why 64B??? Neel??
static_assert(sizeof(mputrace_trace_hdr_t) == 128,
              "mpu trace instance struct should be 128B");

typedef struct sdptrace_trace_hdr_s {
  uint8_t pipeline_num;
  uint32_t stage_num;
  uint8_t enable;     
  uint8_t trace_trigger_enable;     
  uint8_t sw_reset_enable;  //elbtrace reset_sdp will take care of this   
  uint8_t no_trace_when_full;
  uint8_t stop_when_full;   
  uint32_t ctl_base_addr;
  uint32_t phv_base_addr;
  uint32_t ring_size;
  uint512_t trigger_data; //todo: shd this map to PHV?    
  uint512_t trigger_mask;    //todo: shd this map to PHV?

  uint8_t __pad[10];    // Pad to 64 bytes boundary
} sdptrace_trace_hdr_t;

  //todo: it's 150, but somehow reads as 182. Check why
  static_assert(sizeof(sdptrace_trace_hdr_t) == 192,
		"sdp trace instance struct should be 192B");

typedef struct dmatrace_trace_hdr_s {
  uint8_t pipeline_num;
    uint8_t enable;     
    uint8_t phv_enable;     
    uint8_t capture_all;     
    uint8_t axi_err_enable;     
    uint8_t pkt_phv_sync_err_enable;     
  uint8_t wrap;  
  uint8_t reset;  //elbtrace reset_dma will take care of this   
  uint32_t buf_size;
  uint64_t base_addr;

  uint8_t __pad[44];    // Pad to 64 bytes

} dmatrace_trace_hdr_t;

static_assert(sizeof(dmatrace_trace_hdr_t) == 64,
              "dma trace instance struct should be 64B");


  ///////// PHV Intrinsic fields //////////

 typedef struct __attribute__ ((packed)) elb_phv_intr_global_c_s {
  uint64_t tm_iport:4;
  uint64_t tm_oport:4;
  uint64_t tm_iq:5;
  uint64_t lif:11;
  uint64_t timestamp:48;
  uint64_t  tm_span_session:8;
  uint64_t tm_replicate_ptr:16;
  uint64_t tm_replicate_en:1;
  uint64_t tm_cpu:1;
  uint64_t tm_q_depth:14;
  uint64_t drop:1;
  uint64_t bypass:1;
  uint64_t hw_error:1;
  uint64_t tm_oq:5;
  uint64_t debug_trace:1;
  uint64_t csum_err:5;
  uint64_t error_bits:6;
  uint64_t tm_instance_type:4;
  } elb_phv_intr_global_c_t;
  //17B

 typedef struct __attribute__ ((packed)) elb_phv_intr_p4_c_s {
  elb_phv_intr_global_c_t phv_global;
  uint64_t crc_err:1;
  uint64_t len_err:4;
  uint64_t recirc_count:3;
  uint64_t parser_err:1;
  uint64_t crypto_hdr:1;
  uint64_t frame_size:14;
  uint64_t no_data:1;
  uint64_t recirc:1;
  uint64_t packet_len:14;
  uint64_t csum_error_hi:2;
  uint64_t deparser_csum_disable:7;
  uint64_t deparser_crc_disable:1;
  uint64_t roce_table_ptr:9;
  uint64_t roce_csum_table:6;
  uint64_t roce_enable:1;
  uint64_t payload_start:14;
  uint64_t payload_size:14;
  uint64_t auth_result:1;
  uint64_t elb_align_pad:25;
  uint64_t stage_skip:8;
  uint64_t phv_id:10;
  uint64_t app_type:4;
  uint64_t elb_p4_rsv:2;
  uint64_t padding_1:40;
  uint64_t padding_0:64;
  uint64_t hdr_vld_1:64;
  uint64_t hdr_vld_0:64;
  } elb_phv_intr_p4_c_t;

 typedef struct __attribute__ ((packed)) elb_phv_intr_txdma_c_s {
  elb_phv_intr_global_c_t phv_global;
  uint64_t crc_err:1;
  uint64_t len_err:4;
  uint64_t recirc_count:3;
  uint64_t parser_err:1;
  uint64_t crypto_hdr:1;
  uint64_t frame_size:14;
  uint64_t no_data:1;
  uint64_t recirc:1;
  uint64_t packet_len:14;
  uint64_t qid:24;
  uint64_t dma_cmd_ptr:6;
  uint64_t qstate_addr:34;
  uint64_t qtype:3;
  uint64_t txdma_rsv:5;
  uint64_t elb_align_pad:8;
  uint64_t stage_skip:8;
  uint64_t phv_id:10;
  uint64_t app_type:4;
  uint64_t elb_txdma_rsv:2;
  uint64_t padding_3:40;
  uint64_t padding_2:64;
  uint64_t padding_1:64;
  uint64_t padding_0:64;
  } elb_phv_intr_txdma_c_t;

 typedef struct __attribute__ ((packed)) elb_phv_intr_rxdma_c_s {
  elb_phv_intr_global_c_t phv_global;
  uint64_t crc_err:1;
  uint64_t len_err:4;
  uint64_t recirc_count:3;
  uint64_t parser_err:1;
  uint64_t crypto_hdr:1;
  uint64_t frame_size:14;
  uint64_t no_data:1;
  uint64_t recirc:1;
  uint64_t packet_len:14;
  uint64_t qid:24;
  uint64_t dma_cmd_ptr:6;
  uint64_t qstate_addr:34;
  uint64_t qtype:3;
  uint64_t rx_splitter_offset:10;
  uint64_t rxdma_rsv:3;
  uint64_t stage_skip;
  uint64_t phv_id:10;
  uint64_t app_type:4;
  uint64_t elb_rxdma_rsv:2;
  uint64_t padding_3:40;
  uint64_t padding_2:64;
  uint64_t padding_1:64;
  uint64_t padding_0:64;
 } elb_phv_intr_rxdma_c_t;






#pragma pack(pop)

extern std::map<int, int> max_stages;

std::string mputrace_pipeline_str_get(int);
std::string dmatrace_pipeline_str_get(int);
void mputrace_cfg_trace(int, int, int, mputrace_cfg_inst_t *);

void sdptrace_cfg_trace(int, int, sdptrace_cfg_inst_t *);

void dmatrace_cfg_trace(int, dmatrace_cfg_inst_t *);

/// Does cfg of mputrace registers as specified in the json file
///
/// param[in] cfg_file Json file containing the config for the trace registers
void elbtrace_cfg(const char *cfg_file, std::string mod_name);

/// Does show of enabled mputrace registers in a tabular format.
void mputrace_show(void);
void sdptrace_show(void);
void dmatrace_show(void);

/// Dumps the contents of trace info from HBM into the specified dump file
///
/// param[in] dump_file File name to dump the trace info from HBM
void elbtrace_dump(const char *dump_file, std::string mod_name);

/// Does reset of trace registers.
void mputrace_reset(void);
void sdptrace_reset(void);
void dmatrace_reset(void);

void sdptrace_json_capture_parse (ptree &pt, sdptrace_cfg_inst_t *cfg_inst);
void sdptrace_json_capture_parse_verify (sdptrace_cfg_inst_t *cfg_inst);

}    // end namespace platform
}    // end namespace sdk

#endif    // __MPU_TRACE_HPP__
