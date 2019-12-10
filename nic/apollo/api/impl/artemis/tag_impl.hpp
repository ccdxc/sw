//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// tag implementation in the p4/hw
///
//----------------------------------------------------------------------------

#ifndef __TAG_IMPL_HPP__
#define __TAG_IMPL_HPP__

#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_tag.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_TAG_IMPL - tag functionality
/// \ingroup PDS_TAG
/// \@{

/// \brief  tag implementation
class tag_impl : public impl_base {
public:
    /// \brief      factory method to allocate & initialize tag impl instance
    /// \param[in]  spec tag configuration
    /// \return     new instance of tag or NULL, in case of error
    static tag_impl *factory(pds_tag_spec_t *spec);

    /// \brief      release all the s/w state associated with the given
    ///             tag instance, if any, and free the memory
    /// \param[in]  impl tag impl instance to be freed
    // NOTE: h/w entries should have been cleaned up (by calling
    //       impl->cleanup_hw() before calling this
    static void destroy(tag_impl *impl);

    /// \brief      allocate/reserve h/w resources for this object
    /// \param[in]  api_obj API object for which resources are being reserved
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *api_obj,
                                        obj_ctxt_t *obj_ctxt) override;

    /// \brief  free h/w resources used by this object, if any
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(api_base *api_obj) override;

    /// \brief      free h/w resources used by this object, if any
    ///             (this API is invoked during object deletes)
    /// \param[in]  api_obj api object holding the resources
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t nuke_resources(api_base *api_obj) override;

    /// \brief      program all h/w tables relevant to this object except
    ///             stage 0 table(s), if any
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_hw(api_base *api_obj,
                                 obj_ctxt_t *obj_ctxt) override;

    /// \brief      cleanup all h/w tables relevant to this object except
    ///             stage 0 table(s), if any, by updating packed entries with
    ///             latest epoch#
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_hw(api_base *api_obj,
                                 obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief      activate the epoch in the dataplane by programming stage 0
    ///             tables, if any
    /// \param[in]  api_obj  (cloned) API api object being activated
    /// \param[in]  orig_obj previous/original unmodified object
    /// \param[in]  epoch    epoch being activated
    /// \param[in]  api_op   api operation
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_hw(api_base *api_obj, api_base *orig_obj,
                                  pds_epoch_t epoch, api_op_t api_op,
                                  obj_ctxt_t *obj_ctxt) override;

    mem_addr_t lpm_root_addr(void) { return lpm_root_addr_; }

private:
    /// \brief  constructor
    tag_impl() {
        lpm_root_addr_ = 0xFFFFFFFFFFFFFFFFUL;
    }

    /// \brief  destructor
    ~tag_impl() {}

private:
    mem_addr_t    lpm_root_addr_;   ///< LPM tree's root node address
} __PACK__;

/// @}

}    // namespace impl
}    // namespace api

#endif    // __TAG_IMPL_HPP__
