/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    impl_base.hpp
 *
 * @brief   base object definition for all impl objects
 */

#if !defined (__IMPL_BASE_HPP__)
#define __IMPL_BASE_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/hal/apollo/framework/obj_base.hpp"
#include "nic/hal/apollo/framework/api.hpp"

namespace api {

/**
 * @brief    base class for all impl objects
 */
class impl_base : public obj_base {
public:
    /**
     * @brief    constructor
     */
    impl_base(){};

    /**
     * @brief    destructor
     */
    ~impl_base(){};

    /**
     * @brief        factory method to instantiate an object
     * @param[in]    api_ctxt API context carrying object related configuration
     */
    static api_base *factory(api_ctxt_t *api_ctxt);

    /**
     * @brief    program all h/w tables relevant to this object except stage 0
     *           table(s), if any and also set the valid bit
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t program_hw(obj_ctxt_t *obj_ctxt) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /**
     * @brief    cleanup all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     *           and setting invalid bit (if any) in the h/w entries
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t cleanup_hw(obj_ctxt_t *obj_ctxt) {
        return sdk::SDK_RET_INVALID_OP;
    }
};

}    // namespace api
 
#endif    /** __IMPL_BASE_HPP__ */
