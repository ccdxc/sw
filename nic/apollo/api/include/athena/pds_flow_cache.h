//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines FLOW CACHE API
///
//----------------------------------------------------------------------------


#ifndef __PDS_FLOW_CACHE_H__
#define __PDS_FLOW_CACHE_H__

#include "nic/sdk/include/sdk/ip.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup PDS_FLOW_CACHE 
/// @{

/// \brief TCP/UDP key fields
typedef struct pds_flow_key_tcp_udp_s {
    uint16_t    sport;    ///< Src port
    uint16_t    dport;    ///< Dest port
} pds_flow_key_tcp_udp_t;

/// \brief ICMP key fields
typedef struct pds_flow_key_icmp_s {
    uint8_t    type;          ///< Type
    uint8_t    code;          ///< Code
    uint8_t    identifier;    ///< Identifier
} pds_flow_key_icmp_t;

/// \brief L4 specific key fields
typedef union pds_flow_key_l4_s {
    pds_flow_key_tcp_udp_t    tcp_udp;    ///< TCP/UDP key fields
    pds_flow_key_icmp_t       icmp;       ///< ICMP key fields
} pds_flow_key_l4_t;

/// \brief Generic flow key
typedef struct pds_flow_key_s {
    uint16_t             vnic_id;                    ///< VNIC id
    uint64_t             smac;                       ///< Src MAC addr
    uint64_t             dmac;                       ///< Dest MAC addr
    uint8_t              ip_addr_family;             ///< IP addr family
    uint8_t              ip_saddr[IP6_ADDR8_LEN];    ///< IP src addr
    uint8_t              ip_daddr[IP6_ADDR8_LEN];    ///< IP dest addr
    uint8_t              ip_proto;                   ///< IP protocol
    pds_flow_key_l4_t    l4;                         ///< L4 key fields
} __PACK__ pds_flow_key_t;

/// \brief Index type
typedef enum pds_flow_spec_index_type_e {
    PDS_FLOW_SPEC_INDEX_SESSION = 0,    /// Session index
    PDS_FLOW_SPEC_INDEX_CONNTRACK,      /// Conntrack index
} pds_flow_spec_index_type_t;

/// \brief Flow data
typedef struct pds_flow_data_s {
    pds_flow_spec_index_type_t     index_type;    ///< index type
    uint32_t                       index;         ///< session index
} __PACK__ pds_flow_data_t;

/// \brief Flow spec
typedef struct pds_flow_spec_s {
    pds_flow_key_t     key;     ///< flow key
    pds_flow_data_t    data;    ///< flow data
} __PACK__ pds_flow_spec_t;

/// \brief Flow status
typedef struct pds_flow_status_s {
} __PACK__ pds_flow_status_t; 

/// \brief Flow information
typedef struct pds_flow_info_s {
    pds_flow_spec_t      spec;        ///< specification
    pds_flow_status_t    status;      ///< operational status
} __PACK__ pds_flow_info_t;


/// \brief     create flow cache table
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    This needs to be called precise once by application.
sdk_ret_t pds_flow_cache_create(void);

/// \brief     set valid core id.
/// \return    void
/// \remark    A valid core id should be passed
//             This needs to be called on every data core of the application
void pds_flow_cache_set_core_id(uint32_t core_id);


/// \brief     create flow cache entry
/// \param[in] spec flow specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid session info ID and key should be passed
sdk_ret_t pds_flow_cache_entry_create(pds_flow_spec_t *spec);

/// \brief     read flow cache entry
/// \param[in] key flow key
/// \param[out] info flow information
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid flow key should be passed
sdk_ret_t pds_flow_cache_entry_read(pds_flow_key_t *key,
                                    pds_flow_info_t *info);

/// \brief     update flow cache entry
/// \param[in] spec flow specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid session info ID and key should be passed
sdk_ret_t pds_flow_cache_entry_update(pds_flow_spec_t *spec);

/// \brief     delete flow cache entry
/// \param[in] key flow key
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid flow key should be passed
sdk_ret_t pds_flow_cache_entry_delete(pds_flow_key_t *key);

/// @}

#ifdef __cplusplus
}
#endif

#endif  // __PDS_FLOW_CACHE_H__
