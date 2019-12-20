//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines stooge/side-kick ojbect that serves as base obj for all
/// objects that are only relayed, but de-duped by the API engine and processed
/// in a batch
///
//----------------------------------------------------------------------------

#ifndef __FRAMEWORK_API_STOOGE_HPP__
#define __FRAMEWORK_API_STOOGE_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/framework/obj_base.hpp"
#include "nic/apollo/framework/api.hpp"
#include "nic/apollo/framework/api_ctxt.hpp"
#include "nic/apollo/api/include/pds.hpp"

using std::string;

namespace api {

/// \brief  base class for all api related objects
class api_stooge : public api_base {
public:
    /// \brief  Allocate hardware resources for this object
    /// \param[in] orig_obj Old version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t reserve_resources(api_base *orig_obj,
                                        api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief    free hardware resources used by this object, if any
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t release_resources(void) override {
        return SDK_RET_OK;
    }

    /// \brief program config in the hardware during create operation
    /// Program all hardware tables relevant to this object except stage 0
    /// table(s), if any and also set the valid bit
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_create(api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief    cleanup config from the hardware
    /// cleanup all hardware tables relevant to this object except stage 0
    /// table(s), if any, by updating packed entries with latest epoch#
    /// and setting invalid bit (if any) in the hardware entries
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t cleanup_config(api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief    compute the object diff during update operation compare the
    ///           attributes of the object on which this API is invoked and the
    ///           attrs provided in the update API call passed in the object
    ///           context (as cloned object + api_params) and compute the upd
    ///            bitmap (and stash in the object context for later use)
    /// \param[in] obj_ctxt    transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t compute_update(api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief compute all the objects depending on this object and add to
    ///        framework's dependency list
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t add_deps(api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief    update config in the hardware
    /// update all hardware tables relevant to this object except stage 0
    /// table(s), if any, by updating packed entries with latest epoch#
    /// \param[in] orig_obj old/original version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t program_update(api_base *orig_obj,
                                     api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

    /// \brief activate the epoch in the dataplane
    /// \param[in] epoch epoch/version of new config
    /// \param[in] api_op API operation
    /// \param[in] orig_obj old/original version of the unmodified object
    /// \param[in] obj_ctxt transient state associated with this API
    /// \return #SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t activate_config(pds_epoch_t epoch, api_op_t api_op,
                                      api_base *orig_obj,
                                      api_obj_ctxt_t *obj_ctxt) override {
        return SDK_RET_OK;
    }

protected:
    /// \brief constructor
    api_stooge() {};

    /// \brief destructor
    virtual ~api_stooge(){};

protected:
} __PACK__;

}    // namespace api

using api::api_stooge;

#endif    // __FRAMEWORK_API_STOOGE_HPP__
