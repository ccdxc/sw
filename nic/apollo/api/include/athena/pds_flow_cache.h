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

/// \brief Flow direction
typedef enum pds_flow_direction_e {
    PDS_FLOW_DIR_INVALID = 0,       ///< Invalid direction
    PDS_FLOW_DIR_HOST_TO_SWITCH,    ///< Host to switch direction
    PDS_FLOW_DIR_SWITCH_TO_HOST     ///< Switch to host direction
} pds_flow_direction_t;

/// \brief Flow key for host to switch direction
typedef struct pds_flow_key_host_to_switch_s {
    uint16_t    vlan_id;                       ///< vlan id
    uint8_t     ip_addr_family;                ///< IP addr family
    uint8_t     ip_saddr[INET6_ADDRSTRLEN];    ///< IP src addr
    uint8_t     ip_daddr[INET6_ADDRSTRLEN];    ///< IP dst addr
    uint8_t     ip_proto;                      ///< IP protocol
    uint16_t    l4_sport;                      ///< L4 src port
    uint16_t    l4_dport;                      ///< L4 dest port
    uint8_t     tcp_flags;                     ///< TCP flags
} __PACK__ pds_flow_key_host_to_switch_t;

/// \brief Flow key for switch to host direction
typedef struct pds_flow_key_switch_to_host_s {
    uint32_t    mpls1_label;                   ///< MPLS label 1
    uint32_t    mpls2_label;                   ///< MPLS label 2
    uint8_t     ip_addr_family;                ///< IP addr family
    uint8_t     ip_saddr[INET6_ADDRSTRLEN];    ///< IP src addr
    uint8_t     ip_daddr[INET6_ADDRSTRLEN];    ///< IP dest addr
    uint8_t     ip_proto;                      ///< IP protocol
    uint16_t    l4_sport;                      ///< L4 src port
    uint16_t    l4_dport;                      ///< L4 dest port
    uint8_t     tcp_flags;                     ///< TCP flags
} __PACK__ pds_flow_key_switch_to_host_t;

/// \brief Generic flow key 
typedef struct pds_flow_key_s {
    pds_flow_direction_t                 flow_dir;          ///< flow direction
    union {
        pds_flow_key_host_to_switch_t    host_to_switch;    ///< host to sw key
        pds_flow_key_switch_to_host_t    switch_to_host;    ///< sw to host key
    } u;
} __PACK__ pds_flow_key_t;

/// \brief Flow data
typedef struct pds_flow_data_s {
    uint32_t    session_info_id;    ///< session info index
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
/// \param[in] core_id core id
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid core id should be passed
//             This needs to be called on every data core of the application
sdk_ret_t pds_flow_cache_create(uint32_t core_id);

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
