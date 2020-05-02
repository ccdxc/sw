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

#include "nic/sdk/upgrade/include/ev.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/batch.hpp"
#include "nic/apollo/api/internal/upgrade_ev.hpp"

using namespace api;

namespace test {
namespace api {

#define WF_TRACE_ERR(cond, err_str)  \
do {                                 \
    if (unlikely(!(cond))) {         \
        cerr << err_str << endl;     \
        SDK_ASSERT(cond);            \
    }                                \
} while (FALSE)

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
    sdk_ret_t ret;

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    ret = many_create<feeder_T>(bctxt, feeder);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_B1 batch1 - create one failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder, SDK_RET_OK);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_B1 batch1 - read one failed");

    bctxt = batch_start();
    ret = many_delete<feeder_T>(bctxt, feeder);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_B1 batch2 - delete one failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_B1 batch2 - read one failed");
}

/// \brief WF_B2
/// [ Create One ] - Read - [ Update One ] - Read
/// Create one object in a batch and update the same object in another batch
/// \anchor WF_B2
template <typename feeder_T>
inline void workflow_b2(feeder_T& feeder, feeder_T& feeder1A) {
    sdk_ret_t ret;

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    ret = many_create<feeder_T>(bctxt, feeder);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_B2 batch1 - create one failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder, SDK_RET_OK);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_B2 batch1 - read one failed");

    bctxt = batch_start();
    ret = many_update<feeder_T>(bctxt, feeder1A);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_B2 batch2 - update one failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1A, SDK_RET_OK);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_B2 batch2 - read one failed");

    // cleanup
    bctxt = batch_start();
    ret = many_delete<feeder_T>(bctxt, feeder1A);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_B2 cleanup - delete one failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1A, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_B2 cleanup - read one failed");
}

/// \brief WF_1
/// [ Create SetMax - Delete SetMax ] - Read
/// Create and delete max objects in the same batch. The operation should be
/// de-duped by framework and is a NO-OP from hardware perspective
/// \anchor WF_1
template <typename feeder_T>
inline void workflow_1(feeder_T& feeder) {
    sdk_ret_t ret;

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    ret = many_create<feeder_T>(bctxt, feeder);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_1 batch1 - create setmax failed");
    ret = many_delete<feeder_T>(bctxt, feeder);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_1 batch1 - delete setmax failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_1 batch1 - read setmax failed");
}

/// \brief WF_2
/// [ Create SetMax - Delete SetMax - Create SetMax ] - Read
/// Create, delete max objects and create max objects again in the same batch
/// \anchor WF_2
template <typename feeder_T>
inline void workflow_2(feeder_T& feeder) {
    sdk_ret_t ret;

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    ret = many_create<feeder_T>(bctxt, feeder);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_2 batch1 - create setmax failed");
    ret = many_delete<feeder_T>(bctxt, feeder);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_2 batch1 - delete setmax failed");
    ret = many_create<feeder_T>(bctxt, feeder);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_2 batch1 - create setmax failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder, SDK_RET_OK);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_2 batch1 - read failed");

    // cleanup
    bctxt = batch_start();
    ret = many_delete<feeder_T>(bctxt, feeder);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_2 cleanup - delete setmax failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_2 cleanup - read setmax failed");
}

