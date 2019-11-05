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

    /// \brief     initialize a nexthop group with the given config
    /// \param[in] api_ctxt API context carrying the configuration
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /// \brief     allocate h/w resources for this object
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        obj_ctxt_t *obj_ctxt) override;

    /// \brief  free h/w resources used by this object, if any
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(void) override;

    /// \brief     program all h/w tables relevant to this object except
    ///            stage 0 table(s), if any
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_config(obj_ctxt_t *obj_ctxt) override;

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
    virtual sdk_ret_t cleanup_config(obj_ctxt_t *obj_ctxt) override;

    /// \brief     update all h/w tables relevant to this object except
    ///            stage 0 table(s), if any, by updating packed entries
    ///            with latest epoch#
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_config(api_base *orig_obj,
                                    obj_ctxt_t *obj_ctxt) override;

    /// \brief     activate the epoch in the dataplane by programming
    ///            stage 0 tables, if any
    /// \param[in] epoch    epoch being activated
    /// \param[in] api_op   api operation
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      obj_ctxt_t *obj_ctxt) override;

    /// \brief re-activate config in the hardware stage 0 tables relevant to
    ///        this object, if any, this reactivation must be based on existing
    ///        state and any of the state present in the dirty object list
    ///        (like clone objects etc.) only and not directly on db objects
    /// \param[in] api_op API operation
    /// \return #SDK_RET_OK on success, failure status code on error
    /// NOTE: this method is called when an object is in the dependent/puppet
    ///       object list
    virtual sdk_ret_t reactivate_config(pds_epoch_t epoch,
                                        api_op_t api_op) override;

    ///\brief read config from hardware
    ///\param[in] key    pointer to the key object
    ///\param[out] info  pointer to the info object
    ///\return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_nexthop_group_key_t *key,
                   pds_nexthop_group_info_t *info);

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
                                obj_ctxt_t *obj_ctxt) override;

    /// \brief initiate delay deletion of this object
    virtual sdk_ret_t delay_delete(void) override;

    /// \brief return stringified key of the object (for debugging)
    virtual string key2str(void) const override {
        return "nh-group-" + std::to_string(key_.id);
    }

    /// \brief     helper function to get key given nexthop group
    /// \param[in] entry pointer to nexthop group instance
    /// \return    pointer to the nexthop group instance's key
    static void *nexthop_group_key_func_get(void *entry) {
        nexthop_group *nh_group = (nexthop_group *)entry;
        return (void *)&(nh_group->key_);
    }

    /// \brief   helper function to get size of key
    /// \return  size of key
    static uint32_t key_size(void) {
        return sizeof(pds_nexthop_group_key_t);
    }

    /// \brief     return the key of the nexthop group
    /// \return    nexthop group key
    pds_nexthop_group_key_t key(void) const { return key_; }

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

    /// \brief  free h/w resources used by this object, if any
    ///         (this API is invoked during object deletes)
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t nuke_resources_(void);

private:
    pds_nexthop_group_key_t key_;        ///< nexthop group key
    pds_nexthop_group_type_t type_;      ///< nexthop group type
    uint8_t num_nexthops_;              ///< number of nexthops in this group
    ht_ctxt_t ht_ctxt_;                  ///< hash table context
    impl_base *impl_;                    ///< impl object instance
    friend class nexthop_group_state;    ///< a friend of nexthop_group
} __PACK__;

/// \@}

}    // namespace api

using api::nexthop_group;

#endif    // __API_NEXTHOP_GROUP_HPP__
