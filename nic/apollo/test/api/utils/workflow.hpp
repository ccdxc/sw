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

#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/batch.hpp"

namespace test {
namespace api {

/// \defgroup WF Workflow
/// This group implements the gtests workflows which will be called by various
/// api test case binaries [Ex: vnic, subnet, etc. api gtest binary]. The
/// workflows work with the api feeder, which is filled by the calling binary.
/// All workflows are defined below. [ ] represents api objects in the batch.
/// "SetMax" represents the maximum elements possible for that api object class.
/// @{

/// \brief WF_B1
/// [ Create One ] - Read - [ Delete One ] - Read
/// Create one object in a batch and delete the same object in another batch
/// \anchor WF_B1
template <typename feeder_T>
inline void workflow_b1(feeder_T& feeder) {
    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder, SDK_RET_OK);

    bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief WF_B2
/// [ Create One ] - Read - [ Update One ] - Read
/// Create one object in a batch and update the same object in another batch
/// \anchor WF_B2
template <typename feeder_T>
inline void workflow_b2(feeder_T& feeder, feeder_T& feeder1A) {
    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder, SDK_RET_OK);

    bctxt = batch_start();
    many_update<feeder_T>(bctxt, feeder1A);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1A, SDK_RET_OK);

    // cleanup
    bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder1A);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1A, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief WF_1
/// [ Create SetMax - Delete SetMax ] - Read
/// Create and delete max objects in the same batch. The operation should be
/// de-duped by framework and is a NO-OP from hardware perspective
/// \anchor WF_1
template <typename feeder_T>
inline void workflow_1(feeder_T& feeder) {
    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder);
    many_delete<feeder_T>(bctxt, feeder);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief WF_2
/// [ Create SetMax - Delete SetMax - Create SetMax ] - Read
/// Create, delete max objects and create max objects again in the same batch
/// \anchor WF_2
template <typename feeder_T>
inline void workflow_2(feeder_T& feeder) {
    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder);
    many_delete<feeder_T>(bctxt, feeder);
    many_create<feeder_T>(bctxt, feeder);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder, SDK_RET_OK);

    // cleanup
    bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief WF_3
/// [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
/// Create, delete some and create another set of nodes in the same batch
/// \anchor WF_3
template <typename feeder_T>
inline void workflow_3(feeder_T& feeder1, feeder_T& feeder2,
                       feeder_T& feeder3) {
    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder1);
    many_create<feeder_T>(bctxt, feeder2);
    many_delete<feeder_T>(bctxt, feeder1);
    many_create<feeder_T>(bctxt, feeder3);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    many_read<feeder_T>(feeder2);
    many_read<feeder_T>(feeder3);

    // cleanup
    bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder2);
    many_delete<feeder_T>(bctxt, feeder3);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    many_read<feeder_T>(feeder3, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief WF_4
/// [ Create SetMax ] - Read - [ Delete SetMax ] - Read
/// Create and delete max entries in two batches.
/// \anchor WF_4
template <typename feeder_T>
inline void workflow_4(feeder_T& feeder) {
    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder);

    bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief WF_5
/// [ Create Set1, Set2 ] - Read - [ Delete Set1 - Create Set3 ] - Read
/// Create and delete mix and match of entries in two batches
/// \anchor WF_5
template <typename feeder_T>
inline void workflow_5(feeder_T& feeder1, feeder_T& feeder2,
                       feeder_T& feeder3) {
    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder1);
    many_create<feeder_T>(bctxt, feeder2);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1);
    many_read<feeder_T>(feeder2);

    bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder1);
    many_create<feeder_T>(bctxt, feeder3);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    many_read<feeder_T>(feeder2);
    many_read<feeder_T>(feeder3);

    // cleanup
    bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder2);
    many_delete<feeder_T>(bctxt, feeder3);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    many_read<feeder_T>(feeder3, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief WF_6
/// [ Create SetMax - Update SetMax - Update SetMax - Delete SetMax ] - Read
/// Create update, update and delete max entries in the same batch
/// NO-OP kind of result from hardware perspective
/// \anchor WF_6
template <typename feeder_T>
inline void workflow_6(feeder_T& feeder1, feeder_T& feeder1A,
                       feeder_T& feeder1B) {
    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder1);
    many_update<feeder_T>(bctxt, feeder1A);
    many_update<feeder_T>(bctxt, feeder1B);
    many_delete<feeder_T>(bctxt, feeder1B);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1B, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief WF_7
/// [ Create SetMax - Delete SetMax - Create SetMax - Update SetMax] - Read -
///     [ Update SetMax ] - Read
/// Create, delete aa
/// \anchor WF_7
template <typename feeder_T>
inline void workflow_7(feeder_T& feeder1, feeder_T& feeder1A,
                       feeder_T& feeder1B) {
    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder1);
    many_delete<feeder_T>(bctxt, feeder1);
    many_create<feeder_T>(bctxt, feeder1);
    many_update<feeder_T>(bctxt, feeder1A);
    many_update<feeder_T>(bctxt, feeder1B);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1B);

    // cleanup
    bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder1B);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1B, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief WF_8
