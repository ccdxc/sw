/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    utils.hpp
 *
 * @brief   helper functions
 */

#if !defined (__UTILS_HPP__)
#define __UTILS_HPP__

#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/tep.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

/**
 * @brief    given two epochs, pick invalid one or the lowest numbered epoch so
 *           we can overwrite its contents, and return its index
 * @param[in] epoch1    epoch 1 value
 * @param[in] epoch2    epoch 2 value
 * @return index of lowered numbered epoch or 1st invalid one
 */
// TODO: we will handle the case when epoch rollsover later !!
static inline uint8_t
pick_older_epoch_idx (uint32_t epoch1, uint32_t epoch2)
{
    if (epoch1 == PDS_EPOCH_INVALID) {
        return 0;
    } else if ((epoch2 == PDS_EPOCH_INVALID) || (epoch2 < epoch1)) {
        return 1;
    }
    return 0;
}

/// \brief check if given two encaps are same
///
/// \param[in] encap1    encap1 to compare
/// \param[in] encap2    encap2 to compare
/// \returns TRUE if value of encap1 & encap2 are equal, else FALSE
static inline bool
pdsencap_isequal (const pds_encap_t *encap1, const pds_encap_t *encap2)
{
    // compare encap type
    if (encap1->type != encap2->type) {
        return FALSE;
    }
    // compare encap value
    switch (encap1->type) {
    case PDS_ENCAP_TYPE_DOT1Q:
        if (encap1->val.vlan_tag != encap2->val.vlan_tag) {
            return FALSE;
        }
        break;
    case PDS_ENCAP_TYPE_QINQ:
        if ((encap1->val.qinq_tag.c_tag != encap2->val.qinq_tag.c_tag) ||
            (encap1->val.qinq_tag.s_tag != encap2->val.qinq_tag.s_tag)) {
            return FALSE;
        }
        break;
    case PDS_ENCAP_TYPE_MPLSoUDP:
        if (encap1->val.mpls_tag != encap2->val.mpls_tag) {
            return FALSE;
        }
        break;
    case PDS_ENCAP_TYPE_VXLAN:
        if (encap1->val.vnid != encap2->val.vnid) {
            return FALSE;
        }
        break;
    default:
        if (encap1->val.value != encap2->val.value) {
            return FALSE;
        }
        break;
    }
    return TRUE;
}

}    // namespace api

#endif    /** __UTILS_HPP__ */

