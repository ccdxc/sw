// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file handles mirror session
///
//----------------------------------------------------------------------------

#ifndef __API_MIRROR_HPP__
#define __API_MIRROR_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_mirror.hpp"

namespace api {

// forward declaration
// class mirror_state;

/// \defgroup PDS_MIRROR_SESSION - mirror session functionality
/// @{

/// \brief    mirror session
class mirror_session : public api_base {
public:
    /// \brief          factory method to allocate and initialize a mirror
    ///                 session entry
    /// \param[in]      spec    mirror session information
    /// \return         new instance of mirror or NULL, in case of error
    static mirror_session *factory(pds_mirror_session_spec_t *spec);

    /// \brief          release all the s/w state associate with the given
    ///                 mirror session, if any, and free the memory
    /// \param[in]      ms    mirror session to be freed
    /// \NOTE: h/w entries should have been cleaned up (by calling
    ///        impl->cleanup_hw() before calling this
    static void destroy(mirror_session *ms);

    /// \brief    clone this object and return cloned object
    /// \param[in]    api_ctxt API context carrying object related configuration
    /// \return       new object instance of current object
    virtual api_base *clone(api_ctxt_t *api_ctxt) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] session    mirror session to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(mirror_session *session);

    /// \brief     allocate h/w resources for this object
    /// \param[in] orig_obj    old version of the unmodified object
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     free h/w resources used by this object, if any
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(void) override;
    /// \brief          initialize mirror session with the given config
    /// \param[in]      api_ctxt API context carrying the configuration
    /// \return         SDK_RET_OK on success, failure status code on error

    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

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

    /// \brief        add all objects that may be affected if this object is
    ///               updated to framework's object dependency list
    /// \param[in]    obj_ctxt    transient state associated with this API
    ///                           processing
    /// \return       SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_deps(api_obj_ctxt_t *obj_ctxt) override {
        // no other objects are affected if mirror session is modified
        return SDK_RET_OK;
    }

    /// \brief          update all h/w tables relevant to this object except
    ///                 stage 0 table(s), if any, by updating packed entries
    ///                 with latest epoch#
    /// \param[in]      orig_obj    old version of the unmodified object
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_update(api_base *orig_obj,
                                    api_obj_ctxt_t *obj_ctxt) override;

    /// \brief          activate the epoch in the dataplane by programming
    ///                 stage 0 tables, if any
    /// \param[in]      epoch       epoch being activated
    /// \param[in]      api_op      api operation
    /// \param[in]      orig_obj old/original version of the unmodified object
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      api_base *orig_obj,
                                      api_obj_ctxt_t *obj_ctxt) override;

    ///\brief read config
    ///\param[out] info pointer to the info object
    ///\return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_mirror_session_info_t *info);

    /// \brief          add given mirror session to the database
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_to_db(void) override;

    /// \brief          delete given mirror session from the database
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
    virtual sdk_ret_t delay_delete(void) override;

    /// \brief          return stringified key of the object (for debugging)
    virtual string key2str(void) const override {
        return "mrror-" + std::string(key_.str());
    }

    /// \brief     return the key/id of this vnic
    /// \return    key/id of the vnic object
    const pds_obj_key_t key(void) const { return key_; }

    /// \brief     return the uplink interface in RSPAN configuration
    /// \return    uplink interface in RSPAN configuration
    const pds_obj_key_t& rspan_uplink_if(void) const { return rspan_.uplink_if_; }

    /// \brief     return the encap in RSPAN configuration
    /// \return    encap in RSPAN configuration
    pds_encap_t rspan_encap(void) const { return rspan_.encap_; }

    /// \brief     return the ERSPAN destination vpc
    /// \return    ERSPAN destination vpc
    const pds_obj_key_t& erspan_vpc(void) const { return erspan_.vpc_; }

    /// \brief     return the ERSPAN destination TEP
    /// \return    ERSPAN destination TEP
    const pds_obj_key_t& erspan_dest_tep(void) const { return erspan_.tep_; }

    /// \brief     return the ERSPAN destination mapping
    /// \return    ERSPAN destination mapping
    const pds_obj_key_t& erspan_dest_mapping(void) const { return erspan_.mapping_; }

    /// \brief     return impl instance of this vnic object
    /// \return    impl instance of the vnic object
    impl_base *impl(void) { return impl_; }

private:
    /// \brief constructor
    mirror_session();

    /// \brief destructor
    ~mirror_session();

    /// \brief      fill the vnic sw spec
    /// \param[out] spec specification
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t fill_spec_(pds_mirror_session_spec_t *spec);

    /// \brief    free h/w resources used by this object, if any
    ///           (this API is invoked during object deletes)
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t nuke_resources_(void);

private:
    pds_obj_key_t key_;                ///< mirror session keyA
    pds_mirror_session_type_t type;    ///< mirror session type
    union {
        struct {
            /// uplink interface the rspanned packets go out on
            pds_obj_key_t uplink_if_;
            /// encap on the spanned packets (only .1q is supported)
            pds_encap_t encap_;
        } rspan_;
        struct {
            /// vpc where ERSPAN destination is (and the
            pds_obj_key_t vpc_;
            union {
                /// Tunnel IP, in case ERSPAN destination is in underlay VPC
                pds_obj_key_t tep_;
                /// mapping key, in case ERSPAN destination is in overlay VPC
                pds_obj_key_t mapping_;
            };
        } erspan_;
    };
    impl_base *impl_;                  ///< impl object instance

    ///< mirror_session_state is friend of mirror_session
    friend class mirror_session_state;
} __PACK__;

/// \@}    // end of PDS_MIRROR_SESSION

}    // namespace api

using api::mirror_session;

#endif    // __API_MIRROR_HPP__
