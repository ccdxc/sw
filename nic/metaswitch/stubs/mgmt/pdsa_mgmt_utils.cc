// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Common helper APIs for metaswitch stub programming 

#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#define SHARED_DATA_TYPE CSS_LOCAL

using boost::property_tree::ptree;
using namespace std;

NBB_VOID 
pdsa_convert_ip_addr_to_amb_ip_addr (ip_addr_t     pdsa_ip_addr, 
                                     NBB_LONG      *type, 
                                     NBB_ULONG     *len, 
                                     NBB_BYTE      *amb_ip_addr)
{
    switch (pdsa_ip_addr.af)
    {
        case IP_AF_IPV4:
            *type = AMB_INETWK_ADDR_TYPE_IPV4;
            *len = AMB_MAX_IPV4_ADDR_LEN;
            break;

        case IP_AF_IPV6:
            *type = AMB_INETWK_ADDR_TYPE_IPV6;
            *len = AMB_MAX_IPV6_ADDR_LEN;
            break;

        default:
            *type = *len = 0;
            return;
    }

    NBB_MEMCPY (amb_ip_addr, &pdsa_ip_addr.addr, *len);
    return;
}

NBB_VOID
pdsa_convert_long_to_pdsa_ipv4_addr (NBB_ULONG ip, ip_addr_t *pdsa_ip_addr)
{
    pdsa_ip_addr->af            = IP_AF_IPV4;
    pdsa_ip_addr->addr.v4_addr  = htonl(ip);
}

int
parse_json_config (pdsa_config_t *conf) {
    ptree       pt;
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
    value           = pt.get <std::string>("local.ip","");
    conf->g_node_a_ip     = inet_network (value.c_str());
    value           = pt.get <std::string>("local.ac-ip","");
    conf->g_node_a_ac_ip  = inet_network (value.c_str());
    value           = pt.get <std::string>("remote.ip","");
    conf->g_node_b_ip     = inet_network (value.c_str());
    value           = pt.get <std::string>("remote.ac-ip","");
    conf->g_node_b_ac_ip  = inet_network (value.c_str());
    value           = pt.get <std::string>("if-index","");
    conf->g_evpn_if_index = strtol (value.c_str(),NULL, 0);

    return 0;
}

