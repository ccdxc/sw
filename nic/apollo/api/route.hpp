//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// route table handling
///
//----------------------------------------------------------------------------

#ifndef __ROUTE_HPP__
#define __ROUTE_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_stooge.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_route.hpp"

namespace api {

#define PDS_ROUTE_TABLE_UPD_ROUTES           0x1
#define PDS_ROUTE_TABLE_UPD_ROUTE_ADD        0x2
#define PDS_ROUTE_TABLE_UPD_ROUTE_DEL        0x4
#define PDS_ROUTE_TABLE_UPD_ROUTE_UPD        0x8

/// \defgroup PDS_ROUTE_TABLE - route table functionality
/// \ingroup PDS_ROUTE
/// @{

// forward declaration
class route_table_state;

/// \brief route table
class route_table : public api_base {
public:
    /// \brief    factory method to allocate & initialize a route table instance
    /// \param[in] spec route table configuration
    /// \return    new instance of route table or NULL, in case of error
    static route_table *factory(pds_route_table_spec_t *spec);

    /**
     * @brief    release all the s/w state associate with the given route table,
     *           if any, and free the memory
     * @param[in] rtable     route table to be freed
     * NOTE: h/w entries should have been cleaned up (by calling
     *       impl->cleanup_hw() before calling this
     */
    static void destroy(route_table *table);

    /// \brief    clone this object and return cloned object
    /// \param[in]    api_ctxt API context carrying object related configuration
    /// \return       new object instance of current object
    virtual api_base *clone(api_ctxt_t *api_ctxt) override;

    /// \brief    clone this object and return cloned object
    /// \return    new object instance of current object
    /// \remark  this version of clone API is needed when we need to clone
    ///          the object (and its impl) when we don't have a spec
    virtual api_base *clone(void) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] table    route table to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(route_table *table);

    /**
     * @brief    allocate h/w resources for this object
     * @param[in] orig_obj    old version of the unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        api_obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief     free h/w resources used by this object, if any
     * @return    SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t release_resources(void) override;

    /**
     * @brief     initialize route table instance with the given config
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
    virtual sdk_ret_t program_create(api_obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    cleanup all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t cleanup_config(api_obj_ctxt_t *obj_ctxt) override {
        // there is no need cleanup in either the rollback case or
        // route table delete case, we simply have to free the resources in
        // either case
        return SDK_RET_OK;
    }

    /**
     * @brief    compute all the objects depending on this object and add to
     *           framework's dependency list
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t add_deps(api_obj_ctxt_t *obj_ctxt) override;

    /// \brief    compute the object diff during update operation compare the
    ///           attributes of the object on which this API is invoked and the
    ///           attrs provided in the update API call passed in the object
    ///           context (as cloned object + api_params) and compute the upd
    ///           bitmap (and stash in the object context for later use)
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t compute_update(api_obj_ctxt_t *obj_ctxt) override;

    /**
     * @brief    update all h/w tables relevant to this object except stage 0
     *           table(s), if any, by updating packed entries with latest epoch#
     * @param[in] orig_obj    old version of the unmodified object
     * @param[in] obj_ctxt    transient state associated with this API
     * @return   SDK_RET_OK on success, failure status code on error
     */
    virtual sdk_ret_t program_update(api_base *orig_obj,
                                    api_obj_ctxt_t *obj_ctxt) override;

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
                                      api_obj_ctxt_t *obj_ctxt) override;

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
                                api_obj_ctxt_t *obj_ctxt) override;

    /**< @brief    initiate delay deletion of this object */
    virtual sdk_ret_t delay_delete(void) override;

    /**< @brief    return stringified key of the object (for debugging) */
    virtual string key2str(void) const override {
            return "route-table-"  + std::string(key_.str());
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

    /// \brief          return the route table key/id
    /// \return         key/id of the route table
    const pds_obj_key_t key(void) const { return key_; }

    /// \brief return address family of this routing table
    /// \return IP_AF_IPV4, if routing table is IPv4 or else IP_AF_IPV6
    uint8_t af(void) const { return af_; }

    /// \brief return the number of routes in the route table
    /// \return number of routes in the route table
    uint32_t num_routes(void) const { return num_routes_; }

    /// \brief     return impl instance of this route table object
    /// \return    impl instance of the route table object
    impl_base *impl(void) { return impl_; }

    /**
     * @brief     return num routes in this route table
     * @return    num of routes
     */
     uint32_t num_routes(void) { return num_routes_; }

private:
    /// \brief    constructor
    route_table();

    /// \brief    destructor
    ~route_table();

    /// \brief      fill the route table sw spec
    /// \param[out] spec specification
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t fill_spec_(pds_route_table_spec_t *spec);

    /// \brief     initialize route table instance with the given route table's
    ///            config
    /// \param[in] rtable    route table from which the config is copied from
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t init_config_(route_table *rtable);

    /// \brief     free h/w resources used by this object, if any
    ///            (this API is invoked during object deletes)
    /// \return    SDK_RET_OK on success, failure status code on error
    sdk_ret_t nuke_resources_(void);

private:
    pds_obj_key_t    key_;        ///< route table key
    uint8_t          af_;         ///< IP_AF_IPV4 or IP_AF_IPV6
    uint32_t         num_routes_; ///< number of routes in this table
    ht_ctxt_t        ht_ctxt_;    ///< hash table context
    impl_base        *impl_;      ///< impl object instance

    // route_table_state class is friend of route_table
    friend class route_table_state;
} __PACK__;

