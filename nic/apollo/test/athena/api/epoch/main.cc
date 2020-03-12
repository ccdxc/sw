
//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the all epoch test cases for athena
///
//----------------------------------------------------------------------------
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/test/api/utils/base.hpp"
#include "nic/apollo/api/include/athena/pds_epoch.h"

namespace test {
namespace api {

//----------------------------------------------------------------------------
// EPOCH test class
//----------------------------------------------------------------------------

class epoch_test : public pds_test_base {
protected:
    epoch_test() {}
    virtual ~epoch_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        pds_test_base::SetUpTestCase(g_tc_params);
    }
    static void TearDownTestCase() {
        pds_test_base::TearDownTestCase();
    }
};

//----------------------------------------------------------------------------
// Epoch test cases implementation
//----------------------------------------------------------------------------

/// \defgroup EPOCH Epoch Tests
/// @{

/// \brief Epoch tests
TEST_F(epoch_test, epoch_crud) {
    pds_epoch_spec_t spec = { 0 };
    pds_epoch_key_t key = { 0 };
    pds_epoch_info_t info = { 0 };

    spec.key.epoch_id = 1;
    spec.data.epoch = 100;
    SDK_ASSERT(pds_epoch_create(&spec) == SDK_RET_OK);

    memset(&info, 0, sizeof(info)); 
    key.epoch_id = 1;
    SDK_ASSERT(pds_epoch_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(info.spec.data.epoch == 100);

    memset(&spec, 0, sizeof(spec)); 
    spec.key.epoch_id = 2;
    spec.data.epoch = 200;
    SDK_ASSERT(pds_epoch_create(&spec) == SDK_RET_OK);

    memset(&info, 0, sizeof(info)); 
    key.epoch_id = 2;
    SDK_ASSERT(pds_epoch_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(info.spec.data.epoch == 200);

    memset(&spec, 0, sizeof(spec)); 
    spec.key.epoch_id = 1000;
    spec.data.epoch = 1010;
    SDK_ASSERT(pds_epoch_create(&spec) == SDK_RET_OK);

    memset(&info, 0, sizeof(info)); 
    key.epoch_id = 1000;
    SDK_ASSERT(pds_epoch_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(info.spec.data.epoch == 1010);

    key.epoch_id = 2;
    SDK_ASSERT(pds_epoch_delete(&key) == SDK_RET_OK);

    key.epoch_id = 2;
    SDK_ASSERT(pds_epoch_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(info.spec.data.epoch == 0);

    memset(&spec, 0, sizeof(spec)); 
    spec.key.epoch_id = 0;
    spec.data.epoch = 2000;
    SDK_ASSERT(pds_epoch_create(&spec) == SDK_RET_INVALID_ARG);

    memset(&spec, 0, sizeof(spec)); 
    spec.key.epoch_id = PDS_EPOCH_ID_MAX;
    spec.data.epoch = 55100;
    SDK_ASSERT(pds_epoch_create(&spec) == SDK_RET_INVALID_ARG);

    memset(&spec, 0, sizeof(spec)); 
    spec.key.epoch_id = 1000;
    spec.data.epoch = 11100;
    SDK_ASSERT(pds_epoch_create(&spec) == SDK_RET_OK);

    memset(&info, 0, sizeof(info)); 
    key.epoch_id = 1000;
    SDK_ASSERT(pds_epoch_read(&key, &info) == SDK_RET_OK);
    SDK_ASSERT(info.spec.data.epoch == 11100);

}

/// @}

}    // namespace api
}    // namespace test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

/// @private
int
main (int argc, char **argv)
{
    return api_test_program_run(argc, argv);
}
