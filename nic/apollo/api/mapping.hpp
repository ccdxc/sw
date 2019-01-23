/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    mapping.hpp
 *
 * @brief   This file deals with mapping functionality
 */

#if !defined (__MAPPING_HPP__)
#define __MAPPING_HPP__

#include "lib/slab/slab.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/include/api/oci_mapping.hpp"

namespace api {

/**
 * @defgroup OCI_MAPPING_ENTRY - mapping functionality
 * @ingroup OCI_MAPPING
 * @{
 */

/**
 * @brief    mapping entry
 */
class mapping_entry : public api_base {
public:
    /**
     * @brief    factory method to allocate and initialize a mapping entry
     * @param[in] oci_mapping    mapping information
     * @return    new instance of mapping or NULL, in case of error
     */
    static mapping_entry *factory(oci_mapping_t *oci_mapping);

    /**
     * @brief    release all the s/w state associate with the given mapping,
     *           if any, and free the memory
     * @param[in] mapping     mapping to be freed
     * NOTE: h/w entries should have been cleaned up (by calling
     *       impl->cleanup_hw() before calling this
     */
    static void destroy(mapping_entry *mapping);

    /**
     * @brief     initialize mapping entry with the given config
     * @param[in] api_ctxt API context carrying the configuration
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /**
     * @brief    program all h/w tables relevant to this object except stage 0
     *           table(s), if any
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t program_config(obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    cleanup all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
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
     * @brief     add given mapping to the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t add_to_db(void) override;

    /**
     * @brief     delete given mapping from the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t del_from_db(void) override;

    /**
     * @brief    this method is called on new object that needs to replace the
     *           old version of the object in the DBs
     * @param[in] orig_obj    old version of the unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t update_db(api_base *orig_obj,
                                obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    initiate delay deletion of this object
     */
    virtual sdk_ret_t delay_delete(void) override;

private:
    /**< @brief    constructor */
    mapping_entry();

    /**< @brief    destructor */
    ~mapping_entry();

    /**
     * @brief    allocate h/w resources for this object
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t reserve_resources_(void);

    /**
     * @brief     free h/w resources used by this object, if any
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t release_resources_(void);

private:
    impl_base        *impl_;      /**< impl object instance */
} __PACK__;


/** @} */    // end of OCI_MAPPING_ENTRY

/**
 * @defgroup OCI_MAPPING_STATE - mapping state functionality
 * @ingroup OCI_MAPPING
 * @{
 */

/**
 * @brief    state maintained for MAPPINGs
 */
class mapping_state : public obj_base {
public:
    /**
     * @brief    constructor
     */
    mapping_state();

    /**
     * @brief    destructor
     */
    ~mapping_state();

    /**
     * @brief    allocate memory required for a mapping
     * @return pointer to the allocated mapping, NULL if no memory
     */
    mapping_entry *mapping_alloc(void);

    /**
     * @brief      free mapping instance back to slab
     * @param[in]  mapping   pointer to the allocated mapping
     */
    void mapping_free(mapping_entry *mapping);

#if 0
    /**
     * @brief     lookup a mapping in database given the key
     * @param[in] mapping_key mapping key
     */
    mapping_entry *mapping_find(oci_mapping_key_t *mapping_key) const;
#endif
    friend void slab_delay_delete_cb(void *timer, uint32_t slab_id, void *elem);

private:
    slab *mapping_slab(void) { return mapping_slab_; }

private:
    slab    *mapping_slab_;    /**< slab for allocating mapping entry */
};

/** @} */    // end of OCI_MAPPING_STATE

}    // namespace api

using api::mapping_entry;
using api::mapping_state;

#endif    /** __MAPPING_HPP__ */
