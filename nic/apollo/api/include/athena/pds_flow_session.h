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
    uint16_t    vlan_id;               ///< VLAN id
} __PACK__ pds_flow_session_l2_encap_t;

/// \brief IP encapsulation
typedef struct pds_flow_session_ip_encap_s {
    uint8_t     ip_ttl;      ///< IP time to live
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
} __PACK__ pds_flow_session_mplsoudp_encap_t;

/// \brief MPLS over GRE encapsulation
typedef struct pds_flow_session_mplsogre_encap_s {
    pds_flow_session_l2_encap_t    l2_encap;       ///< L2 encap
    pds_flow_session_ip_encap_t    ip_encap;       ///< IP encap
    uint32_t                       mpls1_label;    ///< MPLS label 1
    uint32_t                       mpls2_label;    ///< MPLS label 2
} __PACK__ pds_flow_session_mplsogre_encap_t;

/// \brief Encapsulation type
typedef enum pds_encap_type_e {
    ENCAP_TYPE_NONE = 0,    ///< No encap
    ENCAP_TYPE_L2,          ///< L2 encap
    ENCAP_TYPE_MPLSOUDP,    ///< MPLS over UDP encap
    ENCAP_TYPE_MPLSOGRE,    ///< MPLS over GRE encap
} pds_encap_type_t;

/// \brief Packet rewrite data
typedef struct pds_flow_session_rewrite_info_s {
    /// \brief Corresponds to L2 header for switch-to-host direction or
    /// substrate header for host-to-switch direction
    uint8_t                                 pop_outer_header;
    ///< Packet rewrite type
    pds_user_packet_rewrite_type_t          user_packet_rewrite_type;
    ///< Packet rewrite IP
    uint8_t                                 user_packet_rewrite_ip[INET6_ADDRSTRLEN];
    ///< Encapsulation type
    pds_encap_type_t                        encap_type;
    ///< Encapsulation data
    union {
        pds_flow_session_l2_encap_t         l2_encap;        ///< L2 encap
        pds_flow_session_mplsoudp_encap_t   mplsoudp_encap;  ///< MPLS over UDP
        pds_flow_session_mplsogre_encap_t   mplsogre_encap;  ///< MPLS over GRE
    } u;
} __PACK__ pds_flow_session_rewrite_info_t;

/// \brief Session data
typedef struct pds_flow_session_data_s {
    uint32_t                        epoch1;                  ///< Epoch1 val
    uint16_t                        epoch1_id;               ///< Epoch1 id
    uint32_t                        epoch2;                  ///< Epoch2 val
    uint16_t                        epoch2_id;               ///< Epoch2 id
    uint32_t                        config_substrate_src_ip; ///< IP substrate
    uint16_t                        policer_pps_id;          ///< Policer PPS id
    uint16_t                        policer_bw_id;           ///< Policer BW id
    uint32_t                        counter_set1_id;         ///< Counterset1 id
    uint32_t                        counter_set2_id;         ///< Counterset2 id
    uint32_t                        histogram_id;            ///< Histogram id
    pds_flow_session_rewrite_info_t rewrite_info;            ///< Rewrite data
} __PACK__ pds_flow_session_data_t;

/// \brief Session key
typedef struct pds_flow_session_key_s {
    uint32_t session_info_id;    ///< Session info id
} __PACK__ pds_flow_session_key_t;

/// \brief Session specification
typedef struct pds_flow_session_spec_s {
    pds_flow_session_key_t key;    ///< Session key
    pds_flow_session_data_t data;  ///< Session data
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

#endif // __PDS_FLOW_SESSION_H__
