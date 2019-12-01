//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// TEP entry handling
///
//----------------------------------------------------------------------------

#ifndef __API__TEP_HPP__
#define __API__TEP_HPP__

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"

// TODO: fix this when fte plugin is available
#define PDS_REMOTE_TEP_MAC            0x0E0D0A0B0200

namespace api {

/// \defgroup PDS_TEP_ENTRY - TEP entry functionality
/// \ingroup PDS_TEP
/// @{

/// \brief TEP entry
class tep_entry : public api_base {
public:
    /// \brief     factory method to allocate and initialize a TEP entry
    /// \param[in] spec specification
    /// \return    new instance of TEP or NULL, in case of error
    static tep_entry *factory(pds_tep_spec_t *spec);

    /// \brief     release all the s/w state associate with the given TEP,
    //             if any, and free the memory
    /// \param[in] tep TEP to be freed
    /// \NOTE:     h/w entries should have been cleaned up (by calling
    ///            impl->cleanup_hw() before calling this
    static void destroy(tep_entry *tep);

    /// \brief    clone this object and return cloned object
    /// \param[in]    api_ctxt API context carrying object related configuration
    /// \return       new object instance of current object
    virtual api_base *clone(api_ctxt_t *api_ctxt) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] tep    TEP entry to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(tep_entry *tep);

    /// \brief     initialize TEP entry with the given config
    /// \param[in] api_ctxt API context carrying the configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /// \brief     allocate h/w resources for this object
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        obj_ctxt_t *obj_ctxt) override;

    /// \brief     program all h/w tables relevant to this object except
    ///            stage 0 table(s), if any
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_create(obj_ctxt_t *obj_ctxt) override;

    /// \brief  free h/w resources used by this object, if any
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(void) override;

    /// \brief     cleanup all h/w tables relevant to this object except
    ///            stage 0 table(s), if any, by updating packed entries
    ///            with latest epoch#
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_config(obj_ctxt_t *obj_ctxt) override;

    /// \brief     update all h/w tables relevant to this object except stage 0
    ///            table(s), if any, by updating packed entries with
    ///            latest epoch#
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_update(api_base *orig_obj,
                                    obj_ctxt_t *obj_ctxt) override;

    /// \brief     activate the epoch in the dataplane by programming stage 0
    ///            tables, if any
    /// \param[in] epoch    epoch being activated
    /// \param[in] api_op   api operation
    /// \param[in] orig_obj old/original version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// return     SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      api_base *orig_obj,
                                      obj_ctxt_t *obj_ctxt) override;

    /// \brief  add given TEP to the database
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_to_db(void) override;

    /// \brief  delete given TEP from the database
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t del_from_db(void) override;

    /// \brief     this method is called on new object that needs to replace
    ///            the old version of the object in the DBs
    /// \param[in] orig_obj old version of the object being swapped out
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_db(api_base *orig_obj,
                                obj_ctxt_t *obj_ctxt) override;

    /// \brief initiate delay deletion of this object
    virtual sdk_ret_t delay_delete(void) override;

    /// \brief return stringified key of the object (for debugging)
    virtual string key2str(void) const override {
        return "tep-"  + std::to_string(key_.id) + "-" +
                        string(ipaddr2str(&remote_ip_));
    }

    ///\brief read config
    ///\param[in]  key Pointer to the key object
    ///\param[out] info Pointer to the info object
    ///\return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_tep_key_t *key, pds_tep_info_t *info);

    /// \brief     helper function to get key given TEP entry
    /// \param[in] entry pointer to TEP instance
    /// \return    pointer to the TEP instance's key
    static void *tep_key_func_get(void *entry) {
        tep_entry *tep = (tep_entry *)entry;
        return (void *)&(tep->key_);
    }

    /// \brief   helper function to get size of key
    /// \return  size of key
    static uint32_t key_size(void) {
        return sizeof(pds_tep_key_t);
    }

    /// \brief    return TEP's key
    /// \return    key of the TEP
    pds_tep_key_t key(void) const { return key_; }

    /// \brief    return TEP's type
    /// \return    type of the TEP
    pds_tep_type_t type(void) const { return type_; }

    /// \brief    return true if this remote service TEP
    /// \return    true if the TEP is service TEP and also remote
    bool remote_svc(void) const { return remote_svc_; }

    /// \brief    return TEP IPv4 address
    /// \return    IPv4 address of the TEP
    ip_addr_t& ip(void) { return remote_ip_; }

    /// \brief    return the MAC address corresponding to this TEP
    /// \return    ethernet MAC address of this TEP (configured/learnt)
    mac_addr_t& mac(void) { return mac_; }

    /// \brief    return the nexthop type
    /// \return    type of the nexthop this TEP is pointing to
    pds_nh_type_t nh_type(void) const { return nh_type_; }

    /// \brief    return the nexthop of this TEP
    /// \return    nexthop this TEP is pointing to
    pds_nexthop_key_t nh(void) const { return nh_; }

    /// \brief    return the nexthop group of this TEP
    /// \return    nexthop group this TEP is pointing to
    pds_nexthop_group_key_t nh_group(void) const { return nh_group_; }

    /// \brief    return the TEP this TEP points to
    /// \return    TEP this TEP is pointing to
    pds_tep_key_t tep(void) const { return tep_; }

    /// \brief    return impl instance of this TEP object
    /// \return    impl instance of the TEP object
    impl_base *impl(void) { return impl_; }

private:
    /// \brief constructor
    tep_entry();

    /// \brief destructor
    ~tep_entry();

    /// \brief  free h/w resources used by this object, if any
    ///         (this API is invoked during object deletes)
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t nuke_resources_(void);

private:
    /// \NOTE: the mac address for this TEP will be learnt in the datapath from
    ///        ARP traffic in the underlay, but we don't need to store it in
    ///        s/w, we will directly refresh the TEP_TX table
    pds_tep_key_t  key_;        ///< TEP key
    pds_tep_type_t type_;       ///< TEP type
    ip_addr_t      remote_ip_;  ///< TEP IP
    bool           remote_svc_; ///< true if this is remote (service) TEP
    mac_addr_t     mac_;        ///< (learnt/configured) MAC address of this TEP
    pds_nh_type_t  nh_type_;    ///< type of the nexthop for this TEP
    ///< one of possible nexthop types
    union {
        pds_nexthop_key_t nh_;
        pds_nexthop_group_key_t nh_group_;
        pds_tep_key_t tep_;
    };
    ht_ctxt_t      ht_ctxt_;    ///< hash table context
    impl_base      *impl_;      ///< impl object instance
    friend class   tep_state;   ///< tep_state is friend of tep_entry
} __PACK__;

/// \@}

}  // namespace api

using api::tep_entry;

#endif    // __API__TEP_HPP__
