//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the feeder base class
///
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_FEEDER_HPP__
#define __TEST_API_UTILS_FEEDER_HPP__

#include "nic/sdk/include/sdk/base.hpp"

namespace test {
namespace api {

#define BIT(x) (1ul << (x))

/// \defgroup PDS_FEEDER Feeder module
/// @{

/// \brief Base class for feeder
/// This class will be used by all modules to create single or multiple
/// configuration specs
class feeder {
public:
    uint32_t num_obj;    ///< Number of objects created

    /// \brief Initialize the iterator
    void iter_init(void) { cur_iter_pos = 0; }
    /// \brief Returns true if there are more objects
    bool iter_more(void) const { return (cur_iter_pos < num_obj); }
    /// \brief Whether this object support read
    bool read_unsupported(void) const { return false; }

protected:
    uint32_t cur_iter_pos;  ///< Current iterator position
};

/// @}

}    // namespace api
}    // namespace test

#endif    // __TEST_API_UTILS_FEEDER_HPP__
