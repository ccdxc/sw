/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    api_obj.hpp
 *
 * @brief   base object definition for all API objects
 */

#if !defined (__API_OBJ_HPP__)
#define __API_OBJ_HPP__

#include "nic/hal/apollo/framework/api.hpp"

namespace api {

/**
 * @brief    base class for all objects
 */
class api_base {
public:
    api_base(){};
    ~api_base(){};
    static api_base *factory(obj_id_t obj_id) { return NULL; } // TODO: fill this in later
    static void destroy(api_base *obj) { } // TODO: fill this in later
    sdk_ret_t init(api_ctxt_t *api_ctxt) { return sdk::SDK_RET_OK; } // TODO: fill this in later
    void set_dirty(void) { dirty_ = true; }
    bool dirty(void) const { return dirty_; }
    virtual sdk_ret_t add_to_db(void) { return sdk::SDK_RET_OK; } // TODO: fill this in later
    virtual sdk_ret_t del_from_db(void) { return sdk::SDK_RET_OK; } // TODO: fill this in later
    // TODO: ignore_dirty is on shaky ground, will try to get rid of it later
    static api_base *find_obj(api_ctxt_t *api_ctxt, bool ignore_dirty) { return NULL; } // TODO: fill this in later

protected:
    /**
     * @brief    process a create/delete/update/get operation on an object
     * @param[in] api_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t process_api(api_ctxt_t *api_ctxt) { return sdk::SDK_RET_OK; }

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
     *            here
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

