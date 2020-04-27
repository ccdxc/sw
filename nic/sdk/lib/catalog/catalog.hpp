// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __CATALOG_HPP__
#define __CATALOG_HPP__

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"
#include "include/sdk/platform.hpp"

namespace sdk {
namespace lib {

using boost::property_tree::ptree;

#define MAX_ASICS          1
#define MAX_ASIC_PORTS     9
#define MAX_LOGICAL_PORTS  MAX_ASIC_PORTS
#define MAX_FP_PORTS       3
#define MAX_PORT_LANES     4
#define MAX_SERDES         9
#define SERDES_SBUS_START  34
#define MAX_PORT_SPEEDS    8
#define MAX_BO_MODES       4
#define DEFAULT_CATALOG_PATH    "/nic/conf/"
// TODO only applicable for Capri
#define MGMT_BASE_PORT     9
#define MAX_CLOCK_FREQ     2
#define MAX_PCIE_PORTSPECS 8

typedef enum card_id_e {
    CARD_ID_NAPLES100,
    CARD_ID_NAPLES25,
    CARD_ID_NAPLES25_SWM
} card_id_t;

typedef enum mac_mode_e {
    MAC_MODE_1x100g,
    MAC_MODE_1x40g,
    MAC_MODE_1x50g,
    MAC_MODE_2x40g,
    MAC_MODE_2x50g,
    MAC_MODE_1x50g_2x25g,
    MAC_MODE_2x25g_1x50g,
    MAC_MODE_4x25g,
    MAC_MODE_4x10g,
    MAC_MODE_4x1g,
    MAC_MODE_MAX
} mac_mode_t;

typedef struct serdes_info_s {
    uint32_t sbus_divider;
    uint32_t tx_slip_value;
    uint32_t rx_slip_value;
    uint32_t width;
    uint8_t  tx_pol;
    uint8_t  rx_pol;
    uint8_t  rx_term;
    uint32_t amp;
    uint32_t pre;
    uint32_t post;
} serdes_info_t;

typedef struct ch_profile_ {
   uint32_t ch_mode;
   uint32_t speed;
   uint32_t port_enable;
} ch_profile_t;

typedef struct mac_profile_ {
   uint32_t     glbl_mode;
   uint32_t     channel;
   uint32_t     tdm_slot;
   ch_profile_t ch_profile[MAX_PORT_LANES];
} mac_profile_t;

typedef struct catalog_logical_port_s {
    uint32_t          asic;
    uint32_t          asic_port;
} catalog_logical_port_t;

typedef struct catalog_logical_oob_port_s {
    uint32_t          phy_id;
    uint32_t          hw_port;
    port_speed_t      speed;
    bool              auto_neg_enable;
} catalog_logical_oob_port_t;

typedef struct catalog_fp_port_s {
    uint32_t    breakout_modes;   // bitmap of breakout modes
    port_type_t type;             // port type
    uint32_t    num_lanes;
    port_admin_state_t admin_state;
    port_speed_t speed;
    port_fec_type_t fec_type;
} catalog_fp_port_t;

typedef struct catalog_asic_port_s {
    uint32_t    mac_id;
    uint32_t    mac_ch;
    uint32_t    sbus_addr;
    uint32_t    tm_port;
} catalog_asic_port_t;

typedef struct catalog_asic_s {
    sdk::platform::asic_type_t type;
    uint32_t     max_ports;
    catalog_asic_port_t ports[MAX_ASIC_PORTS];
} catalog_asic_t;

typedef struct catalog_voltage_s {
    uint32_t     startup_arm;
    uint32_t     startup_vdd;
} catalog_voltage_t;

typedef struct aacs_info_s {
    uint8_t                  server_en;                         // enable aacs server
    uint8_t                  connect;                           // connect to aacs server
    std::string              server_ip;                         // aacs server ip
    uint32_t                 server_port;                       // aacs server port
} aacs_info_t;

typedef struct catalog_clock_info_s {
    uint16_t      clock_freq;           // Asic Clock freq
    uint64_t      clock_multiplier;     // Multiplier based on the clock frequency
} catalog_clock_info_t;

typedef struct catalog_pcie_portspec_s {
    uint8_t     host;
    uint8_t     port;
    uint8_t     gen;
    uint8_t     width;
} catalog_pcie_portspec_t;

typedef struct catalog_s {
    std::string                catalog_file;                          // catalog file name with absolute path
    card_id_t                  card_id;                               // card id for the board
    uint32_t                   max_mpu_per_stage;                     // max MPU per pipeline stage
    uint32_t                   mpu_trace_size;                        // MPU trace size
    uint64_t                   cores_mask;                            // mask of all control/data cores
    std::string                form_factor;                           // Form factor of the card
    uint32_t                   cpld_id;                               // CPLD ID on this card
    uint32_t                   emmc_size;                             // eMMC size on card
    uint32_t                   memory_size;                           // Total Memory on card
    uint32_t                   num_asics;                             // number of asics on the board
    uint32_t                   num_logical_ports;                     // number of logical port in the board
    uint32_t                   num_logical_oob_ports;                 // number of logical oob port in the board
    uint32_t                   num_fp_ports;                          // number of front panel ports in the board
    bool                       access_mock_mode;                      // do not access HW, dump only reads/writes
    catalog_asic_t             asics[MAX_ASICS];                      // per asic information
    catalog_logical_port_t     logical_ports[MAX_LOGICAL_PORTS];      // per port information
    catalog_logical_oob_port_t logical_oob_ports[MAX_LOGICAL_PORTS];  // per oob port information
    catalog_fp_port_t          fp_ports[MAX_FP_PORTS];                // per port information
    mac_profile_t              mac_profiles[MAC_MODE_MAX];            // MAC profiles
    mac_profile_t              mgmt_mac_profiles[MAC_MODE_MAX];       // MGMT MAC profiles
    catalog_voltage_t          voltages;                              // Voltage parameters for the board.
    uint32_t                   halfclock_hbmtemp;                     // temp limit when system goes to half clock
    uint32_t                   aux_fan;                               // card has auxilary fan
    uint32_t                   aux_fan_threshold;                     // temp at which the auilary fan needs to turn on

    // pcie parameters
    uint16_t                   pcie_vendorid;                         // pcie vendor id
    uint16_t                   pcie_subvendorid;                      // pcie subvendor id
    uint16_t                   pcie_subdeviceid;                      // pcie subdevice id
    uint8_t                    pcie_long_lived;                       // aux powered swm card
    uint32_t                   pcie_clock_freq;                       // core clock freq
    uint32_t                   pcie_nportspecs;                       // pcie number of portspecs[]
    catalog_pcie_portspec_t    pcie_portspecs[MAX_PCIE_PORTSPECS];    // pcie port info

    // serdes parameters
    aacs_info_t                aacs_info;                             // avago aacs info
    uint32_t                   serdes_jtag_id;                        // jtag for serdes
    uint8_t                    num_sbus_rings;                        // number of sbus rings on chip
    uint8_t                    sbm_clk_div;                           // SBUS master clock divider
    uint32_t                   serdes_build_id;                       // serdes FW build ID
    uint32_t                   serdes_rev_id;                         // serdes FW rev ID
    std::string                serdes_fw_file;                        // serdes FW file
    serdes_info_t              serdes[MAX_SERDES]
                                     [MAX_PORT_SPEEDS]
                                     [sdk::types::CABLE_TYPE_MAX];
    uint8_t                    num_clock_info;                        // number of clock multipliers
    catalog_clock_info_t       clock_info[MAX_CLOCK_FREQ];            // Clock info for P4 adjustments
} catalog_t;

class catalog {
public:
    static catalog *factory(std::string catalog_file_path="",
                            std::string catalog_file_name="",
                            platform_type_t platform = platform_type_t::PLATFORM_TYPE_NONE);

