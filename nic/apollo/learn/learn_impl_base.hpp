//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// pipeline specific abstractions needed by learn module
///
//----------------------------------------------------------------------------

#ifndef __LEARN_LEARN_IMPL_HPP__
#define __LEARN_LEARN_IMPL_HPP__

#include "nic/sdk/include/sdk/types.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/learn/learn.hpp"

namespace learn {
namespace impl {

/// \brief learn information parsed from p4 hdr/packet
typedef struct learn_info_s {
    uint16_t lif;                       ///< ingress lif
    pds_obj_key_t subnet;               ///< subnet key
    pds_encap_t encap;                  ///< vnic encap
    uint8_t l2_offset;                  ///< offset to l2 header
    uint8_t l3_offset;                  ///< offset to l3 header
    pkt_type_t pkt_type;                ///< protocol type
    uint32_t hints;                     ///< hint flags for future use
} learn_info_t;

/// \brief nexthop type for pkts sent out from learn
enum {
    LEARN_NH_TYPE_NONE = 0,             ///< send pkt back to p4 ingress
    LEARN_NH_TYPE_DATAPATH_MNIC,        ///< send pkt to datapath mnic (VPP)
    LEARN_NH_TYPE_VNIC,                 ///< send pkt to host vnic
};

/// \brief information needed to construct arm to p4 tx hdr
typedef struct p4_tx_info_s {
    pds_lif_id_t slif;                  ///< source lif
    uint8_t nh_type;                    ///< nexthop type
    pds_obj_key_t vnic_key;             ///< valid when nh_type is VNIC
} p4_tx_info_t;

/// \brief          return arm to p4 packet rx header size
/// \return         arm to p4 header length
uint16_t arm_to_p4_hdr_sz(void);

/// \brief          return p4 to arm packet tx header size
/// \return         p4 to arm header length
uint16_t p4_to_arm_hdr_sz(void);

/// \brief          get learn lif name
/// \return         learn lif name C string
const char *learn_lif_name(void);

/// \brief          parse pipeline specific rx hdr for learn info
/// \param[in]      pkt     pointer to packet's p4 rx header
/// \param[out]     info    pointer to learn info structure
/// \return         SDK_RET_OK on success, SDK_RET_ERR on failure to parse
sdk_ret_t extract_info_from_p4_hdr(char *pkt, learn_info_t *info);

/// \brief          fill cpu to p4 tx header with information from rx header
/// \param[in]      tx_hdr  pointer to start of p4 tx header of the packet
/// \param[in]      tx_info pointer destination for the packet
/// \return         None
void arm_to_p4_tx_hdr_fill(char *tx_hdr, p4_tx_info_t *tx_info);

/// \brief          lookup for the presence of a remote endpoint
/// \param[in]      key     mapping key
/// \return         SDK_RET_OK if remote endpoint exists,
///                 SDK_RET_ENTRY_NOT_FOUND if remote end point does noe exist
///                 error code otherwise
sdk_ret_t remote_mapping_find(pds_mapping_key_t *key);

}    // namespace learn
}    // namespace impl

#endif    // __LEARN_LEARN_IMPL_HPP__
