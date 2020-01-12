//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file handles subnet entry handling
///
//----------------------------------------------------------------------------

#ifndef __API_SUBNET_HPP__
#define __API_SUBNET_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/framework/impl_base.hpp"

namespace api {

// forward declaration
class subnet_state;

// attribute update bits for subnet object
#define PDS_SUBNET_UPD_ROUTE_TABLE         0x1
#define PDS_SUBNET_UPD_POLICY              0x2
#define PDS_SUBNET_UPD_HOST_IFINDEX        0x4

/// \defgroup PDS_SUBNET_ENTRY - subnet entry functionality
/// \ingroup PDS_SUBNET
/// @{

/// \brief    subnet entry
class subnet_entry : public api_base {
public:
    /// \brief          factory method to allocate and initialize a subnet entry
    /// \param[in]      pds_subnet    subnet information
    /// \return         new instance of subnet or NULL, in case of error
    static subnet_entry *factory(pds_subnet_spec_t *pds_subnet);

    /// \brief          release all the s/w state associate with the given
    ///                 subnet, if any, and free the memory
    /// \param[in]      subnet     subnet to be freed
    /// \NOTE: h/w entries should have been cleaned up (by calling
    ///        impl->cleanup_hw() before calling this
    static void destroy(subnet_entry *subnet);

    /// \brief    clone this object and return cloned object
    /// \param[in]    api_ctxt API context carrying object related configuration
    /// \return       new object instance of current object
    virtual api_base *clone(api_ctxt_t *api_ctxt) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] subnet    subnet to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(subnet_entry *subnet);

    /// \brief          allocate h/w resources for this object
    /// \param[in]      orig_obj    old version of the unmodified object
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        api_obj_ctxt_t *obj_ctxt) override;

    /// \brief          free h/w resources used by this object, if any
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(void) override;

    /// \brief          initialize subnet entry with the given config
    /// \param[in]      api_ctxt API context carrying the configuration
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /// \brief populate the IPC msg with object specific information
    ///        so it can be sent to other components
    /// \param[in] msg         IPC message to be filled in
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t populate_msg(pds_msg_t *msg,
                                   api_obj_ctxt_t *obj_ctxt) override;

    /// \brief          program all h/w tables relevant to this object except
    ///                 stage 0 table(s), if any
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_create(api_obj_ctxt_t *obj_ctxt) override;

    /// \brief          cleanup all h/w tables relevant to this object except
    ///                 stage 0 table(s), if any, by updating packed entries
    ///                 with latest epoch#
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_config(api_obj_ctxt_t *obj_ctxt) override;

    /// \brief    compute the object diff during update operation compare the
    ///           attributes of the object on which this API is invoked and the
    ///           attrs provided in the update API call passed in the object
    ///           context (as cloned object + api_params) and compute the upd
    ///           bitmap (and stash in the object context for later use)
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t compute_update(api_obj_ctxt_t *obj_ctxt) override;

    /// \brief          add all objects that may be affected if this object is
    ///                 updated to framework's object dependency list
    /// \param[in]      obj_ctxt    transient state associated with this API
    ///                             processing
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_deps(api_obj_ctxt_t *obj_ctxt) override;

    /// \brief          update all h/w tables relevant to this object except
    ///                 stage 0 table(s), if any, by updating packed entries
    ///                 with latest epoch#
    /// \param[in]      orig_obj    old version of the unmodified object
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_update(api_base *orig_obj,
                                     api_obj_ctxt_t *obj_ctxt) override;

    /// \param[in]      epoch       epoch being activated
    /// \param[in]      api_op      api operation
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \param[in]      orig_obj old/original version of the unmodified object
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      api_base *orig_obj,
                                      api_obj_ctxt_t *obj_ctxt) override;

    /// \brief          reprogram all h/w tables relevant to this object except
    ///                 stage 0 table(s), if any
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reprogram_config(api_obj_ctxt_t *obj_ctxt) override;

    /// \brief re-activate config in the hardware stage 0 tables relevant to
    ///        this object, if any, this reactivation must be based on existing
    ///        state and any of the state present in the dirty object list
    ///        (like clone objects etc.) only and not directly on db objects
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    /// NOTE: this method is called when an object is in the dependent/puppet
    ///       object list
    virtual sdk_ret_t reactivate_config(pds_epoch_t epoch,
                                        api_obj_ctxt_t *obj_ctxt) override;

    /// \brief          read config
    /// \param[out]     info pointer to the info object
    /// \return         SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_subnet_info_t *info);

    /// \brief          add given subnet to the database
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_to_db(void) override;

    /// \brief          delete given subnet from the database
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t del_from_db(void) override;

    /// \brief          this method is called on new object that needs to
    ///                 replace the old version of the object in the DBs
    /// \param[in]      orig_obj    old version of the object being swapped out
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_db(api_base *orig_obj,
                                api_obj_ctxt_t *obj_ctxt) override;

    /// \brief          initiate delay deletion of this object
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t delay_delete(void) override;

    /// \brief          return stringified key of the object (for debugging)
    virtual string key2str(void) const override {
        return "subnet-" + std::string(key_.str());
    }

    /// \brief          helper function to get key given subnet entry
    /// \param[in]      entry    pointer to subnet instance
    /// \return         pointer to the subnet instance's key
    static void *subnet_key_func_get(void *entry) {
        subnet_entry *subnet = (subnet_entry *)entry;
        return (void *)&(subnet->key_);
    }

    /// \brief          return router mac of this subnet
    /// \return         virtual router (VR) mac of this subnet
    mac_addr_t &vr_mac(void) { return vr_mac_; }

    /// \brief          return the subnet key/id
    /// \return         key/id of the subnet
    pds_subnet_key_t key(void) const { return key_; }

    /// \brief          return h/w index for this subnet
    /// \return         h/w table index for this subnet
    uint16_t hw_id(void) const { return hw_id_; }

    /// \brief     return vpc of this subnet
    /// \return    vpc key of this subnet
    pds_vpc_key_t vpc(void) const { return vpc_; }

    /// \brief      return fabric encap of this subnet
    /// \return     fabric encap of this subnet
    pds_encap_t fabric_encap(void) const { return fabric_encap_; }

    /// \brief      return IPv4 route table configured on the subnet
    /// \return     IPv4 route table this subnet
    pds_route_table_key_t v4_route_table(void) const { return v4_route_table_; }

    /// \brief      return IPv6 route table configured on the subnet
    /// \return     IPv6 route table this subnet
    pds_route_table_key_t v6_route_table(void) const { return v6_route_table_; }

    /// \brief     return number of IPv4 ingress policies on the subnet
    /// \return    number of IPv4 ingress policies on the subnet
    uint8_t num_ing_v4_policy(void) const {
        return num_ing_v4_policy_;
    }

    /// \brief          return ingress IPv4 security policy of the subnet
    /// \param[in] n    policy number being queried
    /// \return         ingress IPv4 security policy of the subnet
    pds_policy_key_t ing_v4_policy(uint32_t n) const {
        return ing_v4_policy_[n];
    }

    /// \brief     return number of IPv6 ingress policies on the subnet
    /// \return    number of IPv6 ingress policies on the subnet
    uint8_t num_ing_v6_policy(void) const {
        return num_ing_v6_policy_;
    }

    /// \brief          return ingress IPv6 security policy of the subnet
    /// \param[in] n    policy number being queried
    /// \return         ingress IPv6 security policy of the subnet
    pds_policy_key_t ing_v6_policy(uint32_t n) const {
        return ing_v6_policy_[n];
    }

    /// \brief     return number of IPv4 egress policies on the subnet
    /// \return    number of IPv4 egress policies on the subnet
    uint8_t num_egr_v4_policy(void) const {
        return num_egr_v4_policy_;
    }

    /// \brief          return egress IPv4 security policy of the subnet
    /// \param[in] n    policy number being queried
    /// \return         egress IPv4 security policy of the subnet
    pds_policy_key_t egr_v4_policy(uint32_t n) const {
        return egr_v4_policy_[n];
    }

    /// \brief     return number of IPv6 egress policies on the subnet
    /// \return    number of IPv6 egress policies on the subnet
    uint8_t num_egr_v6_policy(void) const {
        return num_egr_v6_policy_;
    }

    /// \brief          return egress IPv6 security policy of the subnet
    /// \param[in] n    policy number being queried
    /// \return         egress IPv6 security policy of the subnet
    pds_policy_key_t egr_v6_policy(uint32_t n) const {
        return egr_v6_policy_[n];
    }

    /// \brief      return host interface on which this subnet is deployed
    /// \return     host interface on which this subnet is deployed
    pds_ifindex_t host_ifindex(void) const { return host_ifindex_; }

    /// \brief     return impl instance of this subnet object
    /// \return    impl instance of the subnet object
    impl_base *impl(void) { return impl_; }

