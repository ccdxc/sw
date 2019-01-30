/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    api_obj.hpp
 *
 * @brief   base object definition for all API objects
 */

#if !defined (__API_BASE_HPP__)
#define __API_BASE_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/framework/obj_base.hpp"
#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/include/api/oci.hpp"

using std::string;

namespace api {

/**
 * @brief    base class for all api related objects
 */
class api_base : public obj_base {
public:
    /**
     * @brief    constructor
     */
    api_base(){};

    /**
     * @brief    destructor
     */
    ~api_base(){};

    /**
     * @brief        factory method to instantiate an object
     * @param[in]    api_ctxt API context carrying object related configuration
     */
    static api_base *factory(api_ctxt_t *api_ctxt);

    /**
     * @brief    initiaize the api object with given config
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /**
     * @brief    allocate h/w resources for this object
     * @param[in] orig_obj    old version of the unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        obj_ctxt_t *obj_ctxt) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /**
     * @brief    program all h/w tables relevant to this object except stage 0
     *           table(s), if any and also set the valid bit
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t program_config(obj_ctxt_t *obj_ctxt) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /**
     * @brief     free h/w resources used by this object, if any
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t release_resources(void) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /**
     * @brief    cleanup all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     *           and setting invalid bit (if any) in the h/w entries
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t cleanup_config(obj_ctxt_t *obj_ctxt) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /**
     * @brief    update all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     * @param[in] orig_obj    old version of the unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t update_config(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /**
     * @brief    activate the epoch in the dataplane
     * @param[in] api_op      api operation
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t activate_config(oci_epoch_t epoch, api_op_t api_op,
                                      obj_ctxt_t *obj_ctxt) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /**
     * @brief    this method is called on new object that needs to replace the
     *           old version of the object in the DBs
     * @param[in] old         old version of the object being swapped out
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t update_db(api_base *old_obj, obj_ctxt_t *obj_ctxt) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /**< @brief        add the object to corresponding internal db(s) */
    virtual sdk_ret_t add_to_db(void) { return sdk::SDK_RET_INVALID_OP; }

    /**< @brief        delete the object from corresponding internal db(s) */
    virtual sdk_ret_t del_from_db(void) { return sdk::SDK_RET_INVALID_OP; }

    /**< @brief        enqueue the object for delayed destruction */
    virtual sdk_ret_t delay_delete(void) { return sdk::SDK_RET_INVALID_OP; }

    /**
     * @brief        find an object based on the object id & key information
     *  @param[in]    api_ctxt API context carrying object related information
     * TODO: skip_dirty is on shaky ground, will try to get rid of it later
     */
    static api_base *find_obj(api_ctxt_t *api_ctxt, bool skip_dirty=false);

    /**< @brief    clone this object and return cloned object */
    virtual api_base *clone(void) { return NULL; }

    /**< @brief    mark the object as dirty */
    void set_in_dirty_list(void) { in_dirty_list_ = true; }

    /**< @brief    return true if the object is in dirty list */
    bool is_in_dirty_list(void) const { return in_dirty_list_; }

    /**< @brief    clear the dirty bit on this object */
    void clear_in_dirty_list(void) { in_dirty_list_ = false; }

    /**
     * @brief    set hw dirty bit to indicate that hw entries are updated
     *           with the config in this object, but epoch not yet activated
     */
    void set_hw_dirty(void) { hw_dirty_ = true; }

    /**< @brief    return true if hw is "touched" with the config */
    bool is_hw_dirty(void) const { return hw_dirty_; }

    /**< @brief    clear hw dirty bit on this object */
    void clear_hw_dirty(void) { hw_dirty_ = false; }

    /**< @brief    return true if this is 'stateless' object */
    bool stateless(void) { return stateless_; }

    /**< @brief    return string to uniquely identify obj (for debugging) */
    virtual string tostr(void) const { return "api_base"; }

protected:
    bool    in_dirty_list_;    /**< true if object is in the dirty list */
    bool    hw_dirty_;         /**< true if hw entries are updated but not yet activated */
    bool    stateless_;        /**< true this object doesn't go into any databases */
} __PACK__;

}    // namespace api

using api::api_base;
 
#endif    /** __API_BASE_HPP__ */
