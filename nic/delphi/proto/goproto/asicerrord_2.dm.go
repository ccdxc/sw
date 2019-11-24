// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
	"github.com/pensando/sw/venice/utils/tsdb/metrics"
)

type Dprdpr0intflopfifo0Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	DataMuxForceBypassCrcFlopFfOvflow metrics.Counter

	DprCrcInfoFlopFfOvflow metrics.Counter

	DprCrcUpdateInfoFlopFfOvflow metrics.Counter

	DprCsumInfoFlopFfOvflow metrics.Counter

	DataMuxForceBypassCsumFlopFfOvflow metrics.Counter

	DprCsumUpdateInfoFlopFfOvflow metrics.Counter

	PtrEarlyPktEopInfoFlopFfOvflow metrics.Counter

	DataMuxEopErrFlopFfOvflow metrics.Counter

	PktinEopErrFlopFfOvflow metrics.Counter

	CsumErrFlopFfOvflow metrics.Counter

	CrcErrFlopFfOvflow metrics.Counter

	DataMuxDropFlopFfOvflow metrics.Counter

	PhvPktDataFlopFfOvflow metrics.Counter

	PktoutLenCellFlopFfOvflow metrics.Counter

	PaddingSizeFlopFfOvflow metrics.Counter

	PktinErrFlopFfOvflow metrics.Counter

	PhvNoDataFlopFfOvflow metrics.Counter

	PtrLookahaedFlopFfOvflow metrics.Counter

	EopVldFlopFfOvflow metrics.Counter

	CsumCalVldFlopFfOvflow metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dprdpr0intflopfifo0Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dprdpr0intflopfifo0Metrics) Size() int {
	sz := 0

	sz += mtr.DataMuxForceBypassCrcFlopFfOvflow.Size()

	sz += mtr.DprCrcInfoFlopFfOvflow.Size()

	sz += mtr.DprCrcUpdateInfoFlopFfOvflow.Size()

	sz += mtr.DprCsumInfoFlopFfOvflow.Size()

	sz += mtr.DataMuxForceBypassCsumFlopFfOvflow.Size()

	sz += mtr.DprCsumUpdateInfoFlopFfOvflow.Size()

	sz += mtr.PtrEarlyPktEopInfoFlopFfOvflow.Size()

	sz += mtr.DataMuxEopErrFlopFfOvflow.Size()

	sz += mtr.PktinEopErrFlopFfOvflow.Size()

	sz += mtr.CsumErrFlopFfOvflow.Size()

	sz += mtr.CrcErrFlopFfOvflow.Size()

	sz += mtr.DataMuxDropFlopFfOvflow.Size()

	sz += mtr.PhvPktDataFlopFfOvflow.Size()

	sz += mtr.PktoutLenCellFlopFfOvflow.Size()

	sz += mtr.PaddingSizeFlopFfOvflow.Size()

	sz += mtr.PktinErrFlopFfOvflow.Size()

	sz += mtr.PhvNoDataFlopFfOvflow.Size()

	sz += mtr.PtrLookahaedFlopFfOvflow.Size()

	sz += mtr.EopVldFlopFfOvflow.Size()

	sz += mtr.CsumCalVldFlopFfOvflow.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.DataMuxForceBypassCrcFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DataMuxForceBypassCrcFlopFfOvflow.Size()

	mtr.DprCrcInfoFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DprCrcInfoFlopFfOvflow.Size()

	mtr.DprCrcUpdateInfoFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DprCrcUpdateInfoFlopFfOvflow.Size()

	mtr.DprCsumInfoFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DprCsumInfoFlopFfOvflow.Size()

	mtr.DataMuxForceBypassCsumFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DataMuxForceBypassCsumFlopFfOvflow.Size()

	mtr.DprCsumUpdateInfoFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DprCsumUpdateInfoFlopFfOvflow.Size()

	mtr.PtrEarlyPktEopInfoFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PtrEarlyPktEopInfoFlopFfOvflow.Size()

	mtr.DataMuxEopErrFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DataMuxEopErrFlopFfOvflow.Size()

	mtr.PktinEopErrFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktinEopErrFlopFfOvflow.Size()

	mtr.CsumErrFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CsumErrFlopFfOvflow.Size()

	mtr.CrcErrFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CrcErrFlopFfOvflow.Size()

	mtr.DataMuxDropFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DataMuxDropFlopFfOvflow.Size()

	mtr.PhvPktDataFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvPktDataFlopFfOvflow.Size()

	mtr.PktoutLenCellFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktoutLenCellFlopFfOvflow.Size()

	mtr.PaddingSizeFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PaddingSizeFlopFfOvflow.Size()

	mtr.PktinErrFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktinErrFlopFfOvflow.Size()

	mtr.PhvNoDataFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvNoDataFlopFfOvflow.Size()

	mtr.PtrLookahaedFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PtrLookahaedFlopFfOvflow.Size()

	mtr.EopVldFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.EopVldFlopFfOvflow.Size()

	mtr.CsumCalVldFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CsumCalVldFlopFfOvflow.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "DataMuxForceBypassCrcFlopFfOvflow" {
		return offset
	}
	offset += mtr.DataMuxForceBypassCrcFlopFfOvflow.Size()

	if fldName == "DprCrcInfoFlopFfOvflow" {
		return offset
	}
	offset += mtr.DprCrcInfoFlopFfOvflow.Size()

	if fldName == "DprCrcUpdateInfoFlopFfOvflow" {
		return offset
	}
	offset += mtr.DprCrcUpdateInfoFlopFfOvflow.Size()

	if fldName == "DprCsumInfoFlopFfOvflow" {
		return offset
	}
	offset += mtr.DprCsumInfoFlopFfOvflow.Size()

	if fldName == "DataMuxForceBypassCsumFlopFfOvflow" {
		return offset
	}
	offset += mtr.DataMuxForceBypassCsumFlopFfOvflow.Size()

	if fldName == "DprCsumUpdateInfoFlopFfOvflow" {
		return offset
	}
	offset += mtr.DprCsumUpdateInfoFlopFfOvflow.Size()

	if fldName == "PtrEarlyPktEopInfoFlopFfOvflow" {
		return offset
	}
	offset += mtr.PtrEarlyPktEopInfoFlopFfOvflow.Size()

	if fldName == "DataMuxEopErrFlopFfOvflow" {
		return offset
	}
	offset += mtr.DataMuxEopErrFlopFfOvflow.Size()

	if fldName == "PktinEopErrFlopFfOvflow" {
		return offset
	}
	offset += mtr.PktinEopErrFlopFfOvflow.Size()

	if fldName == "CsumErrFlopFfOvflow" {
		return offset
	}
	offset += mtr.CsumErrFlopFfOvflow.Size()

	if fldName == "CrcErrFlopFfOvflow" {
		return offset
	}
	offset += mtr.CrcErrFlopFfOvflow.Size()

	if fldName == "DataMuxDropFlopFfOvflow" {
		return offset
	}
	offset += mtr.DataMuxDropFlopFfOvflow.Size()

	if fldName == "PhvPktDataFlopFfOvflow" {
		return offset
	}
	offset += mtr.PhvPktDataFlopFfOvflow.Size()

	if fldName == "PktoutLenCellFlopFfOvflow" {
		return offset
	}
	offset += mtr.PktoutLenCellFlopFfOvflow.Size()

	if fldName == "PaddingSizeFlopFfOvflow" {
		return offset
	}
	offset += mtr.PaddingSizeFlopFfOvflow.Size()

	if fldName == "PktinErrFlopFfOvflow" {
		return offset
	}
	offset += mtr.PktinErrFlopFfOvflow.Size()

	if fldName == "PhvNoDataFlopFfOvflow" {
		return offset
	}
	offset += mtr.PhvNoDataFlopFfOvflow.Size()

	if fldName == "PtrLookahaedFlopFfOvflow" {
		return offset
	}
	offset += mtr.PtrLookahaedFlopFfOvflow.Size()

	if fldName == "EopVldFlopFfOvflow" {
		return offset
	}
	offset += mtr.EopVldFlopFfOvflow.Size()

	if fldName == "CsumCalVldFlopFfOvflow" {
		return offset
	}
	offset += mtr.CsumCalVldFlopFfOvflow.Size()

	return offset
}

// SetDataMuxForceBypassCrcFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) SetDataMuxForceBypassCrcFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DataMuxForceBypassCrcFlopFfOvflow"))
	return nil
}

// SetDprCrcInfoFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) SetDprCrcInfoFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCrcInfoFlopFfOvflow"))
	return nil
}

// SetDprCrcUpdateInfoFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) SetDprCrcUpdateInfoFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCrcUpdateInfoFlopFfOvflow"))
	return nil
}

// SetDprCsumInfoFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) SetDprCsumInfoFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCsumInfoFlopFfOvflow"))
	return nil
}

// SetDataMuxForceBypassCsumFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) SetDataMuxForceBypassCsumFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DataMuxForceBypassCsumFlopFfOvflow"))
	return nil
}

// SetDprCsumUpdateInfoFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) SetDprCsumUpdateInfoFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCsumUpdateInfoFlopFfOvflow"))
	return nil
}

// SetPtrEarlyPktEopInfoFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) SetPtrEarlyPktEopInfoFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PtrEarlyPktEopInfoFlopFfOvflow"))
	return nil
}

// SetDataMuxEopErrFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) SetDataMuxEopErrFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DataMuxEopErrFlopFfOvflow"))
	return nil
}

// SetPktinEopErrFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) SetPktinEopErrFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktinEopErrFlopFfOvflow"))
	return nil
}

// SetCsumErrFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) SetCsumErrFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CsumErrFlopFfOvflow"))
	return nil
}

// SetCrcErrFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) SetCrcErrFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CrcErrFlopFfOvflow"))
	return nil
}

// SetDataMuxDropFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) SetDataMuxDropFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DataMuxDropFlopFfOvflow"))
	return nil
}

// SetPhvPktDataFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) SetPhvPktDataFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvPktDataFlopFfOvflow"))
	return nil
}

// SetPktoutLenCellFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) SetPktoutLenCellFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktoutLenCellFlopFfOvflow"))
	return nil
}

// SetPaddingSizeFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) SetPaddingSizeFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PaddingSizeFlopFfOvflow"))
	return nil
}

// SetPktinErrFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) SetPktinErrFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktinErrFlopFfOvflow"))
	return nil
}

// SetPhvNoDataFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) SetPhvNoDataFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvNoDataFlopFfOvflow"))
	return nil
}

// SetPtrLookahaedFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) SetPtrLookahaedFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PtrLookahaedFlopFfOvflow"))
	return nil
}

// SetEopVldFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) SetEopVldFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopVldFlopFfOvflow"))
	return nil
}

// SetCsumCalVldFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo0Metrics) SetCsumCalVldFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CsumCalVldFlopFfOvflow"))
	return nil
}

// Dprdpr0intflopfifo0MetricsIterator is the iterator object
type Dprdpr0intflopfifo0MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dprdpr0intflopfifo0MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dprdpr0intflopfifo0MetricsIterator) Next() *Dprdpr0intflopfifo0Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dprdpr0intflopfifo0Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dprdpr0intflopfifo0MetricsIterator) Find(key uint64) (*Dprdpr0intflopfifo0Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dprdpr0intflopfifo0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dprdpr0intflopfifo0MetricsIterator) Create(key uint64) (*Dprdpr0intflopfifo0Metrics, error) {
	tmtr := &Dprdpr0intflopfifo0Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dprdpr0intflopfifo0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dprdpr0intflopfifo0MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dprdpr0intflopfifo0MetricsIterator) Free() {
	it.iter.Free()
}

// NewDprdpr0intflopfifo0MetricsIterator returns an iterator
func NewDprdpr0intflopfifo0MetricsIterator() (*Dprdpr0intflopfifo0MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dprdpr0intflopfifo0Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dprdpr0intflopfifo0MetricsIterator{iter: iter}, nil
}

type Dprdpr0intflopfifo1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	DataMuxForceBypassCrcFlopFfUndflow metrics.Counter

	DprCrcInfoFlopFfUndflow metrics.Counter

	DprCrcUpdateInfoFlopFfUndflow metrics.Counter

	DprCsumInfoFlopFfUndflow metrics.Counter

	DataMuxForceBypassCsumFlopFfUndflow metrics.Counter

	DprCsumUpdateInfoFlopFfUndflow metrics.Counter

	PtrEarlyPktEopInfoFlopFfUndflow metrics.Counter

	DataMuxEopErrFlopFfUndflow metrics.Counter

	PktinEopErrFlopFfUndflow metrics.Counter

	CsumErrFlopFfUndflow metrics.Counter

	CrcErrFlopFfUndflow metrics.Counter

	DataMuxDropFlopFfUndflow metrics.Counter

	PhvPktDataFlopFfUndflow metrics.Counter

	PktoutLenCellFlopFfUndflow metrics.Counter

	PaddingSizeFlopFfUndflow metrics.Counter

	PktinErrFlopFfUndflow metrics.Counter

	PhvNoDataFlopFfUndflow metrics.Counter

	PtrLookahaedFlopFfUndflow metrics.Counter

	EopVldFlopFfUndflow metrics.Counter

	CsumCalVldFlopFfUndflow metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dprdpr0intflopfifo1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dprdpr0intflopfifo1Metrics) Size() int {
	sz := 0

	sz += mtr.DataMuxForceBypassCrcFlopFfUndflow.Size()

	sz += mtr.DprCrcInfoFlopFfUndflow.Size()

	sz += mtr.DprCrcUpdateInfoFlopFfUndflow.Size()

	sz += mtr.DprCsumInfoFlopFfUndflow.Size()

	sz += mtr.DataMuxForceBypassCsumFlopFfUndflow.Size()

	sz += mtr.DprCsumUpdateInfoFlopFfUndflow.Size()

	sz += mtr.PtrEarlyPktEopInfoFlopFfUndflow.Size()

	sz += mtr.DataMuxEopErrFlopFfUndflow.Size()

	sz += mtr.PktinEopErrFlopFfUndflow.Size()

	sz += mtr.CsumErrFlopFfUndflow.Size()

	sz += mtr.CrcErrFlopFfUndflow.Size()

	sz += mtr.DataMuxDropFlopFfUndflow.Size()

	sz += mtr.PhvPktDataFlopFfUndflow.Size()

	sz += mtr.PktoutLenCellFlopFfUndflow.Size()

	sz += mtr.PaddingSizeFlopFfUndflow.Size()

	sz += mtr.PktinErrFlopFfUndflow.Size()

	sz += mtr.PhvNoDataFlopFfUndflow.Size()

	sz += mtr.PtrLookahaedFlopFfUndflow.Size()

	sz += mtr.EopVldFlopFfUndflow.Size()

	sz += mtr.CsumCalVldFlopFfUndflow.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.DataMuxForceBypassCrcFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DataMuxForceBypassCrcFlopFfUndflow.Size()

	mtr.DprCrcInfoFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DprCrcInfoFlopFfUndflow.Size()

	mtr.DprCrcUpdateInfoFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DprCrcUpdateInfoFlopFfUndflow.Size()

	mtr.DprCsumInfoFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DprCsumInfoFlopFfUndflow.Size()

	mtr.DataMuxForceBypassCsumFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DataMuxForceBypassCsumFlopFfUndflow.Size()

	mtr.DprCsumUpdateInfoFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DprCsumUpdateInfoFlopFfUndflow.Size()

	mtr.PtrEarlyPktEopInfoFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PtrEarlyPktEopInfoFlopFfUndflow.Size()

	mtr.DataMuxEopErrFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DataMuxEopErrFlopFfUndflow.Size()

	mtr.PktinEopErrFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktinEopErrFlopFfUndflow.Size()

	mtr.CsumErrFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CsumErrFlopFfUndflow.Size()

	mtr.CrcErrFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CrcErrFlopFfUndflow.Size()

	mtr.DataMuxDropFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DataMuxDropFlopFfUndflow.Size()

	mtr.PhvPktDataFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvPktDataFlopFfUndflow.Size()

	mtr.PktoutLenCellFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktoutLenCellFlopFfUndflow.Size()

	mtr.PaddingSizeFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PaddingSizeFlopFfUndflow.Size()

	mtr.PktinErrFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktinErrFlopFfUndflow.Size()

	mtr.PhvNoDataFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvNoDataFlopFfUndflow.Size()

	mtr.PtrLookahaedFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PtrLookahaedFlopFfUndflow.Size()

	mtr.EopVldFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.EopVldFlopFfUndflow.Size()

	mtr.CsumCalVldFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CsumCalVldFlopFfUndflow.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "DataMuxForceBypassCrcFlopFfUndflow" {
		return offset
	}
	offset += mtr.DataMuxForceBypassCrcFlopFfUndflow.Size()

	if fldName == "DprCrcInfoFlopFfUndflow" {
		return offset
	}
	offset += mtr.DprCrcInfoFlopFfUndflow.Size()

	if fldName == "DprCrcUpdateInfoFlopFfUndflow" {
		return offset
	}
	offset += mtr.DprCrcUpdateInfoFlopFfUndflow.Size()

	if fldName == "DprCsumInfoFlopFfUndflow" {
		return offset
	}
	offset += mtr.DprCsumInfoFlopFfUndflow.Size()

	if fldName == "DataMuxForceBypassCsumFlopFfUndflow" {
		return offset
	}
	offset += mtr.DataMuxForceBypassCsumFlopFfUndflow.Size()

	if fldName == "DprCsumUpdateInfoFlopFfUndflow" {
		return offset
	}
	offset += mtr.DprCsumUpdateInfoFlopFfUndflow.Size()

	if fldName == "PtrEarlyPktEopInfoFlopFfUndflow" {
		return offset
	}
	offset += mtr.PtrEarlyPktEopInfoFlopFfUndflow.Size()

	if fldName == "DataMuxEopErrFlopFfUndflow" {
		return offset
	}
	offset += mtr.DataMuxEopErrFlopFfUndflow.Size()

	if fldName == "PktinEopErrFlopFfUndflow" {
		return offset
	}
	offset += mtr.PktinEopErrFlopFfUndflow.Size()

	if fldName == "CsumErrFlopFfUndflow" {
		return offset
	}
	offset += mtr.CsumErrFlopFfUndflow.Size()

	if fldName == "CrcErrFlopFfUndflow" {
		return offset
	}
	offset += mtr.CrcErrFlopFfUndflow.Size()

	if fldName == "DataMuxDropFlopFfUndflow" {
		return offset
	}
	offset += mtr.DataMuxDropFlopFfUndflow.Size()

	if fldName == "PhvPktDataFlopFfUndflow" {
		return offset
	}
	offset += mtr.PhvPktDataFlopFfUndflow.Size()

	if fldName == "PktoutLenCellFlopFfUndflow" {
		return offset
	}
	offset += mtr.PktoutLenCellFlopFfUndflow.Size()

	if fldName == "PaddingSizeFlopFfUndflow" {
		return offset
	}
	offset += mtr.PaddingSizeFlopFfUndflow.Size()

	if fldName == "PktinErrFlopFfUndflow" {
		return offset
	}
	offset += mtr.PktinErrFlopFfUndflow.Size()

	if fldName == "PhvNoDataFlopFfUndflow" {
		return offset
	}
	offset += mtr.PhvNoDataFlopFfUndflow.Size()

	if fldName == "PtrLookahaedFlopFfUndflow" {
		return offset
	}
	offset += mtr.PtrLookahaedFlopFfUndflow.Size()

	if fldName == "EopVldFlopFfUndflow" {
		return offset
	}
	offset += mtr.EopVldFlopFfUndflow.Size()

	if fldName == "CsumCalVldFlopFfUndflow" {
		return offset
	}
	offset += mtr.CsumCalVldFlopFfUndflow.Size()

	return offset
}

// SetDataMuxForceBypassCrcFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) SetDataMuxForceBypassCrcFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DataMuxForceBypassCrcFlopFfUndflow"))
	return nil
}

// SetDprCrcInfoFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) SetDprCrcInfoFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCrcInfoFlopFfUndflow"))
	return nil
}

// SetDprCrcUpdateInfoFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) SetDprCrcUpdateInfoFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCrcUpdateInfoFlopFfUndflow"))
	return nil
}

// SetDprCsumInfoFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) SetDprCsumInfoFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCsumInfoFlopFfUndflow"))
	return nil
}

// SetDataMuxForceBypassCsumFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) SetDataMuxForceBypassCsumFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DataMuxForceBypassCsumFlopFfUndflow"))
	return nil
}

// SetDprCsumUpdateInfoFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) SetDprCsumUpdateInfoFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCsumUpdateInfoFlopFfUndflow"))
	return nil
}

// SetPtrEarlyPktEopInfoFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) SetPtrEarlyPktEopInfoFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PtrEarlyPktEopInfoFlopFfUndflow"))
	return nil
}

// SetDataMuxEopErrFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) SetDataMuxEopErrFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DataMuxEopErrFlopFfUndflow"))
	return nil
}

// SetPktinEopErrFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) SetPktinEopErrFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktinEopErrFlopFfUndflow"))
	return nil
}

// SetCsumErrFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) SetCsumErrFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CsumErrFlopFfUndflow"))
	return nil
}

// SetCrcErrFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) SetCrcErrFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CrcErrFlopFfUndflow"))
	return nil
}

// SetDataMuxDropFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) SetDataMuxDropFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DataMuxDropFlopFfUndflow"))
	return nil
}

// SetPhvPktDataFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) SetPhvPktDataFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvPktDataFlopFfUndflow"))
	return nil
}

// SetPktoutLenCellFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) SetPktoutLenCellFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktoutLenCellFlopFfUndflow"))
	return nil
}

// SetPaddingSizeFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) SetPaddingSizeFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PaddingSizeFlopFfUndflow"))
	return nil
}

// SetPktinErrFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) SetPktinErrFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktinErrFlopFfUndflow"))
	return nil
}

// SetPhvNoDataFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) SetPhvNoDataFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvNoDataFlopFfUndflow"))
	return nil
}

// SetPtrLookahaedFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) SetPtrLookahaedFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PtrLookahaedFlopFfUndflow"))
	return nil
}

// SetEopVldFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) SetEopVldFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopVldFlopFfUndflow"))
	return nil
}

// SetCsumCalVldFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr0intflopfifo1Metrics) SetCsumCalVldFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CsumCalVldFlopFfUndflow"))
	return nil
}

// Dprdpr0intflopfifo1MetricsIterator is the iterator object
type Dprdpr0intflopfifo1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dprdpr0intflopfifo1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dprdpr0intflopfifo1MetricsIterator) Next() *Dprdpr0intflopfifo1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dprdpr0intflopfifo1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dprdpr0intflopfifo1MetricsIterator) Find(key uint64) (*Dprdpr0intflopfifo1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dprdpr0intflopfifo1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dprdpr0intflopfifo1MetricsIterator) Create(key uint64) (*Dprdpr0intflopfifo1Metrics, error) {
	tmtr := &Dprdpr0intflopfifo1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dprdpr0intflopfifo1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dprdpr0intflopfifo1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dprdpr0intflopfifo1MetricsIterator) Free() {
	it.iter.Free()
}

// NewDprdpr0intflopfifo1MetricsIterator returns an iterator
func NewDprdpr0intflopfifo1MetricsIterator() (*Dprdpr0intflopfifo1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dprdpr0intflopfifo1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dprdpr0intflopfifo1MetricsIterator{iter: iter}, nil
}

type Dprdpr0intspareMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Spare_0 metrics.Counter

	Spare_1 metrics.Counter

	Spare_2 metrics.Counter

	Spare_3 metrics.Counter

	Spare_4 metrics.Counter

	Spare_5 metrics.Counter

	Spare_6 metrics.Counter

	Spare_7 metrics.Counter

	Spare_8 metrics.Counter

	Spare_9 metrics.Counter

	Spare_10 metrics.Counter

	Spare_11 metrics.Counter

	Spare_12 metrics.Counter

	Spare_13 metrics.Counter

	Spare_14 metrics.Counter

	Spare_15 metrics.Counter

	Spare_16 metrics.Counter

	Spare_17 metrics.Counter

	Spare_18 metrics.Counter

	Spare_19 metrics.Counter

	Spare_20 metrics.Counter

	Spare_21 metrics.Counter

	Spare_22 metrics.Counter

	Spare_23 metrics.Counter

	Spare_24 metrics.Counter

	Spare_25 metrics.Counter

	Spare_26 metrics.Counter

	Spare_27 metrics.Counter

	Spare_28 metrics.Counter

	Spare_29 metrics.Counter

	Spare_30 metrics.Counter

	Spare_31 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dprdpr0intspareMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dprdpr0intspareMetrics) Size() int {
	sz := 0

	sz += mtr.Spare_0.Size()

	sz += mtr.Spare_1.Size()

	sz += mtr.Spare_2.Size()

	sz += mtr.Spare_3.Size()

	sz += mtr.Spare_4.Size()

	sz += mtr.Spare_5.Size()

	sz += mtr.Spare_6.Size()

	sz += mtr.Spare_7.Size()

	sz += mtr.Spare_8.Size()

	sz += mtr.Spare_9.Size()

	sz += mtr.Spare_10.Size()

	sz += mtr.Spare_11.Size()

	sz += mtr.Spare_12.Size()

	sz += mtr.Spare_13.Size()

	sz += mtr.Spare_14.Size()

	sz += mtr.Spare_15.Size()

	sz += mtr.Spare_16.Size()

	sz += mtr.Spare_17.Size()

	sz += mtr.Spare_18.Size()

	sz += mtr.Spare_19.Size()

	sz += mtr.Spare_20.Size()

	sz += mtr.Spare_21.Size()

	sz += mtr.Spare_22.Size()

	sz += mtr.Spare_23.Size()

	sz += mtr.Spare_24.Size()

	sz += mtr.Spare_25.Size()

	sz += mtr.Spare_26.Size()

	sz += mtr.Spare_27.Size()

	sz += mtr.Spare_28.Size()

	sz += mtr.Spare_29.Size()

	sz += mtr.Spare_30.Size()

	sz += mtr.Spare_31.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dprdpr0intspareMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Spare_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_0.Size()

	mtr.Spare_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_1.Size()

	mtr.Spare_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_2.Size()

	mtr.Spare_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_3.Size()

	mtr.Spare_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_4.Size()

	mtr.Spare_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_5.Size()

	mtr.Spare_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_6.Size()

	mtr.Spare_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_7.Size()

	mtr.Spare_8 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_8.Size()

	mtr.Spare_9 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_9.Size()

	mtr.Spare_10 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_10.Size()

	mtr.Spare_11 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_11.Size()

	mtr.Spare_12 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_12.Size()

	mtr.Spare_13 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_13.Size()

	mtr.Spare_14 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_14.Size()

	mtr.Spare_15 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_15.Size()

	mtr.Spare_16 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_16.Size()

	mtr.Spare_17 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_17.Size()

	mtr.Spare_18 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_18.Size()

	mtr.Spare_19 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_19.Size()

	mtr.Spare_20 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_20.Size()

	mtr.Spare_21 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_21.Size()

	mtr.Spare_22 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_22.Size()

	mtr.Spare_23 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_23.Size()

	mtr.Spare_24 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_24.Size()

	mtr.Spare_25 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_25.Size()

	mtr.Spare_26 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_26.Size()

	mtr.Spare_27 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_27.Size()

	mtr.Spare_28 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_28.Size()

	mtr.Spare_29 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_29.Size()

	mtr.Spare_30 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_30.Size()

	mtr.Spare_31 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_31.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dprdpr0intspareMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Spare_0" {
		return offset
	}
	offset += mtr.Spare_0.Size()

	if fldName == "Spare_1" {
		return offset
	}
	offset += mtr.Spare_1.Size()

	if fldName == "Spare_2" {
		return offset
	}
	offset += mtr.Spare_2.Size()

	if fldName == "Spare_3" {
		return offset
	}
	offset += mtr.Spare_3.Size()

	if fldName == "Spare_4" {
		return offset
	}
	offset += mtr.Spare_4.Size()

	if fldName == "Spare_5" {
		return offset
	}
	offset += mtr.Spare_5.Size()

	if fldName == "Spare_6" {
		return offset
	}
	offset += mtr.Spare_6.Size()

	if fldName == "Spare_7" {
		return offset
	}
	offset += mtr.Spare_7.Size()

	if fldName == "Spare_8" {
		return offset
	}
	offset += mtr.Spare_8.Size()

	if fldName == "Spare_9" {
		return offset
	}
	offset += mtr.Spare_9.Size()

	if fldName == "Spare_10" {
		return offset
	}
	offset += mtr.Spare_10.Size()

	if fldName == "Spare_11" {
		return offset
	}
	offset += mtr.Spare_11.Size()

	if fldName == "Spare_12" {
		return offset
	}
	offset += mtr.Spare_12.Size()

	if fldName == "Spare_13" {
		return offset
	}
	offset += mtr.Spare_13.Size()

	if fldName == "Spare_14" {
		return offset
	}
	offset += mtr.Spare_14.Size()

	if fldName == "Spare_15" {
		return offset
	}
	offset += mtr.Spare_15.Size()

	if fldName == "Spare_16" {
		return offset
	}
	offset += mtr.Spare_16.Size()

	if fldName == "Spare_17" {
		return offset
	}
	offset += mtr.Spare_17.Size()

	if fldName == "Spare_18" {
		return offset
	}
	offset += mtr.Spare_18.Size()

	if fldName == "Spare_19" {
		return offset
	}
	offset += mtr.Spare_19.Size()

	if fldName == "Spare_20" {
		return offset
	}
	offset += mtr.Spare_20.Size()

	if fldName == "Spare_21" {
		return offset
	}
	offset += mtr.Spare_21.Size()

	if fldName == "Spare_22" {
		return offset
	}
	offset += mtr.Spare_22.Size()

	if fldName == "Spare_23" {
		return offset
	}
	offset += mtr.Spare_23.Size()

	if fldName == "Spare_24" {
		return offset
	}
	offset += mtr.Spare_24.Size()

	if fldName == "Spare_25" {
		return offset
	}
	offset += mtr.Spare_25.Size()

	if fldName == "Spare_26" {
		return offset
	}
	offset += mtr.Spare_26.Size()

	if fldName == "Spare_27" {
		return offset
	}
	offset += mtr.Spare_27.Size()

	if fldName == "Spare_28" {
		return offset
	}
	offset += mtr.Spare_28.Size()

	if fldName == "Spare_29" {
		return offset
	}
	offset += mtr.Spare_29.Size()

	if fldName == "Spare_30" {
		return offset
	}
	offset += mtr.Spare_30.Size()

	if fldName == "Spare_31" {
		return offset
	}
	offset += mtr.Spare_31.Size()

	return offset
}

// SetSpare_0 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_0"))
	return nil
}

// SetSpare_1 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_1"))
	return nil
}