private:
    /// \brief constructor
    subnet_entry();

    /// \brief destructor
    ~subnet_entry();

    /// \brief      fill the subnet sw spec
    /// \param[out] spec specification
    void fill_spec_(pds_subnet_spec_t *spec);

    /// \brief    free h/w resources used by this object, if any
    ///           (this API is invoked during object deletes)
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t nuke_resources_(void);

private:
    pds_subnet_key_t key_;                    ///< subnet key
    pds_vpc_key_t vpc_;                       ///< vpc of this subnet
    pds_encap_t fabric_encap_;                ///< fabric encap of this subnet
    pds_route_table_key_t v4_route_table_;    ///< IPv4 route table id
    pds_route_table_key_t v6_route_table_;    ///< IPv6 route table id
    ///< number of ingress IPv4 policies
    uint8_t num_ing_v4_policy_;
    ///< ingress IPv4 policies
    pds_policy_key_t ing_v4_policy_[PDS_MAX_SUBNET_POLICY];
    ///< number of ingress IPv6 policies
    uint8_t num_ing_v6_policy_;
    ///< ingress IPv6 policies
    pds_policy_key_t ing_v6_policy_[PDS_MAX_SUBNET_POLICY];
    ///< number of egress IPv4 policies
    uint8_t num_egr_v4_policy_;
    ///< egress IPv4 policies
    pds_policy_key_t egr_v4_policy_[PDS_MAX_SUBNET_POLICY];
    ///< number of egress IPv6 policies
    uint8_t num_egr_v6_policy_;
    ///< egress IPv6 policies
    pds_policy_key_t egr_v6_policy_[PDS_MAX_SUBNET_POLICY];
    mac_addr_t vr_mac_;                       ///< virtual router MAC
    pds_ifindex_t host_ifindex_;              ///< PF/VF attached to this subnet

    ht_ctxt_t ht_ctxt_;                       ///< hash table context

    // P4 datapath specific state
    uint32_t hw_id_;              ///< hardware id
    impl_base *impl_;             ///< impl object instance
    friend class subnet_state;    ///< subnet_state is friend of subnet_entry
} __PACK__;

/// \@}    // end of PDS_SUBNET_ENTRY

}    // namespace api

using api::subnet_entry;

#endif    // __API_SUBNET_HPP__
