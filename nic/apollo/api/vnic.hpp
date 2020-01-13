//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vnic entry handling
///
//----------------------------------------------------------------------------

#ifndef __VNIC_HPP__
#define __VNIC_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"

namespace api {

// forward declaration
class vnic_state;

// attribute update bits for vnic object
#define PDS_VNIC_UPD_VNIC_ENCAP          0x1
#define PDS_VNIC_UPD_SWITCH_VNIC         0x2
#define PDS_VNIC_UPD_POLICY              0x4
#define PDS_VNIC_UPD_HOST_IFINDEX        0x8
#define PDS_VNIC_UPD_METER               0x10
// route table is not an attr inside the vnic object, however
// a route table change on subnet can affect vnic's programming
// TODO: revisit and see if there is a better way to handle this
#define PDS_VNIC_UPD_ROUTE_TABLE         0x20

/// \defgroup PDS_VNIC_ENTRY - vnic functionality
/// \ingroup PDS_VNIC
/// @{

/// \brief    vnic entry
class vnic_entry : public api_base {
public:
    /// \brief  factory method to allocate and initialize a vnic entry
    /// \param[in] pds_vnic    vnic information
    /// \return    new instance of vnic or NULL, in case of error
    static vnic_entry *factory(pds_vnic_spec_t *pds_vnic);

    /// \brief  release all the s/w state associate with the given vnic,
    ///         if any, and free the memory
    /// \param[in] vnic  vnic to be freed
    ///                  NOTE: h/w entries should have been cleaned up (by
    ///                  calling impl->cleanup_hw() before calling this
    static void destroy(vnic_entry *vnic);

    /// \brief    clone this object and return cloned object
    /// \param[in]    api_ctxt API context carrying object related configuration
    /// \return       new object instance of current object
    virtual api_base *clone(api_ctxt_t *api_ctxt) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] vnic    vnic enry to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(vnic_entry *vnic);

    /// \brief     allocate h/w resources for this object
    /// \param[in] orig_obj    old version of the unmodified object
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     free h/w resources used by this object, if any
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(void) override;

    /// \brief     initialize vnic entry with the given config
    /// \param[in] api_ctxt API context carrying the configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /// \brief populate the IPC msg with object specific information
    ///        so it can be sent to other components
    /// \param[in] msg         IPC message to be filled in
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t populate_msg(pds_msg_t *msg,
                                   api_obj_ctxt_t *obj_ctxt) override;

    /// \brief    program all h/w tables relevant to this object except stage 0
    ///           table(s), if any
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_create(api_obj_ctxt_t *obj_ctxt) override;

    /// \brief    cleanup all h/w tables relevant to this object except stage 0
    ///           table(s), if any, by updating packed entries with latest
    ///           epoch#
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_config(api_obj_ctxt_t *obj_ctxt) override;

    /// \brief    compute the object diff during update operation compare the
    ///           attributes of the object on which this API is invoked and the
    ///           attrs provided in the update API call passed in the object
    ///           context (as cloned object + api_params) and compute the upd
    ///           bitmap (and stash in the object context for later use)
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t compute_update(api_obj_ctxt_t *obj_ctxt) override;

