// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package gometrics

// #cgo CFLAGS: -I${SRCDIR} -I${SRCDIR}/../../../../
// #cgo CXXFLAGS: -std=c++11
// #cgo LDFLAGS: ${SRCDIR}/../../../build/aarch64/iris/out/libdelphishm_lib/libdelphishm.a ${SRCDIR}/../../../build/aarch64/iris/out/libdelphiutils_lib/libdelphiutils.a -L${SRCDIR}/../../../build/aarch64/iris/lib -L${SRCDIR}/../../../build/aarch64/iris/lib/ -lstdc++ -lm -lrt -ldl -lsdkpal -llogger -lprotobuf
// #include "nic/delphi/shm/delphi_metrics_cgo.h"
import "C"
import (
	"errors"
	"fmt"
	"unsafe"

	"github.com/pensando/sw/venice/utils/tsdb/metrics"
)

// delphiMetricsIterator keeps internal state of the metrics iterator
type delphiMetricsIterator struct {
	kind     string // metrics kind
	cgo_iter C.DelphiMetricsIterator_cgo
}

type delphiMetrics struct {
	entry  C.DelphiMetrics_cgo
	keylen int
	vallen int
}

// newMetricsIterator returns new metrics iterator for a kind
func newMetricsIterator(kind string) (MetricsIterator, error) {
	// mount shared memory if its not already mounted

	// cgo wrapper to get metrics iterator
	iter := delphiMetricsIterator{
		kind:     kind,
		cgo_iter: C.NewMetricsIterator_cgo(C.CString(kind)),
	}

	// return the iterator interface
	return &iter, nil
}

// Find returns a metrics by its key
func (it *delphiMetricsIterator) Find(key []byte) (Metrics, error) {
	mtr := C.MetricsIteratorFind_cgo(it.cgo_iter, C.CString(string(key)), C.int(len(key)))
	if mtr == nil {
		return nil, errors.New("Could not find the key")
	}

	// construct the object
	entry := delphiMetrics{
		entry:  mtr,
		keylen: int(C.MetricsEntryKeylen(mtr)),
		vallen: int(C.MetricsEntryVallen(mtr)),
	}

	return &entry, nil

}

// HasNext returns true if iterator has next element
func (it *delphiMetricsIterator) HasNext() bool {
	return bool(C.MetricsIteratorIsNotNil_cgo(it.cgo_iter))
}

// Next returns next element in the iterator
func (it *delphiMetricsIterator) Next() Metrics {
	if it.HasNext() {
		mtr := C.MetricsIteratorNext_cgo(it.cgo_iter)
		entry := delphiMetrics{
			entry:  mtr,
			keylen: int(C.MetricsEntryKeylen(mtr)),
			vallen: int(C.MetricsEntryVallen(mtr)),
		}

		return &entry
	}
	return nil
}

func (it *delphiMetricsIterator) Create(key []byte, length int) Metrics {

	return &delphiMetrics{
		entry:  C.MetricsCreateEntry(C.CString(it.kind), C.CString(string(key)), C.int(len(key)), C.int(length)),
		keylen: len(key),
		vallen: length,
	}
}

func (it *delphiMetricsIterator) Delete(key []byte) error {
	C.MetricsIteratorDelete_cgo(it.cgo_iter, C.CString(string(key)), C.int(len(key)))
	return nil
}

func (it *delphiMetricsIterator) Free() {
	C.MetricsIteratorFree_cgo(it.cgo_iter)
}

func (m *delphiMetrics) String() string {
	kBytes := C.GoBytes(unsafe.Pointer(C.MetricsEntryKey(m.entry)), C.int(m.keylen))
	retstr := ""

	for i := 0; i < m.keylen; i++ {
		retstr += fmt.Sprintf("%d ", kBytes[i])
	}

	return retstr
}

// returns the key for the metrics
func (m *delphiMetrics) GetKey() []byte {
	return C.GoBytes(unsafe.Pointer(C.MetricsEntryKey(m.entry)), C.MetricsEntryKeylen(m.entry))
}

func (m *delphiMetrics) GetCounter(offset int) metrics.Counter {
	return metrics.Counter(C.GetCounter(m.entry, C.int(offset)))
}

func (m *delphiMetrics) SetCounter(cntr metrics.Counter, offset int) {
	C.SetCounter(m.entry, C.longlong(cntr), C.int(offset))
}

func (m *delphiMetrics) GetGauge(offset int) metrics.Gauge {
	return metrics.Gauge(C.GetGauge(m.entry, C.int(offset)))
}

func (m *delphiMetrics) SetGauge(gge metrics.Gauge, offset int) {
	C.SetGauge(m.entry, C.double(gge), C.int(offset))
}

func init() {
	C.DelphiMetricsInit_cgo()
}
