//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines L2 FLOW CACHE API
///
//----------------------------------------------------------------------------

#ifndef P4_14
#ifndef __PDS_L2_FLOW_CACHE_H__
#define __PDS_L2_FLOW_CACHE_H__

#include "pds_base.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup PDS_L2_FLOW_CACHE 
/// @{

#define PDS_L2_FLOW_TABLE_MAX_RECIRC    (8)


/// \brief Generic l2 flow key
typedef struct pds_l2_flow_key_s {
    uint16_t             vnic_id;                    ///< VNIC id
    uint8_t              dmac[ETH_ADDR_LEN];         ///< MAC dest add 
} pds_l2_flow_key_t;

/// \brief L2_flow data
typedef struct pds_l2_flow_data_s {
    uint32_t                       index;         ///< session index
} pds_l2_flow_data_t;

/// \brief L2 Flow spec
typedef struct pds_l2_flow_spec_s {
    pds_l2_flow_key_t     key;     ///< l2_flow key
    pds_l2_flow_data_t    data;    ///< l2_flow data
} pds_l2_flow_spec_t;

/// \brief L2 Flow information
typedef struct pds_l2_flow_info_s {
    pds_l2_flow_spec_t      spec;        ///< specification
} pds_l2_flow_info_t;

/// \brief L2 Flow iterate callback arg
typedef struct pds_l2_flow_iter_cb_arg_s {
    bool                  force_read;
    pds_l2_flow_key_t     l2_flow_key;
    pds_l2_flow_data_t    l2_flow_appdata;
} pds_l2_flow_iter_cb_arg_t;

/// \brief L2 Flow statistics
typedef struct pds_l2_flow_stats_t {
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
    uint64_t    table_insert_lvl[PDS_L2_FLOW_TABLE_MAX_RECIRC];
    uint64_t    table_remove_lvl[PDS_L2_FLOW_TABLE_MAX_RECIRC];
} pds_l2_flow_stats_t;

/// \brief     Flow iterate callback function type
/// \remark    This function needs to be defined by the application.
typedef void (*pds_l2_flow_iter_cb_t) (pds_l2_flow_iter_cb_arg_t *);

/// \brief     create l2 flow cache entry
/// \param[in] spec flow specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid session info ID and key should be passed
pds_ret_t pds_l2_flow_cache_entry_create(pds_l2_flow_spec_t *spec);

/// \brief     read l2 flow cache entry
/// \param[in] key l2 flow key
/// \param[out] info flow information
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid l2 flow key should be passed
pds_ret_t pds_l2_flow_cache_entry_read(pds_l2_flow_key_t *key,
                                    pds_l2_flow_info_t *info);

/// \brief     update l2 flow cache entry
/// \param[in] spec l2 flow specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid session info ID and key should be passed
pds_ret_t pds_l2_flow_cache_entry_update(pds_l2_flow_spec_t *spec);

/// \brief     delete l2 flow cache entry
/// \param[in] key l2 flow key
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid flow key should be passed
pds_ret_t pds_l2_flow_cache_entry_delete(pds_l2_flow_key_t *key);

/// \brief     delete l2 flow cache entry corresponding to flow info
/// \param[out] info l2 flow information
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid session info ID should be passed
//pds_ret_t pds_l2_flow_cache_entry_delete_by_flow_info(pds_l2_flow_info_t *info);

/// \brief     iterate through l2 flow cache table
/// \param[in] iterate callback function
///  \param[in] iterate callback argument
/// \return    #SDK_RET_OK on success, failure status code on error
pds_ret_t pds_l2_flow_cache_entry_iterate(pds_l2_flow_iter_cb_t iter_cb,
                                       pds_l2_flow_iter_cb_arg_t *iter_cb_arg);

/// \brief     get l2 flow cache statistics
/// \param[in] core_id of the thread
/// \param[out] stats l2 flow statistics
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    If core id is passed as -1, local thread id is used
///            From control thread, a valid data core id should be passed
///            This needs to be called from control/master core
///            for every data core of the application
pds_ret_t pds_l2_flow_cache_stats_get(int32_t core_id, pds_l2_flow_stats_t *stats);

/// \brief     clear the flow cache table
/// \return    #SDK_RET_OK on success, failure status code on error
pds_ret_t pds_l2_flow_cache_table_clear(void);

/// @}

#ifdef __cplusplus
}
#endif

#endif  // __PDS_L2_FLOW_CACHE_H__
#endif
