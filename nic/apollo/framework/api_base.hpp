//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines base ojbect for all API objects
///
//----------------------------------------------------------------------------

#ifndef __FRAMEWORK_API_BASE_HPP__
#define __FRAMEWORK_API_BASE_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/framework/obj_base.hpp"
#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/api/include/pds.hpp"

using std::string;

//typedef struct obj_ctxt_s obj_ctxt_t;

namespace api {

/// \brief  Basse class for all api related objects
class api_base : public obj_base {
public:
     /// \brief Constructor
    api_base() {
        in_dirty_list_ = 0;
        in_deps_list_ = 0;
        rsvd_rsc_ = 0;
    };

     /// \brief Destructor
    virtual ~api_base(){};

    /// \brief Factory method to instantiate an object
    /// \param[in] api_ctxt API context carrying object related configuration
    static api_base *factory(api_ctxt_t *api_ctxt);

    /// \brief build method to instantiate an object based on current (s/w
    //         and/or hw state)
    /// \param[in] api_ctxt API context carrying object related configuration
    /// \remark
    /// This API is expected to be used by the API engine only while handling
    //  updates or deletes on stateless objects
    static api_base *build(api_ctxt_t *api_ctxt);

    /// \brief    free a stateless entry's temporary s/w only resources like
    ///           memory etc., for a stateless entry calling destroy() will
    ///           remove resources from h/w, which can't be done during ADD/UPD
    ///           etc. operations esp. when object is constructed on the fly
    /// \param[in] obj_id     object identifier
    /// \param[in] api_obj    api object being freed
    static void soft_delete(obj_id_t obj_id, api_base *api_obj);

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] obj_id     object identifier
    /// \param[in] api_obj    api object being freed
    /// \return   SDK_RET_OK or error code
    static sdk_ret_t free(obj_id_t obj_id, api_base *api_obj);

