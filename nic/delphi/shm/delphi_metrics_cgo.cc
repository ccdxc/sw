// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

// CGO wrappers for accessing delphi metrics

#include <stdbool.h>
#include <stdio.h>
#include <cstdint>

#include "nic/delphi/shm/delphi_metrics_cgo.h"
#include "nic/delphi/shm/delphi_metrics.hpp"

using namespace delphi::shm;

DelphiMetricsIterator_cgo NewMetricsIterator_cgo(const char *kind) {
    fprintf(stderr, "Cgo NewMetricsIterator_cgo got called for kind %s\n", kind);

    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = delphi::metrics::DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // create the table if it doesnt exist already
    MetricsCreateTable(kind);

    // FIXME: need to find a way to keep the unique_ptr alive while iterator is in use
    static delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table(kind);
    assert(tbl != NULL);

    // create the iterator
    delphi::shm::TableIterator *tbl_iter = new delphi::shm::TableIterator();
    *tbl_iter = tbl->Iterator();

    return (void *)tbl_iter;
}

bool MetricsIteratorIsNotNil_cgo(DelphiMetricsIterator_cgo iter) {
    // fprintf(stderr, "Cgo MetricsIteratorIsNotNil_cgo returning %d\n", ((delphi::shm::TableIterator *)iter)->IsNotNil());

    return ((delphi::shm::TableIterator *)iter)->IsNotNil();
}

bool MetricsIteratorIsNil_cgo(DelphiMetricsIterator_cgo iter) {
    return ((delphi::shm::TableIterator *)iter)->IsNil();
}

DelphiMetrics_cgo MetricsIteratorNext_cgo(DelphiMetricsIterator_cgo iter) {
    // get current value
    auto *mptr = ((delphi::shm::TableIterator *)iter)->Value();

    // move the iterator
    ((delphi::shm::TableIterator *)iter)->Next();

    // return current value
    return (void *)mptr;
}

DelphiMetrics_cgo MetricsIteratorFind_cgo(DelphiMetricsIterator_cgo iter, char *key, int keylen) {
    // return the current value
    auto *mptr = ((delphi::shm::TableIterator *)iter)->Find(key, keylen);
    return (void *)mptr;
}

void MetricsIteratorDelete_cgo(DelphiMetricsIterator_cgo iter, char *key, int keylen) {
    ((delphi::shm::TableIterator *)iter)->Delete(key, keylen);
}

DelphiMetrics_cgo MetricsIteratorGet_cgo(DelphiMetricsIterator_cgo iter) {
    // return the current value
    auto *mptr = ((delphi::shm::TableIterator *)iter)->Value();
    return (void *)mptr;
}

const char *MetricsEntryKey(DelphiMetrics_cgo mtr) {
    return KEY_PTR_FROM_HASH_ENTRY(HASH_ENTRY_FROM_VAL_PTR(mtr, delphi::metrics::DelphiMetrics::GetDelphiShm()));
}

const char *MetricsEntryValue(DelphiMetrics_cgo mtr) {
    return (char *)mtr;
}

int MetricsEntryKeylen(DelphiMetrics_cgo mtr) {
    return (int)(HASH_ENTRY_FROM_VAL_PTR(mtr, delphi::metrics::DelphiMetrics::GetDelphiShm())->key_len);
}

int MetricsEntryVallen(DelphiMetrics_cgo mtr) {
    return (int)(HASH_ENTRY_FROM_VAL_PTR(mtr, delphi::metrics::DelphiMetrics::GetDelphiShm())->val_len);
}

void MetricsCreateTable(const char *kind) {
    fprintf(stderr, "Cgo MetricsCreateTable got called for kind %s\n", kind);

    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = delphi::metrics::DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // create the table in shared memory
    delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table(kind);
    if (tbl == NULL) {
        tbl = shm->Kvstore()->CreateTable(kind, DEFAULT_METRIC_TBL_SIZE);
        assert(tbl != NULL);
    }

    return;
}

DelphiMetrics_cgo MetricsCreateEntry(const char *kind, char *key, int keylen, int vallen) {
    fprintf(stderr, "Cgo MetricsCreateEntry got called for kind %s, key(%d): %s\n", kind, keylen, key);

    // get the shared memory object
    delphi::shm::DelphiShmPtr shm = delphi::metrics::DelphiMetrics::GetDelphiShm();
    assert(shm != NULL);

    // get the table
    delphi::shm::TableMgrUptr tbl = shm->Kvstore()->Table(kind);
    assert(tbl != NULL);

    // create an entry
    char *valptr = (char *)tbl->Create(key, keylen, vallen);
    assert(valptr != NULL);

    return (void *)valptr;
}

int GetCounter(DelphiMetrics_cgo mtr, int offset) {
    void *ptr = (void *)((intptr_t)mtr + offset);
    return *(int *)ptr;
}

void SetCounter(DelphiMetrics_cgo mtr, int val, int offset) {
    void *ptr = (void *)((intptr_t)mtr + offset);
     *(int *)ptr = val;
}

double GetGauge(DelphiMetrics_cgo mtr, int offset) {
    void *ptr = (void *)((intptr_t)mtr + offset);
    return *(double *)ptr;
}

void SetGauge(DelphiMetrics_cgo mtr, double val, int offset) {
    void *ptr = (void *)((intptr_t)mtr + offset);
     *(double *)ptr = val;
}
