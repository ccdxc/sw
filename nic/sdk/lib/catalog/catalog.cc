// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <sys/sysinfo.h>
#include <libgen.h>
#include "lib/catalog/catalog.hpp"
#include "platform/fru/fru.hpp"
#include "platform/pal/include/pal_types.h"
#include "platform/pal/include/pal.h"
#include "include/sdk/mem.hpp"
#include "lib/utils/utils.hpp"
#include "include/sdk/if.hpp"

namespace sdk {
namespace lib {

int
catalog::logical_port_to_phy_port(uint32_t logical_port)
{
    switch (logical_port) {
    case 1 ... 4:
        return 1;

    case 5 ... 8:
        return 2;

    default:
        return -1;
    }
}

uint32_t
catalog::logical_port_to_tm_port(uint32_t logical_port)
{
    uint32_t asic = 0;
    uint32_t asic_port = 0;

    logical_port_to_asic_port(logical_port, &asic, &asic_port);
    return catalog_db_.asics[asic].ports[asic_port].tm_port;
}

sdk_ret_t
catalog::populate_asic_port(ptree::value_type &asic_port,
                            catalog_asic_port_t *asic_port_p)
{
    asic_port_p->mac_id    = asic_port.second.get<uint32_t>("mac_id", 0);
    asic_port_p->mac_ch    = asic_port.second.get<uint32_t>("mac_ch", 0);
    asic_port_p->sbus_addr = asic_port.second.get<uint32_t>("sbus_addr", 0);
    asic_port_p->tm_port   = asic_port.second.get<uint32_t>("tm_port", 0);

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
    if (asic.second.get<std::string>("name", "") ==  "capri") {
        asic_p->type = asic_type_t::SDK_ASIC_TYPE_CAPRI;
    } else {
        asic_p->type = asic_type_t::SDK_ASIC_TYPE_NONE;
    }

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

bool
catalog::catalog_str_to_bool(std::string val)
{
    if (val == "true") {
        return true;
    }
    return false;
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
catalog::populate_logical_port(ptree::value_type &logical_port,
                              catalog_logical_port_t *logical_port_p)
{
    logical_port_p->asic = logical_port.second.get<uint32_t>("asic", 0);
    logical_port_p->asic_port = logical_port.second.get<uint32_t>("asic_port", 0);

    return SDK_RET_OK;
}

sdk_ret_t
catalog::populate_logical_ports(ptree &prop_tree)
{
    for (ptree::value_type &logical_port : prop_tree.get_child("logical_ports")) {
        catalog_logical_port_t *logical_port_p =
                            &catalog_db_.logical_ports[
                            logical_port.second.get<uint32_t>("port_num", 0) - 1];

        populate_logical_port(logical_port, logical_port_p);
    }

    return SDK_RET_OK;
}

sdk_ret_t
catalog::populate_logical_oob_port(
                            ptree::value_type &logical_oob_port,
                            catalog_logical_oob_port_t *logical_oob_port_p)
{
    std::string val;

    val = logical_oob_port.second.get<std::string>("phy_id", "0");
    logical_oob_port_p->phy_id = strtoul(val.c_str(), NULL, 16);
    val = logical_oob_port.second.get<std::string>("hw_port", "0");
    logical_oob_port_p->hw_port = strtoul(val.c_str(), NULL, 16);
    val = logical_oob_port.second.get<std::string>("speed", "1G");
    logical_oob_port_p->speed = catalog_speed_to_port_speed(val);
    val = logical_oob_port.second.get<std::string>("auto_neg_enable", "false");
    logical_oob_port_p->auto_neg_enable = catalog_str_to_bool(val);
    return SDK_RET_OK;
}

sdk_ret_t
catalog::populate_logical_oob_ports(ptree &prop_tree)
{
    boost::optional<ptree&> child =
                    prop_tree.get_child_optional("logical_oob_ports");

    if (!child) {
        return SDK_RET_OK;
    }
    for (ptree::value_type &logical_oob_port :
                            prop_tree.get_child("logical_oob_ports")) {
        catalog_logical_oob_port_t *logical_oob_port_p =
                    &catalog_db_.logical_oob_ports[
                    logical_oob_port.second.get<uint32_t>("port_num", 0) - 1];
        populate_logical_oob_port(logical_oob_port, logical_oob_port_p);
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

sdk_ret_t
catalog::populate_clock_info (ptree &prop_tree)
{
    uint8_t  num_freq = 0;

    catalog_db_.num_clock_info = prop_tree.get<uint32_t>("num_clock_info", 0); 
    for (ptree::value_type &clock_info : prop_tree.get_child("clock_info")) {
         if (num_freq < MAX_CLOCK_FREQ) {
             catalog_db_.clock_info[num_freq].clock_freq = clock_info.second.get<uint16_t>("freq", 0);
             catalog_db_.clock_info[num_freq].clock_multiplier = clock_info.second.get<uint32_t>("multiplier", 0);
             num_freq += 1;
          }
    }

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
        std::string tx_pol, rx_pol;

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
        tx_pol = serdes.second.get<std::string>("tx_pol", "");
        rx_pol = serdes.second.get<std::string>("rx_pol", "");

        serdes_info->tx_pol = strtoul(tx_pol.c_str(), NULL, 16);
        serdes_info->rx_pol = strtoul(rx_pol.c_str(), NULL, 16);

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
catalog::populate_pcie(ptree &pt)
{
    int nportspecs = 0;

    boost::optional<ptree&> pcieptopt = pt.get_child_optional("pcie");
    if (pcieptopt) {
        ptree &pciept = pt.get_child("pcie");
        std::string s;

        const std::string penid("0x1dd8"); // PCI_VENDOR_ID_PENSANDO
        s = pciept.get<std::string>("vendorid", penid);
        catalog_db_.pcie_vendorid = strtoul(s.c_str(), NULL, 16);

        s = pciept.get<std::string>("subvendorid", penid);
        catalog_db_.pcie_subvendorid = strtoul(s.c_str(), NULL, 16);

        s = pciept.get<std::string>("subdeviceid", "");
        catalog_db_.pcie_subdeviceid = strtoul(s.c_str(), NULL, 16);

        s = pciept.get<std::string>("long_lived", "");
        catalog_db_.pcie_long_lived = strtoul(s.c_str(), NULL, 16);

        for (ptree::value_type &v : pciept.get_child("portspecs")) {
            if (nportspecs < MAX_PCIE_PORTSPECS) {
                catalog_pcie_portspec_t *hp;
                hp = &catalog_db_.pcie_portspecs[nportspecs];
                hp->host  = v.second.get<uint8_t>("host");
                hp->port  = v.second.get<uint8_t>("port");
                hp->gen   = v.second.get<uint8_t>("gen");
                hp->width = v.second.get<uint8_t>("width");
                nportspecs++;
            }
        }
    }
    catalog_db_.pcie_nportspecs = nportspecs;
    return SDK_RET_OK;
}

sdk_ret_t
catalog::populate_voltages(ptree &prop_tree)
{

    catalog_db_.voltages.startup_arm  = prop_tree.get<std::uint32_t>("voltages.startup_arm", 0);
    catalog_db_.voltages.startup_vdd  = prop_tree.get<std::uint32_t>("voltages.startup_vdd", 0);

    return SDK_RET_OK;
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

    catalog_db_.sbm_clk_div =
                        prop_tree.get<std::uint8_t>("serdes.sbm_clk_div", 0);

    return parse_serdes_file(serdes_file);
}

card_id_t
catalog::catalog_board_type_to_sdk_card_id(std::string board_type)
{
    if (board_type.compare("Naples100") == 0) {
        return CARD_ID_NAPLES100;
    } else if (board_type.compare("Naples25") == 0) {
        return CARD_ID_NAPLES25;
    } else if (board_type.compare("Naples25 SWM") == 0) {
        return CARD_ID_NAPLES25;
    } else {
        return CARD_ID_NAPLES100;
    }
}

sdk_ret_t
catalog::populate_catalog(std::string &catalog_file, ptree &prop_tree)
{
    catalog_db_.card_id = catalog_board_type_to_sdk_card_id(
                                            prop_tree.get<std::string>("board_type", ""));
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

    std::string val = prop_tree.get<std::string>("cpld_id", "");
    catalog_db_.cpld_id = strtoul(val.c_str(), NULL, 16);

    catalog_db_.form_factor = prop_tree.get<std::string>("form_factor", "");
    catalog_db_.emmc_size = prop_tree.get<uint32_t>("emmc_size", 0);
    catalog_db_.memory_size = prop_tree.get<uint32_t>("memory_size", 0);

    catalog_db_.num_asics = prop_tree.get<uint32_t>("num_asics", 0);
    catalog_db_.num_logical_ports =
                            prop_tree.get<uint32_t>("num_logical_ports", 0);
    catalog_db_.num_logical_oob_ports =
                        prop_tree.get<uint32_t>("num_logical_oob_ports", 0);
    catalog_db_.num_fp_ports = prop_tree.get<uint32_t>("num_fp_ports", 0);
    catalog_db_.halfclock_hbmtemp = prop_tree.get<uint32_t>("halfclock_hbmtemp", 105);

    populate_voltages(prop_tree);

    populate_asics(prop_tree);

    populate_fp_ports(prop_tree);

    populate_logical_ports(prop_tree);
    populate_logical_oob_ports(prop_tree);

    populate_mgmt_mac_profiles(prop_tree);

    populate_mac_profiles(prop_tree);

    populate_pcie(prop_tree);

    populate_serdes(dirname((char*)catalog_file.c_str()), prop_tree);

    populate_clock_info(prop_tree);

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

    catalog_db_.catalog_file = catalog_file;
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
catalog::factory(std::string catalog_file_path, std::string catalog_file_name,
                 platform_type_t platform) {
    sdk_ret_t  ret;
    void       *mem;
    catalog    *new_catalog = NULL;
    int pal_platform;

    //If user didn't provide platfrm then we try to determine platform through pal
    if (platform == platform_type_t::PLATFORM_TYPE_NONE) {
        pal_platform = pal_get_env();

        switch (pal_platform) {
            case PAL_ENV_ASIC:
                platform = platform_type_t::PLATFORM_TYPE_HW;
                break;
            case PAL_ENV_HAPS:
                platform = platform_type_t::PLATFORM_TYPE_HAPS;
                break;
            case PAL_ENV_ZEBU:
                platform = platform_type_t::PLATFORM_TYPE_ZEBU;
                break;
            default:
                platform = platform_type_t::PLATFORM_TYPE_SIM;
                break;
        }
    }

    if (catalog_file_path.empty()) {
        if (char* env = std::getenv("CATALOG_PATH"))
            catalog_file_path = env;
        else
            catalog_file_path = DEFAULT_CATALOG_PATH;
    }

    if (catalog_file_name.empty()) {

        if (platform == platform_type_t::PLATFORM_TYPE_HW) {
            std::string part_num(32, '\0');
            std::string part_id;

            if (sdk::platform::readFruKey(PARTNUM_KEY, part_num) == 0) {
                // Pensando has part numbers starting with prefix
                // 68-. The check is to find out which catalog file
                // to use. Catalog file is named either by using
                // the first 7 characters for pensando part number
                // and full part number otherwise.
                if (part_num.substr(0, 3) == "68-") {
                    // first 7 characters are the part identifiers
                    part_id = part_num.substr(0, 7);
                } else {
                    part_id = part_num;
                }
                catalog_file_name = "/catalog_hw_" + part_id + ".json";
            } else {
                SDK_TRACE_ERR("part-id from FRU is empty. Please program the correct FRU");
                SDK_TRACE_ERR("Using default catalog.");
                catalog_file_name = "/catalog_hw_68-0003.json";
            }
        }
        else if (platform == platform_type_t::PLATFORM_TYPE_SIM ||
                platform == platform_type_t::PLATFORM_TYPE_MOCK) {
            catalog_file_name = "/catalog.json";
        }
    }

    std::string catalog_file = catalog_file_path + catalog_file_name;

    // make sure file exists
    if (access(catalog_file.c_str(), R_OK) < 0) {
        SDK_TRACE_ERR("config file %s has no read permissions",
                      catalog_file.c_str());
        catalog_file = catalog_file_path + "/catalog_hw_68-0003.json";
        if (access(catalog_file.c_str(), R_OK) < 0) {
            SDK_TRACE_ERR("default config file %s has no read permissions",
                         catalog_file.c_str());
            return NULL;
        }
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
catalog::get_child_str (std::string path, std::string& child_str)
{
    sdk_ret_t ret;
    ptree prop_tree;

    child_str = "";

    ret = get_ptree_(catalog_db_.catalog_file, prop_tree);
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

catalog_logical_port_t *
catalog::logical_port_internal(uint32_t logical_port)
{
    return &catalog_db_.logical_ports[logical_port - 1];
}

catalog_logical_oob_port_t *
catalog::logical_oob_port_internal(uint32_t logical_oob_port)
{
    return &catalog_db_.logical_oob_ports[logical_oob_port - 1];
}

port_type_t
catalog::port_type_fp (uint32_t fp_port)
{
    return catalog_db_.fp_ports[fp_port-1].type;
}

uint32_t
catalog::num_lanes_fp (uint32_t fp_port)
{
    return catalog_db_.fp_ports[fp_port-1].num_lanes;
}

uint32_t
catalog::breakout_modes(uint32_t fp_port)
{
    return catalog_db_.fp_ports[fp_port-1].breakout_modes;
}

uint32_t
catalog::asic_port_to_mac_id(uint32_t asic, uint32_t asic_port)
{
    return catalog_db_.asics[asic].ports[asic_port].mac_id;
}

uint32_t
catalog::asic_port_to_mac_ch(uint32_t asic, uint32_t asic_port)
{
    return catalog_db_.asics[asic].ports[asic_port].mac_ch;
}

void
catalog::logical_port_to_asic_port(uint32_t logicalport,
                                   uint32_t *asic, uint32_t *asic_port)
{
    catalog_logical_port_t *catalog_logical_port_p =
                                logical_port_internal(logicalport);
    *asic = catalog_logical_port_p->asic;
    *asic_port = catalog_logical_port_p->asic_port;
}

uint32_t
catalog::mac_id(uint32_t logical_port, uint32_t lane)
{
    uint32_t asic = 0;
    uint32_t asic_port = 0;

    logical_port_to_asic_port(logical_port, &asic, &asic_port);
    return catalog_db_.asics[asic].ports[asic_port + lane].mac_id;
}

uint32_t
catalog::mac_ch(uint32_t logical_port, uint32_t lane)
{
    uint32_t asic = 0;
    uint32_t asic_port = 0;

    logical_port_to_asic_port(logical_port, &asic, &asic_port);
    return catalog_db_.asics[asic].ports[asic_port + lane].mac_ch;
}

uint32_t
catalog::sbus_addr(uint32_t logical_port, uint32_t lane)
{
    uint32_t asic = 0;
    uint32_t asic_port = 0;

    logical_port_to_asic_port(logical_port, &asic, &asic_port);
    return catalog_db_.asics[asic].ports[asic_port + lane].sbus_addr;
}

uint32_t
catalog::sbus_addr(uint32_t asic_num, uint32_t asic_port, uint32_t lane)
{
    return catalog_db_.asics[asic_num].
                       ports[asic_port + lane].sbus_addr;
}

uint32_t
catalog::oob_phy_id(uint32_t logical_oob_port) {
    catalog_logical_oob_port_t *catalog_logical_oob_port =
                                logical_oob_port_internal(logical_oob_port);
    return catalog_logical_oob_port->phy_id;
}

uint32_t
catalog::oob_hw_port(uint32_t logical_oob_port) {
    catalog_logical_oob_port_t *catalog_logical_oob_port =
                                logical_oob_port_internal(logical_oob_port);
    return catalog_logical_oob_port->hw_port;
}

port_speed_t
catalog::oob_speed(uint32_t logical_oob_port) {
    catalog_logical_oob_port_t *catalog_logical_oob_port =
                                logical_oob_port_internal(logical_oob_port);
    return catalog_logical_oob_port->speed;
}

bool
catalog::oob_auto_neg_enable(uint32_t logical_oob_port) {
    catalog_logical_oob_port_t *catalog_logical_oob_port =
                                logical_oob_port_internal(logical_oob_port);
    return catalog_logical_oob_port->auto_neg_enable;
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

uint32_t
catalog::ifindex_to_logical_port(uint32_t ifindex)
{
    uint32_t parent = ETH_IFINDEX_TO_PARENT_PORT(ifindex);
    uint32_t child = ETH_IFINDEX_TO_CHILD_PORT(ifindex);
    return ((parent - 1) * MAX_PORT_LANES) + child;
}

uint32_t
catalog::ifindex_to_phy_port(uint32_t ifindex)
{
    return ETH_IFINDEX_TO_PARENT_PORT(ifindex);
}

uint32_t
catalog::logical_port_to_ifindex(uint32_t logical_port)
{
    uint32_t parent = ((logical_port - 1)/MAX_PORT_LANES) + 1;
    uint32_t child = ((logical_port - 1) % MAX_PORT_LANES) + 1;
    return ETH_IFINDEX(slot(), parent, child);
}

std::string
catalog::logical_port_to_str(uint32_t logical_port, port_type_t port_type)
{
    uint32_t parent = 0;

    if (port_type == port_type_t::PORT_TYPE_ETH) {
        parent = ((logical_port - 1)/MAX_PORT_LANES) + 1;
        return "eth" + std::to_string(slot()) + "/" + std::to_string(parent);
    }
    parent = logical_port - MGMT_BASE_PORT;
    return "mgmt" + std::to_string(parent);
}

uint32_t
catalog::ifindex_to_tm_port(uint32_t ifindex)
{
    uint32_t logical_port;

    logical_port = ifindex_to_logical_port(ifindex);
    return catalog::logical_port_to_tm_port(logical_port);
}

uint32_t
catalog::tm_port_to_ifindex(uint32_t tm_port) {
    uint32_t ifindex = 0xFFFFFFFF;

    // logical ports are 1 based
    for (uint32_t logical_port = 1; logical_port < num_logical_ports();
                                                    logical_port++) {
        if (logical_port_to_tm_port(logical_port) == tm_port) {
            ifindex = logical_port_to_ifindex(logical_port);
            break;
        }
    }
    return ifindex;
}

uint32_t
catalog::clock_get_multiplier(uint16_t freq) {
    for (uint8_t idx=0; idx<MAX_CLOCK_FREQ; idx++) {
         if (freq == catalog_db_.clock_info[idx].clock_freq)
             return catalog_db_.clock_info[idx].clock_multiplier;
    }
    return 0;
}

}    // namespace lib
}    // namespace sdk