    /// \brief        add all objects that may be affected if this object is
    ///               updated to framework's object dependency list
    /// \param[in]    obj_ctxt    transient state associated with this API
    ///                           processing
    /// \return       SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_deps(api_obj_ctxt_t *obj_ctxt) override {
        // no other objects are effected if vnic is modified
        // NOTE: assumption is that none of key or immutable fields (e.g., type
        // of vnic, vlan of the vnic etc.) are modifiable and agent will catch
        // such errors
        return SDK_RET_OK;
    }

    /// \brief    update all h/w tables relevant to this object except stage 0
    ///           table(s), if any, by updating packed entries with latest
    ///           epoch#
    /// \param[in] orig_obj    old version of the unmodified object
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_update(api_base *orig_obj,
                                     api_obj_ctxt_t *obj_ctxt) override;

    /// \brief    activate the epoch in the dataplane by programming stage 0
    ///           tables, if any
    /// \param[in] epoch       epoch being activated
    /// \param[in] api_op      api operation
    /// \param[in] orig_obj old/original version of the unmodified object
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      api_base *orig_obj,
                                      api_obj_ctxt_t *obj_ctxt) override;

    /// \brief          reprogram all h/w tables relevant to this object and
    ///                 dependent on other objects except stage 0 table(s),
    ///                 if any
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reprogram_config(api_obj_ctxt_t *obj_ctxt) override;

    /// \brief re-activate config in the hardware stage 0 tables relevant to
    ///        this object, if any, this reactivation must be based on existing
    ///        state and any of the state present in the dirty object list
    ///        (like clone objects etc.) only and not directly on db objects
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    /// NOTE: this method is called when an object is in the dependent/puppet
    ///       object list
    virtual sdk_ret_t reactivate_config(pds_epoch_t epoch,
                                        api_obj_ctxt_t *obj_ctxt) override;

    ///\brief      read config
    ///\param[out] info pointer to the info object
    ///\return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_vnic_info_t *info);

    /// \brief    add given vnic to the database
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_to_db(void) override;

    /// \brief    delete given vnic from the database
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t del_from_db(void) override;

    /// \brief    this method is called on new object that needs to replace the
    ///           old version of the object in the DBs
    /// \param[in] orig_obj    old version of the unmodified object
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_db(api_base *orig_obj,
                                api_obj_ctxt_t *obj_ctxt) override;

    /// \brief    initiate delay deletion of this object
    virtual sdk_ret_t delay_delete(void) override;

    /// \brief    return stringified key of the object (for debugging)
    virtual string key2str(void) const override {
        return "vnic-"  + std::string(key_.str());
    }

    /// \brief     helper function to get key given vnic entry
    /// \param[in] entry    pointer to vnic instance
    /// \return    pointer to the vnic instance's key
    static void *vnic_key_func_get(void *entry) {
        vnic_entry *vnic = (vnic_entry *)entry;
        return (void *)&(vnic->key_);
    }

    /// \brief     return the key/id of this vnic
    /// \return    key/id of the vnic object
    pds_obj_key_t key(void) const { return key_; }

    /// \brief     return impl instance of this vnic object
    /// \return    impl instance of the vnic object
    impl_base *impl(void) { return impl_; }

    /// \brief     return subnet of this vnic
    /// \return    key of the subnet this vnic belongs to
    pds_obj_key_t subnet(void) { return subnet_; }

    /// \brief     return meter policy of this vnic
    /// \param[in] af    IP address family
    /// \return    key of the meter policy being applied
    pds_meter_key_t meter(uint8_t af) {
        if (af == IP_AF_IPV4) {
            return v4_meter_;
        }
        return v6_meter_;
    }

    /// \brief     return vnic encap information of vnic
    /// \return    vnic encap type and value
    pds_encap_t vnic_encap(void) { return vnic_encap_; }

    /// \brief     return fabric encap information of vnic
    /// \return    fabric encap type and value
    pds_encap_t fabric_encap(void) { return fabric_encap_; }

    /// \brief     return true if vnic is switch vnic or else return false
    /// \return    true if vnic is switch vnic, else false
    bool switch_vnic(void) { return switch_vnic_; }

    /// \brief     return the MAC address corresponding to this vnic
    /// \return    ethernet MAC address of this vnic
    mac_addr_t& mac(void) { return mac_; }

    /// \brief     return the associated host lif's ifindex
    /// \return    ifindex of the host lif corresponding to this vnic
    pds_ifindex_t host_ifindex(void) const { return host_ifindex_; }

    /// \brief     return number of IPv4 ingress policies on the vnic
    /// \return    number of IPv4 ingress policies on the vnic
    uint8_t num_ing_v4_policy(void) const {
        return num_ing_v4_policy_;
    }

    /// \brief          return ingress IPv4 security policy of the vnic
    /// \param[in] n    policy number being queried
    /// \return         ingress IPv4 security policy of the vnic
    pds_obj_key_t ing_v4_policy(uint32_t n) const {
        return ing_v4_policy_[n];
    }

    /// \brief     return number of IPv6 ingress policies on the vnic
    /// \return    number of IPv6 ingress policies on the vnic
    uint8_t num_ing_v6_policy(void) const {
        return num_ing_v6_policy_;
    }

    /// \brief          return ingress IPv6 security policy of the vnic
    /// \param[in] n    policy number being queried
    /// \return         ingress IPv6 security policy of the vnic
    pds_obj_key_t ing_v6_policy(uint32_t n) const {
        return ing_v6_policy_[n];
    }

    /// \brief     return number of IPv4 egress policies on the vnic
    /// \return    number of IPv4 egress policies on the vnic
    uint8_t num_egr_v4_policy(void) const {
        return num_egr_v4_policy_;
    }

    /// \brief          return egress IPv4 security policy of the vnic
    /// \param[in] n    policy number being queried
    /// \return         egress IPv4 security policy of the vnic
    pds_obj_key_t egr_v4_policy(uint32_t n) const {
        return egr_v4_policy_[n];
    }

    /// \brief     return number of IPv6 egress policies on the vnic
    /// \return    number of IPv6 egress policies on the vnic
    uint8_t num_egr_v6_policy(void) const {
        return num_egr_v6_policy_;
    }

    /// \brief          return egress IPv6 security policy of the vnic
    /// \param[in] n    policy number being queried
    /// \return         egress IPv6 security policy of the vnic
    pds_obj_key_t egr_v6_policy(uint32_t n) const {
        return egr_v6_policy_[n];
    }

