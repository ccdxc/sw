//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// vpc implementation state
///
//----------------------------------------------------------------------------

#ifndef __VPC_IMPL_STATE_HPP__
#define __VPC_IMPL_STATE_HPP__

#include "nic/sdk/lib/rte_indexer/rte_indexer.hpp"
#include "nic/sdk/lib/table/slhash/slhash.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/state_base.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/impl/apulu/vpc_impl.hpp"

using sdk::table::slhash;

namespace api {
namespace impl {

/// \defgroup PDS_VPC_IMPL_STATE vpc state functionality
/// \ingroup PDS_VPC
/// @{

// forward declaration
class vpc_impl;

/// \brief  state maintained for VPCs
class vpc_impl_state : public state_base {
public:
    /// \brief  constructor
    vpc_impl_state(pds_state *state);

    /// \brief  destructor
    ~vpc_impl_state();

    /// \brief  allocate memory required for a vpc impl instance
    /// \return pointer to the allocated instance, NULL if no memory
    vpc_impl *alloc(void);

    /// \brief     free vpc impl instance back
    /// \param[in] impl pointer to the allocated impl instance
    void free(vpc_impl *impl);

    /// \brief  API to initiate transaction over all the table manamgement
    ///         library instances
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_begin(void);

    /// \brief  API to end transaction over all the table manamgement
    ///         library instances
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_transaction_end(void);

    /// \brief     API to get table stats
    /// \param[in]  cb    callback to be called on stats
    ///             ctxt    opaque ctxt passed to the callback
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t table_stats(debug::table_stats_get_cb_t cb, void *ctxt);

    /// \brief      API to find vpc impl obj using hw id
    /// \param[in]  hw_id   vpc h/w id
    /// \return     vpc impl object
    vpc_impl *find(uint16_t hw_id);

    /// \brief      API to insert vpc impl into hash table
    /// \param[in]  hw_id   vpc h/w id
    /// \param[in]  impl    vpc impl object
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t insert(uint16_t hw_id, vpc_impl *impl);

    /// \brief      API to update vpc impl in the hash table
    /// \param[in]  hw_id   vpc h/w id
    /// \param[in]  impl    vpc impl object to be updated with
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(uint16_t hw_id, vpc_impl *impl);

    /// \brief      API to remove hw id and vpc key from the hash table
    /// \param[in]  hw_id   vpc h/w id
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t remove(uint16_t hw_id);

    /// \brief      API to allocate class id given user configured tag
    /// \param[in]   tag     user give tag (for the mapping)
    /// \param[in]   local   if true, tag is that of local mapping, else for
    ///                      remote mapping
    /// \param[out]  class_id class_id corresponding to the tag, if allocation
    ///              was succesful
    /// \remark if the tag is seen 1st time, new class id allocated for it
    ///         if tag is already seen, refcount is incremented and
    ///         corresponding class_id is returned
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t alloc_class_id(uint32_t tag, bool local, uint32_t *class_id);

    /// \brief      API to release class id that was previously allocated
    /// \param[in]   class_id class id allocated
    /// \param[in]   local    if true, tag is that of local mapping, else for
    ///                       remote mapping
    /// \remark if the refcount corresponding to the class goes down to 0,
    ///         class_id is freed back to the pool
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t release_class_id(uint32_t class_id, bool local);

    /// \brief      API to release a tag, if its not in use
    /// \param[in]   tag     user given tag (for the mapping)
    /// \param[in]   local   if true, tag is that of local mapping, else for
    ///                      remote mapping
    /// \remark if the refcount corresponding to the tag goes down to 0,
    ///         corresponding class_id is freed back to the pool
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t release_tag(uint32_t tag, bool local);

    /// \brief      API to find user configured tag, given the class id
    /// \param[in]   class_id class id allocated
    /// \param[out]  tag      user given tag (for the mapping)
    /// \param[in]   local   if true, tag is that of local mapping, else for
    ///                      remote mapping
    /// \return     SDK_RET_OK on success, failure status code on error
    sdk_ret_t find_tag(uint32_t class_id, uint32_t *tag, bool local);

private:
    slhash *vni_tbl(void) { return vni_tbl_; }
    rte_indexer *vpc_idxr(void) const { return vpc_idxr_; }
    ht *impl_ht(void) const { return impl_ht_; }
    friend class vpc_impl;      // vpc_impl class is friend of vpc_impl_state
    friend class subnet_impl;   // subnet_impl class is friend of vpc_impl_state

private:
    ///< hash table for vpc VxLAN vnids
    slhash      *vni_tbl_;
    ///< indexer to allocate hw vpc id
    rte_indexer *vpc_idxr_;
    ///< state needed for tag/class maintenance for all mappings
    vpc_impl_tag_state_t *tag_state_;
    ///< hash table for hw_id to vpc key
    ht *impl_ht_;
};

/// @}

}    // namespace impl
}    // namespace api

#endif    // __VPC_IMPL_STATE_HPP__
