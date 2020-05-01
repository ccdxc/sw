//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines DNAT MAP API
///
//----------------------------------------------------------------------------


#ifndef __PDS_DNAT_H__
#define __PDS_DNAT_H__

#include "pds_base.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup PDS_DNAT
/// @{

/// \brief Dnat map key
typedef struct pds_dnat_mapping_key_s {
    uint16_t    vnic_id;                 ///< Vnic id
    uint8_t     key_type;               ///< Key type
    uint8_t     addr[IP6_ADDR8_LEN];    ///< IP address
} pds_dnat_mapping_key_t;

/// \brief Dnat map data
typedef struct pds_dnat_mapping_data_s {
    uint8_t    addr_type;              ///< Address type
    uint8_t    addr[IP6_ADDR8_LEN];    ///< IP address
    uint16_t   epoch;                  ///< DNAT Epoch Value
} pds_dnat_mapping_data_t;

/// \brief Dnat map specification
typedef struct pds_dnat_mapping_spec_s {
    pds_dnat_mapping_key_t     key;     ///< Dnat map key
    pds_dnat_mapping_data_t    data;    ///< Dnat map data
} pds_dnat_mapping_spec_t;

/// \brief Dnat map info
typedef struct pds_dnat_mapping_info_s {
    pds_dnat_mapping_spec_t      spec;      ///< Specification
} pds_dnat_mapping_info_t;

/// \brief     create dnat map entry
/// \param[in] spec dnat mapping specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid IP address and vnic id should be passed
pds_ret_t pds_dnat_map_entry_create(pds_dnat_mapping_spec_t *spec);

/// \brief     read dnat mapping entry
/// \param[in] key dnat mapping key
/// \param[out] info dnat mapping info
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid IP address and vnic id should be passed
pds_ret_t pds_dnat_map_entry_read(pds_dnat_mapping_key_t *key,
                                  pds_dnat_mapping_info_t *info);

/// \brief     update dnat map entry
/// \param[in] spec dnat mapping specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid IP address and vnic id should be passed
pds_ret_t pds_dnat_map_entry_update(pds_dnat_mapping_spec_t *spec);

/// \brief     delete dnat map entry
/// \param[in] key dnat mapping key
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid IP address and vnic id should be passed
pds_ret_t pds_dnat_map_entry_delete(pds_dnat_mapping_key_t *key);

/// \brief     clear the DNAT mapping table
/// \return    #SDK_RET_OK on success, failure status code on error
pds_ret_t pds_dnat_map_table_clear(void);

/// @}

#ifdef __cplusplus
}
#endif

#endif  // __PDS_DNAT_H__
