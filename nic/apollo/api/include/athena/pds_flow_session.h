//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines FLOW SESSION API
///
//----------------------------------------------------------------------------

#ifndef __PDS_FLOW_SESSION_H__
#define __PDS_FLOW_SESSION_H__

#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup PDS_FLOW_SESSION
/// @{

// Maximum session index
#define PDS_FLOW_SESSION_INFO_ID_MAX    (4 * 1024 * 1024)

// Flow direction bitmask
#define HOST_TO_SWITCH    0x1    ///< Host to Switch
#define SWITCH_TO_HOST    0x2    ///< Switch to Host

/// \brief Packet rewrite type
typedef enum pds_user_packet_rewrite_type_e {
    REWRITE_TYPE_NONE = 0,    ///< None
    REWRITE_TYPE_SNAT = 1,    ///< Source NAT
    REWRITE_TYPE_DNAT = 2,    ///< Destination NAT
} pds_user_packet_rewrite_type_t;

/// \brief L2 encapsulation
typedef struct pds_flow_session_l2_encap_s {
    uint8_t     smac[ETH_ADDR_LEN];    ///< Source MAC
    uint8_t     dmac[ETH_ADDR_LEN];    ///< Destination MAC
    uint8_t     insert_vlan_tag;       ///< Insert VLAN tag
    uint16_t    vlan_id;               ///< VLAN id
} __PACK__ pds_flow_session_l2_encap_t;

/// \brief IP encapsulation
typedef struct pds_flow_session_ip_encap_s {
    uint32_t    ip_saddr;    ///< IP src address
    uint32_t    ip_daddr;    ///< IP dest address
} __PACK__ pds_flow_session_ip_encap_t;

/// \brief UDP encapsulation
typedef struct pds_flow_session_udp_encap_s {
    uint16_t    udp_sport;    ///< source port
    uint16_t    udp_dport;    ///< destination port
} __PACK__ pds_flow_session_udp_encap_t;

/// \brief MPLS over UDP encapsulation
typedef struct pds_flow_session_mplsoudp_encap_s {
    pds_flow_session_l2_encap_t     l2_encap;       ///< L2 encap
    pds_flow_session_ip_encap_t     ip_encap;       ///< IP encap
    pds_flow_session_udp_encap_t    udp_encap;      ///< UDP encap
    uint32_t                        mpls1_label;    ///< MPLS label 1
    uint32_t                        mpls2_label;    ///< MPLS label 2
    uint32_t                        mpls3_label;    ///< MPLS label 3
} __PACK__ pds_flow_session_mplsoudp_encap_t;

/// \brief MPLS over GRE encapsulation
typedef struct pds_flow_session_mplsogre_encap_s {
    pds_flow_session_l2_encap_t    l2_encap;       ///< L2 encap
    pds_flow_session_ip_encap_t    ip_encap;       ///< IP encap
    uint32_t                       mpls1_label;    ///< MPLS label 1
    uint32_t                       mpls2_label;    ///< MPLS label 2
    uint32_t                       mpls3_label;    ///< MPLS label 3
} __PACK__ pds_flow_session_mplsogre_encap_t;

/// \brief Encapsulation type
typedef enum pds_flow_session_encap_e {
    ENCAP_TYPE_NONE = 0,    ///< No encap
    ENCAP_TYPE_L2,          ///< L2 encap
    ENCAP_TYPE_MPLSOUDP,    ///< MPLS over UDP encap
    ENCAP_TYPE_MPLSOGRE,    ///< MPLS over GRE encap
    ENCAP_TYPE_GENEVE,      ///< Geneve encap
} pds_flow_session_encap_t;

typedef enum pds_egress_action_e {
    EGRESS_ACTION_NONE = 0,        ///< Invalid
    EGRESS_ACTION_TX_TO_HOST,      ///< Forward to Host
    EGRESS_ACTION_TX_TO_SWITCH,    ///< Forward to Switch
    EGRESS_ACTION_DROP,            ///< Drop the packet
    EGRESS_ACTION_DROP_BY_SL,      ///< Drop the packet due to security list rule
    EGRESS_ACTION_THROTTLE,        ///< Throttle the flow
} pds_egress_action_t;

/// \brief Packet rewrite data
typedef struct pds_flow_session_rewrite_info_s {
    /// \brief Corresponds to L2 header for switch-to-host direction or
    /// substrate header for host-to-switch direction
    uint8_t                                 strip_encap_header;
    ///< Strip L2 header
    uint8_t                                 strip_l2_header;
    ///< Strip vlan header
    uint8_t                                 strip_vlan_tag;
    ///< Packet rewrite type
    pds_user_packet_rewrite_type_t          user_packet_rewrite_type;
    ///< Packet rewrite IP, could be v4 or v6
    uint8_t                                 user_packet_rewrite_ip[IP6_ADDR8_LEN];
    ///< Encapsulation type
    pds_flow_session_encap_t                encap_type;
    ///< Encapsulation data
    union {
        ///< L2 encap
        pds_flow_session_l2_encap_t         l2_encap;
        ///< MPLS over UDP
        pds_flow_session_mplsoudp_encap_t   mplsoudp_encap;
        ///< MPLS over GRE
        pds_flow_session_mplsogre_encap_t   mplsogre_encap;
    } u;
} __PACK__ pds_flow_session_rewrite_info_t;

/// \brief Session data
typedef struct pds_flow_session_flow_data_s {
    ///< Session entry valid bit
    uint8_t                         session_info_valid;
    ///< Epoch1 value
    uint16_t                        epoch1;
    ///< Epoch1 id
    uint32_t                        epoch1_id;
    ///< Epoch2 value
    uint16_t                        epoch2;
    ///< Epoch2 id
    uint32_t                        epoch2_id;
    ///< Bitmask of allowed flow states
    uint16_t                        allowed_flow_state_bitmask;
    ///< Action to be taken on egress
    pds_egress_action_t             egress_action;
    ///< Policer pps id
    uint16_t                        policer_pps_id;
    ///< Policer bw id
    uint16_t                        policer_bw_id;
    ///< Statistics id
    uint16_t                        statistics_id;
    ///< Statistics mask
    uint16_t                        statistics_mask;
    ///< Histogram id
    uint16_t                        histogram_id;
    ///< Rewrite info
    pds_flow_session_rewrite_info_t rewrite_info;
} __PACK__ pds_flow_session_flow_data_t;

/// \brief Session specification
typedef struct pds_flow_session_data_s {
    uint32_t                        conntrack_id;                ///< Connection track id 
    pds_flow_session_flow_data_t    host_to_switch_flow_info;    ///< Host to Switch data
    pds_flow_session_flow_data_t    switch_to_host_flow_info;    ///< Switch to Host data
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

/// \brief     update flow session common info entry
/// \param[in] spec flow session specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid session info ID(key) and data should be passed
sdk_ret_t pds_flow_session_common_info_update(pds_flow_session_spec_t *spec);

/// \brief     update flow session rewrite info entry
/// \param[in] spec flow session specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid session info ID(key) and data should be passed
sdk_ret_t pds_flow_session_rewrite_info_update(pds_flow_session_spec_t *spec);

/// \brief     delete flow session info entry
/// \param[in] key flow session key
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid session info ID(key) should be passed
sdk_ret_t pds_flow_session_info_delete(pds_flow_session_key_t *key);

/// @}

#ifdef __cplusplus
}
#endif

#endif // __PDS_FLOW_SESSION_H__