/// \brief WF_3
/// [ Create Set1, Set2 - Delete Set1 - Create Set3 ] - Read
/// Create, delete some and create another set of nodes in the same batch
/// \anchor WF_3
template <typename feeder_T>
inline void workflow_3(feeder_T& feeder1, feeder_T& feeder2,
                       feeder_T& feeder3) {
    sdk_ret_t ret;

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    ret = many_create<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_3 batch1 - create set1 failed");
    ret = many_create<feeder_T>(bctxt, feeder2);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_3 batch1 - create set2 failed");
    ret = many_delete<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_3 batch1 - delete set1 failed");
    ret = many_create<feeder_T>(bctxt, feeder3);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_3 batch1 - create set3 failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_3 batch1 - read set1 failed");
    ret = many_read<feeder_T>(feeder2);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_3 batch1 - read set2 failed");
    ret = many_read<feeder_T>(feeder3);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_3 batch1 - read set3 failed");

    // cleanup
    bctxt = batch_start();
    ret = many_delete<feeder_T>(bctxt, feeder2);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_3 cleanup - delete set2 failed");
    ret = many_delete<feeder_T>(bctxt, feeder3);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_3 cleanup - delete set3 failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_3 cleanup - read set2 failed");
    ret = many_read<feeder_T>(feeder3, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_3 cleanup - read set3 failed");
}

/// \brief WF_4
/// [ Create SetMax ] - Read - [ Delete SetMax ] - Read
/// Create and delete max entries in two batches.
/// \anchor WF_4
template <typename feeder_T>
inline void workflow_4(feeder_T& feeder) {
    sdk_ret_t ret;

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    ret = many_create<feeder_T>(bctxt, feeder);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_4 batch1 - create setmax failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_4 batch1 - read setmax failed");

    bctxt = batch_start();
    ret = many_delete<feeder_T>(bctxt, feeder);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_4 batch2 - delete setmax failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_4 batch2 - read setmax failed");
}

/// \brief WF_5
/// [ Create Set1, Set2 ] - Read - [ Delete Set1 - Create Set3 ] - Read
/// Create and delete mix and match of entries in two batches
/// \anchor WF_5
template <typename feeder_T>
inline void workflow_5(feeder_T& feeder1, feeder_T& feeder2,
                       feeder_T& feeder3) {
    sdk_ret_t ret;

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    ret = many_create<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_5 batch1 - create set1 failed");
    ret = many_create<feeder_T>(bctxt, feeder2);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_5 batch1 - create set2 failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_5 batch1 - read set1 failed");
    ret = many_read<feeder_T>(feeder2);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_5 batch1 - read set2 failed");

    bctxt = batch_start();
    ret = many_delete<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_5 batch2 - delete set1 failed");
    ret = many_create<feeder_T>(bctxt, feeder3);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_5 batch2 - create set3 failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_5 batch2 - read set1 failed");
    ret = many_read<feeder_T>(feeder2);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_5 batch2 - read set2 failed");
    ret = many_read<feeder_T>(feeder3);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_5 batch2 - read set3 failed");

    // cleanup
    bctxt = batch_start();
    ret = many_delete<feeder_T>(bctxt, feeder2);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_5 cleanup - delete set2 failed");
    ret = many_delete<feeder_T>(bctxt, feeder3);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_5 cleanup - delete set3 failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_5 cleanup - read set2 failed");
    ret = many_read<feeder_T>(feeder3, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_5 cleanup - read set3 failed");
}

/// \brief WF_6
/// [ Create SetMax - Update SetMax - Update SetMax - Delete SetMax ] - Read
/// \anchor WF_6
template <typename feeder_T>
inline void workflow_6(feeder_T& feeder1, feeder_T& feeder1A,
                       feeder_T& feeder1B) {
    sdk_ret_t ret;

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    ret = many_create<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_6 batch1 - create setmax failed");
    ret = many_update<feeder_T>(bctxt, feeder1A);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_6 batch1 - update setmax failed");
    ret = many_update<feeder_T>(bctxt, feeder1B);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_6 batch1 - update setmax2 failed");
    ret = many_delete<feeder_T>(bctxt, feeder1B);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_6 batch1 - delete setmax failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1B, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_6 batch1 - read setmax failed");
}

/// \brief WF_7
/// [ Create SetMax - Delete SetMax - Create SetMax - Update SetMax] - Read -
///     [ Update SetMax ] - Read
/// \anchor WF_7
template <typename feeder_T>
inline void workflow_7(feeder_T& feeder1, feeder_T& feeder1A,
                       feeder_T& feeder1B) {
    sdk_ret_t ret;

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    ret = many_create<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_7 batch1 - create setmax failed");
    ret = many_delete<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_7 batch1 - delete setmax failed");
    ret = many_create<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_7 batch1 - create setmax2 failed");
    ret = many_update<feeder_T>(bctxt, feeder1A);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_7 batch1 - update setmax failed");
    ret = many_update<feeder_T>(bctxt, feeder1B);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_7 batch1 - update setmax2 failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1B);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_7 batch2 - read setmax failed");

    // cleanup
    bctxt = batch_start();
    ret = many_delete<feeder_T>(bctxt, feeder1B);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_7 cleanup - delete setmax failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1B, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_7 cleanup - read setmax failed");
}

/// \brief WF_8
/// [ Create SetMax - Update SetMax ] - Read - [ Update SetMax ] - Read -
///     [ Delete SetMax ] - Read
/// \anchor WF_8
template <typename feeder_T>
inline void workflow_8(feeder_T& feeder1, feeder_T& feeder1A,
                       feeder_T& feeder1B) {
    sdk_ret_t ret;

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    ret = many_create<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_8 batch1 - create setmax failed");
    ret = many_update<feeder_T>(bctxt, feeder1A);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_8 batch1 - update setmax failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1A);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_8 batch1 - read setmax failed");

    bctxt = batch_start();
    ret = many_update<feeder_T>(bctxt, feeder1B);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_8 batch2 - update setmax failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1B);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_8 batch2 - read set max failed");

    bctxt = batch_start();
    ret = many_delete<feeder_T>(bctxt, feeder1B);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_8 batch3 - delete setmax failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1B, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_8 batch1 - read setmax failed");
}

