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
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"

namespace api {

// forward declaration
class vpc_state;

/// \defgroup PDS_VPC_ENTRY - VPC entry functionality
/// \ingroup PDS_VPC
/// @{

/// \brief VPC entry
class vpc_entry : public api_base {
public:
    /// \brief     factory method to allocate and initialize a VPC entry
    /// \param[in] spec VPC specification
    /// \return    new instance of VPC or NULL, in case of error
    static vpc_entry *factory(pds_vpc_spec_t *spec);

    /// \brief     release all the s/w state associate with the given VPC,
    ///            if any, and free the memory
    /// \param[in] vpc VPC to be freed
    /// \NOTE:     h/w entries should have been cleaned up (by calling
    ///            impl->cleanup_hw() before calling this
    static void destroy(vpc_entry *vpc);

    /// \brief    clone this object and return cloned object
    /// \param[in]    api_ctxt API context carrying object related configuration
    /// \return       new object instance of current object
    virtual api_base *clone(api_ctxt_t *api_ctxt) override;

    /// \brief    free all the memory associated with this object without
    ///           touching any of the databases or h/w etc.
    /// \param[in] vpc    vpc to be freed
    /// \return   sdk_ret_ok or error code
    static sdk_ret_t free(vpc_entry *vpc);

    /// \brief     initialize a VPC entry with the given config
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
    virtual sdk_ret_t program_create(obj_ctxt_t *obj_ctxt) override;

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

    /// \brief    compute the object diff during update operation compare the
    ///           attributes of the object on which this API is invoked and the
    ///           attrs provided in the update API call passed in the object
    ///           context (as cloned object + api_params) and compute the upd
    ///           bitmap (and stash in the object context for later use)
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t compute_update(obj_ctxt_t *obj_ctxt) override;

    /// \brief     update all h/w tables relevant to this object except
    ///            stage 0 table(s), if any, by updating packed entries
    ///            with latest epoch#
    /// \param[in] orig_obj old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_update(api_base *orig_obj,
                                    obj_ctxt_t *obj_ctxt) override;

    /// \brief          add all objects that may be affected if this object is
    ///                 updated to framework's object dependency list
    /// \param[in]      obj_ctxt    transient state associated with this API
    ///                             processing
    /// \return         SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_deps(obj_ctxt_t *obj_ctxt) override;

    /// \brief     activate the epoch in the dataplane by programming
    ///            stage 0 tables, if any
    /// \param[in] epoch    epoch being activated
    /// \param[in] api_op   api operation
    /// \param[in] orig_obj old/original version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      api_base *orig_obj,
                                      obj_ctxt_t *obj_ctxt) override;

    /// \brief  add given VPC to the database
    /// \return SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_to_db(void) override;

    /// \brief  delete given VPC from the database
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

    /// \brief          read config
    /// \param[in]      key pointer to the key object
    /// \param[out]     info pointer to the info object
    /// \return         SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_vpc_key_t *key, pds_vpc_info_t *info);

    /// \brief return stringified key of the object (for debugging)
    virtual string key2str(void) const override {
        return "vpc-" + std::to_string(key_.id);
    }

    /// \brief     helper function to get key given VPC entry
    /// \param[in] entry pointer to VPC instance
    /// \return    pointer to the VPC instance's key
    static void *vpc_key_func_get(void *entry) {
        vpc_entry *vpc = (vpc_entry *)entry;
        return (void *)&(vpc->key_);
    }

    /// \brief  return the key of VPC
    /// \return key of the VPC
    pds_vpc_key_t key(void) const { return key_; }

    /// \brief  return the type of VPC
    /// \return PDS_VPC_TYPE_UNDERLAY or PDS_VPC_TYPE_TENANT
    pds_vpc_type_t type(void) const { return type_; }

    /// \brief  return the fabric encap of this VPC
    /// \return   VxLAN or MPLSoUDP encap type & val
    pds_encap_t fabric_encap(void) const { return fabric_encap_; }

    /// \brief return the IPv4 route table of this VPC
    /// \return  IPv4 route table key of this VPC
    pds_route_table_key_t v4_route_table(void) const { return v4_route_table_; }

    /// \brief return the IPv6 route table of this VPC
    /// \return  IPv6 route table key of this VPC
    pds_route_table_key_t v6_route_table(void) const { return v6_route_table_; }

    /// \brief    return true if NAT46 prefix configured is valid
    /// \return    true or false based on whether NAT46 prefix is valid or not
    bool nat46_prefix_valid(void) const { return nat46_pfx_valid_; }

    /// \brief  return the type of VPC
    /// \return    IPv6 prefix to be used for NAT46
    ip_prefix_t& nat46_prefix(void) { return nat46_pfx_; }

    /// \brief  return h/w index for this VPC
    /// \return h/w table index for this VPC
    uint16_t hw_id(void) { return hw_id_; }

private:
    /// \brief constructor
    vpc_entry();

    /// \brief destructor
    ~vpc_entry();

    /// \brief  free h/w resources used by this object, if any
    ///         (this API is invoked during object deletes)
    /// \return SDK_RET_OK on success, failure status code on error
    sdk_ret_t nuke_resources_(void);

private:
    pds_vpc_key_t key_;                       ///< VPC key
    pds_vpc_type_t type_;                     ///< VPC type
    pds_encap_t fabric_encap_;                ///< fabric encap information
    pds_route_table_key_t v4_route_table_;    ///< IPv4 route table id
    pds_route_table_key_t v6_route_table_;    ///< IPv6 route table id
    bool nat46_pfx_valid_;                    ///< TRUE if NAT46 prefix is valid
    ip_prefix_t nat46_pfx_;                   ///< IPv6 prefix to be used to
                                              ///< (S)NAT outbound traffic
                                              ///< towards a IPv6-only cluster
    ht_ctxt_t ht_ctxt_;                       ///< hash table context

    // P4 datapath specific state
    uint16_t hw_id_;                          ///< hardware id
    impl_base *impl_;                         ///< impl object instance

    friend class vpc_state;                   ///< a friend of vpc_entry
} __PACK__;

/// \@}

}    // namespace api

using api::vpc_entry;

#endif    // __API_VPC_HPP__
