// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <sys/sysinfo.h>
#include <libgen.h>
#include "lib/catalog/catalog.hpp"
#include "include/sdk/mem.hpp"
#include "include/sdk/utils.hpp"

namespace sdk {
namespace lib {

sdk_ret_t
catalog::populate_asic_port(ptree::value_type &asic_port,
                            catalog_asic_port_t *asic_port_p)
{
    asic_port_p->mac_id    = asic_port.second.get<uint32_t>("mac_id", 0);
    asic_port_p->mac_ch    = asic_port.second.get<uint32_t>("mac_ch", 0);
    asic_port_p->sbus_addr = asic_port.second.get<uint32_t>("sbus_addr", 0);

    return SDK_RET_OK;
}

sdk_ret_t
catalog::populate_asic_ports(ptree::value_type &asic, catalog_asic_t *asic_p)
{
    for (ptree::value_type &asic_port : asic.second.get_child("ports")) {
        catalog_asic_port_t *asic_port_p =
                &asic_p->ports[asic_port.second.get<uint32_t>("port_num", 0)];

        populate_asic_port(asic_port, asic_port_p);
    }

    return SDK_RET_OK;
}

sdk_ret_t
catalog::populate_asic(ptree::value_type &asic, catalog_asic_t *asic_p)
{
    asic_p->name = asic.second.get<std::string>("name", "");
    asic_p->max_ports = asic.second.get<uint32_t>("max_ports", 0);

    populate_asic_ports(asic, asic_p);

    return SDK_RET_OK;
}

sdk_ret_t
catalog::populate_asics(ptree &prop_tree)
{
    for (ptree::value_type &asic : prop_tree.get_child("asics")) {
        catalog_asic_t *asic_p =
                &catalog_db_.asics[asic.second.get<uint32_t>("num", 0)];

        populate_asic(asic, asic_p);
    }

    return SDK_RET_OK;
}

port_speed_t
catalog::catalog_speed_to_port_speed(std::string speed)
{
    if (speed == "1G") {
        return port_speed_t::PORT_SPEED_1G;
    } else if (speed == "10G") {
        return port_speed_t::PORT_SPEED_10G;
    } else if (speed == "25G") {
        return port_speed_t::PORT_SPEED_25G;
    } else if (speed == "40G") {
        return port_speed_t::PORT_SPEED_40G;
    } else if (speed == "50G") {
        return port_speed_t::PORT_SPEED_50G;
    } else if (speed == "100G") {
        return port_speed_t::PORT_SPEED_100G;
    }

    return port_speed_t::PORT_SPEED_NONE;
}

port_type_t
catalog::catalog_type_to_port_type(std::string type)
{
    if (type == "eth") {
        return port_type_t::PORT_TYPE_ETH;
    } else if (type == "mgmt") {
        return port_type_t::PORT_TYPE_MGMT;
    }

    return port_type_t::PORT_TYPE_NONE;
}

port_fec_type_t
catalog::catalog_fec_type_to_port_fec_type (std::string type)
{
    if (type == "rs") {
        return port_fec_type_t::PORT_FEC_TYPE_RS;
    } else if (type == "fc") {
        return port_fec_type_t::PORT_FEC_TYPE_FC;
    }

    return port_fec_type_t::PORT_FEC_TYPE_NONE;
}

port_breakout_mode_t
catalog::parse_breakout_mode(std::string breakout_mode)
{
    if (breakout_mode == "4x25G") {
        return port_breakout_mode_t::BREAKOUT_MODE_4x25G;
    } else if (breakout_mode == "4x10G") {
        return port_breakout_mode_t::BREAKOUT_MODE_4x10G;
    } else if (breakout_mode == "2x50G") {
        return port_breakout_mode_t::BREAKOUT_MODE_2x50G;
    }

    return port_breakout_mode_t::BREAKOUT_MODE_NONE;
}

sdk_ret_t
catalog::populate_fp_port(ptree::value_type &fp_port,
                          catalog_fp_port_t *fp_port_p)
{
    std::string type = fp_port.second.get<std::string>("type", "");
    fp_port_p->type = catalog_type_to_port_type(type);

    fp_port_p->num_lanes = fp_port.second.get<uint32_t>("num_lanes", 0);

    for (ptree::value_type &breakout_mode :
                            fp_port.second.get_child("breakout_modes")) {
        port_breakout_mode_t mode = parse_breakout_mode(
                        breakout_mode.second.get_value<std::string>());
        fp_port_p->breakout_modes |= (1 << static_cast<uint32_t>(mode));
    }

    return SDK_RET_OK;
}

sdk_ret_t
catalog::populate_fp_ports(ptree &prop_tree)
{
    for (ptree::value_type &fp_port : prop_tree.get_child("fp_ports")) {
        catalog_fp_port_t *fp_port_p =
                            &catalog_db_.fp_ports[
                            fp_port.second.get<uint32_t>("port_num", 0) - 1];

        populate_fp_port(fp_port, fp_port_p);
    }

    return SDK_RET_OK;
}

sdk_ret_t
catalog::populate_uplink_port(ptree::value_type &uplink_port,
                              catalog_uplink_port_t *uplink_port_p)
{
    uplink_port_p->asic = uplink_port.second.get<uint32_t>("asic", 0);
    uplink_port_p->asic_port = uplink_port.second.get<uint32_t>("asic_port", 0);
    uplink_port_p->num_lanes = uplink_port.second.get<uint32_t>("num_lanes", 1);
    uplink_port_p->enabled = uplink_port.second.get<bool>("enabled", true);

    std::string speed = uplink_port.second.get<std::string>("speed", "");
    uplink_port_p->speed = catalog_speed_to_port_speed(speed);

    std::string fec_type = uplink_port.second.get<std::string>("fec", "");
    uplink_port_p->fec_type = catalog_fec_type_to_port_fec_type(fec_type);

    std::string type = uplink_port.second.get<std::string>("type", "");
    uplink_port_p->type = catalog_type_to_port_type(type);

    return SDK_RET_OK;
}

sdk_ret_t
catalog::populate_uplink_ports(ptree &prop_tree)
{
    for (ptree::value_type &uplink_port : prop_tree.get_child("uplink_ports")) {
        catalog_uplink_port_t *uplink_port_p =
                            &catalog_db_.uplink_ports[
                            uplink_port.second.get<uint32_t>("port_num", 0) - 1];

        populate_uplink_port(uplink_port, uplink_port_p);
    }

    return SDK_RET_OK;
}

platform_type_t
catalog::catalog_platform_type_to_platform_type(std::string platform_type)
{
    if (platform_type == "haps") {
        return platform_type_t::PLATFORM_TYPE_HAPS;
    } else if (platform_type == "hw") {
        return platform_type_t::PLATFORM_TYPE_HW;
    } else if (platform_type == "sim") {
        return platform_type_t::PLATFORM_TYPE_SIM;
    } else if (platform_type == "mock") {
        return platform_type_t::PLATFORM_TYPE_MOCK;
    } else if (platform_type == "zebu") {
        return platform_type_t::PLATFORM_TYPE_ZEBU;
    }

    return platform_type_t::PLATFORM_TYPE_NONE;
}

sdk_ret_t
catalog::populate_qos_profile(ptree &prop_tree)
{
    qos_profile_t &qos_profile = catalog_db_.qos_profile;

    qos_profile.sw_init_enable = prop_tree.get<bool>("qos.profile.sw_init_enable", false);
    qos_profile.sw_cfg_write_enable = prop_tree.get<bool>("qos.profile.sw_cfg_write_enable", false);
    qos_profile.jumbo_mtu = prop_tree.get<uint32_t>("qos.profile.jumbo_mtu", 0);
    qos_profile.num_uplink_qs = prop_tree.get<uint32_t>("qos.profile.num_uplink_qs", 0);
    qos_profile.num_p4ig_qs = prop_tree.get<uint32_t>("qos.profile.num_p4ig_qs", 0);
    qos_profile.num_p4eg_qs = prop_tree.get<uint32_t>("qos.profile.num_p4eg_qs", 0);
    qos_profile.num_dma_qs = prop_tree.get<uint32_t>("qos.profile.num_dma_qs", 0);

    return SDK_RET_OK;
}

sdk_ret_t
catalog::populate_mac_ch_profile(ch_profile_t *ch_profile,
                                 std::string  profile_str,
                                 ptree        &prop_tree)
{
    ch_profile->speed = prop_tree.get<uint32_t>(profile_str + ".speed", 0);

    std::string val = prop_tree.get<std::string>(profile_str + ".ch_mode", "");
    ch_profile->ch_mode = strtoul(val.c_str(), NULL, 16);

    return SDK_RET_OK;
}

sdk_ret_t
catalog::populate_mac_profile(mac_profile_t *mac_profile,
                              std::string   profile_str,
                              ptree         &prop_tree)
{
    std::string val =
        prop_tree.get<std::string>(profile_str + ".glbl_mode", "");
    mac_profile->glbl_mode = strtoul(val.c_str(), NULL, 16);

    val = prop_tree.get<std::string>(profile_str + ".channel", "");
    mac_profile->channel = strtoul(val.c_str(), NULL, 16);

    val = prop_tree.get<std::string>(profile_str + ".tdm_slot", "");
    mac_profile->tdm_slot = strtoul(val.c_str(), NULL, 16);

    populate_mac_ch_profile(&(mac_profile->ch_profile[0]),
                            profile_str + ".0",
                            prop_tree);
    populate_mac_ch_profile(&(mac_profile->ch_profile[1]),
                            profile_str + ".1",
                            prop_tree);
    populate_mac_ch_profile(&(mac_profile->ch_profile[2]),
                            profile_str + ".2",
                            prop_tree);
    populate_mac_ch_profile(&(mac_profile->ch_profile[3]),
                            profile_str + ".3",
                            prop_tree);

    return SDK_RET_OK;
}

sdk_ret_t
catalog::populate_mgmt_mac_profiles(ptree &prop_tree)
{
    int mode = 0;

    mode = static_cast<int> (MAC_MODE_4x1g);
    populate_mac_profile(&(catalog_db_.mgmt_mac_profiles[mode]),
                         "bx.mode_4x1g",
                         prop_tree);
    return SDK_RET_OK;
}

sdk_ret_t
catalog::populate_mac_profiles(ptree &prop_tree)
{
    int mode = 0;

    mode = static_cast<int> (MAC_MODE_1x100g);
    populate_mac_profile(&(catalog_db_.mac_profiles[mode]),
                         "mx.mode_1x100g",
                         prop_tree);

    mode = static_cast<int> (MAC_MODE_2x25g_1x50g);
    populate_mac_profile(&(catalog_db_.mac_profiles[mode]),
                         "mx.mode_2x25g_1x50g",
                         prop_tree);

    mode = static_cast<int> (MAC_MODE_1x40g);
    populate_mac_profile(&(catalog_db_.mac_profiles[mode]),
                         "mx.mode_1x40g",
                         prop_tree);

    mode = static_cast<int> (MAC_MODE_1x50g);
    populate_mac_profile(&(catalog_db_.mac_profiles[mode]),
                         "mx.mode_1x50g",
                         prop_tree);

    mode = static_cast<int> (MAC_MODE_2x40g);
    populate_mac_profile(&(catalog_db_.mac_profiles[mode]),
                         "mx.mode_2x40g",
                         prop_tree);

    mode = static_cast<int> (MAC_MODE_2x50g);
    populate_mac_profile(&(catalog_db_.mac_profiles[mode]),
                         "mx.mode_2x50g",
                         prop_tree);

    mode = static_cast<int> (MAC_MODE_1x50g_2x25g);
    populate_mac_profile(&(catalog_db_.mac_profiles[mode]),
                         "mx.mode_1x50g_2x25g",
                         prop_tree);

    mode = static_cast<int> (MAC_MODE_4x25g);
    populate_mac_profile(&(catalog_db_.mac_profiles[mode]),
                         "mx.mode_4x25g",
                         prop_tree);

    mode = static_cast<int> (MAC_MODE_4x10g);
    populate_mac_profile(&(catalog_db_.mac_profiles[mode]),
                         "mx.mode_4x10g",
                         prop_tree);

    mode = static_cast<int> (MAC_MODE_4x1g);
    populate_mac_profile(&(catalog_db_.mac_profiles[mode]),
                         "mx.mode_4x1g",
                         prop_tree);
    return SDK_RET_OK;
}

uint32_t
catalog::serdes_index_get(uint32_t sbus_addr)
{
    if (sbus_addr < SERDES_SBUS_START) {
        return sbus_addr;
    }

    return (sbus_addr - SERDES_SBUS_START);
}

uint8_t
catalog::cable_type_get(std::string cable_type_str)
{
    if (cable_type_str == "CU") {
        return sdk::types::CABLE_TYPE_CU;
    }

    return sdk::types::CABLE_TYPE_FIBER;
}

sdk_ret_t
catalog::parse_serdes(ptree &prop_tree)
{
    for (ptree::value_type &serdes : prop_tree.get_child("")) {

        uint32_t mac_id      = serdes.second.get<uint32_t>("mac_id", 0);
        uint32_t mac_ch      = serdes.second.get<uint32_t>("mac_ch", 0);
        uint32_t sbus_addr   = serdes.second.get<uint32_t>("sbus_addr", 0);
        uint32_t serdes_lane = 0;
        uint8_t  cable_type  = 0;
        uint32_t port_speed  = 0;

        std::string slip = "";

        std::string cable_type_str =
                    serdes.second.get<std::string>("cable_type", "CU");
        std::string speed_str = serdes.second.get<std::string>("speed", "10G");

        serdes_lane = serdes_index_get(sbus_addr);
        cable_type  = cable_type_get(cable_type_str);
        port_speed  = static_cast<uint32_t>(
                                catalog_speed_to_port_speed(speed_str));

        serdes_info_t *serdes_info =
                    &catalog_db_.serdes[serdes_lane][port_speed][cable_type];

        // sbus speed divider
        serdes_info->sbus_divider =
                            serdes.second.get<uint32_t>("sbus_divider", 0);

        // Tx/Rx slip values
        slip = serdes.second.get<std::string>("tx_slip_value", "");

        serdes_info->tx_slip_value = strtoul(slip.c_str(), NULL, 16);

        slip = serdes.second.get<std::string>("rx_slip_value", "");

        serdes_info->rx_slip_value = strtoul(slip.c_str(), NULL, 16);

        serdes_info->width = serdes.second.get<uint32_t>("width", 0);

        // Tx/Rx polatiry
        serdes_info->tx_pol = serdes.second.get<uint8_t>("tx_pol", 0);
        serdes_info->rx_pol = serdes.second.get<uint8_t>("rx_pol", 0);

        // Rx termination
        serdes_info->rx_term = serdes.second.get<uint8_t>("rx_termination", 0);

        // DFE Tx params
        serdes_info->amp = serdes.second.get<uint32_t>("main", 0);
        serdes_info->pre = serdes.second.get<uint32_t>("pre1", 0);
        serdes_info->post = serdes.second.get<uint32_t>("post", 0);

        uint32_t asic = 0;
        uint32_t asic_port = 0;

        catalog_asic_port_t *asic_port_p = NULL;

        for (asic_port = 0; asic_port < MAX_ASIC_PORTS; ++asic_port) {
            asic_port_p = &catalog_db_.asics[asic].ports[asic_port];
            if (asic_port_p->mac_id == mac_id &&
                    asic_port_p->mac_ch == mac_ch) {
                asic_port_p->sbus_addr = sbus_addr;
                break;
            }
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
catalog::parse_serdes_file(std::string& serdes_file)
{
    sdk_ret_t ret;
    ptree prop_tree;

    ret = get_ptree_(serdes_file, prop_tree);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    return parse_serdes(prop_tree);
}

sdk_ret_t
catalog::populate_serdes(char *dir_name, ptree &prop_tree)
{
    std::string jtag_id = prop_tree.get<std::string>("serdes.jtag_id", "");

    catalog_db_.serdes_jtag_id = strtoul(jtag_id.c_str(), NULL, 16);

    catalog_db_.num_sbus_rings =
                        prop_tree.get<std::uint8_t>("serdes.num_sbus_rings", 0);
    catalog_db_.aacs_info.server_en   =
                        prop_tree.get<std::uint8_t>("serdes.aacs_server", 0);
    catalog_db_.aacs_info.connect     =
                        prop_tree.get<std::uint8_t>("serdes.aacs_connect", 0);
    catalog_db_.aacs_info.server_ip   =
                        prop_tree.get<std::string>("serdes.ip", "");
    catalog_db_.aacs_info.server_port =
                        prop_tree.get<std::uint32_t>("serdes.port", 0);

    catalog_db_.serdes_fw_file =
                        prop_tree.get<std::string>("serdes.fw", "");

    std::string serdes_build_id =
                        prop_tree.get<std::string>("serdes.build_id", "");
    catalog_db_.serdes_build_id = strtoul(serdes_build_id.c_str(), NULL, 16);

    std::string serdes_rev_id =
                        prop_tree.get<std::string>("serdes.rev_id", "");
    catalog_db_.serdes_rev_id = strtoul(serdes_rev_id.c_str(), NULL, 16);

    std::string serdes_file =
                        prop_tree.get<std::string>("serdes.serdes_file", "");

    serdes_file = std::string(dir_name) + "/" + serdes_file;

    return parse_serdes_file(serdes_file);
}

sdk_ret_t
catalog::populate_catalog(std::string &catalog_file, ptree &prop_tree)
{
    catalog_db_.card_index = prop_tree.get<uint32_t>("card_index", 0);
    catalog_db_.max_mpu_per_stage = prop_tree.get<uint32_t>("max_mpu_per_stage", 0);
    catalog_db_.mpu_trace_size = prop_tree.get<uint32_t>("mpu_trace_size", 0);

    std::string str = prop_tree.get<std::string>("cores_mask", "");
    catalog_db_.cores_mask = std::stoul (str, nullptr, 16);

    // validate cores_mask against the number of available processor cores
    if (catalog_db_.cores_mask == 0 ||
        ffs_msb(catalog_db_.cores_mask) > get_nprocs()) {
        SDK_TRACE_ERR("Invalid cores_mask 0x%lx."
                      " Available cores: %d\n",
                      catalog_db_.cores_mask,
                      get_nprocs());
        return SDK_RET_ERR;
    }

    catalog_db_.num_asics = prop_tree.get<uint32_t>("num_asics", 0);
    catalog_db_.num_uplink_ports =
                            prop_tree.get<uint32_t>("num_uplink_ports", 0);

    catalog_db_.num_fp_ports = prop_tree.get<uint32_t>("num_fp_ports", 0);

    populate_asics(prop_tree);

    populate_fp_ports(prop_tree);

    populate_uplink_ports(prop_tree);

    populate_mgmt_mac_profiles(prop_tree);

    populate_mac_profiles(prop_tree);

    populate_serdes(dirname((char*)catalog_file.c_str()), prop_tree);

    populate_qos_profile(prop_tree);

    return SDK_RET_OK;
}

sdk_ret_t
catalog::get_ptree_ (std::string& catalog_file, ptree& prop_tree)
{
    if (access(catalog_file.c_str(), R_OK) < 0) {
        SDK_TRACE_ERR("config file %s has no read permissions",
                      catalog_file.c_str());
        return SDK_RET_ERR;
    }

    boost::property_tree::read_json(catalog_file, prop_tree);
    return SDK_RET_OK;
}

//------------------------------------------------------------------------------
// initialize an instance of catalog class
//------------------------------------------------------------------------------
sdk_ret_t
catalog::init(std::string &catalog_file)
{
    sdk_ret_t ret;
    ptree prop_tree;

    ret = get_ptree_(catalog_file, prop_tree);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    return populate_catalog(catalog_file, prop_tree);
}

//------------------------------------------------------------------------------
// factory method for this class
//------------------------------------------------------------------------------
catalog *
catalog::factory(std::string catalog_file) {
    sdk_ret_t  ret;
    void       *mem;
    catalog    *new_catalog = NULL;

    // make sure file exists
    if (access(catalog_file.c_str(), R_OK) < 0) {
        SDK_TRACE_ERR("config file %s has no read permissions",
                      catalog_file.c_str());
        return NULL;
    }

    mem = SDK_CALLOC(sdk::SDK_MEM_ALLOC_CATALOG, sizeof(catalog));
    if (!mem) {
        SDK_TRACE_ERR("Failed to allocate memory for catalog");
        return NULL;
    }

    new_catalog = new (mem) catalog();
    ret = new_catalog->init(catalog_file);
    if (ret != SDK_RET_OK) {
        SDK_TRACE_ERR("Failed to init catalog");
        new_catalog->~catalog();
        SDK_FREE(sdk::SDK_MEM_ALLOC_CATALOG, new_catalog);
        return NULL;
    }

    return new_catalog;
}

sdk_ret_t
catalog::get_child_str (std::string catalog_file, std::string path, std::string& child_str) 
{
    sdk_ret_t ret;
    ptree prop_tree;

    child_str = "";

    ret = get_ptree_(catalog_file, prop_tree);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    auto child = prop_tree.get_child_optional(path);

    if (child) {
        std::stringstream ss;
        write_json(ss, *child);
        child_str = ss.str();
    }

    return SDK_RET_OK;
}

//------------------------------------------------------------------------------
// destructor
//------------------------------------------------------------------------------
catalog::~catalog()
{
}

void
catalog::destroy(catalog *clog)
{
    if (!clog) {
        return;
    }
    clog->~catalog();
    SDK_FREE(sdk::SDK_MEM_ALLOC_CATALOG, clog);
}

catalog_uplink_port_t *
catalog::uplink_port(uint32_t port)
{
    return &catalog_db_.uplink_ports[port-1];
}

port_speed_t
catalog::port_speed(uint32_t port)
{
    // TODO: do out of bound error check here !!
    return catalog_db_.uplink_ports[port-1].speed;
}

port_type_t
catalog::port_type_fp (uint32_t port)
{
    return catalog_db_.fp_ports[port-1].type;
}

uint32_t
catalog::num_lanes_fp (uint32_t port)
{
    return catalog_db_.fp_ports[port-1].num_lanes;
}

uint32_t
catalog::breakout_modes(uint32_t port)
{
    return catalog_db_.fp_ports[port-1].breakout_modes;
}

uint32_t
catalog::num_lanes(uint32_t port)
{
    return catalog_db_.uplink_ports[port-1].num_lanes;
}

port_type_t
catalog::port_type(uint32_t port)
{
    return catalog_db_.uplink_ports[port-1].type;
}

port_fec_type_t
catalog::port_fec_type(uint32_t port)
{
    return catalog_db_.uplink_ports[port-1].fec_type;
}

bool
catalog::enabled(uint32_t port)
{
    return catalog_db_.uplink_ports[port-1].enabled;
}

uint32_t
catalog::mac_id(uint32_t port, uint32_t lane)
{
    catalog_uplink_port_t *catalog_uplink_port_p = uplink_port(port);
    uint32_t asic = catalog_uplink_port_p->asic;
    uint32_t asic_port = catalog_uplink_port_p->asic_port;

    return catalog_db_.asics[asic].ports[asic_port + lane].mac_id;
}

uint32_t
catalog::mac_ch(uint32_t port, uint32_t lane)
{
    catalog_uplink_port_t *catalog_uplink_port_p = uplink_port(port);
    uint32_t asic = catalog_uplink_port_p->asic;
    uint32_t asic_port = catalog_uplink_port_p->asic_port;

    return catalog_db_.asics[asic].ports[asic_port + lane].mac_ch;
}

uint32_t
catalog::sbus_addr(uint32_t port, uint32_t lane)
{
    catalog_uplink_port_t *catalog_uplink_port_p = uplink_port(port);
    uint32_t asic = catalog_uplink_port_p->asic;
    uint32_t asic_port = catalog_uplink_port_p->asic_port;

    return catalog_db_.asics[asic].ports[asic_port + lane].sbus_addr;
}

uint32_t
catalog::sbus_addr(uint32_t asic_num, uint32_t asic_port, uint32_t lane)
{
    uint32_t mac_id      = asic_num;
    uint32_t mac_ch      = asic_port;
    uint32_t l_asic_num  = 0;
    uint32_t l_asic_port = 0;

    // TODO work around until port pd structure is
    // updated to hold asic number and asic port
    for (l_asic_num = 0; l_asic_num < MAX_ASICS; ++l_asic_num) {
        for (l_asic_port = 0; l_asic_port < MAX_ASIC_PORTS; ++l_asic_port) {
            catalog_asic_port_t *catalog_asic_port =
                        &catalog_db_.asics[l_asic_num].ports[l_asic_port];
            if (catalog_asic_port->mac_id == mac_id &&
                catalog_asic_port->mac_ch == mac_ch) {
                return catalog_db_.asics[l_asic_num].
                                    ports[l_asic_port + lane].sbus_addr;
            }
        }
    }

    return 0x0;
}

serdes_info_t*
catalog::serdes_info_get(uint32_t sbus_addr,
                         uint32_t port_speed,
                         uint32_t cable_type)
{
    uint32_t addr = serdes_index_get(sbus_addr);

    return &catalog_db_.serdes[addr][port_speed][cable_type];
}

uint32_t
catalog::glbl_mode_mgmt(mac_mode_t mac_mode)
{
    int mode = static_cast<int>(mac_mode);
    return catalog_db_.mgmt_mac_profiles[mode].glbl_mode;
}

uint32_t
catalog::ch_mode_mgmt(mac_mode_t mac_mode, uint32_t ch)
{
    int mode = static_cast<int>(mac_mode);
    return catalog_db_.mgmt_mac_profiles[mode].ch_profile[ch].ch_mode;
}

uint32_t
catalog::glbl_mode(mac_mode_t mac_mode)
{
    int mode = static_cast<int>(mac_mode);
    return catalog_db_.mac_profiles[mode].glbl_mode;
}

uint32_t
catalog::ch_mode(mac_mode_t mac_mode, uint32_t ch)
{
    int mode = static_cast<int>(mac_mode);
    return catalog_db_.mac_profiles[mode].ch_profile[ch].ch_mode;
}

}    // namespace lib
}    // namespace sdk
