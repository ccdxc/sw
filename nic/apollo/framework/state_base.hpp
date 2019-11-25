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
typedef bool (state_walk_cb_t)(void *obj, void *ctxt);

/// \brief     generic walk callback function type for slab objects
/// \param[in] obj     slab instance
/// \param[in] ctxt    opaque context that the callback can interpret
/// \return    true if walk needs to be stopped or false if walk needs to
///            continue
typedef bool (slab_walk_cb_t)(void *obj, void *ctxt);

/// \brief counters maintained per instance of the state/store class
typedef struct state_counters_s {
    uint32_t insert_ok;     ///< number of successful inserts
    uint32_t insert_err;    ///< number of insert errors
    uint32_t remove_ok;     ///< number of successful deletes
    uint32_t remove_err;    ///< number of delete errors
    uint32_t update_ok;     ///< number of successful updates
    uint32_t update_err;    ///< number of update errors
    uint32_t num_elems;     ///< number of elements/objects currently in store
} state_counters_t;

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

    /// \brief API to walk all the slabs
    /// \param[in] walk_cb    callback to be invoked for every slab
    /// \param[in] ctxt       opaque context passed back to the callback
    /// \return   SDK_RET_OK on success, failure status code on error
    virtual sdk_ret_t slab_walk(state_walk_cb_t walk_cb, void *ctxt) {
        return SDK_RET_INVALID_OP;
    }

    /// \brief API to return the store counters
    /// \return    state/store counters
    const state_counters_t& counters(void) const { return counters_; }

protected:
    ///< all the counters in one place
    state_counters_t counters_;
};

}    // namespace api

#endif    // __FRAMEWORK_STATE_BASE_HPP__
