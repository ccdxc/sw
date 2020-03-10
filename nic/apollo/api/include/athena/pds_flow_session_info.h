//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines FLOW SESSION INFO API
///
//----------------------------------------------------------------------------

#ifndef __PDS_FLOW_SESSION_INFO_H__
#define __PDS_FLOW_SESSION_INFO_H__

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup PDS_FLOW_SESSION_INFO
/// @{

// Maximum session index
#define PDS_FLOW_SESSION_INFO_ID_MAX    (4 * 1024 * 1024)

// Statistics mask length
//#define PDS_FLOW_STATS_MASK_LEN         (104 / 8)    ///< 104 Counters
#define PDS_FLOW_STATS_MASK_LEN         (32 / 8)    ///< truncated bitmap for now

// Flow direction bitmask
#define HOST_TO_SWITCH    0x1    ///< Host to Switch
#define SWITCH_TO_HOST    0x2    ///< Switch to Host

typedef enum pds_egress_action_e {
    EGRESS_ACTION_NONE = 0,        ///< Invalid
    EGRESS_ACTION_TX_TO_HOST,      ///< Forward to Host
    EGRESS_ACTION_TX_TO_SWITCH,    ///< Forward to Switch
    EGRESS_ACTION_DROP,            ///< Drop the packet
    EGRESS_ACTION_DROP_BY_SL,      ///< Drop the packet due to security list rule
    EGRESS_ACTION_THROTTLE,        ///< Throttle the flow
} pds_egress_action_t;

/// \brief Session data
typedef struct pds_flow_session_flow_data_s {
    ///< Epoch vnic value
    uint16_t                        epoch_vnic;
    ///< Epoch vnic id
    uint32_t                        epoch_vnic_id;
    ///< Epoch mapping
    uint16_t                        epoch_mapping;
    ///< Epoch mapping id
    uint32_t                        epoch_mapping_id;
    ///< Policer bw1 id
    uint16_t                        policer_bw1_id;
    ///< Policer bw2 id
    uint16_t                        policer_bw2_id;
    ///< VNIC statistics id
    uint16_t                        vnic_stats_id;
    ///< VNIC statistics mask
    uint8_t                         vnic_stats_mask[PDS_FLOW_STATS_MASK_LEN];
    ///< VNIC histogram latency id
    uint16_t                        vnic_histogram_latency_id;
    ///< VNIC histogram packet length id
    uint16_t                        vnic_histogram_packet_len_id;
    ///< TCP Flags bitmap
    uint8_t                         tcp_flags_bitmap;
    ///< Session rewrite id
    uint32_t                        rewrite_id;
    ///< Bitmask of allowed flow states
    uint16_t                        allowed_flow_state_bitmask;
    ///< Action to be taken on egress
    pds_egress_action_t             egress_action;
} __PACK__ pds_flow_session_flow_data_t;

/// \brief Session specification
typedef struct pds_flow_session_data_s {
    ///< Connection track id
    uint32_t                        conntrack_id;
    ///< Flag to skip flow log
    uint8_t                         skip_flow_log;
    ///< Host MAC validation support
    uint8_t                         host_mac[ETH_ADDR_LEN];
    ///< Host to Switch data
    pds_flow_session_flow_data_t    host_to_switch_flow_info;
    ///< Switch to Host data
    pds_flow_session_flow_data_t    switch_to_host_flow_info;
} __PACK__ pds_flow_session_data_t;

/// \brief Session key
typedef struct pds_flow_session_key_s {
    uint8_t     direction;          ///< Direction
    uint32_t    session_info_id;    ///< Session info id
} __PACK__ pds_flow_session_key_t;

/// \brief Session specification
typedef struct pds_flow_session_spec_s {
    pds_flow_session_key_t     key;             ///< Session key
    pds_flow_session_data_t    data;            ///< Session data
} __PACK__ pds_flow_session_spec_t;

/// \brief Session status
typedef struct pds_flow_session_status_s {
} __PACK__ pds_flow_session_status_t;

/// \brief Session info
typedef struct pds_flow_session_info_s {
    pds_flow_session_spec_t      spec;      ///< Specification
    pds_flow_session_status_t    status;    ///< Status
} __PACK__ pds_flow_session_info_t;

/// \brief     create flow session info entry
/// \param[in] spec flow session specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid session info ID(key) and data should be passed
sdk_ret_t pds_flow_session_info_create(pds_flow_session_spec_t *spec);

/// \brief      read a flow session info entry
/// \param[in]  key flow session key
/// \param[out] info flow session info
/// \return     #SDK_RET_OK on success, failure status code on error
/// \remark     A valid session info ID(key) should be passed
sdk_ret_t pds_flow_session_info_read(pds_flow_session_key_t *key,
                                     pds_flow_session_info_t *info);

/// \brief     update flow session info entry
/// \param[in] spec flow session specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid session info ID(key) and data should be passed
sdk_ret_t pds_flow_session_info_update(pds_flow_session_spec_t *spec);

/// \brief     delete flow session info entry
/// \param[in] key flow session key
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid session info ID(key) should be passed
sdk_ret_t pds_flow_session_info_delete(pds_flow_session_key_t *key);

/// @}

#ifdef __cplusplus
}
#endif

#endif // __PDS_FLOW_SESSION_INFO_H__
