//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// meter implementation in the p4/hw
///
//----------------------------------------------------------------------------

#if !defined (__METER_IMPL_HPP__)
#define __METER_IMPL_HPP__

#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_meter.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_METER_IMPL meter functionality
/// \ingroup PDS_METER
/// @{

/// \brief  meter implementation
class meter_impl : public impl_base {
public:
    /// \brief      factory method to allocate & initialize meter impl instance
    /// \param[in]  spec meter configuration
    /// \return     new instance of meter or NULL, in case of error
    static meter_impl *factory(pds_meter_spec_t *spec);

    /// \brief      release all the s/w state associated with the given
    ///             meter instance, if any, and free the memory
    /// \param[in]  impl meter impl instance to be freed
    // NOTE: h/w entries should have been cleaned up (by calling
    //       impl->cleanup_hw() before calling this
    static void destroy(meter_impl *impl);

    /// \brief      allocate/reserve h/w resources for this object
    /// \param[in]  orig_obj old version of the unmodified object
    /// \param[in]  obj_ctxt transient state associated with this API
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
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
    ///             stage0 table(s), if any, by updating packed entries with
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

    /// \brief      read spec, statistics and status from hw tables
    /// \param[in]  api_obj  API object
    /// \param[in]  key  pointer to meter key
    /// \param[out] info pointer to meter info
    /// \return     #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t read_hw(api_base *api_obj, obj_key_t *key,
                              obj_info_t *info) override;

    /// \brief      return LPM base/root address in the memory
    /// \return     memory address of the meter LPM tree
    mem_addr_t lpm_root_addr(void) { return lpm_root_addr_; }

    /// \brief      fill the Meter stats
    /// \param[out] stats statistics
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t fill_stats_(pds_meter_info_t *info);

private:
    /// \brief  constructor
    meter_impl() {
        lpm_root_addr_ = 0xFFFFFFFFFFFFFFFFUL;
        policer_base_hw_idx_ = 0xFFFFFFFF;
        stats_base_hw_idx_ = 0xFFFFFFFF;
        num_policers_ = 0;
        num_stats_entries_ = 0;
    }

    /// \brief  destructor
    ~meter_impl() {}

private:
    mem_addr_t    lpm_root_addr_;          ///< LPM tree's root node address
    uint32_t      policer_base_hw_idx_;    ///< policer base index
    uint32_t      stats_base_hw_idx_;      ///< accounting stats base index
    uint32_t      num_policers_;           ///< number of policers allocated
    uint32_t      num_stats_entries_;      ///< number of meter stats entries allocated
} __PACK__;

/// @}

}    // namespace impl
}    // namespace api

#endif    // __METER_IMPL_HPP__
