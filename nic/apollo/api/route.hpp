/**
 * Copyright (c) 2018 Pensando Systems, Inc.
 *
 * @file    route.hpp
 *
 * @brief   route table handling
 */

#if !defined (__ROUTE_HPP__)
#define __ROUTE_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_route.hpp"

namespace api {

/**
 * @defgroup PDS_ROUTE_TABLE - route table functionality
 * @ingroup PDS_ROUTE
 * @{
 */

// forward declaration
class route_table_state;

/**
 * @brief    route table
 */
class route_table : public api_base {
public:
    /**
     * @brief    factory method to allocate & initialize a route table instance
     * @param[in] spec route table configuration
     * @return    new instance of route table or NULL, in case of error
     */
    static route_table *factory(pds_route_table_spec_t *spec);

    /**
     * @brief    release all the s/w state associate with the given route table,
     *           if any, and free the memory
     * @param[in] rtable     route table to be freed
     * NOTE: h/w entries should have been cleaned up (by calling
     *       impl->cleanup_hw() before calling this
     */
    static void destroy(route_table *table);

    /**
     * @brief     initialize route table instance with the given config
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
     * @brief     free h/w resources used by this object, if any
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t release_resources(void) override;

    /**
     * @brief    program all h/w tables relevant to this object except stage 0
     *           table(s), if any
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t program_create(obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    cleanup all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t cleanup_config(obj_ctxt_t *obj_ctxt) override {
        // there is no need cleanup in either the rollback case or
        // route table delete case, we simply have to free the resources in
        // either case
        return SDK_RET_OK;
    }

    /**
     * @brief    update all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     * @param[in] orig_obj    old version of the unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t program_update(api_base *orig_obj,
                                    obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    activate the epoch in the dataplane by programming stage 0
     *           tables, if any
     * @param[in] epoch       epoch being activated
     * @param[in] api_op      api operation
     * @param[in] orig_obj    orig_obj old/original version of unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      api_base *orig_obj,
                                      obj_ctxt_t *obj_ctxt) override;

    ///\brief read config
    ///\param[out] info Pointer to the info object
    ///\return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_route_table_info_t *info);

    /**
     * @brief     add given route table to the database
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t add_to_db(void) override;

    /**
     * @brief     delete route table from the database
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

    /**< @brief    initiate delay deletion of this object */
    virtual sdk_ret_t delay_delete(void) override;

    /**
     * @brief    compute all the objects depending on this object and add to
     *           framework's dependency list
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t add_deps(obj_ctxt_t *obj_ctxt) override;

    /**< @brief    return stringified key of the object (for debugging) */
    virtual string key2str(void) const override {
        return "route-table-"  + std::to_string(key_.id);
    }

    /**
     * @brief     helper function to get key given route table
     * @param[in] entry    pointer to route table instance
     * @return    pointer to the route table instance's key
     */
    static void *route_table_key_func_get(void *entry) {
        route_table *table = (route_table *)entry;
        return (void *)&(table->key_);
    }

    /// \brief   helper function to get size of key
    /// \return  size of key
    static uint32_t key_size(void) {
        return sizeof(pds_route_table_key_t);
    }

    /// \brief          return the route table key/id
    /// \return         key/id of the route table
    pds_route_table_key_t key(void) const { return key_; }

    /**
     * @brief return address family of this routing table
     * @return IP_AF_IPV4, if routing table is IPv4 or else IP_AF_IPV6
     */
    uint8_t af(void) const { return af_; }

    /**
     * @brief     return impl instance of this route table object
     * @return    impl instance of the route table object
     */
    impl_base *impl(void) { return impl_; }

private:
    /**< @brief    constructor */
    route_table();

    /**< @brief    destructor */
    ~route_table();

    /// \brief      fill the route table sw spec
    /// \param[out] spec specification
    void fill_spec_(pds_route_table_spec_t *spec);

    /**
     * @brief     free h/w resources used by this object, if any
     *            (this API is invoked during object deletes)
     * @return    SDK_RET_OK on success, failure status code on error
     */
    sdk_ret_t nuke_resources_(void);

private:
    pds_route_table_key_t    key_;        /**< route table key */
    uint8_t                  af_;         /**< IP_AF_IPV4 or IP_AF_IPV6 */
    ht_ctxt_t                ht_ctxt_;    /**< hash table context */
    impl_base                *impl_;      /**< impl object instance */

    friend class route_table_state;    // route_table_state is friend
                                       // of route_table
} __PACK__;

/** @} */    // end of PDS_ROUTE_TABLE

}    // namespace api

using api::route_table;

#endif    /** __ROUTE_HPP__ */
