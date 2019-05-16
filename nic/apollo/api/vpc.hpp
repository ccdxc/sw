//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vpc entry handling
///
//----------------------------------------------------------------------------

#ifndef __API_VPC_HPP__
#define __API_VPC_HPP__

#include "nic/sdk/lib/ht/ht.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"

namespace api {

// forward declaration
class vpc_state;

/// \defgroup PDS_VPC_ENTRY - vpc entry functionality
/// \ingroup PDS_VPC
/// @{

/// \brief    vpc entry
class vpc_entry : public api_base {
public:
    /// \brief          factory method to allocate and initialize a vpc entry
    /// \param[in]      spec    vpc information
    /// \return         new instance of vpc or NULL, in case of error
    static vpc_entry *factory(pds_vpc_spec_t *spec);

    /// \brief          release all the s/w state associate with the given vpc,
    ///                 if any, and free the memory
    /// \param[in]      vpc     vpc to be freed
    /// \NOTE: h/w entries should have been cleaned up (by calling
    ///        impl->cleanup_hw() before calling this
    static void destroy(vpc_entry *vpc);

    /// \brief          initialize vpc entry with the given config
    /// \param[in]      api_ctxt API context carrying the configuration
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t init_config(api_ctxt_t *api_ctxt) override;

    /// \brief          allocate h/w resources for this object
    /// \param[in]      orig_obj    old version of the unmodified object
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        obj_ctxt_t *obj_ctxt) override;

    /// \brief          free h/w resources used by this object, if any
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(void) override;

    /// \brief          program all h/w tables relevant to this object except
    ///                 stage 0 table(s), if any
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_config(obj_ctxt_t *obj_ctxt) override {
        // no programming is done for this object, hence this is a no-op
        return SDK_RET_OK;
    }

    /// \brief          cleanup all h/w tables relevant to this object except
    ///                 stage 0 table(s), if any, by updating packed entries
    ///                 with latest epoch#
    /// \param[in]      obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_config(obj_ctxt_t *obj_ctxt) override {
        // no programming is done for this object, hence this is a no-op
        return SDK_RET_OK;
    }

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
    /// \param          obj_ctxt    transient state associated with this API
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      obj_ctxt_t *obj_ctxt) override;

    /// \brief          add given vpc to the database
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_to_db(void) override;

    /// \brief          delete given vpc from the database
    ///
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
        return "vpc-" + std::to_string(key_.id);
    }

    /// \brief          helper function to get key given vpc entry
    /// \param          entry    pointer to vpc instance
    /// \return         pointer to the vpc instance's key
    static void *vpc_key_func_get(void *entry) {
        vpc_entry *vpc = (vpc_entry *)entry;
        return (void *)&(vpc->key_);
    }

    /// \brief          helper function to compute hash value for given vpc id
    /// \param[in]      key        vpc's key
    /// \param[in]      ht_size    hash table size
    /// \return         hash value
    static uint32_t vpc_hash_func_compute(void *key, uint32_t ht_size) {
        return hash_algo::fnv_hash(key, sizeof(pds_vpc_key_t)) % ht_size;
    }

    /// \brief          helper function to compare two vpc keys
    /// \param[in]      key1        pointer to vpc's key
    /// \param[in]      key2        pointer to vpc's key
    /// \return         0 if keys are same or else non-zero value
    static bool vpc_key_func_compare(void *key1, void *key2) {
        SDK_ASSERT((key1 != NULL) && (key2 != NULL));
        if (!memcmp(key1, key2, sizeof(pds_vpc_key_t))) {
            return true;
        }
        return false;
    }

    /// \brief          return the type of VPC
    /// \return         PDS_VPC_TYPE_SUBSTRATE or PDS_VPC_TYPE_TENANT
    pds_vpc_type_t type(void) const { return type_; }

    /// \brief          return h/w index for this vpc
    /// \return         h/w table index for this vpc
    uint16_t hw_id(void) { return hw_id_; }

private:
    /// \brief constructor
    vpc_entry();

    /// \brief destructor
    ~vpc_entry();

    /// \brief    free h/w resources used by this object, if any
    ///           (this API is invoked during object deletes)
    /// \return   SDK_RET_OK on success, failure status code on error
    sdk_ret_t nuke_resources_(void);

private:
    pds_vpc_key_t key_;                       ///< vpc key
    pds_vpc_type_t type_;                     ///< vpc type
    mac_addr_t vr_mac_;                       ///< virtual router MAC
    pds_encap_t fabric_encap_;                ///< fabric encap information
    pds_route_table_key_t v4_route_table_;    ///< IPv4 route table id
    pds_route_table_key_t v6_route_table_;    ///< IPv6 route table id
    ht_ctxt_t ht_ctxt_;                       ///< hash table context

    // P4 datapath specific state
    uint16_t hw_id_;           ///< hardware id

    friend class vpc_state;    ///< vpc_state is friend of vpc_entry
} __PACK__;

/// \@}

}    // namespace api

using api::vpc_entry;

#endif    // __API_VPC_HPP__
