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
#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/obj_base.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl.hpp"
#include "nic/apollo/framework/asic_impl_base.hpp"
#include "nic/apollo/framework/pipeline_impl_base.hpp"
#include "nic/sdk/asic/asic.hpp"

namespace impl {

/**
 * @brief    base class for all impl objects
 */
class impl_base : public obj_base {
public:
    /**< @brief    constructor */
    impl_base() {}

    /**< @brief    destructor */
    ~impl_base() {}

    /**
     * @brief    one time init function that must be called during bring up
     * @param[in]    params      initialization parameters passed by application
     * @param[in]    asic_cfg    asic configuration parameters
     * @return       SDK_RET_OK on success, failure status code on error
     */
    static sdk_ret_t init(oci_init_params_t *params, asic_cfg_t *asic_cfg);

    /**
     * @brief    dump all the debug information to given file
     * @param[in] fp    file handle
     */
    static void debug_dump(FILE *fp);

    /**
     * @brief        factory method to instantiate an impl object
     * @param[in]    impl    object id
     * @param[in]    args    args (not interpreted by this class)
     */
    static impl_base *factory(impl_obj_id_t obj_id, void *args);

    /**
     * @brief    release all the resources associated with this object
     *           and free the memory
     * @param[in] impl_obj    impl instance to be freed
     */
    static void destroy(impl_obj_id_t obj_id, impl_base *impl_obj);

    /**
     * @brief    allocate/reserve h/w resources for this object
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t reserve_resources(api_base *api_obj) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /**
     * @brief     free h/w resources used by this object, if any
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t release_resources(api_base *api_obj) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /**
     * @brief    program all h/w tables relevant to this object except stage 0
     *           table(s), if any
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t program_hw(api_base *api_obj,
                                 obj_ctxt_t *obj_ctxt) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /**
     * @brief    cleanup all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t cleanup_hw(api_base *api_obj,
                                 obj_ctxt_t *obj_ctxt) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /**
     * @brief    update all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     * @param[in] orig_obj    old version of the unmodified object
     * @param[in] curr_obj    cloned and updated version of the object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t update_hw(api_base *orig_obj, api_base *curr_obj,
                                obj_ctxt_t *obj_ctxt) {
        return sdk::SDK_RET_INVALID_OP;
    }

    /**
     * @brief    activate the epoch in the dataplane by programming stage 0
     *           tables, if any
     * @param[in] epoch       epoch being activated
     * @param[in] api_op      api operation
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t activate_hw(api_base *api_obj,
                                  oci_epoch_t epoch,
                                  api_op_t api_op,
                                  obj_ctxt_t *obj_ctxt) {
        return sdk::SDK_RET_INVALID_OP;
    }

private:
    static asic_impl_base        *asic_impl_;
    static pipeline_impl_base    *pipeline_impl_;
};

}    // namespace impl

using impl::impl_base;
 
#endif    /** __IMPL_BASE_HPP__ */
