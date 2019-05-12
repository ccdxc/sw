//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// Basic object definition for all objects
///
//----------------------------------------------------------------------------

#ifndef __FRAMEWORK_OBJ_BASE_HPP__
#define __FRAMEWORK_OBJ_BASE_HPP__

namespace api {

/// \brief Base class for all objects
class obj_base {
public:
    /// \brief constructor
    obj_base(){};

    /// \brief destructor
    virtual ~obj_base() {};
};

}    // namespace api

using api::obj_base;

#endif    // __FRAMEWORK_OBJ_BASE_HPP__
