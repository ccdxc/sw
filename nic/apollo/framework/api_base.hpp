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
#include "nic/apollo/include/api/pds.hpp"

using std::string;

namespace api {

/// \brief  Basse class for all api related objects
class api_base : public obj_base {
public:
     /// \brief Constructor
    api_base(){};

     /// \brief Destructor
    ~api_base(){};

    /// \brief Factory method to instantiate an object
    ///
    /// \param[in] api_ctxt API context carrying object related configuration
    static api_base *factory(api_ctxt_t *api_ctxt);

    /// \brief Initiaize the api object with given config
    ///
    /// \param[in] api_ctxt Transient state associated with this API
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /// \brief  Allocate hardware resources for this object
    ///
    /// \param[in] orig_obj Old version of the unmodified object
    /// \param[in] obj_ctxt Transient state associated with this API
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        obj_ctxt_t *obj_ctxt) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /// \brief Program config in the hardware
    ///
    /// Program all hardware tables relevant to this object except stage 0
    /// table(s), if any and also set the valid bit
    ///
    /// \param[in] obj_ctxt Transient state associated with this API
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_config(obj_ctxt_t *obj_ctxt) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /// \brief Free hardware resources used by this object, if any
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(void) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /// \brief Cleanup config from the hardware
    ///
    /// Cleanup all hardware tables relevant to this object except stage 0
    /// table(s), if any, by updating packed entries with latest epoch#
    /// and setting invalid bit (if any) in the hardware entries
    ///
    /// \param[in] obj_ctxt Transient state associated with this API
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_config(obj_ctxt_t *obj_ctxt) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /// \brief Update config in the hardware
    ///
    /// Update all hardware tables relevant to this object except stage 0
    /// table(s), if any, by updating packed entries with latest epoch#
    ///
    /// \param[in] orig_obj Old version of the unmodified object
    /// \param[in] obj_ctxt Transient state associated with this API
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /// \brief Activate the epoch in the dataplane
    ///
    /// \param[in] api_op API operation
    /// \param[in] obj_ctxt Transient state associated with this API
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      obj_ctxt_t *obj_ctxt) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /// \brief Update software database with new object
    ///
    /// This method is called on new object that needs to replace the
    /// old version of the object in the DBs
    ///
    /// \param[in] old Old version of the object being swapped out
    /// \param[in] obj_ctxt Transient state associated with this API
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_db(api_base *old_obj, obj_ctxt_t *obj_ctxt) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /// \brief Add the object to corresponding internal db(s)
    virtual sdk_ret_t add_to_db(void) { return sdk::SDK_RET_INVALID_OP; }

    /// \brief Delete the object from corresponding internal db(s)
    virtual sdk_ret_t del_from_db(void) { return sdk::SDK_RET_INVALID_OP; }

    ///< \brief Enqueue the object for delayed destruction
    virtual sdk_ret_t delay_delete(void) { return sdk::SDK_RET_INVALID_OP; }

    /// \brief Find an object based on the object id & key information
    ///
    /// \param[in] api_ctxt API context carrying object related information
    ///
    /// \remark
    ///   - TODO: skip_dirty is on shaky ground, will try to get rid of it later
    static api_base *find_obj(api_ctxt_t *api_ctxt, bool skip_dirty=false);

    /// \brief Clone this object and return cloned object
    virtual api_base *clone(void) { return NULL; }

    /// \brief Mark the object as dirty
    void set_in_dirty_list(void) { in_dirty_list_ = true; }

    /// \brief Returns true if the object is in dirty list
    bool is_in_dirty_list(void) const { return in_dirty_list_; }

    /// \brief Clear the dirty bit on this object
    void clear_in_dirty_list(void) { in_dirty_list_ = false; }

    /// \brief Set hardware dirty
    ///
    /// Set hardware dirty bit to indicate that hw entries are updated
    /// with the config in this object, but epoch not yet activated
    void set_hw_dirty(void) { hw_dirty_ = true; }

    /// \brief Return true if hw is "touched" with the config
    bool is_hw_dirty(void) const { return hw_dirty_; }

    /// \brief Clear hw dirty bit on this object
    void clear_hw_dirty(void) { hw_dirty_ = false; }

    /// \brief Return true if this is 'stateless' object
    bool stateless(void) { return stateless_; }

    /// \brief Return stringified key of the object (for debugging)
    virtual string key2str(void) const { return "api_base_key"; }

    /// \brief Return stringified contents of the obj (for debugging)
    virtual string tostr(void) const { return "api_base"; }

protected:
    bool    in_dirty_list_; ///< True if object is in the dirty list
    bool    hw_dirty_;      ///< True if hw entries are updated,
                            ///<  but not yet activated
    bool    stateless_;     ///< True this object doesn't go into any databases
} __PACK__;

}    // namespace api

using api::api_base;
 
#endif    // __FRAMEWORK_API_BASE_HPP__
