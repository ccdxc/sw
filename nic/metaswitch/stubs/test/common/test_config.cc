//----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//----------------------------------------------------------------------------
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "nic/metaswitch/stubs/test/common/test_config.hpp"
#include <arpa/inet.h>
#include <iostream>

int g_node_id = 0;

using boost::property_tree::ptree;
namespace pds_ms_test {
int
parse_json_config (test_config_t *conf, uint8_t node) {
    ptree       pt;
    uint32_t    test_config = 0;
    std::string file, cfg_path, value;

    if (!std::getenv("CONFIG_PATH")) {
        fprintf(stderr, "CONFIG_PATH env var is not set!\n");
        return -1;
    }
    // form the full path to the config directory
    cfg_path = std::string(std::getenv("CONFIG_PATH"));
    if (cfg_path.empty()) {
        cfg_path = std::string("./");
    } else {
        cfg_path += "/";
    }

    // make sure the cfg file exists
    file = cfg_path + "/" + std::string("evpn.json");
    if (access(file.c_str(), R_OK) < 0) {
        fprintf(stderr, "Config file %s doesn't exist or not accessible\n",
                file.c_str());
        return -1;
    }
    std::ifstream json_cfg (file.c_str());
    if (!json_cfg)
    {
        fprintf(stderr, "Config file %s doesn't exist or not accessible\n",
                file.c_str());
        return -1;
    }

    // read config
    read_json (json_cfg, pt);
    try {
        // if there is test-config string, read it
        test_config = pt.get <uint32_t>("test-config", 0);
    }
    catch (std::exception const&  ex) {
        // Otherwise, test config is not required
        fprintf(stderr, "Config file %s doesn't have test-config enabled!\n",
                file.c_str());
    }

    if (!test_config)
    {
        return -1;
    }
    g_node_id = test_config;

    try {
    value                   = pt.get <std::string>("local.ip");
    conf->local_ip_addr     = inet_addr (value.c_str());
    value                   = pt.get <std::string>("local.ip2");
    conf->local_ip_addr_2     = inet_addr (value.c_str());
    value                   = pt.get <std::string>("local.lo-ip");
    conf->local_lo_ip_addr  = inet_addr (value.c_str());
    value                   = pt.get <std::string>("local.gwip");
    conf->local_gwip_addr   = inet_addr (value.c_str());
    value                   = pt.get <std::string>("local.ac-ip");
    conf->local_mai_ip      = inet_addr (value.c_str());
    value                   = pt.get <std::string>("remote.ip");
    conf->remote_ip_addr    = inet_addr (value.c_str());
    value                   = pt.get <std::string>("remote.ip2");
    conf->remote_ip_addr_2    = inet_addr (value.c_str());
    value                   = pt.get <std::string>("remote.lo-ip");
    conf->remote_lo_ip_addr = inet_addr (value.c_str());
    value                   = pt.get <std::string>("remote.lo-ip2");
    conf->remote_lo_ip_addr_2 = inet_addr (value.c_str());
    value                   = pt.get <std::string>("eth-if-index");
    conf->eth_if_index      = strtol (value.c_str(),NULL, 0);
    value                   = pt.get <std::string>("eth-if-index-2");
    conf->eth_if_index_2    = strtol (value.c_str(),NULL, 0);
    value                   = pt.get <std::string>("lif-if-index");
    conf->lif_if_index      = strtol (value.c_str(),NULL, 0);
    value                   = pt.get <std::string>("local.as");
    conf->local_asn         = strtol (value.c_str(),NULL, 0);
    value                   = pt.get <std::string>("remote.as");
    conf->remote_asn        = strtol (value.c_str(),NULL, 0);
    value                   = pt.get <std::string>("vni");
    conf->vni               = strtol (value.c_str(),NULL, 0);
    value                   = pt.get <std::string>("route.dest_ip");
    conf->route_dest_ip     = inet_addr (value.c_str());
    value                   = pt.get <std::string>("route.nh_ip");
    conf->route_nh_ip       = inet_addr (value.c_str());
    value                   = pt.get <std::string>("route.prefix");
    conf->route_prefix_len  = strtol (value.c_str(), NULL, 0);

    // RD and RT config parsing
    conf->manual_rd         = pt.get <uint8_t>("manual-rd");
    conf->manual_rt         = pt.get <uint8_t>("manual-rt");
    if (conf->manual_rd) {
        uint8_t i = 0;
        for (ptree::value_type &rd : pt.get_child("route-distinguisher")) {
            conf->rd[i++] = rd.second.get_value<uint8_t>();
        }
        printf ("Using Manual RD\n");
    }
    if (conf->manual_rt) {
        uint8_t i = 0;
        for (ptree::value_type &rt : pt.get_child("route-target")) {
            conf->rt[i++] = rt.second.get_value<uint8_t>();
        }
        printf ("Using Manual RT\n");
    }
    } catch (std::exception const&  ex) {
        fprintf(stderr, "Config file %s doesn't have required fields\n",
                file.c_str());
        exit(1);
    }

    return 0;
}

} // end namespace pds_ms_test