// SetSpare_2 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_2"))
	return nil
}

// SetSpare_3 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_3"))
	return nil
}

// SetSpare_4 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_4"))
	return nil
}

// SetSpare_5 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_5"))
	return nil
}

// SetSpare_6 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_6"))
	return nil
}

// SetSpare_7 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_7"))
	return nil
}

// SetSpare_8 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_8"))
	return nil
}

// SetSpare_9 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_9(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_9"))
	return nil
}

// SetSpare_10 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_10(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_10"))
	return nil
}

// SetSpare_11 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_11(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_11"))
	return nil
}

// SetSpare_12 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_12(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_12"))
	return nil
}

// SetSpare_13 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_13(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_13"))
	return nil
}

// SetSpare_14 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_14(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_14"))
	return nil
}

// SetSpare_15 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_15(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_15"))
	return nil
}

// SetSpare_16 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_16(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_16"))
	return nil
}

// SetSpare_17 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_17(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_17"))
	return nil
}

// SetSpare_18 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_18(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_18"))
	return nil
}

// SetSpare_19 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_19(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_19"))
	return nil
}

// SetSpare_20 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_20(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_20"))
	return nil
}

// SetSpare_21 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_21(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_21"))
	return nil
}

// SetSpare_22 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_22(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_22"))
	return nil
}

// SetSpare_23 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_23(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_23"))
	return nil
}

// SetSpare_24 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_24(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_24"))
	return nil
}

// SetSpare_25 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_25(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_25"))
	return nil
}

// SetSpare_26 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_26(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_26"))
	return nil
}

// SetSpare_27 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_27(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_27"))
	return nil
}

// SetSpare_28 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_28(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_28"))
	return nil
}

// SetSpare_29 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_29(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_29"))
	return nil
}

// SetSpare_30 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_30(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_30"))
	return nil
}

// SetSpare_31 sets cunter in shared memory
func (mtr *Dprdpr0intspareMetrics) SetSpare_31(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_31"))
	return nil
}

// Dprdpr0intspareMetricsIterator is the iterator object
type Dprdpr0intspareMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dprdpr0intspareMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dprdpr0intspareMetricsIterator) Next() *Dprdpr0intspareMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dprdpr0intspareMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dprdpr0intspareMetricsIterator) Find(key uint64) (*Dprdpr0intspareMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dprdpr0intspareMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dprdpr0intspareMetricsIterator) Create(key uint64) (*Dprdpr0intspareMetrics, error) {
	tmtr := &Dprdpr0intspareMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dprdpr0intspareMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dprdpr0intspareMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dprdpr0intspareMetricsIterator) Free() {
	it.iter.Free()
}

// NewDprdpr0intspareMetricsIterator returns an iterator
func NewDprdpr0intspareMetricsIterator() (*Dprdpr0intspareMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dprdpr0intspareMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dprdpr0intspareMetricsIterator{iter: iter}, nil
}

type Dprdpr0intcreditMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	EgressCreditOvflow metrics.Counter

	EgressCreditUndflow metrics.Counter

	PktoutCreditOvflow metrics.Counter

	PktoutCreditUndflow metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dprdpr0intcreditMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dprdpr0intcreditMetrics) Size() int {
	sz := 0

	sz += mtr.EgressCreditOvflow.Size()

	sz += mtr.EgressCreditUndflow.Size()

	sz += mtr.PktoutCreditOvflow.Size()

	sz += mtr.PktoutCreditUndflow.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dprdpr0intcreditMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.EgressCreditOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.EgressCreditOvflow.Size()

	mtr.EgressCreditUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.EgressCreditUndflow.Size()

	mtr.PktoutCreditOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktoutCreditOvflow.Size()

	mtr.PktoutCreditUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktoutCreditUndflow.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dprdpr0intcreditMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "EgressCreditOvflow" {
		return offset
	}
	offset += mtr.EgressCreditOvflow.Size()

	if fldName == "EgressCreditUndflow" {
		return offset
	}
	offset += mtr.EgressCreditUndflow.Size()

	if fldName == "PktoutCreditOvflow" {
		return offset
	}
	offset += mtr.PktoutCreditOvflow.Size()

	if fldName == "PktoutCreditUndflow" {
		return offset
	}
	offset += mtr.PktoutCreditUndflow.Size()

	return offset
}

// SetEgressCreditOvflow sets cunter in shared memory
func (mtr *Dprdpr0intcreditMetrics) SetEgressCreditOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EgressCreditOvflow"))
	return nil
}

// SetEgressCreditUndflow sets cunter in shared memory
func (mtr *Dprdpr0intcreditMetrics) SetEgressCreditUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EgressCreditUndflow"))
	return nil
}

// SetPktoutCreditOvflow sets cunter in shared memory
func (mtr *Dprdpr0intcreditMetrics) SetPktoutCreditOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktoutCreditOvflow"))
	return nil
}

// SetPktoutCreditUndflow sets cunter in shared memory
func (mtr *Dprdpr0intcreditMetrics) SetPktoutCreditUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktoutCreditUndflow"))
	return nil
}

// Dprdpr0intcreditMetricsIterator is the iterator object
type Dprdpr0intcreditMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dprdpr0intcreditMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dprdpr0intcreditMetricsIterator) Next() *Dprdpr0intcreditMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dprdpr0intcreditMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dprdpr0intcreditMetricsIterator) Find(key uint64) (*Dprdpr0intcreditMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dprdpr0intcreditMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dprdpr0intcreditMetricsIterator) Create(key uint64) (*Dprdpr0intcreditMetrics, error) {
	tmtr := &Dprdpr0intcreditMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dprdpr0intcreditMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dprdpr0intcreditMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dprdpr0intcreditMetricsIterator) Free() {
	it.iter.Free()
}

// NewDprdpr0intcreditMetricsIterator returns an iterator
func NewDprdpr0intcreditMetricsIterator() (*Dprdpr0intcreditMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dprdpr0intcreditMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dprdpr0intcreditMetricsIterator{iter: iter}, nil
}

type Dprdpr1intreg1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	ErrPhvSopNoEop metrics.Counter

	ErrPhvEopNoSop metrics.Counter

	ErrOhiSopNoEop metrics.Counter

	ErrOhiEopNoSop metrics.Counter

	ErrPktinSopNoEop metrics.Counter

	ErrPktinEopNoSop metrics.Counter

	ErrCsumOffsetGtPktSize_4 metrics.Counter

	ErrCsumOffsetGtPktSize_3 metrics.Counter

	ErrCsumOffsetGtPktSize_2 metrics.Counter

	ErrCsumOffsetGtPktSize_1 metrics.Counter

	ErrCsumOffsetGtPktSize_0 metrics.Counter

	ErrCsumPhdrOffsetGtPktSize_4 metrics.Counter

	ErrCsumPhdrOffsetGtPktSize_3 metrics.Counter

	ErrCsumPhdrOffsetGtPktSize_2 metrics.Counter

	ErrCsumPhdrOffsetGtPktSize_1 metrics.Counter

	ErrCsumPhdrOffsetGtPktSize_0 metrics.Counter

	ErrCsumLocGtPktSize_4 metrics.Counter

	ErrCsumLocGtPktSize_3 metrics.Counter

	ErrCsumLocGtPktSize_2 metrics.Counter

	ErrCsumLocGtPktSize_1 metrics.Counter

	ErrCsumLocGtPktSize_0 metrics.Counter

	ErrCrcOffsetGtPktSize metrics.Counter

	ErrCrcLocGtPktSize metrics.Counter

	ErrPtrFfOverflow metrics.Counter

	ErrCsumFfOverflow metrics.Counter

	ErrPktoutFfOverflow metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dprdpr1intreg1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dprdpr1intreg1Metrics) Size() int {
	sz := 0

	sz += mtr.ErrPhvSopNoEop.Size()

	sz += mtr.ErrPhvEopNoSop.Size()

	sz += mtr.ErrOhiSopNoEop.Size()

	sz += mtr.ErrOhiEopNoSop.Size()

	sz += mtr.ErrPktinSopNoEop.Size()

	sz += mtr.ErrPktinEopNoSop.Size()

	sz += mtr.ErrCsumOffsetGtPktSize_4.Size()

	sz += mtr.ErrCsumOffsetGtPktSize_3.Size()

	sz += mtr.ErrCsumOffsetGtPktSize_2.Size()

	sz += mtr.ErrCsumOffsetGtPktSize_1.Size()

	sz += mtr.ErrCsumOffsetGtPktSize_0.Size()

	sz += mtr.ErrCsumPhdrOffsetGtPktSize_4.Size()

	sz += mtr.ErrCsumPhdrOffsetGtPktSize_3.Size()

	sz += mtr.ErrCsumPhdrOffsetGtPktSize_2.Size()

	sz += mtr.ErrCsumPhdrOffsetGtPktSize_1.Size()

	sz += mtr.ErrCsumPhdrOffsetGtPktSize_0.Size()

	sz += mtr.ErrCsumLocGtPktSize_4.Size()

	sz += mtr.ErrCsumLocGtPktSize_3.Size()

	sz += mtr.ErrCsumLocGtPktSize_2.Size()

	sz += mtr.ErrCsumLocGtPktSize_1.Size()

	sz += mtr.ErrCsumLocGtPktSize_0.Size()

	sz += mtr.ErrCrcOffsetGtPktSize.Size()

	sz += mtr.ErrCrcLocGtPktSize.Size()

	sz += mtr.ErrPtrFfOverflow.Size()

	sz += mtr.ErrCsumFfOverflow.Size()

	sz += mtr.ErrPktoutFfOverflow.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dprdpr1intreg1Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.ErrPhvSopNoEop = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPhvSopNoEop.Size()

	mtr.ErrPhvEopNoSop = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPhvEopNoSop.Size()

	mtr.ErrOhiSopNoEop = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrOhiSopNoEop.Size()

	mtr.ErrOhiEopNoSop = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrOhiEopNoSop.Size()

	mtr.ErrPktinSopNoEop = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPktinSopNoEop.Size()

	mtr.ErrPktinEopNoSop = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPktinEopNoSop.Size()

	mtr.ErrCsumOffsetGtPktSize_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumOffsetGtPktSize_4.Size()

	mtr.ErrCsumOffsetGtPktSize_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumOffsetGtPktSize_3.Size()

	mtr.ErrCsumOffsetGtPktSize_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumOffsetGtPktSize_2.Size()

	mtr.ErrCsumOffsetGtPktSize_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumOffsetGtPktSize_1.Size()

	mtr.ErrCsumOffsetGtPktSize_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumOffsetGtPktSize_0.Size()

	mtr.ErrCsumPhdrOffsetGtPktSize_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumPhdrOffsetGtPktSize_4.Size()

	mtr.ErrCsumPhdrOffsetGtPktSize_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumPhdrOffsetGtPktSize_3.Size()

	mtr.ErrCsumPhdrOffsetGtPktSize_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumPhdrOffsetGtPktSize_2.Size()

	mtr.ErrCsumPhdrOffsetGtPktSize_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumPhdrOffsetGtPktSize_1.Size()

	mtr.ErrCsumPhdrOffsetGtPktSize_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumPhdrOffsetGtPktSize_0.Size()

	mtr.ErrCsumLocGtPktSize_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumLocGtPktSize_4.Size()

	mtr.ErrCsumLocGtPktSize_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumLocGtPktSize_3.Size()

	mtr.ErrCsumLocGtPktSize_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumLocGtPktSize_2.Size()

	mtr.ErrCsumLocGtPktSize_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumLocGtPktSize_1.Size()

	mtr.ErrCsumLocGtPktSize_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumLocGtPktSize_0.Size()

	mtr.ErrCrcOffsetGtPktSize = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCrcOffsetGtPktSize.Size()

	mtr.ErrCrcLocGtPktSize = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCrcLocGtPktSize.Size()

	mtr.ErrPtrFfOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPtrFfOverflow.Size()

	mtr.ErrCsumFfOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumFfOverflow.Size()

	mtr.ErrPktoutFfOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPktoutFfOverflow.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dprdpr1intreg1Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "ErrPhvSopNoEop" {
		return offset
	}
	offset += mtr.ErrPhvSopNoEop.Size()

	if fldName == "ErrPhvEopNoSop" {
		return offset
	}
	offset += mtr.ErrPhvEopNoSop.Size()

	if fldName == "ErrOhiSopNoEop" {
		return offset
	}
	offset += mtr.ErrOhiSopNoEop.Size()

	if fldName == "ErrOhiEopNoSop" {
		return offset
	}
	offset += mtr.ErrOhiEopNoSop.Size()

	if fldName == "ErrPktinSopNoEop" {
		return offset
	}
	offset += mtr.ErrPktinSopNoEop.Size()

	if fldName == "ErrPktinEopNoSop" {
		return offset
	}
	offset += mtr.ErrPktinEopNoSop.Size()

	if fldName == "ErrCsumOffsetGtPktSize_4" {
		return offset
	}
	offset += mtr.ErrCsumOffsetGtPktSize_4.Size()

	if fldName == "ErrCsumOffsetGtPktSize_3" {
		return offset
	}
	offset += mtr.ErrCsumOffsetGtPktSize_3.Size()

	if fldName == "ErrCsumOffsetGtPktSize_2" {
		return offset
	}
	offset += mtr.ErrCsumOffsetGtPktSize_2.Size()

	if fldName == "ErrCsumOffsetGtPktSize_1" {
		return offset
	}
	offset += mtr.ErrCsumOffsetGtPktSize_1.Size()

	if fldName == "ErrCsumOffsetGtPktSize_0" {
		return offset
	}
	offset += mtr.ErrCsumOffsetGtPktSize_0.Size()

	if fldName == "ErrCsumPhdrOffsetGtPktSize_4" {
		return offset
	}
	offset += mtr.ErrCsumPhdrOffsetGtPktSize_4.Size()

	if fldName == "ErrCsumPhdrOffsetGtPktSize_3" {
		return offset
	}
	offset += mtr.ErrCsumPhdrOffsetGtPktSize_3.Size()

	if fldName == "ErrCsumPhdrOffsetGtPktSize_2" {
		return offset
	}
	offset += mtr.ErrCsumPhdrOffsetGtPktSize_2.Size()

	if fldName == "ErrCsumPhdrOffsetGtPktSize_1" {
		return offset
	}
	offset += mtr.ErrCsumPhdrOffsetGtPktSize_1.Size()

	if fldName == "ErrCsumPhdrOffsetGtPktSize_0" {
		return offset
	}
	offset += mtr.ErrCsumPhdrOffsetGtPktSize_0.Size()

	if fldName == "ErrCsumLocGtPktSize_4" {
		return offset
	}
	offset += mtr.ErrCsumLocGtPktSize_4.Size()

	if fldName == "ErrCsumLocGtPktSize_3" {
		return offset
	}
	offset += mtr.ErrCsumLocGtPktSize_3.Size()

	if fldName == "ErrCsumLocGtPktSize_2" {
		return offset
	}
	offset += mtr.ErrCsumLocGtPktSize_2.Size()

	if fldName == "ErrCsumLocGtPktSize_1" {
		return offset
	}
	offset += mtr.ErrCsumLocGtPktSize_1.Size()

	if fldName == "ErrCsumLocGtPktSize_0" {
		return offset
	}
	offset += mtr.ErrCsumLocGtPktSize_0.Size()

	if fldName == "ErrCrcOffsetGtPktSize" {
		return offset
	}
	offset += mtr.ErrCrcOffsetGtPktSize.Size()

	if fldName == "ErrCrcLocGtPktSize" {
		return offset
	}
	offset += mtr.ErrCrcLocGtPktSize.Size()

	if fldName == "ErrPtrFfOverflow" {
		return offset
	}
	offset += mtr.ErrPtrFfOverflow.Size()

	if fldName == "ErrCsumFfOverflow" {
		return offset
	}
	offset += mtr.ErrCsumFfOverflow.Size()

	if fldName == "ErrPktoutFfOverflow" {
		return offset
	}
	offset += mtr.ErrPktoutFfOverflow.Size()

	return offset
}

// SetErrPhvSopNoEop sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrPhvSopNoEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPhvSopNoEop"))
	return nil
}

// SetErrPhvEopNoSop sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrPhvEopNoSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPhvEopNoSop"))
	return nil
}

// SetErrOhiSopNoEop sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrOhiSopNoEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrOhiSopNoEop"))
	return nil
}

// SetErrOhiEopNoSop sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrOhiEopNoSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrOhiEopNoSop"))
	return nil
}

// SetErrPktinSopNoEop sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrPktinSopNoEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPktinSopNoEop"))
	return nil
}

// SetErrPktinEopNoSop sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrPktinEopNoSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPktinEopNoSop"))
	return nil
}

// SetErrCsumOffsetGtPktSize_4 sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrCsumOffsetGtPktSize_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumOffsetGtPktSize_4"))
	return nil
}

// SetErrCsumOffsetGtPktSize_3 sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrCsumOffsetGtPktSize_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumOffsetGtPktSize_3"))
	return nil
}

// SetErrCsumOffsetGtPktSize_2 sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrCsumOffsetGtPktSize_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumOffsetGtPktSize_2"))
	return nil
}

// SetErrCsumOffsetGtPktSize_1 sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrCsumOffsetGtPktSize_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumOffsetGtPktSize_1"))
	return nil
}

// SetErrCsumOffsetGtPktSize_0 sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrCsumOffsetGtPktSize_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumOffsetGtPktSize_0"))
	return nil
}

// SetErrCsumPhdrOffsetGtPktSize_4 sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrCsumPhdrOffsetGtPktSize_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumPhdrOffsetGtPktSize_4"))
	return nil
}

// SetErrCsumPhdrOffsetGtPktSize_3 sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrCsumPhdrOffsetGtPktSize_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumPhdrOffsetGtPktSize_3"))
	return nil
}

// SetErrCsumPhdrOffsetGtPktSize_2 sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrCsumPhdrOffsetGtPktSize_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumPhdrOffsetGtPktSize_2"))
	return nil
}

// SetErrCsumPhdrOffsetGtPktSize_1 sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrCsumPhdrOffsetGtPktSize_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumPhdrOffsetGtPktSize_1"))
	return nil
}

// SetErrCsumPhdrOffsetGtPktSize_0 sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrCsumPhdrOffsetGtPktSize_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumPhdrOffsetGtPktSize_0"))
	return nil
}

// SetErrCsumLocGtPktSize_4 sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrCsumLocGtPktSize_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumLocGtPktSize_4"))
	return nil
}

// SetErrCsumLocGtPktSize_3 sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrCsumLocGtPktSize_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumLocGtPktSize_3"))
	return nil
}

// SetErrCsumLocGtPktSize_2 sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrCsumLocGtPktSize_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumLocGtPktSize_2"))
	return nil
}

// SetErrCsumLocGtPktSize_1 sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrCsumLocGtPktSize_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumLocGtPktSize_1"))
	return nil
}

// SetErrCsumLocGtPktSize_0 sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrCsumLocGtPktSize_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumLocGtPktSize_0"))
	return nil
}

// SetErrCrcOffsetGtPktSize sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrCrcOffsetGtPktSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCrcOffsetGtPktSize"))
	return nil
}

// SetErrCrcLocGtPktSize sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrCrcLocGtPktSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCrcLocGtPktSize"))
	return nil
}

// SetErrPtrFfOverflow sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrPtrFfOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPtrFfOverflow"))
	return nil
}

// SetErrCsumFfOverflow sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrCsumFfOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumFfOverflow"))
	return nil
}

// SetErrPktoutFfOverflow sets cunter in shared memory
func (mtr *Dprdpr1intreg1Metrics) SetErrPktoutFfOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPktoutFfOverflow"))
	return nil
}

// Dprdpr1intreg1MetricsIterator is the iterator object
type Dprdpr1intreg1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dprdpr1intreg1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dprdpr1intreg1MetricsIterator) Next() *Dprdpr1intreg1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dprdpr1intreg1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dprdpr1intreg1MetricsIterator) Find(key uint64) (*Dprdpr1intreg1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dprdpr1intreg1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dprdpr1intreg1MetricsIterator) Create(key uint64) (*Dprdpr1intreg1Metrics, error) {
	tmtr := &Dprdpr1intreg1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dprdpr1intreg1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dprdpr1intreg1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dprdpr1intreg1MetricsIterator) Free() {
	it.iter.Free()
}

// NewDprdpr1intreg1MetricsIterator returns an iterator
func NewDprdpr1intreg1MetricsIterator() (*Dprdpr1intreg1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dprdpr1intreg1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dprdpr1intreg1MetricsIterator{iter: iter}, nil
}

type Dprdpr1intreg2Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	ErrCsumStartGtEnd_4 metrics.Counter

	ErrCsumStartGtEnd_3 metrics.Counter

	ErrCsumStartGtEnd_2 metrics.Counter

	ErrCsumStartGtEnd_1 metrics.Counter

	ErrCsumStartGtEnd_0 metrics.Counter

	ErrCrcStartGtEnd metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dprdpr1intreg2Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dprdpr1intreg2Metrics) Size() int {
	sz := 0

	sz += mtr.ErrCsumStartGtEnd_4.Size()

	sz += mtr.ErrCsumStartGtEnd_3.Size()

	sz += mtr.ErrCsumStartGtEnd_2.Size()

	sz += mtr.ErrCsumStartGtEnd_1.Size()

	sz += mtr.ErrCsumStartGtEnd_0.Size()

	sz += mtr.ErrCrcStartGtEnd.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dprdpr1intreg2Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.ErrCsumStartGtEnd_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumStartGtEnd_4.Size()

	mtr.ErrCsumStartGtEnd_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumStartGtEnd_3.Size()

	mtr.ErrCsumStartGtEnd_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumStartGtEnd_2.Size()

	mtr.ErrCsumStartGtEnd_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumStartGtEnd_1.Size()

	mtr.ErrCsumStartGtEnd_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumStartGtEnd_0.Size()

	mtr.ErrCrcStartGtEnd = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCrcStartGtEnd.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dprdpr1intreg2Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "ErrCsumStartGtEnd_4" {
		return offset
	}
	offset += mtr.ErrCsumStartGtEnd_4.Size()

	if fldName == "ErrCsumStartGtEnd_3" {
		return offset
	}
	offset += mtr.ErrCsumStartGtEnd_3.Size()

	if fldName == "ErrCsumStartGtEnd_2" {
		return offset
	}
	offset += mtr.ErrCsumStartGtEnd_2.Size()

	if fldName == "ErrCsumStartGtEnd_1" {
		return offset
	}
	offset += mtr.ErrCsumStartGtEnd_1.Size()

	if fldName == "ErrCsumStartGtEnd_0" {
		return offset
	}
	offset += mtr.ErrCsumStartGtEnd_0.Size()

	if fldName == "ErrCrcStartGtEnd" {
		return offset
	}
	offset += mtr.ErrCrcStartGtEnd.Size()

	return offset
}

// SetErrCsumStartGtEnd_4 sets cunter in shared memory
func (mtr *Dprdpr1intreg2Metrics) SetErrCsumStartGtEnd_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumStartGtEnd_4"))
	return nil
}

// SetErrCsumStartGtEnd_3 sets cunter in shared memory
func (mtr *Dprdpr1intreg2Metrics) SetErrCsumStartGtEnd_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumStartGtEnd_3"))
	return nil
}

// SetErrCsumStartGtEnd_2 sets cunter in shared memory
func (mtr *Dprdpr1intreg2Metrics) SetErrCsumStartGtEnd_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumStartGtEnd_2"))
	return nil
}

// SetErrCsumStartGtEnd_1 sets cunter in shared memory
func (mtr *Dprdpr1intreg2Metrics) SetErrCsumStartGtEnd_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumStartGtEnd_1"))
	return nil
}

// SetErrCsumStartGtEnd_0 sets cunter in shared memory
func (mtr *Dprdpr1intreg2Metrics) SetErrCsumStartGtEnd_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumStartGtEnd_0"))
	return nil
}

// SetErrCrcStartGtEnd sets cunter in shared memory
func (mtr *Dprdpr1intreg2Metrics) SetErrCrcStartGtEnd(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCrcStartGtEnd"))
	return nil
}

// Dprdpr1intreg2MetricsIterator is the iterator object
type Dprdpr1intreg2MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dprdpr1intreg2MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dprdpr1intreg2MetricsIterator) Next() *Dprdpr1intreg2Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dprdpr1intreg2Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dprdpr1intreg2MetricsIterator) Find(key uint64) (*Dprdpr1intreg2Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dprdpr1intreg2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dprdpr1intreg2MetricsIterator) Create(key uint64) (*Dprdpr1intreg2Metrics, error) {
	tmtr := &Dprdpr1intreg2Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dprdpr1intreg2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dprdpr1intreg2MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dprdpr1intreg2MetricsIterator) Free() {
	it.iter.Free()
}

// NewDprdpr1intreg2MetricsIterator returns an iterator
func NewDprdpr1intreg2MetricsIterator() (*Dprdpr1intreg2MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dprdpr1intreg2Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dprdpr1intreg2MetricsIterator{iter: iter}, nil
}

type Dprdpr1intsramseccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	DprPktinFifoUncorrectable metrics.Counter

	DprPktinFifoCorrectable metrics.Counter

	DprCsumFifoUncorrectable metrics.Counter

	DprCsumFifoCorrectable metrics.Counter

	DprPhvFifoUncorrectable metrics.Counter

	DprPhvFifoCorrectable metrics.Counter

	DprOhiFifoUncorrectable metrics.Counter

	DprOhiFifoCorrectable metrics.Counter

	DprPtrFifoUncorrectable metrics.Counter

	DprPtrFifoCorrectable metrics.Counter

	DprPktoutFifoUncorrectable metrics.Counter

	DprPktoutFifoCorrectable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dprdpr1intsramseccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dprdpr1intsramseccMetrics) Size() int {
	sz := 0

	sz += mtr.DprPktinFifoUncorrectable.Size()

	sz += mtr.DprPktinFifoCorrectable.Size()

	sz += mtr.DprCsumFifoUncorrectable.Size()

	sz += mtr.DprCsumFifoCorrectable.Size()

	sz += mtr.DprPhvFifoUncorrectable.Size()

	sz += mtr.DprPhvFifoCorrectable.Size()

	sz += mtr.DprOhiFifoUncorrectable.Size()

	sz += mtr.DprOhiFifoCorrectable.Size()

	sz += mtr.DprPtrFifoUncorrectable.Size()

	sz += mtr.DprPtrFifoCorrectable.Size()

	sz += mtr.DprPktoutFifoUncorrectable.Size()

	sz += mtr.DprPktoutFifoCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dprdpr1intsramseccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.DprPktinFifoUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.DprPktinFifoUncorrectable.Size()

	mtr.DprPktinFifoCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.DprPktinFifoCorrectable.Size()

	mtr.DprCsumFifoUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.DprCsumFifoUncorrectable.Size()

	mtr.DprCsumFifoCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.DprCsumFifoCorrectable.Size()

	mtr.DprPhvFifoUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.DprPhvFifoUncorrectable.Size()

	mtr.DprPhvFifoCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.DprPhvFifoCorrectable.Size()

	mtr.DprOhiFifoUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.DprOhiFifoUncorrectable.Size()

	mtr.DprOhiFifoCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.DprOhiFifoCorrectable.Size()

	mtr.DprPtrFifoUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.DprPtrFifoUncorrectable.Size()

	mtr.DprPtrFifoCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.DprPtrFifoCorrectable.Size()

	mtr.DprPktoutFifoUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.DprPktoutFifoUncorrectable.Size()

	mtr.DprPktoutFifoCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.DprPktoutFifoCorrectable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dprdpr1intsramseccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "DprPktinFifoUncorrectable" {
		return offset
	}
	offset += mtr.DprPktinFifoUncorrectable.Size()

	if fldName == "DprPktinFifoCorrectable" {
		return offset
	}
	offset += mtr.DprPktinFifoCorrectable.Size()

	if fldName == "DprCsumFifoUncorrectable" {
		return offset
	}
	offset += mtr.DprCsumFifoUncorrectable.Size()

	if fldName == "DprCsumFifoCorrectable" {
		return offset
	}
	offset += mtr.DprCsumFifoCorrectable.Size()

	if fldName == "DprPhvFifoUncorrectable" {
		return offset
	}
	offset += mtr.DprPhvFifoUncorrectable.Size()

	if fldName == "DprPhvFifoCorrectable" {
		return offset
	}
	offset += mtr.DprPhvFifoCorrectable.Size()

	if fldName == "DprOhiFifoUncorrectable" {
		return offset
	}
	offset += mtr.DprOhiFifoUncorrectable.Size()

	if fldName == "DprOhiFifoCorrectable" {
		return offset
	}
	offset += mtr.DprOhiFifoCorrectable.Size()

	if fldName == "DprPtrFifoUncorrectable" {
		return offset
	}
	offset += mtr.DprPtrFifoUncorrectable.Size()

	if fldName == "DprPtrFifoCorrectable" {
		return offset
	}
	offset += mtr.DprPtrFifoCorrectable.Size()

	if fldName == "DprPktoutFifoUncorrectable" {
		return offset
	}
	offset += mtr.DprPktoutFifoUncorrectable.Size()

	if fldName == "DprPktoutFifoCorrectable" {
		return offset
	}
	offset += mtr.DprPktoutFifoCorrectable.Size()

	return offset
}

// SetDprPktinFifoUncorrectable sets cunter in shared memory
func (mtr *Dprdpr1intsramseccMetrics) SetDprPktinFifoUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPktinFifoUncorrectable"))
	return nil
}

// SetDprPktinFifoCorrectable sets cunter in shared memory
func (mtr *Dprdpr1intsramseccMetrics) SetDprPktinFifoCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPktinFifoCorrectable"))
	return nil
}

// SetDprCsumFifoUncorrectable sets cunter in shared memory
func (mtr *Dprdpr1intsramseccMetrics) SetDprCsumFifoUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCsumFifoUncorrectable"))
	return nil
}

// SetDprCsumFifoCorrectable sets cunter in shared memory
func (mtr *Dprdpr1intsramseccMetrics) SetDprCsumFifoCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCsumFifoCorrectable"))
	return nil
}

// SetDprPhvFifoUncorrectable sets cunter in shared memory
func (mtr *Dprdpr1intsramseccMetrics) SetDprPhvFifoUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPhvFifoUncorrectable"))
	return nil
}

// SetDprPhvFifoCorrectable sets cunter in shared memory
func (mtr *Dprdpr1intsramseccMetrics) SetDprPhvFifoCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPhvFifoCorrectable"))
	return nil
}

// SetDprOhiFifoUncorrectable sets cunter in shared memory
func (mtr *Dprdpr1intsramseccMetrics) SetDprOhiFifoUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprOhiFifoUncorrectable"))
	return nil
}

