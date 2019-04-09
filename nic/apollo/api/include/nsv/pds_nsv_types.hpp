//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines NVME Storage virtualization types
///
//----------------------------------------------------------------------------

#ifndef __INCLUDE_API_PDS_NVS_TYPES_HPP__
#define __INCLUDE_API_PDS_NVS_TYPES_HPP__

/// \defgroup PDS_NSV_TYPES Object ID and types 
/// @{

/// \brief Basic NSV Types
typedef uint8_t pds_nsv_cntrlr_id_t;
typedef uint16_t pds_nsv_ns_id_t;
typedef uint16_t pds_nsv_session_id_t;
typedef uint8_t pds_nsv_subsys_id_t;
typedef uint16_t pds_nsv_tgt_id_t;

/// \brief NVME Qualified Name
#define PDS_NVS_NQN_LEN (224)
typedef struct pds_nsv_nqn_s {
    uint8_t nqn[PDS_NVS_NQN_LEN];
} __PACK__ pds_nsv_nqn_t;

/// \brief NVME Host ID
#define PDS_NVME_HOST_ID_LEN (16)
typedef struct pds_nsv_hostid_s {
    uint8_t id[PDS_NVME_HOST_ID_LEN];
} __PACK__ pds_nsv_hostid_t;

/// \biref Namespace Size 
typedef uint64_t pds_nvs_nsze_t;

/// \brief Namespace ID
typedef uint32_t pds_nsv_nsid_t;

/// \brief Crypto key for Enctypt/Decrypt namespace
typedef struct pds_nsv_crypto_key_s {
    void *data_key;       ///< Key used to encrypt data
    void *iv_key;         ///< Key used to encrypt iv
    uint16_t data_key_sz; ///< Data key len
    uint16_t iv_key_sz;   ///< IV key len
} __PACK__ pds_nsv_crypto_key_t;

/// @}

#endif    // __INCLUDE_API_PDS_NVS_TYPES_HPP__
