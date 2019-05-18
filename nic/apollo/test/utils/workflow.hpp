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

#define MANY_CREATE(seed)                                           \
    ASSERT_TRUE(class_T::many_create(seed) == sdk::SDK_RET_OK)

#define MANY_READ(seed)                                             \
    ASSERT_TRUE(class_T::many_read(seed) == sdk::SDK_RET_OK)

#define MANY_READ_FAIL(seed, exp_result)                            \
    ASSERT_TRUE(class_T::many_read(                                 \
        seed, exp_result) == sdk::SDK_RET_OK)

#define MANY_UPDATE(seed)                                           \
    ASSERT_TRUE(class_T::many_update(seed) == sdk::SDK_RET_OK)

#define MANY_DELETE(seed)                                           \
    ASSERT_TRUE(class_T::many_delete(seed) == sdk::SDK_RET_OK)

// WF_1: [ Create SetMax - Delete SetMax ] - Read
template <typename class_T, typename seed_T>
inline void workflow_1(seed_T *seed) {
    // trigger
    batch_start();
    MANY_CREATE(seed);
    MANY_DELETE(seed);
    batch_commit();

    MANY_READ_FAIL(seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_2: [ Create SetMax - Delete SetMax - Create SetMax ] - Read
template <typename class_T, typename seed_T>
inline void workflow_2(seed_T *seed) {
    // trigger
    batch_start();
    MANY_CREATE(seed);
    MANY_DELETE(seed);
    MANY_CREATE(seed);
    batch_commit();

    MANY_READ(seed);

    // cleanup
    batch_start();
    MANY_DELETE(seed);
    batch_commit();

    MANY_READ_FAIL(seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_3: [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
template <typename class_T, typename seed_T>
inline void workflow_3(seed_T *seed1, seed_T *seed2, seed_T *seed3) {
    // trigger
    batch_start();
    MANY_CREATE(seed1);
    MANY_CREATE(seed2);
    MANY_DELETE(seed1);
    MANY_CREATE(seed3);
    batch_commit();

    MANY_READ_FAIL(seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    MANY_READ(seed2);
    MANY_READ(seed3);

    // cleanup
    batch_start();
    MANY_DELETE(seed2);
    MANY_DELETE(seed3);
    batch_commit();

    MANY_READ_FAIL(seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    MANY_READ_FAIL(seed3, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_4: [ Create SetMax ] - Read - [ Delete SetMax ] - Read
template <typename class_T, typename seed_T>
inline void workflow_4(seed_T *seed) {
    // trigger
    batch_start();
    MANY_CREATE(seed);
    batch_commit();

    MANY_READ(seed);

    batch_start();
    MANY_DELETE(seed);
    batch_commit();

    MANY_READ_FAIL(seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_5: [ Create Set1, Set2 ] - Read - [ Delete Set1 - Create Set3 ] - Read
template <typename class_T, typename seed_T>
inline void workflow_5(seed_T *seed1, seed_T *seed2, seed_T *seed3) {
    // trigger
    batch_start();
    MANY_CREATE(seed1);
    MANY_CREATE(seed2);
    batch_commit();

    MANY_READ(seed1);
    MANY_READ(seed2);

    batch_start();
    MANY_DELETE(seed1);
    MANY_CREATE(seed3);
    batch_commit();

    MANY_READ_FAIL(seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    MANY_READ(seed2);
    MANY_READ(seed3);

    // cleanup
    batch_start();
    MANY_DELETE(seed2);
    MANY_DELETE(seed3);
    batch_commit();

    MANY_READ_FAIL(seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    MANY_READ_FAIL(seed3, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_6: [ Create SetMax - Update SetMax - Update SetMax - Delete SetMax ]
//       - Read
template <typename class_T, typename seed_T>
inline void workflow_6(seed_T *seed1, seed_T *seed2, seed_T *seed3) {
    // trigger
    batch_start();
    MANY_CREATE(seed1);
    MANY_UPDATE(seed2);
    MANY_UPDATE(seed3);
    MANY_DELETE(seed3);
    batch_commit();

    MANY_READ_FAIL(seed3, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_7: [ Create SetMax - Delete SetMax - Create SetMax - Update SetMax -
//         Update SetMax ] - Read
template <typename class_T, typename seed_T>
inline void workflow_7(seed_T *seed1, seed_T *seed2, seed_T *seed3) {
    // trigger
    batch_start();
    MANY_CREATE(seed1);
    MANY_DELETE(seed2);
    MANY_CREATE(seed1);
    MANY_UPDATE(seed2);
    MANY_UPDATE(seed3);
    batch_commit();

    MANY_READ(seed3);

    // cleanup
    batch_start();
    MANY_DELETE(seed3);
    batch_commit();

    MANY_READ_FAIL(seed3, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_8: [ Create SetMax - Update SetMax ] - Read - [ Update SetMax ] - Read -
//       [ Delete SetMax ] - Read
template <typename class_T, typename seed_T>
inline void workflow_8(seed_T *seed1, seed_T *seed2, seed_T *seed3) {
    // trigger
    batch_start();
    MANY_CREATE(seed1);
    MANY_UPDATE(seed2);
    batch_commit();

    MANY_READ(seed2);

    batch_start();
    MANY_UPDATE(seed3);
    batch_commit();

    MANY_READ(seed3);

    batch_start();
    MANY_DELETE(seed3);
    batch_commit();

    MANY_READ_FAIL(seed3, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_9: [ Create SetMax ] - Read - [ Update SetMax - Delete SetMax ] - Read
template <typename class_T, typename seed_T>
inline void workflow_9(seed_T *seed1, seed_T *seed2) {
    // trigger
    batch_start();
    MANY_CREATE(seed1);
    batch_commit();

    MANY_READ(seed1);

    batch_start();
    MANY_UPDATE(seed2);
    MANY_DELETE(seed2);
    batch_commit();

    MANY_READ_FAIL(seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_10: [ Create Set1, Set2, Set3 - Delete Set1 - Update Set2 ] - Read -
//        [ Update Set3 - Delete Set2 - Create Set4] - Read
template <typename class_T, typename seed_T>
inline void workflow_10(seed_T *seed1, seed_T *seed2, seed_T *seed2A,
                        seed_T *seed3, seed_T *seed3A, seed_T *seed4) {
    // trigger
    batch_start();
    MANY_CREATE(seed1);
    MANY_CREATE(seed2);
    MANY_CREATE(seed3);
    MANY_DELETE(seed1);
    MANY_UPDATE(seed2A);
    batch_commit();

    MANY_READ_FAIL(seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    MANY_READ(seed2A);
    MANY_READ(seed3);

    batch_start();
    MANY_UPDATE(seed3A);
    MANY_DELETE(seed2A);
    MANY_CREATE(seed4);
    batch_commit();

    MANY_READ(seed3A);
    MANY_READ_FAIL(seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    MANY_READ(seed4);

    // cleanup
    batch_start();
    MANY_DELETE(seed4);
    MANY_DELETE(seed3A);
    batch_commit();

    MANY_READ_FAIL(seed4, sdk::SDK_RET_ENTRY_NOT_FOUND);
    MANY_READ_FAIL(seed3A, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_N_1: [ Create SetMax ] - [ Create SetMax ] - Read
template <typename class_T, typename seed_T>
inline void workflow_neg_1(seed_T *seed) {
    // trigger
    batch_start();
    MANY_CREATE(seed);
    batch_commit();

    MANY_READ(seed);

    batch_start();
    MANY_CREATE(seed);
    batch_commit_fail();
    batch_abort();

    MANY_READ(seed);

    // cleanup
    batch_start();
    MANY_DELETE(seed);
    batch_commit();

    MANY_READ_FAIL(seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_N_2: [ Create SetMax+1 ] - Read
template <typename class_T, typename seed_T>
inline void workflow_neg_2(seed_T *seed) {
    // trigger
    batch_start();
    MANY_CREATE(seed);
    batch_commit_fail();
    batch_abort();

    MANY_READ_FAIL(seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_N_3: Read NonEx - [ Delete NonExMax ]
template <typename class_T, typename seed_T>
inline void workflow_neg_3(seed_T *seed) {
    // trigger
    MANY_READ_FAIL(seed, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // trigger
    batch_start();
    MANY_DELETE(seed);
    batch_commit_fail();
    batch_abort();
}

// WF_N_4: [ Create Set1 ] - Read - [ Delete Set1, Set2 ] - Read
template <typename class_T, typename seed_T>
inline void workflow_neg_4(seed_T *seed1, seed_T *seed2) {
    // trigger
    batch_start();
    MANY_CREATE(seed1);
    batch_commit();

    MANY_READ(seed1);

    batch_start();
    MANY_DELETE(seed1);
    MANY_DELETE(seed2);
    batch_commit_fail();
    batch_abort();

    MANY_READ(seed1);
    MANY_READ_FAIL(seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // cleanup
    batch_start();
    MANY_DELETE(seed1);
    batch_commit();

    MANY_READ_FAIL(seed2, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_N_5: [ Create SetMax ] - Read - [ Delete SetMax - Update SetMax ] - Read
template <typename class_T, typename seed_T>
inline void workflow_neg_5(seed_T *seed1, seed_T *seed1A) {
    // trigger
    batch_start();
    MANY_CREATE(seed1);
    batch_commit();

    MANY_READ(seed1);

    batch_start();
    MANY_DELETE(seed1);
    MANY_UPDATE(seed1A);
    batch_commit_fail();
    batch_abort();

    MANY_READ(seed1);

    // cleanup
    batch_start();
    MANY_DELETE(seed1);
    batch_commit();

    MANY_READ_FAIL(seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_N_6: [ Update SetMax ] - Read
template <typename class_T, typename seed_T>
inline void workflow_neg_6(seed_T *seed) {
    // trigger
    batch_start();
    MANY_UPDATE(seed);
    batch_commit_fail();
    batch_abort();

    MANY_READ_FAIL(seed, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_N_7: [ Create SetMax ] - Read - [ Update SetMax + 1 ] - Read
template <typename class_T, typename seed_T>
inline void workflow_neg_7(seed_T *seed1, seed_T *seed1A) {
    // trigger
    batch_start();
    MANY_CREATE(seed1);
    batch_commit();

    MANY_READ(seed1);

    batch_start();
    MANY_UPDATE(seed1A);
    batch_commit_fail();
    batch_abort();

    MANY_READ(seed1);
    // TODO: looks wrong, entry should be found, right?
    // MANY_READ_FAIL(seed1A, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // cleanup
    batch_start();
    MANY_DELETE(seed1);
    batch_commit();

    MANY_READ_FAIL(seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_N_8: [ Create Set1 ] - Read - [ Update Set1 - Update Set2 ] - Read
template <typename class_T, typename seed_T>
inline void workflow_neg_8(seed_T *seed1, seed_T *seed1A, seed_T *seed2) {
    // trigger
    batch_start();
    MANY_CREATE(seed1);
    batch_commit();

    MANY_READ(seed1);

    batch_start();
    MANY_UPDATE(seed1A);
    MANY_UPDATE(seed2);
    batch_commit_fail();
    batch_abort();

    MANY_READ(seed1);

    // cleanup
    batch_start();
    MANY_DELETE(seed1);
    batch_commit();

    MANY_READ_FAIL(seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_N_9: [ Create Set1 ] - Read - [ Delete Set1 - Update Set2 ] - Read
template <typename class_T, typename seed_T>
inline void workflow_neg_9(seed_T *seed1, seed_T *seed2) {
    // trigger
    batch_start();
    MANY_CREATE(seed1);
    batch_commit();

    MANY_READ(seed1);

    batch_start();
    MANY_DELETE(seed1);
    MANY_UPDATE(seed2);
    batch_commit_fail();
    batch_abort();

    MANY_READ(seed1);

    // cleanup
    batch_start();
    MANY_DELETE(seed1);
    batch_commit();

    MANY_READ_FAIL(seed1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

}    // end namespace

#endif    // __TEST_UTILS_WORKFLOW_HPP__
