//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// base class for the state/db objects
///
//----------------------------------------------------------------------------

#ifndef __FRAMEWORK_STATE_BASE_HPP__
#define __FRAMEWORK_STATE_BASE_HPP__

#include "nic/apollo/framework/obj_base.hpp"
#include "nic/apollo/framework/api_base.hpp"

namespace api {

/// \defgroup PDS_PIPELINE_IMPL Pipeline wrapper implementation
/// @{

/// \brief     generic walk callback function type for db elements (i.e., api
///            objects)
/// \param[in] obj     object instance
/// \param[in] ctxt    opaque context that the callback can interpret
/// \return    true if walk needs to be stopped or false if walk needs to
///            continue
typedef bool (state_walk_cb_t)(void *api_obj, void *ctxt);

/// \brief state base class
class state_base : public obj_base {
public:
    /// \brief constructor
    state_base() {};

    /// \brief destructor
    ~state_base(){};

    /// \brief API to walk all the db elements
    /// \param[in] walk_cb    callback to be invoked for every node
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t walk(state_walk_cb_t walk_cb, void *ctxt) {
        return SDK_RET_INVALID_OP;
    }
};

}    // namespace api

#endif    // __FRAMEWORK_STATE_BASE_HPP__