/// [ Create SetMax - Update SetMax ] - Read - [ Update SetMax ] - Read -
///     [ Delete SetMax ] - Read
/// Create update in a batch, update in a batch, delete in a batch max
/// checking multiple updates, each in different batch
/// \anchor WF_8
template <typename feeder_T>
inline void workflow_8(feeder_T& feeder1, feeder_T& feeder1A,
                       feeder_T& feeder1B) {
    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder1);
    many_update<feeder_T>(bctxt, feeder1A);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1A);

    bctxt = batch_start();
    many_update<feeder_T>(bctxt, feeder1B);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1B);

    bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder1B);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1B, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief WF_9
/// [ Create SetMax ] - Read - [ Update SetMax - Delete SetMax ] - Read
/// Create in a batch, update and delete in a batch.
/// \anchor WF_9
template <typename feeder_T>
inline void workflow_9(feeder_T& feeder1, feeder_T& feeder1A) {
    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder1);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1);

    bctxt = batch_start();
    many_update<feeder_T>(bctxt, feeder1A);
    many_delete<feeder_T>(bctxt, feeder1A);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1A, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief WF_10
/// [ Create Set1, Set2, Set3 - Delete Set1 - Update Set2 ] - Read -
///     [ Update Set3 - Delete Set2 - Create Set4] - Read
/// \anchor WF_10
template <typename feeder_T>
inline void workflow_10(feeder_T& feeder1, feeder_T& feeder2,
                        feeder_T& feeder2A, feeder_T& feeder3,
                        feeder_T& feeder3A, feeder_T& feeder4) {
    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder1);
    many_create<feeder_T>(bctxt, feeder2);
    many_create<feeder_T>(bctxt, feeder3);
    many_delete<feeder_T>(bctxt, feeder1);
    many_update<feeder_T>(bctxt, feeder2A);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    many_read<feeder_T>(feeder2A);
    many_read<feeder_T>(feeder3);

    bctxt = batch_start();
    many_update<feeder_T>(bctxt, feeder3A);
    many_delete<feeder_T>(bctxt, feeder2A);
    many_create<feeder_T>(bctxt, feeder4);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder3A);
    many_read<feeder_T>(feeder2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    many_read<feeder_T>(feeder4);

    // cleanup
    bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder4);
    many_delete<feeder_T>(bctxt, feeder3A);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder4, sdk::SDK_RET_ENTRY_NOT_FOUND);
    many_read<feeder_T>(feeder3A, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief WF_N_1
/// [ Create SetMax ] - [ Create SetMax ] - Read
/// Create maximum number of a given api objects supported in two batches
/// \anchor WF_N_1
template <typename feeder_T>
inline void workflow_neg_1(feeder_T& feeder) {
    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder);

    bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder);
    batch_commit_fail(bctxt);

    many_read<feeder_T>(feeder);

    // cleanup
    bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief WF_N_2