// SetDprOhiFifoCorrectable sets cunter in shared memory
func (mtr *Dprdpr1intsramseccMetrics) SetDprOhiFifoCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprOhiFifoCorrectable"))
	return nil
}

// SetDprPtrFifoUncorrectable sets cunter in shared memory
func (mtr *Dprdpr1intsramseccMetrics) SetDprPtrFifoUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPtrFifoUncorrectable"))
	return nil
}

// SetDprPtrFifoCorrectable sets cunter in shared memory
func (mtr *Dprdpr1intsramseccMetrics) SetDprPtrFifoCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPtrFifoCorrectable"))
	return nil
}

// SetDprPktoutFifoUncorrectable sets cunter in shared memory
func (mtr *Dprdpr1intsramseccMetrics) SetDprPktoutFifoUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPktoutFifoUncorrectable"))
	return nil
}

// SetDprPktoutFifoCorrectable sets cunter in shared memory
func (mtr *Dprdpr1intsramseccMetrics) SetDprPktoutFifoCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPktoutFifoCorrectable"))
	return nil
}

// Dprdpr1intsramseccMetricsIterator is the iterator object
type Dprdpr1intsramseccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dprdpr1intsramseccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dprdpr1intsramseccMetricsIterator) Next() *Dprdpr1intsramseccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dprdpr1intsramseccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dprdpr1intsramseccMetricsIterator) Find(key uint64) (*Dprdpr1intsramseccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dprdpr1intsramseccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dprdpr1intsramseccMetricsIterator) Create(key uint64) (*Dprdpr1intsramseccMetrics, error) {
	tmtr := &Dprdpr1intsramseccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dprdpr1intsramseccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dprdpr1intsramseccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dprdpr1intsramseccMetricsIterator) Free() {
	it.iter.Free()
}

// NewDprdpr1intsramseccMetricsIterator returns an iterator
func NewDprdpr1intsramseccMetricsIterator() (*Dprdpr1intsramseccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dprdpr1intsramseccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dprdpr1intsramseccMetricsIterator{iter: iter}, nil
}

type Dprdpr1intfifoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PhvFfOvflow metrics.Counter

	OhiFfOvflow metrics.Counter

	PktinFfOvflow metrics.Counter

	PktoutFfUndflow metrics.Counter

	CsumFfOvflow metrics.Counter

	PtrFfOvflow metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dprdpr1intfifoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dprdpr1intfifoMetrics) Size() int {
	sz := 0

	sz += mtr.PhvFfOvflow.Size()

	sz += mtr.OhiFfOvflow.Size()

	sz += mtr.PktinFfOvflow.Size()

	sz += mtr.PktoutFfUndflow.Size()

	sz += mtr.CsumFfOvflow.Size()

	sz += mtr.PtrFfOvflow.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dprdpr1intfifoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.PhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvFfOvflow.Size()

	mtr.OhiFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.OhiFfOvflow.Size()

	mtr.PktinFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktinFfOvflow.Size()

	mtr.PktoutFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktoutFfUndflow.Size()

	mtr.CsumFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CsumFfOvflow.Size()

	mtr.PtrFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PtrFfOvflow.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dprdpr1intfifoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "PhvFfOvflow" {
		return offset
	}
	offset += mtr.PhvFfOvflow.Size()

	if fldName == "OhiFfOvflow" {
		return offset
	}
	offset += mtr.OhiFfOvflow.Size()

	if fldName == "PktinFfOvflow" {
		return offset
	}
	offset += mtr.PktinFfOvflow.Size()

	if fldName == "PktoutFfUndflow" {
		return offset
	}
	offset += mtr.PktoutFfUndflow.Size()

	if fldName == "CsumFfOvflow" {
		return offset
	}
	offset += mtr.CsumFfOvflow.Size()

	if fldName == "PtrFfOvflow" {
		return offset
	}
	offset += mtr.PtrFfOvflow.Size()

	return offset
}

// SetPhvFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intfifoMetrics) SetPhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvFfOvflow"))
	return nil
}

// SetOhiFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intfifoMetrics) SetOhiFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OhiFfOvflow"))
	return nil
}

// SetPktinFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intfifoMetrics) SetPktinFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktinFfOvflow"))
	return nil
}

// SetPktoutFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intfifoMetrics) SetPktoutFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktoutFfUndflow"))
	return nil
}

// SetCsumFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intfifoMetrics) SetCsumFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CsumFfOvflow"))
	return nil
}

// SetPtrFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intfifoMetrics) SetPtrFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PtrFfOvflow"))
	return nil
}

// Dprdpr1intfifoMetricsIterator is the iterator object
type Dprdpr1intfifoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dprdpr1intfifoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dprdpr1intfifoMetricsIterator) Next() *Dprdpr1intfifoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dprdpr1intfifoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dprdpr1intfifoMetricsIterator) Find(key uint64) (*Dprdpr1intfifoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dprdpr1intfifoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dprdpr1intfifoMetricsIterator) Create(key uint64) (*Dprdpr1intfifoMetrics, error) {
	tmtr := &Dprdpr1intfifoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dprdpr1intfifoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dprdpr1intfifoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dprdpr1intfifoMetricsIterator) Free() {
	it.iter.Free()
}

// NewDprdpr1intfifoMetricsIterator returns an iterator
func NewDprdpr1intfifoMetricsIterator() (*Dprdpr1intfifoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dprdpr1intfifoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dprdpr1intfifoMetricsIterator{iter: iter}, nil
}

type Dprdpr1intflopfifo0Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	DataMuxForceBypassCrcFlopFfOvflow metrics.Counter

	DprCrcInfoFlopFfOvflow metrics.Counter

	DprCrcUpdateInfoFlopFfOvflow metrics.Counter

	DprCsumInfoFlopFfOvflow metrics.Counter

	DataMuxForceBypassCsumFlopFfOvflow metrics.Counter

	DprCsumUpdateInfoFlopFfOvflow metrics.Counter

	PtrEarlyPktEopInfoFlopFfOvflow metrics.Counter

	DataMuxEopErrFlopFfOvflow metrics.Counter

	PktinEopErrFlopFfOvflow metrics.Counter

	CsumErrFlopFfOvflow metrics.Counter

	CrcErrFlopFfOvflow metrics.Counter

	DataMuxDropFlopFfOvflow metrics.Counter

	PhvPktDataFlopFfOvflow metrics.Counter

	PktoutLenCellFlopFfOvflow metrics.Counter

	PaddingSizeFlopFfOvflow metrics.Counter

	PktinErrFlopFfOvflow metrics.Counter

	PhvNoDataFlopFfOvflow metrics.Counter

	PtrLookahaedFlopFfOvflow metrics.Counter

	EopVldFlopFfOvflow metrics.Counter

	CsumCalVldFlopFfOvflow metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dprdpr1intflopfifo0Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dprdpr1intflopfifo0Metrics) Size() int {
	sz := 0

	sz += mtr.DataMuxForceBypassCrcFlopFfOvflow.Size()

	sz += mtr.DprCrcInfoFlopFfOvflow.Size()

	sz += mtr.DprCrcUpdateInfoFlopFfOvflow.Size()

	sz += mtr.DprCsumInfoFlopFfOvflow.Size()

	sz += mtr.DataMuxForceBypassCsumFlopFfOvflow.Size()

	sz += mtr.DprCsumUpdateInfoFlopFfOvflow.Size()

	sz += mtr.PtrEarlyPktEopInfoFlopFfOvflow.Size()

	sz += mtr.DataMuxEopErrFlopFfOvflow.Size()

	sz += mtr.PktinEopErrFlopFfOvflow.Size()

	sz += mtr.CsumErrFlopFfOvflow.Size()

	sz += mtr.CrcErrFlopFfOvflow.Size()

	sz += mtr.DataMuxDropFlopFfOvflow.Size()

	sz += mtr.PhvPktDataFlopFfOvflow.Size()

	sz += mtr.PktoutLenCellFlopFfOvflow.Size()

	sz += mtr.PaddingSizeFlopFfOvflow.Size()

	sz += mtr.PktinErrFlopFfOvflow.Size()

	sz += mtr.PhvNoDataFlopFfOvflow.Size()

	sz += mtr.PtrLookahaedFlopFfOvflow.Size()

	sz += mtr.EopVldFlopFfOvflow.Size()

	sz += mtr.CsumCalVldFlopFfOvflow.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.DataMuxForceBypassCrcFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DataMuxForceBypassCrcFlopFfOvflow.Size()

	mtr.DprCrcInfoFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DprCrcInfoFlopFfOvflow.Size()

	mtr.DprCrcUpdateInfoFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DprCrcUpdateInfoFlopFfOvflow.Size()

	mtr.DprCsumInfoFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DprCsumInfoFlopFfOvflow.Size()

	mtr.DataMuxForceBypassCsumFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DataMuxForceBypassCsumFlopFfOvflow.Size()

	mtr.DprCsumUpdateInfoFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DprCsumUpdateInfoFlopFfOvflow.Size()

	mtr.PtrEarlyPktEopInfoFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PtrEarlyPktEopInfoFlopFfOvflow.Size()

	mtr.DataMuxEopErrFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DataMuxEopErrFlopFfOvflow.Size()

	mtr.PktinEopErrFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktinEopErrFlopFfOvflow.Size()

	mtr.CsumErrFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CsumErrFlopFfOvflow.Size()

	mtr.CrcErrFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CrcErrFlopFfOvflow.Size()

	mtr.DataMuxDropFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DataMuxDropFlopFfOvflow.Size()

	mtr.PhvPktDataFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvPktDataFlopFfOvflow.Size()

	mtr.PktoutLenCellFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktoutLenCellFlopFfOvflow.Size()

	mtr.PaddingSizeFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PaddingSizeFlopFfOvflow.Size()

	mtr.PktinErrFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktinErrFlopFfOvflow.Size()

	mtr.PhvNoDataFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvNoDataFlopFfOvflow.Size()

	mtr.PtrLookahaedFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PtrLookahaedFlopFfOvflow.Size()

	mtr.EopVldFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.EopVldFlopFfOvflow.Size()

	mtr.CsumCalVldFlopFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CsumCalVldFlopFfOvflow.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "DataMuxForceBypassCrcFlopFfOvflow" {
		return offset
	}
	offset += mtr.DataMuxForceBypassCrcFlopFfOvflow.Size()

	if fldName == "DprCrcInfoFlopFfOvflow" {
		return offset
	}
	offset += mtr.DprCrcInfoFlopFfOvflow.Size()

	if fldName == "DprCrcUpdateInfoFlopFfOvflow" {
		return offset
	}
	offset += mtr.DprCrcUpdateInfoFlopFfOvflow.Size()

	if fldName == "DprCsumInfoFlopFfOvflow" {
		return offset
	}
	offset += mtr.DprCsumInfoFlopFfOvflow.Size()

	if fldName == "DataMuxForceBypassCsumFlopFfOvflow" {
		return offset
	}
	offset += mtr.DataMuxForceBypassCsumFlopFfOvflow.Size()

	if fldName == "DprCsumUpdateInfoFlopFfOvflow" {
		return offset
	}
	offset += mtr.DprCsumUpdateInfoFlopFfOvflow.Size()

	if fldName == "PtrEarlyPktEopInfoFlopFfOvflow" {
		return offset
	}
	offset += mtr.PtrEarlyPktEopInfoFlopFfOvflow.Size()

	if fldName == "DataMuxEopErrFlopFfOvflow" {
		return offset
	}
	offset += mtr.DataMuxEopErrFlopFfOvflow.Size()

	if fldName == "PktinEopErrFlopFfOvflow" {
		return offset
	}
	offset += mtr.PktinEopErrFlopFfOvflow.Size()

	if fldName == "CsumErrFlopFfOvflow" {
		return offset
	}
	offset += mtr.CsumErrFlopFfOvflow.Size()

	if fldName == "CrcErrFlopFfOvflow" {
		return offset
	}
	offset += mtr.CrcErrFlopFfOvflow.Size()

	if fldName == "DataMuxDropFlopFfOvflow" {
		return offset
	}
	offset += mtr.DataMuxDropFlopFfOvflow.Size()

	if fldName == "PhvPktDataFlopFfOvflow" {
		return offset
	}
	offset += mtr.PhvPktDataFlopFfOvflow.Size()

	if fldName == "PktoutLenCellFlopFfOvflow" {
		return offset
	}
	offset += mtr.PktoutLenCellFlopFfOvflow.Size()

	if fldName == "PaddingSizeFlopFfOvflow" {
		return offset
	}
	offset += mtr.PaddingSizeFlopFfOvflow.Size()

	if fldName == "PktinErrFlopFfOvflow" {
		return offset
	}
	offset += mtr.PktinErrFlopFfOvflow.Size()

	if fldName == "PhvNoDataFlopFfOvflow" {
		return offset
	}
	offset += mtr.PhvNoDataFlopFfOvflow.Size()

	if fldName == "PtrLookahaedFlopFfOvflow" {
		return offset
	}
	offset += mtr.PtrLookahaedFlopFfOvflow.Size()

	if fldName == "EopVldFlopFfOvflow" {
		return offset
	}
	offset += mtr.EopVldFlopFfOvflow.Size()

	if fldName == "CsumCalVldFlopFfOvflow" {
		return offset
	}
	offset += mtr.CsumCalVldFlopFfOvflow.Size()

	return offset
}

// SetDataMuxForceBypassCrcFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) SetDataMuxForceBypassCrcFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DataMuxForceBypassCrcFlopFfOvflow"))
	return nil
}

// SetDprCrcInfoFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) SetDprCrcInfoFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCrcInfoFlopFfOvflow"))
	return nil
}

// SetDprCrcUpdateInfoFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) SetDprCrcUpdateInfoFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCrcUpdateInfoFlopFfOvflow"))
	return nil
}

// SetDprCsumInfoFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) SetDprCsumInfoFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCsumInfoFlopFfOvflow"))
	return nil
}

// SetDataMuxForceBypassCsumFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) SetDataMuxForceBypassCsumFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DataMuxForceBypassCsumFlopFfOvflow"))
	return nil
}

// SetDprCsumUpdateInfoFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) SetDprCsumUpdateInfoFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCsumUpdateInfoFlopFfOvflow"))
	return nil
}

// SetPtrEarlyPktEopInfoFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) SetPtrEarlyPktEopInfoFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PtrEarlyPktEopInfoFlopFfOvflow"))
	return nil
}

// SetDataMuxEopErrFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) SetDataMuxEopErrFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DataMuxEopErrFlopFfOvflow"))
	return nil
}

// SetPktinEopErrFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) SetPktinEopErrFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktinEopErrFlopFfOvflow"))
	return nil
}

// SetCsumErrFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) SetCsumErrFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CsumErrFlopFfOvflow"))
	return nil
}

// SetCrcErrFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) SetCrcErrFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CrcErrFlopFfOvflow"))
	return nil
}

// SetDataMuxDropFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) SetDataMuxDropFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DataMuxDropFlopFfOvflow"))
	return nil
}

// SetPhvPktDataFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) SetPhvPktDataFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvPktDataFlopFfOvflow"))
	return nil
}

// SetPktoutLenCellFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) SetPktoutLenCellFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktoutLenCellFlopFfOvflow"))
	return nil
}

// SetPaddingSizeFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) SetPaddingSizeFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PaddingSizeFlopFfOvflow"))
	return nil
}

// SetPktinErrFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) SetPktinErrFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktinErrFlopFfOvflow"))
	return nil
}

// SetPhvNoDataFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) SetPhvNoDataFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvNoDataFlopFfOvflow"))
	return nil
}

// SetPtrLookahaedFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) SetPtrLookahaedFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PtrLookahaedFlopFfOvflow"))
	return nil
}

// SetEopVldFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) SetEopVldFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopVldFlopFfOvflow"))
	return nil
}

// SetCsumCalVldFlopFfOvflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo0Metrics) SetCsumCalVldFlopFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CsumCalVldFlopFfOvflow"))
	return nil
}

// Dprdpr1intflopfifo0MetricsIterator is the iterator object
type Dprdpr1intflopfifo0MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dprdpr1intflopfifo0MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dprdpr1intflopfifo0MetricsIterator) Next() *Dprdpr1intflopfifo0Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dprdpr1intflopfifo0Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dprdpr1intflopfifo0MetricsIterator) Find(key uint64) (*Dprdpr1intflopfifo0Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dprdpr1intflopfifo0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dprdpr1intflopfifo0MetricsIterator) Create(key uint64) (*Dprdpr1intflopfifo0Metrics, error) {
	tmtr := &Dprdpr1intflopfifo0Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dprdpr1intflopfifo0Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dprdpr1intflopfifo0MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dprdpr1intflopfifo0MetricsIterator) Free() {
	it.iter.Free()
}

// NewDprdpr1intflopfifo0MetricsIterator returns an iterator
func NewDprdpr1intflopfifo0MetricsIterator() (*Dprdpr1intflopfifo0MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dprdpr1intflopfifo0Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dprdpr1intflopfifo0MetricsIterator{iter: iter}, nil
}

type Dprdpr1intflopfifo1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	DataMuxForceBypassCrcFlopFfUndflow metrics.Counter

	DprCrcInfoFlopFfUndflow metrics.Counter

	DprCrcUpdateInfoFlopFfUndflow metrics.Counter

	DprCsumInfoFlopFfUndflow metrics.Counter

	DataMuxForceBypassCsumFlopFfUndflow metrics.Counter

	DprCsumUpdateInfoFlopFfUndflow metrics.Counter

	PtrEarlyPktEopInfoFlopFfUndflow metrics.Counter

	DataMuxEopErrFlopFfUndflow metrics.Counter

	PktinEopErrFlopFfUndflow metrics.Counter

	CsumErrFlopFfUndflow metrics.Counter

	CrcErrFlopFfUndflow metrics.Counter

	DataMuxDropFlopFfUndflow metrics.Counter

	PhvPktDataFlopFfUndflow metrics.Counter

	PktoutLenCellFlopFfUndflow metrics.Counter

	PaddingSizeFlopFfUndflow metrics.Counter

	PktinErrFlopFfUndflow metrics.Counter

	PhvNoDataFlopFfUndflow metrics.Counter

	PtrLookahaedFlopFfUndflow metrics.Counter

	EopVldFlopFfUndflow metrics.Counter

	CsumCalVldFlopFfUndflow metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dprdpr1intflopfifo1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dprdpr1intflopfifo1Metrics) Size() int {
	sz := 0

	sz += mtr.DataMuxForceBypassCrcFlopFfUndflow.Size()

	sz += mtr.DprCrcInfoFlopFfUndflow.Size()

	sz += mtr.DprCrcUpdateInfoFlopFfUndflow.Size()

	sz += mtr.DprCsumInfoFlopFfUndflow.Size()

	sz += mtr.DataMuxForceBypassCsumFlopFfUndflow.Size()

	sz += mtr.DprCsumUpdateInfoFlopFfUndflow.Size()

	sz += mtr.PtrEarlyPktEopInfoFlopFfUndflow.Size()

	sz += mtr.DataMuxEopErrFlopFfUndflow.Size()

	sz += mtr.PktinEopErrFlopFfUndflow.Size()

	sz += mtr.CsumErrFlopFfUndflow.Size()

	sz += mtr.CrcErrFlopFfUndflow.Size()

	sz += mtr.DataMuxDropFlopFfUndflow.Size()

	sz += mtr.PhvPktDataFlopFfUndflow.Size()

	sz += mtr.PktoutLenCellFlopFfUndflow.Size()

	sz += mtr.PaddingSizeFlopFfUndflow.Size()

	sz += mtr.PktinErrFlopFfUndflow.Size()

	sz += mtr.PhvNoDataFlopFfUndflow.Size()

	sz += mtr.PtrLookahaedFlopFfUndflow.Size()

	sz += mtr.EopVldFlopFfUndflow.Size()

	sz += mtr.CsumCalVldFlopFfUndflow.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.DataMuxForceBypassCrcFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DataMuxForceBypassCrcFlopFfUndflow.Size()

	mtr.DprCrcInfoFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DprCrcInfoFlopFfUndflow.Size()

	mtr.DprCrcUpdateInfoFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DprCrcUpdateInfoFlopFfUndflow.Size()

	mtr.DprCsumInfoFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DprCsumInfoFlopFfUndflow.Size()

	mtr.DataMuxForceBypassCsumFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DataMuxForceBypassCsumFlopFfUndflow.Size()

	mtr.DprCsumUpdateInfoFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DprCsumUpdateInfoFlopFfUndflow.Size()

	mtr.PtrEarlyPktEopInfoFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PtrEarlyPktEopInfoFlopFfUndflow.Size()

	mtr.DataMuxEopErrFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DataMuxEopErrFlopFfUndflow.Size()

	mtr.PktinEopErrFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktinEopErrFlopFfUndflow.Size()

	mtr.CsumErrFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CsumErrFlopFfUndflow.Size()

	mtr.CrcErrFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CrcErrFlopFfUndflow.Size()

	mtr.DataMuxDropFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DataMuxDropFlopFfUndflow.Size()

	mtr.PhvPktDataFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvPktDataFlopFfUndflow.Size()

	mtr.PktoutLenCellFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktoutLenCellFlopFfUndflow.Size()

	mtr.PaddingSizeFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PaddingSizeFlopFfUndflow.Size()

	mtr.PktinErrFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktinErrFlopFfUndflow.Size()

	mtr.PhvNoDataFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvNoDataFlopFfUndflow.Size()

	mtr.PtrLookahaedFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PtrLookahaedFlopFfUndflow.Size()

	mtr.EopVldFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.EopVldFlopFfUndflow.Size()

	mtr.CsumCalVldFlopFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CsumCalVldFlopFfUndflow.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "DataMuxForceBypassCrcFlopFfUndflow" {
		return offset
	}
	offset += mtr.DataMuxForceBypassCrcFlopFfUndflow.Size()

	if fldName == "DprCrcInfoFlopFfUndflow" {
		return offset
	}
	offset += mtr.DprCrcInfoFlopFfUndflow.Size()

	if fldName == "DprCrcUpdateInfoFlopFfUndflow" {
		return offset
	}
	offset += mtr.DprCrcUpdateInfoFlopFfUndflow.Size()

	if fldName == "DprCsumInfoFlopFfUndflow" {
		return offset
	}
	offset += mtr.DprCsumInfoFlopFfUndflow.Size()

	if fldName == "DataMuxForceBypassCsumFlopFfUndflow" {
		return offset
	}
	offset += mtr.DataMuxForceBypassCsumFlopFfUndflow.Size()

	if fldName == "DprCsumUpdateInfoFlopFfUndflow" {
		return offset
	}
	offset += mtr.DprCsumUpdateInfoFlopFfUndflow.Size()

	if fldName == "PtrEarlyPktEopInfoFlopFfUndflow" {
		return offset
	}
	offset += mtr.PtrEarlyPktEopInfoFlopFfUndflow.Size()

	if fldName == "DataMuxEopErrFlopFfUndflow" {
		return offset
	}
	offset += mtr.DataMuxEopErrFlopFfUndflow.Size()

	if fldName == "PktinEopErrFlopFfUndflow" {
		return offset
	}
	offset += mtr.PktinEopErrFlopFfUndflow.Size()

	if fldName == "CsumErrFlopFfUndflow" {
		return offset
	}
	offset += mtr.CsumErrFlopFfUndflow.Size()

	if fldName == "CrcErrFlopFfUndflow" {
		return offset
	}
	offset += mtr.CrcErrFlopFfUndflow.Size()

	if fldName == "DataMuxDropFlopFfUndflow" {
		return offset
	}
	offset += mtr.DataMuxDropFlopFfUndflow.Size()

	if fldName == "PhvPktDataFlopFfUndflow" {
		return offset
	}
	offset += mtr.PhvPktDataFlopFfUndflow.Size()

	if fldName == "PktoutLenCellFlopFfUndflow" {
		return offset
	}
	offset += mtr.PktoutLenCellFlopFfUndflow.Size()

	if fldName == "PaddingSizeFlopFfUndflow" {
		return offset
	}
	offset += mtr.PaddingSizeFlopFfUndflow.Size()

	if fldName == "PktinErrFlopFfUndflow" {
		return offset
	}
	offset += mtr.PktinErrFlopFfUndflow.Size()

	if fldName == "PhvNoDataFlopFfUndflow" {
		return offset
	}
	offset += mtr.PhvNoDataFlopFfUndflow.Size()

	if fldName == "PtrLookahaedFlopFfUndflow" {
		return offset
	}
	offset += mtr.PtrLookahaedFlopFfUndflow.Size()

	if fldName == "EopVldFlopFfUndflow" {
		return offset
	}
	offset += mtr.EopVldFlopFfUndflow.Size()

	if fldName == "CsumCalVldFlopFfUndflow" {
		return offset
	}
	offset += mtr.CsumCalVldFlopFfUndflow.Size()

	return offset
}

// SetDataMuxForceBypassCrcFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) SetDataMuxForceBypassCrcFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DataMuxForceBypassCrcFlopFfUndflow"))
	return nil
}

// SetDprCrcInfoFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) SetDprCrcInfoFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCrcInfoFlopFfUndflow"))
	return nil
}

// SetDprCrcUpdateInfoFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) SetDprCrcUpdateInfoFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCrcUpdateInfoFlopFfUndflow"))
	return nil
}

// SetDprCsumInfoFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) SetDprCsumInfoFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCsumInfoFlopFfUndflow"))
	return nil
}

// SetDataMuxForceBypassCsumFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) SetDataMuxForceBypassCsumFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DataMuxForceBypassCsumFlopFfUndflow"))
	return nil
}

// SetDprCsumUpdateInfoFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) SetDprCsumUpdateInfoFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCsumUpdateInfoFlopFfUndflow"))
	return nil
}

// SetPtrEarlyPktEopInfoFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) SetPtrEarlyPktEopInfoFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PtrEarlyPktEopInfoFlopFfUndflow"))
	return nil
}

// SetDataMuxEopErrFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) SetDataMuxEopErrFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DataMuxEopErrFlopFfUndflow"))
	return nil
}

// SetPktinEopErrFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) SetPktinEopErrFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktinEopErrFlopFfUndflow"))
	return nil
}

// SetCsumErrFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) SetCsumErrFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CsumErrFlopFfUndflow"))
	return nil
}

// SetCrcErrFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) SetCrcErrFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CrcErrFlopFfUndflow"))
	return nil
}

// SetDataMuxDropFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) SetDataMuxDropFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DataMuxDropFlopFfUndflow"))
	return nil
}

// SetPhvPktDataFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) SetPhvPktDataFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvPktDataFlopFfUndflow"))
	return nil
}

// SetPktoutLenCellFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) SetPktoutLenCellFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktoutLenCellFlopFfUndflow"))
	return nil
}

// SetPaddingSizeFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) SetPaddingSizeFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PaddingSizeFlopFfUndflow"))
	return nil
}

// SetPktinErrFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) SetPktinErrFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktinErrFlopFfUndflow"))
	return nil
}

// SetPhvNoDataFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) SetPhvNoDataFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvNoDataFlopFfUndflow"))
	return nil
}

// SetPtrLookahaedFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) SetPtrLookahaedFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PtrLookahaedFlopFfUndflow"))
	return nil
}

// SetEopVldFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) SetEopVldFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopVldFlopFfUndflow"))
	return nil
}

// SetCsumCalVldFlopFfUndflow sets cunter in shared memory
func (mtr *Dprdpr1intflopfifo1Metrics) SetCsumCalVldFlopFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CsumCalVldFlopFfUndflow"))
	return nil
}

// Dprdpr1intflopfifo1MetricsIterator is the iterator object
type Dprdpr1intflopfifo1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dprdpr1intflopfifo1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dprdpr1intflopfifo1MetricsIterator) Next() *Dprdpr1intflopfifo1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dprdpr1intflopfifo1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dprdpr1intflopfifo1MetricsIterator) Find(key uint64) (*Dprdpr1intflopfifo1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dprdpr1intflopfifo1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dprdpr1intflopfifo1MetricsIterator) Create(key uint64) (*Dprdpr1intflopfifo1Metrics, error) {
	tmtr := &Dprdpr1intflopfifo1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dprdpr1intflopfifo1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dprdpr1intflopfifo1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dprdpr1intflopfifo1MetricsIterator) Free() {
	it.iter.Free()
}

// NewDprdpr1intflopfifo1MetricsIterator returns an iterator
func NewDprdpr1intflopfifo1MetricsIterator() (*Dprdpr1intflopfifo1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dprdpr1intflopfifo1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dprdpr1intflopfifo1MetricsIterator{iter: iter}, nil
}