/// \brief WF_9
/// [ Create SetMax ] - Read - [ Update SetMax - Delete SetMax ] - Read
/// Create in a batch, update and delete in a batch.
/// \anchor WF_9
template <typename feeder_T>
inline void workflow_9(feeder_T& feeder1, feeder_T& feeder1A) {
    sdk_ret_t ret;

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    ret = many_create<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_9 batch1 - create setmax failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_9  batch1 - read setmax failed");

    bctxt = batch_start();
    ret = many_update<feeder_T>(bctxt, feeder1A);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_9 batch2 - update setmax failed");
    ret = many_delete<feeder_T>(bctxt, feeder1A);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_9 batch2 - delete setmax failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1A, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_9 batch2 - read setmax failed");
}

/// \brief WF_10
/// [ Create Set1, Set2, Set3 - Delete Set1 - Update Set2 ] - Read -
///     [ Update Set3 - Delete Set2 - Create Set4] - Read
/// \anchor WF_10
template <typename feeder_T>
inline void workflow_10(feeder_T& feeder1, feeder_T& feeder2,
                        feeder_T& feeder2A, feeder_T& feeder3,
                        feeder_T& feeder3A, feeder_T& feeder4) {
    sdk_ret_t ret;

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    ret = many_create<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_10 batch1 - create set1 failed");
    ret = many_create<feeder_T>(bctxt, feeder2);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_10 batch1 - create set2 failed");
    ret = many_create<feeder_T>(bctxt, feeder3);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_10 batch1 - create set3 failed");
    ret = many_delete<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_10 batch1 - delete set1 failed");
    ret = many_update<feeder_T>(bctxt, feeder2A);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_10 batch1 - update set2 failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_10 batch1 - read set1 failed");
    ret = many_read<feeder_T>(feeder2A);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_10 batch1 - read set2 failed");
    ret = many_read<feeder_T>(feeder3);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_10 batch1 - read set3 failed");

    bctxt = batch_start();
    ret = many_update<feeder_T>(bctxt, feeder3A);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_10 batch2 - update set3 failed");
    ret = many_delete<feeder_T>(bctxt, feeder2A);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_10 batch2 - delete set2 failed");
    ret = many_create<feeder_T>(bctxt, feeder4);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_10 batch2 - create set4 failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder3A);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_10 batch2 - read set3 failed");
    ret = many_read<feeder_T>(feeder2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_10 batch2 - read set2 failed");
    ret = many_read<feeder_T>(feeder4);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_10 batch2 - read set4 failed");

    // cleanup
    bctxt = batch_start();
    ret = many_delete<feeder_T>(bctxt, feeder4);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_10 cleanup - delete set4 failed");
    ret = many_delete<feeder_T>(bctxt, feeder3A);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_10 cleanup - delete set3 failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder4, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_10 cleanup - read set4 failed");
    ret = many_read<feeder_T>(feeder3A, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_10 cleanup - read set3 failed");
}

/// \brief WF_N1
/// [ Create SetMax ] - [ Create SetMax ] - Read
/// \anchor WF_N_1
template <typename feeder_T>
inline void workflow_neg_1(feeder_T& feeder) {
    sdk_ret_t ret;

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    ret = many_create<feeder_T>(bctxt, feeder);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N1 batch1 - create setmax failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N1 batch1 - read setmax failed");

    bctxt = batch_start();
    ret = many_create<feeder_T>(bctxt, feeder);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N1 batch2 - create setmax failed");
    batch_commit_fail(bctxt);

    ret = many_read<feeder_T>(feeder);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N1 batch2 - read setmax failed");

    // cleanup
    bctxt = batch_start();
    ret = many_delete<feeder_T>(bctxt, feeder);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N1 cleanup - delete setmax failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N1 cleanup - read setmax failed");
}

/// \brief WF_N_2
/// [ Create SetMax+1 ] - Read
/// \anchor WF_N_2
template <typename feeder_T>
inline void workflow_neg_2(feeder_T& feeder) {
    sdk_ret_t ret;

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    ret = many_create<feeder_T>(bctxt, feeder);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N2 batch1 - create setmax+1 failed");
    batch_commit_fail(bctxt);

    ret = many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N2 batch1 - read setmax+1 failed");
}

/// \brief WF_N_3
/// Read NonEx - [ Delete NonExMax ] - Read - [ Update NonExMax ] - Read
/// \anchor WF_N_3
template <typename feeder_T>
inline void workflow_neg_3(feeder_T& feeder) {
    sdk_ret_t ret;

    // trigger
    ret = many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N3 - read nonex failed");

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    ret = many_delete<feeder_T>(bctxt, feeder);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N3 batch1 - delete nonexmax failed");
    batch_commit_fail(bctxt);

    ret = many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N3 batch1 - read nonexmax failed");

    // trigger
    bctxt = batch_start();
    ret = many_update<feeder_T>(bctxt, feeder);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N3 batch2 - update nonexmax failed");
    batch_commit_fail(bctxt);

    ret = many_read<feeder_T>(feeder, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N3 batch2 - read nonexmax failed");
}

/// \brief WF_N_4
/// [ Create Set1 ] - Read - [ Delete Set1, Set2 ] - Read
/// \anchor WF_N_4
template <typename feeder_T>
inline void workflow_neg_4(feeder_T& feeder1, feeder_T& feeder2) {
    sdk_ret_t ret;

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    ret = many_create<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N4 batch1 - create set1 failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N4 batch1 - read set1 failed");

    bctxt = batch_start();
    ret = many_delete<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N4 batch2 - delete set1 failed");
    ret = many_delete<feeder_T>(bctxt, feeder2);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N4 batch2 - delete set2 failed");
    batch_commit_fail(bctxt);

    ret = many_read<feeder_T>(feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N4 batch2 - read set1 failed");
    ret = many_read<feeder_T>(feeder2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N4 batch2 - read set2 failed");

    // cleanup
    bctxt = batch_start();
    ret = many_delete<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N4 cleanup - delete set1 failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder2, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N4 cleanup - read set1 failed");
}

/// \brief WF_N_5
/// [ Create SetMax ] - Read - [ Delete SetMax - Update SetMax ] - Read
/// \anchor WF_N_5
template <typename feeder_T>
inline void workflow_neg_5(feeder_T& feeder1, feeder_T& feeder1A) {
    sdk_ret_t ret;

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    ret = many_create<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N5 batch1 - create setmax failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N5 batch1 - read setmax failed");

    bctxt = batch_start();
    ret = many_delete<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N5 batch2 - delete setmax failed");
    ret = many_update<feeder_T>(bctxt, feeder1A);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N5 batch2 - update setmax failed");
    batch_commit_fail(bctxt);

    ret = many_read<feeder_T>(feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N5 batch2 - read setmax failed");

    // cleanup
    bctxt = batch_start();
    ret = many_delete<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N5 cleanup - delete setmax failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N5 cleanup - read setmax failed");
}

/// \brief WF_N_6
/// [ Create SetMax ] - Read - [ Update SetMax + 1 ] - Read
/// \anchor WF_N_6
template <typename feeder_T>
inline void workflow_neg_6(feeder_T& feeder1, feeder_T& feeder1A) {
    sdk_ret_t ret;

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    ret = many_create<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N6 batch1 - create setmax failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N6 batch1 - read setmax failed");

    bctxt = batch_start();
    ret = many_update<feeder_T>(bctxt, feeder1A);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N6 batch2 - update setmax+1 failed");
    batch_commit_fail(bctxt);

    ret = many_read<feeder_T>(feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N6 batch2 - read setmax failed");

    // cleanup
    bctxt = batch_start();
    ret = many_delete<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N6 cleanup - delete setmax failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N6 cleanup - read setmax failed");
}

/// \brief WF_N_7
/// [ Create Set1 ] - Read - [ Update Set1 - Update Set2 ] - Read
/// \anchor WF_N_7
template <typename feeder_T>
inline void workflow_neg_7(feeder_T& feeder1, feeder_T& feeder1A,
                           feeder_T& feeder2) {
    sdk_ret_t ret;

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    ret = many_create<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N7 batch1 - create set1 failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N7 batch1 - read set1 failed");

    bctxt = batch_start();
    ret = many_update<feeder_T>(bctxt, feeder1A);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N7 batch2 - update set1 failed");
    ret = many_update<feeder_T>(bctxt, feeder2);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N7 batch2 - update set2 failed");
    batch_commit_fail(bctxt);

    ret = many_read<feeder_T>(feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N7 batch2 - read set1 failed");

    // cleanup
    bctxt = batch_start();
    ret = many_delete<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N7 cleanup - delete set1 failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N7 cleanup - read set1 failed");
}

/// \brief WF_N_8
/// [ Create Set1 ] - Read - [ Delete Set1 - Update Set2 ] - Read
/// \anchor WF_N_8
template <typename feeder_T>
inline void workflow_neg_8(feeder_T& feeder1, feeder_T& feeder2) {
    sdk_ret_t ret;

    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    ret = many_create<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N8 batch1 - create set1 failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N8 batch1 - read set1 failed");

    bctxt = batch_start();
    ret = many_delete<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N8 batch2 - create set1 failed");
    ret = many_update<feeder_T>(bctxt, feeder2);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N8 batch2 - update set2 failed");
    batch_commit_fail(bctxt);

    ret = many_read<feeder_T>(feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N8 batch2 - read set1 failed");

    // cleanup
    bctxt = batch_start();
    ret = many_delete<feeder_T>(bctxt, feeder1);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N8 cleanup - delete set1 failed");
    batch_commit(bctxt);

    ret = many_read<feeder_T>(feeder1, sdk::SDK_RET_ENTRY_NOT_FOUND);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_N8 cleanup - read set1 failed");
}

/// \brief WF_U_1
/// [ Create Set1 ] - Read - Backup - [ Delete Set1 ] -
/// Restore - [ Create Set1 ] - Read_Compare
/// Create set1 in a batch, Read and Save the obj info
/// Delete set1, restore the saved obj info. Create set1 again
/// Read obj info and comapre it with restored obj
template <typename feeder_T>
inline void workflow_u1(feeder_T& feeder1)
{
    sdk_ret_t ret;
    upg_mode_t mode;

    mode = upg_mode_t::UPGRADE_MODE_HITLESS;
    // trigger
    pds_batch_ctxt_t bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder1);
    batch_commit(bctxt);
    many_read<feeder_T>(feeder1);

    // bakcup the objs
    ret = upg_obj_backup(mode);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_U1 batch1 - upg backup failed");

    bctxt = batch_start();
    many_delete<feeder_T>(bctxt, feeder1);
    batch_commit(bctxt);

    // restore the objs
    ret = upg_obj_restore(mode);
    WF_TRACE_ERR((ret == SDK_RET_OK), "WF_U1 batch1 - upg restore failed");

    // config replay
    bctxt = batch_start();
    many_create<feeder_T>(bctxt, feeder1);
    batch_commit(bctxt);

    // compare read values with stashed during backup
    many_read_cmp<feeder_T>(feeder1);
}

/// @}

}    // end namespace api
}    // end namespace test

#endif    // __TEST_API_UTILS_WORKFLOW_HPP__