    static void destroy(catalog *clog);
    static sdk_ret_t get_ptree_(std::string& catalog_file, ptree& prop_tree);
    static int logical_port_to_phy_port(uint32_t logical_port);
    static uint32_t ifindex_to_logical_port(uint32_t ifindex);
    static uint32_t ifindex_to_phy_port(uint32_t ifindex);
    static uint32_t logical_port_to_ifindex(uint32_t logical_port);
    static std::string logical_port_to_str(
                        uint32_t logical_port, port_type_t port_type);
    static uint8_t slot(void) { return 1; }

    // catalog to sdk conversion
    static port_speed_t catalog_speed_to_port_speed(std::string speed);
    static port_type_t catalog_type_to_port_type(std::string type);
    static port_fec_type_t catalog_fec_type_to_port_fec_type(std::string type);
    static port_admin_state_t catalog_admin_st_to_port_admin_st(std::string admin_state);

    static platform_type_t catalog_platform_type_to_platform_type(
                                            std::string platform_type);
    static bool catalog_str_to_bool(std::string val);

    // copp policer config
    sdk_ret_t get_child_str(std::string path, std::string& child_str);

    // catalog globals
    catalog_t *catalog_db(void) { return &catalog_db_; }
    uint64_t cores_mask (void) const { return catalog_db_.cores_mask; }
    card_id_t card_id(void) { return catalog_db_.card_id; }
    bool is_card_naples25(void);
    bool is_card_naples25_swm(void);
    uint32_t cpld_id() { return catalog_db_.cpld_id; }
    bool access_mock_mode(void) { return catalog_db_.access_mock_mode; }
    uint32_t max_mpu_per_stage(void) const {
        return catalog_db_.max_mpu_per_stage;
    }
    uint32_t mpu_trace_size (void) const {
        return catalog_db_.mpu_trace_size;
    }
    uint32_t startup_vdd (void) const { return catalog_db_.voltages.startup_vdd; }
    uint32_t startup_arm (void) const { return catalog_db_.voltages.startup_arm; }
    uint32_t hbmtemperature_threshold (void) const { return catalog_db_.halfclock_hbmtemp; }
    uint32_t aux_fan (void) const { return catalog_db_.aux_fan; }
    uint32_t aux_fan_threshold (void) const { return catalog_db_.aux_fan_threshold; }
    uint32_t memory_capacity(void) const { return catalog_db_.memory_size; }
    std::string memory_capacity_str(void) const {
        if (catalog_db_.memory_size == 8) {
            return "8g";
        } else if (catalog_db_.memory_size == 4) {
            return "4g";
        }
        // 8G by default (e.g. sim environment)
        return "8g";
    }

