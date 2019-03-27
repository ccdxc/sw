//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "base.hpp"

// Basic tests with TXN
#define TEST_CLASS_NAME txn_basic
#define TEST_OP_COUNT 1
#define TEST_USE_HANDLE_OR_KEY USE_HANDLE
#define TEST_SET_OR_CLR_HASH CLR_HASH
#include "txn_tests.hpp"

// Basic scale tests with TXN
#define TEST_CLASS_NAME txn_scale
#define TEST_OP_COUNT TABLE_SIZE
#define TEST_USE_HANDLE_OR_KEY USE_HANDLE
#define TEST_SET_OR_CLR_HASH CLR_HASH
#include "txn_tests.hpp"
