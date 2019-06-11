//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains all workflows
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_WORKFLOW1_HPP__
#define __TEST_UTILS_WORKFLOW1_HPP__

#include "nic/apollo/test/utils/batch.hpp"
#include "nic/apollo/test/utils/api_base.hpp"

namespace api_test {

// WF_1: [ Create SetMax - Delete SetMax ] - Read
template <typename feeder_T>
inline void workflow_1(feeder_T& feeder) {
    // trigger
    batch_start();
    many_create<feeder_T>(feeder);
    many_delete<feeder_T>(feeder);
    batch_commit();

    many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_2: [ Create SetMax - Delete SetMax - Create SetMax ] - Read
template <typename feeder_T>
inline void workflow_2(feeder_T& feeder) {
    // trigger
    batch_start();
    many_create<feeder_T>(feeder);
    many_delete<feeder_T>(feeder);
    many_create<feeder_T>(feeder);
    batch_commit();

    many_read<feeder_T>(feeder, sdk::SDK_RET_OK);

    // cleanup
    batch_start();
    many_delete<feeder_T>(feeder);
    batch_commit();

    many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_3: [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
template <typename feeder_T>
inline void workflow_3(feeder_T& feeder1, feeder_T& feeder2,
                       feeder_T& feeder3) {
    // trigger
    batch_start();
    many_create<feeder_T>(feeder1);
    many_create<feeder_T>(feeder2);
    many_delete<feeder_T>(feeder1);
    many_create<feeder_T>(feeder3);
    batch_commit();

    many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    many_read<feeder_T>(feeder2);
    many_read<feeder_T>(feeder3);

    // cleanup
    batch_start();
    many_delete<feeder_T>(feeder2);
    many_delete<feeder_T>(feeder3);
    batch_commit();

    many_read<feeder_T>(feeder2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    many_read<feeder_T>(feeder3, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_4: [ Create SetMax ] - Read - [ Delete SetMax ] - Read
template <typename feeder_T>
inline void workflow_4(feeder_T& feeder) {
    // trigger
    batch_start();
    many_create<feeder_T>(feeder);
    batch_commit();

    many_read<feeder_T>(feeder);

    batch_start();
    many_delete<feeder_T>(feeder);
    batch_commit();

    many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_5: [ Create Set1, Set2 ] - Read - [ Delete Set1 - Create Set3 ] - Read
template <typename feeder_T>
inline void workflow_5(feeder_T& feeder1, feeder_T& feeder2,
                       feeder_T& feeder3) {
    // trigger
    batch_start();
    many_create<feeder_T>(feeder1);
    many_create<feeder_T>(feeder2);
    batch_commit();

    many_read<feeder_T>(feeder1);
    many_read<feeder_T>(feeder2);

    batch_start();
    many_delete<feeder_T>(feeder1);
    many_create<feeder_T>(feeder3);
    batch_commit();

    many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    many_read<feeder_T>(feeder2);
    many_read<feeder_T>(feeder3);

    // cleanup
    batch_start();
    many_delete<feeder_T>(feeder2);
    many_delete<feeder_T>(feeder3);
    batch_commit();

    many_read<feeder_T>(feeder2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    many_read<feeder_T>(feeder3, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_6: [ Create SetMax - Update SetMax - Update SetMax - Delete SetMax ]
//       - Read
template <typename feeder_T>
inline void workflow_6(feeder_T& feeder1, feeder_T& feeder1A,
                       feeder_T& feeder1B) {
    // trigger
    batch_start();
    many_create<feeder_T>(feeder1);
    many_update<feeder_T>(feeder1A);
    many_update<feeder_T>(feeder1B);
    many_delete<feeder_T>(feeder1B);
    batch_commit();

    many_read<feeder_T>(feeder1B, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_7: [ Create SetMax - Delete SetMax - Create SetMax - Update SetMax -
//         Update SetMax ] - Read
template <typename feeder_T>
inline void workflow_7(feeder_T& feeder1, feeder_T& feeder1A,
                       feeder_T& feeder1B) {
    // trigger
    batch_start();
    many_create<feeder_T>(feeder1);
    many_delete<feeder_T>(feeder1);
    many_create<feeder_T>(feeder1);
    many_update<feeder_T>(feeder1A);
    many_update<feeder_T>(feeder1B);
    batch_commit();

    many_read<feeder_T>(feeder1B);

    // cleanup
    batch_start();
    many_delete<feeder_T>(feeder1B);
    batch_commit();

    many_read<feeder_T>(feeder1B, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_8: [ Create SetMax - Update SetMax ] - Read - [ Update SetMax ] - Read -
//       [ Delete SetMax ] - Read
template <typename feeder_T>
inline void workflow_8(feeder_T& feeder1, feeder_T& feeder1A,
                       feeder_T& feeder1B) {
    // trigger
    batch_start();
    many_create<feeder_T>(feeder1);
    many_update<feeder_T>(feeder1A);
    batch_commit();

    many_read<feeder_T>(feeder1A);

    batch_start();
    many_update<feeder_T>(feeder1B);
    batch_commit();

    many_read<feeder_T>(feeder1B);

    batch_start();
    many_delete<feeder_T>(feeder1B);
    batch_commit();

    many_read<feeder_T>(feeder1B, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_9: [ Create SetMax ] - Read - [ Update SetMax - Delete SetMax ] - Read
template <typename feeder_T>
inline void workflow_9(feeder_T& feeder1, feeder_T& feeder1A) {
    // trigger
    batch_start();
    many_create<feeder_T>(feeder1);
    batch_commit();

    many_read<feeder_T>(feeder1);

    batch_start();
    many_update<feeder_T>(feeder1A);
    many_delete<feeder_T>(feeder1A);
    batch_commit();

    many_read<feeder_T>(feeder1A, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_10: [ Create Set1, Set2, Set3 - Delete Set1 - Update Set2 ] - Read -
//        [ Update Set3 - Delete Set2 - Create Set4] - Read
template <typename feeder_T>
inline void workflow_10(feeder_T& feeder1, feeder_T& feeder2,
                        feeder_T& feeder2A, feeder_T& feeder3,
                        feeder_T& feeder3A, feeder_T& feeder4) {
    // trigger
    batch_start();
    many_create<feeder_T>(feeder1);
    many_create<feeder_T>(feeder2);
    many_create<feeder_T>(feeder3);
    many_delete<feeder_T>(feeder1);
    many_update<feeder_T>(feeder2A);
    batch_commit();

    many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    many_read<feeder_T>(feeder2A);
    many_read<feeder_T>(feeder3);

    batch_start();
    many_update<feeder_T>(feeder3A);
    many_delete<feeder_T>(feeder2A);
    many_create<feeder_T>(feeder4);
    batch_commit();

    many_read<feeder_T>(feeder3A);
    many_read<feeder_T>(feeder2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    many_read<feeder_T>(feeder4);

    // cleanup
    batch_start();
    many_delete<feeder_T>(feeder4);
    many_delete<feeder_T>(feeder3A);
    batch_commit();

    many_read<feeder_T>(feeder4, sdk::SDK_RET_ENTRY_NOT_FOUND);
    many_read<feeder_T>(feeder3A, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_N_1: [ Create SetMax ] - [ Create SetMax ] - Read
template <typename feeder_T>
inline void workflow_neg_1(feeder_T& feeder) {
    // trigger
    batch_start();
    many_create<feeder_T>(feeder);
    batch_commit();

    many_read<feeder_T>(feeder);

    batch_start();
    many_create<feeder_T>(feeder);
    batch_commit_fail();
    batch_abort();

    many_read<feeder_T>(feeder);

    // cleanup
    batch_start();
    many_delete<feeder_T>(feeder);
    batch_commit();

    many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_N_2: [ Create SetMax+1 ] - Read
template <typename feeder_T>
inline void workflow_neg_2(feeder_T& feeder) {
    // trigger
    batch_start();
    many_create<feeder_T>(feeder);
    batch_commit_fail();
    batch_abort();

    many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_N_3: Read NonEx - [ Delete NonExMax ] - [ Update NonExMax ] - Read
template <typename feeder_T>
inline void workflow_neg_3(feeder_T& feeder) {
    // trigger
    many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // trigger
    batch_start();
    many_delete<feeder_T>(feeder);
    batch_commit_fail();
    batch_abort();

    // trigger
    batch_start();
    many_update<feeder_T>(feeder);
    batch_commit_fail();
    batch_abort();

    many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_N_4: [ Create Set1 ] - Read - [ Delete Set1, Set2 ] - Read
template <typename feeder_T>
inline void workflow_neg_4(feeder_T& feeder1, feeder_T& feeder2) {
    // trigger
    batch_start();
    many_create<feeder_T>(feeder1);
    batch_commit();

    many_read<feeder_T>(feeder1);

    batch_start();
    many_delete<feeder_T>(feeder1);
    many_delete<feeder_T>(feeder2);
    batch_commit_fail();
    batch_abort();

    many_read<feeder_T>(feeder1);
    many_read<feeder_T>(feeder2, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // cleanup
    batch_start();
    many_delete<feeder_T>(feeder1);
    batch_commit();

    many_read<feeder_T>(feeder2, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_N_5: [ Create SetMax ] - Read - [ Delete SetMax - Update SetMax ] - Read
template <typename feeder_T>
inline void workflow_neg_5(feeder_T& feeder1, feeder_T& feeder1A) {
    // trigger
    batch_start();
    many_create<feeder_T>(feeder1);
    batch_commit();

    many_read<feeder_T>(feeder1);

    batch_start();
    many_delete<feeder_T>(feeder1);
    many_update<feeder_T>(feeder1A);
    batch_commit_fail();
    batch_abort();

    many_read<feeder_T>(feeder1);

    // cleanup
    batch_start();
    many_delete<feeder_T>(feeder1);
    batch_commit();

    many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_N_6: [ Create SetMax ] - Read - [ Update SetMax + 1 ] - Read
template <typename feeder_T>
inline void workflow_neg_6(feeder_T& feeder1, feeder_T& feeder1A) {
    // trigger
    batch_start();
    many_create<feeder_T>(feeder1);
    batch_commit();

    many_read<feeder_T>(feeder1);

    batch_start();
    many_update<feeder_T>(feeder1A);
    batch_commit_fail();
    batch_abort();

    many_read<feeder_T>(feeder1);

    // cleanup
    batch_start();
    many_delete<feeder_T>(feeder1);
    batch_commit();

    many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_N_7: [ Create Set1 ] - Read - [ Update Set1 - Update Set2 ] - Read
template <typename feeder_T>
inline void workflow_neg_7(feeder_T& feeder1, feeder_T& feeder1A,
                           feeder_T& feeder2) {
    // trigger
    batch_start();
    many_create<feeder_T>(feeder1);
    batch_commit();

    many_read<feeder_T>(feeder1);

    batch_start();
    many_update<feeder_T>(feeder1A);
    many_update<feeder_T>(feeder2);
    batch_commit_fail();
    batch_abort();

    many_read<feeder_T>(feeder1);

    // cleanup
    batch_start();
    many_delete<feeder_T>(feeder1);
    batch_commit();

    many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

// WF_N_8: [ Create Set1 ] - Read - [ Delete Set1 - Update Set2 ] - Read
template <typename feeder_T>
inline void workflow_neg_8(feeder_T& feeder1, feeder_T& feeder2) {
    // trigger
    batch_start();
    many_create<feeder_T>(feeder1);
    batch_commit();

    many_read<feeder_T>(feeder1);

    batch_start();
    many_delete<feeder_T>(feeder1);
    many_update<feeder_T>(feeder2);
    batch_commit_fail();
    batch_abort();

    many_read<feeder_T>(feeder1);

    // cleanup
    batch_start();
    many_delete<feeder_T>(feeder1);
    batch_commit();

    many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

}    // end namespace

#endif    // __TEST_UTILS_WORKFLOW1_HPP__
