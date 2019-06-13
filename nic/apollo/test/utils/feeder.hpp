//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the feeder base class
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_FEEDER_HPP__
#define __TEST_UTILS_FEEDER_HPP__

#include "nic/sdk/include/sdk/base.hpp"

namespace api_test {

// Base class for feeder
class feeder {
public:
    uint32_t num_obj;

    // Iterate helper routines
    void iter_init() { cur_iter_pos = 0; }
    bool iter_more() { return (cur_iter_pos < num_obj); }

protected:
    uint32_t cur_iter_pos;
};

}    // namespace api_test

#endif    // __TEST_UTILS_FEEDER_HPP__
