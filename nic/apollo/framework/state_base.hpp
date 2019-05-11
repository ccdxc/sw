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

namespace api {

/// \defgroup PDS_PIPELINE_IMPL Pipeline wrapper implementation
/// @{

/// \brief state base class
class state_base : public obj_base {
public:
     /// \brief constructor
    state_base() {};

     /// \brief destructor
    ~state_base(){};
};

}    // namespace api

#endif    // __FRAMEWORK_STATE_BASE_HPP__
