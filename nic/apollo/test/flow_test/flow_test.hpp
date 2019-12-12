//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __FLOW_TEST_HPP__
#define __FLOW_TEST_HPP__

#include "nic/apollo/test/scale/test_common.hpp"

#ifndef SKIP_FLOW

#ifdef APOLLO
#include "flow_test_apollo.hpp"
#elif defined(ARTEMIS)
#include "flow_test_artemis.hpp"
#elif defined(APULU)
#include "flow_test_apulu.hpp"
#elif defined(POSEIDON)
#include "flow_test_poseidon.hpp"
#else
#include "flow_test_iris.hpp"
#endif

#endif

#endif
