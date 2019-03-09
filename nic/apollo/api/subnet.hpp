//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file handles subnet entry handling
///
//----------------------------------------------------------------------------

#if !defined(__API_SUBNET_HPP__)
#define __API_SUBNET_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/include/api/pds_subnet.hpp"

namespace api {

/// \defgroup PDS_SUBNET_ENTRY - subnet entry functionality
/// \ingroup PDS_SUBNET
/// @{

/// \brief    subnet entry
class subnet_entry : public api_base {
public:
    /// \brief          factory method to allocate and initialize a subnet entry
    /// \param[in]      pds_subnet    subnet information
    /// \return         new instance of subnet or NULL, in case of error
    static subnet_entry *factory(pds_subnet_spec_t *pds_subnet);

    /// \brief          release all the s/w state associate with the given
    ///                 subnet, if any, and free the memory
    /// \param[in]      subnet     subnet to be freed
    /// \NOTE: h/w entries should have been cleaned up (by calling
    ///        impl->cleanup_hw() before calling this
    static void destroy(subnet_entry *subnet);

    /// \brief          initialize subnet entry with the given config
    /// \param[in]      api_ctxt API context carrying the configuration
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /// \brief          allocate h/w resources for this object
    /// \param[in]      orig_obj    old version of the unmodified object
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        obj_ctxt_t *obj_ctxt) override;

    /// \brief          program all h/w tables relevant to this object except
    ///                 stage 0 table(s), if any
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_config(obj_ctxt_t *obj_ctxt) override;

    /// \brief          free h/w resources used by this object, if any
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(void) override;

    /// \brief          cleanup all h/w tables relevant to this object except
    ///                 stage 0 table(s), if any, by updating packed entries
    ///                 with latest epoch#
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_config(obj_ctxt_t *obj_ctxt) override;

    /// \brief          update all h/w tables relevant to this object except
    ///                 stage 0 table(s), if any, by updating packed entries
    ///                 with latest epoch#
    /// \param[in]      orig_obj    old version of the unmodified object
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_config(api_base *orig_obj,
                                    obj_ctxt_t *obj_ctxt) override;

    /// \brief          activate the epoch in the dataplane by programming
    ///                 stage 0 tables, if any
    /// \param[in]      epoch       epoch being activated
    /// \param[in]      api_op      api operation
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      obj_ctxt_t *obj_ctxt) override;

    /// \brief          add given subnet to the database
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_to_db(void) override;

    /// \brief          delete given subnet from the database
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t del_from_db(void) override;

    /// \brief          this method is called on new object that needs to
    ///                 replace theold version of the object in the DBs
    /// \param[in]      orig_obj    old version of the object being swapped out
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_db(api_base *orig_obj,
                                obj_ctxt_t *obj_ctxt) override;

    /// \brief          initiate delay deletion of this object
    virtual sdk_ret_t delay_delete(void) override;

    /// \brief          return stringified key of the object (for debugging)
    virtual string key2str(void) const override {
        return "subnet-" + std::to_string(key_.id);
    }

    /// \brief          helper function to get key given subnet entry
    /// \param[in]      entry    pointer to subnet instance
    /// \return         pointer to the subnet instance's key
    static void *subnet_key_func_get(void *entry) {
        subnet_entry *subnet = (subnet_entry *)entry;
        return (void *)&(subnet->key_);
    }

    /// \brief          helper function to compute hash value for given subnet id
    /// \param[in]      key        subnet's key
    /// \param[in]      ht_size    hash table size
    /// \return         hash value
    static uint32_t subnet_hash_func_compute(void *key, uint32_t ht_size) {
        return hash_algo::fnv_hash(key, sizeof(pds_subnet_key_t)) % ht_size;
    }

    /// \brief          helper function to compare two subnet keys
    /// \param[in]      key1        pointer to subnet's key
    /// \param[in]      key2        pointer to subnet's key
    /// \return         0 if keys are same or else non-zero value
    static bool subnet_key_func_compare(void *key1, void *key2) {
        SDK_ASSERT((key1 != NULL) && (key2 != NULL));
        if (!memcmp(key1, key2, sizeof(pds_subnet_key_t)))
            return true;

        return false;
    }

    mac_addr_t &vr_mac(void) { return vr_mac_; }

    /// \brief          return h/w index for this subnet
    /// \return         h/w table index for this subnet
    uint16_t hw_id(void) const { return hw_id_; }
    pds_route_table_key_t v4_route_table(void) const { return v4_route_table_; }
    pds_route_table_key_t v6_route_table(void) const { return v6_route_table_; }
    pds_policy_key_t ing_v4_policy(void) const { return ing_v4_policy_; }
    pds_policy_key_t ing_v6_policy(void) const { return ing_v6_policy_; }
    pds_policy_key_t egr_v4_policy(void) const { return egr_v4_policy_; }
    pds_policy_key_t egr_v6_policy(void) const { return egr_v6_policy_; }

private:
    /// \brief constructor
    subnet_entry();

    /// \brief destructor
    ~subnet_entry();

private:
    pds_subnet_key_t key_;                    ///< subnet Key
    pds_route_table_key_t v4_route_table_;    ///< route table id
    pds_route_table_key_t v6_route_table_;    ///< route table id
    pds_policy_key_t ing_v4_policy_;          ///< ingress IPv4 policy id
    pds_policy_key_t ing_v6_policy_;          ///< ingress IPv6 policy id
    pds_policy_key_t egr_v4_policy_;          ///< ingress IPv4 policy id
    pds_policy_key_t egr_v6_policy_;          ///< ingress IPv6 policy id
    mac_addr_t vr_mac_;                       ///< virtual router MAC
    ht_ctxt_t ht_ctxt_;                       ///< hash table context

    // P4 datapath specific state
    uint16_t hw_id_;                 ///< hardware id
    // TODO Statistics for subnet, there arent any as of now
} __PACK__;

/// \@}    // end of PDS_SUBNET_ENTRY

}    // namespace api

using api::subnet_entry;

#endif    // __API_SUBNET_HPP__
