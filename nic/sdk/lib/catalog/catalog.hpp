// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __CATALOG_HPP__
#define __CATALOG_HPP__

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"

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

typedef enum card_id_e {
    CARD_ID_NAPLES100,
    CARD_ID_NAPLES25
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

typedef struct catalog_fp_port_s {
    uint32_t    breakout_modes;   // bitmap of breakout modes
    port_type_t type;             // port type
    uint32_t    num_lanes;
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

typedef struct aacs_info_s {
    uint8_t                  server_en;                         // enable aacs server
    uint8_t                  connect;                           // connect to aacs server
    std::string              server_ip;                         // aacs server ip
    uint32_t                 server_port;                       // aacs server port
} aacs_info_t;

typedef struct catalog_s {
    std::string              catalog_file;                      // catalog file name with absolute path
    card_id_t                card_id;                           // card id for the board
    uint32_t                 max_mpu_per_stage;                 // max MPU per pipeline stage
    uint32_t                 mpu_trace_size;                    // MPU trace size
    uint64_t                 cores_mask;                        // mask of all control/data cores
    std::string              form_factor;                       // Form factor of the card
    uint32_t                 cpld_id;                           // CPLD ID on this card
    uint32_t                 emmc_size;                         // eMMC size on card
    uint32_t                 memory_size;                       // Total Memory on card
    uint32_t                 num_asics;                         // number of asics on the board
    uint32_t                 num_logical_ports;                 // number of logical port in the board
    uint32_t                 num_fp_ports;                      // number of front panel ports in the board
    platform_type_t          platform_type;                     // platform type
    bool                     access_mock_mode;                  // do not access HW, dump only reads/writes
    catalog_asic_t           asics[MAX_ASICS];                  // per asic information
    catalog_logical_port_t   logical_ports[MAX_LOGICAL_PORTS];  // per port information
    catalog_fp_port_t        fp_ports[MAX_FP_PORTS];            // per port information
    mac_profile_t            mac_profiles[MAC_MODE_MAX];        // MAC profiles
    mac_profile_t            mgmt_mac_profiles[MAC_MODE_MAX];   // MGMT MAC profiles

    // pcie parameters
    uint8_t                  pcie_hostport_mask;                // host ports enabled
    uint8_t                  pcie_gen;                          // pcie speed gen 1-4
    uint8_t                  pcie_width;                        // pcie lane width 1-16
    uint16_t                 pcie_subdeviceid;                  // pcie subdevice id

    // serdes parameters
    aacs_info_t              aacs_info;                         // avago aacs info
    uint32_t                 serdes_jtag_id;                    // jtag for serdes
    uint8_t                  num_sbus_rings;                    // number of sbus rings on chip
    uint8_t                  sbm_clk_div;                       // SBUS master clock divider
    uint32_t                 serdes_build_id;                   // serdes FW build ID
    uint32_t                 serdes_rev_id;                     // serdes FW rev ID
    std::string              serdes_fw_file;                    // serdes FW file
    serdes_info_t            serdes[MAX_SERDES]
                                   [MAX_PORT_SPEEDS]
                                   [sdk::types::CABLE_TYPE_MAX];
} catalog_t;

class catalog {
public:
    static catalog *factory(std::string catalog_file_path="", std::string catalog_file_name="", platform_type_t platform = platform_type_t::PLATFORM_TYPE_NONE);

    static void destroy(catalog *clog);
    static sdk_ret_t get_ptree_(std::string& catalog_file, ptree& prop_tree);
    static int logical_port_to_phy_port(uint32_t logical_port);
    static uint32_t ifindex_to_logical_port(uint32_t ifindex);
    static uint32_t logical_port_to_ifindex(uint32_t logical_port);
    uint32_t ifindex_to_tm_port(uint32_t ifindex);

    // catalog to sdk conversion
    static port_speed_t catalog_speed_to_port_speed(std::string speed);
    static port_type_t catalog_type_to_port_type(std::string type);
    static port_fec_type_t catalog_fec_type_to_port_fec_type(std::string type);
    static platform_type_t catalog_platform_type_to_platform_type(
                                            std::string platform_type);
    // copp policer config
    sdk_ret_t get_child_str(std::string path, std::string& child_str);

    // catalog globals
    catalog_t *catalog_db(void) { return &catalog_db_; }
    uint64_t cores_mask (void) const { return catalog_db_.cores_mask; }
    card_id_t card_id(void) { return catalog_db_.card_id; }
    platform_type_t platform_type(void) const {
        return catalog_db_.platform_type;
    }
    uint32_t cpld_id() { return catalog_db_.cpld_id; }
    bool access_mock_mode(void) { return catalog_db_.access_mock_mode; }
    uint32_t max_mpu_per_stage(void) const {
        return catalog_db_.max_mpu_per_stage;
    }
    uint32_t mpu_trace_size (void) const {
        return catalog_db_.mpu_trace_size;
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

    // lookups based on logical port
    uint32_t num_logical_ports(void) const { return catalog_db_.num_logical_ports; }
    uint32_t sbus_addr(uint32_t logical_port, uint32_t lane);
    uint32_t mac_id(uint32_t logical_port, uint32_t lane);
    uint32_t mac_ch(uint32_t logical_port, uint32_t lane);
    uint32_t logical_port_to_tm_port(uint32_t logical_port);

    // return number of physical ports (fp_ports == phy_ports)
    uint8_t slot(void) { return 1; }
    uint32_t num_fp_ports(void) const { return catalog_db_.num_fp_ports; }
    port_type_t  port_type_fp(uint32_t fp_port);
    uint32_t     num_lanes_fp(uint32_t fp_port);
    uint32_t     breakout_modes(uint32_t fp_port);

    // MX configs
    uint32_t     glbl_mode(mac_mode_t mac_mode);
    uint32_t     ch_mode(mac_mode_t mac_mode, uint32_t ch);
    uint32_t     glbl_mode_mgmt(mac_mode_t mac_mode);
    uint32_t     ch_mode_mgmt(mac_mode_t mac_mode, uint32_t ch);

    // pcie configs
    uint8_t      pcie_hostport_mask(void) { return catalog_db_.pcie_hostport_mask; }
    uint8_t      pcie_gen(void) { return catalog_db_.pcie_gen; }
    uint8_t      pcie_width(void) { return catalog_db_.pcie_width; }
    uint16_t     pcie_subdeviceid(void) { return catalog_db_.pcie_subdeviceid; }

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

    catalog_logical_port_t *logical_port(uint32_t port);
    uint32_t  serdes_index_get(uint32_t sbus_addr);
    uint8_t   cable_type_get(std::string cable_type_str);
    card_id_t catalog_card_id_to_sdk_card_id(uint32_t card_id);
    void logical_port_to_asic_port(uint32_t logical_port,
                                   uint32_t *asic, uint32_t *asic_port);
};

}    // namespace lib
}    // namespace sdk

using sdk::lib::mac_mode_t;
using sdk::lib::serdes_info_t;
using sdk::lib::catalog;

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
