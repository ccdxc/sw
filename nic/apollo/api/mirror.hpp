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

    ///
    /// \brief    build object given its key from the (sw and/or hw state we
    ///           have) and return an instance of the object (this is useful for
    ///           stateless objects to be operated on by framework during DELETE
    ///           or UPDATE operations)
    /// \param[in] key    key of object instance of interest
    static mirror_session *build(pds_mirror_session_key_t *key);

    ///
    /// \brief    free a stateless entry's temporary s/w only resources like
    ///           memory etc., for a stateless entry calling destroy() will
    ///           remove resources from h/w, which can't be done during ADD/UPD
    ///           etc. operations esp. when object is constructed on the fly
    /// \param[in] ms    mirror session
    ///
    static void soft_delete(mirror_session *ms);

    /// \brief          initialize mirror session with the given config
    /// \param[in]      api_ctxt API context carrying the configuration
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /// \brief          allocate h/w resources for this object
    /// \param[in]      orig_obj    old version of the unmodified object
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        api_obj_ctxt_t *obj_ctxt) override;

    /// \brief          program all h/w tables relevant to this object except
    ///                 stage 0 table(s), if any
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_create(api_obj_ctxt_t *obj_ctxt) override;

    /// \brief          free h/w resources used by this object, if any
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(void) override;

    /// \brief          cleanup all h/w tables relevant to this object except
    ///                 stage 0 table(s), if any, by updating packed entries
    ///                 with latest epoch#
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_config(api_obj_ctxt_t *obj_ctxt) override;

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
        return "ms-" + std::to_string(key_.id);
    }

    ///\brief read config
    ///\param[in]  key Pointer to the key object
    ///\param[out] info Pointer to the info object
    ///\return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_mirror_session_key_t *key, pds_mirror_session_info_t *info);

private:
    /// \brief constructor
    mirror_session();

    /// \brief destructor
    ~mirror_session();

    /// \brief    free h/w resources used by this object, if any
    ///           (this API is invoked during object deletes)
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t nuke_resources_(void);

private:
    pds_mirror_session_key_t    key_;        ///< mirror session key
    impl_base                   *impl_;      ///< impl object instance

    ///< mirror_session_state is friend of mirror_session
    friend class mirror_session_state;
} __PACK__;

/// \@}    // end of MIRROR_SESSION_ENTRY

}    // namespace api

using api::mirror_session;

#endif    // __API_MIRROR_HPP__
