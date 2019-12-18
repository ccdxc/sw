//----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//----------------------------------------------------------------------------
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "nic/metaswitch/stubs/test/common/test_config.hpp"
#include <arpa/inet.h>
#include <iostream>

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
    try
    {
        // if there is test-config string, read it
        test_config = pt.get <uint32_t>("test-config", 0);
    }
    catch (std::exception const&  ex)
    {
        // Otherwise, test config is not required
        fprintf(stderr, "Config file %s doesn't have test-config enabled!\n",
                file.c_str());
    }

    if (!test_config)
    {
        return -1;
    }

    value                   = pt.get <std::string>("local.ip","");
    conf->local_ip_addr     = inet_network (value.c_str());
    value                   = pt.get <std::string>("local.gwip","");
    conf->local_gwip_addr   = inet_network (value.c_str());
    value                   = pt.get <std::string>("local.ac-ip","");
    conf->local_mai_ip      = inet_network (value.c_str());
    value                   = pt.get <std::string>("remote.ip","");
    conf->remote_ip_addr    = inet_network (value.c_str());
    value                   = pt.get <std::string>("eth-if-index","");
    conf->eth_if_index      = strtol (value.c_str(),NULL, 0);
    value                   = pt.get <std::string>("lif-if-index","");
    conf->lif_if_index      = strtol (value.c_str(),NULL, 0);
    value                   = pt.get <std::string>("local.as","");
    conf->local_asn         = strtol (value.c_str(),NULL, 0);
    value                   = pt.get <std::string>("remote.as","");
    conf->remote_asn        = strtol (value.c_str(),NULL, 0);
    value                   = pt.get <std::string>("vni","");
    conf->vni               = strtol (value.c_str(),NULL, 0);

    return 0;
}

} // end namespace pds_ms_test