    // lookups based on tm_port
    uint32_t tm_port_to_ifindex(uint32_t tm_port);

    // lookups based on ifindex
    uint32_t ifindex_to_tm_port(uint32_t ifindex);

    // lookups based on asic
    sdk::platform::asic_type_t asic_type(uint32_t asic) {
        return catalog_db_.asics[asic].type;
    }

    // lookups based on asic, asic_port
    uint32_t num_asic_ports(uint32_t asic) {
        return catalog_db_.asics[asic].max_ports;
    }
    uint32_t sbus_addr_asic_port(uint32_t asic, uint32_t asic_port) {
        return catalog_db_.asics[asic].ports[asic_port].sbus_addr;
    }
    uint32_t sbus_addr(uint32_t asic_num, uint32_t asic_port, uint32_t lane);
    uint32_t asic_port_to_mac_id(uint32_t asic, uint32_t asic_port);
    uint32_t asic_port_to_mac_ch(uint32_t asic, uint32_t asic_port);

    // lookups based on logical oob ports
    uint32_t num_logical_oob_ports(void) const {
        return catalog_db_.num_logical_oob_ports;
    }
    uint32_t oob_phy_id(uint32_t logical_oob_port);
    uint32_t oob_hw_port(uint32_t logical_oob_port);
    port_speed_t oob_speed(uint32_t logical_oob_port);
    bool oob_auto_neg_enable(uint32_t logical_oob_port);

