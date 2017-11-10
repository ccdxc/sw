// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/utils/catalog/catalog.hpp"

namespace hal {
namespace utils {

hal_ret_t
catalog::populate_asic_port(ptree::value_type &asic_port,
                            catalog_asic_port_t *asic_port_p)
{
    asic_port_p->mac_id = asic_port.second.get<uint32_t>("mac_id", 0);
    asic_port_p->mac_ch = asic_port.second.get<uint32_t>("mac_ch", 0);

    return HAL_RET_OK;
}

hal_ret_t
catalog::populate_asic_ports(ptree::value_type &asic,
                             catalog_asic_t *asic_p)
{
    for (ptree::value_type &asic_port : asic.second.get_child("ports")) {
        catalog_asic_port_t *asic_port_p =
                &asic_p->ports[asic_port.second.get<uint32_t>("port_num", 0)];

        populate_asic_port(asic_port, asic_port_p);
    }

    return HAL_RET_OK;
}

hal_ret_t
catalog::populate_asic(ptree::value_type &asic,
                       catalog_asic_t *asic_p)
{
    asic_p->name = asic.second.get<std::string>("name", "");
    asic_p->max_ports = asic.second.get<uint32_t>("max_ports", 0);

    populate_asic_ports(asic, asic_p);

    return HAL_RET_OK;
}

hal_ret_t
catalog::populate_asics(ptree &prop_tree)
{
    for (ptree::value_type &asic : prop_tree.get_child("asics")) {
        catalog_asic_t *asic_p =
                &catalog_db_.asics[asic.second.get<uint32_t>("num", 0)];

        populate_asic(asic, asic_p);
    }

    return HAL_RET_OK;
}

::port::PortSpeed
catalog::catalog_speed_to_port_speed(std::string speed)
{
    if (speed == "10G") {
        return ::port::PORT_SPEED_10G;
    }

    if (speed == "25G") {
        return ::port::PORT_SPEED_25G;
    }

    if (speed == "100G") {
        return ::port::PORT_SPEED_100G;
    }

    return ::port::PORT_SPEED_NONE;
}

::port::PortType
catalog::catalog_type_to_port_type(std::string type)
{
    if (type == "eth") {
        return ::port::PORT_TYPE_ETH;
    }

    return ::port::PORT_TYPE_NONE;
}

hal_ret_t
catalog::populate_uplink_port(ptree::value_type &uplink_port,
                              catalog_uplink_port_t *uplink_port_p)
{
    uplink_port_p->asic = uplink_port.second.get<uint32_t>("asic", 0);

    uplink_port_p->asic_port = uplink_port.second.get<uint32_t>("asic_port", 0);

    uplink_port_p->num_lanes = uplink_port.second.get<uint32_t>("num_lanes", 1);

    uplink_port_p->enabled = uplink_port.second.get<bool>("enabled", true);

    std::string speed = uplink_port.second.get<std::string>("speed", "");

    uplink_port_p->speed = catalog::catalog_speed_to_port_speed(speed);

    std::string type = uplink_port.second.get<std::string>("type", "");

    uplink_port_p->type = catalog::catalog_type_to_port_type(type);

    return HAL_RET_OK;
}

hal_ret_t
catalog::populate_uplink_ports(ptree &prop_tree)
{
    for (ptree::value_type &uplink_port : prop_tree.get_child("uplink_ports")) {
        catalog_uplink_port_t *uplink_port_p =
                            &catalog_db_.uplink_ports[
                            uplink_port.second.get<uint32_t>("port_num", 0)];

        populate_uplink_port(uplink_port, uplink_port_p);
    }

    return HAL_RET_OK;
}

platform_type_t
catalog_platform_type_to_platform_type(std::string platform_type)
{
    if (platform_type == "haps") {
        return PLATFORM_TYPE_HAPS;
    }

    return PLATFORM_TYPE_NONE;
}

hal_ret_t
catalog::populate_catalog(ptree &prop_tree)
{
    catalog_db_.card_index = prop_tree.get<uint32_t>("card_index", 0);

    catalog_db_.num_asics = prop_tree.get<uint32_t>("num_asics", 0);

    catalog_db_.num_uplink_ports =
                            prop_tree.get<uint32_t>("num_uplink_ports", 0);

    catalog_db_.vrf_id = prop_tree.get<uint32_t>("vrf_id", 0);

    catalog_db_.access_mock_mode = prop_tree.get<bool>("access_mock_mode", false);

    std::string platform_type =
                            prop_tree.get<std::string>("platform_type", "");

    catalog_db_.platform_type =
                        catalog_platform_type_to_platform_type(platform_type);

    populate_asics(prop_tree);

    populate_uplink_ports(prop_tree);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// initialize an instance of catalog class
//------------------------------------------------------------------------------
hal_ret_t
catalog::init(std::string catalog_file)
{
    ptree prop_tree;
    boost::property_tree::read_json(catalog_file, prop_tree);

    populate_catalog(prop_tree);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// factory method for this class
//------------------------------------------------------------------------------
catalog *
catalog::factory(std::string catalog_file) {
    hal_ret_t  ret;
    catalog    *new_catalog = NULL;

    // make sure file exists
    if (access(catalog_file.c_str(), R_OK) < 0) {
        HAL_TRACE_ERR("{}: config file {} has no read permissions",
                      __FUNCTION__,  catalog_file.c_str());
        return NULL;
    }

    new_catalog = (catalog*)HAL_MALLOC(HAL_MEM_ALLOC_CATALOG, sizeof(catalog));
    if (new_catalog == NULL) {
        return NULL;
    }

    ret = new_catalog->init(catalog_file);
    if (ret != HAL_RET_OK) {
        HAL_FREE(HAL_MEM_ALLOC_CATALOG, new_catalog);
        return NULL;
    }

    return new_catalog;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
catalog::~catalog()
{
}

catalog_uplink_port_t*
catalog::uplink_port(uint32_t port)
{
    return &catalog_db_.uplink_ports[port];
}

catalog_asic_port_t*
catalog::asic_port(uint32_t port)
{
    catalog_uplink_port_t *catalog_uplink_port_p = uplink_port(port);

    uint32_t asic = catalog_uplink_port_p->asic;
    uint32_t asic_port = catalog_uplink_port_p->asic_port;

    return &catalog_db_.asics[asic].ports[asic_port];
}

uint32_t
catalog::num_uplink_ports()
{
    return catalog_db_.num_uplink_ports;
}

uint32_t
catalog::vrf_id()
{
    return catalog_db_.vrf_id;
}

}    // namespace utils
}    // namespace hal