type Dprdpr1intspareMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Spare_0 metrics.Counter

	Spare_1 metrics.Counter

	Spare_2 metrics.Counter

	Spare_3 metrics.Counter

	Spare_4 metrics.Counter

	Spare_5 metrics.Counter

	Spare_6 metrics.Counter

	Spare_7 metrics.Counter

	Spare_8 metrics.Counter

	Spare_9 metrics.Counter

	Spare_10 metrics.Counter

	Spare_11 metrics.Counter

	Spare_12 metrics.Counter

	Spare_13 metrics.Counter

	Spare_14 metrics.Counter

	Spare_15 metrics.Counter

	Spare_16 metrics.Counter

	Spare_17 metrics.Counter

	Spare_18 metrics.Counter

	Spare_19 metrics.Counter

	Spare_20 metrics.Counter

	Spare_21 metrics.Counter

	Spare_22 metrics.Counter

	Spare_23 metrics.Counter

	Spare_24 metrics.Counter

	Spare_25 metrics.Counter

	Spare_26 metrics.Counter

	Spare_27 metrics.Counter

	Spare_28 metrics.Counter

	Spare_29 metrics.Counter

	Spare_30 metrics.Counter

	Spare_31 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dprdpr1intspareMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dprdpr1intspareMetrics) Size() int {
	sz := 0

	sz += mtr.Spare_0.Size()

	sz += mtr.Spare_1.Size()

	sz += mtr.Spare_2.Size()

	sz += mtr.Spare_3.Size()

	sz += mtr.Spare_4.Size()

	sz += mtr.Spare_5.Size()

	sz += mtr.Spare_6.Size()

	sz += mtr.Spare_7.Size()

	sz += mtr.Spare_8.Size()

	sz += mtr.Spare_9.Size()

	sz += mtr.Spare_10.Size()

	sz += mtr.Spare_11.Size()

	sz += mtr.Spare_12.Size()

	sz += mtr.Spare_13.Size()

	sz += mtr.Spare_14.Size()

	sz += mtr.Spare_15.Size()

	sz += mtr.Spare_16.Size()

	sz += mtr.Spare_17.Size()

	sz += mtr.Spare_18.Size()

	sz += mtr.Spare_19.Size()

	sz += mtr.Spare_20.Size()

	sz += mtr.Spare_21.Size()

	sz += mtr.Spare_22.Size()

	sz += mtr.Spare_23.Size()

	sz += mtr.Spare_24.Size()

	sz += mtr.Spare_25.Size()

	sz += mtr.Spare_26.Size()

	sz += mtr.Spare_27.Size()

	sz += mtr.Spare_28.Size()

	sz += mtr.Spare_29.Size()

	sz += mtr.Spare_30.Size()

	sz += mtr.Spare_31.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dprdpr1intspareMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Spare_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_0.Size()

	mtr.Spare_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_1.Size()

	mtr.Spare_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_2.Size()

	mtr.Spare_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_3.Size()

	mtr.Spare_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_4.Size()

	mtr.Spare_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_5.Size()

	mtr.Spare_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_6.Size()

	mtr.Spare_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_7.Size()

	mtr.Spare_8 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_8.Size()

	mtr.Spare_9 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_9.Size()

	mtr.Spare_10 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_10.Size()

	mtr.Spare_11 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_11.Size()

	mtr.Spare_12 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_12.Size()

	mtr.Spare_13 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_13.Size()

	mtr.Spare_14 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_14.Size()

	mtr.Spare_15 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_15.Size()

	mtr.Spare_16 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_16.Size()

	mtr.Spare_17 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_17.Size()

	mtr.Spare_18 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_18.Size()

	mtr.Spare_19 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_19.Size()

	mtr.Spare_20 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_20.Size()

	mtr.Spare_21 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_21.Size()

	mtr.Spare_22 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_22.Size()

	mtr.Spare_23 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_23.Size()

	mtr.Spare_24 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_24.Size()

	mtr.Spare_25 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_25.Size()

	mtr.Spare_26 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_26.Size()

	mtr.Spare_27 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_27.Size()

	mtr.Spare_28 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_28.Size()

	mtr.Spare_29 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_29.Size()

	mtr.Spare_30 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_30.Size()

	mtr.Spare_31 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_31.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dprdpr1intspareMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Spare_0" {
		return offset
	}
	offset += mtr.Spare_0.Size()

	if fldName == "Spare_1" {
		return offset
	}
	offset += mtr.Spare_1.Size()

	if fldName == "Spare_2" {
		return offset
	}
	offset += mtr.Spare_2.Size()

	if fldName == "Spare_3" {
		return offset
	}
	offset += mtr.Spare_3.Size()

	if fldName == "Spare_4" {
		return offset
	}
	offset += mtr.Spare_4.Size()

	if fldName == "Spare_5" {
		return offset
	}
	offset += mtr.Spare_5.Size()

	if fldName == "Spare_6" {
		return offset
	}
	offset += mtr.Spare_6.Size()

	if fldName == "Spare_7" {
		return offset
	}
	offset += mtr.Spare_7.Size()

	if fldName == "Spare_8" {
		return offset
	}
	offset += mtr.Spare_8.Size()

	if fldName == "Spare_9" {
		return offset
	}
	offset += mtr.Spare_9.Size()

	if fldName == "Spare_10" {
		return offset
	}
	offset += mtr.Spare_10.Size()

	if fldName == "Spare_11" {
		return offset
	}
	offset += mtr.Spare_11.Size()

	if fldName == "Spare_12" {
		return offset
	}
	offset += mtr.Spare_12.Size()

	if fldName == "Spare_13" {
		return offset
	}
	offset += mtr.Spare_13.Size()

	if fldName == "Spare_14" {
		return offset
	}
	offset += mtr.Spare_14.Size()

	if fldName == "Spare_15" {
		return offset
	}
	offset += mtr.Spare_15.Size()

	if fldName == "Spare_16" {
		return offset
	}
	offset += mtr.Spare_16.Size()

	if fldName == "Spare_17" {
		return offset
	}
	offset += mtr.Spare_17.Size()

	if fldName == "Spare_18" {
		return offset
	}
	offset += mtr.Spare_18.Size()

	if fldName == "Spare_19" {
		return offset
	}
	offset += mtr.Spare_19.Size()

	if fldName == "Spare_20" {
		return offset
	}
	offset += mtr.Spare_20.Size()

	if fldName == "Spare_21" {
		return offset
	}
	offset += mtr.Spare_21.Size()

	if fldName == "Spare_22" {
		return offset
	}
	offset += mtr.Spare_22.Size()

	if fldName == "Spare_23" {
		return offset
	}
	offset += mtr.Spare_23.Size()

	if fldName == "Spare_24" {
		return offset
	}
	offset += mtr.Spare_24.Size()

	if fldName == "Spare_25" {
		return offset
	}
	offset += mtr.Spare_25.Size()

	if fldName == "Spare_26" {
		return offset
	}
	offset += mtr.Spare_26.Size()

	if fldName == "Spare_27" {
		return offset
	}
	offset += mtr.Spare_27.Size()

	if fldName == "Spare_28" {
		return offset
	}
	offset += mtr.Spare_28.Size()

	if fldName == "Spare_29" {
		return offset
	}
	offset += mtr.Spare_29.Size()

	if fldName == "Spare_30" {
		return offset
	}
	offset += mtr.Spare_30.Size()

	if fldName == "Spare_31" {
		return offset
	}
	offset += mtr.Spare_31.Size()

	return offset
}

// SetSpare_0 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_0"))
	return nil
}

// SetSpare_1 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_1"))
	return nil
}

// SetSpare_2 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_2"))
	return nil
}

// SetSpare_3 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_3"))
	return nil
}

// SetSpare_4 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_4"))
	return nil
}

// SetSpare_5 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_5"))
	return nil
}

// SetSpare_6 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_6"))
	return nil
}

// SetSpare_7 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_7"))
	return nil
}

// SetSpare_8 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_8"))
	return nil
}

// SetSpare_9 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_9(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_9"))
	return nil
}

// SetSpare_10 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_10(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_10"))
	return nil
}

// SetSpare_11 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_11(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_11"))
	return nil
}

// SetSpare_12 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_12(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_12"))
	return nil
}

// SetSpare_13 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_13(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_13"))
	return nil
}

// SetSpare_14 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_14(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_14"))
	return nil
}

// SetSpare_15 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_15(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_15"))
	return nil
}

// SetSpare_16 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_16(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_16"))
	return nil
}

// SetSpare_17 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_17(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_17"))
	return nil
}

// SetSpare_18 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_18(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_18"))
	return nil
}

// SetSpare_19 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_19(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_19"))
	return nil
}

// SetSpare_20 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_20(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_20"))
	return nil
}

// SetSpare_21 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_21(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_21"))
	return nil
}

// SetSpare_22 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_22(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_22"))
	return nil
}

// SetSpare_23 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_23(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_23"))
	return nil
}

// SetSpare_24 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_24(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_24"))
	return nil
}

// SetSpare_25 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_25(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_25"))
	return nil
}

// SetSpare_26 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_26(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_26"))
	return nil
}

// SetSpare_27 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_27(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_27"))
	return nil
}

// SetSpare_28 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_28(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_28"))
	return nil
}

// SetSpare_29 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_29(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_29"))
	return nil
}

// SetSpare_30 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_30(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_30"))
	return nil
}

// SetSpare_31 sets cunter in shared memory
func (mtr *Dprdpr1intspareMetrics) SetSpare_31(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_31"))
	return nil
}

// Dprdpr1intspareMetricsIterator is the iterator object
type Dprdpr1intspareMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dprdpr1intspareMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dprdpr1intspareMetricsIterator) Next() *Dprdpr1intspareMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dprdpr1intspareMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dprdpr1intspareMetricsIterator) Find(key uint64) (*Dprdpr1intspareMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dprdpr1intspareMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dprdpr1intspareMetricsIterator) Create(key uint64) (*Dprdpr1intspareMetrics, error) {
	tmtr := &Dprdpr1intspareMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dprdpr1intspareMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dprdpr1intspareMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dprdpr1intspareMetricsIterator) Free() {
	it.iter.Free()
}

// NewDprdpr1intspareMetricsIterator returns an iterator
func NewDprdpr1intspareMetricsIterator() (*Dprdpr1intspareMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dprdpr1intspareMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dprdpr1intspareMetricsIterator{iter: iter}, nil
}

type Dprdpr1intcreditMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	EgressCreditOvflow metrics.Counter

	EgressCreditUndflow metrics.Counter

	PktoutCreditOvflow metrics.Counter

	PktoutCreditUndflow metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dprdpr1intcreditMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dprdpr1intcreditMetrics) Size() int {
	sz := 0

	sz += mtr.EgressCreditOvflow.Size()

	sz += mtr.EgressCreditUndflow.Size()

	sz += mtr.PktoutCreditOvflow.Size()

	sz += mtr.PktoutCreditUndflow.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dprdpr1intcreditMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.EgressCreditOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.EgressCreditOvflow.Size()

	mtr.EgressCreditUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.EgressCreditUndflow.Size()

	mtr.PktoutCreditOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktoutCreditOvflow.Size()

	mtr.PktoutCreditUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktoutCreditUndflow.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dprdpr1intcreditMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "EgressCreditOvflow" {
		return offset
	}
	offset += mtr.EgressCreditOvflow.Size()

	if fldName == "EgressCreditUndflow" {
		return offset
	}
	offset += mtr.EgressCreditUndflow.Size()

	if fldName == "PktoutCreditOvflow" {
		return offset
	}
	offset += mtr.PktoutCreditOvflow.Size()

	if fldName == "PktoutCreditUndflow" {
		return offset
	}
	offset += mtr.PktoutCreditUndflow.Size()

	return offset
}

// SetEgressCreditOvflow sets cunter in shared memory
func (mtr *Dprdpr1intcreditMetrics) SetEgressCreditOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EgressCreditOvflow"))
	return nil
}

// SetEgressCreditUndflow sets cunter in shared memory
func (mtr *Dprdpr1intcreditMetrics) SetEgressCreditUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EgressCreditUndflow"))
	return nil
}

// SetPktoutCreditOvflow sets cunter in shared memory
func (mtr *Dprdpr1intcreditMetrics) SetPktoutCreditOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktoutCreditOvflow"))
	return nil
}

// SetPktoutCreditUndflow sets cunter in shared memory
func (mtr *Dprdpr1intcreditMetrics) SetPktoutCreditUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktoutCreditUndflow"))
	return nil
}

// Dprdpr1intcreditMetricsIterator is the iterator object
type Dprdpr1intcreditMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dprdpr1intcreditMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dprdpr1intcreditMetricsIterator) Next() *Dprdpr1intcreditMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dprdpr1intcreditMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dprdpr1intcreditMetricsIterator) Find(key uint64) (*Dprdpr1intcreditMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dprdpr1intcreditMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dprdpr1intcreditMetricsIterator) Create(key uint64) (*Dprdpr1intcreditMetrics, error) {
	tmtr := &Dprdpr1intcreditMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dprdpr1intcreditMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dprdpr1intcreditMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dprdpr1intcreditMetricsIterator) Free() {
	it.iter.Free()
}

// NewDprdpr1intcreditMetricsIterator returns an iterator
func NewDprdpr1intcreditMetricsIterator() (*Dprdpr1intcreditMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dprdpr1intcreditMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dprdpr1intcreditMetricsIterator{iter: iter}, nil
}

type Mcmc0intmcMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MchInt metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Mcmc0intmcMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Mcmc0intmcMetrics) Size() int {
	sz := 0

	sz += mtr.MchInt.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Mcmc0intmcMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MchInt = mtr.metrics.GetCounter(offset)
	offset += mtr.MchInt.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Mcmc0intmcMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MchInt" {
		return offset
	}
	offset += mtr.MchInt.Size()

	return offset
}

// SetMchInt sets cunter in shared memory
func (mtr *Mcmc0intmcMetrics) SetMchInt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MchInt"))
	return nil
}

// Mcmc0intmcMetricsIterator is the iterator object
type Mcmc0intmcMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Mcmc0intmcMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Mcmc0intmcMetricsIterator) Next() *Mcmc0intmcMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Mcmc0intmcMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Mcmc0intmcMetricsIterator) Find(key uint64) (*Mcmc0intmcMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Mcmc0intmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Mcmc0intmcMetricsIterator) Create(key uint64) (*Mcmc0intmcMetrics, error) {
	tmtr := &Mcmc0intmcMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Mcmc0intmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Mcmc0intmcMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Mcmc0intmcMetricsIterator) Free() {
	it.iter.Free()
}

// NewMcmc0intmcMetricsIterator returns an iterator
func NewMcmc0intmcMetricsIterator() (*Mcmc0intmcMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Mcmc0intmcMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Mcmc0intmcMetricsIterator{iter: iter}, nil
}

type Mcmc0mchintmcMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Ecc_1BitThreshPs1 metrics.Counter

	Ecc_1BitThreshPs0 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Mcmc0mchintmcMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Mcmc0mchintmcMetrics) Size() int {
	sz := 0

	sz += mtr.Ecc_1BitThreshPs1.Size()

	sz += mtr.Ecc_1BitThreshPs0.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Mcmc0mchintmcMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Ecc_1BitThreshPs1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Ecc_1BitThreshPs1.Size()

	mtr.Ecc_1BitThreshPs0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Ecc_1BitThreshPs0.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Mcmc0mchintmcMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Ecc_1BitThreshPs1" {
		return offset
	}
	offset += mtr.Ecc_1BitThreshPs1.Size()

	if fldName == "Ecc_1BitThreshPs0" {
		return offset
	}
	offset += mtr.Ecc_1BitThreshPs0.Size()

	return offset
}

// SetEcc_1BitThreshPs1 sets cunter in shared memory
func (mtr *Mcmc0mchintmcMetrics) SetEcc_1BitThreshPs1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ecc_1BitThreshPs1"))
	return nil
}

// SetEcc_1BitThreshPs0 sets cunter in shared memory
func (mtr *Mcmc0mchintmcMetrics) SetEcc_1BitThreshPs0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ecc_1BitThreshPs0"))
	return nil
}

// Mcmc0mchintmcMetricsIterator is the iterator object
type Mcmc0mchintmcMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Mcmc0mchintmcMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Mcmc0mchintmcMetricsIterator) Next() *Mcmc0mchintmcMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Mcmc0mchintmcMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Mcmc0mchintmcMetricsIterator) Find(key uint64) (*Mcmc0mchintmcMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Mcmc0mchintmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Mcmc0mchintmcMetricsIterator) Create(key uint64) (*Mcmc0mchintmcMetrics, error) {
	tmtr := &Mcmc0mchintmcMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Mcmc0mchintmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Mcmc0mchintmcMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Mcmc0mchintmcMetricsIterator) Free() {
	it.iter.Free()
}

// NewMcmc0mchintmcMetricsIterator returns an iterator
func NewMcmc0mchintmcMetricsIterator() (*Mcmc0mchintmcMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Mcmc0mchintmcMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Mcmc0mchintmcMetricsIterator{iter: iter}, nil
}

type Mcmc1intmcMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MchInt metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Mcmc1intmcMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Mcmc1intmcMetrics) Size() int {
	sz := 0

	sz += mtr.MchInt.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Mcmc1intmcMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MchInt = mtr.metrics.GetCounter(offset)
	offset += mtr.MchInt.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Mcmc1intmcMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MchInt" {
		return offset
	}
	offset += mtr.MchInt.Size()

	return offset
}

// SetMchInt sets cunter in shared memory
func (mtr *Mcmc1intmcMetrics) SetMchInt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MchInt"))
	return nil
}

// Mcmc1intmcMetricsIterator is the iterator object
type Mcmc1intmcMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Mcmc1intmcMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Mcmc1intmcMetricsIterator) Next() *Mcmc1intmcMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Mcmc1intmcMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Mcmc1intmcMetricsIterator) Find(key uint64) (*Mcmc1intmcMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Mcmc1intmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Mcmc1intmcMetricsIterator) Create(key uint64) (*Mcmc1intmcMetrics, error) {
	tmtr := &Mcmc1intmcMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Mcmc1intmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Mcmc1intmcMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Mcmc1intmcMetricsIterator) Free() {
	it.iter.Free()
}

// NewMcmc1intmcMetricsIterator returns an iterator
func NewMcmc1intmcMetricsIterator() (*Mcmc1intmcMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Mcmc1intmcMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Mcmc1intmcMetricsIterator{iter: iter}, nil
}

type Mcmc1mchintmcMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Ecc_1BitThreshPs1 metrics.Counter

	Ecc_1BitThreshPs0 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Mcmc1mchintmcMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Mcmc1mchintmcMetrics) Size() int {
	sz := 0

	sz += mtr.Ecc_1BitThreshPs1.Size()

	sz += mtr.Ecc_1BitThreshPs0.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Mcmc1mchintmcMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Ecc_1BitThreshPs1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Ecc_1BitThreshPs1.Size()

	mtr.Ecc_1BitThreshPs0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Ecc_1BitThreshPs0.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Mcmc1mchintmcMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Ecc_1BitThreshPs1" {
		return offset
	}
	offset += mtr.Ecc_1BitThreshPs1.Size()

	if fldName == "Ecc_1BitThreshPs0" {
		return offset
	}
	offset += mtr.Ecc_1BitThreshPs0.Size()

	return offset
}

// SetEcc_1BitThreshPs1 sets cunter in shared memory
func (mtr *Mcmc1mchintmcMetrics) SetEcc_1BitThreshPs1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ecc_1BitThreshPs1"))
	return nil
}

// SetEcc_1BitThreshPs0 sets cunter in shared memory
func (mtr *Mcmc1mchintmcMetrics) SetEcc_1BitThreshPs0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ecc_1BitThreshPs0"))
	return nil
}

// Mcmc1mchintmcMetricsIterator is the iterator object
type Mcmc1mchintmcMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Mcmc1mchintmcMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Mcmc1mchintmcMetricsIterator) Next() *Mcmc1mchintmcMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Mcmc1mchintmcMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Mcmc1mchintmcMetricsIterator) Find(key uint64) (*Mcmc1mchintmcMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Mcmc1mchintmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Mcmc1mchintmcMetricsIterator) Create(key uint64) (*Mcmc1mchintmcMetrics, error) {
	tmtr := &Mcmc1mchintmcMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Mcmc1mchintmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Mcmc1mchintmcMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Mcmc1mchintmcMetricsIterator) Free() {
	it.iter.Free()
}

// NewMcmc1mchintmcMetricsIterator returns an iterator
func NewMcmc1mchintmcMetricsIterator() (*Mcmc1mchintmcMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Mcmc1mchintmcMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Mcmc1mchintmcMetricsIterator{iter: iter}, nil
}

type Mcmc2intmcMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MchInt metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Mcmc2intmcMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Mcmc2intmcMetrics) Size() int {
	sz := 0

	sz += mtr.MchInt.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Mcmc2intmcMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MchInt = mtr.metrics.GetCounter(offset)
	offset += mtr.MchInt.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Mcmc2intmcMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MchInt" {
		return offset
	}
	offset += mtr.MchInt.Size()

	return offset
}

// SetMchInt sets cunter in shared memory
func (mtr *Mcmc2intmcMetrics) SetMchInt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MchInt"))
	return nil
}

// Mcmc2intmcMetricsIterator is the iterator object
type Mcmc2intmcMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Mcmc2intmcMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Mcmc2intmcMetricsIterator) Next() *Mcmc2intmcMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Mcmc2intmcMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Mcmc2intmcMetricsIterator) Find(key uint64) (*Mcmc2intmcMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Mcmc2intmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Mcmc2intmcMetricsIterator) Create(key uint64) (*Mcmc2intmcMetrics, error) {
	tmtr := &Mcmc2intmcMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Mcmc2intmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Mcmc2intmcMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Mcmc2intmcMetricsIterator) Free() {
	it.iter.Free()
}

// NewMcmc2intmcMetricsIterator returns an iterator
func NewMcmc2intmcMetricsIterator() (*Mcmc2intmcMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Mcmc2intmcMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Mcmc2intmcMetricsIterator{iter: iter}, nil
}

type Mcmc2mchintmcMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Ecc_1BitThreshPs1 metrics.Counter

	Ecc_1BitThreshPs0 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Mcmc2mchintmcMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Mcmc2mchintmcMetrics) Size() int {
	sz := 0

	sz += mtr.Ecc_1BitThreshPs1.Size()

	sz += mtr.Ecc_1BitThreshPs0.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Mcmc2mchintmcMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Ecc_1BitThreshPs1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Ecc_1BitThreshPs1.Size()

	mtr.Ecc_1BitThreshPs0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Ecc_1BitThreshPs0.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Mcmc2mchintmcMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Ecc_1BitThreshPs1" {
		return offset
	}
	offset += mtr.Ecc_1BitThreshPs1.Size()

	if fldName == "Ecc_1BitThreshPs0" {
		return offset
	}
	offset += mtr.Ecc_1BitThreshPs0.Size()

	return offset
}

// SetEcc_1BitThreshPs1 sets cunter in shared memory
func (mtr *Mcmc2mchintmcMetrics) SetEcc_1BitThreshPs1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ecc_1BitThreshPs1"))
	return nil
}

// SetEcc_1BitThreshPs0 sets cunter in shared memory
func (mtr *Mcmc2mchintmcMetrics) SetEcc_1BitThreshPs0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ecc_1BitThreshPs0"))
	return nil
}

// Mcmc2mchintmcMetricsIterator is the iterator object
type Mcmc2mchintmcMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Mcmc2mchintmcMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Mcmc2mchintmcMetricsIterator) Next() *Mcmc2mchintmcMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Mcmc2mchintmcMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Mcmc2mchintmcMetricsIterator) Find(key uint64) (*Mcmc2mchintmcMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Mcmc2mchintmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Mcmc2mchintmcMetricsIterator) Create(key uint64) (*Mcmc2mchintmcMetrics, error) {
	tmtr := &Mcmc2mchintmcMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Mcmc2mchintmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Mcmc2mchintmcMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Mcmc2mchintmcMetricsIterator) Free() {
	it.iter.Free()
}

// NewMcmc2mchintmcMetricsIterator returns an iterator
func NewMcmc2mchintmcMetricsIterator() (*Mcmc2mchintmcMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Mcmc2mchintmcMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Mcmc2mchintmcMetricsIterator{iter: iter}, nil
}

type Mcmc3intmcMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MchInt metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Mcmc3intmcMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Mcmc3intmcMetrics) Size() int {
	sz := 0

	sz += mtr.MchInt.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Mcmc3intmcMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MchInt = mtr.metrics.GetCounter(offset)
	offset += mtr.MchInt.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Mcmc3intmcMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MchInt" {
		return offset
	}
	offset += mtr.MchInt.Size()

	return offset
}

// SetMchInt sets cunter in shared memory
func (mtr *Mcmc3intmcMetrics) SetMchInt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MchInt"))
	return nil
}

// Mcmc3intmcMetricsIterator is the iterator object
type Mcmc3intmcMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Mcmc3intmcMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Mcmc3intmcMetricsIterator) Next() *Mcmc3intmcMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Mcmc3intmcMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Mcmc3intmcMetricsIterator) Find(key uint64) (*Mcmc3intmcMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Mcmc3intmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Mcmc3intmcMetricsIterator) Create(key uint64) (*Mcmc3intmcMetrics, error) {
	tmtr := &Mcmc3intmcMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Mcmc3intmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Mcmc3intmcMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Mcmc3intmcMetricsIterator) Free() {
	it.iter.Free()
}

// NewMcmc3intmcMetricsIterator returns an iterator
func NewMcmc3intmcMetricsIterator() (*Mcmc3intmcMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Mcmc3intmcMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Mcmc3intmcMetricsIterator{iter: iter}, nil
}

type Mcmc3mchintmcMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Ecc_1BitThreshPs1 metrics.Counter

	Ecc_1BitThreshPs0 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Mcmc3mchintmcMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Mcmc3mchintmcMetrics) Size() int {
	sz := 0

	sz += mtr.Ecc_1BitThreshPs1.Size()

	sz += mtr.Ecc_1BitThreshPs0.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Mcmc3mchintmcMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Ecc_1BitThreshPs1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Ecc_1BitThreshPs1.Size()

	mtr.Ecc_1BitThreshPs0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Ecc_1BitThreshPs0.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Mcmc3mchintmcMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Ecc_1BitThreshPs1" {
		return offset
	}
	offset += mtr.Ecc_1BitThreshPs1.Size()

	if fldName == "Ecc_1BitThreshPs0" {
		return offset
	}
	offset += mtr.Ecc_1BitThreshPs0.Size()

	return offset
}

// SetEcc_1BitThreshPs1 sets cunter in shared memory
func (mtr *Mcmc3mchintmcMetrics) SetEcc_1BitThreshPs1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ecc_1BitThreshPs1"))
	return nil
}

// SetEcc_1BitThreshPs0 sets cunter in shared memory
func (mtr *Mcmc3mchintmcMetrics) SetEcc_1BitThreshPs0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ecc_1BitThreshPs0"))
	return nil
}

// Mcmc3mchintmcMetricsIterator is the iterator object
type Mcmc3mchintmcMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Mcmc3mchintmcMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Mcmc3mchintmcMetricsIterator) Next() *Mcmc3mchintmcMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Mcmc3mchintmcMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Mcmc3mchintmcMetricsIterator) Find(key uint64) (*Mcmc3mchintmcMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Mcmc3mchintmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Mcmc3mchintmcMetricsIterator) Create(key uint64) (*Mcmc3mchintmcMetrics, error) {
	tmtr := &Mcmc3mchintmcMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Mcmc3mchintmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Mcmc3mchintmcMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Mcmc3mchintmcMetricsIterator) Free() {
	it.iter.Free()
}

// NewMcmc3mchintmcMetricsIterator returns an iterator
func NewMcmc3mchintmcMetricsIterator() (*Mcmc3mchintmcMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Mcmc3mchintmcMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Mcmc3mchintmcMetricsIterator{iter: iter}, nil
}

type Mcmc4intmcMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MchInt metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Mcmc4intmcMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Mcmc4intmcMetrics) Size() int {
	sz := 0

	sz += mtr.MchInt.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Mcmc4intmcMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MchInt = mtr.metrics.GetCounter(offset)
	offset += mtr.MchInt.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Mcmc4intmcMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MchInt" {
		return offset
	}
	offset += mtr.MchInt.Size()

	return offset
}

// SetMchInt sets cunter in shared memory
func (mtr *Mcmc4intmcMetrics) SetMchInt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MchInt"))
	return nil
}

// Mcmc4intmcMetricsIterator is the iterator object
type Mcmc4intmcMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Mcmc4intmcMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Mcmc4intmcMetricsIterator) Next() *Mcmc4intmcMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Mcmc4intmcMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Mcmc4intmcMetricsIterator) Find(key uint64) (*Mcmc4intmcMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Mcmc4intmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Mcmc4intmcMetricsIterator) Create(key uint64) (*Mcmc4intmcMetrics, error) {
	tmtr := &Mcmc4intmcMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Mcmc4intmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Mcmc4intmcMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Mcmc4intmcMetricsIterator) Free() {
	it.iter.Free()
}

// NewMcmc4intmcMetricsIterator returns an iterator
func NewMcmc4intmcMetricsIterator() (*Mcmc4intmcMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Mcmc4intmcMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Mcmc4intmcMetricsIterator{iter: iter}, nil
}

type Mcmc4mchintmcMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Ecc_1BitThreshPs1 metrics.Counter

	Ecc_1BitThreshPs0 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Mcmc4mchintmcMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Mcmc4mchintmcMetrics) Size() int {
	sz := 0

	sz += mtr.Ecc_1BitThreshPs1.Size()

	sz += mtr.Ecc_1BitThreshPs0.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Mcmc4mchintmcMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Ecc_1BitThreshPs1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Ecc_1BitThreshPs1.Size()

	mtr.Ecc_1BitThreshPs0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Ecc_1BitThreshPs0.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Mcmc4mchintmcMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Ecc_1BitThreshPs1" {
		return offset
	}
	offset += mtr.Ecc_1BitThreshPs1.Size()

	if fldName == "Ecc_1BitThreshPs0" {
		return offset
	}
	offset += mtr.Ecc_1BitThreshPs0.Size()

	return offset
}

// SetEcc_1BitThreshPs1 sets cunter in shared memory
func (mtr *Mcmc4mchintmcMetrics) SetEcc_1BitThreshPs1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ecc_1BitThreshPs1"))
	return nil
}

// SetEcc_1BitThreshPs0 sets cunter in shared memory
func (mtr *Mcmc4mchintmcMetrics) SetEcc_1BitThreshPs0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ecc_1BitThreshPs0"))
	return nil
}

// Mcmc4mchintmcMetricsIterator is the iterator object
type Mcmc4mchintmcMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Mcmc4mchintmcMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Mcmc4mchintmcMetricsIterator) Next() *Mcmc4mchintmcMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Mcmc4mchintmcMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Mcmc4mchintmcMetricsIterator) Find(key uint64) (*Mcmc4mchintmcMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Mcmc4mchintmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Mcmc4mchintmcMetricsIterator) Create(key uint64) (*Mcmc4mchintmcMetrics, error) {
	tmtr := &Mcmc4mchintmcMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Mcmc4mchintmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Mcmc4mchintmcMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Mcmc4mchintmcMetricsIterator) Free() {
	it.iter.Free()
}

// NewMcmc4mchintmcMetricsIterator returns an iterator
func NewMcmc4mchintmcMetricsIterator() (*Mcmc4mchintmcMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Mcmc4mchintmcMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Mcmc4mchintmcMetricsIterator{iter: iter}, nil
}

type Mcmc5intmcMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MchInt metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Mcmc5intmcMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Mcmc5intmcMetrics) Size() int {
	sz := 0

	sz += mtr.MchInt.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Mcmc5intmcMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MchInt = mtr.metrics.GetCounter(offset)
	offset += mtr.MchInt.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Mcmc5intmcMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MchInt" {
		return offset
	}
	offset += mtr.MchInt.Size()

	return offset
}

// SetMchInt sets cunter in shared memory
func (mtr *Mcmc5intmcMetrics) SetMchInt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MchInt"))
	return nil
}

// Mcmc5intmcMetricsIterator is the iterator object
type Mcmc5intmcMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Mcmc5intmcMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Mcmc5intmcMetricsIterator) Next() *Mcmc5intmcMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Mcmc5intmcMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Mcmc5intmcMetricsIterator) Find(key uint64) (*Mcmc5intmcMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Mcmc5intmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Mcmc5intmcMetricsIterator) Create(key uint64) (*Mcmc5intmcMetrics, error) {
	tmtr := &Mcmc5intmcMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Mcmc5intmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Mcmc5intmcMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Mcmc5intmcMetricsIterator) Free() {
	it.iter.Free()
}

// NewMcmc5intmcMetricsIterator returns an iterator
func NewMcmc5intmcMetricsIterator() (*Mcmc5intmcMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Mcmc5intmcMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Mcmc5intmcMetricsIterator{iter: iter}, nil
}

