//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "base.hpp"

// Basic Tests
#define TEST_CLASS_NAME basic
#define TEST_OP_COUNT 1
#define TEST_USE_HANDLE_OR_KEY USE_KEY
#define TEST_SET_OR_CLR_HASH CLR_HASH
#include "hash_tests.hpp"

// Basic Scale Tests
#define TEST_CLASS_NAME basic_scale
#define TEST_OP_COUNT TABLE_SIZE
#define TEST_USE_HANDLE_OR_KEY USE_KEY
#define TEST_SET_OR_CLR_HASH CLR_HASH
#include "hash_tests.hpp"

// Basic tests with Hash
#define TEST_CLASS_NAME basic_with_hash
#define TEST_OP_COUNT 1
#define TEST_USE_HANDLE_OR_KEY USE_KEY
#define TEST_SET_OR_CLR_HASH SET_HASH
#include "hash_tests.hpp"

// Basic scale tests with Hash
#define TEST_CLASS_NAME basic_scale_with_hash
#define TEST_OP_COUNT TABLE_SIZE
#define TEST_USE_HANDLE_OR_KEY USE_KEY
#define TEST_SET_OR_CLR_HASH SET_HASH
#include "hash_tests.hpp"
