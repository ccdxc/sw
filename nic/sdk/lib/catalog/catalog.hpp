// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __CATALOG_HPP__
#define __CATALOG_HPP__

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "sdk/base.hpp"

namespace sdk {
namespace lib {

#define MAX_ASICS                                    1
#define MAX_ASIC_PORTS                               8
#define MAX_UPLINK_PORTS                             8

typedef enum port_speed_e {
    PORT_SPEED_NONE    = 0,
    PORT_SPEED_10G     = 1,
    PORT_SPEED_25G     = 2,
    PORT_SPEED_100G    = 3,
} port_speed_t;

typedef enum port_type_e {
    PORT_TYPE_NONE    = 0,
    PORT_TYPE_ETH     = 1,
} port_type_t;

using boost::property_tree::ptree;

typedef struct catalog_uplink_port_s {
    uint32_t          asic;
    uint32_t          asic_port;
    uint32_t          num_lanes;
    bool              enabled;
    port_speed_t      speed;
    port_type_t       type;
} catalog_uplink_port_t;

typedef struct catalog_asic_port_s {
    uint32_t    mac_id;
    uint32_t    mac_ch;
    uint32_t    sbus_addr;
} catalog_asic_port_t;

typedef struct catalog_asic_s {
    std::string  name;
    uint32_t     max_ports;
    catalog_asic_port_t ports[MAX_ASIC_PORTS];
} catalog_asic_t;

typedef enum platform_type_e {
    PLATFORM_TYPE_NONE = 0,
    PLATFORM_TYPE_SIM  = 1,
    PLATFORM_TYPE_HAPS = 2,
    PLATFORM_TYPE_HW   = 3,
} platform_type_t;

typedef struct catalog_s {
    uint32_t                 card_index;                        // card index for the board
    uint32_t                 num_asics;                         // number of asics on the board
    uint32_t                 num_uplink_ports;                  // number of uplinks in the board
    platform_type_t          platform_type;                     // platform type
    bool                     access_mock_mode;                  // do not access HW, dump only reads/writes
    catalog_asic_t           asics[MAX_ASICS];                  // per asic information
    catalog_uplink_port_t    uplink_ports[MAX_UPLINK_PORTS];    // per port information
} catalog_t;

class catalog {
public:
    static catalog *factory(std::string catalog_file);
    static void destroy(catalog *clog);
    static port_speed_t catalog_speed_to_port_speed(std::string speed);
    static port_type_t catalog_type_to_port_type(std::string type);
    static platform_type_t catalog_platform_type_to_platform_type(
                                            std::string platform_type);
    catalog_t *catalog_db(void) { return &catalog_db_; }
    catalog_uplink_port_t *uplink_port(uint32_t uplink_port);
    catalog_asic_port_t *asic_port(uint32_t uplink_port);
    uint32_t num_uplink_ports(void) const { return catalog_db_.num_uplink_ports; }
    platform_type_t platform_type(void) const { return catalog_db_.platform_type; }
    bool access_mock_mode(void) { return catalog_db_.access_mock_mode; }
    uint32_t sbus_addr(uint32_t asic_num, uint32_t asic_port, uint32_t lane);

private:
    catalog_t    catalog_db_;   // whole catalog database

private:
    catalog() {};
    ~catalog();

    sdk_ret_t init(std::string catalog_file);

    // populate the board level config
    sdk_ret_t populate_catalog(ptree &prop_tree);

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

    // populate uplink port level config
    sdk_ret_t populate_uplink_port(ptree::value_type &uplink_port,
                                   catalog_uplink_port_t *uplink_port_p);

    // populate config for all uplink ports
    sdk_ret_t populate_uplink_ports(ptree &prop_tree);
};

}    // namespace lib
}    // namespace sdk

#endif    //__CATALOG_HPP__