type Mcmc5mchintmcMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Ecc_1BitThreshPs1 metrics.Counter

	Ecc_1BitThreshPs0 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Mcmc5mchintmcMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Mcmc5mchintmcMetrics) Size() int {
	sz := 0

	sz += mtr.Ecc_1BitThreshPs1.Size()

	sz += mtr.Ecc_1BitThreshPs0.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Mcmc5mchintmcMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Ecc_1BitThreshPs1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Ecc_1BitThreshPs1.Size()

	mtr.Ecc_1BitThreshPs0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Ecc_1BitThreshPs0.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Mcmc5mchintmcMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Ecc_1BitThreshPs1" {
		return offset
	}
	offset += mtr.Ecc_1BitThreshPs1.Size()

	if fldName == "Ecc_1BitThreshPs0" {
		return offset
	}
	offset += mtr.Ecc_1BitThreshPs0.Size()

	return offset
}

// SetEcc_1BitThreshPs1 sets cunter in shared memory
func (mtr *Mcmc5mchintmcMetrics) SetEcc_1BitThreshPs1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ecc_1BitThreshPs1"))
	return nil
}

// SetEcc_1BitThreshPs0 sets cunter in shared memory
func (mtr *Mcmc5mchintmcMetrics) SetEcc_1BitThreshPs0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ecc_1BitThreshPs0"))
	return nil
}

// Mcmc5mchintmcMetricsIterator is the iterator object
type Mcmc5mchintmcMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Mcmc5mchintmcMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Mcmc5mchintmcMetricsIterator) Next() *Mcmc5mchintmcMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Mcmc5mchintmcMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Mcmc5mchintmcMetricsIterator) Find(key uint64) (*Mcmc5mchintmcMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Mcmc5mchintmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Mcmc5mchintmcMetricsIterator) Create(key uint64) (*Mcmc5mchintmcMetrics, error) {
	tmtr := &Mcmc5mchintmcMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Mcmc5mchintmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Mcmc5mchintmcMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Mcmc5mchintmcMetricsIterator) Free() {
	it.iter.Free()
}

// NewMcmc5mchintmcMetricsIterator returns an iterator
func NewMcmc5mchintmcMetricsIterator() (*Mcmc5mchintmcMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Mcmc5mchintmcMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Mcmc5mchintmcMetricsIterator{iter: iter}, nil
}

type Mcmc6intmcMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MchInt metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Mcmc6intmcMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Mcmc6intmcMetrics) Size() int {
	sz := 0

	sz += mtr.MchInt.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Mcmc6intmcMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MchInt = mtr.metrics.GetCounter(offset)
	offset += mtr.MchInt.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Mcmc6intmcMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MchInt" {
		return offset
	}
	offset += mtr.MchInt.Size()

	return offset
}

// SetMchInt sets cunter in shared memory
func (mtr *Mcmc6intmcMetrics) SetMchInt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MchInt"))
	return nil
}

// Mcmc6intmcMetricsIterator is the iterator object
type Mcmc6intmcMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Mcmc6intmcMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Mcmc6intmcMetricsIterator) Next() *Mcmc6intmcMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Mcmc6intmcMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Mcmc6intmcMetricsIterator) Find(key uint64) (*Mcmc6intmcMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Mcmc6intmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Mcmc6intmcMetricsIterator) Create(key uint64) (*Mcmc6intmcMetrics, error) {
	tmtr := &Mcmc6intmcMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Mcmc6intmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Mcmc6intmcMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Mcmc6intmcMetricsIterator) Free() {
	it.iter.Free()
}

// NewMcmc6intmcMetricsIterator returns an iterator
func NewMcmc6intmcMetricsIterator() (*Mcmc6intmcMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Mcmc6intmcMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Mcmc6intmcMetricsIterator{iter: iter}, nil
}

type Mcmc6mchintmcMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Ecc_1BitThreshPs1 metrics.Counter

	Ecc_1BitThreshPs0 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Mcmc6mchintmcMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Mcmc6mchintmcMetrics) Size() int {
	sz := 0

	sz += mtr.Ecc_1BitThreshPs1.Size()

	sz += mtr.Ecc_1BitThreshPs0.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Mcmc6mchintmcMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Ecc_1BitThreshPs1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Ecc_1BitThreshPs1.Size()

	mtr.Ecc_1BitThreshPs0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Ecc_1BitThreshPs0.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Mcmc6mchintmcMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Ecc_1BitThreshPs1" {
		return offset
	}
	offset += mtr.Ecc_1BitThreshPs1.Size()

	if fldName == "Ecc_1BitThreshPs0" {
		return offset
	}
	offset += mtr.Ecc_1BitThreshPs0.Size()

	return offset
}

// SetEcc_1BitThreshPs1 sets cunter in shared memory
func (mtr *Mcmc6mchintmcMetrics) SetEcc_1BitThreshPs1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ecc_1BitThreshPs1"))
	return nil
}

// SetEcc_1BitThreshPs0 sets cunter in shared memory
func (mtr *Mcmc6mchintmcMetrics) SetEcc_1BitThreshPs0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ecc_1BitThreshPs0"))
	return nil
}

// Mcmc6mchintmcMetricsIterator is the iterator object
type Mcmc6mchintmcMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Mcmc6mchintmcMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Mcmc6mchintmcMetricsIterator) Next() *Mcmc6mchintmcMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Mcmc6mchintmcMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Mcmc6mchintmcMetricsIterator) Find(key uint64) (*Mcmc6mchintmcMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Mcmc6mchintmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Mcmc6mchintmcMetricsIterator) Create(key uint64) (*Mcmc6mchintmcMetrics, error) {
	tmtr := &Mcmc6mchintmcMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Mcmc6mchintmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Mcmc6mchintmcMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Mcmc6mchintmcMetricsIterator) Free() {
	it.iter.Free()
}

// NewMcmc6mchintmcMetricsIterator returns an iterator
func NewMcmc6mchintmcMetricsIterator() (*Mcmc6mchintmcMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Mcmc6mchintmcMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Mcmc6mchintmcMetricsIterator{iter: iter}, nil
}

type Mcmc7intmcMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MchInt metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Mcmc7intmcMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Mcmc7intmcMetrics) Size() int {
	sz := 0

	sz += mtr.MchInt.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Mcmc7intmcMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MchInt = mtr.metrics.GetCounter(offset)
	offset += mtr.MchInt.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Mcmc7intmcMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MchInt" {
		return offset
	}
	offset += mtr.MchInt.Size()

	return offset
}

// SetMchInt sets cunter in shared memory
func (mtr *Mcmc7intmcMetrics) SetMchInt(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MchInt"))
	return nil
}

// Mcmc7intmcMetricsIterator is the iterator object
type Mcmc7intmcMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Mcmc7intmcMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Mcmc7intmcMetricsIterator) Next() *Mcmc7intmcMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Mcmc7intmcMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Mcmc7intmcMetricsIterator) Find(key uint64) (*Mcmc7intmcMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Mcmc7intmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Mcmc7intmcMetricsIterator) Create(key uint64) (*Mcmc7intmcMetrics, error) {
	tmtr := &Mcmc7intmcMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Mcmc7intmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Mcmc7intmcMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Mcmc7intmcMetricsIterator) Free() {
	it.iter.Free()
}

// NewMcmc7intmcMetricsIterator returns an iterator
func NewMcmc7intmcMetricsIterator() (*Mcmc7intmcMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Mcmc7intmcMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Mcmc7intmcMetricsIterator{iter: iter}, nil
}

type Mcmc7mchintmcMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Ecc_1BitThreshPs1 metrics.Counter

	Ecc_1BitThreshPs0 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Mcmc7mchintmcMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Mcmc7mchintmcMetrics) Size() int {
	sz := 0

	sz += mtr.Ecc_1BitThreshPs1.Size()

	sz += mtr.Ecc_1BitThreshPs0.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Mcmc7mchintmcMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Ecc_1BitThreshPs1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Ecc_1BitThreshPs1.Size()

	mtr.Ecc_1BitThreshPs0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Ecc_1BitThreshPs0.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Mcmc7mchintmcMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Ecc_1BitThreshPs1" {
		return offset
	}
	offset += mtr.Ecc_1BitThreshPs1.Size()

	if fldName == "Ecc_1BitThreshPs0" {
		return offset
	}
	offset += mtr.Ecc_1BitThreshPs0.Size()

	return offset
}

// SetEcc_1BitThreshPs1 sets cunter in shared memory
func (mtr *Mcmc7mchintmcMetrics) SetEcc_1BitThreshPs1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ecc_1BitThreshPs1"))
	return nil
}

// SetEcc_1BitThreshPs0 sets cunter in shared memory
func (mtr *Mcmc7mchintmcMetrics) SetEcc_1BitThreshPs0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ecc_1BitThreshPs0"))
	return nil
}

// Mcmc7mchintmcMetricsIterator is the iterator object
type Mcmc7mchintmcMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Mcmc7mchintmcMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Mcmc7mchintmcMetricsIterator) Next() *Mcmc7mchintmcMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Mcmc7mchintmcMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Mcmc7mchintmcMetricsIterator) Find(key uint64) (*Mcmc7mchintmcMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Mcmc7mchintmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Mcmc7mchintmcMetricsIterator) Create(key uint64) (*Mcmc7mchintmcMetrics, error) {
	tmtr := &Mcmc7mchintmcMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Mcmc7mchintmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Mcmc7mchintmcMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Mcmc7mchintmcMetricsIterator) Free() {
	it.iter.Free()
}

// NewMcmc7mchintmcMetricsIterator returns an iterator
func NewMcmc7mchintmcMetricsIterator() (*Mcmc7mchintmcMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Mcmc7mchintmcMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Mcmc7mchintmcMetricsIterator{iter: iter}, nil
}

type Dppdpp0intreg1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	ErrPhvSopNoEop metrics.Counter

	ErrPhvEopNoSop metrics.Counter

	ErrOhiSopNoEop metrics.Counter

	ErrOhiEopNoSop metrics.Counter

	ErrFramerCreditOverrun metrics.Counter

	ErrPacketsInFlightCreditOverrun metrics.Counter

	ErrNullHdrVld metrics.Counter

	ErrNullHdrfldVld metrics.Counter

	ErrMaxPktSize metrics.Counter

	ErrMaxActiveHdrs metrics.Counter

	ErrPhvNoDataReferenceOhi metrics.Counter

	ErrCsumMultipleHdr metrics.Counter

	ErrCsumMultipleHdrCopy metrics.Counter

	ErrCrcMultipleHdr metrics.Counter

	ErrPtrFifoCreditOverrun metrics.Counter

	ErrClipMaxPktSize metrics.Counter

	ErrMinPktSize metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dppdpp0intreg1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dppdpp0intreg1Metrics) Size() int {
	sz := 0

	sz += mtr.ErrPhvSopNoEop.Size()

	sz += mtr.ErrPhvEopNoSop.Size()

	sz += mtr.ErrOhiSopNoEop.Size()

	sz += mtr.ErrOhiEopNoSop.Size()

	sz += mtr.ErrFramerCreditOverrun.Size()

	sz += mtr.ErrPacketsInFlightCreditOverrun.Size()

	sz += mtr.ErrNullHdrVld.Size()

	sz += mtr.ErrNullHdrfldVld.Size()

	sz += mtr.ErrMaxPktSize.Size()

	sz += mtr.ErrMaxActiveHdrs.Size()

	sz += mtr.ErrPhvNoDataReferenceOhi.Size()

	sz += mtr.ErrCsumMultipleHdr.Size()

	sz += mtr.ErrCsumMultipleHdrCopy.Size()

	sz += mtr.ErrCrcMultipleHdr.Size()

	sz += mtr.ErrPtrFifoCreditOverrun.Size()

	sz += mtr.ErrClipMaxPktSize.Size()

	sz += mtr.ErrMinPktSize.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dppdpp0intreg1Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.ErrPhvSopNoEop = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPhvSopNoEop.Size()

	mtr.ErrPhvEopNoSop = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPhvEopNoSop.Size()

	mtr.ErrOhiSopNoEop = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrOhiSopNoEop.Size()

	mtr.ErrOhiEopNoSop = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrOhiEopNoSop.Size()

	mtr.ErrFramerCreditOverrun = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrFramerCreditOverrun.Size()

	mtr.ErrPacketsInFlightCreditOverrun = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPacketsInFlightCreditOverrun.Size()

	mtr.ErrNullHdrVld = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrNullHdrVld.Size()

	mtr.ErrNullHdrfldVld = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrNullHdrfldVld.Size()

	mtr.ErrMaxPktSize = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrMaxPktSize.Size()

	mtr.ErrMaxActiveHdrs = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrMaxActiveHdrs.Size()

	mtr.ErrPhvNoDataReferenceOhi = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPhvNoDataReferenceOhi.Size()

	mtr.ErrCsumMultipleHdr = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumMultipleHdr.Size()

	mtr.ErrCsumMultipleHdrCopy = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumMultipleHdrCopy.Size()

	mtr.ErrCrcMultipleHdr = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCrcMultipleHdr.Size()

	mtr.ErrPtrFifoCreditOverrun = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPtrFifoCreditOverrun.Size()

	mtr.ErrClipMaxPktSize = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrClipMaxPktSize.Size()

	mtr.ErrMinPktSize = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrMinPktSize.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dppdpp0intreg1Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "ErrPhvSopNoEop" {
		return offset
	}
	offset += mtr.ErrPhvSopNoEop.Size()

	if fldName == "ErrPhvEopNoSop" {
		return offset
	}
	offset += mtr.ErrPhvEopNoSop.Size()

	if fldName == "ErrOhiSopNoEop" {
		return offset
	}
	offset += mtr.ErrOhiSopNoEop.Size()

	if fldName == "ErrOhiEopNoSop" {
		return offset
	}
	offset += mtr.ErrOhiEopNoSop.Size()

	if fldName == "ErrFramerCreditOverrun" {
		return offset
	}
	offset += mtr.ErrFramerCreditOverrun.Size()

	if fldName == "ErrPacketsInFlightCreditOverrun" {
		return offset
	}
	offset += mtr.ErrPacketsInFlightCreditOverrun.Size()

	if fldName == "ErrNullHdrVld" {
		return offset
	}
	offset += mtr.ErrNullHdrVld.Size()

	if fldName == "ErrNullHdrfldVld" {
		return offset
	}
	offset += mtr.ErrNullHdrfldVld.Size()

	if fldName == "ErrMaxPktSize" {
		return offset
	}
	offset += mtr.ErrMaxPktSize.Size()

	if fldName == "ErrMaxActiveHdrs" {
		return offset
	}
	offset += mtr.ErrMaxActiveHdrs.Size()

	if fldName == "ErrPhvNoDataReferenceOhi" {
		return offset
	}
	offset += mtr.ErrPhvNoDataReferenceOhi.Size()

	if fldName == "ErrCsumMultipleHdr" {
		return offset
	}
	offset += mtr.ErrCsumMultipleHdr.Size()

	if fldName == "ErrCsumMultipleHdrCopy" {
		return offset
	}
	offset += mtr.ErrCsumMultipleHdrCopy.Size()

	if fldName == "ErrCrcMultipleHdr" {
		return offset
	}
	offset += mtr.ErrCrcMultipleHdr.Size()

	if fldName == "ErrPtrFifoCreditOverrun" {
		return offset
	}
	offset += mtr.ErrPtrFifoCreditOverrun.Size()

	if fldName == "ErrClipMaxPktSize" {
		return offset
	}
	offset += mtr.ErrClipMaxPktSize.Size()

	if fldName == "ErrMinPktSize" {
		return offset
	}
	offset += mtr.ErrMinPktSize.Size()

	return offset
}

// SetErrPhvSopNoEop sets cunter in shared memory
func (mtr *Dppdpp0intreg1Metrics) SetErrPhvSopNoEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPhvSopNoEop"))
	return nil
}

// SetErrPhvEopNoSop sets cunter in shared memory
func (mtr *Dppdpp0intreg1Metrics) SetErrPhvEopNoSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPhvEopNoSop"))
	return nil
}

// SetErrOhiSopNoEop sets cunter in shared memory
func (mtr *Dppdpp0intreg1Metrics) SetErrOhiSopNoEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrOhiSopNoEop"))
	return nil
}

// SetErrOhiEopNoSop sets cunter in shared memory
func (mtr *Dppdpp0intreg1Metrics) SetErrOhiEopNoSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrOhiEopNoSop"))
	return nil
}

// SetErrFramerCreditOverrun sets cunter in shared memory
func (mtr *Dppdpp0intreg1Metrics) SetErrFramerCreditOverrun(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrFramerCreditOverrun"))
	return nil
}

// SetErrPacketsInFlightCreditOverrun sets cunter in shared memory
func (mtr *Dppdpp0intreg1Metrics) SetErrPacketsInFlightCreditOverrun(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPacketsInFlightCreditOverrun"))
	return nil
}

// SetErrNullHdrVld sets cunter in shared memory
func (mtr *Dppdpp0intreg1Metrics) SetErrNullHdrVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrNullHdrVld"))
	return nil
}

// SetErrNullHdrfldVld sets cunter in shared memory
func (mtr *Dppdpp0intreg1Metrics) SetErrNullHdrfldVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrNullHdrfldVld"))
	return nil
}

// SetErrMaxPktSize sets cunter in shared memory
func (mtr *Dppdpp0intreg1Metrics) SetErrMaxPktSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrMaxPktSize"))
	return nil
}

// SetErrMaxActiveHdrs sets cunter in shared memory
func (mtr *Dppdpp0intreg1Metrics) SetErrMaxActiveHdrs(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrMaxActiveHdrs"))
	return nil
}

// SetErrPhvNoDataReferenceOhi sets cunter in shared memory
func (mtr *Dppdpp0intreg1Metrics) SetErrPhvNoDataReferenceOhi(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPhvNoDataReferenceOhi"))
	return nil
}

// SetErrCsumMultipleHdr sets cunter in shared memory
func (mtr *Dppdpp0intreg1Metrics) SetErrCsumMultipleHdr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumMultipleHdr"))
	return nil
}

// SetErrCsumMultipleHdrCopy sets cunter in shared memory
func (mtr *Dppdpp0intreg1Metrics) SetErrCsumMultipleHdrCopy(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumMultipleHdrCopy"))
	return nil
}

// SetErrCrcMultipleHdr sets cunter in shared memory
func (mtr *Dppdpp0intreg1Metrics) SetErrCrcMultipleHdr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCrcMultipleHdr"))
	return nil
}

// SetErrPtrFifoCreditOverrun sets cunter in shared memory
func (mtr *Dppdpp0intreg1Metrics) SetErrPtrFifoCreditOverrun(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPtrFifoCreditOverrun"))
	return nil
}

// SetErrClipMaxPktSize sets cunter in shared memory
func (mtr *Dppdpp0intreg1Metrics) SetErrClipMaxPktSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrClipMaxPktSize"))
	return nil
}

// SetErrMinPktSize sets cunter in shared memory
func (mtr *Dppdpp0intreg1Metrics) SetErrMinPktSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrMinPktSize"))
	return nil
}

// Dppdpp0intreg1MetricsIterator is the iterator object
type Dppdpp0intreg1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dppdpp0intreg1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dppdpp0intreg1MetricsIterator) Next() *Dppdpp0intreg1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dppdpp0intreg1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dppdpp0intreg1MetricsIterator) Find(key uint64) (*Dppdpp0intreg1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dppdpp0intreg1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dppdpp0intreg1MetricsIterator) Create(key uint64) (*Dppdpp0intreg1Metrics, error) {
	tmtr := &Dppdpp0intreg1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dppdpp0intreg1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dppdpp0intreg1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dppdpp0intreg1MetricsIterator) Free() {
	it.iter.Free()
}

// NewDppdpp0intreg1MetricsIterator returns an iterator
func NewDppdpp0intreg1MetricsIterator() (*Dppdpp0intreg1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dppdpp0intreg1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dppdpp0intreg1MetricsIterator{iter: iter}, nil
}

type Dppdpp0intsramseccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	DppPhvFifoUncorrectable metrics.Counter

	DppPhvFifoCorrectable metrics.Counter

	DppOhiFifoUncorrectable metrics.Counter

	DppOhiFifoCorrectable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dppdpp0intsramseccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dppdpp0intsramseccMetrics) Size() int {
	sz := 0

	sz += mtr.DppPhvFifoUncorrectable.Size()

	sz += mtr.DppPhvFifoCorrectable.Size()

	sz += mtr.DppOhiFifoUncorrectable.Size()

	sz += mtr.DppOhiFifoCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dppdpp0intsramseccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.DppPhvFifoUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.DppPhvFifoUncorrectable.Size()

	mtr.DppPhvFifoCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.DppPhvFifoCorrectable.Size()

	mtr.DppOhiFifoUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.DppOhiFifoUncorrectable.Size()

	mtr.DppOhiFifoCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.DppOhiFifoCorrectable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dppdpp0intsramseccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "DppPhvFifoUncorrectable" {
		return offset
	}
	offset += mtr.DppPhvFifoUncorrectable.Size()

	if fldName == "DppPhvFifoCorrectable" {
		return offset
	}
	offset += mtr.DppPhvFifoCorrectable.Size()

	if fldName == "DppOhiFifoUncorrectable" {
		return offset
	}
	offset += mtr.DppOhiFifoUncorrectable.Size()

	if fldName == "DppOhiFifoCorrectable" {
		return offset
	}
	offset += mtr.DppOhiFifoCorrectable.Size()

	return offset
}

// SetDppPhvFifoUncorrectable sets cunter in shared memory
func (mtr *Dppdpp0intsramseccMetrics) SetDppPhvFifoUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DppPhvFifoUncorrectable"))
	return nil
}

// SetDppPhvFifoCorrectable sets cunter in shared memory
func (mtr *Dppdpp0intsramseccMetrics) SetDppPhvFifoCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DppPhvFifoCorrectable"))
	return nil
}

// SetDppOhiFifoUncorrectable sets cunter in shared memory
func (mtr *Dppdpp0intsramseccMetrics) SetDppOhiFifoUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DppOhiFifoUncorrectable"))
	return nil
}

// SetDppOhiFifoCorrectable sets cunter in shared memory
func (mtr *Dppdpp0intsramseccMetrics) SetDppOhiFifoCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DppOhiFifoCorrectable"))
	return nil
}

// Dppdpp0intsramseccMetricsIterator is the iterator object
type Dppdpp0intsramseccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dppdpp0intsramseccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dppdpp0intsramseccMetricsIterator) Next() *Dppdpp0intsramseccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dppdpp0intsramseccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dppdpp0intsramseccMetricsIterator) Find(key uint64) (*Dppdpp0intsramseccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dppdpp0intsramseccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dppdpp0intsramseccMetricsIterator) Create(key uint64) (*Dppdpp0intsramseccMetrics, error) {
	tmtr := &Dppdpp0intsramseccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dppdpp0intsramseccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dppdpp0intsramseccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dppdpp0intsramseccMetricsIterator) Free() {
	it.iter.Free()
}

// NewDppdpp0intsramseccMetricsIterator returns an iterator
func NewDppdpp0intsramseccMetricsIterator() (*Dppdpp0intsramseccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dppdpp0intsramseccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dppdpp0intsramseccMetricsIterator{iter: iter}, nil
}

type Dppdpp0intfifoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PhvFfOverflow metrics.Counter

	OhiFfOverflow metrics.Counter

	PktSizeFfOvflow metrics.Counter

	PktSizeFfUndflow metrics.Counter

	CsumPhvFfOvflow metrics.Counter

	CsumPhvFfUndflow metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dppdpp0intfifoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dppdpp0intfifoMetrics) Size() int {
	sz := 0

	sz += mtr.PhvFfOverflow.Size()

	sz += mtr.OhiFfOverflow.Size()

	sz += mtr.PktSizeFfOvflow.Size()

	sz += mtr.PktSizeFfUndflow.Size()

	sz += mtr.CsumPhvFfOvflow.Size()

	sz += mtr.CsumPhvFfUndflow.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dppdpp0intfifoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.PhvFfOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvFfOverflow.Size()

	mtr.OhiFfOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.OhiFfOverflow.Size()

	mtr.PktSizeFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeFfOvflow.Size()

	mtr.PktSizeFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeFfUndflow.Size()

	mtr.CsumPhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CsumPhvFfOvflow.Size()

	mtr.CsumPhvFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CsumPhvFfUndflow.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dppdpp0intfifoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "PhvFfOverflow" {
		return offset
	}
	offset += mtr.PhvFfOverflow.Size()

	if fldName == "OhiFfOverflow" {
		return offset
	}
	offset += mtr.OhiFfOverflow.Size()

	if fldName == "PktSizeFfOvflow" {
		return offset
	}
	offset += mtr.PktSizeFfOvflow.Size()

	if fldName == "PktSizeFfUndflow" {
		return offset
	}
	offset += mtr.PktSizeFfUndflow.Size()

	if fldName == "CsumPhvFfOvflow" {
		return offset
	}
	offset += mtr.CsumPhvFfOvflow.Size()

	if fldName == "CsumPhvFfUndflow" {
		return offset
	}
	offset += mtr.CsumPhvFfUndflow.Size()

	return offset
}

// SetPhvFfOverflow sets cunter in shared memory
func (mtr *Dppdpp0intfifoMetrics) SetPhvFfOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvFfOverflow"))
	return nil
}

// SetOhiFfOverflow sets cunter in shared memory
func (mtr *Dppdpp0intfifoMetrics) SetOhiFfOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OhiFfOverflow"))
	return nil
}

// SetPktSizeFfOvflow sets cunter in shared memory
func (mtr *Dppdpp0intfifoMetrics) SetPktSizeFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeFfOvflow"))
	return nil
}

// SetPktSizeFfUndflow sets cunter in shared memory
func (mtr *Dppdpp0intfifoMetrics) SetPktSizeFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeFfUndflow"))
	return nil
}

// SetCsumPhvFfOvflow sets cunter in shared memory
func (mtr *Dppdpp0intfifoMetrics) SetCsumPhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CsumPhvFfOvflow"))
	return nil
}

// SetCsumPhvFfUndflow sets cunter in shared memory
func (mtr *Dppdpp0intfifoMetrics) SetCsumPhvFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CsumPhvFfUndflow"))
	return nil
}

// Dppdpp0intfifoMetricsIterator is the iterator object
type Dppdpp0intfifoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dppdpp0intfifoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dppdpp0intfifoMetricsIterator) Next() *Dppdpp0intfifoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dppdpp0intfifoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dppdpp0intfifoMetricsIterator) Find(key uint64) (*Dppdpp0intfifoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dppdpp0intfifoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dppdpp0intfifoMetricsIterator) Create(key uint64) (*Dppdpp0intfifoMetrics, error) {
	tmtr := &Dppdpp0intfifoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dppdpp0intfifoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dppdpp0intfifoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dppdpp0intfifoMetricsIterator) Free() {
	it.iter.Free()
}

// NewDppdpp0intfifoMetricsIterator returns an iterator
func NewDppdpp0intfifoMetricsIterator() (*Dppdpp0intfifoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dppdpp0intfifoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dppdpp0intfifoMetricsIterator{iter: iter}, nil
}

