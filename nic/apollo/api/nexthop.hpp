//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// nexthop handling
///
//----------------------------------------------------------------------------

#ifndef __API_NEXTHOP_HPP__
#define __API_NEXTHOP_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"

namespace api {

// forward declaration
class nexthop_state;

/// \defgroup PDS_NEXTHOP_ENTRY - nexthop functionality
/// \ingroup PDS_NEXTHOP
/// @{

/// \brief nexthop entry
class nexthop : public api_base {
public:
    /// \brief     factory method to allocate and initialize a nexthop entry
    /// \param[in] spec nexthop specification
    /// \return    new instance of nexthop or NULL, in case of error
    static nexthop *factory(pds_nexthop_spec_t *spec);

    /// \brief     release all the s/w state associate with the given nexthop,
    ///            if any, and free the memory
    /// \param[in] nh    nexthop to be freed
    static void destroy(nexthop *nh);

    /// \brief    clone this object and return cloned object
    /// \param[in]    api_ctxt API context carrying object related configuration
    /// \return       new object instance of current object
    virtual api_base *clone(api_ctxt_t *api_ctxt) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] nh    nexthop to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(nexthop *nh);

    /// \brief     initialize a nexthop entry with the given config
    /// \param[in] api_ctxt API context carrying the configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /// \brief     allocate h/w resources for this object
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        api_obj_ctxt_t *obj_ctxt) override;

    /// \brief  free h/w resources used by this object, if any
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(void) override;

    /// \brief     program all h/w tables relevant to this object except
    ///            stage 0 table(s), if any
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_create(api_obj_ctxt_t *obj_ctxt) override;

    /// \brief          reprogram all h/w tables relevant to this object and
    ///                 dependent on other objects except stage 0 table(s),
    ///                 if any
    /// \param[in] api_op    API operation
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reprogram_config(api_op_t api_op) override;

    /// \brief     cleanup all h/w tables relevant to this object except
    ///            stage 0 table(s), if any, by updating packed entries
    ///            with latest epoch#
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_config(api_obj_ctxt_t *obj_ctxt) override;

    /// \brief    compute the object diff during update operation compare the
    ///           attributes of the object on which this API is invoked and the
    ///           attrs provided in the update API call passed in the object
    ///           context (as cloned object + api_params) and compute the upd
    ///           bitmap (and stash in the object context for later use)
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t compute_update(api_obj_ctxt_t *obj_ctxt) override;

    /// \brief          add all objects that may be affected if this object is
    ///                 updated to framework's object dependency list
    /// \param[in]      obj_ctxt    transient state associated with this API
    ///                             processing
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_deps(api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     update all h/w tables relevant to this object except
    ///            stage 0 table(s), if any, by updating packed entries
    ///            with latest epoch#
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_update(api_base *orig_obj,
                                     api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     activate the epoch in the dataplane by programming
    ///            stage 0 tables, if any
    /// \param[in] epoch    epoch being activated
    /// \param[in] api_op   api operation
    /// \param[in]      orig_obj old/original version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      api_base *orig_obj,
                                      api_obj_ctxt_t *obj_ctxt) override;

    /// \brief re-activate config in the hardware stage 0 tables relevant to
    ///        this object, if any, this reactivation must be based on existing
    ///        state and any of the state present in the dirty object list
    ///        (like clone objects etc.) only and not directly on db objects
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    /// NOTE: this method is called when an object is in the dependent/puppet
    ///       object list
    virtual sdk_ret_t reactivate_config(pds_epoch_t epoch,
                                        api_obj_ctxt_t *obj_ctxt) override;

    /// \brief  add given nexthop to the database
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_to_db(void) override;

    /// \brief  delete given nexthop from the database
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t del_from_db(void) override;

    /// \brief     this method is called on new object that needs to
    ///            replace the old version of the object in the DBs
    /// \param[in] orig_obj old version of the object being swapped out
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_db(api_base *orig_obj,
                                api_obj_ctxt_t *obj_ctxt) override;

    /// \brief initiate delay deletion of this object
    virtual sdk_ret_t delay_delete(void) override;

    /// \brief return stringified key of the object (for debugging)
    virtual string key2str(void) const override {
        return "nh-" + std::to_string(key_.id);
    }

    /// \brief     helper function to get key given nexthop entry
    /// \param[in] entry pointer to nexthop instance
    /// \return    pointer to the nexthop instance's key
    static void *nexthop_key_func_get(void *entry) {
        nexthop *nh = (nexthop *)entry;
        return (void *)&(nh->key_);
    }

    ///\brief      read config
    ///\param[out] info pointer to the info object
    ///\return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_nexthop_info_t *info);

    /// \brief     return key/id of the nexthop
    /// \return    key/id of the nexthop
    pds_nexthop_key_t key(void) const { return key_; }

    /// \brief     return the type of the nexthop
    /// \return    type of the nexthop
    pds_nh_type_t type(void) const { return type_; }

    /// \brief     return impl instance of this nexthop object
    /// \return    impl instance of the nexthop object
    impl_base *impl(void) { return impl_; }

private:
    /// \brief constructor
    nexthop();

    /// \brief destructor
    ~nexthop();

    /// \brief      fill the nexthop sw spec
    /// \param[out] spec specification
    void fill_spec_(pds_nexthop_spec_t *spec);

    /// \brief  free h/w resources used by this object, if any
    ///         (this API is invoked during object deletes)
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t nuke_resources_(void);

private:
    pds_nexthop_key_t key_;    ///< nexthop key
    pds_nh_type_t type_;       ///< nexthop type
    ht_ctxt_t ht_ctxt_;        ///< hash table context
    impl_base *impl_;          ///< impl object instance

    friend class nexthop_state;    ///< a friend of nexthop
} __PACK__;

/// \@}

}    // namespace api

using api::nexthop;

#endif    // __API_NEXTHOP_HPP__
