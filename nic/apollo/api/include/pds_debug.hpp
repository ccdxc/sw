//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines debug APIs
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_DEBUG_HPP__
#define __INCLUDE_API_PDS_DEBUG_HPP__

#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_flow.hpp"
#include "nic/apollo/api/debug.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/asic/pd/pd.hpp"
#include "nic/utils/ftlite/ftlite_ipv4_structs.hpp"
#include "nic/utils/ftlite/ftlite_ipv6_structs.hpp"
#include "nic/sdk/platform/capri/capri_tm_rw.hpp"
#include "nic/sdk/platform/capri/capri_tm_utils.hpp"

/// \cond DO_NOT_DOCUMENT

typedef enum pds_clock_freq_e {
    PDS_CLOCK_FREQUENCY_833  = 0,
    PDS_CLOCK_FREQUENCY_900  = 1,
    PDS_CLOCK_FREQUENCY_957  = 2,
    PDS_CLOCK_FREQUENCY_1033 = 3,
    PDS_CLOCK_FREQUENCY_1100 = 4,
    PDS_CLOCK_FREQUENCY_1666 = 5,
    PDS_CLOCK_FREQUENCY_2200 = 6,
} pds_clock_freq_t;

typedef struct pds_system_temperature_e {
    uint32_t dietemp;
    uint32_t localtemp;
    uint32_t hbmtemp;
} pds_system_temperature_t;

typedef struct pds_system_power_e {
    uint32_t pin;
    uint32_t pout1;
    uint32_t pout2;
} pds_system_power_t;

typedef struct pds_table_stats_e {
    std::string table_name;
    sdk::table::sdk_table_api_stats_t api_stats;
    sdk::table::sdk_table_stats_t     table_stats;
} pds_table_stats_t;

typedef struct pds_pb_debug_stats_e {
    uint32_t port;
    sdk::tm_debug_stats_t stats;
    sdk::platform::capri::capri_queue_stats_t qos_queue_stats;
} pds_pb_debug_stats_t;

typedef struct pds_meter_debug_stats_e {
    uint32_t idx;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
} pds_meter_debug_stats_t;

typedef struct session_stats_entry_s {
    uint64_t iflow_bytes_count;
    uint64_t iflow_packet_count;
    uint64_t rflow_bytes_count;
    uint64_t rflow_packet_count;
} pds_session_debug_stats_t;

typedef enum mapping_dump_type_e {
    MAPPING_DUMP_TYPE_LOCAL = 0,
    MAPPING_DUMP_TYPE_REMOTE_L2,
    MAPPING_DUMP_TYPE_REMOTE_L3,
} mapping_dump_type_t;

typedef struct mapping_hw_key_s {
    pds_mapping_type_t type;
    union {
        ///< L3 key
        struct {
            uint32_t  vpc;        ///< HW ID of VPC this IP belongs to
            ip_addr_t ip_addr;    ///< IP address of the mapping
        };
        ///< L2 key
        struct {
            uint32_t   subnet;    ///< HW ID of subnet of the mapping
            mac_addr_t mac_addr;  ///< MAC address of the mapping
        };
    };
} mapping_hw_key_t;

typedef struct mapping_dump_args_s {
    bool key_valid;
    mapping_hw_key_t skey;
    mapping_dump_type_t type;
} mapping_dump_args_t;

typedef struct cmd_args_s {
    bool valid;
    union {
        mapping_dump_args_t mapping_dump;
        pds_obj_key_t port_id;
    };
} cmd_args_t;

typedef enum cli_cmd_e {
    CLI_CMD_START = 0,
    CLI_CMD_MAPPING_DUMP = CLI_CMD_START,
    CLI_CMD_INTR_DUMP,
    CLI_CMD_INTR_CLEAR,
    CLI_CMD_API_ENGINE_STATS_DUMP,
    CLI_CMD_FLOW_DUMP,
    CLI_CMD_STORE_STATS_DUMP,
    CLI_CMD_NAT_PB_DUMP,
    CLI_CMD_NACL_DUMP,
    CLI_CMD_PORT_FSM_DUMP,
    CLI_CMD_MAX = 255,
} cli_cmd_t;

typedef struct cmd_ctxt_s {
    int fd;           // File descriptor
    cli_cmd_t cmd;    // CLI command
    cmd_args_t args;  // Command arguments
} cmd_ctxt_t;

namespace debug {

typedef void (*table_stats_get_cb_t)(pds_table_stats_t *stats, void *ctxt);
typedef void (*pb_stats_get_cb_t) (pds_pb_debug_stats_t *stats, void *ctxt);
typedef void (*meter_stats_get_cb_t) (pds_meter_debug_stats_t *stats, void *ctxt);
typedef void (*session_stats_get_cb_t) (uint32_t idx, pds_session_debug_stats_t *stats, void *ctxt);
typedef void (*session_get_cb_t) (void *ctxt);
typedef void (*flow_get_cb_t) (ftlite::internal::ipv4_entry_t *ipv4_entry,
                               ftlite::internal::ipv6_entry_t *ipv6_entry,
                               void *ctxt);
sdk_ret_t pds_clock_frequency_update(pds_clock_freq_t freq);
sdk_ret_t pds_arm_clock_frequency_update(pds_clock_freq_t freq);
sdk_ret_t pds_get_system_temperature(pds_system_temperature_t *temp);
sdk_ret_t pds_get_system_power(pds_system_power_t *pow);
sdk_ret_t pds_table_stats_get(table_stats_get_cb_t cb, void *ctxt);
sdk_ret_t pds_handle_cmd(cmd_ctxt_t *ctxt);
sdk_ret_t pds_llc_setup(sdk::asic::pd::llc_counters_t *llc_args);
sdk_ret_t pds_llc_get(sdk::asic::pd::llc_counters_t *llc_args);
sdk_ret_t pds_pb_stats_get(debug::pb_stats_get_cb_t cb, void *ctxt);
sdk_ret_t pds_meter_stats_get(debug::meter_stats_get_cb_t cb, uint32_t lowidx, uint32_t highidx, void *ctxt);
sdk_ret_t pds_session_stats_get(debug::session_stats_get_cb_t cb, uint32_t lowidx, uint32_t highidx, void *ctxt);
sdk_ret_t pds_session_get(debug::session_get_cb_t cb, void *ctxt);
sdk_ret_t pds_flow_get(debug::flow_get_cb_t cb, void *ctxt);
sdk_ret_t pds_flow_summary_get(pds_flow_stats_summary_t *flow_stats);
sdk_ret_t pds_session_clear(uint32_t idx);
sdk_ret_t pds_flow_clear(pds_flow_key_t key);
sdk_ret_t pds_fte_api_stats_get(void);
sdk_ret_t pds_fte_api_stats_clear(void);
sdk_ret_t pds_fte_table_stats_get(void);
sdk_ret_t pds_fte_table_stats_clear(void);
sdk_ret_t pds_slab_get(api::state_walk_cb_t cb, void *ctxt);
sdk_ret_t dump_interrupts(int fd);
sdk_ret_t clear_interrupts(int fd);
/**
 * @brief    start aacs server
 * @param[in]    aacs_server_port     AACS server port
 * @return       SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t start_aacs_server(uint32_t aacs_server_port);

/**
 * @brief    stop aacs server
 * @return       SDK_RET_OK on success, failure status code on error
 */
sdk_ret_t stop_aacs_server(void);

}    // namespace debug

/// \endcond
#endif    // __INCLUDE_API_PDS_DEBUG_HPP__
