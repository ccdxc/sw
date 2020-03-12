//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines EPOCH API
///
//----------------------------------------------------------------------------


#ifndef __PDS_EPOCH_H__
#define __PDS_EPOCH_H__

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup PDS_EPOCH
/// @{

/// \brief Maximum epoch index
#define PDS_EPOCH_ID_MAX    (1 * 1024 * 1024)

/// \brief Epoch key
typedef struct pds_epoch_key_s {
    uint16_t    epoch_id;    ///< Epoch id
} __PACK__ pds_epoch_key_t;

/// \brief Epoch data
typedef struct pds_epoch_data_s {
    uint32_t    epoch;    ///< Epoch value
} __PACK__ pds_epoch_data_t;

/// \brief Epoch specification
typedef struct pds_epoch_spec_s {
    pds_epoch_key_t     key;     ///< Epoch key
    pds_epoch_data_t    data;    ///< Epoch data
} __PACK__ pds_epoch_spec_t;

/// \brief Epoch status
typedef struct pds_epoch_status_s {
} __PACK__ pds_epoch_status_t;

/// \brief Epoch info
typedef struct pds_epoch_info_s {
    pds_epoch_spec_t      spec;      ///< Specification
    pds_epoch_status_t    status;    ///< Status
} __PACK__ pds_epoch_info_t;


/// \brief     create epoch entry
/// \param[in] spec epoch specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid epoch value and index should be passed
sdk_ret_t pds_epoch_create(pds_epoch_spec_t *spec);

/// \brief     read epoch entry
/// \param[in] key epoch key
/// \param[out] info epoch info
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid epoch index should be passed
sdk_ret_t pds_epoch_read(pds_epoch_key_t *key, pds_epoch_info_t *info);

/// \brief     update epoch entry
/// \param[in] spec epoch specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid epoch value and index should be passed
sdk_ret_t pds_epoch_update(pds_epoch_spec_t *spec);

/// \brief     delete epoch entry
/// \param[in] key epoch key
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid epoch index should be passed
sdk_ret_t pds_epoch_delete(pds_epoch_key_t *key);

/// @}

#ifdef __cplusplus
}
#endif

#endif  // __PDS_EPOCH_H__
