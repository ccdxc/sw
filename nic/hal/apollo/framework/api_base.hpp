/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    api_obj.hpp
 *
 * @brief   base object definition for all API objects
 */

#if !defined (__API_OBJ_HPP__)
#define __API_OBJ_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/hal/apollo/framework/api.hpp"

namespace api {

/**
 * @brief    base class for all objects
 */
class api_base {
public:
    /**
     * @brief    constructor
     */
    api_base(){};

    /**
     * @brief    destructor
     */
    ~api_base(){};

    /** @brief        factory method to instantiate an object
     *  @param[in]    api_ctxt API context carrying object related configuration
     */
    static api_base *factory(api_ctxt_t *api_ctxt);

    /**< @brief        mark the object as dirty */
    void set_dirty(void) { dirty_ = true; }

    /**< @brief        return true if the object is dirty */
    bool dirty(void) const { return dirty_; }

    /** @brief        add the object to corresponding internal db(s)
     */
    virtual sdk_ret_t add_to_db(void) { return sdk::SDK_RET_INVALID_OP; }

    /** @brief        delete the object from corresponding internal db(s)
     */
    virtual sdk_ret_t del_from_db(void) { return sdk::SDK_RET_INVALID_OP; }

    /** @brief        find an object based on the object id & key information
     *  @param[in]    api_ctxt API context carrying object related information
     * TODO: ignore_dirty is on shaky ground, will try to get rid of it later
     */
    static api_base *find_obj(api_ctxt_t *api_ctxt, bool ignore_dirty);

    /**
     * @brief    clone this object and return cloned object
     */
    virtual api_base *clone(void) { return NULL; }

    /**
     * @brief    process a create operation on an object
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t process_create(api_ctxt_t *api_ctxt) { return sdk::SDK_RET_INVALID_OP; }
    /**
     * @brief    process a delete operation on an object
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t process_delete(api_ctxt_t *api_ctxt) { return sdk::SDK_RET_INVALID_OP; }
    /**
     * @brief    process a update operation on an object
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t process_update(api_ctxt_t *api_ctxt) { return sdk::SDK_RET_INVALID_OP; }

    /**
     * @brief    process a get operation on an object
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t process_get(api_ctxt_t *api_ctxt) { return sdk::SDK_RET_INVALID_OP; }

    /**
     * @brief    commit() is invokved during commit phase of the API processing
     *           and is not expected to fail as all required resources are
     *           already allocated by now. Based on the API operation, this API
     *           is expected to process either create/retrieve/update/delete. If
     *           any temporary state was stashed in the api_ctxt while
     *           processing this API, it should be freed here
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     *
     * NOTE:     commit() is not expected to fail
     */
    virtual sdk_ret_t commit(api_ctxt_t *api_ctxt) { return sdk::SDK_RET_OK; }

    /**
     * @brief     abort() is invoked during abort phase of the API processing
     *            and is not expected to fail. During this phase, all associated
     *            resources must be freed and global DBs need to be restored
     *            back to their original state and any transient state stashed
     *            in api_ctxt while processing this API should also be freed
     *            here. Note that abort() can be called on an object that is not
     *            fully programmed in h/w, so we should be able to handle cases
     *            where only partial resources allocated and where partial
     *            information is programmed in the h/w (even after allocating
     *            resources fully)
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t abort(api_ctxt_t *api_ctxt) { return sdk::SDK_RET_OK; }

protected:
    bool    dirty_;    /**< indicates whether object is committed or not. while
                            API batch processing is in progress, object can be
                            in its corresponding db and marked as dirty if it
                            is going through or not */
};

}    // namespace api

#endif    /** __API_OBJ_HPP__ */

