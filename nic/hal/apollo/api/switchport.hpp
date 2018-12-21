/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    switchport.hpp
 *
 * @brief   APIs for switchport implementation
 */

#if !defined (__SWITCHPORT_HPP__)
#define __SWITCHPORT_HPP__

#include "nic/sdk/include/sdk/slab.hpp"
#include "nic/sdk/include/sdk/indexer.hpp"
#include "nic/sdk/include/sdk/ht.hpp"
#include "nic/hal/apollo/framework/api_base.hpp"
#include "nic/hal/apollo/include/api/oci_switchport.hpp"

namespace api {

/**
 * @defgroup OCI_SWITCHPORT_STATE - switchport functionality
 * @ingroup OCI_SWITCHPORT
 * @{
 */

/**
 * @brief    switchport entry
 */
class switchport_entry : public api_base {
public:
    /**< @brief    constructor */
    switchport_entry() {}

    /**< @brief    destructor */
    ~switchport_entry() {}

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

#if 0
    /**
     * @brief     update/override the switchport object with given config
     * @param[in] api_ctxt API context carrying the configuration
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t update_config(api_ctxt_t *api_ctxt) override;
#endif

    /**
     * @brief    program all h/w tables relevant to this object except stage 0
     *           table(s), if any
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t program_config(obj_ctxt_t *obj_ctxt) override;

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
     * @brief     add given switchport to the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t add_to_db(void) override;

    /**
     * @brief    this method is called on new object that needs to replace the
     *           old version of the object in the DBs
     * @param[in] orig_obj    old version of the object being swapped out
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) override;

private:

    /**
     * @brief    allocate h/w resources for this object
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t alloc_resources_(void);

    /**
     * @brief     free h/w resources used by this object, if any
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t free_resources_(void);

private:
    ipv4_addr_t    ip_addr_;       /**< physical IP (aka. MyTEP IP) in substrate */
    mac_addr_t     mac_addr_;      /**< MyTEP mac address */
    ipv4_addr_t    gw_ip_addr_;    /**< IP of default gw in the substrate */
} __PACK__;

/**
 * @brief    state maintained for switchports
 */
class switchport_state : public obj_base {
public:
    /**
     * @brief    constructor
     */
    switchport_state() {}

    /**
     * @brief    destructor
     */
    ~switchport_state() {}

    /**
     * @brief    allocate memory required for switchport object
     * @return pointer to the allocated switchport, NULL if no memory
     */
    switchport_entry *switchport_alloc(void) {
        /**< switchport is singleton, no need to allocate memory dynamically */
        return &switchport_cfg_;
    }

    /**
     * @brief     lookup a switchport in database given the key
     * @param[in] switchport_key switchport key
     */
    switchport_entry *switchport_find(void) {
        return &switchport_cfg_;
    }

private:
    switchport_entry    switchport_cfg_;
};

/** * @} */    // end of OCI_SWITHCPORT_STATE

}    // namespace api

#endif    /** __SWITCHPORT_HPP__ */
