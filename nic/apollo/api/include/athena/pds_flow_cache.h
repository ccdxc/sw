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

#define PDS_FLOW_TABLE_MAX_RECIRC    (8)

/// \brief Key type
typedef enum pds_key_type_e {
    KEY_TYPE_INVALID = 0,
    KEY_TYPE_IPV4,          ///< IPV4
    KEY_TYPE_IPV6,          ///< IPV6
    KEY_TYPE_L2,            ///< L2 flow
    KEY_TYPE_MAX
} pds_key_type_t;

/// \brief TCP/UDP key fields
typedef struct pds_flow_key_tcp_udp_s {
    uint16_t    sport;    ///< Src port
    uint16_t    dport;    ///< Dest port
} pds_flow_key_tcp_udp_t;

/// \brief ICMP key fields
typedef struct pds_flow_key_icmp_s {
    uint8_t    type;          ///< Type
    uint8_t    code;          ///< Code
    uint16_t   identifier;    ///< Identifier
} pds_flow_key_icmp_t;

/// \brief L4 specific key fields
typedef union pds_flow_key_l4_s {
    pds_flow_key_tcp_udp_t    tcp_udp;    ///< TCP/UDP key fields
    pds_flow_key_icmp_t       icmp;       ///< ICMP key fields
} pds_flow_key_l4_t;

/// \brief Generic flow key
typedef struct pds_flow_key_s {
    uint16_t             vnic_id;                    ///< VNIC id
    pds_key_type_t       key_type;                   ///< Key type
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

/// \brief Flow iterate callback arg
typedef struct pds_flow_iter_cb_arg_s {
    pds_flow_key_t     flow_key;
    pds_flow_data_t    flow_appdata;
} __PACK__ pds_flow_iter_cb_arg_t;

/// \brief Flow statistics
typedef struct pds_flow_stats_t {
    // Flow cache API stats
    uint64_t    api_insert;
    uint64_t    api_insert_duplicate;
    uint64_t    api_insert_fail;
    uint64_t    api_insert_recirc_fail;
    uint64_t    api_remove;
    uint64_t    api_remove_not_found;
    uint64_t    api_remove_fail;
    uint64_t    api_update;
    uint64_t    api_update_fail;
    uint64_t    api_get;
    uint64_t    api_get_fail;
    uint64_t    api_reserve;
    uint64_t    api_reserve_fail;
    uint64_t    api_release;
    uint64_t    api_release_fail;

    // Flow cache table stats
    uint64_t    table_entries;
    uint64_t    table_collisions;
    uint64_t    table_insert;
    uint64_t    table_remove;
    uint64_t    table_read;
    uint64_t    table_write;
    uint64_t    table_insert_lvl[PDS_FLOW_TABLE_MAX_RECIRC];
    uint64_t    table_remove_lvl[PDS_FLOW_TABLE_MAX_RECIRC];
} __PACK__ pds_flow_stats_t;

/// \brief     Flow iterate callback function type
/// \remark    This function needs to be defined by the application.
typedef void (*pds_flow_iter_cb_t) (pds_flow_iter_cb_arg_t *);

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

/// \brief     iterate through flow cache table
/// \param[in] iterate callback function
///  \param[in] iterate callback argument
/// \return    #SDK_RET_OK on success, failure status code on error
sdk_ret_t pds_flow_cache_entry_iterate(pds_flow_iter_cb_t iter_cb,
                                       pds_flow_iter_cb_arg_t *iter_cb_arg);

/// \brief     get flow cache statistics
/// \param[in] core_id of the thread
/// \param[out] stats flow statistics
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    If core id is passed as -1, local thread id is used
///            From control thread, a valid data core id should be passed
///            This needs to be called from control/master core
///            for every data core of the application
sdk_ret_t pds_flow_cache_stats_get(int32_t core_id, pds_flow_stats_t *stats);

/// @}

#ifdef __cplusplus
}
#endif

#endif  // __PDS_FLOW_CACHE_H__
