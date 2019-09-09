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
        return "nh-group-" + std::to_string(key_);
    }

    /// \brief     helper function to get key given nexthop group
    /// \param[in] entry pointer to nexthop group instance
    /// \return    pointer to the nexthop group instance's key
    static void *nexthop_group_key_func_get(void *entry) {
        nexthop_group *nh_group = (nexthop_group *)entry;
        return (void *)&(nh_group->key_);
    }

    /// \brief     helper function to compute hash value for given
    ///            nexthop group's key
    /// \param[in] key     nexthop group 's key
    /// \param[in] ht_size hash table size
    /// \return    hash value
    static uint32_t nexthop_group_hash_func_compute(void *key,
                                                    uint32_t ht_size) {
        return hash_algo::fnv_hash(key,
                                   sizeof(pds_nexthop_group_key_t)) % ht_size;
    }

    /// \brief     helper function to compare two nexthop group's keys
    /// \param[in] key1 pointer to nexthop group's key
    /// \param[in] key2 pointer to nexthop group's key
    /// \return    0 if keys are same or else non-zero value
    static bool nexthop_group_key_func_compare(void *key1, void *key2) {
        SDK_ASSERT((key1 != NULL) && (key2 != NULL));
        if (!memcmp(key1, key2, sizeof(pds_nexthop_group_key_t))) {
            return true;
        }
        return false;
    }

    /// \brief     return the type of the nexthop group
    /// \return    type of the nexthop group
    pds_nexthop_group_type_t type(void) const { return type_; }

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
    pds_nexthop_group_key_t key_;             ///< nexthop group key
    pds_nexthop_group_type_t type_;           ///< nexthop group type
    ht_ctxt_t ht_ctxt_;                       ///< hash table context
    impl_base *impl_;                         ///< impl object instance

    friend class nexthop_group_state;         ///< a friend of nexthop_group
} __PACK__;

/// \@}

}    // namespace api

using api::nexthop_group;

#endif    // __API_NEXTHOP_GROUP_HPP__