    /// \brief Initiaize the api object with given config
    /// \param[in] api_ctxt Transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) {
        return SDK_RET_INVALID_OP;
    }

    /// \brief  Allocate hardware resources for this object
    /// \param[in] orig_obj Old version of the unmodified object
    /// \param[in] obj_ctxt Transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        obj_ctxt_t *obj_ctxt) {
        return SDK_RET_INVALID_OP;
    }

    /// \brief program config in the hardware during create operation
    /// Program all hardware tables relevant to this object except stage 0
    /// table(s), if any and also set the valid bit
    /// \param[in] obj_ctxt Transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_create(obj_ctxt_t *obj_ctxt) {
        return SDK_RET_INVALID_OP;
    }

    /// \brief    re-program config in the hardware
    /// re-program all hardware tables relevant to this object except stage 0
    /// table(s), if any and this reprogramming must be based on existing state
    /// and any of the state present in the dirty object list (like clone
    /// objects etc.)
    /// \param[in] api_op API operation
    /// \return #SDK_RET_OK on success, failure status code on error
    /// NOTE: this method is called when an object is in the dependent/puppet
    ///       object list
    virtual sdk_ret_t reprogram_config(api_op_t api_op) {
        return SDK_RET_INVALID_OP;
    }

    /// \brief    free hardware resources used by this object, if any
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(void) {
        return SDK_RET_INVALID_OP;
    }

    /// \brief    cleanup config from the hardware
    /// Cleanup all hardware tables relevant to this object except stage 0
    /// table(s), if any, by updating packed entries with latest epoch#
    /// and setting invalid bit (if any) in the hardware entries
    /// \param[in] obj_ctxt Transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_config(obj_ctxt_t *obj_ctxt) {
        return SDK_RET_INVALID_OP;
    }

    /// \brief    compute the object diff during update operation compare the
    ///           attributes of the object on which this API is invoked and the
    ///           attrs provided in the update API call passed in the object
    ///           context (as cloned object + api_params) and compute the upd
    ///            bitmap (and stash in the object context for later use)
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t compute_update(obj_ctxt_t *obj_ctxt) {
        return SDK_RET_INVALID_OP;
    }

    /// \brief    update config in the hardware
    /// update all hardware tables relevant to this object except stage 0
    /// table(s), if any, by updating packed entries with latest epoch#
    /// \param[in] orig_obj old/original version of the unmodified object
    /// \param[in] obj_ctxt Transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_update(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
        return SDK_RET_INVALID_OP;
    }

    /// \brief Activate the epoch in the dataplane
    /// \param[in] epoch epoch/version of new config
    /// \param[in] api_op API operation
    /// \param[in] orig_obj old/original version of the unmodified object
    /// \param[in] obj_ctxt Transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      api_base *orig_obj,
                                      obj_ctxt_t *obj_ctxt) {
        return SDK_RET_INVALID_OP;
    }

    /// \brief re-activate config in the hardware stage 0 tables relevant to
    ///        this object, if any, this reactivation must be based on existing
    ///        state and any of the state present in the dirty object list
    ///        (like clone objects etc.) only and not directly on db objects
    /// \param[in] epoch epoch/version of new config
    /// \param[in] api_op API operation
    /// \return #SDK_RET_OK on success, failure status code on error
    /// NOTE: this method is called when an object is in the dependent/puppet
    ///       object list
    virtual sdk_ret_t reactivate_config(pds_epoch_t epoch, api_op_t api_op) {
        return SDK_RET_INVALID_OP;
    }

    /// \brief Update software database with new object
    /// This method is called on new object that needs to replace the
    /// old version of the object in the DBs
    /// \param[in] old Old version of the object being swapped out
    /// \param[in] obj_ctxt Transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_db(api_base *old_obj, obj_ctxt_t *obj_ctxt) {
        return SDK_RET_INVALID_OP;
    }

    /// \brief Add the object to corresponding internal db(s)
    virtual sdk_ret_t add_to_db(void) { return SDK_RET_INVALID_OP; }

    /// \brief Delete the object from corresponding internal db(s)
    virtual sdk_ret_t del_from_db(void) { return SDK_RET_INVALID_OP; }

    ///< \brief Enqueue the object for delayed destruction
    virtual sdk_ret_t delay_delete(void) { return SDK_RET_INVALID_OP; }

    /// \brief Find an object based on the object id & key information
    /// \param[in] api_ctxt API context carrying object related information
    /// \remark
    ///   - TODO: skip_dirty is on shaky ground, will try to get rid of it later
    static api_base *find_obj(api_ctxt_t *api_ctxt, bool skip_dirty=false);

    /// \brief Find an object based on the object id & key information
    /// \param[in] obj_id    object id
    /// \param[in] key       pointer to the key of the object
    /// \remark
    ///   - This API will try to find the object from the dirty list and
    ///     dependency list and return that first (potentially a cloned obj),
    ///     if its not in these lists the original object from db will be
    ///     returned as-is
    static api_base *find_obj(obj_id_t obj_id, void *key);

    /// \brief clone this object and return cloned object
    /// \param[in]    api_ctxt API context carrying object related configuration
    /// \return       new object instance of current object
    virtual api_base *clone(api_ctxt_t *api_ctxt) {
        return NULL;
    }

    /// \brief compute all the objects depending on this object and add to
    ///        framework's dependency list
    /// \param[in] obj_ctxt Transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_deps(obj_ctxt_t *obj_ctxt) {
        return SDK_RET_INVALID_OP;
    }

    /// \brief Mark the object as dirty
    void set_in_dirty_list(void) { in_dirty_list_ = 1; }

    /// \brief returns true if the object is in dirty list
    bool in_dirty_list(void) const { return in_dirty_list_ ? true : false; }

    /// \brief Clear the dirty bit on this object
    void clear_in_dirty_list(void) { in_dirty_list_ = 0; }

    /// \brief Mark the object as dependent object
    void set_in_deps_list(void) { in_deps_list_ = 1; }

    /// \brief returns true if the object is in dependent list
    bool in_deps_list(void) const { return in_deps_list_ ? true : false; }

    /// \brief Clear the dependent object bit on this object
    void clear_in_deps_list(void) { in_deps_list_ = 0; }

    /// \brief return true if object is 'stateless' given an object id
    static bool stateless(obj_id_t obj_id);

    /// \brief return true if object reserved any h/w resources
    bool rsvd_rsc(void) const { return rsvd_rsc_ ? true : false; }

    /// \brief set reserved resources flag on this object
    void set_rsvd_rsc(void) { rsvd_rsc_ = 1; }

    /// \brief clear reserved resources flag on this object
    void clear_rsvd_rsc(void) { rsvd_rsc_ = 0; }

    /// \brief return stringified key of the object (for debugging)
    virtual string key2str(void) const { return "api_base_key"; }

    /// \brief return stringified contents of the obj (for debugging)
    virtual string tostr(void) const { return "api_base"; }

protected:
    uint8_t in_dirty_list_:1;    ///< true if object is in the dirty list
    uint8_t in_deps_list_:1;     ///< true if object is in dependent object list
    uint8_t rsvd_rsc_:1;         ///< true if resources are reserved
} __PACK__;

}    // namespace api

using api::api_base;

#endif    // __FRAMEWORK_API_BASE_HPP__
