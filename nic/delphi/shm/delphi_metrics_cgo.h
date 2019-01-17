// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

// CGO wrappers for accessing delphi metrics

#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// c++ objects converted to void * for cgo
typedef void *DelphiMetrics_cgo;
typedef void *DelphiMetricsIterator_cgo;
typedef void *DelphiMetricsTable_cgo;

DelphiMetricsIterator_cgo NewMetricsIterator_cgo(const char *kind);

bool MetricsIteratorIsNotNil_cgo(DelphiMetricsIterator_cgo iter);
bool MetricsIteratorIsNil_cgo(DelphiMetricsIterator_cgo iter);

DelphiMetrics_cgo MetricsIteratorNext_cgo(DelphiMetricsIterator_cgo iter);

DelphiMetrics_cgo MetricsIteratorGet_cgo(DelphiMetricsIterator_cgo iter);
DelphiMetrics_cgo MetricsIteratorFind_cgo(DelphiMetricsIterator_cgo iter, char *key, int keylen);
void MetricsIteratorDelete_cgo(DelphiMetricsIterator_cgo iter, char *key, int keylen);
void MetricsIteratorFree_cgo(DelphiMetricsIterator_cgo metr);


void MetricsCreateTable(const char *kind);
DelphiMetrics_cgo MetricsCreateEntry(const char *kind, char *key, int keylen, int vallen);
const char *MetricsEntryKey(DelphiMetrics_cgo mtr);
const char *MetricsEntryValue(DelphiMetrics_cgo mtr);
int MetricsEntryKeylen(DelphiMetrics_cgo mtr);
int MetricsEntryVallen(DelphiMetrics_cgo mtr);

int GetCounter(DelphiMetrics_cgo mtr, int offset);
void SetCounter(DelphiMetrics_cgo mtr, int val, int offset);
double GetGauge(DelphiMetrics_cgo mtr, int offset);
void SetGauge(DelphiMetrics_cgo mtr, double val, int offset);

void DelphiMetricsInit_cgo();

#ifdef __cplusplus
}
#endif