type Dppdpp0intspareMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Spare_0 metrics.Counter

	Spare_1 metrics.Counter

	Spare_2 metrics.Counter

	Spare_3 metrics.Counter

	Spare_4 metrics.Counter

	Spare_5 metrics.Counter

	Spare_6 metrics.Counter

	Spare_7 metrics.Counter

	Spare_8 metrics.Counter

	Spare_9 metrics.Counter

	Spare_10 metrics.Counter

	Spare_11 metrics.Counter

	Spare_12 metrics.Counter

	Spare_13 metrics.Counter

	Spare_14 metrics.Counter

	Spare_15 metrics.Counter

	Spare_16 metrics.Counter

	Spare_17 metrics.Counter

	Spare_18 metrics.Counter

	Spare_19 metrics.Counter

	Spare_20 metrics.Counter

	Spare_21 metrics.Counter

	Spare_22 metrics.Counter

	Spare_23 metrics.Counter

	Spare_24 metrics.Counter

	Spare_25 metrics.Counter

	Spare_26 metrics.Counter

	Spare_27 metrics.Counter

	Spare_28 metrics.Counter

	Spare_29 metrics.Counter

	Spare_30 metrics.Counter

	Spare_31 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dppdpp0intspareMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dppdpp0intspareMetrics) Size() int {
	sz := 0

	sz += mtr.Spare_0.Size()

	sz += mtr.Spare_1.Size()

	sz += mtr.Spare_2.Size()

	sz += mtr.Spare_3.Size()

	sz += mtr.Spare_4.Size()

	sz += mtr.Spare_5.Size()

	sz += mtr.Spare_6.Size()

	sz += mtr.Spare_7.Size()

	sz += mtr.Spare_8.Size()

	sz += mtr.Spare_9.Size()

	sz += mtr.Spare_10.Size()

	sz += mtr.Spare_11.Size()

	sz += mtr.Spare_12.Size()

	sz += mtr.Spare_13.Size()

	sz += mtr.Spare_14.Size()

	sz += mtr.Spare_15.Size()

	sz += mtr.Spare_16.Size()

	sz += mtr.Spare_17.Size()

	sz += mtr.Spare_18.Size()

	sz += mtr.Spare_19.Size()

	sz += mtr.Spare_20.Size()

	sz += mtr.Spare_21.Size()

	sz += mtr.Spare_22.Size()

	sz += mtr.Spare_23.Size()

	sz += mtr.Spare_24.Size()

	sz += mtr.Spare_25.Size()

	sz += mtr.Spare_26.Size()

	sz += mtr.Spare_27.Size()

	sz += mtr.Spare_28.Size()

	sz += mtr.Spare_29.Size()

	sz += mtr.Spare_30.Size()

	sz += mtr.Spare_31.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dppdpp0intspareMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Spare_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_0.Size()

	mtr.Spare_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_1.Size()

	mtr.Spare_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_2.Size()

	mtr.Spare_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_3.Size()

	mtr.Spare_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_4.Size()

	mtr.Spare_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_5.Size()

	mtr.Spare_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_6.Size()

	mtr.Spare_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_7.Size()

	mtr.Spare_8 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_8.Size()

	mtr.Spare_9 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_9.Size()

	mtr.Spare_10 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_10.Size()

	mtr.Spare_11 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_11.Size()

	mtr.Spare_12 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_12.Size()

	mtr.Spare_13 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_13.Size()

	mtr.Spare_14 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_14.Size()

	mtr.Spare_15 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_15.Size()

	mtr.Spare_16 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_16.Size()

	mtr.Spare_17 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_17.Size()

	mtr.Spare_18 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_18.Size()

	mtr.Spare_19 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_19.Size()

	mtr.Spare_20 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_20.Size()

	mtr.Spare_21 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_21.Size()

	mtr.Spare_22 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_22.Size()

	mtr.Spare_23 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_23.Size()

	mtr.Spare_24 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_24.Size()

	mtr.Spare_25 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_25.Size()

	mtr.Spare_26 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_26.Size()

	mtr.Spare_27 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_27.Size()

	mtr.Spare_28 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_28.Size()

	mtr.Spare_29 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_29.Size()

	mtr.Spare_30 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_30.Size()

	mtr.Spare_31 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_31.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dppdpp0intspareMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Spare_0" {
		return offset
	}
	offset += mtr.Spare_0.Size()

	if fldName == "Spare_1" {
		return offset
	}
	offset += mtr.Spare_1.Size()

	if fldName == "Spare_2" {
		return offset
	}
	offset += mtr.Spare_2.Size()

	if fldName == "Spare_3" {
		return offset
	}
	offset += mtr.Spare_3.Size()

	if fldName == "Spare_4" {
		return offset
	}
	offset += mtr.Spare_4.Size()

	if fldName == "Spare_5" {
		return offset
	}
	offset += mtr.Spare_5.Size()

	if fldName == "Spare_6" {
		return offset
	}
	offset += mtr.Spare_6.Size()

	if fldName == "Spare_7" {
		return offset
	}
	offset += mtr.Spare_7.Size()

	if fldName == "Spare_8" {
		return offset
	}
	offset += mtr.Spare_8.Size()

	if fldName == "Spare_9" {
		return offset
	}
	offset += mtr.Spare_9.Size()

	if fldName == "Spare_10" {
		return offset
	}
	offset += mtr.Spare_10.Size()

	if fldName == "Spare_11" {
		return offset
	}
	offset += mtr.Spare_11.Size()

	if fldName == "Spare_12" {
		return offset
	}
	offset += mtr.Spare_12.Size()

	if fldName == "Spare_13" {
		return offset
	}
	offset += mtr.Spare_13.Size()

	if fldName == "Spare_14" {
		return offset
	}
	offset += mtr.Spare_14.Size()

	if fldName == "Spare_15" {
		return offset
	}
	offset += mtr.Spare_15.Size()

	if fldName == "Spare_16" {
		return offset
	}
	offset += mtr.Spare_16.Size()

	if fldName == "Spare_17" {
		return offset
	}
	offset += mtr.Spare_17.Size()

	if fldName == "Spare_18" {
		return offset
	}
	offset += mtr.Spare_18.Size()

	if fldName == "Spare_19" {
		return offset
	}
	offset += mtr.Spare_19.Size()

	if fldName == "Spare_20" {
		return offset
	}
	offset += mtr.Spare_20.Size()

	if fldName == "Spare_21" {
		return offset
	}
	offset += mtr.Spare_21.Size()

	if fldName == "Spare_22" {
		return offset
	}
	offset += mtr.Spare_22.Size()

	if fldName == "Spare_23" {
		return offset
	}
	offset += mtr.Spare_23.Size()

	if fldName == "Spare_24" {
		return offset
	}
	offset += mtr.Spare_24.Size()

	if fldName == "Spare_25" {
		return offset
	}
	offset += mtr.Spare_25.Size()

	if fldName == "Spare_26" {
		return offset
	}
	offset += mtr.Spare_26.Size()

	if fldName == "Spare_27" {
		return offset
	}
	offset += mtr.Spare_27.Size()

	if fldName == "Spare_28" {
		return offset
	}
	offset += mtr.Spare_28.Size()

	if fldName == "Spare_29" {
		return offset
	}
	offset += mtr.Spare_29.Size()

	if fldName == "Spare_30" {
		return offset
	}
	offset += mtr.Spare_30.Size()

	if fldName == "Spare_31" {
		return offset
	}
	offset += mtr.Spare_31.Size()

	return offset
}

// SetSpare_0 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_0"))
	return nil
}

// SetSpare_1 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_1"))
	return nil
}

// SetSpare_2 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_2"))
	return nil
}

// SetSpare_3 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_3"))
	return nil
}

// SetSpare_4 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_4"))
	return nil
}

// SetSpare_5 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_5"))
	return nil
}

// SetSpare_6 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_6"))
	return nil
}

// SetSpare_7 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_7"))
	return nil
}

// SetSpare_8 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_8"))
	return nil
}

// SetSpare_9 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_9(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_9"))
	return nil
}

// SetSpare_10 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_10(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_10"))
	return nil
}

// SetSpare_11 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_11(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_11"))
	return nil
}

// SetSpare_12 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_12(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_12"))
	return nil
}

// SetSpare_13 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_13(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_13"))
	return nil
}

// SetSpare_14 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_14(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_14"))
	return nil
}

// SetSpare_15 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_15(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_15"))
	return nil
}

// SetSpare_16 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_16(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_16"))
	return nil
}

// SetSpare_17 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_17(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_17"))
	return nil
}

// SetSpare_18 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_18(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_18"))
	return nil
}

// SetSpare_19 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_19(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_19"))
	return nil
}

// SetSpare_20 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_20(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_20"))
	return nil
}

// SetSpare_21 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_21(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_21"))
	return nil
}

// SetSpare_22 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_22(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_22"))
	return nil
}

// SetSpare_23 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_23(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_23"))
	return nil
}

// SetSpare_24 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_24(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_24"))
	return nil
}

// SetSpare_25 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_25(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_25"))
	return nil
}

// SetSpare_26 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_26(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_26"))
	return nil
}

// SetSpare_27 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_27(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_27"))
	return nil
}

// SetSpare_28 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_28(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_28"))
	return nil
}

// SetSpare_29 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_29(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_29"))
	return nil
}

// SetSpare_30 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_30(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_30"))
	return nil
}

// SetSpare_31 sets cunter in shared memory
func (mtr *Dppdpp0intspareMetrics) SetSpare_31(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_31"))
	return nil
}

// Dppdpp0intspareMetricsIterator is the iterator object
type Dppdpp0intspareMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dppdpp0intspareMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dppdpp0intspareMetricsIterator) Next() *Dppdpp0intspareMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dppdpp0intspareMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dppdpp0intspareMetricsIterator) Find(key uint64) (*Dppdpp0intspareMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dppdpp0intspareMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dppdpp0intspareMetricsIterator) Create(key uint64) (*Dppdpp0intspareMetrics, error) {
	tmtr := &Dppdpp0intspareMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dppdpp0intspareMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dppdpp0intspareMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dppdpp0intspareMetricsIterator) Free() {
	it.iter.Free()
}

// NewDppdpp0intspareMetricsIterator returns an iterator
func NewDppdpp0intspareMetricsIterator() (*Dppdpp0intspareMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dppdpp0intspareMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dppdpp0intspareMetricsIterator{iter: iter}, nil
}

type Dppdpp0intcreditMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PtrCreditOvflow metrics.Counter

	PtrCreditUndflow metrics.Counter

	PktCreditOvflow metrics.Counter

	PktCreditUndflow metrics.Counter

	FramerCreditOvflow metrics.Counter

	FramerCreditUndflow metrics.Counter

	FramerHdrfldVldOvfl metrics.Counter

	FramerHdrfldOffsetOvfl metrics.Counter

	ErrFramerHdrsizeZeroOvfl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dppdpp0intcreditMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dppdpp0intcreditMetrics) Size() int {
	sz := 0

	sz += mtr.PtrCreditOvflow.Size()

	sz += mtr.PtrCreditUndflow.Size()

	sz += mtr.PktCreditOvflow.Size()

	sz += mtr.PktCreditUndflow.Size()

	sz += mtr.FramerCreditOvflow.Size()

	sz += mtr.FramerCreditUndflow.Size()

	sz += mtr.FramerHdrfldVldOvfl.Size()

	sz += mtr.FramerHdrfldOffsetOvfl.Size()

	sz += mtr.ErrFramerHdrsizeZeroOvfl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dppdpp0intcreditMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.PtrCreditOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PtrCreditOvflow.Size()

	mtr.PtrCreditUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PtrCreditUndflow.Size()

	mtr.PktCreditOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktCreditOvflow.Size()

	mtr.PktCreditUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktCreditUndflow.Size()

	mtr.FramerCreditOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.FramerCreditOvflow.Size()

	mtr.FramerCreditUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.FramerCreditUndflow.Size()

	mtr.FramerHdrfldVldOvfl = mtr.metrics.GetCounter(offset)
	offset += mtr.FramerHdrfldVldOvfl.Size()

	mtr.FramerHdrfldOffsetOvfl = mtr.metrics.GetCounter(offset)
	offset += mtr.FramerHdrfldOffsetOvfl.Size()

	mtr.ErrFramerHdrsizeZeroOvfl = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrFramerHdrsizeZeroOvfl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dppdpp0intcreditMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "PtrCreditOvflow" {
		return offset
	}
	offset += mtr.PtrCreditOvflow.Size()

	if fldName == "PtrCreditUndflow" {
		return offset
	}
	offset += mtr.PtrCreditUndflow.Size()

	if fldName == "PktCreditOvflow" {
		return offset
	}
	offset += mtr.PktCreditOvflow.Size()

	if fldName == "PktCreditUndflow" {
		return offset
	}
	offset += mtr.PktCreditUndflow.Size()

	if fldName == "FramerCreditOvflow" {
		return offset
	}
	offset += mtr.FramerCreditOvflow.Size()

	if fldName == "FramerCreditUndflow" {
		return offset
	}
	offset += mtr.FramerCreditUndflow.Size()

	if fldName == "FramerHdrfldVldOvfl" {
		return offset
	}
	offset += mtr.FramerHdrfldVldOvfl.Size()

	if fldName == "FramerHdrfldOffsetOvfl" {
		return offset
	}
	offset += mtr.FramerHdrfldOffsetOvfl.Size()

	if fldName == "ErrFramerHdrsizeZeroOvfl" {
		return offset
	}
	offset += mtr.ErrFramerHdrsizeZeroOvfl.Size()

	return offset
}

// SetPtrCreditOvflow sets cunter in shared memory
func (mtr *Dppdpp0intcreditMetrics) SetPtrCreditOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PtrCreditOvflow"))
	return nil
}

// SetPtrCreditUndflow sets cunter in shared memory
func (mtr *Dppdpp0intcreditMetrics) SetPtrCreditUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PtrCreditUndflow"))
	return nil
}

// SetPktCreditOvflow sets cunter in shared memory
func (mtr *Dppdpp0intcreditMetrics) SetPktCreditOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktCreditOvflow"))
	return nil
}

// SetPktCreditUndflow sets cunter in shared memory
func (mtr *Dppdpp0intcreditMetrics) SetPktCreditUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktCreditUndflow"))
	return nil
}

// SetFramerCreditOvflow sets cunter in shared memory
func (mtr *Dppdpp0intcreditMetrics) SetFramerCreditOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramerCreditOvflow"))
	return nil
}

// SetFramerCreditUndflow sets cunter in shared memory
func (mtr *Dppdpp0intcreditMetrics) SetFramerCreditUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramerCreditUndflow"))
	return nil
}

// SetFramerHdrfldVldOvfl sets cunter in shared memory
func (mtr *Dppdpp0intcreditMetrics) SetFramerHdrfldVldOvfl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramerHdrfldVldOvfl"))
	return nil
}

// SetFramerHdrfldOffsetOvfl sets cunter in shared memory
func (mtr *Dppdpp0intcreditMetrics) SetFramerHdrfldOffsetOvfl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramerHdrfldOffsetOvfl"))
	return nil
}

// SetErrFramerHdrsizeZeroOvfl sets cunter in shared memory
func (mtr *Dppdpp0intcreditMetrics) SetErrFramerHdrsizeZeroOvfl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrFramerHdrsizeZeroOvfl"))
	return nil
}

// Dppdpp0intcreditMetricsIterator is the iterator object
type Dppdpp0intcreditMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dppdpp0intcreditMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dppdpp0intcreditMetricsIterator) Next() *Dppdpp0intcreditMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dppdpp0intcreditMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dppdpp0intcreditMetricsIterator) Find(key uint64) (*Dppdpp0intcreditMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dppdpp0intcreditMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dppdpp0intcreditMetricsIterator) Create(key uint64) (*Dppdpp0intcreditMetrics, error) {
	tmtr := &Dppdpp0intcreditMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dppdpp0intcreditMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dppdpp0intcreditMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dppdpp0intcreditMetricsIterator) Free() {
	it.iter.Free()
}

// NewDppdpp0intcreditMetricsIterator returns an iterator
func NewDppdpp0intcreditMetricsIterator() (*Dppdpp0intcreditMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dppdpp0intcreditMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dppdpp0intcreditMetricsIterator{iter: iter}, nil
}

type Dppdpp1intreg1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	ErrPhvSopNoEop metrics.Counter

	ErrPhvEopNoSop metrics.Counter

	ErrOhiSopNoEop metrics.Counter

	ErrOhiEopNoSop metrics.Counter

	ErrFramerCreditOverrun metrics.Counter

	ErrPacketsInFlightCreditOverrun metrics.Counter

	ErrNullHdrVld metrics.Counter

	ErrNullHdrfldVld metrics.Counter

	ErrMaxPktSize metrics.Counter

	ErrMaxActiveHdrs metrics.Counter

	ErrPhvNoDataReferenceOhi metrics.Counter

	ErrCsumMultipleHdr metrics.Counter

	ErrCsumMultipleHdrCopy metrics.Counter

	ErrCrcMultipleHdr metrics.Counter

	ErrPtrFifoCreditOverrun metrics.Counter

	ErrClipMaxPktSize metrics.Counter

	ErrMinPktSize metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dppdpp1intreg1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dppdpp1intreg1Metrics) Size() int {
	sz := 0

	sz += mtr.ErrPhvSopNoEop.Size()

	sz += mtr.ErrPhvEopNoSop.Size()

	sz += mtr.ErrOhiSopNoEop.Size()

	sz += mtr.ErrOhiEopNoSop.Size()

	sz += mtr.ErrFramerCreditOverrun.Size()

	sz += mtr.ErrPacketsInFlightCreditOverrun.Size()

	sz += mtr.ErrNullHdrVld.Size()

	sz += mtr.ErrNullHdrfldVld.Size()

	sz += mtr.ErrMaxPktSize.Size()

	sz += mtr.ErrMaxActiveHdrs.Size()

	sz += mtr.ErrPhvNoDataReferenceOhi.Size()

	sz += mtr.ErrCsumMultipleHdr.Size()

	sz += mtr.ErrCsumMultipleHdrCopy.Size()

	sz += mtr.ErrCrcMultipleHdr.Size()

	sz += mtr.ErrPtrFifoCreditOverrun.Size()

	sz += mtr.ErrClipMaxPktSize.Size()

	sz += mtr.ErrMinPktSize.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dppdpp1intreg1Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.ErrPhvSopNoEop = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPhvSopNoEop.Size()

	mtr.ErrPhvEopNoSop = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPhvEopNoSop.Size()

	mtr.ErrOhiSopNoEop = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrOhiSopNoEop.Size()

	mtr.ErrOhiEopNoSop = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrOhiEopNoSop.Size()

	mtr.ErrFramerCreditOverrun = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrFramerCreditOverrun.Size()

	mtr.ErrPacketsInFlightCreditOverrun = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPacketsInFlightCreditOverrun.Size()

	mtr.ErrNullHdrVld = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrNullHdrVld.Size()

	mtr.ErrNullHdrfldVld = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrNullHdrfldVld.Size()

	mtr.ErrMaxPktSize = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrMaxPktSize.Size()

	mtr.ErrMaxActiveHdrs = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrMaxActiveHdrs.Size()

	mtr.ErrPhvNoDataReferenceOhi = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPhvNoDataReferenceOhi.Size()

	mtr.ErrCsumMultipleHdr = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumMultipleHdr.Size()

	mtr.ErrCsumMultipleHdrCopy = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumMultipleHdrCopy.Size()

	mtr.ErrCrcMultipleHdr = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCrcMultipleHdr.Size()

	mtr.ErrPtrFifoCreditOverrun = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPtrFifoCreditOverrun.Size()

	mtr.ErrClipMaxPktSize = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrClipMaxPktSize.Size()

	mtr.ErrMinPktSize = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrMinPktSize.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dppdpp1intreg1Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "ErrPhvSopNoEop" {
		return offset
	}
	offset += mtr.ErrPhvSopNoEop.Size()

	if fldName == "ErrPhvEopNoSop" {
		return offset
	}
	offset += mtr.ErrPhvEopNoSop.Size()

	if fldName == "ErrOhiSopNoEop" {
		return offset
	}
	offset += mtr.ErrOhiSopNoEop.Size()

	if fldName == "ErrOhiEopNoSop" {
		return offset
	}
	offset += mtr.ErrOhiEopNoSop.Size()

	if fldName == "ErrFramerCreditOverrun" {
		return offset
	}
	offset += mtr.ErrFramerCreditOverrun.Size()

	if fldName == "ErrPacketsInFlightCreditOverrun" {
		return offset
	}
	offset += mtr.ErrPacketsInFlightCreditOverrun.Size()

	if fldName == "ErrNullHdrVld" {
		return offset
	}
	offset += mtr.ErrNullHdrVld.Size()

	if fldName == "ErrNullHdrfldVld" {
		return offset
	}
	offset += mtr.ErrNullHdrfldVld.Size()

	if fldName == "ErrMaxPktSize" {
		return offset
	}
	offset += mtr.ErrMaxPktSize.Size()

	if fldName == "ErrMaxActiveHdrs" {
		return offset
	}
	offset += mtr.ErrMaxActiveHdrs.Size()

	if fldName == "ErrPhvNoDataReferenceOhi" {
		return offset
	}
	offset += mtr.ErrPhvNoDataReferenceOhi.Size()

	if fldName == "ErrCsumMultipleHdr" {
		return offset
	}
	offset += mtr.ErrCsumMultipleHdr.Size()

	if fldName == "ErrCsumMultipleHdrCopy" {
		return offset
	}
	offset += mtr.ErrCsumMultipleHdrCopy.Size()

	if fldName == "ErrCrcMultipleHdr" {
		return offset
	}
	offset += mtr.ErrCrcMultipleHdr.Size()

	if fldName == "ErrPtrFifoCreditOverrun" {
		return offset
	}
	offset += mtr.ErrPtrFifoCreditOverrun.Size()

	if fldName == "ErrClipMaxPktSize" {
		return offset
	}
	offset += mtr.ErrClipMaxPktSize.Size()

	if fldName == "ErrMinPktSize" {
		return offset
	}
	offset += mtr.ErrMinPktSize.Size()

	return offset
}

// SetErrPhvSopNoEop sets cunter in shared memory
func (mtr *Dppdpp1intreg1Metrics) SetErrPhvSopNoEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPhvSopNoEop"))
	return nil
}

// SetErrPhvEopNoSop sets cunter in shared memory
func (mtr *Dppdpp1intreg1Metrics) SetErrPhvEopNoSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPhvEopNoSop"))
	return nil
}

// SetErrOhiSopNoEop sets cunter in shared memory
func (mtr *Dppdpp1intreg1Metrics) SetErrOhiSopNoEop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrOhiSopNoEop"))
	return nil
}

// SetErrOhiEopNoSop sets cunter in shared memory
func (mtr *Dppdpp1intreg1Metrics) SetErrOhiEopNoSop(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrOhiEopNoSop"))
	return nil
}

// SetErrFramerCreditOverrun sets cunter in shared memory
func (mtr *Dppdpp1intreg1Metrics) SetErrFramerCreditOverrun(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrFramerCreditOverrun"))
	return nil
}

// SetErrPacketsInFlightCreditOverrun sets cunter in shared memory
func (mtr *Dppdpp1intreg1Metrics) SetErrPacketsInFlightCreditOverrun(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPacketsInFlightCreditOverrun"))
	return nil
}

// SetErrNullHdrVld sets cunter in shared memory
func (mtr *Dppdpp1intreg1Metrics) SetErrNullHdrVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrNullHdrVld"))
	return nil
}

// SetErrNullHdrfldVld sets cunter in shared memory
func (mtr *Dppdpp1intreg1Metrics) SetErrNullHdrfldVld(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrNullHdrfldVld"))
	return nil
}

// SetErrMaxPktSize sets cunter in shared memory
func (mtr *Dppdpp1intreg1Metrics) SetErrMaxPktSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrMaxPktSize"))
	return nil
}

// SetErrMaxActiveHdrs sets cunter in shared memory
func (mtr *Dppdpp1intreg1Metrics) SetErrMaxActiveHdrs(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrMaxActiveHdrs"))
	return nil
}

// SetErrPhvNoDataReferenceOhi sets cunter in shared memory
func (mtr *Dppdpp1intreg1Metrics) SetErrPhvNoDataReferenceOhi(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPhvNoDataReferenceOhi"))
	return nil
}

// SetErrCsumMultipleHdr sets cunter in shared memory
func (mtr *Dppdpp1intreg1Metrics) SetErrCsumMultipleHdr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumMultipleHdr"))
	return nil
}

// SetErrCsumMultipleHdrCopy sets cunter in shared memory
func (mtr *Dppdpp1intreg1Metrics) SetErrCsumMultipleHdrCopy(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumMultipleHdrCopy"))
	return nil
}

// SetErrCrcMultipleHdr sets cunter in shared memory
func (mtr *Dppdpp1intreg1Metrics) SetErrCrcMultipleHdr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCrcMultipleHdr"))
	return nil
}

// SetErrPtrFifoCreditOverrun sets cunter in shared memory
func (mtr *Dppdpp1intreg1Metrics) SetErrPtrFifoCreditOverrun(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPtrFifoCreditOverrun"))
	return nil
}

// SetErrClipMaxPktSize sets cunter in shared memory
func (mtr *Dppdpp1intreg1Metrics) SetErrClipMaxPktSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrClipMaxPktSize"))
	return nil
}

// SetErrMinPktSize sets cunter in shared memory
func (mtr *Dppdpp1intreg1Metrics) SetErrMinPktSize(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrMinPktSize"))
	return nil
}

// Dppdpp1intreg1MetricsIterator is the iterator object
type Dppdpp1intreg1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dppdpp1intreg1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dppdpp1intreg1MetricsIterator) Next() *Dppdpp1intreg1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dppdpp1intreg1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dppdpp1intreg1MetricsIterator) Find(key uint64) (*Dppdpp1intreg1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dppdpp1intreg1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dppdpp1intreg1MetricsIterator) Create(key uint64) (*Dppdpp1intreg1Metrics, error) {
	tmtr := &Dppdpp1intreg1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dppdpp1intreg1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dppdpp1intreg1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dppdpp1intreg1MetricsIterator) Free() {
	it.iter.Free()
}

// NewDppdpp1intreg1MetricsIterator returns an iterator
func NewDppdpp1intreg1MetricsIterator() (*Dppdpp1intreg1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dppdpp1intreg1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dppdpp1intreg1MetricsIterator{iter: iter}, nil
}

type Dppdpp1intsramseccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	DppPhvFifoUncorrectable metrics.Counter

	DppPhvFifoCorrectable metrics.Counter

	DppOhiFifoUncorrectable metrics.Counter

	DppOhiFifoCorrectable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dppdpp1intsramseccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dppdpp1intsramseccMetrics) Size() int {
	sz := 0

	sz += mtr.DppPhvFifoUncorrectable.Size()

	sz += mtr.DppPhvFifoCorrectable.Size()

	sz += mtr.DppOhiFifoUncorrectable.Size()

	sz += mtr.DppOhiFifoCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dppdpp1intsramseccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.DppPhvFifoUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.DppPhvFifoUncorrectable.Size()

	mtr.DppPhvFifoCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.DppPhvFifoCorrectable.Size()

	mtr.DppOhiFifoUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.DppOhiFifoUncorrectable.Size()

	mtr.DppOhiFifoCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.DppOhiFifoCorrectable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dppdpp1intsramseccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "DppPhvFifoUncorrectable" {
		return offset
	}
	offset += mtr.DppPhvFifoUncorrectable.Size()

	if fldName == "DppPhvFifoCorrectable" {
		return offset
	}
	offset += mtr.DppPhvFifoCorrectable.Size()

	if fldName == "DppOhiFifoUncorrectable" {
		return offset
	}
	offset += mtr.DppOhiFifoUncorrectable.Size()

	if fldName == "DppOhiFifoCorrectable" {
		return offset
	}
	offset += mtr.DppOhiFifoCorrectable.Size()

	return offset
}

// SetDppPhvFifoUncorrectable sets cunter in shared memory
func (mtr *Dppdpp1intsramseccMetrics) SetDppPhvFifoUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DppPhvFifoUncorrectable"))
	return nil
}

// SetDppPhvFifoCorrectable sets cunter in shared memory
func (mtr *Dppdpp1intsramseccMetrics) SetDppPhvFifoCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DppPhvFifoCorrectable"))
	return nil
}

// SetDppOhiFifoUncorrectable sets cunter in shared memory
func (mtr *Dppdpp1intsramseccMetrics) SetDppOhiFifoUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DppOhiFifoUncorrectable"))
	return nil
}

// SetDppOhiFifoCorrectable sets cunter in shared memory
func (mtr *Dppdpp1intsramseccMetrics) SetDppOhiFifoCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DppOhiFifoCorrectable"))
	return nil
}

// Dppdpp1intsramseccMetricsIterator is the iterator object
type Dppdpp1intsramseccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dppdpp1intsramseccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dppdpp1intsramseccMetricsIterator) Next() *Dppdpp1intsramseccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dppdpp1intsramseccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dppdpp1intsramseccMetricsIterator) Find(key uint64) (*Dppdpp1intsramseccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dppdpp1intsramseccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dppdpp1intsramseccMetricsIterator) Create(key uint64) (*Dppdpp1intsramseccMetrics, error) {
	tmtr := &Dppdpp1intsramseccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dppdpp1intsramseccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dppdpp1intsramseccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dppdpp1intsramseccMetricsIterator) Free() {
	it.iter.Free()
}

// NewDppdpp1intsramseccMetricsIterator returns an iterator
func NewDppdpp1intsramseccMetricsIterator() (*Dppdpp1intsramseccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dppdpp1intsramseccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dppdpp1intsramseccMetricsIterator{iter: iter}, nil
}

type Dppdpp1intfifoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PhvFfOverflow metrics.Counter

	OhiFfOverflow metrics.Counter

	PktSizeFfOvflow metrics.Counter

	PktSizeFfUndflow metrics.Counter

	CsumPhvFfOvflow metrics.Counter

	CsumPhvFfUndflow metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dppdpp1intfifoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dppdpp1intfifoMetrics) Size() int {
	sz := 0

	sz += mtr.PhvFfOverflow.Size()

	sz += mtr.OhiFfOverflow.Size()

	sz += mtr.PktSizeFfOvflow.Size()

	sz += mtr.PktSizeFfUndflow.Size()

	sz += mtr.CsumPhvFfOvflow.Size()

	sz += mtr.CsumPhvFfUndflow.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dppdpp1intfifoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.PhvFfOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvFfOverflow.Size()

	mtr.OhiFfOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.OhiFfOverflow.Size()

	mtr.PktSizeFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeFfOvflow.Size()

	mtr.PktSizeFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktSizeFfUndflow.Size()

	mtr.CsumPhvFfOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CsumPhvFfOvflow.Size()

	mtr.CsumPhvFfUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.CsumPhvFfUndflow.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dppdpp1intfifoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "PhvFfOverflow" {
		return offset
	}
	offset += mtr.PhvFfOverflow.Size()

	if fldName == "OhiFfOverflow" {
		return offset
	}
	offset += mtr.OhiFfOverflow.Size()

	if fldName == "PktSizeFfOvflow" {
		return offset
	}
	offset += mtr.PktSizeFfOvflow.Size()

	if fldName == "PktSizeFfUndflow" {
		return offset
	}
	offset += mtr.PktSizeFfUndflow.Size()

	if fldName == "CsumPhvFfOvflow" {
		return offset
	}
	offset += mtr.CsumPhvFfOvflow.Size()

	if fldName == "CsumPhvFfUndflow" {
		return offset
	}
	offset += mtr.CsumPhvFfUndflow.Size()

	return offset
}

// SetPhvFfOverflow sets cunter in shared memory
func (mtr *Dppdpp1intfifoMetrics) SetPhvFfOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvFfOverflow"))
	return nil
}

// SetOhiFfOverflow sets cunter in shared memory
func (mtr *Dppdpp1intfifoMetrics) SetOhiFfOverflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OhiFfOverflow"))
	return nil
}

// SetPktSizeFfOvflow sets cunter in shared memory
func (mtr *Dppdpp1intfifoMetrics) SetPktSizeFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeFfOvflow"))
	return nil
}

// SetPktSizeFfUndflow sets cunter in shared memory
func (mtr *Dppdpp1intfifoMetrics) SetPktSizeFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeFfUndflow"))
	return nil
}

// SetCsumPhvFfOvflow sets cunter in shared memory
func (mtr *Dppdpp1intfifoMetrics) SetCsumPhvFfOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CsumPhvFfOvflow"))
	return nil
}

// SetCsumPhvFfUndflow sets cunter in shared memory
func (mtr *Dppdpp1intfifoMetrics) SetCsumPhvFfUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CsumPhvFfUndflow"))
	return nil
}

// Dppdpp1intfifoMetricsIterator is the iterator object
type Dppdpp1intfifoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dppdpp1intfifoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dppdpp1intfifoMetricsIterator) Next() *Dppdpp1intfifoMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dppdpp1intfifoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dppdpp1intfifoMetricsIterator) Find(key uint64) (*Dppdpp1intfifoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dppdpp1intfifoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dppdpp1intfifoMetricsIterator) Create(key uint64) (*Dppdpp1intfifoMetrics, error) {
	tmtr := &Dppdpp1intfifoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dppdpp1intfifoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dppdpp1intfifoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dppdpp1intfifoMetricsIterator) Free() {
	it.iter.Free()
}

// NewDppdpp1intfifoMetricsIterator returns an iterator
func NewDppdpp1intfifoMetricsIterator() (*Dppdpp1intfifoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dppdpp1intfifoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dppdpp1intfifoMetricsIterator{iter: iter}, nil
}