    // lookups based on logical port
    uint32_t num_logical_ports(void) const { return catalog_db_.num_logical_ports; }
    uint32_t sbus_addr(uint32_t logical_port, uint32_t lane);
    uint32_t mac_id(uint32_t logical_port, uint32_t lane);
    uint32_t mac_ch(uint32_t logical_port, uint32_t lane);
    uint32_t logical_port_to_tm_port(uint32_t logical_port);

    // return number of physical ports (fp_ports == phy_ports)
    uint32_t num_fp_ports(void) const { return catalog_db_.num_fp_ports; }
    port_type_t  port_type_fp(uint32_t fp_port);
    port_admin_state_t  admin_state_fp(uint32_t fp_port);
    port_speed_t port_speed_fp(uint32_t fp_port);
    port_fec_type_t port_fec_type_fp(uint32_t fp_port);
    uint32_t     num_lanes_fp(uint32_t fp_port);
    uint32_t     breakout_modes(uint32_t fp_port);

    // MX configs
    uint32_t     glbl_mode(mac_mode_t mac_mode);
    uint32_t     ch_mode(mac_mode_t mac_mode, uint32_t ch);
    uint32_t     glbl_mode_mgmt(mac_mode_t mac_mode);
    uint32_t     ch_mode_mgmt(mac_mode_t mac_mode, uint32_t ch);

    // pcie configs
    uint16_t     pcie_vendorid(void) { return catalog_db_.pcie_vendorid; }
    uint16_t     pcie_subvendorid(void) { return catalog_db_.pcie_subvendorid;}
    uint16_t     pcie_subdeviceid(void) { return catalog_db_.pcie_subdeviceid;}
    uint8_t      pcie_long_lived(void) { return catalog_db_.pcie_long_lived; }
    uint32_t     pcie_clock_freq(void) { return catalog_db_.pcie_clock_freq; }
    uint32_t     pcie_nportspecs(void) { return catalog_db_.pcie_nportspecs; }
    uint8_t      pcie_host(uint32_t idx)
                        { return catalog_db_.pcie_portspecs[idx].host; }
    uint8_t      pcie_port(uint32_t idx)
                        { return catalog_db_.pcie_portspecs[idx].port; }
    uint8_t      pcie_gen(uint32_t idx)
                        { return catalog_db_.pcie_portspecs[idx].gen; }
    uint8_t      pcie_width(uint32_t idx)
                        { return catalog_db_.pcie_portspecs[idx].width; }

    // serdes configs
    uint32_t     jtag_id(void) { return catalog_db_.serdes_jtag_id;  }
    uint32_t     num_sbus_rings(void) { return catalog_db_.num_sbus_rings;  }
    uint32_t     sbm_clk_div(void) { return catalog_db_.sbm_clk_div;     }
    uint32_t     serdes_build_id(void) { return catalog_db_.serdes_build_id; }
    uint32_t     serdes_rev_id(void) { return catalog_db_.serdes_rev_id;   }
    std::string  serdes_fw_file(void) { return catalog_db_.serdes_fw_file;  }
    uint8_t aacs_server_en(void) { return catalog_db_.aacs_info.server_en; }
    uint8_t aacs_connect(void)   { return catalog_db_.aacs_info.connect; }
    std::string  aacs_server_ip(void) {
        return catalog_db_.aacs_info.server_ip;
    }
    uint32_t aacs_server_port(void) {
        return catalog_db_.aacs_info.server_port;
    }
    serdes_info_t* serdes_info_get(uint32_t sbus_addr,
                                   uint32_t port_speed,
                                   uint32_t cable_type);

    uint32_t num_clock_info(void) const { return catalog_db_.num_clock_info; }
    uint64_t clock_get_multiplier(uint16_t freq);

private:
    catalog_t    catalog_db_;   // whole catalog database

private:
    catalog() {};
    ~catalog();

    sdk_ret_t init(std::string &catalog_file);

    // populate the board level config
    sdk_ret_t populate_catalog(std::string &catalog_file, ptree &prop_tree);

    // populate asic level config
    sdk_ret_t populate_asic(ptree::value_type &asic,
                            catalog_asic_t *asic_p);

