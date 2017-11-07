// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __NIC_UTILS_CATALOG_HPP__
#define __NIC_UTILS_CATALOG_HPP__

#include "nic/include/base.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "nic/gen/proto/hal/port.pb.h"

namespace hal {
namespace utils {

#define MAX_ASICS 1
#define MAX_ASIC_PORTS 2
#define MAX_UPLINK_PORTS 8

using boost::property_tree::ptree;

typedef struct catalog_uplink_port_s {
    uint32_t          asic;
    uint32_t          asic_port;
    uint32_t          num_lanes;
    bool              enabled;
    ::port::PortSpeed speed;
    ::port::PortType  type;
} catalog_uplink_port_t;

typedef struct catalog_asic_port_s {
    uint32_t    mac_id;
    uint32_t    mac_ch;
} catalog_asic_port_t;

typedef struct catalog_asic_s {
    std::string  name;
    uint32_t     max_ports;
    catalog_asic_port_t ports[MAX_ASIC_PORTS];
} catalog_asic_t;

typedef struct catalog_s {
    uint32_t card_index;         // card index for the board
    uint32_t num_asics;          // number of asics on the board
    uint32_t num_uplink_ports;   // number of uplinks in the board
    uint32_t tenant_id;          // tenant ID

    catalog_asic_t asics[MAX_ASICS];
    catalog_uplink_port_t uplink_ports[MAX_UPLINK_PORTS];
} catalog_t;

class catalog {
public:
    static catalog *factory(std::string catalog_file);

    static ::port::PortSpeed catalog_speed_to_port_speed(std::string speed);

    static ::port::PortType catalog_type_to_port_type(std::string type);

    catalog_t *catalog_db() { return &catalog_db_; }

    catalog_uplink_port_t *uplink_port(uint32_t uplink_port);

    catalog_asic_port_t *asic_port(uint32_t uplink_port);

    uint32_t num_uplink_ports();

    uint32_t tenant_id();

    ~catalog();

private:
    catalog_t    catalog_db_;   // catalog instance

private:
    catalog() {};

    hal_ret_t init(std::string catalog_file);

    /* populate the board level config */
    hal_ret_t populate_catalog(ptree &prop_tree);

    /* iterate over asics */
    hal_ret_t populate_asics(ptree &prop_tree);

    /* populate asic level config */
    hal_ret_t populate_asic(ptree::value_type &asic,
                            catalog_asic_t *asic_p);

    /* iterate over asic ports */
    hal_ret_t populate_asic_ports(ptree::value_type &asic,
                                  catalog_asic_t *asic_p);

    /* populate asic port level config */
    hal_ret_t populate_asic_port(ptree::value_type &asic_port,
                                 catalog_asic_port_t *asic_port_p);

    /* iterate over uplink ports */
    hal_ret_t populate_uplink_ports(ptree &prop_tree);

    /* populate uplink port level config */
    hal_ret_t populate_uplink_port(ptree::value_type &uplink_port,
                                   catalog_uplink_port_t *uplink_port_p);
};

}    // namespace utils
}    // namespace hal

#endif    //__NIC_UTILS_CATALOG_HPP__
