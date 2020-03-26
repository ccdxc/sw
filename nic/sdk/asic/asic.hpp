// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __SDK_ASIC_HPP__
#define __SDK_ASIC_HPP__

#include <string>
#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"
#include "lib/catalog/catalog.hpp"
#include "platform/utils/mpartition.hpp"
#include "platform/ring/ring.hpp"
#include "p4/loader/loader.hpp"
#include "lib/device/device.hpp"

namespace sdk {
namespace asic {

#define SDK_ASIC_PGM_CFG_MAX        3
#define SDK_ASIC_ASM_CFG_MAX        3

#define SDK_ASIC_REPL_ENTRY_WIDTH   (64)
#define SDK_ASIC_REPL_TABLE_DEPTH   (64*1024)

#define NUM_MAX_COSES                           16

#define ASIC_TXS_MAX_TABLE_ENTRIES              2048
// 2K * 8K scheduler
#define ASIC_TXS_SCHEDULER_MAP_MAX_ENTRIES      2048
#define ASIC_TXS_SCHEDULER_NUM_QUEUES_PER_ENTRY 8192
// Timer definitions
#define ASIC_TIMER_WHEEL_DEPTH                  4096
#define ASIC_TIMER_NUM_KEY_PER_CACHE_LINE       16
#define ASIC_TIMER_NUM_DATA_PER_CACHE_LINE      12
// This needs to be a power of 2
#define ASIC_TIMER_NUM_KEY_CACHE_LINES          1024

// each line is 64B
// Each key in key line takes up 1 line in data space
#define ASIC_TIMER_HBM_DATA_SPACE \
        (ASIC_TIMER_NUM_KEY_CACHE_LINES * ASIC_TIMER_NUM_KEY_PER_CACHE_LINE * 64)

#define ASIC_TIMER_HBM_KEY_SPACE \
        (ASIC_TIMER_NUM_KEY_CACHE_LINES * 64)

#define ASIC_TIMER_HBM_SPACE \
        (ASIC_TIMER_HBM_KEY_SPACE + ASIC_TIMER_HBM_DATA_SPACE)

#define ASIC_MAX_TIMERS \
        (ASIC_TIMER_NUM_KEY_CACHE_LINES * ASIC_TIMER_NUM_KEY_PER_CACHE_LINE * \
         ASIC_TIMER_NUM_DATA_PER_CACHE_LINE)

//16 64B PHV entries(Flits)
#define ASIC_SW_PHV_NUM_MEM_ENTRIES 16

//8 Profiles (config and control)
#define ASIC_SW_PHV_NUM_PROFILES  8

// Number of parser instances
#define ASIC_NUM_PPA 2

// Asic Flit size in bytes
#define ASIC_FLIT_SIZE (512/8)

typedef void (*completion_cb_t)(sdk_status_t status);

typedef struct asic_pgm_cfg_s {
    std::string                 path;
} asic_pgm_cfg_t;

typedef struct asic_asm_cfg_s {
    std::string          name;
    std::string          path;
    std::string          base_addr;
    mpu_pgm_sort_t       sort_func;
    mpu_pgm_symbols_t    symbols_func;
} asic_asm_cfg_t;

typedef struct asic_cfg_s {
    sdk::platform::asic_type_t  asic_type;
    std::string                 default_config_dir;
    uint32_t                    admin_cos;
    uint32_t                    repl_entry_width;
    std::string                 cfg_path;
    std::string                 pgm_name;
    uint8_t                     num_pgm_cfgs;
    uint8_t                     num_asm_cfgs;
    uint8_t                     num_rings;
    asic_pgm_cfg_t              pgm_cfg[SDK_ASIC_PGM_CFG_MAX];
    asic_asm_cfg_t              asm_cfg[SDK_ASIC_ASM_CFG_MAX];
    sdk::lib::catalog           *catalog;
    mpartition                  *mempartition;
    sdk::platform::ring_meta_t  *ring_meta;
    platform_type_t             platform;
    completion_cb_t             completion_func;
    bool                        is_slave;
    bool                        p4_cache;
    bool                        p4plus_cache;
    bool                        llc_cache;
    sdk::lib::device_profile_t  *device_profile;
} asic_cfg_t;

// TODO: please move this to sdk/lib/p4 later !!
typedef struct p4_table_mem_layout_ {
    uint16_t    entry_width;    /* In units of memory words.. 16b  in case of PIPE tables */
                                /* In units of bytes in case of HBM table */
    uint16_t    entry_width_bits;
    uint32_t    start_index;
    uint32_t    end_index;
    uint16_t    top_left_x;
    uint16_t    top_left_y;
    uint8_t     top_left_block;
    uint16_t    btm_right_x;
    uint16_t    btm_right_y;
    uint8_t     btm_right_block;
    uint8_t     num_buckets;
    uint32_t    tabledepth;
    mem_addr_t  base_mem_pa; /* Physical addres in  memory */
    mem_addr_t  base_mem_va; /* Virtual  address in  memory */
    char        *tablename;
} p4_table_mem_layout_t;

typedef enum asic_block_e {
    ASIC_BLOCK_PACKET_BUFFER,
    ASIC_BLOCK_TXDMA,
    ASIC_BLOCK_RXDMA,
    ASIC_BLOCK_MS,
    ASIC_BLOCK_PCIE,
    ASIC_BLOCK_MAX
} asic_block_t;

typedef struct asic_bw_s {
    double read;
    double write;
} asic_bw_t;

typedef struct asic_hbm_bw_s {
    asic_block_t type;
    uint64_t clk_diff;
    asic_bw_t max;
    asic_bw_t avg;
} asic_hbm_bw_t;

// Asic init types. Programmer has to understand these init types and write the
// asic init codes under the below types. Purposes of these types are
//
// Hard :    Default type.
//           Initializes all the registers,state variables and bringups all the
//           threads as per the code flow.
//
// Soft :    Initializes the state variables for read access to the tables and registers.
//           Will not launch the process threads as per the code flow.
//           Example usage for this initialization is for "CLI" where it requires to
//           access the registers or tables for debugging.
//           Codes which are relevant for the above case should be put under asic_is_soft_init()
//
// Upgrade : Will be setup by Upgrade manager before launching the new application.
//           Does not initializes the registers as it is configured by the Hard init
//           Initializes all the state variables to accept the new configuration from Agent
//           and program the HW tables.
//           Registers/tables which are shared by new and currently-active processes/pipeline,
//           are modified in quiesced state.
//           Codes which are relevant for the above case should be put under is_upgrade_init()
typedef enum asic_init_type_e {
    ASIC_INIT_TYPE_HARD = 0,
    ASIC_INIT_TYPE_SOFT = 1,
    ASIC_INIT_TYPE_UPGRADE = 2
} asic_init_type_t;

// Asic states.
// Running  : Default state.
// Quiesced : HW has been quiesced to modify HW registers which are used in the P4 data path.
//            Once the modification has been done, programmer should move back the state to
//            running.
typedef enum asic_state_e {
    ASIC_STATE_RUNNING = 0,
    ASIC_STATE_QUIESCED = 1
} asic_state_t;

typedef struct lif_qtype_info_s {
    uint8_t entries;
    uint8_t size;
    uint8_t cosA;
    uint8_t cosB;
} __PACK__ lif_qtype_info_t;

const static uint32_t kNumQTypes = 8;
const static uint32_t kAllocUnit = 4096;

typedef struct lif_qstate_s {
    uint32_t lif_id;
    uint32_t allocation_size;
    uint64_t hbm_address;
    uint8_t hint_cos;
    uint8_t enable;
    struct {
        lif_qtype_info_t qtype_info;
        uint32_t hbm_offset;
        uint32_t qsize;
        uint32_t rsvd;
        uint32_t num_queues;
        uint8_t  coses;
    } type[kNumQTypes];
} __PACK__ lif_qstate_t;

// returns true if the init type is SOFT, false otherwise
bool asic_is_soft_init(void);
// returns true if the init type is UPGRADE, false otherwise
bool is_upgrade_init(void);
// returns true if the init type is HARD, false otherwise(SOFT/UPGRADE)
bool asic_is_hard_init(void);
void asic_set_init_type(asic_init_type_t type);
void set_state(asic_state_t state);
bool asic_is_quiesced(void);

typedef enum p4plus_cache_action_e {
    P4PLUS_CACHE_ACTION_NONE        = 0x0,
    P4PLUS_CACHE_INVALIDATE_RXDMA   = 0x1,
    P4PLUS_CACHE_INVALIDATE_TXDMA   = 0x2,
    P4PLUS_CACHE_INVALIDATE_BOTH    = P4PLUS_CACHE_INVALIDATE_RXDMA |
                                      P4PLUS_CACHE_INVALIDATE_TXDMA
} p4plus_cache_action_t;

// sw phv pipeline type
typedef enum asic_swphv_type_e {
    ASIC_SWPHV_TYPE_RXDMA   = 0,    // P4+ RxDMA
    ASIC_SWPHV_TYPE_TXDMA   = 1,    // P4+ TxDMA
    ASIC_SWPHV_TYPE_INGRESS = 2,    // P4 Ingress
    ASIC_SWPHV_TYPE_EGRESS  = 3,    // P4 Egress
} asic_swphv_type_t;

// sw phv injection state
typedef struct asic_sw_phv_state_s {
    bool        enabled;
    bool        done;
    uint32_t    current_cntr;
    uint32_t    no_data_cntr;
    uint32_t    drop_no_data_cntr;
} asic_sw_phv_state_t;

}    // namespace asic
}    // namespace sdk

using sdk::asic::asic_pgm_cfg_t;
using sdk::asic::asic_asm_cfg_t;
using sdk::asic::asic_cfg_t;
using sdk::asic::asic_block_t;
using sdk::asic::asic_bw_t;
using sdk::asic::asic_hbm_bw_t;
using sdk::asic::p4_table_mem_layout_t;
using sdk::asic::lif_qstate_t;
using sdk::asic::lif_qtype_info_t;
using sdk::asic::kAllocUnit;
using sdk::asic::kNumQTypes;
using sdk::asic::p4plus_cache_action_t;
using sdk::asic::p4plus_cache_action_t::P4PLUS_CACHE_ACTION_NONE;
using sdk::asic::p4plus_cache_action_t::P4PLUS_CACHE_INVALIDATE_RXDMA;
using sdk::asic::p4plus_cache_action_t::P4PLUS_CACHE_INVALIDATE_TXDMA;
using sdk::asic::p4plus_cache_action_t::P4PLUS_CACHE_INVALIDATE_BOTH;


#endif    // __SDK_ASIC_HPP__