type Dppdpp1intspareMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Spare_0 metrics.Counter

	Spare_1 metrics.Counter

	Spare_2 metrics.Counter

	Spare_3 metrics.Counter

	Spare_4 metrics.Counter

	Spare_5 metrics.Counter

	Spare_6 metrics.Counter

	Spare_7 metrics.Counter

	Spare_8 metrics.Counter

	Spare_9 metrics.Counter

	Spare_10 metrics.Counter

	Spare_11 metrics.Counter

	Spare_12 metrics.Counter

	Spare_13 metrics.Counter

	Spare_14 metrics.Counter

	Spare_15 metrics.Counter

	Spare_16 metrics.Counter

	Spare_17 metrics.Counter

	Spare_18 metrics.Counter

	Spare_19 metrics.Counter

	Spare_20 metrics.Counter

	Spare_21 metrics.Counter

	Spare_22 metrics.Counter

	Spare_23 metrics.Counter

	Spare_24 metrics.Counter

	Spare_25 metrics.Counter

	Spare_26 metrics.Counter

	Spare_27 metrics.Counter

	Spare_28 metrics.Counter

	Spare_29 metrics.Counter

	Spare_30 metrics.Counter

	Spare_31 metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dppdpp1intspareMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dppdpp1intspareMetrics) Size() int {
	sz := 0

	sz += mtr.Spare_0.Size()

	sz += mtr.Spare_1.Size()

	sz += mtr.Spare_2.Size()

	sz += mtr.Spare_3.Size()

	sz += mtr.Spare_4.Size()

	sz += mtr.Spare_5.Size()

	sz += mtr.Spare_6.Size()

	sz += mtr.Spare_7.Size()

	sz += mtr.Spare_8.Size()

	sz += mtr.Spare_9.Size()

	sz += mtr.Spare_10.Size()

	sz += mtr.Spare_11.Size()

	sz += mtr.Spare_12.Size()

	sz += mtr.Spare_13.Size()

	sz += mtr.Spare_14.Size()

	sz += mtr.Spare_15.Size()

	sz += mtr.Spare_16.Size()

	sz += mtr.Spare_17.Size()

	sz += mtr.Spare_18.Size()

	sz += mtr.Spare_19.Size()

	sz += mtr.Spare_20.Size()

	sz += mtr.Spare_21.Size()

	sz += mtr.Spare_22.Size()

	sz += mtr.Spare_23.Size()

	sz += mtr.Spare_24.Size()

	sz += mtr.Spare_25.Size()

	sz += mtr.Spare_26.Size()

	sz += mtr.Spare_27.Size()

	sz += mtr.Spare_28.Size()

	sz += mtr.Spare_29.Size()

	sz += mtr.Spare_30.Size()

	sz += mtr.Spare_31.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dppdpp1intspareMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Spare_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_0.Size()

	mtr.Spare_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_1.Size()

	mtr.Spare_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_2.Size()

	mtr.Spare_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_3.Size()

	mtr.Spare_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_4.Size()

	mtr.Spare_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_5.Size()

	mtr.Spare_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_6.Size()

	mtr.Spare_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_7.Size()

	mtr.Spare_8 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_8.Size()

	mtr.Spare_9 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_9.Size()

	mtr.Spare_10 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_10.Size()

	mtr.Spare_11 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_11.Size()

	mtr.Spare_12 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_12.Size()

	mtr.Spare_13 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_13.Size()

	mtr.Spare_14 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_14.Size()

	mtr.Spare_15 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_15.Size()

	mtr.Spare_16 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_16.Size()

	mtr.Spare_17 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_17.Size()

	mtr.Spare_18 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_18.Size()

	mtr.Spare_19 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_19.Size()

	mtr.Spare_20 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_20.Size()

	mtr.Spare_21 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_21.Size()

	mtr.Spare_22 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_22.Size()

	mtr.Spare_23 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_23.Size()

	mtr.Spare_24 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_24.Size()

	mtr.Spare_25 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_25.Size()

	mtr.Spare_26 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_26.Size()

	mtr.Spare_27 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_27.Size()

	mtr.Spare_28 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_28.Size()

	mtr.Spare_29 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_29.Size()

	mtr.Spare_30 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_30.Size()

	mtr.Spare_31 = mtr.metrics.GetCounter(offset)
	offset += mtr.Spare_31.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dppdpp1intspareMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Spare_0" {
		return offset
	}
	offset += mtr.Spare_0.Size()

	if fldName == "Spare_1" {
		return offset
	}
	offset += mtr.Spare_1.Size()

	if fldName == "Spare_2" {
		return offset
	}
	offset += mtr.Spare_2.Size()

	if fldName == "Spare_3" {
		return offset
	}
	offset += mtr.Spare_3.Size()

	if fldName == "Spare_4" {
		return offset
	}
	offset += mtr.Spare_4.Size()

	if fldName == "Spare_5" {
		return offset
	}
	offset += mtr.Spare_5.Size()

	if fldName == "Spare_6" {
		return offset
	}
	offset += mtr.Spare_6.Size()

	if fldName == "Spare_7" {
		return offset
	}
	offset += mtr.Spare_7.Size()

	if fldName == "Spare_8" {
		return offset
	}
	offset += mtr.Spare_8.Size()

	if fldName == "Spare_9" {
		return offset
	}
	offset += mtr.Spare_9.Size()

	if fldName == "Spare_10" {
		return offset
	}
	offset += mtr.Spare_10.Size()

	if fldName == "Spare_11" {
		return offset
	}
	offset += mtr.Spare_11.Size()

	if fldName == "Spare_12" {
		return offset
	}
	offset += mtr.Spare_12.Size()

	if fldName == "Spare_13" {
		return offset
	}
	offset += mtr.Spare_13.Size()

	if fldName == "Spare_14" {
		return offset
	}
	offset += mtr.Spare_14.Size()

	if fldName == "Spare_15" {
		return offset
	}
	offset += mtr.Spare_15.Size()

	if fldName == "Spare_16" {
		return offset
	}
	offset += mtr.Spare_16.Size()

	if fldName == "Spare_17" {
		return offset
	}
	offset += mtr.Spare_17.Size()

	if fldName == "Spare_18" {
		return offset
	}
	offset += mtr.Spare_18.Size()

	if fldName == "Spare_19" {
		return offset
	}
	offset += mtr.Spare_19.Size()

	if fldName == "Spare_20" {
		return offset
	}
	offset += mtr.Spare_20.Size()

	if fldName == "Spare_21" {
		return offset
	}
	offset += mtr.Spare_21.Size()

	if fldName == "Spare_22" {
		return offset
	}
	offset += mtr.Spare_22.Size()

	if fldName == "Spare_23" {
		return offset
	}
	offset += mtr.Spare_23.Size()

	if fldName == "Spare_24" {
		return offset
	}
	offset += mtr.Spare_24.Size()

	if fldName == "Spare_25" {
		return offset
	}
	offset += mtr.Spare_25.Size()

	if fldName == "Spare_26" {
		return offset
	}
	offset += mtr.Spare_26.Size()

	if fldName == "Spare_27" {
		return offset
	}
	offset += mtr.Spare_27.Size()

	if fldName == "Spare_28" {
		return offset
	}
	offset += mtr.Spare_28.Size()

	if fldName == "Spare_29" {
		return offset
	}
	offset += mtr.Spare_29.Size()

	if fldName == "Spare_30" {
		return offset
	}
	offset += mtr.Spare_30.Size()

	if fldName == "Spare_31" {
		return offset
	}
	offset += mtr.Spare_31.Size()

	return offset
}

// SetSpare_0 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_0(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_0"))
	return nil
}

// SetSpare_1 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_1(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_1"))
	return nil
}

// SetSpare_2 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_2(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_2"))
	return nil
}

// SetSpare_3 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_3(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_3"))
	return nil
}

// SetSpare_4 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_4(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_4"))
	return nil
}

// SetSpare_5 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_5(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_5"))
	return nil
}

// SetSpare_6 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_6(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_6"))
	return nil
}

// SetSpare_7 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_7(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_7"))
	return nil
}

// SetSpare_8 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_8(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_8"))
	return nil
}

// SetSpare_9 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_9(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_9"))
	return nil
}

// SetSpare_10 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_10(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_10"))
	return nil
}

// SetSpare_11 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_11(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_11"))
	return nil
}

// SetSpare_12 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_12(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_12"))
	return nil
}

// SetSpare_13 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_13(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_13"))
	return nil
}

// SetSpare_14 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_14(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_14"))
	return nil
}

// SetSpare_15 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_15(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_15"))
	return nil
}

// SetSpare_16 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_16(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_16"))
	return nil
}

// SetSpare_17 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_17(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_17"))
	return nil
}

// SetSpare_18 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_18(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_18"))
	return nil
}

// SetSpare_19 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_19(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_19"))
	return nil
}

// SetSpare_20 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_20(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_20"))
	return nil
}

// SetSpare_21 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_21(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_21"))
	return nil
}

// SetSpare_22 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_22(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_22"))
	return nil
}

// SetSpare_23 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_23(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_23"))
	return nil
}

// SetSpare_24 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_24(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_24"))
	return nil
}

// SetSpare_25 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_25(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_25"))
	return nil
}

// SetSpare_26 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_26(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_26"))
	return nil
}

// SetSpare_27 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_27(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_27"))
	return nil
}

// SetSpare_28 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_28(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_28"))
	return nil
}

// SetSpare_29 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_29(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_29"))
	return nil
}

// SetSpare_30 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_30(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_30"))
	return nil
}

// SetSpare_31 sets cunter in shared memory
func (mtr *Dppdpp1intspareMetrics) SetSpare_31(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_31"))
	return nil
}

// Dppdpp1intspareMetricsIterator is the iterator object
type Dppdpp1intspareMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dppdpp1intspareMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dppdpp1intspareMetricsIterator) Next() *Dppdpp1intspareMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dppdpp1intspareMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dppdpp1intspareMetricsIterator) Find(key uint64) (*Dppdpp1intspareMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dppdpp1intspareMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dppdpp1intspareMetricsIterator) Create(key uint64) (*Dppdpp1intspareMetrics, error) {
	tmtr := &Dppdpp1intspareMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dppdpp1intspareMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dppdpp1intspareMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dppdpp1intspareMetricsIterator) Free() {
	it.iter.Free()
}

// NewDppdpp1intspareMetricsIterator returns an iterator
func NewDppdpp1intspareMetricsIterator() (*Dppdpp1intspareMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dppdpp1intspareMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dppdpp1intspareMetricsIterator{iter: iter}, nil
}

type Dppdpp1intcreditMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PtrCreditOvflow metrics.Counter

	PtrCreditUndflow metrics.Counter

	PktCreditOvflow metrics.Counter

	PktCreditUndflow metrics.Counter

	FramerCreditOvflow metrics.Counter

	FramerCreditUndflow metrics.Counter

	FramerHdrfldVldOvfl metrics.Counter

	FramerHdrfldOffsetOvfl metrics.Counter

	ErrFramerHdrsizeZeroOvfl metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dppdpp1intcreditMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dppdpp1intcreditMetrics) Size() int {
	sz := 0

	sz += mtr.PtrCreditOvflow.Size()

	sz += mtr.PtrCreditUndflow.Size()

	sz += mtr.PktCreditOvflow.Size()

	sz += mtr.PktCreditUndflow.Size()

	sz += mtr.FramerCreditOvflow.Size()

	sz += mtr.FramerCreditUndflow.Size()

	sz += mtr.FramerHdrfldVldOvfl.Size()

	sz += mtr.FramerHdrfldOffsetOvfl.Size()

	sz += mtr.ErrFramerHdrsizeZeroOvfl.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dppdpp1intcreditMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.PtrCreditOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PtrCreditOvflow.Size()

	mtr.PtrCreditUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PtrCreditUndflow.Size()

	mtr.PktCreditOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktCreditOvflow.Size()

	mtr.PktCreditUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.PktCreditUndflow.Size()

	mtr.FramerCreditOvflow = mtr.metrics.GetCounter(offset)
	offset += mtr.FramerCreditOvflow.Size()

	mtr.FramerCreditUndflow = mtr.metrics.GetCounter(offset)
	offset += mtr.FramerCreditUndflow.Size()

	mtr.FramerHdrfldVldOvfl = mtr.metrics.GetCounter(offset)
	offset += mtr.FramerHdrfldVldOvfl.Size()

	mtr.FramerHdrfldOffsetOvfl = mtr.metrics.GetCounter(offset)
	offset += mtr.FramerHdrfldOffsetOvfl.Size()

	mtr.ErrFramerHdrsizeZeroOvfl = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrFramerHdrsizeZeroOvfl.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dppdpp1intcreditMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "PtrCreditOvflow" {
		return offset
	}
	offset += mtr.PtrCreditOvflow.Size()

	if fldName == "PtrCreditUndflow" {
		return offset
	}
	offset += mtr.PtrCreditUndflow.Size()

	if fldName == "PktCreditOvflow" {
		return offset
	}
	offset += mtr.PktCreditOvflow.Size()

	if fldName == "PktCreditUndflow" {
		return offset
	}
	offset += mtr.PktCreditUndflow.Size()

	if fldName == "FramerCreditOvflow" {
		return offset
	}
	offset += mtr.FramerCreditOvflow.Size()

	if fldName == "FramerCreditUndflow" {
		return offset
	}
	offset += mtr.FramerCreditUndflow.Size()

	if fldName == "FramerHdrfldVldOvfl" {
		return offset
	}
	offset += mtr.FramerHdrfldVldOvfl.Size()

	if fldName == "FramerHdrfldOffsetOvfl" {
		return offset
	}
	offset += mtr.FramerHdrfldOffsetOvfl.Size()

	if fldName == "ErrFramerHdrsizeZeroOvfl" {
		return offset
	}
	offset += mtr.ErrFramerHdrsizeZeroOvfl.Size()

	return offset
}

// SetPtrCreditOvflow sets cunter in shared memory
func (mtr *Dppdpp1intcreditMetrics) SetPtrCreditOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PtrCreditOvflow"))
	return nil
}

// SetPtrCreditUndflow sets cunter in shared memory
func (mtr *Dppdpp1intcreditMetrics) SetPtrCreditUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PtrCreditUndflow"))
	return nil
}

// SetPktCreditOvflow sets cunter in shared memory
func (mtr *Dppdpp1intcreditMetrics) SetPktCreditOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktCreditOvflow"))
	return nil
}

// SetPktCreditUndflow sets cunter in shared memory
func (mtr *Dppdpp1intcreditMetrics) SetPktCreditUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktCreditUndflow"))
	return nil
}

// SetFramerCreditOvflow sets cunter in shared memory
func (mtr *Dppdpp1intcreditMetrics) SetFramerCreditOvflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramerCreditOvflow"))
	return nil
}

// SetFramerCreditUndflow sets cunter in shared memory
func (mtr *Dppdpp1intcreditMetrics) SetFramerCreditUndflow(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramerCreditUndflow"))
	return nil
}

// SetFramerHdrfldVldOvfl sets cunter in shared memory
func (mtr *Dppdpp1intcreditMetrics) SetFramerHdrfldVldOvfl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramerHdrfldVldOvfl"))
	return nil
}

// SetFramerHdrfldOffsetOvfl sets cunter in shared memory
func (mtr *Dppdpp1intcreditMetrics) SetFramerHdrfldOffsetOvfl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramerHdrfldOffsetOvfl"))
	return nil
}

// SetErrFramerHdrsizeZeroOvfl sets cunter in shared memory
func (mtr *Dppdpp1intcreditMetrics) SetErrFramerHdrsizeZeroOvfl(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrFramerHdrsizeZeroOvfl"))
	return nil
}

// Dppdpp1intcreditMetricsIterator is the iterator object
type Dppdpp1intcreditMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dppdpp1intcreditMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dppdpp1intcreditMetricsIterator) Next() *Dppdpp1intcreditMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Dppdpp1intcreditMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dppdpp1intcreditMetricsIterator) Find(key uint64) (*Dppdpp1intcreditMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dppdpp1intcreditMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dppdpp1intcreditMetricsIterator) Create(key uint64) (*Dppdpp1intcreditMetrics, error) {
	tmtr := &Dppdpp1intcreditMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dppdpp1intcreditMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dppdpp1intcreditMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Dppdpp1intcreditMetricsIterator) Free() {
	it.iter.Free()
}

// NewDppdpp1intcreditMetricsIterator returns an iterator
func NewDppdpp1intcreditMetricsIterator() (*Dppdpp1intcreditMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dppdpp1intcreditMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dppdpp1intcreditMetricsIterator{iter: iter}, nil
}

type MsmsintmsMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SbusErr metrics.Counter

	ReiErr metrics.Counter

	ReiPllErr metrics.Counter

	PkEfuseInitReadTimeout metrics.Counter

	PkEfuseCpuReadTimeout metrics.Counter

	PkEfuseCpuBurnTimeout metrics.Counter

	PkEfuseBurnImmTimeout metrics.Counter

	Ds16EfuseInitReadTimeout metrics.Counter

	Ds16EfuseCpuReadTimeout metrics.Counter

	AhbAxiErr metrics.Counter

	EmmcPreAxiErr metrics.Counter

	FlAxiErr metrics.Counter

	MsEsecMbPreAxiErr metrics.Counter

	MsapPreAxiErr metrics.Counter

	NxmsA13AxiErr metrics.Counter

	RbmAxiErr metrics.Counter

	SsramAxiErr metrics.Counter

	AhbAxiWerr metrics.Counter

	EmmcPreAxiWerr metrics.Counter

	FlAxiWerr metrics.Counter

	MsEsecMbPreAxiWerr metrics.Counter

	MsapPreAxiWerr metrics.Counter

	NxmsA13AxiWerr metrics.Counter

	RbmAxiWerr metrics.Counter

	SsramAxiWerr metrics.Counter

	AddrFilterWrite metrics.Counter

	AddrFilterRead metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *MsmsintmsMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *MsmsintmsMetrics) Size() int {
	sz := 0

	sz += mtr.SbusErr.Size()

	sz += mtr.ReiErr.Size()

	sz += mtr.ReiPllErr.Size()

	sz += mtr.PkEfuseInitReadTimeout.Size()

	sz += mtr.PkEfuseCpuReadTimeout.Size()

	sz += mtr.PkEfuseCpuBurnTimeout.Size()

	sz += mtr.PkEfuseBurnImmTimeout.Size()

	sz += mtr.Ds16EfuseInitReadTimeout.Size()

	sz += mtr.Ds16EfuseCpuReadTimeout.Size()

	sz += mtr.AhbAxiErr.Size()

	sz += mtr.EmmcPreAxiErr.Size()

	sz += mtr.FlAxiErr.Size()

	sz += mtr.MsEsecMbPreAxiErr.Size()

	sz += mtr.MsapPreAxiErr.Size()

	sz += mtr.NxmsA13AxiErr.Size()

	sz += mtr.RbmAxiErr.Size()

	sz += mtr.SsramAxiErr.Size()

	sz += mtr.AhbAxiWerr.Size()

	sz += mtr.EmmcPreAxiWerr.Size()

	sz += mtr.FlAxiWerr.Size()

	sz += mtr.MsEsecMbPreAxiWerr.Size()

	sz += mtr.MsapPreAxiWerr.Size()

	sz += mtr.NxmsA13AxiWerr.Size()

	sz += mtr.RbmAxiWerr.Size()

	sz += mtr.SsramAxiWerr.Size()

	sz += mtr.AddrFilterWrite.Size()

	sz += mtr.AddrFilterRead.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *MsmsintmsMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SbusErr = mtr.metrics.GetCounter(offset)
	offset += mtr.SbusErr.Size()

	mtr.ReiErr = mtr.metrics.GetCounter(offset)
	offset += mtr.ReiErr.Size()

	mtr.ReiPllErr = mtr.metrics.GetCounter(offset)
	offset += mtr.ReiPllErr.Size()

	mtr.PkEfuseInitReadTimeout = mtr.metrics.GetCounter(offset)
	offset += mtr.PkEfuseInitReadTimeout.Size()

	mtr.PkEfuseCpuReadTimeout = mtr.metrics.GetCounter(offset)
	offset += mtr.PkEfuseCpuReadTimeout.Size()

	mtr.PkEfuseCpuBurnTimeout = mtr.metrics.GetCounter(offset)
	offset += mtr.PkEfuseCpuBurnTimeout.Size()

	mtr.PkEfuseBurnImmTimeout = mtr.metrics.GetCounter(offset)
	offset += mtr.PkEfuseBurnImmTimeout.Size()

	mtr.Ds16EfuseInitReadTimeout = mtr.metrics.GetCounter(offset)
	offset += mtr.Ds16EfuseInitReadTimeout.Size()

	mtr.Ds16EfuseCpuReadTimeout = mtr.metrics.GetCounter(offset)
	offset += mtr.Ds16EfuseCpuReadTimeout.Size()

	mtr.AhbAxiErr = mtr.metrics.GetCounter(offset)
	offset += mtr.AhbAxiErr.Size()

	mtr.EmmcPreAxiErr = mtr.metrics.GetCounter(offset)
	offset += mtr.EmmcPreAxiErr.Size()

	mtr.FlAxiErr = mtr.metrics.GetCounter(offset)
	offset += mtr.FlAxiErr.Size()

	mtr.MsEsecMbPreAxiErr = mtr.metrics.GetCounter(offset)
	offset += mtr.MsEsecMbPreAxiErr.Size()

	mtr.MsapPreAxiErr = mtr.metrics.GetCounter(offset)
	offset += mtr.MsapPreAxiErr.Size()

	mtr.NxmsA13AxiErr = mtr.metrics.GetCounter(offset)
	offset += mtr.NxmsA13AxiErr.Size()

	mtr.RbmAxiErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RbmAxiErr.Size()

	mtr.SsramAxiErr = mtr.metrics.GetCounter(offset)
	offset += mtr.SsramAxiErr.Size()

	mtr.AhbAxiWerr = mtr.metrics.GetCounter(offset)
	offset += mtr.AhbAxiWerr.Size()

	mtr.EmmcPreAxiWerr = mtr.metrics.GetCounter(offset)
	offset += mtr.EmmcPreAxiWerr.Size()

	mtr.FlAxiWerr = mtr.metrics.GetCounter(offset)
	offset += mtr.FlAxiWerr.Size()

	mtr.MsEsecMbPreAxiWerr = mtr.metrics.GetCounter(offset)
	offset += mtr.MsEsecMbPreAxiWerr.Size()

	mtr.MsapPreAxiWerr = mtr.metrics.GetCounter(offset)
	offset += mtr.MsapPreAxiWerr.Size()

	mtr.NxmsA13AxiWerr = mtr.metrics.GetCounter(offset)
	offset += mtr.NxmsA13AxiWerr.Size()

	mtr.RbmAxiWerr = mtr.metrics.GetCounter(offset)
	offset += mtr.RbmAxiWerr.Size()

	mtr.SsramAxiWerr = mtr.metrics.GetCounter(offset)
	offset += mtr.SsramAxiWerr.Size()

	mtr.AddrFilterWrite = mtr.metrics.GetCounter(offset)
	offset += mtr.AddrFilterWrite.Size()

	mtr.AddrFilterRead = mtr.metrics.GetCounter(offset)
	offset += mtr.AddrFilterRead.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *MsmsintmsMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SbusErr" {
		return offset
	}
	offset += mtr.SbusErr.Size()

	if fldName == "ReiErr" {
		return offset
	}
	offset += mtr.ReiErr.Size()

	if fldName == "ReiPllErr" {
		return offset
	}
	offset += mtr.ReiPllErr.Size()

	if fldName == "PkEfuseInitReadTimeout" {
		return offset
	}
	offset += mtr.PkEfuseInitReadTimeout.Size()

	if fldName == "PkEfuseCpuReadTimeout" {
		return offset
	}
	offset += mtr.PkEfuseCpuReadTimeout.Size()

	if fldName == "PkEfuseCpuBurnTimeout" {
		return offset
	}
	offset += mtr.PkEfuseCpuBurnTimeout.Size()

	if fldName == "PkEfuseBurnImmTimeout" {
		return offset
	}
	offset += mtr.PkEfuseBurnImmTimeout.Size()

	if fldName == "Ds16EfuseInitReadTimeout" {
		return offset
	}
	offset += mtr.Ds16EfuseInitReadTimeout.Size()

	if fldName == "Ds16EfuseCpuReadTimeout" {
		return offset
	}
	offset += mtr.Ds16EfuseCpuReadTimeout.Size()

	if fldName == "AhbAxiErr" {
		return offset
	}
	offset += mtr.AhbAxiErr.Size()

	if fldName == "EmmcPreAxiErr" {
		return offset
	}
	offset += mtr.EmmcPreAxiErr.Size()

	if fldName == "FlAxiErr" {
		return offset
	}
	offset += mtr.FlAxiErr.Size()

	if fldName == "MsEsecMbPreAxiErr" {
		return offset
	}
	offset += mtr.MsEsecMbPreAxiErr.Size()

	if fldName == "MsapPreAxiErr" {
		return offset
	}
	offset += mtr.MsapPreAxiErr.Size()

	if fldName == "NxmsA13AxiErr" {
		return offset
	}
	offset += mtr.NxmsA13AxiErr.Size()

	if fldName == "RbmAxiErr" {
		return offset
	}
	offset += mtr.RbmAxiErr.Size()

	if fldName == "SsramAxiErr" {
		return offset
	}
	offset += mtr.SsramAxiErr.Size()

	if fldName == "AhbAxiWerr" {
		return offset
	}
	offset += mtr.AhbAxiWerr.Size()

	if fldName == "EmmcPreAxiWerr" {
		return offset
	}
	offset += mtr.EmmcPreAxiWerr.Size()

	if fldName == "FlAxiWerr" {
		return offset
	}
	offset += mtr.FlAxiWerr.Size()

	if fldName == "MsEsecMbPreAxiWerr" {
		return offset
	}
	offset += mtr.MsEsecMbPreAxiWerr.Size()

	if fldName == "MsapPreAxiWerr" {
		return offset
	}
	offset += mtr.MsapPreAxiWerr.Size()

	if fldName == "NxmsA13AxiWerr" {
		return offset
	}
	offset += mtr.NxmsA13AxiWerr.Size()

	if fldName == "RbmAxiWerr" {
		return offset
	}
	offset += mtr.RbmAxiWerr.Size()

	if fldName == "SsramAxiWerr" {
		return offset
	}
	offset += mtr.SsramAxiWerr.Size()

	if fldName == "AddrFilterWrite" {
		return offset
	}
	offset += mtr.AddrFilterWrite.Size()

	if fldName == "AddrFilterRead" {
		return offset
	}
	offset += mtr.AddrFilterRead.Size()

	return offset
}

// SetSbusErr sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetSbusErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SbusErr"))
	return nil
}

// SetReiErr sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetReiErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ReiErr"))
	return nil
}

// SetReiPllErr sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetReiPllErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ReiPllErr"))
	return nil
}

// SetPkEfuseInitReadTimeout sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetPkEfuseInitReadTimeout(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PkEfuseInitReadTimeout"))
	return nil
}

// SetPkEfuseCpuReadTimeout sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetPkEfuseCpuReadTimeout(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PkEfuseCpuReadTimeout"))
	return nil
}

// SetPkEfuseCpuBurnTimeout sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetPkEfuseCpuBurnTimeout(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PkEfuseCpuBurnTimeout"))
	return nil
}

// SetPkEfuseBurnImmTimeout sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetPkEfuseBurnImmTimeout(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PkEfuseBurnImmTimeout"))
	return nil
}

// SetDs16EfuseInitReadTimeout sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetDs16EfuseInitReadTimeout(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ds16EfuseInitReadTimeout"))
	return nil
}

// SetDs16EfuseCpuReadTimeout sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetDs16EfuseCpuReadTimeout(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ds16EfuseCpuReadTimeout"))
	return nil
}

// SetAhbAxiErr sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetAhbAxiErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AhbAxiErr"))
	return nil
}

// SetEmmcPreAxiErr sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetEmmcPreAxiErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EmmcPreAxiErr"))
	return nil
}

// SetFlAxiErr sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetFlAxiErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FlAxiErr"))
	return nil
}

// SetMsEsecMbPreAxiErr sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetMsEsecMbPreAxiErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsEsecMbPreAxiErr"))
	return nil
}

// SetMsapPreAxiErr sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetMsapPreAxiErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsapPreAxiErr"))
	return nil
}

// SetNxmsA13AxiErr sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetNxmsA13AxiErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NxmsA13AxiErr"))
	return nil
}

// SetRbmAxiErr sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetRbmAxiErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RbmAxiErr"))
	return nil
}

// SetSsramAxiErr sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetSsramAxiErr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SsramAxiErr"))
	return nil
}

// SetAhbAxiWerr sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetAhbAxiWerr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AhbAxiWerr"))
	return nil
}

// SetEmmcPreAxiWerr sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetEmmcPreAxiWerr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EmmcPreAxiWerr"))
	return nil
}

// SetFlAxiWerr sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetFlAxiWerr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FlAxiWerr"))
	return nil
}

// SetMsEsecMbPreAxiWerr sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetMsEsecMbPreAxiWerr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsEsecMbPreAxiWerr"))
	return nil
}

// SetMsapPreAxiWerr sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetMsapPreAxiWerr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MsapPreAxiWerr"))
	return nil
}

// SetNxmsA13AxiWerr sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetNxmsA13AxiWerr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("NxmsA13AxiWerr"))
	return nil
}

// SetRbmAxiWerr sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetRbmAxiWerr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RbmAxiWerr"))
	return nil
}

// SetSsramAxiWerr sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetSsramAxiWerr(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SsramAxiWerr"))
	return nil
}

// SetAddrFilterWrite sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetAddrFilterWrite(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AddrFilterWrite"))
	return nil
}

// SetAddrFilterRead sets cunter in shared memory
func (mtr *MsmsintmsMetrics) SetAddrFilterRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AddrFilterRead"))
	return nil
}

