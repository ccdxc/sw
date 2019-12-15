//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// device implementation in the p4/hw
///
//----------------------------------------------------------------------------

#if !defined (__DEVICE_IMPL_HPP__)
#define __DEVICE_IMPL_HPP__

#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/impl_base.hpp"
#include "nic/apollo/api/include/pds_device.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_DEVICE_IMPL - device functionality
/// \ingroup PDS_DEVICE
/// @{

/// \brief  DEVICE implementation
class device_impl : public impl_base {
public:
    /// \brief     factory method to allocate & initialize device impl
    ///            instance
    /// \param[in] pds_device device information
    /// \return    new instance of device or NULL, in case of error
    static device_impl *factory(pds_device_spec_t *pds_device);

    /// \brief     release all the s/w state associated with the given device,
    ///            if any, and free the memory
    /// \param[in] impl device to be freed
    /// \NOTE:     h/w entries should have been cleaned up (by calling
    ///            impl->cleanup_hw() before calling this
    static void destroy(device_impl *impl);

    /// \brief  constructor
    device_impl() {}

    /// \brief  destructor
    ~device_impl() {}

    /// \brief     program all h/w tables relevant to this object except
    ///            stage 0 table(s), if any
    /// \param[in] api_obj  api object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_hw(api_base *api_obj,
                                 api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief     cleanup all h/w tables relevant to this object except stage 0
    ///            table(s), if any, by updating packed entries with latest
    ///            epoch#
    /// \param[in] api_obj  api object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_hw(api_base *api_obj,
                                 api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief     update all h/w tables relevant to this object except stage 0
    ///            table(s), if any, by updating packed entries with latest
    ///            epoch#
    /// \param[in] curr_obj current version of the unmodified object
    /// \param[in] prev_obj previous version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t update_hw(api_base *curr_obj, api_base *prev_obj,
                                api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief     activate the epoch in the dataplane by programming stage 0
    ///            tables, if any
    /// \param[in] api_obj  (cloned) API api object being activated
    /// \param[in] orig_obj previous/original unmodified object
    /// \param[in] epoch    epoch being activated
    /// \param[in] api_op   api operation
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return    #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_hw(api_base *api_obj, api_base *orig_obj,
                                  pds_epoch_t epoch, api_op_t api_op,
                                  api_obj_ctxt_t *obj_ctxt) override;

    /// \brief      read spec, statistics and status from hw tables
    /// \param[in]  api_obj api object
    /// \param[in]  key     object key
    /// \param[out] info    pointer to device info
    /// \return     #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read_hw(api_base *api_obj, obj_key_t *key, obj_info_t *info);

private:
    /// \brief      populate specification with hardware information
    /// \param[out] spec specification
    void fill_spec_(pds_device_spec_t *spec);

     /// \brief      populate ingress drop stats with hardware information
     /// \param[out] ing_drop_stats Ingress drop statistics
    void fill_ing_drop_stats_(pds_device_drop_stats_t *ing_drop_stats);
};

/// @}

}    // namespace impl
}    // namespace api

#endif    /** __DEVICE_IMPL_HPP__ */
