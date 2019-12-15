//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vpc peer entry handling
///
//----------------------------------------------------------------------------

#ifndef __API_VPC_PEER_HPP__
#define __API_VPC_PEER_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"

namespace api {

// forward declaration
class vpc_peer_state;

/// \defgroup PDS_VPC_PEER - VPC peer entry functionality
/// \ingroup PDS_VPC
/// @{

/// \brief VPC peer entry
class vpc_peer_entry : public api_base {
public:
    /// \brief     factory method to allocate and initialize a VPC peer entry
    /// \param[in] spec    VPC peer specification
    /// \return    new instance of VPC peering or NULL, in case of error
    static vpc_peer_entry *factory(pds_vpc_peer_spec_t *spec);

    /// \brief     release all the s/w state associate with the given VPC
    ///            peering, if any, and free the memory
    /// \param[in] vpc_peer    VPC peering to be freed
    static void destroy(vpc_peer_entry *vpc_peer);

    /// \brief    clone this object and return cloned object
    /// \param[in]    api_ctxt API context carrying object related configuration
    /// \return       new object instance of current object
    virtual api_base *clone(api_ctxt_t *api_ctxt) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] vpc_peer    vpc peer entry to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(vpc_peer_entry *vpc_peer);

    /// \brief    build object given its key from the (sw and/or hw state we
    ///           have) and return an instance of the object (this is useful for
    ///           stateless objects to be operated on by framework during DELETE
    ///           or UPDATE operations)
    /// \param[in] key    key of object instance of interest
    static vpc_peer_entry *build(pds_vpc_peer_key_t *key);

    /// \brief    free a stateless entry's temporary s/w only resources like
    ///           memory etc., for a stateless entry calling destroy() will
    ///           remove resources from h/w, which can't be done during ADD/UPD
    ///           etc. operations esp. when object is constructed on the fly
    /// \param[in] vpc_peer    vpc peering to be freed
    static void soft_delete(vpc_peer_entry *vpc_peer);

    /// \brief     initialize a VPC peering entry with the given config
    /// \param[in] api_ctxt API context carrying the configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /// \brief     allocate h/w resources for this object
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        api_obj_ctxt_t *obj_ctxt) override;

    /// \brief  free h/w resources used by this object, if any
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(void) override;

    /// \brief     program all h/w tables relevant to this object except
    ///            stage 0 table(s), if any
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_create(api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     cleanup all h/w tables relevant to this object except
    ///            stage 0 table(s), if any, by updating packed entries
    ///            with latest epoch#
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_config(api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     update all h/w tables relevant to this object except
    ///            stage 0 table(s), if any, by updating packed entries
    ///            with latest epoch#
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_update(api_base *orig_obj,
                                    api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     activate the epoch in the dataplane by programming
    ///            stage 0 tables, if any
    /// \param[in] epoch    epoch being activated
    /// \param[in] api_op   api operation
    /// \param[in] orig_obj old/original version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      api_base *orig_obj,
                                      api_obj_ctxt_t *obj_ctxt) override;

    /// \brief  add given VPC peering to the database
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_to_db(void) override {
        // no-op for stateless objects
        return SDK_RET_OK;
    }

    /// \brief  delete given VPC peering from the database
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t del_from_db(void) override {
        // no-op for stateless objects
        return SDK_RET_OK;
    }

    /// \brief     this method is called on new object that needs to
    ///            replace the old version of the object in the DBs
    /// \param[in] orig_obj old version of the object being swapped out
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_db(api_base *orig_obj,
                                api_obj_ctxt_t *obj_ctxt) override;

    /// \brief initiate delay deletion of this object
    virtual sdk_ret_t delay_delete(void) override;

    /// \brief return stringified key of the object (for debugging)
    virtual string key2str(void) const override {
        return "vpc-peer-" + std::to_string(key_.id);
    }

    /// \brief          read config
    /// \param[out]     info pointer to the info object
    /// \return         SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_vpc_peer_info_t *info);

    /// \brief    return the key of this object
    /// \return    key contains ID of vpc peer
    pds_vpc_peer_key_t key(void) { return key_; }

private:
    /// \brief constructor
    vpc_peer_entry();

    /// \brief destructor
    ~vpc_peer_entry();

    /// \brief      fill the VPC peer sw spec
    /// \param[out] spec specification
    void fill_spec_(pds_vpc_peer_spec_t *spec);

    /// \brief  free h/w resources used by this object, if any
    ///         (this API is invoked during object deletes)
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t nuke_resources_(void);

private:
    pds_vpc_peer_key_t key_;         ///< VPC peering key
    impl_base          *impl_;       ///< impl object instance

    friend class vpc_peer_state;     ///< a friend of vpc_peer_entry
} __PACK__;

/// \@}

}    // namespace api

using api::vpc_peer_entry;

#endif    // __API_VPC_PEER_HPP__