    // populate the board voltages
    sdk_ret_t populate_voltages(ptree &prop_tree);

    // populate config for all asics
    sdk_ret_t populate_asics(ptree &prop_tree);

    // populate asic port level config
    sdk_ret_t populate_asic_port(ptree::value_type &asic_port,
                                 catalog_asic_port_t *asic_port_p);

    // populate config for all ports
    sdk_ret_t populate_asic_ports(ptree::value_type &asic,
                                  catalog_asic_t *asic_p);

    // populate logical port level config
    sdk_ret_t populate_logical_port(ptree::value_type &logical_port,
                                    catalog_logical_port_t *logical_port_p);

    // populate config for all logical ports
    sdk_ret_t populate_logical_ports(ptree &prop_tree);

    // populate logical oob port level config
    sdk_ret_t populate_logical_oob_port(
                            ptree::value_type &logical_oob_port,
                            catalog_logical_oob_port_t *logical_oob_port_p);

    // populate config for all logical oob ports
    sdk_ret_t populate_logical_oob_ports(ptree &prop_tree);

    // populate fp port level config
    sdk_ret_t populate_fp_port(ptree::value_type &fp_port,
                               catalog_fp_port_t *fp_port_p);

    // populate config for all fp ports
    sdk_ret_t populate_fp_ports(ptree &prop_tree);

    // catalog to sdk conversion
    port_breakout_mode_t parse_breakout_mode(std::string);

    // populate MAC configs
    sdk_ret_t populate_mac_profile(mac_profile_t *mac_profile,
                                   std::string   str,
                                   ptree         &prop_tree);
    sdk_ret_t populate_mgmt_mac_profiles(ptree &prop_tree);
    sdk_ret_t populate_mac_profiles(ptree &prop_tree);
    sdk_ret_t populate_mac_ch_profile(ch_profile_t *ch_profile,
                                      std::string  profile_str,
                                      ptree        &prop_tree);

    // populate pcie settings
    sdk_ret_t populate_pcie(ptree &prop_tree);

    // populate serdes configs
    sdk_ret_t populate_serdes(char *dir_name, ptree &prop_tree);
    sdk_ret_t parse_serdes_file(std::string& serdes_file);
    sdk_ret_t parse_serdes(ptree &prop_tree);

    catalog_logical_port_t *logical_port_internal(uint32_t port);
    catalog_logical_oob_port_t *logical_oob_port_internal(uint32_t port);
    uint32_t  serdes_index_get(uint32_t sbus_addr);
    uint8_t   cable_type_get(std::string cable_type_str);
    card_id_t catalog_board_type_to_sdk_card_id(std::string card_id);
    void logical_port_to_asic_port(uint32_t logical_port,
                                   uint32_t *asic, uint32_t *asic_port);
    // populate clock info for different frequencies
    sdk_ret_t populate_clock_info(ptree &prop_tree);
};

}    // namespace lib
}    // namespace sdk

using sdk::lib::mac_mode_t;
using sdk::lib::serdes_info_t;
using sdk::lib::catalog;
using sdk::lib::card_id_t;

#define MAC_MODE_1x100g mac_mode_t::MAC_MODE_1x100g
#define MAC_MODE_1x40g mac_mode_t::MAC_MODE_1x40g
#define MAC_MODE_1x50g mac_mode_t::MAC_MODE_1x50g
#define MAC_MODE_2x40g mac_mode_t::MAC_MODE_2x40g
#define MAC_MODE_2x50g mac_mode_t::MAC_MODE_2x50g
#define MAC_MODE_1x50g_2x25g mac_mode_t::MAC_MODE_1x50g_2x25g
#define MAC_MODE_2x25g_1x50g mac_mode_t::MAC_MODE_2x25g_1x50g
#define MAC_MODE_4x25g mac_mode_t::MAC_MODE_4x25g
#define MAC_MODE_4x10g mac_mode_t::MAC_MODE_4x10g
#define MAC_MODE_4x1g mac_mode_t::MAC_MODE_4x1g

#endif    //__CATALOG_HPP__
