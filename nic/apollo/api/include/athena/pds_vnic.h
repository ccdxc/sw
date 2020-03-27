//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines VNIC API
///
//----------------------------------------------------------------------------


#ifndef __PDS_VNIC_H__
#define __PDS_VNIC_H__

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup PDS_VNIC
/// @{

/// \brief Maximum table indices
#define PDS_VLAN_ID_MAX       (4096)
#define PDS_MPLS_LABEL_MAX    (1048576)
#define PDS_VNIC_ID_MAX       (512)

/// \brief VNIC type
typedef enum pds_vnic_type_e {
    VNIC_TYPE_L3 = 0,    ///< L3 vnic
    VNIC_TYPE_L2,        ///< L2 vnic 
} pds_vnic_type_t; 

/// \brief Vlan to Vnic map key
typedef struct pds_vlan_to_vnic_map_key_s {
    uint16_t    vlan_id;    ///< VLAN id
} __PACK__ pds_vlan_to_vnic_map_key_t;

/// \brief Vlan to Vnic map data
typedef struct pds_vlan_to_vnic_map_data_s {
    pds_vnic_type_t    vnic_type;    ///< VNIC type
    uint16_t           vnic_id;      ///< VNIC id
} __PACK__ pds_vlan_to_vnic_map_data_t;

/// \brief Vlan to Vnic map specification
typedef struct pds_vlan_to_vnic_map_spec_s {
    pds_vlan_to_vnic_map_key_t     key;     ///< Vlan to Vnic key
    pds_vlan_to_vnic_map_data_t    data;    ///< Vlan to Vnic data
} __PACK__ pds_vlan_to_vnic_map_spec_t;

/// \brief Vlan to Vnic map status
typedef struct pds_vlan_to_vnic_map_status_s {
} __PACK__ pds_vlan_to_vnic_map_status_t;

/// \brief Vlan to Vnic map info
typedef struct pds_vlan_to_vnic_map_info_s {
    pds_vlan_to_vnic_map_spec_t      spec;      ///< Specification
    pds_vlan_to_vnic_map_status_t    status;    ///< Status
} __PACK__ pds_vlan_to_vnic_map_info_t;


/// \brief Mpls label to Vnic map key
typedef struct pds_mpls_label_to_vnic_map_key_s {
    uint32_t    mpls_label;    ///< MPLS label
} __PACK__ pds_mpls_label_to_vnic_map_key_t;

/// \brief Mpls label to Vnic map data
typedef struct pds_mpls_label_to_vnic_map_data_s {
    pds_vnic_type_t    vnic_type;    ///< VNIC type
    uint16_t           vnic_id;      ///< VNIC id
} __PACK__ pds_mpls_label_to_vnic_map_data_t;

/// \brief Mpls label to Vnic map specification
typedef struct pds_mpls_label_to_vnic_map_spec_s {
    pds_mpls_label_to_vnic_map_key_t     key;     ///< Mpls label to Vnic key
    pds_mpls_label_to_vnic_map_data_t    data;    ///< Mpls label to Vnic data
} __PACK__ pds_mpls_label_to_vnic_map_spec_t;

/// \brief Mpls label to Vnic map status
typedef struct pds_mpls_label_to_vnic_map_status_s {
} __PACK__ pds_mpls_label_to_vnic_map_status_t;

/// \brief Mpls label to Vnic map info
typedef struct pds_mpls_label_to_vnic_map_info_s {
    pds_mpls_label_to_vnic_map_spec_t      spec;      ///< Specification
    pds_mpls_label_to_vnic_map_status_t    status;    ///< Status
} __PACK__ pds_mpls_label_to_vnic_map_info_t;

/// \brief     create vlan to vnic map entry
/// \param[in] spec vlan to vnic map specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid vnic id and vlan id should be passed
sdk_ret_t pds_vlan_to_vnic_map_create(pds_vlan_to_vnic_map_spec_t *spec);

/// \brief     read vlan to vnic map entry
/// \param[in] key vlan id
/// \param[out] info vlan to vnic map info
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid vlan id should be passed
sdk_ret_t pds_vlan_to_vnic_map_read(pds_vlan_to_vnic_map_key_t *key,
                                    pds_vlan_to_vnic_map_info_t *info);

/// \brief     update vlan to vnic map entry
/// \param[in] spec vlan to vnic map specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid vnic id and vlan id should be passed
sdk_ret_t pds_vlan_to_vnic_map_update(pds_vlan_to_vnic_map_spec_t *spec);

/// \brief     delete vlan to vnic map entry
/// \param[in] key vlan id
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid vlan id should be passed
sdk_ret_t pds_vlan_to_vnic_map_delete(pds_vlan_to_vnic_map_key_t *key);

/// \brief     create mpls label to vnic map entry
/// \param[in] spec mpls label to vnic map specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid vnic id and mpls label should be passed
sdk_ret_t pds_mpls_label_to_vnic_map_create(pds_mpls_label_to_vnic_map_spec_t *spec);

/// \brief     read mpls label to vnic map entry
/// \param[in] key mpls label
/// \param[out] info mpls label to vnic map info
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid mpls label should be passed
sdk_ret_t pds_mpls_label_to_vnic_map_read(pds_mpls_label_to_vnic_map_key_t *key,
                                          pds_mpls_label_to_vnic_map_info_t *info);

/// \brief     update mpls label to vnic map entry
/// \param[in] spec mpls label to vnic map specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid vnic id and mpls label should be passed
sdk_ret_t pds_mpls_label_to_vnic_map_update(pds_mpls_label_to_vnic_map_spec_t *spec);

/// \brief     delete mpls label to vnic map entry
/// \param[in] key mpls label
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid mpls label should be passed
sdk_ret_t pds_mpls_label_to_vnic_map_delete(pds_mpls_label_to_vnic_map_key_t *key);

/// @}

#ifdef __cplusplus
}
#endif

#endif  // __PDS_VNIC_H__
