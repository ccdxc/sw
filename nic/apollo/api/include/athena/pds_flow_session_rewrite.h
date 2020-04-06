//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines FLOW SESSION REWRITE API
///
//----------------------------------------------------------------------------

#ifndef __PDS_FLOW_SESSION_REWRITE_H__
#define __PDS_FLOW_SESSION_REWRITE_H__

#include "pds_base.h"
#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup PDS_FLOW_SESSION_REWRITE
/// @{

// Maximum session rewrite index
#define PDS_FLOW_SESSION_REWRITE_ID_MAX    (2 * 1024 * 1024)

/// \brief Rewrite NAT type
typedef enum pds_flow_session_rewrite_nat_type_e {
    REWRITE_NAT_TYPE_NONE = 0,      ///< None
    REWRITE_NAT_TYPE_IPV4_SNAT,     ///< Source IPV4 NAT
    REWRITE_NAT_TYPE_IPV4_DNAT,     ///< Destination IPV4 NAT
    REWRITE_NAT_TYPE_IPV4_SPAT,     ///< Source Port NAT
    REWRITE_NAT_TYPE_IPV4_DPAT,     ///< Destination Port NAT
    REWRITE_NAT_TYPE_IPV4_SDPAT,    ///< Source/Destination Port NAT
    REWRITE_NAT_TYPE_IPV6_SNAT,     ///< Source IPV6 NAT
    REWRITE_NAT_TYPE_IPV6_DNAT,     ///< Destination IPV6 NAT
    REWRITE_NAT_TYPE_MAX
} pds_flow_session_rewrite_nat_type_t;

/// \brief Encapsulation type
typedef enum pds_flow_session_encap_e {
    ENCAP_TYPE_NONE = 0,    ///< No encap
    ENCAP_TYPE_L2,          ///< L2 encap
    ENCAP_TYPE_MPLSOUDP,    ///< MPLS over UDP encap
    ENCAP_TYPE_MPLSOGRE,    ///< MPLS over GRE encap
    ENCAP_TYPE_GENEVE,      ///< Geneve encap
    ENCAP_TYPE_MAX
} pds_flow_session_encap_t;

/// \brief Rewrite PAT information
typedef struct pds_flow_session_rewrite_pat_info_s {
    uint32_t            ip_saddr;
    uint32_t            ip_daddr;
    uint16_t            l4_sport;
    uint16_t            l4_dport;
} __PACK__ pds_flow_session_rewrite_pat_info_t;

/// \brief Rewrite NAT informatiom
typedef struct pds_flow_session_rewrite_nat_info_s {
    pds_flow_session_rewrite_nat_type_t        nat_type;
    union {
        uint8_t                                ipv6_addr[IP6_ADDR8_LEN];
        uint32_t                               ipv4_addr;
        pds_flow_session_rewrite_pat_info_t    pat;
    } u;
} __PACK__ pds_flow_session_rewrite_nat_info_t;

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

/// \brief Geneve encapsulation
typedef struct pds_flow_session_geneve_encap_s {
    ///< L2 encap
    pds_flow_session_l2_encap_t     l2_encap;
    ///< IP encap
    pds_flow_session_ip_encap_t     ip_encap;
    ///< UDP encap
    pds_flow_session_udp_encap_t    udp_encap;
    ///< VNI
    uint32_t                        vni;
    ///< Source slot id
    uint32_t                        source_slot_id;
    ///< Destination slot id
    uint32_t                        destination_slot_id;
    ///< SG1 id
    uint16_t                        sg_id1;
    ///< SG2 id
    uint16_t                        sg_id2;
    ///< SG3 id
    uint16_t                        sg_id3;
    ///< SG4 id
    uint16_t                        sg_id4;
    ///< SG5 id
    uint16_t                        sg_id5;
    ///< SG6 id
    uint16_t                        sg_id6;
    ///< Originator physical IP
    uint32_t                        originator_physical_ip;
} __PACK__ pds_flow_session_geneve_encap_t;

/// \brief Packet rewrite data
typedef struct pds_flow_session_rewrite_data_s {
    /// \brief Corresponds to L2 header for switch-to-host direction or
    /// substrate header for host-to-switch direction
    uint8_t                                 strip_encap_header;
    ///< Strip L2 header
    uint8_t                                 strip_l2_header;
    ///< Strip vlan header
    uint8_t                                 strip_vlan_tag;
    ///< NAT rewrite info
    pds_flow_session_rewrite_nat_info_t     nat_info;
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
        ///< Geneve encap
        pds_flow_session_geneve_encap_t     geneve_encap;
    } u;
} __PACK__ pds_flow_session_rewrite_data_t;

/// \brief Session key
typedef struct pds_flow_session_rewrite_key_s {
    uint32_t    session_rewrite_id;    ///< Session rewrite id
} __PACK__ pds_flow_session_rewrite_key_t;

/// \brief Session specification
typedef struct pds_flow_session_rewrite_spec_s {
    pds_flow_session_rewrite_key_t     key;     ///< Session rewrite key
    pds_flow_session_rewrite_data_t    data;    ///< Session rewrite data
} __PACK__ pds_flow_session_rewrite_spec_t;

/// \brief Session status
typedef struct pds_flow_session_rewrite_status_s {
} __PACK__ pds_flow_session_rewrite_status_t;

/// \brief Session info
typedef struct pds_flow_session_rewrite_info_s {
    pds_flow_session_rewrite_spec_t      spec;      ///< Specification
    pds_flow_session_rewrite_status_t    status;    ///< Status
} __PACK__ pds_flow_session_rewrite_info_t;

/// \brief     create flow session rewrite info entry
/// \param[in] spec flow session rewrite specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid session rewrite info ID(key) and data should be passed
pds_ret_t pds_flow_session_rewrite_create(pds_flow_session_rewrite_spec_t *spec);

/// \brief      read a flow session rewrite info entry
/// \param[in]  key flow session rewrite key
/// \param[out] info flow session rewrite info
/// \return     #SDK_RET_OK on success, failure status code on error
/// \remark     A valid session rewrite info ID(key) should be passed
pds_ret_t pds_flow_session_rewrite_read(pds_flow_session_rewrite_key_t *key,
                                        pds_flow_session_rewrite_info_t *info);

/// \brief     update flow session rewrite info entry
/// \param[in] spec flow session rewrite specification
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid session rewrite info ID(key) and data should be passed
pds_ret_t pds_flow_session_rewrite_update(pds_flow_session_rewrite_spec_t *spec);

/// \brief     delete flow session rewrite info entry
/// \param[in] key flow session rewrite key
/// \return    #SDK_RET_OK on success, failure status code on error
/// \remark    A valid session rewrite info ID(key) should be passed
pds_ret_t pds_flow_session_rewrite_delete(pds_flow_session_rewrite_key_t *key);

/// @}

#ifdef __cplusplus
}
#endif

#endif // __PDS_FLOW_SESSION_REWRITE_H__
