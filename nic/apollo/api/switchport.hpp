/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    switchport.hpp
 *
 * @brief   switcthport entry handling
 */

#if !defined (__SWITCHPORT_HPP__)
#define __SWITCHPORT_HPP__

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/include/api/oci_switchport.hpp"

namespace api {

/**
 * @defgroup OCI_SWITCHPORT_ENTRY - switchport entry functionality
 * @ingroup OCI_SWITCHPORT
 * @{
 */

/**
 * @brief    switchport entry
 */
class switchport_entry : public api_base {
public:
    /**
     * @brief    factory method to allocate and initialize switchport entry
     * @param[in] oci_switchport    switchport information
     * @return    new instance of switchport or NULL, in case of error
     */
    static switchport_entry *factory(oci_switchport_t *oci_switchport);

    /**
     * @brief    release all the s/w state associate with the given switchport,
     *           if any, and free the memory
     * @param[in] switchport     switchport to be freed
     * NOTE: h/w entries should have been cleaned up (by calling
     *       impl->cleanup_hw() before calling this
     */
    static void destroy(switchport_entry *switchport);

    /**
     * @brief     initialize switchport entry with the given config
     * @param[in] api_ctxt API context carrying the configuration
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /**
     * @brief    allocate h/w resources for this object
     * @param[in] orig_obj    old version of the unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    program all h/w tables relevant to this object except stage 0
     *           table(s), if any
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t program_config(obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief     free h/w resources used by this object, if any
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t release_resources(void) override;

    /**
     * @brief    cleanup all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     *           and setting invalid bit (if any) in the h/w entries
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t cleanup_config(obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    update all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     * @param[in] orig_obj    old version of the unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t update_config(api_base *orig_obj,
                                    obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    activate the epoch in the dataplane by programming stage 0
     *           tables, if any
     * @param[in] epoch       epoch being activated
     * @param[in] api_op      api operation
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t activate_config(oci_epoch_t epoch, api_op_t api_op,
                                      obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief     add switchport object to the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t add_to_db(void) override;

    /**
     * @brief    del switchport object from the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t del_from_db(void) override;

    /**
     * @brief    this method is called on new object that needs to replace the
     *           old version of the object in the DBs
     * @param[in] orig_obj    old version of the object being swapped out
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t update_db(api_base *orig_obj,
                                obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    initiate delay deletion of this object
     */
    virtual sdk_ret_t delay_delete(void) override;

    ipv4_addr_t ip_addr(void) const { return ip_addr_; }
    mac_addr_t& mac_addr(void) { return mac_addr_; }
    ipv4_addr_t gw_ip_addr(void) const { return gw_ip_addr_; }

private:
    /**< @brief    constructor */
    switchport_entry() {
        impl_ = NULL;
    }

    /**< @brief    destructor */
    ~switchport_entry() {}

private:
    ipv4_addr_t    ip_addr_;       /**< physical IP (aka. MyTEP IP) in substrate */
    mac_addr_t     mac_addr_;      /**< MyTEP mac address */
    ipv4_addr_t    gw_ip_addr_;    /**< IP of default gw in the substrate */
    impl_base      *impl_;         /**< impl object instance */
} __PACK__;

/** @} */    // end of OCI_SWITCHPORT_ENTRY

}    // namespace api

using api::switchport_entry;

#endif    /** __SWITCHPORT_HPP__ */