// MsmsintmsMetricsIterator is the iterator object
type MsmsintmsMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *MsmsintmsMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *MsmsintmsMetricsIterator) Next() *MsmsintmsMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &MsmsintmsMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *MsmsintmsMetricsIterator) Find(key uint64) (*MsmsintmsMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &MsmsintmsMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *MsmsintmsMetricsIterator) Create(key uint64) (*MsmsintmsMetrics, error) {
	tmtr := &MsmsintmsMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &MsmsintmsMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *MsmsintmsMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *MsmsintmsMetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintmsMetricsIterator returns an iterator
func NewMsmsintmsMetricsIterator() (*MsmsintmsMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("MsmsintmsMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &MsmsintmsMetricsIterator{iter: iter}, nil
}

type MsmsintmiscMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	QspiRamEccUncorrectable metrics.Counter

	QspiRamEccCorrectable metrics.Counter

	ElamCapEccUncorrectable metrics.Counter

	ElamCapEccCorrectable metrics.Counter

	AhbEsecDataRamEccUncorrectable metrics.Counter

	AhbEsecDataRamEccCorrectable metrics.Counter

	AhbEsecEsystemRamEccUncorrectable metrics.Counter

	AhbEsecEsystemRamEccCorrectable metrics.Counter

	Bl2RamEccUncorrectable metrics.Counter

	Bl2RamEccCorrectable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *MsmsintmiscMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *MsmsintmiscMetrics) Size() int {
	sz := 0

	sz += mtr.QspiRamEccUncorrectable.Size()

	sz += mtr.QspiRamEccCorrectable.Size()

	sz += mtr.ElamCapEccUncorrectable.Size()

	sz += mtr.ElamCapEccCorrectable.Size()

	sz += mtr.AhbEsecDataRamEccUncorrectable.Size()

	sz += mtr.AhbEsecDataRamEccCorrectable.Size()

	sz += mtr.AhbEsecEsystemRamEccUncorrectable.Size()

	sz += mtr.AhbEsecEsystemRamEccCorrectable.Size()

	sz += mtr.Bl2RamEccUncorrectable.Size()

	sz += mtr.Bl2RamEccCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *MsmsintmiscMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.QspiRamEccUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.QspiRamEccUncorrectable.Size()

	mtr.QspiRamEccCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.QspiRamEccCorrectable.Size()

	mtr.ElamCapEccUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.ElamCapEccUncorrectable.Size()

	mtr.ElamCapEccCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.ElamCapEccCorrectable.Size()

	mtr.AhbEsecDataRamEccUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.AhbEsecDataRamEccUncorrectable.Size()

	mtr.AhbEsecDataRamEccCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.AhbEsecDataRamEccCorrectable.Size()

	mtr.AhbEsecEsystemRamEccUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.AhbEsecEsystemRamEccUncorrectable.Size()

	mtr.AhbEsecEsystemRamEccCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.AhbEsecEsystemRamEccCorrectable.Size()

	mtr.Bl2RamEccUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Bl2RamEccUncorrectable.Size()

	mtr.Bl2RamEccCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Bl2RamEccCorrectable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *MsmsintmiscMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "QspiRamEccUncorrectable" {
		return offset
	}
	offset += mtr.QspiRamEccUncorrectable.Size()

	if fldName == "QspiRamEccCorrectable" {
		return offset
	}
	offset += mtr.QspiRamEccCorrectable.Size()

	if fldName == "ElamCapEccUncorrectable" {
		return offset
	}
	offset += mtr.ElamCapEccUncorrectable.Size()

	if fldName == "ElamCapEccCorrectable" {
		return offset
	}
	offset += mtr.ElamCapEccCorrectable.Size()

	if fldName == "AhbEsecDataRamEccUncorrectable" {
		return offset
	}
	offset += mtr.AhbEsecDataRamEccUncorrectable.Size()

	if fldName == "AhbEsecDataRamEccCorrectable" {
		return offset
	}
	offset += mtr.AhbEsecDataRamEccCorrectable.Size()

	if fldName == "AhbEsecEsystemRamEccUncorrectable" {
		return offset
	}
	offset += mtr.AhbEsecEsystemRamEccUncorrectable.Size()

	if fldName == "AhbEsecEsystemRamEccCorrectable" {
		return offset
	}
	offset += mtr.AhbEsecEsystemRamEccCorrectable.Size()

	if fldName == "Bl2RamEccUncorrectable" {
		return offset
	}
	offset += mtr.Bl2RamEccUncorrectable.Size()

	if fldName == "Bl2RamEccCorrectable" {
		return offset
	}
	offset += mtr.Bl2RamEccCorrectable.Size()

	return offset
}

// SetQspiRamEccUncorrectable sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetQspiRamEccUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QspiRamEccUncorrectable"))
	return nil
}

// SetQspiRamEccCorrectable sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetQspiRamEccCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QspiRamEccCorrectable"))
	return nil
}

// SetElamCapEccUncorrectable sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetElamCapEccUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ElamCapEccUncorrectable"))
	return nil
}

// SetElamCapEccCorrectable sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetElamCapEccCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ElamCapEccCorrectable"))
	return nil
}

// SetAhbEsecDataRamEccUncorrectable sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetAhbEsecDataRamEccUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AhbEsecDataRamEccUncorrectable"))
	return nil
}

// SetAhbEsecDataRamEccCorrectable sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetAhbEsecDataRamEccCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AhbEsecDataRamEccCorrectable"))
	return nil
}

// SetAhbEsecEsystemRamEccUncorrectable sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetAhbEsecEsystemRamEccUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AhbEsecEsystemRamEccUncorrectable"))
	return nil
}

// SetAhbEsecEsystemRamEccCorrectable sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetAhbEsecEsystemRamEccCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AhbEsecEsystemRamEccCorrectable"))
	return nil
}

// SetBl2RamEccUncorrectable sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetBl2RamEccUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Bl2RamEccUncorrectable"))
	return nil
}

// SetBl2RamEccCorrectable sets cunter in shared memory
func (mtr *MsmsintmiscMetrics) SetBl2RamEccCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Bl2RamEccCorrectable"))
	return nil
}

// MsmsintmiscMetricsIterator is the iterator object
type MsmsintmiscMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *MsmsintmiscMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *MsmsintmiscMetricsIterator) Next() *MsmsintmiscMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &MsmsintmiscMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *MsmsintmiscMetricsIterator) Find(key uint64) (*MsmsintmiscMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &MsmsintmiscMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *MsmsintmiscMetricsIterator) Create(key uint64) (*MsmsintmiscMetrics, error) {
	tmtr := &MsmsintmiscMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &MsmsintmiscMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *MsmsintmiscMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *MsmsintmiscMetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintmiscMetricsIterator returns an iterator
func NewMsmsintmiscMetricsIterator() (*MsmsintmiscMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("MsmsintmiscMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &MsmsintmiscMetricsIterator{iter: iter}, nil
}

type Msmsintprp1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Read metrics.Counter

	Security metrics.Counter

	Decode metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintprp1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintprp1Metrics) Size() int {
	sz := 0

	sz += mtr.Read.Size()

	sz += mtr.Security.Size()

	sz += mtr.Decode.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintprp1Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Read = mtr.metrics.GetCounter(offset)
	offset += mtr.Read.Size()

	mtr.Security = mtr.metrics.GetCounter(offset)
	offset += mtr.Security.Size()

	mtr.Decode = mtr.metrics.GetCounter(offset)
	offset += mtr.Decode.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintprp1Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Read" {
		return offset
	}
	offset += mtr.Read.Size()

	if fldName == "Security" {
		return offset
	}
	offset += mtr.Security.Size()

	if fldName == "Decode" {
		return offset
	}
	offset += mtr.Decode.Size()

	return offset
}

// SetRead sets cunter in shared memory
func (mtr *Msmsintprp1Metrics) SetRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Read"))
	return nil
}

// SetSecurity sets cunter in shared memory
func (mtr *Msmsintprp1Metrics) SetSecurity(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Security"))
	return nil
}

// SetDecode sets cunter in shared memory
func (mtr *Msmsintprp1Metrics) SetDecode(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Decode"))
	return nil
}

// Msmsintprp1MetricsIterator is the iterator object
type Msmsintprp1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintprp1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintprp1MetricsIterator) Next() *Msmsintprp1Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintprp1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintprp1MetricsIterator) Find(key uint64) (*Msmsintprp1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintprp1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintprp1MetricsIterator) Create(key uint64) (*Msmsintprp1Metrics, error) {
	tmtr := &Msmsintprp1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintprp1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintprp1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintprp1MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintprp1MetricsIterator returns an iterator
func NewMsmsintprp1MetricsIterator() (*Msmsintprp1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintprp1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintprp1MetricsIterator{iter: iter}, nil
}

type Msmsintprp2Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Read metrics.Counter

	Security metrics.Counter

	Decode metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintprp2Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintprp2Metrics) Size() int {
	sz := 0

	sz += mtr.Read.Size()

	sz += mtr.Security.Size()

	sz += mtr.Decode.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintprp2Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Read = mtr.metrics.GetCounter(offset)
	offset += mtr.Read.Size()

	mtr.Security = mtr.metrics.GetCounter(offset)
	offset += mtr.Security.Size()

	mtr.Decode = mtr.metrics.GetCounter(offset)
	offset += mtr.Decode.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintprp2Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Read" {
		return offset
	}
	offset += mtr.Read.Size()

	if fldName == "Security" {
		return offset
	}
	offset += mtr.Security.Size()

	if fldName == "Decode" {
		return offset
	}
	offset += mtr.Decode.Size()

	return offset
}

// SetRead sets cunter in shared memory
func (mtr *Msmsintprp2Metrics) SetRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Read"))
	return nil
}

// SetSecurity sets cunter in shared memory
func (mtr *Msmsintprp2Metrics) SetSecurity(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Security"))
	return nil
}

// SetDecode sets cunter in shared memory
func (mtr *Msmsintprp2Metrics) SetDecode(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Decode"))
	return nil
}

// Msmsintprp2MetricsIterator is the iterator object
type Msmsintprp2MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintprp2MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintprp2MetricsIterator) Next() *Msmsintprp2Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintprp2Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintprp2MetricsIterator) Find(key uint64) (*Msmsintprp2Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintprp2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintprp2MetricsIterator) Create(key uint64) (*Msmsintprp2Metrics, error) {
	tmtr := &Msmsintprp2Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintprp2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintprp2MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintprp2MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintprp2MetricsIterator returns an iterator
func NewMsmsintprp2MetricsIterator() (*Msmsintprp2MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintprp2Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintprp2MetricsIterator{iter: iter}, nil
}

type Msmsintprp3Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Read metrics.Counter

	Security metrics.Counter

	Decode metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintprp3Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintprp3Metrics) Size() int {
	sz := 0

	sz += mtr.Read.Size()

	sz += mtr.Security.Size()

	sz += mtr.Decode.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintprp3Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Read = mtr.metrics.GetCounter(offset)
	offset += mtr.Read.Size()

	mtr.Security = mtr.metrics.GetCounter(offset)
	offset += mtr.Security.Size()

	mtr.Decode = mtr.metrics.GetCounter(offset)
	offset += mtr.Decode.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintprp3Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Read" {
		return offset
	}
	offset += mtr.Read.Size()

	if fldName == "Security" {
		return offset
	}
	offset += mtr.Security.Size()

	if fldName == "Decode" {
		return offset
	}
	offset += mtr.Decode.Size()

	return offset
}

// SetRead sets cunter in shared memory
func (mtr *Msmsintprp3Metrics) SetRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Read"))
	return nil
}

// SetSecurity sets cunter in shared memory
func (mtr *Msmsintprp3Metrics) SetSecurity(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Security"))
	return nil
}

// SetDecode sets cunter in shared memory
func (mtr *Msmsintprp3Metrics) SetDecode(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Decode"))
	return nil
}

// Msmsintprp3MetricsIterator is the iterator object
type Msmsintprp3MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintprp3MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintprp3MetricsIterator) Next() *Msmsintprp3Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintprp3Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintprp3MetricsIterator) Find(key uint64) (*Msmsintprp3Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintprp3Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintprp3MetricsIterator) Create(key uint64) (*Msmsintprp3Metrics, error) {
	tmtr := &Msmsintprp3Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintprp3Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintprp3MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintprp3MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintprp3MetricsIterator returns an iterator
func NewMsmsintprp3MetricsIterator() (*Msmsintprp3MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintprp3Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintprp3MetricsIterator{iter: iter}, nil
}

type Msmsintprp4Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Read metrics.Counter

	Security metrics.Counter

	Decode metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintprp4Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintprp4Metrics) Size() int {
	sz := 0

	sz += mtr.Read.Size()

	sz += mtr.Security.Size()

	sz += mtr.Decode.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintprp4Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Read = mtr.metrics.GetCounter(offset)
	offset += mtr.Read.Size()

	mtr.Security = mtr.metrics.GetCounter(offset)
	offset += mtr.Security.Size()

	mtr.Decode = mtr.metrics.GetCounter(offset)
	offset += mtr.Decode.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintprp4Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Read" {
		return offset
	}
	offset += mtr.Read.Size()

	if fldName == "Security" {
		return offset
	}
	offset += mtr.Security.Size()

	if fldName == "Decode" {
		return offset
	}
	offset += mtr.Decode.Size()

	return offset
}

// SetRead sets cunter in shared memory
func (mtr *Msmsintprp4Metrics) SetRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Read"))
	return nil
}

// SetSecurity sets cunter in shared memory
func (mtr *Msmsintprp4Metrics) SetSecurity(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Security"))
	return nil
}

// SetDecode sets cunter in shared memory
func (mtr *Msmsintprp4Metrics) SetDecode(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Decode"))
	return nil
}

// Msmsintprp4MetricsIterator is the iterator object
type Msmsintprp4MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintprp4MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintprp4MetricsIterator) Next() *Msmsintprp4Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintprp4Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintprp4MetricsIterator) Find(key uint64) (*Msmsintprp4Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintprp4Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintprp4MetricsIterator) Create(key uint64) (*Msmsintprp4Metrics, error) {
	tmtr := &Msmsintprp4Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintprp4Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintprp4MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintprp4MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintprp4MetricsIterator returns an iterator
func NewMsmsintprp4MetricsIterator() (*Msmsintprp4MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintprp4Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintprp4MetricsIterator{iter: iter}, nil
}

type Msmsintprp5Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Read metrics.Counter

	Security metrics.Counter

	Decode metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Msmsintprp5Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Msmsintprp5Metrics) Size() int {
	sz := 0

	sz += mtr.Read.Size()

	sz += mtr.Security.Size()

	sz += mtr.Decode.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Msmsintprp5Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Read = mtr.metrics.GetCounter(offset)
	offset += mtr.Read.Size()

	mtr.Security = mtr.metrics.GetCounter(offset)
	offset += mtr.Security.Size()

	mtr.Decode = mtr.metrics.GetCounter(offset)
	offset += mtr.Decode.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Msmsintprp5Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Read" {
		return offset
	}
	offset += mtr.Read.Size()

	if fldName == "Security" {
		return offset
	}
	offset += mtr.Security.Size()

	if fldName == "Decode" {
		return offset
	}
	offset += mtr.Decode.Size()

	return offset
}

// SetRead sets cunter in shared memory
func (mtr *Msmsintprp5Metrics) SetRead(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Read"))
	return nil
}

// SetSecurity sets cunter in shared memory
func (mtr *Msmsintprp5Metrics) SetSecurity(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Security"))
	return nil
}

// SetDecode sets cunter in shared memory
func (mtr *Msmsintprp5Metrics) SetDecode(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Decode"))
	return nil
}

// Msmsintprp5MetricsIterator is the iterator object
type Msmsintprp5MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Msmsintprp5MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Msmsintprp5MetricsIterator) Next() *Msmsintprp5Metrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Msmsintprp5Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Msmsintprp5MetricsIterator) Find(key uint64) (*Msmsintprp5Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Msmsintprp5Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Msmsintprp5MetricsIterator) Create(key uint64) (*Msmsintprp5Metrics, error) {
	tmtr := &Msmsintprp5Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Msmsintprp5Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Msmsintprp5MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Msmsintprp5MetricsIterator) Free() {
	it.iter.Free()
}

// NewMsmsintprp5MetricsIterator returns an iterator
func NewMsmsintprp5MetricsIterator() (*Msmsintprp5MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Msmsintprp5Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Msmsintprp5MetricsIterator{iter: iter}, nil
}

type Mxmx0intmacMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Lane0Sbe metrics.Counter

	Lane0Dbe metrics.Counter

	Lane1Sbe metrics.Counter

	Lane1Dbe metrics.Counter

	Lane2Sbe metrics.Counter

	Lane2Dbe metrics.Counter

	Lane3Sbe metrics.Counter

	Lane3Dbe metrics.Counter

	M0PbPbusDrdy metrics.Counter

	M1PbPbusDrdy metrics.Counter

	M2PbPbusDrdy metrics.Counter

	M3PbPbusDrdy metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Mxmx0intmacMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Mxmx0intmacMetrics) Size() int {
	sz := 0

	sz += mtr.Lane0Sbe.Size()

	sz += mtr.Lane0Dbe.Size()

	sz += mtr.Lane1Sbe.Size()

	sz += mtr.Lane1Dbe.Size()

	sz += mtr.Lane2Sbe.Size()

	sz += mtr.Lane2Dbe.Size()

	sz += mtr.Lane3Sbe.Size()

	sz += mtr.Lane3Dbe.Size()

	sz += mtr.M0PbPbusDrdy.Size()

	sz += mtr.M1PbPbusDrdy.Size()

	sz += mtr.M2PbPbusDrdy.Size()

	sz += mtr.M3PbPbusDrdy.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Mxmx0intmacMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Lane0Sbe = mtr.metrics.GetCounter(offset)
	offset += mtr.Lane0Sbe.Size()

	mtr.Lane0Dbe = mtr.metrics.GetCounter(offset)
	offset += mtr.Lane0Dbe.Size()

	mtr.Lane1Sbe = mtr.metrics.GetCounter(offset)
	offset += mtr.Lane1Sbe.Size()

	mtr.Lane1Dbe = mtr.metrics.GetCounter(offset)
	offset += mtr.Lane1Dbe.Size()

	mtr.Lane2Sbe = mtr.metrics.GetCounter(offset)
	offset += mtr.Lane2Sbe.Size()

	mtr.Lane2Dbe = mtr.metrics.GetCounter(offset)
	offset += mtr.Lane2Dbe.Size()

	mtr.Lane3Sbe = mtr.metrics.GetCounter(offset)
	offset += mtr.Lane3Sbe.Size()

	mtr.Lane3Dbe = mtr.metrics.GetCounter(offset)
	offset += mtr.Lane3Dbe.Size()

	mtr.M0PbPbusDrdy = mtr.metrics.GetCounter(offset)
	offset += mtr.M0PbPbusDrdy.Size()

	mtr.M1PbPbusDrdy = mtr.metrics.GetCounter(offset)
	offset += mtr.M1PbPbusDrdy.Size()

	mtr.M2PbPbusDrdy = mtr.metrics.GetCounter(offset)
	offset += mtr.M2PbPbusDrdy.Size()

	mtr.M3PbPbusDrdy = mtr.metrics.GetCounter(offset)
	offset += mtr.M3PbPbusDrdy.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Mxmx0intmacMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Lane0Sbe" {
		return offset
	}
	offset += mtr.Lane0Sbe.Size()

	if fldName == "Lane0Dbe" {
		return offset
	}
	offset += mtr.Lane0Dbe.Size()

	if fldName == "Lane1Sbe" {
		return offset
	}
	offset += mtr.Lane1Sbe.Size()

	if fldName == "Lane1Dbe" {
		return offset
	}
	offset += mtr.Lane1Dbe.Size()

	if fldName == "Lane2Sbe" {
		return offset
	}
	offset += mtr.Lane2Sbe.Size()

	if fldName == "Lane2Dbe" {
		return offset
	}
	offset += mtr.Lane2Dbe.Size()

	if fldName == "Lane3Sbe" {
		return offset
	}
	offset += mtr.Lane3Sbe.Size()

	if fldName == "Lane3Dbe" {
		return offset
	}
	offset += mtr.Lane3Dbe.Size()

	if fldName == "M0PbPbusDrdy" {
		return offset
	}
	offset += mtr.M0PbPbusDrdy.Size()

	if fldName == "M1PbPbusDrdy" {
		return offset
	}
	offset += mtr.M1PbPbusDrdy.Size()

	if fldName == "M2PbPbusDrdy" {
		return offset
	}
	offset += mtr.M2PbPbusDrdy.Size()

	if fldName == "M3PbPbusDrdy" {
		return offset
	}
	offset += mtr.M3PbPbusDrdy.Size()

	return offset
}

// SetLane0Sbe sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetLane0Sbe(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Lane0Sbe"))
	return nil
}

// SetLane0Dbe sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetLane0Dbe(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Lane0Dbe"))
	return nil
}

// SetLane1Sbe sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetLane1Sbe(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Lane1Sbe"))
	return nil
}

// SetLane1Dbe sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetLane1Dbe(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Lane1Dbe"))
	return nil
}

// SetLane2Sbe sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetLane2Sbe(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Lane2Sbe"))
	return nil
}

// SetLane2Dbe sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetLane2Dbe(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Lane2Dbe"))
	return nil
}

// SetLane3Sbe sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetLane3Sbe(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Lane3Sbe"))
	return nil
}

// SetLane3Dbe sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetLane3Dbe(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Lane3Dbe"))
	return nil
}

// SetM0PbPbusDrdy sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetM0PbPbusDrdy(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("M0PbPbusDrdy"))
	return nil
}

// SetM1PbPbusDrdy sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetM1PbPbusDrdy(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("M1PbPbusDrdy"))
	return nil
}

// SetM2PbPbusDrdy sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetM2PbPbusDrdy(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("M2PbPbusDrdy"))
	return nil
}

// SetM3PbPbusDrdy sets cunter in shared memory
func (mtr *Mxmx0intmacMetrics) SetM3PbPbusDrdy(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("M3PbPbusDrdy"))
	return nil
}

// Mxmx0intmacMetricsIterator is the iterator object
type Mxmx0intmacMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Mxmx0intmacMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Mxmx0intmacMetricsIterator) Next() *Mxmx0intmacMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Mxmx0intmacMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Mxmx0intmacMetricsIterator) Find(key uint64) (*Mxmx0intmacMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Mxmx0intmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Mxmx0intmacMetricsIterator) Create(key uint64) (*Mxmx0intmacMetrics, error) {
	tmtr := &Mxmx0intmacMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Mxmx0intmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Mxmx0intmacMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Mxmx0intmacMetricsIterator) Free() {
	it.iter.Free()
}

// NewMxmx0intmacMetricsIterator returns an iterator
func NewMxmx0intmacMetricsIterator() (*Mxmx0intmacMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Mxmx0intmacMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Mxmx0intmacMetricsIterator{iter: iter}, nil
}

type Mxmx0inteccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Mxmx0inteccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Mxmx0inteccMetrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Mxmx0inteccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Mxmx0inteccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Uncorrectable" {
		return offset
	}
	offset += mtr.Uncorrectable.Size()

	if fldName == "Correctable" {
		return offset
	}
	offset += mtr.Correctable.Size()

	return offset
}

// SetUncorrectable sets cunter in shared memory
func (mtr *Mxmx0inteccMetrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *Mxmx0inteccMetrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// Mxmx0inteccMetricsIterator is the iterator object
type Mxmx0inteccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Mxmx0inteccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Mxmx0inteccMetricsIterator) Next() *Mxmx0inteccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Mxmx0inteccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Mxmx0inteccMetricsIterator) Find(key uint64) (*Mxmx0inteccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Mxmx0inteccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Mxmx0inteccMetricsIterator) Create(key uint64) (*Mxmx0inteccMetrics, error) {
	tmtr := &Mxmx0inteccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Mxmx0inteccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Mxmx0inteccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Mxmx0inteccMetricsIterator) Free() {
	it.iter.Free()
}

// NewMxmx0inteccMetricsIterator returns an iterator
func NewMxmx0inteccMetricsIterator() (*Mxmx0inteccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Mxmx0inteccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Mxmx0inteccMetricsIterator{iter: iter}, nil
}

type Mxmx1intmacMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Lane0Sbe metrics.Counter

	Lane0Dbe metrics.Counter

	Lane1Sbe metrics.Counter

	Lane1Dbe metrics.Counter

	Lane2Sbe metrics.Counter

	Lane2Dbe metrics.Counter

	Lane3Sbe metrics.Counter

	Lane3Dbe metrics.Counter

	M0PbPbusDrdy metrics.Counter

	M1PbPbusDrdy metrics.Counter

	M2PbPbusDrdy metrics.Counter

	M3PbPbusDrdy metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Mxmx1intmacMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Mxmx1intmacMetrics) Size() int {
	sz := 0

	sz += mtr.Lane0Sbe.Size()

	sz += mtr.Lane0Dbe.Size()

	sz += mtr.Lane1Sbe.Size()

	sz += mtr.Lane1Dbe.Size()

	sz += mtr.Lane2Sbe.Size()

	sz += mtr.Lane2Dbe.Size()

	sz += mtr.Lane3Sbe.Size()

	sz += mtr.Lane3Dbe.Size()

	sz += mtr.M0PbPbusDrdy.Size()

	sz += mtr.M1PbPbusDrdy.Size()

	sz += mtr.M2PbPbusDrdy.Size()

	sz += mtr.M3PbPbusDrdy.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Mxmx1intmacMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Lane0Sbe = mtr.metrics.GetCounter(offset)
	offset += mtr.Lane0Sbe.Size()

	mtr.Lane0Dbe = mtr.metrics.GetCounter(offset)
	offset += mtr.Lane0Dbe.Size()

	mtr.Lane1Sbe = mtr.metrics.GetCounter(offset)
	offset += mtr.Lane1Sbe.Size()

	mtr.Lane1Dbe = mtr.metrics.GetCounter(offset)
	offset += mtr.Lane1Dbe.Size()

	mtr.Lane2Sbe = mtr.metrics.GetCounter(offset)
	offset += mtr.Lane2Sbe.Size()

	mtr.Lane2Dbe = mtr.metrics.GetCounter(offset)
	offset += mtr.Lane2Dbe.Size()

	mtr.Lane3Sbe = mtr.metrics.GetCounter(offset)
	offset += mtr.Lane3Sbe.Size()

	mtr.Lane3Dbe = mtr.metrics.GetCounter(offset)
	offset += mtr.Lane3Dbe.Size()

	mtr.M0PbPbusDrdy = mtr.metrics.GetCounter(offset)
	offset += mtr.M0PbPbusDrdy.Size()

	mtr.M1PbPbusDrdy = mtr.metrics.GetCounter(offset)
	offset += mtr.M1PbPbusDrdy.Size()

	mtr.M2PbPbusDrdy = mtr.metrics.GetCounter(offset)
	offset += mtr.M2PbPbusDrdy.Size()

	mtr.M3PbPbusDrdy = mtr.metrics.GetCounter(offset)
	offset += mtr.M3PbPbusDrdy.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Mxmx1intmacMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Lane0Sbe" {
		return offset
	}
	offset += mtr.Lane0Sbe.Size()

	if fldName == "Lane0Dbe" {
		return offset
	}
	offset += mtr.Lane0Dbe.Size()

	if fldName == "Lane1Sbe" {
		return offset
	}
	offset += mtr.Lane1Sbe.Size()

	if fldName == "Lane1Dbe" {
		return offset
	}
	offset += mtr.Lane1Dbe.Size()

	if fldName == "Lane2Sbe" {
		return offset
	}
	offset += mtr.Lane2Sbe.Size()

	if fldName == "Lane2Dbe" {
		return offset
	}
	offset += mtr.Lane2Dbe.Size()

	if fldName == "Lane3Sbe" {
		return offset
	}
	offset += mtr.Lane3Sbe.Size()

	if fldName == "Lane3Dbe" {
		return offset
	}
	offset += mtr.Lane3Dbe.Size()

	if fldName == "M0PbPbusDrdy" {
		return offset
	}
	offset += mtr.M0PbPbusDrdy.Size()

	if fldName == "M1PbPbusDrdy" {
		return offset
	}
	offset += mtr.M1PbPbusDrdy.Size()

	if fldName == "M2PbPbusDrdy" {
		return offset
	}
	offset += mtr.M2PbPbusDrdy.Size()

	if fldName == "M3PbPbusDrdy" {
		return offset
	}
	offset += mtr.M3PbPbusDrdy.Size()

	return offset
}

// SetLane0Sbe sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetLane0Sbe(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Lane0Sbe"))
	return nil
}

// SetLane0Dbe sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetLane0Dbe(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Lane0Dbe"))
	return nil
}

// SetLane1Sbe sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetLane1Sbe(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Lane1Sbe"))
	return nil
}

// SetLane1Dbe sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetLane1Dbe(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Lane1Dbe"))
	return nil
}

// SetLane2Sbe sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetLane2Sbe(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Lane2Sbe"))
	return nil
}

// SetLane2Dbe sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetLane2Dbe(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Lane2Dbe"))
	return nil
}

// SetLane3Sbe sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetLane3Sbe(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Lane3Sbe"))
	return nil
}

// SetLane3Dbe sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetLane3Dbe(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Lane3Dbe"))
	return nil
}

// SetM0PbPbusDrdy sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetM0PbPbusDrdy(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("M0PbPbusDrdy"))
	return nil
}

// SetM1PbPbusDrdy sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetM1PbPbusDrdy(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("M1PbPbusDrdy"))
	return nil
}

// SetM2PbPbusDrdy sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetM2PbPbusDrdy(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("M2PbPbusDrdy"))
	return nil
}

// SetM3PbPbusDrdy sets cunter in shared memory
func (mtr *Mxmx1intmacMetrics) SetM3PbPbusDrdy(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("M3PbPbusDrdy"))
	return nil
}

// Mxmx1intmacMetricsIterator is the iterator object
type Mxmx1intmacMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Mxmx1intmacMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Mxmx1intmacMetricsIterator) Next() *Mxmx1intmacMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Mxmx1intmacMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Mxmx1intmacMetricsIterator) Find(key uint64) (*Mxmx1intmacMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Mxmx1intmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Mxmx1intmacMetricsIterator) Create(key uint64) (*Mxmx1intmacMetrics, error) {
	tmtr := &Mxmx1intmacMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Mxmx1intmacMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Mxmx1intmacMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Mxmx1intmacMetricsIterator) Free() {
	it.iter.Free()
}

// NewMxmx1intmacMetricsIterator returns an iterator
func NewMxmx1intmacMetricsIterator() (*Mxmx1intmacMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Mxmx1intmacMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Mxmx1intmacMetricsIterator{iter: iter}, nil
}

type Mxmx1inteccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable metrics.Counter

	Correctable metrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Mxmx1inteccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Mxmx1inteccMetrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Mxmx1inteccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Mxmx1inteccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Uncorrectable" {
		return offset
	}
	offset += mtr.Uncorrectable.Size()

	if fldName == "Correctable" {
		return offset
	}
	offset += mtr.Correctable.Size()

	return offset
}

// SetUncorrectable sets cunter in shared memory
func (mtr *Mxmx1inteccMetrics) SetUncorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *Mxmx1inteccMetrics) SetCorrectable(val metrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// Mxmx1inteccMetricsIterator is the iterator object
type Mxmx1inteccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Mxmx1inteccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Mxmx1inteccMetricsIterator) Next() *Mxmx1inteccMetrics {
	mtr := it.iter.Next()
	if mtr == nil {
		return nil
	}

	tmtr := &Mxmx1inteccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Mxmx1inteccMetricsIterator) Find(key uint64) (*Mxmx1inteccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Mxmx1inteccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Mxmx1inteccMetricsIterator) Create(key uint64) (*Mxmx1inteccMetrics, error) {
	tmtr := &Mxmx1inteccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Mxmx1inteccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Mxmx1inteccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// Free frees the iterator memory
func (it *Mxmx1inteccMetricsIterator) Free() {
	it.iter.Free()
}

// NewMxmx1inteccMetricsIterator returns an iterator
func NewMxmx1inteccMetricsIterator() (*Mxmx1inteccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Mxmx1inteccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Mxmx1inteccMetricsIterator{iter: iter}, nil
}