private:
    /// \brief    constructor
    vnic_entry();

    /// \brief    destructor
    ~vnic_entry();

    /// \brief      fill the vnic sw spec
    /// \param[out] spec specification
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t fill_spec_(pds_vnic_spec_t *spec);

    /// \brief    free h/w resources used by this object, if any
    ///           (this API is invoked during object deletes)
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t nuke_resources_(void);

private:
    pds_obj_key_t    key_;              ///< vnic key
    pds_obj_key_t  subnet_;           ///< subnet of this vnic
    pds_meter_key_t   v4_meter_;         ///< IPv4 metering policy
    pds_meter_key_t   v6_meter_;         ///< IPv6 metering policy
    pds_encap_t       vnic_encap_;       ///< vnic's vlan encap
    pds_encap_t       fabric_encap_;     ///< fabric encap information
    bool              switch_vnic_;      ///< TRUE if this is switch vnic
    mac_addr_t        mac_;              ///< MAC address of this vnic
    pds_ifindex_t     host_ifindex_;     ///< PF/VF this vnic is behind
    ///< number of ingress IPv4 policies
    uint8_t           num_ing_v4_policy_;
    ///< ingress IPv4 policies
    pds_obj_key_t  ing_v4_policy_[PDS_MAX_VNIC_POLICY];
    ///< number of ingress IPv6 policies
    uint8_t           num_ing_v6_policy_;
    ///< ingress IPv6 policies
    pds_obj_key_t  ing_v6_policy_[PDS_MAX_VNIC_POLICY];
    ///< number of egress IPv4 policies
    uint8_t           num_egr_v4_policy_;
    ///< egress IPv4 policies
    pds_obj_key_t  egr_v4_policy_[PDS_MAX_VNIC_POLICY];
    ///< number of egress IPv6 policies
    uint8_t           num_egr_v6_policy_;
    ///< egress IPv6 policies
    pds_obj_key_t  egr_v6_policy_[PDS_MAX_VNIC_POLICY];
    ht_ctxt_t         ht_ctxt_;          ///< hash table context
    impl_base         *impl_;            ///< impl object instance

    friend class vnic_state;    ///< vnic_state is friend of vnic_entry
} __PACK__;

/// @}     // end of PDS_VNIC_ENTRY

}    // namespace api

using api::vnic_entry;

#endif    // __VNIC_HPP__