/// \brief route class
class route : public api_stooge {
public:
    /// \brief    factory method to allocate & initialize a route instance
    /// \param[in] spec route configuration
    /// \return    new instance of route or NULL, in case of error
    static route *factory(pds_route_spec_t *spec);

    /// \brief    release all the s/w state associate with the given route,
    ///           if any, and free the memory
    ///@param[in] rt route to be freed
    static void destroy(route *rt);

    /// \brief    clone this object and return cloned object
    /// \param[in]    api_ctxt API context carrying object related configuration
    /// \return       new object instance of current object
    virtual api_base *clone(api_ctxt_t *api_ctxt) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] rt route to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(route *route);

    /// \brief    build object given its key from the s/w and/or h/w state we
    ///           have and return an instance of the object (this is useful for
    ///           stateless objects to be operated on by framework during DELETE
    ///           or UPDATE operations)
    /// \param[in] key    key of object instance of interest
    /// \return    route instance corresponding to the key or NULL if
    ///            entry is not found
    static route *build(pds_route_key_t *key);

    /// \brief    free a stateless entry's temporary s/w only resources like
    ///           memory etc., for a stateless entry calling destroy() will
    ///           remove resources from h/w, which can't be done during ADD/UPD
    ///           etc. operations esp. when object is constructed on the fly
    /// \param[in] route    route to be freed
    static void soft_delete(route *route);

    /// \brief     initialize route instance with the given config
    /// \param[in] api_ctxt API context carrying the configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /// \brief compute all the objects depending on this object and add to
    ///        framework's dependency list
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_deps(api_obj_ctxt_t *obj_ctxt) override;

    ///\brief read config
    ///\param[out] info pointer to the info object
    ///\return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_route_info_t *info);

    /// \brief     add given route to the database
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_to_db(void) override;

     /// \brief     delete route from the database
     /// \@return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t del_from_db(void) override;

    /// \brief    this method is called on new object that needs to replace the
    ///           old version of the object in the DBs
    /// \param[in] orig_obj    old version of the unmodified object
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_db(api_base *orig_obj,
                                api_obj_ctxt_t *obj_ctxt) override;

    /// \brief    initiate delay deletion of this object
    virtual sdk_ret_t delay_delete(void) override;

    /// \brief    return stringified key of the object (for debugging)
    virtual string key2str(void) const override {
            return "route-"  + std::string(key_.route_table_id.str()) + "/" +
                       std::string(key_.route_id.str());
    }

    /// \brief     helper function to get key given route
    /// \param[in] entry    pointer to route instance
    /// \return    pointer to the route instance's key
    static void *route_key_func_get(void *entry) {
        route *rt = (route *)entry;
        return (void *)&(rt->key_);
    }

    /// \brief     return the route key/id
    /// \return    key/id of the route
    const pds_route_key_t key(void) const { return key_; }

private:
    /// \brief    constructor
    route() {
        ht_ctxt_.reset();
    }

    /// \brief    destructor
    ~route() {}

private:
    /// route key
    pds_route_key_t key_;

    ///< hash table context
    ht_ctxt_t ht_ctxt_;
    // route_state class is friend of route class
    friend class route_state;
};

/// \@}

}    // namespace api

using api::route_table;
using api::route;

#endif    // __ROUTE_HPP__
