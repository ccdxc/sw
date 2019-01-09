/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    route_impl.hpp
 *
 * @brief   route table implementation in the p4/hw
 */
#if !defined (__ROUTE_IMPL_HPP__)
#define __ROUTE_IMPL_HPP__

#include "nic/hal/apollo/framework/api.hpp"
#include "nic/hal/apollo/framework/api_base.hpp"
#include "nic/hal/apollo/framework/impl_base.hpp"
#include "nic/hal/apollo/include/api/oci_route.hpp"

namespace impl {

/**
 * @defgroup OCI_ROUTE_TABLE_IMPL - route table functionality
 * @ingroup OCI_ROUTE
 * @{
 */

/**
 * @brief    route table implementation
 */
class route_table_impl : public impl_base {
public:
    /**
     * @brief    factory method to allocate & initialize
     *           route table impl instance
     * @param[in] oci_route_table    route table information
     * @return    new instance of route table or NULL, in case of error
     */
    static route_table_impl *factory(oci_route_table_t *oci_route_table);

    /**
     * @brief    release all the s/w state associated with the given
     *           route table instance, if any, and free the memory
     * @param[in] impl route table impl instance to be freed
     * NOTE: h/w entries should have been cleaned up (by calling
     *       impl->cleanup_hw() before calling this
     */
    static void destroy(route_table_impl *impl);

    /**
     * @brief    allocate/reserve h/w resources for this object
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t reserve_resources(api_base *api_obj) override;

    /**
     * @brief     free h/w resources used by this object, if any
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t release_resources(api_base *api_obj) override;

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
                                  obj_ctxt_t *obj_ctxt) override;

    uint16_t lpm_root_addr(void) { return lpm_root_addr_; }

private:
    /**< @brief    constructor */
    route_table_impl() {
        lpm_root_addr_ = 0xFFFFFFFFFFFFFFFFUL;
    }

    /**< @brief    destructor */
    ~route_table_impl() {}

private:
    /**< P4 datapath specific state */
    mem_addr_t    lpm_root_addr_;      /**< LPM tree's root node address */
} __PACK__;

/** @} */    // end of OCI_ROUTE_TABLE_IMPL

}    // namespace impl

#endif    /** __ROUTE_IMPL_HPP__ */
