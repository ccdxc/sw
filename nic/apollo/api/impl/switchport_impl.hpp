/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    switchport_impl.hpp
 *
 * @brief   SWITCHPORT implementation in the p4/hw
 */
#if !defined (__SWITCHPORT_IMPL_HPP__)
#define __SWITCHPORT_IMPL_HPP__

#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/include/api/oci_switchport.hpp"

namespace impl {

/**
 * @defgroup OCI_SWITCHPORT_IMPL - switchport functionality
 * @ingroup OCI_SWITCHPORT
 * @{
 */

/**
 * @brief    SWITCHPORT implementation
 */
class switchport_impl : public impl_base {
public:
    /**
     * @brief    factory method to allocate & initialize switchport impl
     *           instance
     * @param[in] oci_switchport    switchport information
     * @return    new instance of switchport or NULL, in case of error
     */
    static switchport_impl *factory(oci_switchport_t *oci_switchport);

    /**
     * @brief    release all the s/w state associated with the given switchport,
     *           if any, and free the memory
     * @param[in] switchport     switchport to be freed
     * NOTE: h/w entries should have been cleaned up (by calling
     *       impl->cleanup_hw() before calling this
     */
    static void destroy(switchport_impl *impl);

    /**< @brief    constructor */
    switchport_impl() {}

    /**< @brief    destructor */
    ~switchport_impl() {}

    /**
     * @brief    program all h/w tables relevant to this object except stage 0
     *           table(s), if any
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t program_hw(api_base *api_obj,
                                 obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    cleanup all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t cleanup_hw(api_base *api_obj,
                                 obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    update all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     * @param[in] orig_obj    old version of the unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t update_hw(api_base *curr_obj, api_base *prev_obj,
                                obj_ctxt_t *obj_ctxt) override;
};

/** @} */    // end of OCI_SWITCHPORT_IMPL

}    // namespace impl

#endif    /** __SWITCHPORT_IMPL_HPP__ */
