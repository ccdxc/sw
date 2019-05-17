//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all workflows
///
//----------------------------------------------------------------------------


#ifndef __TEST_UTILS_WORKFLOW_HPP__
#define __TEST_UTILS_WORKFLOW_HPP__

namespace api_test {

// WF_1: [ Create SetMax, Delete SetMax ] - Read
template <typename class_T, typename seed_T>
inline void workflow_1(seed_T *seed) {
    // trigger
    batch_start();
    ASSERT_TRUE(class_T::many_create(seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(class_T::many_delete(seed) == sdk::SDK_RET_OK);
    batch_commit();

    ASSERT_TRUE(class_T::many_read(
        seed, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

// WF_2: [ Create SetMax, Delete SetMax, Create SetMax ] - Read
template <typename class_T, typename seed_T>
inline void workflow_2(seed_T *seed) {
    // trigger
    batch_start();
    ASSERT_TRUE(class_T::many_create(seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(class_T::many_delete(seed) == sdk::SDK_RET_OK);
    ASSERT_TRUE(class_T::many_create(seed) == sdk::SDK_RET_OK);
    batch_commit();

    ASSERT_TRUE(class_T::many_read(
        seed, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // cleanup
    batch_start();
    ASSERT_TRUE(class_T::many_delete(seed) == sdk::SDK_RET_OK);
    batch_commit();

    ASSERT_TRUE(class_T::many_read(
        seed, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

// WF_3: [ Create Set1 Set2, Delete Set1, Create Set3 ] - Read
template <typename class_T, typename seed_T>
inline void workflow_3(seed_T *seed1, seed_T *seed2, seed_T *seed3) {

    // trigger
    batch_start();
    ASSERT_TRUE(class_T::many_create(seed1) == sdk::SDK_RET_OK);
    ASSERT_TRUE(class_T::many_create(seed2) == sdk::SDK_RET_OK);
    ASSERT_TRUE(class_T::many_delete(seed1) == sdk::SDK_RET_OK);
    ASSERT_TRUE(class_T::many_create(seed3) == sdk::SDK_RET_OK);
    batch_commit();

    ASSERT_TRUE(class_T::many_read(
        seed1, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(class_T::many_read(
        seed2, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);
    ASSERT_TRUE(class_T::many_read(
        seed3, sdk::SDK_RET_OK) == sdk::SDK_RET_OK);

    // cleanup
    batch_start();
    ASSERT_TRUE(class_T::many_delete(seed2) == sdk::SDK_RET_OK);
    ASSERT_TRUE(class_T::many_delete(seed3) == sdk::SDK_RET_OK);
    batch_commit();

    ASSERT_TRUE(class_T::many_read(
        seed2, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
    ASSERT_TRUE(class_T::many_read(
        seed3, sdk::SDK_RET_ENTRY_NOT_FOUND) == sdk::SDK_RET_OK);
}

}    // end namespace

#endif    // __TEST_UTILSWORKFLOW_HPP__
