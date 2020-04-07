//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// nexthop group handling
///
//----------------------------------------------------------------------------

#ifndef __API_NEXTHOP_GROUP_HPP__
#define __API_NEXTHOP_GROUP_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"

namespace api {

// forward declaration
class nexthop_group_state;

/// \defgroup PDS_NEXTHOP_GROUP - nexthop group functionality
/// \ingroup PDS_NEXTHOP
/// @{

/// \brief nexthop group
class nexthop_group : public api_base {
public:
    /// \brief     factory method to allocate and initialize a nexthop group
    /// \param[in] spec nexthop group specification
    /// \return    new instance of nexthop group or NULL, in case of error
    static nexthop_group *factory(pds_nexthop_group_spec_t *spec);

    /// \brief     release all the s/w state associate with the given nexthop
    ///            group, if any, and free the memory
    /// \param[in] nh_group    nexthop group to be freed
    static void destroy(nexthop_group *nh_group);

    /// \brief    clone this object and return cloned object
    /// \param[in]    api_ctxt API context carrying object related configuration
    /// \return       new object instance of current object
    virtual api_base *clone(api_ctxt_t *api_ctxt) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] nh_group    nexthop group to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(nexthop_group *nh_group);

    /// \brief    stash this object into persistent storage
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t backup(void) override;

    /// \brief     allocate h/w resources for this object
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        api_obj_ctxt_t *obj_ctxt) override;

    /// \brief  free h/w resources used by this object, if any
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(void) override;

    /// \brief     initialize a nexthop group with the given config
    /// \param[in] api_ctxt API context carrying the configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /// \brief     program all h/w tables relevant to this object except
    ///            stage 0 table(s), if any
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_create(api_obj_ctxt_t *obj_ctxt) override;

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

    /// \brief        add all objects that may be affected if this object is
    ///               updated to framework's object dependency list
    /// \param[in]    obj_ctxt    transient state associated with this API
    ///                           processing
    /// \return       SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_deps(api_obj_ctxt_t *obj_ctxt) override {
        // no other objects are effected if nexthop group is modified
        return SDK_RET_OK;
    }

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
    /// \param[in] orig_obj old/original version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      api_base *orig_obj,
                                      api_obj_ctxt_t *obj_ctxt) override;

    /// \brief     reprogram all h/w tables relevant to this object and
    ///            dependent on other objects except stage 0 table(s),if any
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reprogram_config(api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_INVALID_OP;
    }

    /// \brief re-activate config in the hardware stage 0 tables relevant to
    ///        this object, if any, this reactivation must be based on existing
    ///        state and any of the state present in the dirty object list
    ///        (like clone objects etc.) only and not directly on db objects
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    /// NOTE: this method is called when an object is in the dependent/puppet
    ///       object list
    virtual sdk_ret_t reactivate_config(pds_epoch_t epoch,
                                        api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_INVALID_OP;
    }

    /// \brief  add given nexthop group to the database
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_to_db(void) override;

    /// \brief  delete given nexthop group from the database
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
        return "nh-group-" + std::string(key_.str());
    }

    /// \brief     helper function to get key given nexthop group
    /// \param[in] entry pointer to nexthop group instance
    /// \return    pointer to the nexthop group instance's key
    static void *nexthop_group_key_func_get(void *entry) {
        nexthop_group *nh_group = (nexthop_group *)entry;
        return (void *)&(nh_group->key_);
    }

    ///\brief      read config
    ///\param[out] info pointer to the info object
    ///\return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_nexthop_group_info_t *info);

    /// \brief     return the key of the nexthop group
    /// \return    nexthop group key
    pds_obj_key_t key(void) const { return key_; }

    /// \brief     return the type of the nexthop group
    /// \return    nexthop group type
    pds_nexthop_group_type_t type(void) const { return type_; }

    /// \brief     return the number of nexthops in this group
    /// \return    number of nexthops of this group
    uint8_t num_nexthops(void) const { return num_nexthops_; }

    /// \brief     return impl instance of this nexthop group object
    /// \return    impl instance of the nexthop group object
    impl_base *impl(void) { return impl_; }

private:
    /// \brief constructor
    nexthop_group();

    /// \brief destructor
    ~nexthop_group();

    /// \brief      fill the nexthop group sw spec
    /// \param[out] spec specification
    void fill_spec_(pds_nexthop_group_spec_t *spec);

    /// \brief  free h/w resources used by this object, if any
    ///         (this API is invoked during object deletes)
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t nuke_resources_(void);

private:
    pds_obj_key_t key_;                  ///< nexthop group key
    pds_nexthop_group_type_t type_;      ///< nexthop group type
    uint8_t num_nexthops_;               ///< number of nexthops in this group

    ///< operational state
    ht_ctxt_t ht_ctxt_;                  ///< hash table context
    impl_base *impl_;                    ///< impl object instance
    friend class nexthop_group_state;    ///< a friend of nexthop_group
} __PACK__;

/// \@}

}    // namespace api

using api::nexthop_group;

#endif    // __API_NEXTHOP_GROUP_HPP__
