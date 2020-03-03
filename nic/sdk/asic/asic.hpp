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
    sdk::platform::asic_type_t          asic_type;
    std::string          default_config_dir;    // TODO: vasanth, pls. remove this up eventually
    uint32_t             admin_cos;
    uint32_t             repl_entry_width;
    std::string          cfg_path;
    std::string          pgm_name;
    uint8_t              num_pgm_cfgs;
    uint8_t              num_asm_cfgs;
    uint8_t              num_rings;
    asic_pgm_cfg_t       pgm_cfg[SDK_ASIC_PGM_CFG_MAX];
    asic_asm_cfg_t       asm_cfg[SDK_ASIC_ASM_CFG_MAX];
    sdk::lib::catalog    *catalog;
    mpartition           *mempartition;
    sdk::platform::ring_meta_t
                         *ring_meta;
    platform_type_t      platform;
    completion_cb_t      completion_func;
    bool                 is_slave;
    sdk::lib::device_profile_t  *device_profile;
} asic_cfg_t;

// initialize the asic
sdk_ret_t asic_init(asic_cfg_t *asic_cfg);
// cleanup asic initialization
void asic_cleanup(void);

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
//           Codes which are relevant for the above case should be put under is_soft_init()
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

// Asic Doorbell address
uint64_t asic_local_dbaddr_get(void);
uint64_t asic_host_dbaddr_get(void);

// returns true if the init type is SOFT, false otherwise
bool is_soft_init(void);
// returns true if the init type is UPGRADE, false otherwise
bool is_upgrade_init(void);
// returns true if the init type is HARD, false otherwise(SOFT/UPGRADE)
bool is_hard_init(void);
void set_init_type(asic_init_type_t type);
void set_state(asic_state_t state);
bool is_quiesced(void);

}    // namespace asic
}    // namespace sdk

using sdk::asic::asic_pgm_cfg_t;
using sdk::asic::asic_asm_cfg_t;
using sdk::asic::asic_cfg_t;
using sdk::asic::asic_block_t;
using sdk::asic::asic_bw_t;
using sdk::asic::asic_hbm_bw_t;

#endif    // __SDK_ASIC_HPP__