/// [ Create SetMax+1 ] - Read
/// Create more than maximum number of a given api objects supported
/// \anchor WF_N_2
template <typename feeder_T>
inline void workflow_neg_2(feeder_T& feeder) {
    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder);
    batch_commit_fail(bctxt);

    many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief WF_N_3
/// Read NonEx - [ Delete NonExMax ] - Read - [ Update NonExMax ] - Read
/// \anchor WF_N_3
template <typename feeder_T>
inline void workflow_neg_3(feeder_T& feeder) {
    // trigger
    many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder);
    batch_commit_fail(bctxt);

    // trigger
    bctxt = batch_start();
    many_update<feeder_T>(bctxt, feeder);
    batch_commit_fail(bctxt);

    many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief WF_N_4
/// [ Create Set1 ] - Read - [ Delete Set1, Set2 ] - Read
/// Invalid batch shouldn't affect entries of previous batch
/// \anchor WF_N_4
template <typename feeder_T>
inline void workflow_neg_4(feeder_T& feeder1, feeder_T& feeder2) {
    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder1);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1);

    bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder1);
    many_delete<feeder_T>(bctxt, feeder2);
    batch_commit_fail(bctxt);

    many_read<feeder_T>(feeder1);
    many_read<feeder_T>(feeder2, sdk::SDK_RET_ENTRY_NOT_FOUND);

    // cleanup
    bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder1);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder2, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief WF_N_5
/// [ Create SetMax ] - Read - [ Delete SetMax - Update SetMax ] - Read
/// \anchor WF_N_5
template <typename feeder_T>
inline void workflow_neg_5(feeder_T& feeder1, feeder_T& feeder1A) {
    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder1);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1);

    bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder1);
    many_update<feeder_T>(bctxt, feeder1A);
    batch_commit_fail(bctxt);

    many_read<feeder_T>(feeder1);

    // cleanup
    bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder1);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief WF_N_6
/// [ Create SetMax ] - Read - [ Update SetMax + 1 ] - Read
/// Updation of more than max entries should fail leaving old state unchanged
/// \anchor WF_N_6
template <typename feeder_T>
inline void workflow_neg_6(feeder_T& feeder1, feeder_T& feeder1A) {
    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder1);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1);

    bctxt = batch_start();
    many_update<feeder_T>(bctxt, feeder1A);
    batch_commit_fail(bctxt);

    many_read<feeder_T>(feeder1);

    // cleanup
    bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder1);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief WF_N_7
/// [ Create Set1 ] - Read - [ Update Set1 - Update Set2 ] - Read
/// Create set1 in a batch, update set1 and set2 in next batch fails leaving
/// set1 unchanged
/// \anchor WF_N_7
template <typename feeder_T>
inline void workflow_neg_7(feeder_T& feeder1, feeder_T& feeder1A,
                           feeder_T& feeder2) {
    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder1);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1);

    bctxt = batch_start();
    many_update<feeder_T>(bctxt, feeder1A);
    many_update<feeder_T>(bctxt, feeder2);
    batch_commit_fail(bctxt);

    many_read<feeder_T>(feeder1);

    // cleanup
    bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder1);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// \brief WF_N_8
/// [ Create Set1 ] - Read - [ Delete Set1 - Update Set2 ] - Read
/// Create set1 in a batch, delete set1 and update set2 in next batch
/// fails leaving set1 unchanged
/// \anchor WF_N_8
template <typename feeder_T>
inline void workflow_neg_8(feeder_T& feeder1, feeder_T& feeder2) {
    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder1);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1);

    bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder1);
    many_update<feeder_T>(bctxt, feeder2);
    batch_commit_fail(bctxt);

    many_read<feeder_T>(feeder1);

    // cleanup
    bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder1);
    batch_commit(bctxt);

    many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
}

/// @}

}    // end namespace api
}    // end namespace test

#endif    // __TEST_API_UTILS_WORKFLOW_HPP__
