// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Common helper APIs for metaswitch stub programming 

#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#define SHARED_DATA_TYPE CSS_LOCAL

using boost::property_tree::ptree;
using namespace std;

void
ip_addr_to_spec (const ip_addr_t *ip_addr,
                 types::IPAddress *ip_addr_spec)
{
    if (ip_addr->af == IP_AF_IPV4) {
        ip_addr_spec->set_af(types::IP_AF_INET);
        ip_addr_spec->set_v4addr(ip_addr->addr.v4_addr);
    } else {
        ip_addr_spec->set_af(types::IP_AF_INET6);
        ip_addr_spec->set_v6addr(ip_addr->addr.v6_addr.addr8, IP6_ADDR8_LEN);
    }

    return;
}

bool
ip_addr_spec_to_ip_addr (const types::IPAddress& in_ipaddr,
                         ip_addr_t *out_ipaddr)
{
    memset(out_ipaddr, 0, sizeof(ip_addr_t));
    if (in_ipaddr.af() == types::IP_AF_INET) {
        out_ipaddr->af = IP_AF_IPV4;
        out_ipaddr->addr.v4_addr = in_ipaddr.v4addr();
    } else if (in_ipaddr.af() == types::IP_AF_INET6) {
        out_ipaddr->af = IP_AF_IPV6;
        memcpy(out_ipaddr->addr.v6_addr.addr8,
               in_ipaddr.v6addr().c_str(),
               IP6_ADDR8_LEN);
    } else {
        return false;
    }

    return true;
}

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
pdsa_convert_amb_ip_addr_to_ip_addr (NBB_BYTE      *amb_ip_addr,
                                     NBB_LONG      type,
                                     NBB_ULONG     len,
                                     ip_addr_t     *pdsa_ip_addr)
{
    switch (type)
    {
        case AMB_INETWK_ADDR_TYPE_IPV4:
            pdsa_ip_addr->af = IP_AF_IPV4;
            break;

        case AMB_INETWK_ADDR_TYPE_IPV6:
            pdsa_ip_addr->af = IP_AF_IPV6;
            break;

        default:
            assert(0);
    }

    NBB_MEMCPY (&pdsa_ip_addr->addr, amb_ip_addr, len);
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

NBB_VOID
pdsa_set_address_oid(NBB_ULONG *oid,
                     const NBB_CHAR  *tableName,
                     const NBB_CHAR  *fieldName,
                     const types::IPAddress &addr)
{
    NBB_ULONG       oidAddrTypeIdx;
    NBB_ULONG       oidAddrIdx;
    NBB_LONG        ambAddrType;
    NBB_ULONG       ambAddrLen;
    NBB_BYTE        ambAddr[AMB_BGP_MAX_IP_PREFIX_LEN];
    ip_addr_t       outAddr;
    NBB_ULONG       ii = 0;

    if (strcmp(fieldName, "remote_addr") == 0) {
        if (strcmp(fieldName, "bgpPeerAfiSafiTable") == 0) {
            oidAddrTypeIdx = AMB_BGP_PAS_REMOTE_ADD_TYP_IX;
            oidAddrIdx = AMB_BGP_PAS_REMOTE_ADDR_INDEX;
        } else if (strcmp(fieldName, "bgpPeerTable") == 0) {
            oidAddrTypeIdx = AMB_BGP_PER_REMOTE_ADD_TYP_IX;
            oidAddrIdx = AMB_BGP_PER_REMOTE_ADDR_INDEX;
        } else if (strcmp(fieldName, "bgpPeerAfiSafiStatusTable") == 0) {
            oidAddrTypeIdx = AMB_BGP_PAST_REMOTE_ADDR_TYP_IX;
            oidAddrIdx = AMB_BGP_PAST_REMOTE_ADDR_INDEX;
        } else if (strcmp(fieldName, "bgpPeerStatusTable") == 0) {
            oidAddrTypeIdx = AMB_BGP_PRST_LOCAL_ADDR_TYP_IX;
            oidAddrIdx = AMB_BGP_PRST_LOCAL_ADDR_INDEX;
        } else {
            assert(0);
        }
    } else if (strcmp(fieldName, "local_addr") == 0) {
        if (strcmp(fieldName, "bgpPeerAfiSafiTable") == 0) {
            oidAddrTypeIdx = AMB_BGP_PAS_LOCAL_ADD_TYP_INDEX;
            oidAddrIdx = AMB_BGP_PAS_LOCAL_ADDR_INDEX;
        } else if (strcmp(fieldName, "bgpPeerTable") == 0) {
            oidAddrTypeIdx = AMB_BGP_PER_LOCAL_ADD_TYP_INDEX;
            oidAddrIdx = AMB_BGP_PER_LOCAL_ADDR_INDEX;
        } else if (strcmp(fieldName, "bgpPeerAfiSafiStatusTable") == 0) {
            oidAddrTypeIdx = AMB_BGP_PAST_LOCAL_ADDR_TYP_IX;
            oidAddrIdx = AMB_BGP_PAST_LOCAL_ADDR_INDEX;
        } else if (strcmp(fieldName, "bgpPeerStatusTable") == 0) {
            oidAddrTypeIdx = AMB_BGP_PRST_REMOTE_ADDR_TYP_IX;
            oidAddrIdx = AMB_BGP_PRST_REMOTE_ADDR_INDEX;
        } else {
            assert(0);
        }
    } else {
        assert(0);
    }

    ip_addr_spec_to_ip_addr(addr, &outAddr);
    pdsa_convert_ip_addr_to_amb_ip_addr(outAddr, &ambAddrType, &ambAddrLen, ambAddr);

    // Fill oid now
    oid[oidAddrTypeIdx]   = ambAddrType;
    oid[oidAddrIdx]       = ambAddrLen;
    for (ii = 0; ii < ambAddrLen; ii++)
    {
        oid[oidAddrIdx + 1 + ii] =
            (NBB_ULONG) ambAddr[ii];
    }
}

NBB_VOID
pdsa_set_address_field(AMB_GEN_IPS *mib_msg,
                       const NBB_CHAR  *tableName,
                       const NBB_CHAR  *fieldName,
                       NBB_VOID        *dest,
                       const types::IPAddress &addr)
{
    NBB_ULONG       addrIdx;
    NBB_ULONG       addrType;
    ip_addr_t       outAddr;

    ip_addr_spec_to_ip_addr(addr, &outAddr);

    if (strcmp(fieldName, "remote_addr") == 0) {
        if (strcmp(fieldName, "bgpPeerAfiSafiTable") == 0) {
            AMB_BGP_PEER_AFI_SAFI *data = (AMB_BGP_PEER_AFI_SAFI *)dest;
            pdsa_convert_ip_addr_to_amb_ip_addr(outAddr,
                                                &data->remote_addr_type,
                                                &data->remote_addr_len,
                                                data->remote_addr);
            addrIdx = AMB_OID_BGP_PAS_REMOTE_ADDR;
            addrType = AMB_OID_BGP_PAS_REMOTE_ADDR_TYP;
        } else if (strcmp(fieldName, "bgpPeerTable") == 0) {
            AMB_BGP_PEER *data = (AMB_BGP_PEER *)dest;
            pdsa_convert_ip_addr_to_amb_ip_addr(outAddr,
                                                &data->remote_addr_type,
                                                &data->remote_addr_len,
                                                data->remote_addr);
            addrIdx = AMB_OID_BGP_PER_REMOTE_ADDR;
            addrType = AMB_OID_BGP_PER_REMOTE_DDR_TYP;
        } else {
            assert(0);
        }
    } else if (strcmp(fieldName, "local_addr") == 0) {
        if (strcmp(fieldName, "bgpPeerAfiSafiTable") == 0) {
            AMB_BGP_PEER_AFI_SAFI *data = (AMB_BGP_PEER_AFI_SAFI *)dest;
            pdsa_convert_ip_addr_to_amb_ip_addr(outAddr,
                                                &data->local_addr_type,
                                                &data->local_addr_len,
                                                data->local_addr);
            addrIdx = AMB_OID_BGP_PAS_LOCAL_ADDR;
            addrType = AMB_OID_BGP_PAS_LOCAL_ADDR_TYP;
        } else if (strcmp(fieldName, "bgpPeerTable") == 0) {
            AMB_BGP_PEER *data = (AMB_BGP_PEER *)dest;
            pdsa_convert_ip_addr_to_amb_ip_addr(outAddr,
                                                &data->local_addr_type,
                                                &data->local_addr_len,
                                                data->local_addr);
            addrIdx = AMB_OID_BGP_PER_LOCAL_ADDR;
            addrType = AMB_OID_BGP_PER_LOCAL_ADDR_TYP;
        } else {
            assert(0);
        }
    } else {
        assert(0);
    }
    AMB_SET_FIELD_PRESENT (mib_msg, addrIdx);
    AMB_SET_FIELD_PRESENT (mib_msg, addrType);
}

NBB_LONG
pdsa_nbb_get_long(NBB_BYTE *byteVal)
{
    NBB_LONG val;
    NBB_GET_LONG(val, byteVal);
    return val;
}

types::IPAddress   res;

types::IPAddress*
pdsa_get_address(const NBB_CHAR  *tableName,
                 const NBB_CHAR  *fieldName,
                 NBB_VOID        *src)
{
    ip_addr_t          pdsa_ip_addr;

    if (strcmp(fieldName, "remote_addr") == 0) {
        if (strcmp(fieldName, "bgpPeerAfiSafiStatusTable") == 0) {
            AMB_BGP_PEER_AFI_SAFI_STAT *data = (AMB_BGP_PEER_AFI_SAFI_STAT *)src;
            pdsa_convert_amb_ip_addr_to_ip_addr(data->remote_addr,
                                                data->remote_addr_type,
                                                data->remote_addr_len,
                                                &pdsa_ip_addr);
        } else if (strcmp(fieldName, "bgpPeerStatusTable") == 0) {
            AMB_BGP_PEER_STATUS *data = (AMB_BGP_PEER_STATUS *)src;
            pdsa_convert_amb_ip_addr_to_ip_addr(data->remote_addr,
                                                data->remote_addr_type,
                                                data->remote_addr_len,
                                                &pdsa_ip_addr);
        } else {
            assert(0);
        }
    } else if (strcmp(fieldName, "local_addr") == 0) {
        if (strcmp(fieldName, "bgpPeerAfiSafiStatusTable") == 0) {
            AMB_BGP_PEER_AFI_SAFI_STAT *data = (AMB_BGP_PEER_AFI_SAFI_STAT *)src;
            pdsa_convert_amb_ip_addr_to_ip_addr(data->local_addr,
                                                data->local_addr_type,
                                                data->local_addr_len,
                                                &pdsa_ip_addr);
        } else if (strcmp(fieldName, "bgpPeerStatusTable") == 0) {
            AMB_BGP_PEER_STATUS *data = (AMB_BGP_PEER_STATUS *)src;
            pdsa_convert_amb_ip_addr_to_ip_addr(data->local_addr,
                                                data->local_addr_type,
                                                data->local_addr_len,
                                                &pdsa_ip_addr);
        } else {
            assert(0);
        }
    } else {
        assert(0);
    }
    ip_addr_to_spec(&pdsa_ip_addr, &res);
    return &res;
}
