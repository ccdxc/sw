//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// lif datapath implementation
///
//----------------------------------------------------------------------------

#ifndef __LIF_IMPL_HPP__
#define __LIF_IMPL_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/sdk/include/sdk/qos.hpp"
#include "nic/sdk/platform/devapi/devapi_types.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_lif.hpp"

#define SDK_MAX_NAME_LEN            16

namespace api {
namespace impl {

// forward declaration
class lif_impl_state;

/// \defgroup PDS_LIF_IMPL - lif entry datapath implementation
/// \ingroup PDS_LIF
/// \@{
/// \brief LIF implementation

class lif_impl : public impl_base {
public:
    /// \brief  factory method to allocate and initialize a lif entry
    /// \param[in] spec    lif configuration parameters
    /// \return    new instance of lif or NULL, in case of error
    static lif_impl *factory(pds_lif_spec_t *spec);

    /// \brief  release all the s/w state associate with the given lif,
    ///         if any, and free the memory
    /// \param[in] impl    lif impl instance to be freed
    ///                  NOTE: h/w entries should have been cleaned up (by
    ///                  calling impl->cleanup_hw() before calling this
    static void destroy(lif_impl *impl);

    /// \brief     helper function to get key given lif entry
    /// \param[in] entry    pointer to lif instance
    /// \return    pointer to the lif instance's key
    static void *lif_key_func_get(void *entry) {
        lif_impl *lif = (lif_impl *)entry;
        return (void *)&(lif->key_);
    }

    /// \brief     helper function to get (internal) lif idgiven lif entry
    /// \param[in] entry    pointer to lif instance
    /// \return    pointer to the lif instance's key
    static void *lif_id_func_get(void *entry) {
        lif_impl *lif = (lif_impl *)entry;
        return (void *)&(lif->id_);
    }

    /// \brief    handle all programming during lif creation
    ///< \param[in] spec    lif configuration parameters
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t create(pds_lif_spec_t *spec);

    ///< \brief    program lif tx policer for given lif
    ///< param[in] lif_id     h/w lif id
    ///< param[in] policer    policer parameters
    /// \return    SDK_RET_OK on success, failure status code on error
    static sdk_ret_t program_tx_policer(uint32_t lif_id,
                                        sdk::policer_t *policer);

    /// \brief     ifindex this lif is pinned to
    /// \return    pinned interface index
    pds_ifindex_t pinned_ifindex(void) const {
        return pinned_if_idx_;
    }

    /// \brief     return type of the lif
    /// \return    lif type
    lif_type_t type(void) const { return type_; }

    /// \brief     return lif key
    /// \return    key of the lif
    const pds_obj_key_t& key(void) const { return key_; }

    /// \brief     return (internal) lif id
    /// \return    id of the lif
    pds_lif_id_t id(void) const { return id_; }

    /// \brief     return encoded ifindex of lif
    /// \return    ifindex of the lif
    pds_ifindex_t ifindex(void) const { return ifindex_; }

    /// \brief     return vnic hw id of this lif
    /// \return    vnic hw id
    uint16_t vnic_hw_id(void) const { return vnic_hw_id_; }

    /// \brief     set/update the name of the lif
    /// \param[in] name    name of the device corresponding to this lif
    void set_name(const char *name) {
        if ((type_ == sdk::platform::LIF_TYPE_HOST_MGMT) ||
            (type_ == sdk::platform::LIF_TYPE_HOST)) {
            memcpy(name_, name, SDK_MAX_NAME_LEN);
        }
    }

    /// \brief     return the name of the lif
    /// \return    return lif name
    const char *name(void) const { return name_; }

    /// \brief     set/update the state of the lif
    void set_state(lif_state_t state) {
        state_ = state;
    }

    /// \brief     return the operational state of the lif
    /// \return    operational state of the lif
    lif_state_t state(void) const { return state_; }

    /// \brief     set/update the mac address of the lif
    /// \param[in] mac    mac address of the device corresponding to this lif
    void set_mac(mac_addr_t mac) {
        memcpy(mac_, mac, ETH_ADDR_LEN);
    }

    /// \brief     return the MAC address corresponding to this lif
    /// \return    ethernet MAC address of this lif
    mac_addr_t& mac(void) { return mac_; }

    /// \brief    return the nexthop index corresponding to this lif
    /// \return   nexthop index of the lif
    uint32_t nh_idx(void) const { return nh_idx_; }

private:
    ///< constructor
    ///< \param[in] spec    lif configuration parameters
    lif_impl(pds_lif_spec_t *spec);

    ///< destructor
    ~lif_impl() {}

    ///< \brief    program necessary h/w entries for oob mnic lif
    ///< \param[in] spec    lif configuration parameters
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t create_oob_mnic_(pds_lif_spec_t *spec);

    ///< \brief    program necessary h/w entries for inband mnic lif(s)
    ///< \param[in] spec    lif configuration parameters
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t create_inb_mnic_(pds_lif_spec_t *spec);

    ///< \brief    program necessary h/w entries for datapath lif(s)
    ///< \param[in] spec    lif configuration parameters
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t create_datapath_mnic_(pds_lif_spec_t *spec);

    ///< \brief    program necessary entries for internal mgmt. mnic lif(s)
    ///< \param[in] spec    lif configuration parameters
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t create_internal_mgmt_mnic_(pds_lif_spec_t *spec);

    ///< \brief    program necessary entries for host (data) lifs
    ///< \param[in] spec    lif configuration parameters
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t create_host_lif_(pds_lif_spec_t *spec);

    ///< \brief    program necessary entries for learn lif(s)
    ///< \param[in] spec    lif configuration parameters
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t create_learn_lif_(pds_lif_spec_t *spec);

private:
    ///< name of the lif
    pds_obj_key_t    key_;            //< lif key
    pds_lif_id_t     id_;             ///< (s/w & h/w) internal lif id
    pds_ifindex_t    pinned_if_idx_;  ///< pinnned if index, if any
    lif_type_t       type_;           ///< type of the lif
    /// name of the lif, if any
    char             name_[SDK_MAX_NAME_LEN];
    mac_addr_t       mac_;            ///< MAC address of lif

    /// operational state
    // TODO: we can have state per pipeline in this class
    //       ideally, we should have the concrete class inside pipeline specific
    //       dir and this should be a base class !!
    pds_ifindex_t    ifindex_;        ///< ifindex of this lif
    uint32_t         nh_idx_;         ///< nexthop idx of this lif
    uint16_t         vnic_hw_id_;     ///< vnic hw id
    lif_state_t      state_;          ///< operational state
    bool             init_done_;      ///< TRUE if lif init is done
    ht_ctxt_t        ht_ctxt_;        ///< hash table context
    ht_ctxt_t        id_ht_ctxt_;     ///< id based hash table context
    ///< lif_impl_state is friend of lif_impl
    friend class lif_impl_state;
} __PACK__;

/// \@}

}    // namespace impl
}    // namespace api

#endif    //    __LIF_IMPL_HPP__
