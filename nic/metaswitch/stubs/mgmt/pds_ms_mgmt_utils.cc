// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Purpose: Common helper APIs for metaswitch stub programming 

#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "include/sdk/base.hpp"

#define SHARED_DATA_TYPE CSS_LOCAL
using namespace types;

void
ip_addr_to_spec (const ip_addr_t *ip_addr,
                 types::IPAddress *ip_addr_spec)
{
    if (ip_addr->af == IP_AF_IPV4) {
        ip_addr_spec->set_af(types::IP_AF_INET);
        ip_addr_spec->set_v4addr(ip_addr->addr.v4_addr);
    } else if (ip_addr->af == IP_AF_IPV6) {
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

//TODO: we may need to revisit 'is_zero_ip_valid' flag
NBB_VOID 
pds_ms_convert_ip_addr_to_amb_ip_addr (ip_addr_t     pds_ms_ip_addr, 
                                       NBB_LONG      *type, 
                                       NBB_ULONG     *len, 
                                       NBB_BYTE      *amb_ip_addr,
                                       uint8_t       is_zero_ip_valid)
{
    switch (pds_ms_ip_addr.af)
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

    if (!is_zero_ip_valid && ip_addr_is_zero(&pds_ms_ip_addr)) {
        *len = 0;
        return;
    }

    NBB_MEMCPY (amb_ip_addr, &pds_ms_ip_addr.addr, *len);
    return;
}

// Converts from byte-order to network-order IP Address
NBB_VOID
pds_ms_convert_amb_ip_addr_to_ip_addr (NBB_BYTE      *amb_ip_addr,
                                     NBB_LONG      type,
                                     NBB_ULONG     len,
                                     ip_addr_t     *proto_ip_addr)
{
    switch (type)
    {
        case AMB_INETWK_ADDR_TYPE_IPV4:
            proto_ip_addr->af = IP_AF_IPV4;
            break;

        case AMB_INETWK_ADDR_TYPE_IPV6:
            proto_ip_addr->af = IP_AF_IPV6;
            break;

        case AMB_INETWK_ADDR_TYPE_OTHER:
            return;

        default:
            assert(0);
    }
    NBB_MEMCPY (&proto_ip_addr->addr, amb_ip_addr, len);
    return;
}

NBB_VOID
pds_ms_set_address_oid(NBB_ULONG *oid,
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
    uint8_t         is_zero_ip_valid = FALSE;

    if (strcmp(fieldName, "remote_addr") == 0) {
        if (strcmp(tableName, "bgpPeerAfiSafiTable") == 0) {
            oidAddrTypeIdx = AMB_BGP_PAS_REMOTE_ADD_TYP_IX;
            oidAddrIdx = AMB_BGP_PAS_REMOTE_ADDR_INDEX;
        } else if (strcmp(tableName, "bgpPeerTable") == 0) {
            oidAddrTypeIdx = AMB_BGP_PER_REMOTE_ADD_TYP_IX;
            oidAddrIdx = AMB_BGP_PER_REMOTE_ADDR_INDEX;
        } else if (strcmp(tableName, "bgpPeerAfiSafiStatusTable") == 0) {
            oidAddrTypeIdx = AMB_BGP_PAST_REMOTE_ADDR_TYP_IX;
            oidAddrIdx = AMB_BGP_PAST_REMOTE_ADDR_INDEX;
        } else if (strcmp(tableName, "bgpPeerStatusTable") == 0) {
            oidAddrTypeIdx = AMB_BGP_PRST_LOCAL_ADDR_TYP_IX;
            oidAddrIdx = AMB_BGP_PRST_LOCAL_ADDR_INDEX;
        } else {
            assert(0);
        }
    } else if (strcmp(fieldName, "local_addr") == 0) {
        if (strcmp(tableName, "bgpPeerAfiSafiTable") == 0) {
            oidAddrTypeIdx = AMB_BGP_PAS_LOCAL_ADD_TYP_INDEX;
            oidAddrIdx = AMB_BGP_PAS_LOCAL_ADDR_INDEX;
        } else if (strcmp(tableName, "bgpPeerTable") == 0) {
            oidAddrTypeIdx = AMB_BGP_PER_LOCAL_ADD_TYP_INDEX;
            oidAddrIdx = AMB_BGP_PER_LOCAL_ADDR_INDEX;
        } else if (strcmp(tableName, "bgpPeerAfiSafiStatusTable") == 0) {
            oidAddrTypeIdx = AMB_BGP_PAST_LOCAL_ADDR_TYP_IX;
            oidAddrIdx = AMB_BGP_PAST_LOCAL_ADDR_INDEX;
        } else if (strcmp(tableName, "bgpPeerStatusTable") == 0) {
            oidAddrTypeIdx = AMB_BGP_PRST_REMOTE_ADDR_TYP_IX;
            oidAddrIdx = AMB_BGP_PRST_REMOTE_ADDR_INDEX;
        } else {
            assert(0);
        }
    } else if (strcmp(fieldName, "ipaddress")  == 0) {
        if (strcmp(tableName, "limL3InterfaceAddressTable") == 0) {
            oidAddrTypeIdx = AMB_LIM_L3_ADDR_IPDDR_TYP_INDEX;
            oidAddrIdx = AMB_LIM_L3_ADDR_IPADDR_INDEX;
        }
    } else if (strcmp(fieldName, "dest_addr")  == 0) {
        if (strcmp(tableName, "rtmStaticRtTable") == 0) {
            is_zero_ip_valid = TRUE;
            oidAddrTypeIdx  = AMB_QCR_STRT_DEST_ADDR_TYPE_IX;
            oidAddrIdx      = AMB_QCR_STRT_DEST_ADDR_INDEX;
         }
    } else if (strcmp(fieldName, "next_hop")  == 0) {
        if (strcmp(tableName, "rtmStaticRtTable") == 0) {
            is_zero_ip_valid = TRUE;
            oidAddrTypeIdx  = AMB_QCR_STRT_NEXT_HOP_TYPE_IX;
            oidAddrIdx      = AMB_QCR_STRT_NEXT_HOP_INDEX;
        }
    } else {
        assert(0);
    }

    ip_addr_spec_to_ip_addr(addr, &outAddr);
    pds_ms_convert_ip_addr_to_amb_ip_addr(outAddr, &ambAddrType,
                                          &ambAddrLen, ambAddr, 
                                          is_zero_ip_valid);

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
pds_ms_set_address_field(AMB_GEN_IPS *mib_msg,
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
        if (strcmp(tableName, "bgpPeerAfiSafiTable") == 0) {
            AMB_BGP_PEER_AFI_SAFI *data = (AMB_BGP_PEER_AFI_SAFI *)dest;
            pds_ms_convert_ip_addr_to_amb_ip_addr(outAddr,
                                                &data->remote_addr_type,
                                                &data->remote_addr_len,
                                                data->remote_addr, FALSE);
            addrIdx = AMB_OID_BGP_PAS_REMOTE_ADDR;
            addrType = AMB_OID_BGP_PAS_REMOTE_ADDR_TYP;
        } else if (strcmp(tableName, "bgpPeerTable") == 0) {
            AMB_BGP_PEER *data = (AMB_BGP_PEER *)dest;
            pds_ms_convert_ip_addr_to_amb_ip_addr(outAddr,
                                                &data->remote_addr_type,
                                                &data->remote_addr_len,
                                                data->remote_addr, FALSE);
            addrIdx = AMB_OID_BGP_PER_REMOTE_ADDR;
            addrType = AMB_OID_BGP_PER_REMOTE_DDR_TYP;
        } else {
            assert(0);
        }
    } else if (strcmp(fieldName, "local_addr") == 0) {
        if (strcmp(tableName, "bgpPeerAfiSafiTable") == 0) {
            AMB_BGP_PEER_AFI_SAFI *data = (AMB_BGP_PEER_AFI_SAFI *)dest;
            pds_ms_convert_ip_addr_to_amb_ip_addr(outAddr,
                                                &data->local_addr_type,
                                                &data->local_addr_len,
                                                data->local_addr, FALSE);
            addrIdx = AMB_OID_BGP_PAS_LOCAL_ADDR;
            addrType = AMB_OID_BGP_PAS_LOCAL_ADDR_TYP;
        } else if (strcmp(tableName, "bgpPeerTable") == 0) {
            AMB_BGP_PEER *data = (AMB_BGP_PEER *)dest;
            pds_ms_convert_ip_addr_to_amb_ip_addr(outAddr,
                                                &data->local_addr_type,
                                                &data->local_addr_len,
                                                data->local_addr, FALSE);
            addrIdx = AMB_OID_BGP_PER_LOCAL_ADDR;
            addrType = AMB_OID_BGP_PER_LOCAL_ADDR_TYP;
        } else {
            assert(0);
        }
    } else if (strcmp(fieldName, "ipaddress")  == 0) {
        if (strcmp(tableName, "limL3InterfaceAddressTable") == 0) {
            AMB_LIM_L3_IF_ADDR *data = (AMB_LIM_L3_IF_ADDR *)dest;
            pds_ms_convert_ip_addr_to_amb_ip_addr (outAddr,
                                                 &data->ipaddr_type, 
                                                 &data->ipaddress_len,
                                                 data->ipaddress, FALSE);
            addrIdx  = AMB_OID_LIM_L3_ADDR_IPADDR;
            addrType = AMB_OID_LIM_L3_ADDR_TYPE; 
        }
    } else if (strcmp(fieldName, "dest_addr")  == 0) {
        if (strcmp(tableName, "rtmStaticRtTable") == 0) {
            AMB_CIPR_RTM_STATIC_RT *data = (AMB_CIPR_RTM_STATIC_RT *)dest;
            pds_ms_convert_ip_addr_to_amb_ip_addr (outAddr,
                                                   &data->dest_addr_type,
                                                   &data->dest_addr_len,
                                                   data->dest_addr, TRUE);
            addrType = AMB_OID_QCR_STRT_DEST_ADDR_TYP;
            addrIdx  = AMB_OID_QCR_STRT_DEST_ADDR;
         }
    } else if (strcmp(fieldName, "next_hop")  == 0) {
        if (strcmp(tableName, "rtmStaticRtTable") == 0) {
            AMB_CIPR_RTM_STATIC_RT *data = (AMB_CIPR_RTM_STATIC_RT *)dest;
            pds_ms_convert_ip_addr_to_amb_ip_addr (outAddr,
                                                   &data->next_hop_type,
                                                   &data->next_hop_len,
                                                   data->next_hop, TRUE);
            addrType = AMB_OID_QCR_STRT_NEXT_HOP_TYP;
            addrIdx  = AMB_OID_QCR_STRT_NEXT_HOP;
        }
    } else {
        assert(0);
    }
    AMB_SET_FIELD_PRESENT (mib_msg, addrIdx);
    AMB_SET_FIELD_PRESENT (mib_msg, addrType);
}

NBB_LONG
pds_ms_nbb_get_long(NBB_BYTE *byteVal)
{
    NBB_LONG val;
    NBB_GET_LONG(val, byteVal);
    return val;
}

NBB_VOID
pds_ms_set_string_in_byte_array_with_len(NBB_BYTE *field, NBB_ULONG &len, string in_str)
{
    len = in_str.length();
    NBB_MEMCPY(field, in_str.c_str(), len);
}

NBB_VOID
pds_ms_set_string_in_byte_array_with_len_oid(NBB_ULONG *oid, string in_str, NBB_LONG setKeyOidIdx, NBB_LONG setKeyOidLenIdx)
{
    oid[setKeyOidLenIdx] = (NBB_ULONG)in_str.length();

    auto str = (const unsigned char*) in_str.c_str();
    for (NBB_ULONG i=0; i<in_str.length(); i++) {
        oid[setKeyOidIdx + i] = (NBB_ULONG)str[i];
    }
}

string
pds_ms_get_string_in_byte_array_with_len(NBB_BYTE *in_str, NBB_ULONG len)
{
    std::string ret(in_str, in_str + len);
    return ret;
}

NBB_VOID
pds_ms_get_string_in_byte_array_with_len_oid(NBB_ULONG *oid, string in_str, NBB_LONG getKeyOidIdx, NBB_LONG getKeyOidLenIdx)
{
    oid[getKeyOidLenIdx] = (NBB_ULONG)in_str.length();
    auto str = (const unsigned char* ) in_str.c_str();
    for (NBB_ULONG i=0; i<in_str.length(); i++) {
        oid[getKeyOidIdx + i] = (NBB_ULONG)str[i];
    }
}

NBB_VOID
pds_ms_set_string_in_byte_array(NBB_BYTE *field, string in_str)
{
    NBB_MEMCPY(field, in_str.c_str(), in_str.length());
}

NBB_VOID
pds_ms_set_string_in_byte_array_oid(NBB_ULONG *oid, string in_str, NBB_LONG setKeyOidIdx)
{
    auto str = (const unsigned char*) in_str.c_str();
    for (NBB_ULONG i=0; i<in_str.length(); i++) {
        oid[setKeyOidIdx + i] = (NBB_ULONG)str[i];
    }
}

string
pds_ms_get_string_in_byte_array(NBB_BYTE *val, NBB_ULONG len)
{
    std::string ret(val, val + len);
    return ret;
}

NBB_VOID
pds_ms_get_string_in_byte_array_oid(NBB_ULONG *oid, string in_str, NBB_LONG getKeyOidIdx)
{
    auto str = (const unsigned char*) in_str.c_str();
    for (NBB_ULONG i=0; i<in_str.length(); i++) {
        oid[getKeyOidIdx + i] = (NBB_ULONG)str[i];
    }
}

types::IPAddress   res;

types::IPAddress*
pds_ms_get_address(const NBB_CHAR  *tableName,
                 const NBB_CHAR  *fieldName,
                 NBB_VOID        *src)
{
    ip_addr_t          pds_ms_ip_addr;

    if (strcmp(fieldName, "remote_addr") == 0) {
        if (strcmp(tableName, "bgpPeerAfiSafiStatusTable") == 0) {
            AMB_BGP_PEER_AFI_SAFI_STAT *data = (AMB_BGP_PEER_AFI_SAFI_STAT *)src;
            pds_ms_convert_amb_ip_addr_to_ip_addr(data->remote_addr,
                                                  data->remote_addr_type,
                                                  data->remote_addr_len,
                                                  &pds_ms_ip_addr);
        } else if (strcmp(tableName, "bgpPeerStatusTable") == 0) {
            AMB_BGP_PEER_STATUS *data = (AMB_BGP_PEER_STATUS *)src;
            pds_ms_convert_amb_ip_addr_to_ip_addr(data->remote_addr,
                                                  data->remote_addr_type,
                                                  data->remote_addr_len,
                                                  &pds_ms_ip_addr);
        } else {
            assert(0);
        }
    } else if (strcmp(fieldName, "local_addr") == 0) {
        if (strcmp(tableName, "bgpPeerAfiSafiStatusTable") == 0) {
            AMB_BGP_PEER_AFI_SAFI_STAT *data = (AMB_BGP_PEER_AFI_SAFI_STAT *)src;
            pds_ms_convert_amb_ip_addr_to_ip_addr(data->local_addr,
                                                  data->local_addr_type,
                                                  data->local_addr_len,
                                                  &pds_ms_ip_addr);
        } else if (strcmp(tableName, "bgpPeerStatusTable") == 0) {
            AMB_BGP_PEER_STATUS *data = (AMB_BGP_PEER_STATUS *)src;
            pds_ms_convert_amb_ip_addr_to_ip_addr(data->local_addr,
                                                  data->local_addr_type,
                                                  data->local_addr_len,
                                                  &pds_ms_ip_addr);
        } else {
            assert(0);
        }
    } else if (strcmp(tableName, "rtmRibTable") == 0) {
        AMB_CIPR_RTM_RIB *data = (AMB_CIPR_RTM_RIB*)src;
        if (strcmp(fieldName, "dest") == 0) {
            pds_ms_convert_amb_ip_addr_to_ip_addr(data->dest,
                                                  data->dest_addr_type,
                                                  data->dest_addr_len,
                                                  &pds_ms_ip_addr);
        } else if (strcmp(fieldName, "next_hop") == 0) {
            pds_ms_convert_amb_ip_addr_to_ip_addr(data->next_hop,
                                                  data->next_hop_type,
                                                  data->next_hop_len,
                                                  &pds_ms_ip_addr);
        } else {
            assert(0);
        }
    } else if (strcmp(tableName, "evpnMacIpTable") == 0) {
        AMB_EVPN_MAC_IP *data = (AMB_EVPN_MAC_IP*)src;
        if (strcmp(fieldName, "ip_address") == 0) {
            pds_ms_convert_amb_ip_addr_to_ip_addr(data->ip_address,
                                                  data->ip_address_type,
                                                  data->ip_address_len,
                                                  &pds_ms_ip_addr);
        } else if (strcmp(fieldName, "bgp_nh_addr") == 0) {
            AMB_EVPN_MAC_IP *data = (AMB_EVPN_MAC_IP*)src;
            pds_ms_convert_amb_ip_addr_to_ip_addr(data->bgp_nh_addr,
                                                  data->bgp_nh_addr_type,
                                                  data->bgp_nh_addr_len,
                                                  &pds_ms_ip_addr);
        } else {
            assert(0);
        }
    } else {
        assert(0);
    }
    ip_addr_to_spec(&pds_ms_ip_addr, &res);
    return &res;
}

sdk_ret_t
pds_ms_api_to_sdk_ret (ApiStatus api_err)
{
    switch (api_err) {
    case API_STATUS_OK: {return SDK_RET_OK;}
    case API_STATUS_ERR: {return SDK_RET_ERR;}
    case API_STATUS_INVALID_ARG: {return SDK_RET_INVALID_ARG;}
    case API_STATUS_EXISTS_ALREADY: {return SDK_RET_ENTRY_EXISTS;}
    case API_STATUS_OUT_OF_MEM: {return SDK_RET_OOM;}
    case API_STATUS_NOT_FOUND: {return SDK_RET_ENTRY_NOT_FOUND;}
    case API_STATUS_OUT_OF_RESOURCE: {return SDK_RET_NO_RESOURCE;}
    case API_STATUS_OPERATION_NOT_ALLOWED: {return SDK_RET_INVALID_OP;}
    default: {return SDK_RET_ERR;} // catch undefined errors
    }
    return SDK_RET_ERR;
}
