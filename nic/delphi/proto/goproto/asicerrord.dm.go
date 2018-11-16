// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package goproto

import (
	"github.com/pensando/sw/api"
	"github.com/pensando/sw/nic/delphi/gosdk/gometrics"
)

type DppintcreditMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PtrCreditOvflow gometrics.Counter

	PtrCreditUndflow gometrics.Counter

	PktCreditOvflow gometrics.Counter

	PktCreditUndflow gometrics.Counter

	FramerCreditOvflow gometrics.Counter

	FramerCreditUndflow gometrics.Counter

	FramerHdrfldVldOvfl gometrics.Counter

	FramerHdrfldOffsetOvfl gometrics.Counter

	ErrFramerHdrsizeZeroOvfl gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *DppintcreditMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *DppintcreditMetrics) Size() int {
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
func (mtr *DppintcreditMetrics) Unmarshal() error {
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
func (mtr *DppintcreditMetrics) getOffset(fldName string) int {
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
func (mtr *DppintcreditMetrics) SetPtrCreditOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PtrCreditOvflow"))
	return nil
}

// SetPtrCreditUndflow sets cunter in shared memory
func (mtr *DppintcreditMetrics) SetPtrCreditUndflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PtrCreditUndflow"))
	return nil
}

// SetPktCreditOvflow sets cunter in shared memory
func (mtr *DppintcreditMetrics) SetPktCreditOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktCreditOvflow"))
	return nil
}

// SetPktCreditUndflow sets cunter in shared memory
func (mtr *DppintcreditMetrics) SetPktCreditUndflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktCreditUndflow"))
	return nil
}

// SetFramerCreditOvflow sets cunter in shared memory
func (mtr *DppintcreditMetrics) SetFramerCreditOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramerCreditOvflow"))
	return nil
}

// SetFramerCreditUndflow sets cunter in shared memory
func (mtr *DppintcreditMetrics) SetFramerCreditUndflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramerCreditUndflow"))
	return nil
}

// SetFramerHdrfldVldOvfl sets cunter in shared memory
func (mtr *DppintcreditMetrics) SetFramerHdrfldVldOvfl(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramerHdrfldVldOvfl"))
	return nil
}

// SetFramerHdrfldOffsetOvfl sets cunter in shared memory
func (mtr *DppintcreditMetrics) SetFramerHdrfldOffsetOvfl(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FramerHdrfldOffsetOvfl"))
	return nil
}

// SetErrFramerHdrsizeZeroOvfl sets cunter in shared memory
func (mtr *DppintcreditMetrics) SetErrFramerHdrsizeZeroOvfl(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrFramerHdrsizeZeroOvfl"))
	return nil
}

// DppintcreditMetricsIterator is the iterator object
type DppintcreditMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *DppintcreditMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *DppintcreditMetricsIterator) Next() *DppintcreditMetrics {
	mtr := it.iter.Next()
	tmtr := &DppintcreditMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *DppintcreditMetricsIterator) Find(key uint64) (*DppintcreditMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &DppintcreditMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *DppintcreditMetricsIterator) Create(key uint64) (*DppintcreditMetrics, error) {
	tmtr := &DppintcreditMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &DppintcreditMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *DppintcreditMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewDppintcreditMetricsIterator returns an iterator
func NewDppintcreditMetricsIterator() (*DppintcreditMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("DppintcreditMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &DppintcreditMetricsIterator{iter: iter}, nil
}

type DppintfifoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PhvFfOverflow gometrics.Counter

	OhiFfOverflow gometrics.Counter

	PktSizeFfOvflow gometrics.Counter

	PktSizeFfUndflow gometrics.Counter

	CsumPhvFfOvflow gometrics.Counter

	CsumPhvFfUndflow gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *DppintfifoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *DppintfifoMetrics) Size() int {
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
func (mtr *DppintfifoMetrics) Unmarshal() error {
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
func (mtr *DppintfifoMetrics) getOffset(fldName string) int {
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
func (mtr *DppintfifoMetrics) SetPhvFfOverflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvFfOverflow"))
	return nil
}

// SetOhiFfOverflow sets cunter in shared memory
func (mtr *DppintfifoMetrics) SetOhiFfOverflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OhiFfOverflow"))
	return nil
}

// SetPktSizeFfOvflow sets cunter in shared memory
func (mtr *DppintfifoMetrics) SetPktSizeFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeFfOvflow"))
	return nil
}

// SetPktSizeFfUndflow sets cunter in shared memory
func (mtr *DppintfifoMetrics) SetPktSizeFfUndflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktSizeFfUndflow"))
	return nil
}

// SetCsumPhvFfOvflow sets cunter in shared memory
func (mtr *DppintfifoMetrics) SetCsumPhvFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CsumPhvFfOvflow"))
	return nil
}

// SetCsumPhvFfUndflow sets cunter in shared memory
func (mtr *DppintfifoMetrics) SetCsumPhvFfUndflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CsumPhvFfUndflow"))
	return nil
}

// DppintfifoMetricsIterator is the iterator object
type DppintfifoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *DppintfifoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *DppintfifoMetricsIterator) Next() *DppintfifoMetrics {
	mtr := it.iter.Next()
	tmtr := &DppintfifoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *DppintfifoMetricsIterator) Find(key uint64) (*DppintfifoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &DppintfifoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *DppintfifoMetricsIterator) Create(key uint64) (*DppintfifoMetrics, error) {
	tmtr := &DppintfifoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &DppintfifoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *DppintfifoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewDppintfifoMetricsIterator returns an iterator
func NewDppintfifoMetricsIterator() (*DppintfifoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("DppintfifoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &DppintfifoMetricsIterator{iter: iter}, nil
}

type Dppintreg1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	ErrPhvSopNoEop gometrics.Counter

	ErrPhvEopNoSop gometrics.Counter

	ErrOhiSopNoEop gometrics.Counter

	ErrOhiEopNoSop gometrics.Counter

	ErrFramerCreditOverrun gometrics.Counter

	ErrPacketsInFlightCreditOverrun gometrics.Counter

	ErrNullHdrVld gometrics.Counter

	ErrNullHdrfldVld gometrics.Counter

	ErrMaxPktSize gometrics.Counter

	ErrMaxActiveHdrs gometrics.Counter

	ErrPhvNoDataReferenceOhi gometrics.Counter

	ErrCsumMultipleHdr gometrics.Counter

	ErrCsumMultipleHdrCopy gometrics.Counter

	ErrCrcMultipleHdr gometrics.Counter

	ErrPtrFifoCreditOverrun gometrics.Counter

	ErrClipMaxPktSize gometrics.Counter

	ErrMinPktSize gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dppintreg1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dppintreg1Metrics) Size() int {
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
func (mtr *Dppintreg1Metrics) Unmarshal() error {
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
func (mtr *Dppintreg1Metrics) getOffset(fldName string) int {
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
func (mtr *Dppintreg1Metrics) SetErrPhvSopNoEop(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPhvSopNoEop"))
	return nil
}

// SetErrPhvEopNoSop sets cunter in shared memory
func (mtr *Dppintreg1Metrics) SetErrPhvEopNoSop(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPhvEopNoSop"))
	return nil
}

// SetErrOhiSopNoEop sets cunter in shared memory
func (mtr *Dppintreg1Metrics) SetErrOhiSopNoEop(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrOhiSopNoEop"))
	return nil
}

// SetErrOhiEopNoSop sets cunter in shared memory
func (mtr *Dppintreg1Metrics) SetErrOhiEopNoSop(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrOhiEopNoSop"))
	return nil
}

// SetErrFramerCreditOverrun sets cunter in shared memory
func (mtr *Dppintreg1Metrics) SetErrFramerCreditOverrun(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrFramerCreditOverrun"))
	return nil
}

// SetErrPacketsInFlightCreditOverrun sets cunter in shared memory
func (mtr *Dppintreg1Metrics) SetErrPacketsInFlightCreditOverrun(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPacketsInFlightCreditOverrun"))
	return nil
}

// SetErrNullHdrVld sets cunter in shared memory
func (mtr *Dppintreg1Metrics) SetErrNullHdrVld(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrNullHdrVld"))
	return nil
}

// SetErrNullHdrfldVld sets cunter in shared memory
func (mtr *Dppintreg1Metrics) SetErrNullHdrfldVld(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrNullHdrfldVld"))
	return nil
}

// SetErrMaxPktSize sets cunter in shared memory
func (mtr *Dppintreg1Metrics) SetErrMaxPktSize(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrMaxPktSize"))
	return nil
}

// SetErrMaxActiveHdrs sets cunter in shared memory
func (mtr *Dppintreg1Metrics) SetErrMaxActiveHdrs(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrMaxActiveHdrs"))
	return nil
}

// SetErrPhvNoDataReferenceOhi sets cunter in shared memory
func (mtr *Dppintreg1Metrics) SetErrPhvNoDataReferenceOhi(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPhvNoDataReferenceOhi"))
	return nil
}

// SetErrCsumMultipleHdr sets cunter in shared memory
func (mtr *Dppintreg1Metrics) SetErrCsumMultipleHdr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumMultipleHdr"))
	return nil
}

// SetErrCsumMultipleHdrCopy sets cunter in shared memory
func (mtr *Dppintreg1Metrics) SetErrCsumMultipleHdrCopy(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumMultipleHdrCopy"))
	return nil
}

// SetErrCrcMultipleHdr sets cunter in shared memory
func (mtr *Dppintreg1Metrics) SetErrCrcMultipleHdr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCrcMultipleHdr"))
	return nil
}

// SetErrPtrFifoCreditOverrun sets cunter in shared memory
func (mtr *Dppintreg1Metrics) SetErrPtrFifoCreditOverrun(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPtrFifoCreditOverrun"))
	return nil
}

// SetErrClipMaxPktSize sets cunter in shared memory
func (mtr *Dppintreg1Metrics) SetErrClipMaxPktSize(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrClipMaxPktSize"))
	return nil
}

// SetErrMinPktSize sets cunter in shared memory
func (mtr *Dppintreg1Metrics) SetErrMinPktSize(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrMinPktSize"))
	return nil
}

// Dppintreg1MetricsIterator is the iterator object
type Dppintreg1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dppintreg1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dppintreg1MetricsIterator) Next() *Dppintreg1Metrics {
	mtr := it.iter.Next()
	tmtr := &Dppintreg1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dppintreg1MetricsIterator) Find(key uint64) (*Dppintreg1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dppintreg1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dppintreg1MetricsIterator) Create(key uint64) (*Dppintreg1Metrics, error) {
	tmtr := &Dppintreg1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dppintreg1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dppintreg1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewDppintreg1MetricsIterator returns an iterator
func NewDppintreg1MetricsIterator() (*Dppintreg1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dppintreg1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dppintreg1MetricsIterator{iter: iter}, nil
}

type Dppintreg2Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	FieldC gometrics.Counter

	FieldD gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dppintreg2Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dppintreg2Metrics) Size() int {
	sz := 0

	sz += mtr.FieldC.Size()

	sz += mtr.FieldD.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dppintreg2Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.FieldC = mtr.metrics.GetCounter(offset)
	offset += mtr.FieldC.Size()

	mtr.FieldD = mtr.metrics.GetCounter(offset)
	offset += mtr.FieldD.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dppintreg2Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "FieldC" {
		return offset
	}
	offset += mtr.FieldC.Size()

	if fldName == "FieldD" {
		return offset
	}
	offset += mtr.FieldD.Size()

	return offset
}

// SetFieldC sets cunter in shared memory
func (mtr *Dppintreg2Metrics) SetFieldC(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FieldC"))
	return nil
}

// SetFieldD sets cunter in shared memory
func (mtr *Dppintreg2Metrics) SetFieldD(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FieldD"))
	return nil
}

// Dppintreg2MetricsIterator is the iterator object
type Dppintreg2MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dppintreg2MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dppintreg2MetricsIterator) Next() *Dppintreg2Metrics {
	mtr := it.iter.Next()
	tmtr := &Dppintreg2Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dppintreg2MetricsIterator) Find(key uint64) (*Dppintreg2Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dppintreg2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dppintreg2MetricsIterator) Create(key uint64) (*Dppintreg2Metrics, error) {
	tmtr := &Dppintreg2Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dppintreg2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dppintreg2MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewDppintreg2MetricsIterator returns an iterator
func NewDppintreg2MetricsIterator() (*Dppintreg2MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dppintreg2Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dppintreg2MetricsIterator{iter: iter}, nil
}

type IntspareMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Spare_0 gometrics.Counter

	Spare_1 gometrics.Counter

	Spare_2 gometrics.Counter

	Spare_3 gometrics.Counter

	Spare_4 gometrics.Counter

	Spare_5 gometrics.Counter

	Spare_6 gometrics.Counter

	Spare_7 gometrics.Counter

	Spare_8 gometrics.Counter

	Spare_9 gometrics.Counter

	Spare_10 gometrics.Counter

	Spare_11 gometrics.Counter

	Spare_12 gometrics.Counter

	Spare_13 gometrics.Counter

	Spare_14 gometrics.Counter

	Spare_15 gometrics.Counter

	Spare_16 gometrics.Counter

	Spare_17 gometrics.Counter

	Spare_18 gometrics.Counter

	Spare_19 gometrics.Counter

	Spare_20 gometrics.Counter

	Spare_21 gometrics.Counter

	Spare_22 gometrics.Counter

	Spare_23 gometrics.Counter

	Spare_24 gometrics.Counter

	Spare_25 gometrics.Counter

	Spare_26 gometrics.Counter

	Spare_27 gometrics.Counter

	Spare_28 gometrics.Counter

	Spare_29 gometrics.Counter

	Spare_30 gometrics.Counter

	Spare_31 gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *IntspareMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *IntspareMetrics) Size() int {
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
func (mtr *IntspareMetrics) Unmarshal() error {
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
func (mtr *IntspareMetrics) getOffset(fldName string) int {
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
func (mtr *IntspareMetrics) SetSpare_0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_0"))
	return nil
}

// SetSpare_1 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_1"))
	return nil
}

// SetSpare_2 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_2"))
	return nil
}

// SetSpare_3 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_3"))
	return nil
}

// SetSpare_4 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_4(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_4"))
	return nil
}

// SetSpare_5 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_5(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_5"))
	return nil
}

// SetSpare_6 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_6(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_6"))
	return nil
}

// SetSpare_7 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_7(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_7"))
	return nil
}

// SetSpare_8 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_8(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_8"))
	return nil
}

// SetSpare_9 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_9(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_9"))
	return nil
}

// SetSpare_10 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_10(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_10"))
	return nil
}

// SetSpare_11 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_11(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_11"))
	return nil
}

// SetSpare_12 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_12(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_12"))
	return nil
}

// SetSpare_13 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_13(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_13"))
	return nil
}

// SetSpare_14 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_14(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_14"))
	return nil
}

// SetSpare_15 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_15(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_15"))
	return nil
}

// SetSpare_16 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_16(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_16"))
	return nil
}

// SetSpare_17 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_17(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_17"))
	return nil
}

// SetSpare_18 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_18(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_18"))
	return nil
}

// SetSpare_19 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_19(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_19"))
	return nil
}

// SetSpare_20 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_20(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_20"))
	return nil
}

// SetSpare_21 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_21(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_21"))
	return nil
}

// SetSpare_22 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_22(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_22"))
	return nil
}

// SetSpare_23 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_23(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_23"))
	return nil
}

// SetSpare_24 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_24(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_24"))
	return nil
}

// SetSpare_25 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_25(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_25"))
	return nil
}

// SetSpare_26 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_26(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_26"))
	return nil
}

// SetSpare_27 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_27(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_27"))
	return nil
}

// SetSpare_28 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_28(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_28"))
	return nil
}

// SetSpare_29 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_29(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_29"))
	return nil
}

// SetSpare_30 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_30(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_30"))
	return nil
}

// SetSpare_31 sets cunter in shared memory
func (mtr *IntspareMetrics) SetSpare_31(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Spare_31"))
	return nil
}

// IntspareMetricsIterator is the iterator object
type IntspareMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *IntspareMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *IntspareMetricsIterator) Next() *IntspareMetrics {
	mtr := it.iter.Next()
	tmtr := &IntspareMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *IntspareMetricsIterator) Find(key uint64) (*IntspareMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &IntspareMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *IntspareMetricsIterator) Create(key uint64) (*IntspareMetrics, error) {
	tmtr := &IntspareMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &IntspareMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *IntspareMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewIntspareMetricsIterator returns an iterator
func NewIntspareMetricsIterator() (*IntspareMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("IntspareMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &IntspareMetricsIterator{iter: iter}, nil
}

type DppintsramseccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	DppPhvFifoUncorrectable gometrics.Counter

	DppPhvFifoCorrectable gometrics.Counter

	DppOhiFifoUncorrectable gometrics.Counter

	DppOhiFifoCorrectable gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *DppintsramseccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *DppintsramseccMetrics) Size() int {
	sz := 0

	sz += mtr.DppPhvFifoUncorrectable.Size()

	sz += mtr.DppPhvFifoCorrectable.Size()

	sz += mtr.DppOhiFifoUncorrectable.Size()

	sz += mtr.DppOhiFifoCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *DppintsramseccMetrics) Unmarshal() error {
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
func (mtr *DppintsramseccMetrics) getOffset(fldName string) int {
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
func (mtr *DppintsramseccMetrics) SetDppPhvFifoUncorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DppPhvFifoUncorrectable"))
	return nil
}

// SetDppPhvFifoCorrectable sets cunter in shared memory
func (mtr *DppintsramseccMetrics) SetDppPhvFifoCorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DppPhvFifoCorrectable"))
	return nil
}

// SetDppOhiFifoUncorrectable sets cunter in shared memory
func (mtr *DppintsramseccMetrics) SetDppOhiFifoUncorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DppOhiFifoUncorrectable"))
	return nil
}

// SetDppOhiFifoCorrectable sets cunter in shared memory
func (mtr *DppintsramseccMetrics) SetDppOhiFifoCorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DppOhiFifoCorrectable"))
	return nil
}

// DppintsramseccMetricsIterator is the iterator object
type DppintsramseccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *DppintsramseccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *DppintsramseccMetricsIterator) Next() *DppintsramseccMetrics {
	mtr := it.iter.Next()
	tmtr := &DppintsramseccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *DppintsramseccMetricsIterator) Find(key uint64) (*DppintsramseccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &DppintsramseccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *DppintsramseccMetricsIterator) Create(key uint64) (*DppintsramseccMetrics, error) {
	tmtr := &DppintsramseccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &DppintsramseccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *DppintsramseccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewDppintsramseccMetricsIterator returns an iterator
func NewDppintsramseccMetricsIterator() (*DppintsramseccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("DppintsramseccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &DppintsramseccMetricsIterator{iter: iter}, nil
}

type DprintcreditMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	EgressCreditOvflow gometrics.Counter

	EgressCreditUndflow gometrics.Counter

	PktoutCreditOvflow gometrics.Counter

	PktoutCreditUndflow gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *DprintcreditMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *DprintcreditMetrics) Size() int {
	sz := 0

	sz += mtr.EgressCreditOvflow.Size()

	sz += mtr.EgressCreditUndflow.Size()

	sz += mtr.PktoutCreditOvflow.Size()

	sz += mtr.PktoutCreditUndflow.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *DprintcreditMetrics) Unmarshal() error {
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
func (mtr *DprintcreditMetrics) getOffset(fldName string) int {
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
func (mtr *DprintcreditMetrics) SetEgressCreditOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EgressCreditOvflow"))
	return nil
}

// SetEgressCreditUndflow sets cunter in shared memory
func (mtr *DprintcreditMetrics) SetEgressCreditUndflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EgressCreditUndflow"))
	return nil
}

// SetPktoutCreditOvflow sets cunter in shared memory
func (mtr *DprintcreditMetrics) SetPktoutCreditOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktoutCreditOvflow"))
	return nil
}

// SetPktoutCreditUndflow sets cunter in shared memory
func (mtr *DprintcreditMetrics) SetPktoutCreditUndflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktoutCreditUndflow"))
	return nil
}

// DprintcreditMetricsIterator is the iterator object
type DprintcreditMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *DprintcreditMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *DprintcreditMetricsIterator) Next() *DprintcreditMetrics {
	mtr := it.iter.Next()
	tmtr := &DprintcreditMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *DprintcreditMetricsIterator) Find(key uint64) (*DprintcreditMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &DprintcreditMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *DprintcreditMetricsIterator) Create(key uint64) (*DprintcreditMetrics, error) {
	tmtr := &DprintcreditMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &DprintcreditMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *DprintcreditMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewDprintcreditMetricsIterator returns an iterator
func NewDprintcreditMetricsIterator() (*DprintcreditMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("DprintcreditMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &DprintcreditMetricsIterator{iter: iter}, nil
}

type DprintfifoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	PhvFfOvflow gometrics.Counter

	OhiFfOvflow gometrics.Counter

	PktinFfOvflow gometrics.Counter

	PktoutFfUndflow gometrics.Counter

	CsumFfOvflow gometrics.Counter

	PtrFfOvflow gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *DprintfifoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *DprintfifoMetrics) Size() int {
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
func (mtr *DprintfifoMetrics) Unmarshal() error {
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
func (mtr *DprintfifoMetrics) getOffset(fldName string) int {
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
func (mtr *DprintfifoMetrics) SetPhvFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvFfOvflow"))
	return nil
}

// SetOhiFfOvflow sets cunter in shared memory
func (mtr *DprintfifoMetrics) SetOhiFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OhiFfOvflow"))
	return nil
}

// SetPktinFfOvflow sets cunter in shared memory
func (mtr *DprintfifoMetrics) SetPktinFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktinFfOvflow"))
	return nil
}

// SetPktoutFfUndflow sets cunter in shared memory
func (mtr *DprintfifoMetrics) SetPktoutFfUndflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktoutFfUndflow"))
	return nil
}

// SetCsumFfOvflow sets cunter in shared memory
func (mtr *DprintfifoMetrics) SetCsumFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CsumFfOvflow"))
	return nil
}

// SetPtrFfOvflow sets cunter in shared memory
func (mtr *DprintfifoMetrics) SetPtrFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PtrFfOvflow"))
	return nil
}

// DprintfifoMetricsIterator is the iterator object
type DprintfifoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *DprintfifoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *DprintfifoMetricsIterator) Next() *DprintfifoMetrics {
	mtr := it.iter.Next()
	tmtr := &DprintfifoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *DprintfifoMetricsIterator) Find(key uint64) (*DprintfifoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &DprintfifoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *DprintfifoMetricsIterator) Create(key uint64) (*DprintfifoMetrics, error) {
	tmtr := &DprintfifoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &DprintfifoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *DprintfifoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewDprintfifoMetricsIterator returns an iterator
func NewDprintfifoMetricsIterator() (*DprintfifoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("DprintfifoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &DprintfifoMetricsIterator{iter: iter}, nil
}

type DprintflopfifoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	DataMuxForceBypassCrcFlopFfOvflow gometrics.Counter

	DprCrcInfoFlopFfOvflow gometrics.Counter

	DprCrcUpdateInfoFlopFfOvflow gometrics.Counter

	DprCsumInfoFlopFfOvflow gometrics.Counter

	DataMuxForceBypassCsumFlopFfOvflow gometrics.Counter

	DprCsumUpdateInfoFlopFfOvflow gometrics.Counter

	PtrEarlyPktEopInfoFlopFfOvflow gometrics.Counter

	DataMuxEopErrFlopFfOvflow gometrics.Counter

	PktinEopErrFlopFfOvflow gometrics.Counter

	CsumErrFlopFfOvflow gometrics.Counter

	CrcErrFlopFfOvflow gometrics.Counter

	DataMuxDropFlopFfOvflow gometrics.Counter

	PhvPktDataFlopFfOvflow gometrics.Counter

	PktoutLenCellFlopFfOvflow gometrics.Counter

	PaddingSizeFlopFfOvflow gometrics.Counter

	PktinErrFlopFfOvflow gometrics.Counter

	PhvNoDataFlopFfOvflow gometrics.Counter

	PtrLookahaedFlopFfOvflow gometrics.Counter

	EopVldFlopFfOvflow gometrics.Counter

	CsumCalVldFlopFfOvflow gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *DprintflopfifoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *DprintflopfifoMetrics) Size() int {
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
func (mtr *DprintflopfifoMetrics) Unmarshal() error {
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
func (mtr *DprintflopfifoMetrics) getOffset(fldName string) int {
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
func (mtr *DprintflopfifoMetrics) SetDataMuxForceBypassCrcFlopFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DataMuxForceBypassCrcFlopFfOvflow"))
	return nil
}

// SetDprCrcInfoFlopFfOvflow sets cunter in shared memory
func (mtr *DprintflopfifoMetrics) SetDprCrcInfoFlopFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCrcInfoFlopFfOvflow"))
	return nil
}

// SetDprCrcUpdateInfoFlopFfOvflow sets cunter in shared memory
func (mtr *DprintflopfifoMetrics) SetDprCrcUpdateInfoFlopFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCrcUpdateInfoFlopFfOvflow"))
	return nil
}

// SetDprCsumInfoFlopFfOvflow sets cunter in shared memory
func (mtr *DprintflopfifoMetrics) SetDprCsumInfoFlopFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCsumInfoFlopFfOvflow"))
	return nil
}

// SetDataMuxForceBypassCsumFlopFfOvflow sets cunter in shared memory
func (mtr *DprintflopfifoMetrics) SetDataMuxForceBypassCsumFlopFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DataMuxForceBypassCsumFlopFfOvflow"))
	return nil
}

// SetDprCsumUpdateInfoFlopFfOvflow sets cunter in shared memory
func (mtr *DprintflopfifoMetrics) SetDprCsumUpdateInfoFlopFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCsumUpdateInfoFlopFfOvflow"))
	return nil
}

// SetPtrEarlyPktEopInfoFlopFfOvflow sets cunter in shared memory
func (mtr *DprintflopfifoMetrics) SetPtrEarlyPktEopInfoFlopFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PtrEarlyPktEopInfoFlopFfOvflow"))
	return nil
}

// SetDataMuxEopErrFlopFfOvflow sets cunter in shared memory
func (mtr *DprintflopfifoMetrics) SetDataMuxEopErrFlopFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DataMuxEopErrFlopFfOvflow"))
	return nil
}

// SetPktinEopErrFlopFfOvflow sets cunter in shared memory
func (mtr *DprintflopfifoMetrics) SetPktinEopErrFlopFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktinEopErrFlopFfOvflow"))
	return nil
}

// SetCsumErrFlopFfOvflow sets cunter in shared memory
func (mtr *DprintflopfifoMetrics) SetCsumErrFlopFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CsumErrFlopFfOvflow"))
	return nil
}

// SetCrcErrFlopFfOvflow sets cunter in shared memory
func (mtr *DprintflopfifoMetrics) SetCrcErrFlopFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CrcErrFlopFfOvflow"))
	return nil
}

// SetDataMuxDropFlopFfOvflow sets cunter in shared memory
func (mtr *DprintflopfifoMetrics) SetDataMuxDropFlopFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DataMuxDropFlopFfOvflow"))
	return nil
}

// SetPhvPktDataFlopFfOvflow sets cunter in shared memory
func (mtr *DprintflopfifoMetrics) SetPhvPktDataFlopFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvPktDataFlopFfOvflow"))
	return nil
}

// SetPktoutLenCellFlopFfOvflow sets cunter in shared memory
func (mtr *DprintflopfifoMetrics) SetPktoutLenCellFlopFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktoutLenCellFlopFfOvflow"))
	return nil
}

// SetPaddingSizeFlopFfOvflow sets cunter in shared memory
func (mtr *DprintflopfifoMetrics) SetPaddingSizeFlopFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PaddingSizeFlopFfOvflow"))
	return nil
}

// SetPktinErrFlopFfOvflow sets cunter in shared memory
func (mtr *DprintflopfifoMetrics) SetPktinErrFlopFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PktinErrFlopFfOvflow"))
	return nil
}

// SetPhvNoDataFlopFfOvflow sets cunter in shared memory
func (mtr *DprintflopfifoMetrics) SetPhvNoDataFlopFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvNoDataFlopFfOvflow"))
	return nil
}

// SetPtrLookahaedFlopFfOvflow sets cunter in shared memory
func (mtr *DprintflopfifoMetrics) SetPtrLookahaedFlopFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PtrLookahaedFlopFfOvflow"))
	return nil
}

// SetEopVldFlopFfOvflow sets cunter in shared memory
func (mtr *DprintflopfifoMetrics) SetEopVldFlopFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopVldFlopFfOvflow"))
	return nil
}

// SetCsumCalVldFlopFfOvflow sets cunter in shared memory
func (mtr *DprintflopfifoMetrics) SetCsumCalVldFlopFfOvflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CsumCalVldFlopFfOvflow"))
	return nil
}

// DprintflopfifoMetricsIterator is the iterator object
type DprintflopfifoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *DprintflopfifoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *DprintflopfifoMetricsIterator) Next() *DprintflopfifoMetrics {
	mtr := it.iter.Next()
	tmtr := &DprintflopfifoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *DprintflopfifoMetricsIterator) Find(key uint64) (*DprintflopfifoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &DprintflopfifoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *DprintflopfifoMetricsIterator) Create(key uint64) (*DprintflopfifoMetrics, error) {
	tmtr := &DprintflopfifoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &DprintflopfifoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *DprintflopfifoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewDprintflopfifoMetricsIterator returns an iterator
func NewDprintflopfifoMetricsIterator() (*DprintflopfifoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("DprintflopfifoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &DprintflopfifoMetricsIterator{iter: iter}, nil
}

type Dprintreg1Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	ErrPhvSopNoEop gometrics.Counter

	ErrPhvEopNoSop gometrics.Counter

	ErrOhiSopNoEop gometrics.Counter

	ErrOhiEopNoSop gometrics.Counter

	ErrPktinSopNoEop gometrics.Counter

	ErrPktinEopNoSop gometrics.Counter

	ErrCsumOffsetGtPktSize_4 gometrics.Counter

	ErrCsumOffsetGtPktSize_3 gometrics.Counter

	ErrCsumOffsetGtPktSize_2 gometrics.Counter

	ErrCsumOffsetGtPktSize_1 gometrics.Counter

	ErrCsumOffsetGtPktSize_0 gometrics.Counter

	ErrCsumPhdrOffsetGtPktSize_4 gometrics.Counter

	ErrCsumPhdrOffsetGtPktSize_3 gometrics.Counter

	ErrCsumPhdrOffsetGtPktSize_2 gometrics.Counter

	ErrCsumPhdrOffsetGtPktSize_1 gometrics.Counter

	ErrCsumPhdrOffsetGtPktSize_0 gometrics.Counter

	ErrCsumLocGtPktSize_4 gometrics.Counter

	ErrCsumLocGtPktSize_3 gometrics.Counter

	ErrCsumLocGtPktSize_2 gometrics.Counter

	ErrCsumLocGtPktSize_1 gometrics.Counter

	ErrCsumLocGtPktSize_0 gometrics.Counter

	ErrCrcOffsetGtPktSize gometrics.Counter

	ErrCrcLocGtPktSize gometrics.Counter

	ErrCrcMaskOffsetGtPktSize gometrics.Counter

	ErrPktEopEarly gometrics.Counter

	ErrPtrFfOverflow gometrics.Counter

	ErrCsumFfOverflow gometrics.Counter

	ErrPktoutFfOverflow gometrics.Counter

	ErrPtrFromCfgOverflow gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dprintreg1Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dprintreg1Metrics) Size() int {
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

	sz += mtr.ErrCrcMaskOffsetGtPktSize.Size()

	sz += mtr.ErrPktEopEarly.Size()

	sz += mtr.ErrPtrFfOverflow.Size()

	sz += mtr.ErrCsumFfOverflow.Size()

	sz += mtr.ErrPktoutFfOverflow.Size()

	sz += mtr.ErrPtrFromCfgOverflow.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dprintreg1Metrics) Unmarshal() error {
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

	mtr.ErrCrcMaskOffsetGtPktSize = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCrcMaskOffsetGtPktSize.Size()

	mtr.ErrPktEopEarly = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPktEopEarly.Size()

	mtr.ErrPtrFfOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPtrFfOverflow.Size()

	mtr.ErrCsumFfOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrCsumFfOverflow.Size()

	mtr.ErrPktoutFfOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPktoutFfOverflow.Size()

	mtr.ErrPtrFromCfgOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.ErrPtrFromCfgOverflow.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *Dprintreg1Metrics) getOffset(fldName string) int {
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

	if fldName == "ErrCrcMaskOffsetGtPktSize" {
		return offset
	}
	offset += mtr.ErrCrcMaskOffsetGtPktSize.Size()

	if fldName == "ErrPktEopEarly" {
		return offset
	}
	offset += mtr.ErrPktEopEarly.Size()

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

	if fldName == "ErrPtrFromCfgOverflow" {
		return offset
	}
	offset += mtr.ErrPtrFromCfgOverflow.Size()

	return offset
}

// SetErrPhvSopNoEop sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrPhvSopNoEop(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPhvSopNoEop"))
	return nil
}

// SetErrPhvEopNoSop sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrPhvEopNoSop(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPhvEopNoSop"))
	return nil
}

// SetErrOhiSopNoEop sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrOhiSopNoEop(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrOhiSopNoEop"))
	return nil
}

// SetErrOhiEopNoSop sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrOhiEopNoSop(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrOhiEopNoSop"))
	return nil
}

// SetErrPktinSopNoEop sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrPktinSopNoEop(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPktinSopNoEop"))
	return nil
}

// SetErrPktinEopNoSop sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrPktinEopNoSop(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPktinEopNoSop"))
	return nil
}

// SetErrCsumOffsetGtPktSize_4 sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrCsumOffsetGtPktSize_4(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumOffsetGtPktSize_4"))
	return nil
}

// SetErrCsumOffsetGtPktSize_3 sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrCsumOffsetGtPktSize_3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumOffsetGtPktSize_3"))
	return nil
}

// SetErrCsumOffsetGtPktSize_2 sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrCsumOffsetGtPktSize_2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumOffsetGtPktSize_2"))
	return nil
}

// SetErrCsumOffsetGtPktSize_1 sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrCsumOffsetGtPktSize_1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumOffsetGtPktSize_1"))
	return nil
}

// SetErrCsumOffsetGtPktSize_0 sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrCsumOffsetGtPktSize_0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumOffsetGtPktSize_0"))
	return nil
}

// SetErrCsumPhdrOffsetGtPktSize_4 sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrCsumPhdrOffsetGtPktSize_4(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumPhdrOffsetGtPktSize_4"))
	return nil
}

// SetErrCsumPhdrOffsetGtPktSize_3 sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrCsumPhdrOffsetGtPktSize_3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumPhdrOffsetGtPktSize_3"))
	return nil
}

// SetErrCsumPhdrOffsetGtPktSize_2 sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrCsumPhdrOffsetGtPktSize_2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumPhdrOffsetGtPktSize_2"))
	return nil
}

// SetErrCsumPhdrOffsetGtPktSize_1 sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrCsumPhdrOffsetGtPktSize_1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumPhdrOffsetGtPktSize_1"))
	return nil
}

// SetErrCsumPhdrOffsetGtPktSize_0 sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrCsumPhdrOffsetGtPktSize_0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumPhdrOffsetGtPktSize_0"))
	return nil
}

// SetErrCsumLocGtPktSize_4 sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrCsumLocGtPktSize_4(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumLocGtPktSize_4"))
	return nil
}

// SetErrCsumLocGtPktSize_3 sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrCsumLocGtPktSize_3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumLocGtPktSize_3"))
	return nil
}

// SetErrCsumLocGtPktSize_2 sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrCsumLocGtPktSize_2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumLocGtPktSize_2"))
	return nil
}

// SetErrCsumLocGtPktSize_1 sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrCsumLocGtPktSize_1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumLocGtPktSize_1"))
	return nil
}

// SetErrCsumLocGtPktSize_0 sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrCsumLocGtPktSize_0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumLocGtPktSize_0"))
	return nil
}

// SetErrCrcOffsetGtPktSize sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrCrcOffsetGtPktSize(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCrcOffsetGtPktSize"))
	return nil
}

// SetErrCrcLocGtPktSize sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrCrcLocGtPktSize(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCrcLocGtPktSize"))
	return nil
}

// SetErrCrcMaskOffsetGtPktSize sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrCrcMaskOffsetGtPktSize(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCrcMaskOffsetGtPktSize"))
	return nil
}

// SetErrPktEopEarly sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrPktEopEarly(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPktEopEarly"))
	return nil
}

// SetErrPtrFfOverflow sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrPtrFfOverflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPtrFfOverflow"))
	return nil
}

// SetErrCsumFfOverflow sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrCsumFfOverflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumFfOverflow"))
	return nil
}

// SetErrPktoutFfOverflow sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrPktoutFfOverflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPktoutFfOverflow"))
	return nil
}

// SetErrPtrFromCfgOverflow sets cunter in shared memory
func (mtr *Dprintreg1Metrics) SetErrPtrFromCfgOverflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrPtrFromCfgOverflow"))
	return nil
}

// Dprintreg1MetricsIterator is the iterator object
type Dprintreg1MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dprintreg1MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dprintreg1MetricsIterator) Next() *Dprintreg1Metrics {
	mtr := it.iter.Next()
	tmtr := &Dprintreg1Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dprintreg1MetricsIterator) Find(key uint64) (*Dprintreg1Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dprintreg1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dprintreg1MetricsIterator) Create(key uint64) (*Dprintreg1Metrics, error) {
	tmtr := &Dprintreg1Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dprintreg1Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dprintreg1MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewDprintreg1MetricsIterator returns an iterator
func NewDprintreg1MetricsIterator() (*Dprintreg1MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dprintreg1Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dprintreg1MetricsIterator{iter: iter}, nil
}

type Dprintreg2Metrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	FieldC gometrics.Counter

	FieldD gometrics.Counter

	ErrCsumStartGtEnd_4 gometrics.Counter

	ErrCsumStartGtEnd_3 gometrics.Counter

	ErrCsumStartGtEnd_2 gometrics.Counter

	ErrCsumStartGtEnd_1 gometrics.Counter

	ErrCsumStartGtEnd_0 gometrics.Counter

	ErrCrcStartGtEnd gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *Dprintreg2Metrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *Dprintreg2Metrics) Size() int {
	sz := 0

	sz += mtr.FieldC.Size()

	sz += mtr.FieldD.Size()

	sz += mtr.ErrCsumStartGtEnd_4.Size()

	sz += mtr.ErrCsumStartGtEnd_3.Size()

	sz += mtr.ErrCsumStartGtEnd_2.Size()

	sz += mtr.ErrCsumStartGtEnd_1.Size()

	sz += mtr.ErrCsumStartGtEnd_0.Size()

	sz += mtr.ErrCrcStartGtEnd.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *Dprintreg2Metrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.FieldC = mtr.metrics.GetCounter(offset)
	offset += mtr.FieldC.Size()

	mtr.FieldD = mtr.metrics.GetCounter(offset)
	offset += mtr.FieldD.Size()

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
func (mtr *Dprintreg2Metrics) getOffset(fldName string) int {
	var offset int

	if fldName == "FieldC" {
		return offset
	}
	offset += mtr.FieldC.Size()

	if fldName == "FieldD" {
		return offset
	}
	offset += mtr.FieldD.Size()

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

// SetFieldC sets cunter in shared memory
func (mtr *Dprintreg2Metrics) SetFieldC(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FieldC"))
	return nil
}

// SetFieldD sets cunter in shared memory
func (mtr *Dprintreg2Metrics) SetFieldD(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("FieldD"))
	return nil
}

// SetErrCsumStartGtEnd_4 sets cunter in shared memory
func (mtr *Dprintreg2Metrics) SetErrCsumStartGtEnd_4(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumStartGtEnd_4"))
	return nil
}

// SetErrCsumStartGtEnd_3 sets cunter in shared memory
func (mtr *Dprintreg2Metrics) SetErrCsumStartGtEnd_3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumStartGtEnd_3"))
	return nil
}

// SetErrCsumStartGtEnd_2 sets cunter in shared memory
func (mtr *Dprintreg2Metrics) SetErrCsumStartGtEnd_2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumStartGtEnd_2"))
	return nil
}

// SetErrCsumStartGtEnd_1 sets cunter in shared memory
func (mtr *Dprintreg2Metrics) SetErrCsumStartGtEnd_1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumStartGtEnd_1"))
	return nil
}

// SetErrCsumStartGtEnd_0 sets cunter in shared memory
func (mtr *Dprintreg2Metrics) SetErrCsumStartGtEnd_0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCsumStartGtEnd_0"))
	return nil
}

// SetErrCrcStartGtEnd sets cunter in shared memory
func (mtr *Dprintreg2Metrics) SetErrCrcStartGtEnd(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ErrCrcStartGtEnd"))
	return nil
}

// Dprintreg2MetricsIterator is the iterator object
type Dprintreg2MetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *Dprintreg2MetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *Dprintreg2MetricsIterator) Next() *Dprintreg2Metrics {
	mtr := it.iter.Next()
	tmtr := &Dprintreg2Metrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *Dprintreg2MetricsIterator) Find(key uint64) (*Dprintreg2Metrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &Dprintreg2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *Dprintreg2MetricsIterator) Create(key uint64) (*Dprintreg2Metrics, error) {
	tmtr := &Dprintreg2Metrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &Dprintreg2Metrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *Dprintreg2MetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewDprintreg2MetricsIterator returns an iterator
func NewDprintreg2MetricsIterator() (*Dprintreg2MetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("Dprintreg2Metrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &Dprintreg2MetricsIterator{iter: iter}, nil
}

type DprintsramseccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	DprPktinFifoUncorrectable gometrics.Counter

	DprPktinFifoCorrectable gometrics.Counter

	DprCsumFifoUncorrectable gometrics.Counter

	DprCsumFifoCorrectable gometrics.Counter

	DprPhvFifoUncorrectable gometrics.Counter

	DprPhvFifoCorrectable gometrics.Counter

	DprOhiFifoUncorrectable gometrics.Counter

	DprOhiFifoCorrectable gometrics.Counter

	DprPtrFifoUncorrectable gometrics.Counter

	DprPtrFifoCorrectable gometrics.Counter

	DprPktoutFifoUncorrectable gometrics.Counter

	DprPktoutFifoCorrectable gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *DprintsramseccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *DprintsramseccMetrics) Size() int {
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
func (mtr *DprintsramseccMetrics) Unmarshal() error {
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
func (mtr *DprintsramseccMetrics) getOffset(fldName string) int {
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
func (mtr *DprintsramseccMetrics) SetDprPktinFifoUncorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPktinFifoUncorrectable"))
	return nil
}

// SetDprPktinFifoCorrectable sets cunter in shared memory
func (mtr *DprintsramseccMetrics) SetDprPktinFifoCorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPktinFifoCorrectable"))
	return nil
}

// SetDprCsumFifoUncorrectable sets cunter in shared memory
func (mtr *DprintsramseccMetrics) SetDprCsumFifoUncorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCsumFifoUncorrectable"))
	return nil
}

// SetDprCsumFifoCorrectable sets cunter in shared memory
func (mtr *DprintsramseccMetrics) SetDprCsumFifoCorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprCsumFifoCorrectable"))
	return nil
}

// SetDprPhvFifoUncorrectable sets cunter in shared memory
func (mtr *DprintsramseccMetrics) SetDprPhvFifoUncorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPhvFifoUncorrectable"))
	return nil
}

// SetDprPhvFifoCorrectable sets cunter in shared memory
func (mtr *DprintsramseccMetrics) SetDprPhvFifoCorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPhvFifoCorrectable"))
	return nil
}

// SetDprOhiFifoUncorrectable sets cunter in shared memory
func (mtr *DprintsramseccMetrics) SetDprOhiFifoUncorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprOhiFifoUncorrectable"))
	return nil
}

// SetDprOhiFifoCorrectable sets cunter in shared memory
func (mtr *DprintsramseccMetrics) SetDprOhiFifoCorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprOhiFifoCorrectable"))
	return nil
}

// SetDprPtrFifoUncorrectable sets cunter in shared memory
func (mtr *DprintsramseccMetrics) SetDprPtrFifoUncorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPtrFifoUncorrectable"))
	return nil
}

// SetDprPtrFifoCorrectable sets cunter in shared memory
func (mtr *DprintsramseccMetrics) SetDprPtrFifoCorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPtrFifoCorrectable"))
	return nil
}

// SetDprPktoutFifoUncorrectable sets cunter in shared memory
func (mtr *DprintsramseccMetrics) SetDprPktoutFifoUncorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPktoutFifoUncorrectable"))
	return nil
}

// SetDprPktoutFifoCorrectable sets cunter in shared memory
func (mtr *DprintsramseccMetrics) SetDprPktoutFifoCorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DprPktoutFifoCorrectable"))
	return nil
}

// DprintsramseccMetricsIterator is the iterator object
type DprintsramseccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *DprintsramseccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *DprintsramseccMetricsIterator) Next() *DprintsramseccMetrics {
	mtr := it.iter.Next()
	tmtr := &DprintsramseccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *DprintsramseccMetricsIterator) Find(key uint64) (*DprintsramseccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &DprintsramseccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *DprintsramseccMetricsIterator) Create(key uint64) (*DprintsramseccMetrics, error) {
	tmtr := &DprintsramseccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &DprintsramseccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *DprintsramseccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewDprintsramseccMetricsIterator returns an iterator
func NewDprintsramseccMetricsIterator() (*DprintsramseccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("DprintsramseccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &DprintsramseccMetricsIterator{iter: iter}, nil
}

type SsepicsintbadaddrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Rdreq0BadAddr gometrics.Counter

	Wrreq0BadAddr gometrics.Counter

	Rdreq1BadAddr gometrics.Counter

	Wrreq1BadAddr gometrics.Counter

	Rdreq2BadAddr gometrics.Counter

	Wrreq2BadAddr gometrics.Counter

	Rdreq3BadAddr gometrics.Counter

	Wrreq3BadAddr gometrics.Counter

	Rdreq4BadAddr gometrics.Counter

	Wrreq4BadAddr gometrics.Counter

	Rdreq5BadAddr gometrics.Counter

	Wrreq5BadAddr gometrics.Counter

	Rdreq6BadAddr gometrics.Counter

	Wrreq6BadAddr gometrics.Counter

	Rdreq7BadAddr gometrics.Counter

	Wrreq7BadAddr gometrics.Counter

	CpuBadAddr gometrics.Counter

	BgBadAddr gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *SsepicsintbadaddrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *SsepicsintbadaddrMetrics) Size() int {
	sz := 0

	sz += mtr.Rdreq0BadAddr.Size()

	sz += mtr.Wrreq0BadAddr.Size()

	sz += mtr.Rdreq1BadAddr.Size()

	sz += mtr.Wrreq1BadAddr.Size()

	sz += mtr.Rdreq2BadAddr.Size()

	sz += mtr.Wrreq2BadAddr.Size()

	sz += mtr.Rdreq3BadAddr.Size()

	sz += mtr.Wrreq3BadAddr.Size()

	sz += mtr.Rdreq4BadAddr.Size()

	sz += mtr.Wrreq4BadAddr.Size()

	sz += mtr.Rdreq5BadAddr.Size()

	sz += mtr.Wrreq5BadAddr.Size()

	sz += mtr.Rdreq6BadAddr.Size()

	sz += mtr.Wrreq6BadAddr.Size()

	sz += mtr.Rdreq7BadAddr.Size()

	sz += mtr.Wrreq7BadAddr.Size()

	sz += mtr.CpuBadAddr.Size()

	sz += mtr.BgBadAddr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *SsepicsintbadaddrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Rdreq0BadAddr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rdreq0BadAddr.Size()

	mtr.Wrreq0BadAddr = mtr.metrics.GetCounter(offset)
	offset += mtr.Wrreq0BadAddr.Size()

	mtr.Rdreq1BadAddr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rdreq1BadAddr.Size()

	mtr.Wrreq1BadAddr = mtr.metrics.GetCounter(offset)
	offset += mtr.Wrreq1BadAddr.Size()

	mtr.Rdreq2BadAddr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rdreq2BadAddr.Size()

	mtr.Wrreq2BadAddr = mtr.metrics.GetCounter(offset)
	offset += mtr.Wrreq2BadAddr.Size()

	mtr.Rdreq3BadAddr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rdreq3BadAddr.Size()

	mtr.Wrreq3BadAddr = mtr.metrics.GetCounter(offset)
	offset += mtr.Wrreq3BadAddr.Size()

	mtr.Rdreq4BadAddr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rdreq4BadAddr.Size()

	mtr.Wrreq4BadAddr = mtr.metrics.GetCounter(offset)
	offset += mtr.Wrreq4BadAddr.Size()

	mtr.Rdreq5BadAddr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rdreq5BadAddr.Size()

	mtr.Wrreq5BadAddr = mtr.metrics.GetCounter(offset)
	offset += mtr.Wrreq5BadAddr.Size()

	mtr.Rdreq6BadAddr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rdreq6BadAddr.Size()

	mtr.Wrreq6BadAddr = mtr.metrics.GetCounter(offset)
	offset += mtr.Wrreq6BadAddr.Size()

	mtr.Rdreq7BadAddr = mtr.metrics.GetCounter(offset)
	offset += mtr.Rdreq7BadAddr.Size()

	mtr.Wrreq7BadAddr = mtr.metrics.GetCounter(offset)
	offset += mtr.Wrreq7BadAddr.Size()

	mtr.CpuBadAddr = mtr.metrics.GetCounter(offset)
	offset += mtr.CpuBadAddr.Size()

	mtr.BgBadAddr = mtr.metrics.GetCounter(offset)
	offset += mtr.BgBadAddr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *SsepicsintbadaddrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Rdreq0BadAddr" {
		return offset
	}
	offset += mtr.Rdreq0BadAddr.Size()

	if fldName == "Wrreq0BadAddr" {
		return offset
	}
	offset += mtr.Wrreq0BadAddr.Size()

	if fldName == "Rdreq1BadAddr" {
		return offset
	}
	offset += mtr.Rdreq1BadAddr.Size()

	if fldName == "Wrreq1BadAddr" {
		return offset
	}
	offset += mtr.Wrreq1BadAddr.Size()

	if fldName == "Rdreq2BadAddr" {
		return offset
	}
	offset += mtr.Rdreq2BadAddr.Size()

	if fldName == "Wrreq2BadAddr" {
		return offset
	}
	offset += mtr.Wrreq2BadAddr.Size()

	if fldName == "Rdreq3BadAddr" {
		return offset
	}
	offset += mtr.Rdreq3BadAddr.Size()

	if fldName == "Wrreq3BadAddr" {
		return offset
	}
	offset += mtr.Wrreq3BadAddr.Size()

	if fldName == "Rdreq4BadAddr" {
		return offset
	}
	offset += mtr.Rdreq4BadAddr.Size()

	if fldName == "Wrreq4BadAddr" {
		return offset
	}
	offset += mtr.Wrreq4BadAddr.Size()

	if fldName == "Rdreq5BadAddr" {
		return offset
	}
	offset += mtr.Rdreq5BadAddr.Size()

	if fldName == "Wrreq5BadAddr" {
		return offset
	}
	offset += mtr.Wrreq5BadAddr.Size()

	if fldName == "Rdreq6BadAddr" {
		return offset
	}
	offset += mtr.Rdreq6BadAddr.Size()

	if fldName == "Wrreq6BadAddr" {
		return offset
	}
	offset += mtr.Wrreq6BadAddr.Size()

	if fldName == "Rdreq7BadAddr" {
		return offset
	}
	offset += mtr.Rdreq7BadAddr.Size()

	if fldName == "Wrreq7BadAddr" {
		return offset
	}
	offset += mtr.Wrreq7BadAddr.Size()

	if fldName == "CpuBadAddr" {
		return offset
	}
	offset += mtr.CpuBadAddr.Size()

	if fldName == "BgBadAddr" {
		return offset
	}
	offset += mtr.BgBadAddr.Size()

	return offset
}

// SetRdreq0BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetRdreq0BadAddr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq0BadAddr"))
	return nil
}

// SetWrreq0BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetWrreq0BadAddr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq0BadAddr"))
	return nil
}

// SetRdreq1BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetRdreq1BadAddr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq1BadAddr"))
	return nil
}

// SetWrreq1BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetWrreq1BadAddr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq1BadAddr"))
	return nil
}

// SetRdreq2BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetRdreq2BadAddr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq2BadAddr"))
	return nil
}

// SetWrreq2BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetWrreq2BadAddr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq2BadAddr"))
	return nil
}

// SetRdreq3BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetRdreq3BadAddr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq3BadAddr"))
	return nil
}

// SetWrreq3BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetWrreq3BadAddr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq3BadAddr"))
	return nil
}

// SetRdreq4BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetRdreq4BadAddr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq4BadAddr"))
	return nil
}

// SetWrreq4BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetWrreq4BadAddr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq4BadAddr"))
	return nil
}

// SetRdreq5BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetRdreq5BadAddr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq5BadAddr"))
	return nil
}

// SetWrreq5BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetWrreq5BadAddr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq5BadAddr"))
	return nil
}

// SetRdreq6BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetRdreq6BadAddr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq6BadAddr"))
	return nil
}

// SetWrreq6BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetWrreq6BadAddr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq6BadAddr"))
	return nil
}

// SetRdreq7BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetRdreq7BadAddr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Rdreq7BadAddr"))
	return nil
}

// SetWrreq7BadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetWrreq7BadAddr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Wrreq7BadAddr"))
	return nil
}

// SetCpuBadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetCpuBadAddr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CpuBadAddr"))
	return nil
}

// SetBgBadAddr sets cunter in shared memory
func (mtr *SsepicsintbadaddrMetrics) SetBgBadAddr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BgBadAddr"))
	return nil
}

// SsepicsintbadaddrMetricsIterator is the iterator object
type SsepicsintbadaddrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *SsepicsintbadaddrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *SsepicsintbadaddrMetricsIterator) Next() *SsepicsintbadaddrMetrics {
	mtr := it.iter.Next()
	tmtr := &SsepicsintbadaddrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *SsepicsintbadaddrMetricsIterator) Find(key uint64) (*SsepicsintbadaddrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &SsepicsintbadaddrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *SsepicsintbadaddrMetricsIterator) Create(key uint64) (*SsepicsintbadaddrMetrics, error) {
	tmtr := &SsepicsintbadaddrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &SsepicsintbadaddrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *SsepicsintbadaddrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewSsepicsintbadaddrMetricsIterator returns an iterator
func NewSsepicsintbadaddrMetricsIterator() (*SsepicsintbadaddrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("SsepicsintbadaddrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &SsepicsintbadaddrMetricsIterator{iter: iter}, nil
}

type SsepicsintbgMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	UnfinishedBg0 gometrics.Counter

	UnfinishedBg1 gometrics.Counter

	UnfinishedBg2 gometrics.Counter

	UnfinishedBg3 gometrics.Counter

	UnfinishedBg4 gometrics.Counter

	UnfinishedBg5 gometrics.Counter

	UnfinishedBg6 gometrics.Counter

	UnfinishedBg7 gometrics.Counter

	UnfinishedBg8 gometrics.Counter

	UnfinishedBg9 gometrics.Counter

	UnfinishedBg10 gometrics.Counter

	UnfinishedBg11 gometrics.Counter

	UnfinishedBg12 gometrics.Counter

	UnfinishedBg13 gometrics.Counter

	UnfinishedBg14 gometrics.Counter

	UnfinishedBg15 gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *SsepicsintbgMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *SsepicsintbgMetrics) Size() int {
	sz := 0

	sz += mtr.UnfinishedBg0.Size()

	sz += mtr.UnfinishedBg1.Size()

	sz += mtr.UnfinishedBg2.Size()

	sz += mtr.UnfinishedBg3.Size()

	sz += mtr.UnfinishedBg4.Size()

	sz += mtr.UnfinishedBg5.Size()

	sz += mtr.UnfinishedBg6.Size()

	sz += mtr.UnfinishedBg7.Size()

	sz += mtr.UnfinishedBg8.Size()

	sz += mtr.UnfinishedBg9.Size()

	sz += mtr.UnfinishedBg10.Size()

	sz += mtr.UnfinishedBg11.Size()

	sz += mtr.UnfinishedBg12.Size()

	sz += mtr.UnfinishedBg13.Size()

	sz += mtr.UnfinishedBg14.Size()

	sz += mtr.UnfinishedBg15.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *SsepicsintbgMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.UnfinishedBg0 = mtr.metrics.GetCounter(offset)
	offset += mtr.UnfinishedBg0.Size()

	mtr.UnfinishedBg1 = mtr.metrics.GetCounter(offset)
	offset += mtr.UnfinishedBg1.Size()

	mtr.UnfinishedBg2 = mtr.metrics.GetCounter(offset)
	offset += mtr.UnfinishedBg2.Size()

	mtr.UnfinishedBg3 = mtr.metrics.GetCounter(offset)
	offset += mtr.UnfinishedBg3.Size()

	mtr.UnfinishedBg4 = mtr.metrics.GetCounter(offset)
	offset += mtr.UnfinishedBg4.Size()

	mtr.UnfinishedBg5 = mtr.metrics.GetCounter(offset)
	offset += mtr.UnfinishedBg5.Size()

	mtr.UnfinishedBg6 = mtr.metrics.GetCounter(offset)
	offset += mtr.UnfinishedBg6.Size()

	mtr.UnfinishedBg7 = mtr.metrics.GetCounter(offset)
	offset += mtr.UnfinishedBg7.Size()

	mtr.UnfinishedBg8 = mtr.metrics.GetCounter(offset)
	offset += mtr.UnfinishedBg8.Size()

	mtr.UnfinishedBg9 = mtr.metrics.GetCounter(offset)
	offset += mtr.UnfinishedBg9.Size()

	mtr.UnfinishedBg10 = mtr.metrics.GetCounter(offset)
	offset += mtr.UnfinishedBg10.Size()

	mtr.UnfinishedBg11 = mtr.metrics.GetCounter(offset)
	offset += mtr.UnfinishedBg11.Size()

	mtr.UnfinishedBg12 = mtr.metrics.GetCounter(offset)
	offset += mtr.UnfinishedBg12.Size()

	mtr.UnfinishedBg13 = mtr.metrics.GetCounter(offset)
	offset += mtr.UnfinishedBg13.Size()

	mtr.UnfinishedBg14 = mtr.metrics.GetCounter(offset)
	offset += mtr.UnfinishedBg14.Size()

	mtr.UnfinishedBg15 = mtr.metrics.GetCounter(offset)
	offset += mtr.UnfinishedBg15.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *SsepicsintbgMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "UnfinishedBg0" {
		return offset
	}
	offset += mtr.UnfinishedBg0.Size()

	if fldName == "UnfinishedBg1" {
		return offset
	}
	offset += mtr.UnfinishedBg1.Size()

	if fldName == "UnfinishedBg2" {
		return offset
	}
	offset += mtr.UnfinishedBg2.Size()

	if fldName == "UnfinishedBg3" {
		return offset
	}
	offset += mtr.UnfinishedBg3.Size()

	if fldName == "UnfinishedBg4" {
		return offset
	}
	offset += mtr.UnfinishedBg4.Size()

	if fldName == "UnfinishedBg5" {
		return offset
	}
	offset += mtr.UnfinishedBg5.Size()

	if fldName == "UnfinishedBg6" {
		return offset
	}
	offset += mtr.UnfinishedBg6.Size()

	if fldName == "UnfinishedBg7" {
		return offset
	}
	offset += mtr.UnfinishedBg7.Size()

	if fldName == "UnfinishedBg8" {
		return offset
	}
	offset += mtr.UnfinishedBg8.Size()

	if fldName == "UnfinishedBg9" {
		return offset
	}
	offset += mtr.UnfinishedBg9.Size()

	if fldName == "UnfinishedBg10" {
		return offset
	}
	offset += mtr.UnfinishedBg10.Size()

	if fldName == "UnfinishedBg11" {
		return offset
	}
	offset += mtr.UnfinishedBg11.Size()

	if fldName == "UnfinishedBg12" {
		return offset
	}
	offset += mtr.UnfinishedBg12.Size()

	if fldName == "UnfinishedBg13" {
		return offset
	}
	offset += mtr.UnfinishedBg13.Size()

	if fldName == "UnfinishedBg14" {
		return offset
	}
	offset += mtr.UnfinishedBg14.Size()

	if fldName == "UnfinishedBg15" {
		return offset
	}
	offset += mtr.UnfinishedBg15.Size()

	return offset
}

// SetUnfinishedBg0 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg0"))
	return nil
}

// SetUnfinishedBg1 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg1"))
	return nil
}

// SetUnfinishedBg2 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg2"))
	return nil
}

// SetUnfinishedBg3 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg3"))
	return nil
}

// SetUnfinishedBg4 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg4(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg4"))
	return nil
}

// SetUnfinishedBg5 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg5(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg5"))
	return nil
}

// SetUnfinishedBg6 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg6(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg6"))
	return nil
}

// SetUnfinishedBg7 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg7(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg7"))
	return nil
}

// SetUnfinishedBg8 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg8(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg8"))
	return nil
}

// SetUnfinishedBg9 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg9(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg9"))
	return nil
}

// SetUnfinishedBg10 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg10(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg10"))
	return nil
}

// SetUnfinishedBg11 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg11(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg11"))
	return nil
}

// SetUnfinishedBg12 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg12(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg12"))
	return nil
}

// SetUnfinishedBg13 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg13(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg13"))
	return nil
}

// SetUnfinishedBg14 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg14(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg14"))
	return nil
}

// SetUnfinishedBg15 sets cunter in shared memory
func (mtr *SsepicsintbgMetrics) SetUnfinishedBg15(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UnfinishedBg15"))
	return nil
}

// SsepicsintbgMetricsIterator is the iterator object
type SsepicsintbgMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *SsepicsintbgMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *SsepicsintbgMetricsIterator) Next() *SsepicsintbgMetrics {
	mtr := it.iter.Next()
	tmtr := &SsepicsintbgMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *SsepicsintbgMetricsIterator) Find(key uint64) (*SsepicsintbgMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &SsepicsintbgMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *SsepicsintbgMetricsIterator) Create(key uint64) (*SsepicsintbgMetrics, error) {
	tmtr := &SsepicsintbgMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &SsepicsintbgMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *SsepicsintbgMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewSsepicsintbgMetricsIterator returns an iterator
func NewSsepicsintbgMetricsIterator() (*SsepicsintbgMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("SsepicsintbgMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &SsepicsintbgMetricsIterator{iter: iter}, nil
}

type SsepicsintpicsMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	UncorrectableEcc gometrics.Counter

	CorrectableEcc gometrics.Counter

	TooManyRlSchError gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *SsepicsintpicsMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *SsepicsintpicsMetrics) Size() int {
	sz := 0

	sz += mtr.UncorrectableEcc.Size()

	sz += mtr.CorrectableEcc.Size()

	sz += mtr.TooManyRlSchError.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *SsepicsintpicsMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.UncorrectableEcc = mtr.metrics.GetCounter(offset)
	offset += mtr.UncorrectableEcc.Size()

	mtr.CorrectableEcc = mtr.metrics.GetCounter(offset)
	offset += mtr.CorrectableEcc.Size()

	mtr.TooManyRlSchError = mtr.metrics.GetCounter(offset)
	offset += mtr.TooManyRlSchError.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *SsepicsintpicsMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "UncorrectableEcc" {
		return offset
	}
	offset += mtr.UncorrectableEcc.Size()

	if fldName == "CorrectableEcc" {
		return offset
	}
	offset += mtr.CorrectableEcc.Size()

	if fldName == "TooManyRlSchError" {
		return offset
	}
	offset += mtr.TooManyRlSchError.Size()

	return offset
}

// SetUncorrectableEcc sets cunter in shared memory
func (mtr *SsepicsintpicsMetrics) SetUncorrectableEcc(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UncorrectableEcc"))
	return nil
}

// SetCorrectableEcc sets cunter in shared memory
func (mtr *SsepicsintpicsMetrics) SetCorrectableEcc(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CorrectableEcc"))
	return nil
}

// SetTooManyRlSchError sets cunter in shared memory
func (mtr *SsepicsintpicsMetrics) SetTooManyRlSchError(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TooManyRlSchError"))
	return nil
}

// SsepicsintpicsMetricsIterator is the iterator object
type SsepicsintpicsMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *SsepicsintpicsMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *SsepicsintpicsMetricsIterator) Next() *SsepicsintpicsMetrics {
	mtr := it.iter.Next()
	tmtr := &SsepicsintpicsMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *SsepicsintpicsMetricsIterator) Find(key uint64) (*SsepicsintpicsMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &SsepicsintpicsMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *SsepicsintpicsMetricsIterator) Create(key uint64) (*SsepicsintpicsMetrics, error) {
	tmtr := &SsepicsintpicsMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &SsepicsintpicsMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *SsepicsintpicsMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewSsepicsintpicsMetricsIterator returns an iterator
func NewSsepicsintpicsMetricsIterator() (*SsepicsintpicsMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("SsepicsintpicsMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &SsepicsintpicsMetricsIterator{iter: iter}, nil
}

type DbwaintdbMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	DbCamConflict gometrics.Counter

	DbPidChkFail gometrics.Counter

	DbQidOverflow gometrics.Counter

	HostRingAccessErr gometrics.Counter

	TotalRingAccessErr gometrics.Counter

	RrespErr gometrics.Counter

	BrespErr gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *DbwaintdbMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *DbwaintdbMetrics) Size() int {
	sz := 0

	sz += mtr.DbCamConflict.Size()

	sz += mtr.DbPidChkFail.Size()

	sz += mtr.DbQidOverflow.Size()

	sz += mtr.HostRingAccessErr.Size()

	sz += mtr.TotalRingAccessErr.Size()

	sz += mtr.RrespErr.Size()

	sz += mtr.BrespErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *DbwaintdbMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.DbCamConflict = mtr.metrics.GetCounter(offset)
	offset += mtr.DbCamConflict.Size()

	mtr.DbPidChkFail = mtr.metrics.GetCounter(offset)
	offset += mtr.DbPidChkFail.Size()

	mtr.DbQidOverflow = mtr.metrics.GetCounter(offset)
	offset += mtr.DbQidOverflow.Size()

	mtr.HostRingAccessErr = mtr.metrics.GetCounter(offset)
	offset += mtr.HostRingAccessErr.Size()

	mtr.TotalRingAccessErr = mtr.metrics.GetCounter(offset)
	offset += mtr.TotalRingAccessErr.Size()

	mtr.RrespErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RrespErr.Size()

	mtr.BrespErr = mtr.metrics.GetCounter(offset)
	offset += mtr.BrespErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *DbwaintdbMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "DbCamConflict" {
		return offset
	}
	offset += mtr.DbCamConflict.Size()

	if fldName == "DbPidChkFail" {
		return offset
	}
	offset += mtr.DbPidChkFail.Size()

	if fldName == "DbQidOverflow" {
		return offset
	}
	offset += mtr.DbQidOverflow.Size()

	if fldName == "HostRingAccessErr" {
		return offset
	}
	offset += mtr.HostRingAccessErr.Size()

	if fldName == "TotalRingAccessErr" {
		return offset
	}
	offset += mtr.TotalRingAccessErr.Size()

	if fldName == "RrespErr" {
		return offset
	}
	offset += mtr.RrespErr.Size()

	if fldName == "BrespErr" {
		return offset
	}
	offset += mtr.BrespErr.Size()

	return offset
}

// SetDbCamConflict sets cunter in shared memory
func (mtr *DbwaintdbMetrics) SetDbCamConflict(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DbCamConflict"))
	return nil
}

// SetDbPidChkFail sets cunter in shared memory
func (mtr *DbwaintdbMetrics) SetDbPidChkFail(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DbPidChkFail"))
	return nil
}

// SetDbQidOverflow sets cunter in shared memory
func (mtr *DbwaintdbMetrics) SetDbQidOverflow(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("DbQidOverflow"))
	return nil
}

// SetHostRingAccessErr sets cunter in shared memory
func (mtr *DbwaintdbMetrics) SetHostRingAccessErr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("HostRingAccessErr"))
	return nil
}

// SetTotalRingAccessErr sets cunter in shared memory
func (mtr *DbwaintdbMetrics) SetTotalRingAccessErr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TotalRingAccessErr"))
	return nil
}

// SetRrespErr sets cunter in shared memory
func (mtr *DbwaintdbMetrics) SetRrespErr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RrespErr"))
	return nil
}

// SetBrespErr sets cunter in shared memory
func (mtr *DbwaintdbMetrics) SetBrespErr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BrespErr"))
	return nil
}

// DbwaintdbMetricsIterator is the iterator object
type DbwaintdbMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *DbwaintdbMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *DbwaintdbMetricsIterator) Next() *DbwaintdbMetrics {
	mtr := it.iter.Next()
	tmtr := &DbwaintdbMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *DbwaintdbMetricsIterator) Find(key uint64) (*DbwaintdbMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &DbwaintdbMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *DbwaintdbMetricsIterator) Create(key uint64) (*DbwaintdbMetrics, error) {
	tmtr := &DbwaintdbMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &DbwaintdbMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *DbwaintdbMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewDbwaintdbMetricsIterator returns an iterator
func NewDbwaintdbMetricsIterator() (*DbwaintdbMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("DbwaintdbMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &DbwaintdbMetricsIterator{iter: iter}, nil
}

type DbwaintlifqstatemapMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	EccUncorrectable gometrics.Counter

	EccCorrectable gometrics.Counter

	QidInvalid gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *DbwaintlifqstatemapMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *DbwaintlifqstatemapMetrics) Size() int {
	sz := 0

	sz += mtr.EccUncorrectable.Size()

	sz += mtr.EccCorrectable.Size()

	sz += mtr.QidInvalid.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *DbwaintlifqstatemapMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.EccUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.EccUncorrectable.Size()

	mtr.EccCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.EccCorrectable.Size()

	mtr.QidInvalid = mtr.metrics.GetCounter(offset)
	offset += mtr.QidInvalid.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *DbwaintlifqstatemapMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "EccUncorrectable" {
		return offset
	}
	offset += mtr.EccUncorrectable.Size()

	if fldName == "EccCorrectable" {
		return offset
	}
	offset += mtr.EccCorrectable.Size()

	if fldName == "QidInvalid" {
		return offset
	}
	offset += mtr.QidInvalid.Size()

	return offset
}

// SetEccUncorrectable sets cunter in shared memory
func (mtr *DbwaintlifqstatemapMetrics) SetEccUncorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EccUncorrectable"))
	return nil
}

// SetEccCorrectable sets cunter in shared memory
func (mtr *DbwaintlifqstatemapMetrics) SetEccCorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EccCorrectable"))
	return nil
}

// SetQidInvalid sets cunter in shared memory
func (mtr *DbwaintlifqstatemapMetrics) SetQidInvalid(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("QidInvalid"))
	return nil
}

// DbwaintlifqstatemapMetricsIterator is the iterator object
type DbwaintlifqstatemapMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *DbwaintlifqstatemapMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *DbwaintlifqstatemapMetricsIterator) Next() *DbwaintlifqstatemapMetrics {
	mtr := it.iter.Next()
	tmtr := &DbwaintlifqstatemapMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *DbwaintlifqstatemapMetricsIterator) Find(key uint64) (*DbwaintlifqstatemapMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &DbwaintlifqstatemapMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *DbwaintlifqstatemapMetricsIterator) Create(key uint64) (*DbwaintlifqstatemapMetrics, error) {
	tmtr := &DbwaintlifqstatemapMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &DbwaintlifqstatemapMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *DbwaintlifqstatemapMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewDbwaintlifqstatemapMetricsIterator returns an iterator
func NewDbwaintlifqstatemapMetricsIterator() (*DbwaintlifqstatemapMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("DbwaintlifqstatemapMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &DbwaintlifqstatemapMetricsIterator{iter: iter}, nil
}

type SgeteinterrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MissSop gometrics.Counter

	MissEop gometrics.Counter

	PhvMaxSize gometrics.Counter

	SpuriousAxiRsp gometrics.Counter

	SpuriousTcamRsp gometrics.Counter

	Te2MpuTimeout gometrics.Counter

	AxiRdrspErr gometrics.Counter

	AxiBadRead gometrics.Counter

	TcamReqIdxFifo gometrics.Counter

	TcamRspIdxFifo gometrics.Counter

	MpuReqIdxFifo gometrics.Counter

	AxiReqIdxFifo gometrics.Counter

	ProcTblVldWoProc gometrics.Counter

	PendWoWb gometrics.Counter

	Pend1WoPend0 gometrics.Counter

	BothPendDown gometrics.Counter

	PendWoProcDown gometrics.Counter

	BothPendWentUp gometrics.Counter

	LoadedButNoProc gometrics.Counter

	LoadedButNoProcTblVld gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *SgeteinterrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *SgeteinterrMetrics) Size() int {
	sz := 0

	sz += mtr.MissSop.Size()

	sz += mtr.MissEop.Size()

	sz += mtr.PhvMaxSize.Size()

	sz += mtr.SpuriousAxiRsp.Size()

	sz += mtr.SpuriousTcamRsp.Size()

	sz += mtr.Te2MpuTimeout.Size()

	sz += mtr.AxiRdrspErr.Size()

	sz += mtr.AxiBadRead.Size()

	sz += mtr.TcamReqIdxFifo.Size()

	sz += mtr.TcamRspIdxFifo.Size()

	sz += mtr.MpuReqIdxFifo.Size()

	sz += mtr.AxiReqIdxFifo.Size()

	sz += mtr.ProcTblVldWoProc.Size()

	sz += mtr.PendWoWb.Size()

	sz += mtr.Pend1WoPend0.Size()

	sz += mtr.BothPendDown.Size()

	sz += mtr.PendWoProcDown.Size()

	sz += mtr.BothPendWentUp.Size()

	sz += mtr.LoadedButNoProc.Size()

	sz += mtr.LoadedButNoProcTblVld.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *SgeteinterrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MissSop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissSop.Size()

	mtr.MissEop = mtr.metrics.GetCounter(offset)
	offset += mtr.MissEop.Size()

	mtr.PhvMaxSize = mtr.metrics.GetCounter(offset)
	offset += mtr.PhvMaxSize.Size()

	mtr.SpuriousAxiRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousAxiRsp.Size()

	mtr.SpuriousTcamRsp = mtr.metrics.GetCounter(offset)
	offset += mtr.SpuriousTcamRsp.Size()

	mtr.Te2MpuTimeout = mtr.metrics.GetCounter(offset)
	offset += mtr.Te2MpuTimeout.Size()

	mtr.AxiRdrspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiRdrspErr.Size()

	mtr.AxiBadRead = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiBadRead.Size()

	mtr.TcamReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamReqIdxFifo.Size()

	mtr.TcamRspIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.TcamRspIdxFifo.Size()

	mtr.MpuReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuReqIdxFifo.Size()

	mtr.AxiReqIdxFifo = mtr.metrics.GetCounter(offset)
	offset += mtr.AxiReqIdxFifo.Size()

	mtr.ProcTblVldWoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.ProcTblVldWoProc.Size()

	mtr.PendWoWb = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoWb.Size()

	mtr.Pend1WoPend0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Pend1WoPend0.Size()

	mtr.BothPendDown = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendDown.Size()

	mtr.PendWoProcDown = mtr.metrics.GetCounter(offset)
	offset += mtr.PendWoProcDown.Size()

	mtr.BothPendWentUp = mtr.metrics.GetCounter(offset)
	offset += mtr.BothPendWentUp.Size()

	mtr.LoadedButNoProc = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProc.Size()

	mtr.LoadedButNoProcTblVld = mtr.metrics.GetCounter(offset)
	offset += mtr.LoadedButNoProcTblVld.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *SgeteinterrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MissSop" {
		return offset
	}
	offset += mtr.MissSop.Size()

	if fldName == "MissEop" {
		return offset
	}
	offset += mtr.MissEop.Size()

	if fldName == "PhvMaxSize" {
		return offset
	}
	offset += mtr.PhvMaxSize.Size()

	if fldName == "SpuriousAxiRsp" {
		return offset
	}
	offset += mtr.SpuriousAxiRsp.Size()

	if fldName == "SpuriousTcamRsp" {
		return offset
	}
	offset += mtr.SpuriousTcamRsp.Size()

	if fldName == "Te2MpuTimeout" {
		return offset
	}
	offset += mtr.Te2MpuTimeout.Size()

	if fldName == "AxiRdrspErr" {
		return offset
	}
	offset += mtr.AxiRdrspErr.Size()

	if fldName == "AxiBadRead" {
		return offset
	}
	offset += mtr.AxiBadRead.Size()

	if fldName == "TcamReqIdxFifo" {
		return offset
	}
	offset += mtr.TcamReqIdxFifo.Size()

	if fldName == "TcamRspIdxFifo" {
		return offset
	}
	offset += mtr.TcamRspIdxFifo.Size()

	if fldName == "MpuReqIdxFifo" {
		return offset
	}
	offset += mtr.MpuReqIdxFifo.Size()

	if fldName == "AxiReqIdxFifo" {
		return offset
	}
	offset += mtr.AxiReqIdxFifo.Size()

	if fldName == "ProcTblVldWoProc" {
		return offset
	}
	offset += mtr.ProcTblVldWoProc.Size()

	if fldName == "PendWoWb" {
		return offset
	}
	offset += mtr.PendWoWb.Size()

	if fldName == "Pend1WoPend0" {
		return offset
	}
	offset += mtr.Pend1WoPend0.Size()

	if fldName == "BothPendDown" {
		return offset
	}
	offset += mtr.BothPendDown.Size()

	if fldName == "PendWoProcDown" {
		return offset
	}
	offset += mtr.PendWoProcDown.Size()

	if fldName == "BothPendWentUp" {
		return offset
	}
	offset += mtr.BothPendWentUp.Size()

	if fldName == "LoadedButNoProc" {
		return offset
	}
	offset += mtr.LoadedButNoProc.Size()

	if fldName == "LoadedButNoProcTblVld" {
		return offset
	}
	offset += mtr.LoadedButNoProcTblVld.Size()

	return offset
}

// SetMissSop sets cunter in shared memory
func (mtr *SgeteinterrMetrics) SetMissSop(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissSop"))
	return nil
}

// SetMissEop sets cunter in shared memory
func (mtr *SgeteinterrMetrics) SetMissEop(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MissEop"))
	return nil
}

// SetPhvMaxSize sets cunter in shared memory
func (mtr *SgeteinterrMetrics) SetPhvMaxSize(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PhvMaxSize"))
	return nil
}

// SetSpuriousAxiRsp sets cunter in shared memory
func (mtr *SgeteinterrMetrics) SetSpuriousAxiRsp(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousAxiRsp"))
	return nil
}

// SetSpuriousTcamRsp sets cunter in shared memory
func (mtr *SgeteinterrMetrics) SetSpuriousTcamRsp(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SpuriousTcamRsp"))
	return nil
}

// SetTe2MpuTimeout sets cunter in shared memory
func (mtr *SgeteinterrMetrics) SetTe2MpuTimeout(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Te2MpuTimeout"))
	return nil
}

// SetAxiRdrspErr sets cunter in shared memory
func (mtr *SgeteinterrMetrics) SetAxiRdrspErr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiRdrspErr"))
	return nil
}

// SetAxiBadRead sets cunter in shared memory
func (mtr *SgeteinterrMetrics) SetAxiBadRead(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiBadRead"))
	return nil
}

// SetTcamReqIdxFifo sets cunter in shared memory
func (mtr *SgeteinterrMetrics) SetTcamReqIdxFifo(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamReqIdxFifo"))
	return nil
}

// SetTcamRspIdxFifo sets cunter in shared memory
func (mtr *SgeteinterrMetrics) SetTcamRspIdxFifo(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TcamRspIdxFifo"))
	return nil
}

// SetMpuReqIdxFifo sets cunter in shared memory
func (mtr *SgeteinterrMetrics) SetMpuReqIdxFifo(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuReqIdxFifo"))
	return nil
}

// SetAxiReqIdxFifo sets cunter in shared memory
func (mtr *SgeteinterrMetrics) SetAxiReqIdxFifo(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("AxiReqIdxFifo"))
	return nil
}

// SetProcTblVldWoProc sets cunter in shared memory
func (mtr *SgeteinterrMetrics) SetProcTblVldWoProc(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProcTblVldWoProc"))
	return nil
}

// SetPendWoWb sets cunter in shared memory
func (mtr *SgeteinterrMetrics) SetPendWoWb(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoWb"))
	return nil
}

// SetPend1WoPend0 sets cunter in shared memory
func (mtr *SgeteinterrMetrics) SetPend1WoPend0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pend1WoPend0"))
	return nil
}

// SetBothPendDown sets cunter in shared memory
func (mtr *SgeteinterrMetrics) SetBothPendDown(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendDown"))
	return nil
}

// SetPendWoProcDown sets cunter in shared memory
func (mtr *SgeteinterrMetrics) SetPendWoProcDown(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PendWoProcDown"))
	return nil
}

// SetBothPendWentUp sets cunter in shared memory
func (mtr *SgeteinterrMetrics) SetBothPendWentUp(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("BothPendWentUp"))
	return nil
}

// SetLoadedButNoProc sets cunter in shared memory
func (mtr *SgeteinterrMetrics) SetLoadedButNoProc(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProc"))
	return nil
}

// SetLoadedButNoProcTblVld sets cunter in shared memory
func (mtr *SgeteinterrMetrics) SetLoadedButNoProcTblVld(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("LoadedButNoProcTblVld"))
	return nil
}

// SgeteinterrMetricsIterator is the iterator object
type SgeteinterrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *SgeteinterrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *SgeteinterrMetricsIterator) Next() *SgeteinterrMetrics {
	mtr := it.iter.Next()
	tmtr := &SgeteinterrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *SgeteinterrMetricsIterator) Find(key uint64) (*SgeteinterrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &SgeteinterrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *SgeteinterrMetricsIterator) Create(key uint64) (*SgeteinterrMetrics, error) {
	tmtr := &SgeteinterrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &SgeteinterrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *SgeteinterrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewSgeteinterrMetricsIterator returns an iterator
func NewSgeteinterrMetricsIterator() (*SgeteinterrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("SgeteinterrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &SgeteinterrMetricsIterator{iter: iter}, nil
}

type SgeteintinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	ProfileCamHit0 gometrics.Counter

	ProfileCamHit1 gometrics.Counter

	ProfileCamHit2 gometrics.Counter

	ProfileCamHit3 gometrics.Counter

	ProfileCamHit4 gometrics.Counter

	ProfileCamHit5 gometrics.Counter

	ProfileCamHit6 gometrics.Counter

	ProfileCamHit7 gometrics.Counter

	ProfileCamHit8 gometrics.Counter

	ProfileCamHit9 gometrics.Counter

	ProfileCamHit10 gometrics.Counter

	ProfileCamHit11 gometrics.Counter

	ProfileCamHit12 gometrics.Counter

	ProfileCamHit13 gometrics.Counter

	ProfileCamHit14 gometrics.Counter

	ProfileCamHit15 gometrics.Counter

	ProfileCamMiss gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *SgeteintinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *SgeteintinfoMetrics) Size() int {
	sz := 0

	sz += mtr.ProfileCamHit0.Size()

	sz += mtr.ProfileCamHit1.Size()

	sz += mtr.ProfileCamHit2.Size()

	sz += mtr.ProfileCamHit3.Size()

	sz += mtr.ProfileCamHit4.Size()

	sz += mtr.ProfileCamHit5.Size()

	sz += mtr.ProfileCamHit6.Size()

	sz += mtr.ProfileCamHit7.Size()

	sz += mtr.ProfileCamHit8.Size()

	sz += mtr.ProfileCamHit9.Size()

	sz += mtr.ProfileCamHit10.Size()

	sz += mtr.ProfileCamHit11.Size()

	sz += mtr.ProfileCamHit12.Size()

	sz += mtr.ProfileCamHit13.Size()

	sz += mtr.ProfileCamHit14.Size()

	sz += mtr.ProfileCamHit15.Size()

	sz += mtr.ProfileCamMiss.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *SgeteintinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.ProfileCamHit0 = mtr.metrics.GetCounter(offset)
	offset += mtr.ProfileCamHit0.Size()

	mtr.ProfileCamHit1 = mtr.metrics.GetCounter(offset)
	offset += mtr.ProfileCamHit1.Size()

	mtr.ProfileCamHit2 = mtr.metrics.GetCounter(offset)
	offset += mtr.ProfileCamHit2.Size()

	mtr.ProfileCamHit3 = mtr.metrics.GetCounter(offset)
	offset += mtr.ProfileCamHit3.Size()

	mtr.ProfileCamHit4 = mtr.metrics.GetCounter(offset)
	offset += mtr.ProfileCamHit4.Size()

	mtr.ProfileCamHit5 = mtr.metrics.GetCounter(offset)
	offset += mtr.ProfileCamHit5.Size()

	mtr.ProfileCamHit6 = mtr.metrics.GetCounter(offset)
	offset += mtr.ProfileCamHit6.Size()

	mtr.ProfileCamHit7 = mtr.metrics.GetCounter(offset)
	offset += mtr.ProfileCamHit7.Size()

	mtr.ProfileCamHit8 = mtr.metrics.GetCounter(offset)
	offset += mtr.ProfileCamHit8.Size()

	mtr.ProfileCamHit9 = mtr.metrics.GetCounter(offset)
	offset += mtr.ProfileCamHit9.Size()

	mtr.ProfileCamHit10 = mtr.metrics.GetCounter(offset)
	offset += mtr.ProfileCamHit10.Size()

	mtr.ProfileCamHit11 = mtr.metrics.GetCounter(offset)
	offset += mtr.ProfileCamHit11.Size()

	mtr.ProfileCamHit12 = mtr.metrics.GetCounter(offset)
	offset += mtr.ProfileCamHit12.Size()

	mtr.ProfileCamHit13 = mtr.metrics.GetCounter(offset)
	offset += mtr.ProfileCamHit13.Size()

	mtr.ProfileCamHit14 = mtr.metrics.GetCounter(offset)
	offset += mtr.ProfileCamHit14.Size()

	mtr.ProfileCamHit15 = mtr.metrics.GetCounter(offset)
	offset += mtr.ProfileCamHit15.Size()

	mtr.ProfileCamMiss = mtr.metrics.GetCounter(offset)
	offset += mtr.ProfileCamMiss.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *SgeteintinfoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "ProfileCamHit0" {
		return offset
	}
	offset += mtr.ProfileCamHit0.Size()

	if fldName == "ProfileCamHit1" {
		return offset
	}
	offset += mtr.ProfileCamHit1.Size()

	if fldName == "ProfileCamHit2" {
		return offset
	}
	offset += mtr.ProfileCamHit2.Size()

	if fldName == "ProfileCamHit3" {
		return offset
	}
	offset += mtr.ProfileCamHit3.Size()

	if fldName == "ProfileCamHit4" {
		return offset
	}
	offset += mtr.ProfileCamHit4.Size()

	if fldName == "ProfileCamHit5" {
		return offset
	}
	offset += mtr.ProfileCamHit5.Size()

	if fldName == "ProfileCamHit6" {
		return offset
	}
	offset += mtr.ProfileCamHit6.Size()

	if fldName == "ProfileCamHit7" {
		return offset
	}
	offset += mtr.ProfileCamHit7.Size()

	if fldName == "ProfileCamHit8" {
		return offset
	}
	offset += mtr.ProfileCamHit8.Size()

	if fldName == "ProfileCamHit9" {
		return offset
	}
	offset += mtr.ProfileCamHit9.Size()

	if fldName == "ProfileCamHit10" {
		return offset
	}
	offset += mtr.ProfileCamHit10.Size()

	if fldName == "ProfileCamHit11" {
		return offset
	}
	offset += mtr.ProfileCamHit11.Size()

	if fldName == "ProfileCamHit12" {
		return offset
	}
	offset += mtr.ProfileCamHit12.Size()

	if fldName == "ProfileCamHit13" {
		return offset
	}
	offset += mtr.ProfileCamHit13.Size()

	if fldName == "ProfileCamHit14" {
		return offset
	}
	offset += mtr.ProfileCamHit14.Size()

	if fldName == "ProfileCamHit15" {
		return offset
	}
	offset += mtr.ProfileCamHit15.Size()

	if fldName == "ProfileCamMiss" {
		return offset
	}
	offset += mtr.ProfileCamMiss.Size()

	return offset
}

// SetProfileCamHit0 sets cunter in shared memory
func (mtr *SgeteintinfoMetrics) SetProfileCamHit0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProfileCamHit0"))
	return nil
}

// SetProfileCamHit1 sets cunter in shared memory
func (mtr *SgeteintinfoMetrics) SetProfileCamHit1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProfileCamHit1"))
	return nil
}

// SetProfileCamHit2 sets cunter in shared memory
func (mtr *SgeteintinfoMetrics) SetProfileCamHit2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProfileCamHit2"))
	return nil
}

// SetProfileCamHit3 sets cunter in shared memory
func (mtr *SgeteintinfoMetrics) SetProfileCamHit3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProfileCamHit3"))
	return nil
}

// SetProfileCamHit4 sets cunter in shared memory
func (mtr *SgeteintinfoMetrics) SetProfileCamHit4(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProfileCamHit4"))
	return nil
}

// SetProfileCamHit5 sets cunter in shared memory
func (mtr *SgeteintinfoMetrics) SetProfileCamHit5(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProfileCamHit5"))
	return nil
}

// SetProfileCamHit6 sets cunter in shared memory
func (mtr *SgeteintinfoMetrics) SetProfileCamHit6(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProfileCamHit6"))
	return nil
}

// SetProfileCamHit7 sets cunter in shared memory
func (mtr *SgeteintinfoMetrics) SetProfileCamHit7(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProfileCamHit7"))
	return nil
}

// SetProfileCamHit8 sets cunter in shared memory
func (mtr *SgeteintinfoMetrics) SetProfileCamHit8(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProfileCamHit8"))
	return nil
}

// SetProfileCamHit9 sets cunter in shared memory
func (mtr *SgeteintinfoMetrics) SetProfileCamHit9(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProfileCamHit9"))
	return nil
}

// SetProfileCamHit10 sets cunter in shared memory
func (mtr *SgeteintinfoMetrics) SetProfileCamHit10(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProfileCamHit10"))
	return nil
}

// SetProfileCamHit11 sets cunter in shared memory
func (mtr *SgeteintinfoMetrics) SetProfileCamHit11(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProfileCamHit11"))
	return nil
}

// SetProfileCamHit12 sets cunter in shared memory
func (mtr *SgeteintinfoMetrics) SetProfileCamHit12(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProfileCamHit12"))
	return nil
}

// SetProfileCamHit13 sets cunter in shared memory
func (mtr *SgeteintinfoMetrics) SetProfileCamHit13(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProfileCamHit13"))
	return nil
}

// SetProfileCamHit14 sets cunter in shared memory
func (mtr *SgeteintinfoMetrics) SetProfileCamHit14(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProfileCamHit14"))
	return nil
}

// SetProfileCamHit15 sets cunter in shared memory
func (mtr *SgeteintinfoMetrics) SetProfileCamHit15(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProfileCamHit15"))
	return nil
}

// SetProfileCamMiss sets cunter in shared memory
func (mtr *SgeteintinfoMetrics) SetProfileCamMiss(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ProfileCamMiss"))
	return nil
}

// SgeteintinfoMetricsIterator is the iterator object
type SgeteintinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *SgeteintinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *SgeteintinfoMetricsIterator) Next() *SgeteintinfoMetrics {
	mtr := it.iter.Next()
	tmtr := &SgeteintinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *SgeteintinfoMetricsIterator) Find(key uint64) (*SgeteintinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &SgeteintinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *SgeteintinfoMetricsIterator) Create(key uint64) (*SgeteintinfoMetrics, error) {
	tmtr := &SgeteintinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &SgeteintinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *SgeteintinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewSgeteintinfoMetricsIterator returns an iterator
func NewSgeteintinfoMetricsIterator() (*SgeteintinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("SgeteintinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &SgeteintinfoMetricsIterator{iter: iter}, nil
}

type SgempuinterrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	ResultsMismatch gometrics.Counter

	SdpMemUncorrectable gometrics.Counter

	SdpMemCorrectable gometrics.Counter

	IllegalOp_0 gometrics.Counter

	IllegalOp_1 gometrics.Counter

	IllegalOp_2 gometrics.Counter

	IllegalOp_3 gometrics.Counter

	MaxInst_0 gometrics.Counter

	MaxInst_1 gometrics.Counter

	MaxInst_2 gometrics.Counter

	MaxInst_3 gometrics.Counter

	Phvwr_0 gometrics.Counter

	Phvwr_1 gometrics.Counter

	Phvwr_2 gometrics.Counter

	Phvwr_3 gometrics.Counter

	WriteErr_0 gometrics.Counter

	WriteErr_1 gometrics.Counter

	WriteErr_2 gometrics.Counter

	WriteErr_3 gometrics.Counter

	CacheAxi_0 gometrics.Counter

	CacheAxi_1 gometrics.Counter

	CacheAxi_2 gometrics.Counter

	CacheAxi_3 gometrics.Counter

	CacheParity_0 gometrics.Counter

	CacheParity_1 gometrics.Counter

	CacheParity_2 gometrics.Counter

	CacheParity_3 gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *SgempuinterrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *SgempuinterrMetrics) Size() int {
	sz := 0

	sz += mtr.ResultsMismatch.Size()

	sz += mtr.SdpMemUncorrectable.Size()

	sz += mtr.SdpMemCorrectable.Size()

	sz += mtr.IllegalOp_0.Size()

	sz += mtr.IllegalOp_1.Size()

	sz += mtr.IllegalOp_2.Size()

	sz += mtr.IllegalOp_3.Size()

	sz += mtr.MaxInst_0.Size()

	sz += mtr.MaxInst_1.Size()

	sz += mtr.MaxInst_2.Size()

	sz += mtr.MaxInst_3.Size()

	sz += mtr.Phvwr_0.Size()

	sz += mtr.Phvwr_1.Size()

	sz += mtr.Phvwr_2.Size()

	sz += mtr.Phvwr_3.Size()

	sz += mtr.WriteErr_0.Size()

	sz += mtr.WriteErr_1.Size()

	sz += mtr.WriteErr_2.Size()

	sz += mtr.WriteErr_3.Size()

	sz += mtr.CacheAxi_0.Size()

	sz += mtr.CacheAxi_1.Size()

	sz += mtr.CacheAxi_2.Size()

	sz += mtr.CacheAxi_3.Size()

	sz += mtr.CacheParity_0.Size()

	sz += mtr.CacheParity_1.Size()

	sz += mtr.CacheParity_2.Size()

	sz += mtr.CacheParity_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *SgempuinterrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.ResultsMismatch = mtr.metrics.GetCounter(offset)
	offset += mtr.ResultsMismatch.Size()

	mtr.SdpMemUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemUncorrectable.Size()

	mtr.SdpMemCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.SdpMemCorrectable.Size()

	mtr.IllegalOp_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_0.Size()

	mtr.IllegalOp_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_1.Size()

	mtr.IllegalOp_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_2.Size()

	mtr.IllegalOp_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.IllegalOp_3.Size()

	mtr.MaxInst_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_0.Size()

	mtr.MaxInst_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_1.Size()

	mtr.MaxInst_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_2.Size()

	mtr.MaxInst_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MaxInst_3.Size()

	mtr.Phvwr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_0.Size()

	mtr.Phvwr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_1.Size()

	mtr.Phvwr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_2.Size()

	mtr.Phvwr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Phvwr_3.Size()

	mtr.WriteErr_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_0.Size()

	mtr.WriteErr_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_1.Size()

	mtr.WriteErr_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_2.Size()

	mtr.WriteErr_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.WriteErr_3.Size()

	mtr.CacheAxi_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_0.Size()

	mtr.CacheAxi_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_1.Size()

	mtr.CacheAxi_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_2.Size()

	mtr.CacheAxi_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheAxi_3.Size()

	mtr.CacheParity_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_0.Size()

	mtr.CacheParity_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_1.Size()

	mtr.CacheParity_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_2.Size()

	mtr.CacheParity_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CacheParity_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *SgempuinterrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "ResultsMismatch" {
		return offset
	}
	offset += mtr.ResultsMismatch.Size()

	if fldName == "SdpMemUncorrectable" {
		return offset
	}
	offset += mtr.SdpMemUncorrectable.Size()

	if fldName == "SdpMemCorrectable" {
		return offset
	}
	offset += mtr.SdpMemCorrectable.Size()

	if fldName == "IllegalOp_0" {
		return offset
	}
	offset += mtr.IllegalOp_0.Size()

	if fldName == "IllegalOp_1" {
		return offset
	}
	offset += mtr.IllegalOp_1.Size()

	if fldName == "IllegalOp_2" {
		return offset
	}
	offset += mtr.IllegalOp_2.Size()

	if fldName == "IllegalOp_3" {
		return offset
	}
	offset += mtr.IllegalOp_3.Size()

	if fldName == "MaxInst_0" {
		return offset
	}
	offset += mtr.MaxInst_0.Size()

	if fldName == "MaxInst_1" {
		return offset
	}
	offset += mtr.MaxInst_1.Size()

	if fldName == "MaxInst_2" {
		return offset
	}
	offset += mtr.MaxInst_2.Size()

	if fldName == "MaxInst_3" {
		return offset
	}
	offset += mtr.MaxInst_3.Size()

	if fldName == "Phvwr_0" {
		return offset
	}
	offset += mtr.Phvwr_0.Size()

	if fldName == "Phvwr_1" {
		return offset
	}
	offset += mtr.Phvwr_1.Size()

	if fldName == "Phvwr_2" {
		return offset
	}
	offset += mtr.Phvwr_2.Size()

	if fldName == "Phvwr_3" {
		return offset
	}
	offset += mtr.Phvwr_3.Size()

	if fldName == "WriteErr_0" {
		return offset
	}
	offset += mtr.WriteErr_0.Size()

	if fldName == "WriteErr_1" {
		return offset
	}
	offset += mtr.WriteErr_1.Size()

	if fldName == "WriteErr_2" {
		return offset
	}
	offset += mtr.WriteErr_2.Size()

	if fldName == "WriteErr_3" {
		return offset
	}
	offset += mtr.WriteErr_3.Size()

	if fldName == "CacheAxi_0" {
		return offset
	}
	offset += mtr.CacheAxi_0.Size()

	if fldName == "CacheAxi_1" {
		return offset
	}
	offset += mtr.CacheAxi_1.Size()

	if fldName == "CacheAxi_2" {
		return offset
	}
	offset += mtr.CacheAxi_2.Size()

	if fldName == "CacheAxi_3" {
		return offset
	}
	offset += mtr.CacheAxi_3.Size()

	if fldName == "CacheParity_0" {
		return offset
	}
	offset += mtr.CacheParity_0.Size()

	if fldName == "CacheParity_1" {
		return offset
	}
	offset += mtr.CacheParity_1.Size()

	if fldName == "CacheParity_2" {
		return offset
	}
	offset += mtr.CacheParity_2.Size()

	if fldName == "CacheParity_3" {
		return offset
	}
	offset += mtr.CacheParity_3.Size()

	return offset
}

// SetResultsMismatch sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetResultsMismatch(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ResultsMismatch"))
	return nil
}

// SetSdpMemUncorrectable sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetSdpMemUncorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemUncorrectable"))
	return nil
}

// SetSdpMemCorrectable sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetSdpMemCorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SdpMemCorrectable"))
	return nil
}

// SetIllegalOp_0 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetIllegalOp_0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_0"))
	return nil
}

// SetIllegalOp_1 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetIllegalOp_1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_1"))
	return nil
}

// SetIllegalOp_2 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetIllegalOp_2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_2"))
	return nil
}

// SetIllegalOp_3 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetIllegalOp_3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IllegalOp_3"))
	return nil
}

// SetMaxInst_0 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetMaxInst_0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_0"))
	return nil
}

// SetMaxInst_1 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetMaxInst_1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_1"))
	return nil
}

// SetMaxInst_2 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetMaxInst_2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_2"))
	return nil
}

// SetMaxInst_3 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetMaxInst_3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MaxInst_3"))
	return nil
}

// SetPhvwr_0 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetPhvwr_0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_0"))
	return nil
}

// SetPhvwr_1 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetPhvwr_1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_1"))
	return nil
}

// SetPhvwr_2 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetPhvwr_2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_2"))
	return nil
}

// SetPhvwr_3 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetPhvwr_3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Phvwr_3"))
	return nil
}

// SetWriteErr_0 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetWriteErr_0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_0"))
	return nil
}

// SetWriteErr_1 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetWriteErr_1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_1"))
	return nil
}

// SetWriteErr_2 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetWriteErr_2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_2"))
	return nil
}

// SetWriteErr_3 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetWriteErr_3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WriteErr_3"))
	return nil
}

// SetCacheAxi_0 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetCacheAxi_0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_0"))
	return nil
}

// SetCacheAxi_1 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetCacheAxi_1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_1"))
	return nil
}

// SetCacheAxi_2 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetCacheAxi_2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_2"))
	return nil
}

// SetCacheAxi_3 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetCacheAxi_3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheAxi_3"))
	return nil
}

// SetCacheParity_0 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetCacheParity_0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_0"))
	return nil
}

// SetCacheParity_1 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetCacheParity_1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_1"))
	return nil
}

// SetCacheParity_2 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetCacheParity_2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_2"))
	return nil
}

// SetCacheParity_3 sets cunter in shared memory
func (mtr *SgempuinterrMetrics) SetCacheParity_3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CacheParity_3"))
	return nil
}

// SgempuinterrMetricsIterator is the iterator object
type SgempuinterrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *SgempuinterrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *SgempuinterrMetricsIterator) Next() *SgempuinterrMetrics {
	mtr := it.iter.Next()
	tmtr := &SgempuinterrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *SgempuinterrMetricsIterator) Find(key uint64) (*SgempuinterrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &SgempuinterrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *SgempuinterrMetricsIterator) Create(key uint64) (*SgempuinterrMetrics, error) {
	tmtr := &SgempuinterrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &SgempuinterrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *SgempuinterrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewSgempuinterrMetricsIterator returns an iterator
func NewSgempuinterrMetricsIterator() (*SgempuinterrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("SgempuinterrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &SgempuinterrMetricsIterator{iter: iter}, nil
}

type SgempuintinfoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	TraceFull_0 gometrics.Counter

	TraceFull_1 gometrics.Counter

	TraceFull_2 gometrics.Counter

	TraceFull_3 gometrics.Counter

	MpuStop_0 gometrics.Counter

	MpuStop_1 gometrics.Counter

	MpuStop_2 gometrics.Counter

	MpuStop_3 gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *SgempuintinfoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *SgempuintinfoMetrics) Size() int {
	sz := 0

	sz += mtr.TraceFull_0.Size()

	sz += mtr.TraceFull_1.Size()

	sz += mtr.TraceFull_2.Size()

	sz += mtr.TraceFull_3.Size()

	sz += mtr.MpuStop_0.Size()

	sz += mtr.MpuStop_1.Size()

	sz += mtr.MpuStop_2.Size()

	sz += mtr.MpuStop_3.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *SgempuintinfoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.TraceFull_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.TraceFull_0.Size()

	mtr.TraceFull_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.TraceFull_1.Size()

	mtr.TraceFull_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.TraceFull_2.Size()

	mtr.TraceFull_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.TraceFull_3.Size()

	mtr.MpuStop_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_0.Size()

	mtr.MpuStop_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_1.Size()

	mtr.MpuStop_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_2.Size()

	mtr.MpuStop_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.MpuStop_3.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *SgempuintinfoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "TraceFull_0" {
		return offset
	}
	offset += mtr.TraceFull_0.Size()

	if fldName == "TraceFull_1" {
		return offset
	}
	offset += mtr.TraceFull_1.Size()

	if fldName == "TraceFull_2" {
		return offset
	}
	offset += mtr.TraceFull_2.Size()

	if fldName == "TraceFull_3" {
		return offset
	}
	offset += mtr.TraceFull_3.Size()

	if fldName == "MpuStop_0" {
		return offset
	}
	offset += mtr.MpuStop_0.Size()

	if fldName == "MpuStop_1" {
		return offset
	}
	offset += mtr.MpuStop_1.Size()

	if fldName == "MpuStop_2" {
		return offset
	}
	offset += mtr.MpuStop_2.Size()

	if fldName == "MpuStop_3" {
		return offset
	}
	offset += mtr.MpuStop_3.Size()

	return offset
}

// SetTraceFull_0 sets cunter in shared memory
func (mtr *SgempuintinfoMetrics) SetTraceFull_0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TraceFull_0"))
	return nil
}

// SetTraceFull_1 sets cunter in shared memory
func (mtr *SgempuintinfoMetrics) SetTraceFull_1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TraceFull_1"))
	return nil
}

// SetTraceFull_2 sets cunter in shared memory
func (mtr *SgempuintinfoMetrics) SetTraceFull_2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TraceFull_2"))
	return nil
}

// SetTraceFull_3 sets cunter in shared memory
func (mtr *SgempuintinfoMetrics) SetTraceFull_3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TraceFull_3"))
	return nil
}

// SetMpuStop_0 sets cunter in shared memory
func (mtr *SgempuintinfoMetrics) SetMpuStop_0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_0"))
	return nil
}

// SetMpuStop_1 sets cunter in shared memory
func (mtr *SgempuintinfoMetrics) SetMpuStop_1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_1"))
	return nil
}

// SetMpuStop_2 sets cunter in shared memory
func (mtr *SgempuintinfoMetrics) SetMpuStop_2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_2"))
	return nil
}

// SetMpuStop_3 sets cunter in shared memory
func (mtr *SgempuintinfoMetrics) SetMpuStop_3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MpuStop_3"))
	return nil
}

// SgempuintinfoMetricsIterator is the iterator object
type SgempuintinfoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *SgempuintinfoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *SgempuintinfoMetricsIterator) Next() *SgempuintinfoMetrics {
	mtr := it.iter.Next()
	tmtr := &SgempuintinfoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *SgempuintinfoMetricsIterator) Find(key uint64) (*SgempuintinfoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &SgempuintinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *SgempuintinfoMetricsIterator) Create(key uint64) (*SgempuintinfoMetrics, error) {
	tmtr := &SgempuintinfoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &SgempuintinfoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *SgempuintinfoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewSgempuintinfoMetricsIterator returns an iterator
func NewSgempuintinfoMetricsIterator() (*SgempuintinfoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("SgempuintinfoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &SgempuintinfoMetricsIterator{iter: iter}, nil
}

type MdhensintaxierrMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	WrspErr gometrics.Counter

	RrspErr gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *MdhensintaxierrMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *MdhensintaxierrMetrics) Size() int {
	sz := 0

	sz += mtr.WrspErr.Size()

	sz += mtr.RrspErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *MdhensintaxierrMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.WrspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.WrspErr.Size()

	mtr.RrspErr = mtr.metrics.GetCounter(offset)
	offset += mtr.RrspErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *MdhensintaxierrMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "WrspErr" {
		return offset
	}
	offset += mtr.WrspErr.Size()

	if fldName == "RrspErr" {
		return offset
	}
	offset += mtr.RrspErr.Size()

	return offset
}

// SetWrspErr sets cunter in shared memory
func (mtr *MdhensintaxierrMetrics) SetWrspErr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("WrspErr"))
	return nil
}

// SetRrspErr sets cunter in shared memory
func (mtr *MdhensintaxierrMetrics) SetRrspErr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RrspErr"))
	return nil
}

// MdhensintaxierrMetricsIterator is the iterator object
type MdhensintaxierrMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *MdhensintaxierrMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *MdhensintaxierrMetricsIterator) Next() *MdhensintaxierrMetrics {
	mtr := it.iter.Next()
	tmtr := &MdhensintaxierrMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *MdhensintaxierrMetricsIterator) Find(key uint64) (*MdhensintaxierrMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &MdhensintaxierrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *MdhensintaxierrMetricsIterator) Create(key uint64) (*MdhensintaxierrMetrics, error) {
	tmtr := &MdhensintaxierrMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &MdhensintaxierrMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *MdhensintaxierrMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewMdhensintaxierrMetricsIterator returns an iterator
func NewMdhensintaxierrMetricsIterator() (*MdhensintaxierrMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("MdhensintaxierrMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &MdhensintaxierrMetricsIterator{iter: iter}, nil
}

type MdhensinteccMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	CorrectableErr gometrics.Counter

	UncorrectableErr gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *MdhensinteccMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *MdhensinteccMetrics) Size() int {
	sz := 0

	sz += mtr.CorrectableErr.Size()

	sz += mtr.UncorrectableErr.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *MdhensinteccMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.CorrectableErr = mtr.metrics.GetCounter(offset)
	offset += mtr.CorrectableErr.Size()

	mtr.UncorrectableErr = mtr.metrics.GetCounter(offset)
	offset += mtr.UncorrectableErr.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *MdhensinteccMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "CorrectableErr" {
		return offset
	}
	offset += mtr.CorrectableErr.Size()

	if fldName == "UncorrectableErr" {
		return offset
	}
	offset += mtr.UncorrectableErr.Size()

	return offset
}

// SetCorrectableErr sets cunter in shared memory
func (mtr *MdhensinteccMetrics) SetCorrectableErr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CorrectableErr"))
	return nil
}

// SetUncorrectableErr sets cunter in shared memory
func (mtr *MdhensinteccMetrics) SetUncorrectableErr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("UncorrectableErr"))
	return nil
}

// MdhensinteccMetricsIterator is the iterator object
type MdhensinteccMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *MdhensinteccMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *MdhensinteccMetricsIterator) Next() *MdhensinteccMetrics {
	mtr := it.iter.Next()
	tmtr := &MdhensinteccMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *MdhensinteccMetricsIterator) Find(key uint64) (*MdhensinteccMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &MdhensinteccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *MdhensinteccMetricsIterator) Create(key uint64) (*MdhensinteccMetrics, error) {
	tmtr := &MdhensinteccMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &MdhensinteccMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *MdhensinteccMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewMdhensinteccMetricsIterator returns an iterator
func NewMdhensinteccMetricsIterator() (*MdhensinteccMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("MdhensinteccMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &MdhensinteccMetricsIterator{iter: iter}, nil
}

type MdhensintipcoreMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	XtsEnc gometrics.Counter

	Xts gometrics.Counter

	Gcm0 gometrics.Counter

	Gcm1 gometrics.Counter

	Drbg gometrics.Counter

	Pk gometrics.Counter

	Cp gometrics.Counter

	Dc gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *MdhensintipcoreMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *MdhensintipcoreMetrics) Size() int {
	sz := 0

	sz += mtr.XtsEnc.Size()

	sz += mtr.Xts.Size()

	sz += mtr.Gcm0.Size()

	sz += mtr.Gcm1.Size()

	sz += mtr.Drbg.Size()

	sz += mtr.Pk.Size()

	sz += mtr.Cp.Size()

	sz += mtr.Dc.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *MdhensintipcoreMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.XtsEnc = mtr.metrics.GetCounter(offset)
	offset += mtr.XtsEnc.Size()

	mtr.Xts = mtr.metrics.GetCounter(offset)
	offset += mtr.Xts.Size()

	mtr.Gcm0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Gcm0.Size()

	mtr.Gcm1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Gcm1.Size()

	mtr.Drbg = mtr.metrics.GetCounter(offset)
	offset += mtr.Drbg.Size()

	mtr.Pk = mtr.metrics.GetCounter(offset)
	offset += mtr.Pk.Size()

	mtr.Cp = mtr.metrics.GetCounter(offset)
	offset += mtr.Cp.Size()

	mtr.Dc = mtr.metrics.GetCounter(offset)
	offset += mtr.Dc.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *MdhensintipcoreMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "XtsEnc" {
		return offset
	}
	offset += mtr.XtsEnc.Size()

	if fldName == "Xts" {
		return offset
	}
	offset += mtr.Xts.Size()

	if fldName == "Gcm0" {
		return offset
	}
	offset += mtr.Gcm0.Size()

	if fldName == "Gcm1" {
		return offset
	}
	offset += mtr.Gcm1.Size()

	if fldName == "Drbg" {
		return offset
	}
	offset += mtr.Drbg.Size()

	if fldName == "Pk" {
		return offset
	}
	offset += mtr.Pk.Size()

	if fldName == "Cp" {
		return offset
	}
	offset += mtr.Cp.Size()

	if fldName == "Dc" {
		return offset
	}
	offset += mtr.Dc.Size()

	return offset
}

// SetXtsEnc sets cunter in shared memory
func (mtr *MdhensintipcoreMetrics) SetXtsEnc(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XtsEnc"))
	return nil
}

// SetXts sets cunter in shared memory
func (mtr *MdhensintipcoreMetrics) SetXts(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Xts"))
	return nil
}

// SetGcm0 sets cunter in shared memory
func (mtr *MdhensintipcoreMetrics) SetGcm0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Gcm0"))
	return nil
}

// SetGcm1 sets cunter in shared memory
func (mtr *MdhensintipcoreMetrics) SetGcm1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Gcm1"))
	return nil
}

// SetDrbg sets cunter in shared memory
func (mtr *MdhensintipcoreMetrics) SetDrbg(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Drbg"))
	return nil
}

// SetPk sets cunter in shared memory
func (mtr *MdhensintipcoreMetrics) SetPk(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pk"))
	return nil
}

// SetCp sets cunter in shared memory
func (mtr *MdhensintipcoreMetrics) SetCp(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Cp"))
	return nil
}

// SetDc sets cunter in shared memory
func (mtr *MdhensintipcoreMetrics) SetDc(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Dc"))
	return nil
}

// MdhensintipcoreMetricsIterator is the iterator object
type MdhensintipcoreMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *MdhensintipcoreMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *MdhensintipcoreMetricsIterator) Next() *MdhensintipcoreMetrics {
	mtr := it.iter.Next()
	tmtr := &MdhensintipcoreMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *MdhensintipcoreMetricsIterator) Find(key uint64) (*MdhensintipcoreMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &MdhensintipcoreMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *MdhensintipcoreMetricsIterator) Create(key uint64) (*MdhensintipcoreMetrics, error) {
	tmtr := &MdhensintipcoreMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &MdhensintipcoreMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *MdhensintipcoreMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewMdhensintipcoreMetricsIterator returns an iterator
func NewMdhensintipcoreMetricsIterator() (*MdhensintipcoreMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("MdhensintipcoreMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &MdhensintipcoreMetricsIterator{iter: iter}, nil
}

type MpmpnsintcryptoMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Mpp0 gometrics.Counter

	Mpp1 gometrics.Counter

	Mpp2 gometrics.Counter

	Mpp3 gometrics.Counter

	Mpp4 gometrics.Counter

	Mpp5 gometrics.Counter

	Mpp6 gometrics.Counter

	Mpp7 gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *MpmpnsintcryptoMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *MpmpnsintcryptoMetrics) Size() int {
	sz := 0

	sz += mtr.Mpp0.Size()

	sz += mtr.Mpp1.Size()

	sz += mtr.Mpp2.Size()

	sz += mtr.Mpp3.Size()

	sz += mtr.Mpp4.Size()

	sz += mtr.Mpp5.Size()

	sz += mtr.Mpp6.Size()

	sz += mtr.Mpp7.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *MpmpnsintcryptoMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Mpp0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Mpp0.Size()

	mtr.Mpp1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Mpp1.Size()

	mtr.Mpp2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Mpp2.Size()

	mtr.Mpp3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Mpp3.Size()

	mtr.Mpp4 = mtr.metrics.GetCounter(offset)
	offset += mtr.Mpp4.Size()

	mtr.Mpp5 = mtr.metrics.GetCounter(offset)
	offset += mtr.Mpp5.Size()

	mtr.Mpp6 = mtr.metrics.GetCounter(offset)
	offset += mtr.Mpp6.Size()

	mtr.Mpp7 = mtr.metrics.GetCounter(offset)
	offset += mtr.Mpp7.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *MpmpnsintcryptoMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Mpp0" {
		return offset
	}
	offset += mtr.Mpp0.Size()

	if fldName == "Mpp1" {
		return offset
	}
	offset += mtr.Mpp1.Size()

	if fldName == "Mpp2" {
		return offset
	}
	offset += mtr.Mpp2.Size()

	if fldName == "Mpp3" {
		return offset
	}
	offset += mtr.Mpp3.Size()

	if fldName == "Mpp4" {
		return offset
	}
	offset += mtr.Mpp4.Size()

	if fldName == "Mpp5" {
		return offset
	}
	offset += mtr.Mpp5.Size()

	if fldName == "Mpp6" {
		return offset
	}
	offset += mtr.Mpp6.Size()

	if fldName == "Mpp7" {
		return offset
	}
	offset += mtr.Mpp7.Size()

	return offset
}

// SetMpp0 sets cunter in shared memory
func (mtr *MpmpnsintcryptoMetrics) SetMpp0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Mpp0"))
	return nil
}

// SetMpp1 sets cunter in shared memory
func (mtr *MpmpnsintcryptoMetrics) SetMpp1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Mpp1"))
	return nil
}

// SetMpp2 sets cunter in shared memory
func (mtr *MpmpnsintcryptoMetrics) SetMpp2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Mpp2"))
	return nil
}

// SetMpp3 sets cunter in shared memory
func (mtr *MpmpnsintcryptoMetrics) SetMpp3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Mpp3"))
	return nil
}

// SetMpp4 sets cunter in shared memory
func (mtr *MpmpnsintcryptoMetrics) SetMpp4(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Mpp4"))
	return nil
}

// SetMpp5 sets cunter in shared memory
func (mtr *MpmpnsintcryptoMetrics) SetMpp5(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Mpp5"))
	return nil
}

// SetMpp6 sets cunter in shared memory
func (mtr *MpmpnsintcryptoMetrics) SetMpp6(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Mpp6"))
	return nil
}

// SetMpp7 sets cunter in shared memory
func (mtr *MpmpnsintcryptoMetrics) SetMpp7(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Mpp7"))
	return nil
}

// MpmpnsintcryptoMetricsIterator is the iterator object
type MpmpnsintcryptoMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *MpmpnsintcryptoMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *MpmpnsintcryptoMetricsIterator) Next() *MpmpnsintcryptoMetrics {
	mtr := it.iter.Next()
	tmtr := &MpmpnsintcryptoMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *MpmpnsintcryptoMetricsIterator) Find(key uint64) (*MpmpnsintcryptoMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &MpmpnsintcryptoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *MpmpnsintcryptoMetricsIterator) Create(key uint64) (*MpmpnsintcryptoMetrics, error) {
	tmtr := &MpmpnsintcryptoMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &MpmpnsintcryptoMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *MpmpnsintcryptoMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewMpmpnsintcryptoMetricsIterator returns an iterator
func NewMpmpnsintcryptoMetricsIterator() (*MpmpnsintcryptoMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("MpmpnsintcryptoMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &MpmpnsintcryptoMetricsIterator{iter: iter}, nil
}

type PbpbcintcreditunderflowMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Port_10 gometrics.Counter

	Port_11 gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbcintcreditunderflowMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbcintcreditunderflowMetrics) Size() int {
	sz := 0

	sz += mtr.Port_10.Size()

	sz += mtr.Port_11.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbcintcreditunderflowMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Port_10 = mtr.metrics.GetCounter(offset)
	offset += mtr.Port_10.Size()

	mtr.Port_11 = mtr.metrics.GetCounter(offset)
	offset += mtr.Port_11.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbcintcreditunderflowMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Port_10" {
		return offset
	}
	offset += mtr.Port_10.Size()

	if fldName == "Port_11" {
		return offset
	}
	offset += mtr.Port_11.Size()

	return offset
}

// SetPort_10 sets cunter in shared memory
func (mtr *PbpbcintcreditunderflowMetrics) SetPort_10(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Port_10"))
	return nil
}

// SetPort_11 sets cunter in shared memory
func (mtr *PbpbcintcreditunderflowMetrics) SetPort_11(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Port_11"))
	return nil
}

// PbpbcintcreditunderflowMetricsIterator is the iterator object
type PbpbcintcreditunderflowMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbcintcreditunderflowMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbcintcreditunderflowMetricsIterator) Next() *PbpbcintcreditunderflowMetrics {
	mtr := it.iter.Next()
	tmtr := &PbpbcintcreditunderflowMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbcintcreditunderflowMetricsIterator) Find(key uint64) (*PbpbcintcreditunderflowMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbcintcreditunderflowMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbcintcreditunderflowMetricsIterator) Create(key uint64) (*PbpbcintcreditunderflowMetrics, error) {
	tmtr := &PbpbcintcreditunderflowMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbcintcreditunderflowMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbcintcreditunderflowMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewPbpbcintcreditunderflowMetricsIterator returns an iterator
func NewPbpbcintcreditunderflowMetricsIterator() (*PbpbcintcreditunderflowMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbcintcreditunderflowMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbcintcreditunderflowMetricsIterator{iter: iter}, nil
}

type InteccdescMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Uncorrectable gometrics.Counter

	Correctable gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *InteccdescMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *InteccdescMetrics) Size() int {
	sz := 0

	sz += mtr.Uncorrectable.Size()

	sz += mtr.Correctable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *InteccdescMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Uncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.Uncorrectable.Size()

	mtr.Correctable = mtr.metrics.GetCounter(offset)
	offset += mtr.Correctable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *InteccdescMetrics) getOffset(fldName string) int {
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
func (mtr *InteccdescMetrics) SetUncorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Uncorrectable"))
	return nil
}

// SetCorrectable sets cunter in shared memory
func (mtr *InteccdescMetrics) SetCorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Correctable"))
	return nil
}

// InteccdescMetricsIterator is the iterator object
type InteccdescMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *InteccdescMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *InteccdescMetricsIterator) Next() *InteccdescMetrics {
	mtr := it.iter.Next()
	tmtr := &InteccdescMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *InteccdescMetricsIterator) Find(key uint64) (*InteccdescMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &InteccdescMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *InteccdescMetricsIterator) Create(key uint64) (*InteccdescMetrics, error) {
	tmtr := &InteccdescMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &InteccdescMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *InteccdescMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewInteccdescMetricsIterator returns an iterator
func NewInteccdescMetricsIterator() (*InteccdescMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("InteccdescMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &InteccdescMetricsIterator{iter: iter}, nil
}

type PbpbcintpbusviolationMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SopSopIn_0 gometrics.Counter

	SopSopIn_1 gometrics.Counter

	SopSopIn_2 gometrics.Counter

	SopSopIn_3 gometrics.Counter

	SopSopIn_4 gometrics.Counter

	SopSopIn_5 gometrics.Counter

	SopSopIn_6 gometrics.Counter

	SopSopIn_7 gometrics.Counter

	SopSopIn_8 gometrics.Counter

	SopSopIn_9 gometrics.Counter

	SopSopIn_10 gometrics.Counter

	SopSopIn_11 gometrics.Counter

	EopEopIn_0 gometrics.Counter

	EopEopIn_1 gometrics.Counter

	EopEopIn_2 gometrics.Counter

	EopEopIn_3 gometrics.Counter

	EopEopIn_4 gometrics.Counter

	EopEopIn_5 gometrics.Counter

	EopEopIn_6 gometrics.Counter

	EopEopIn_7 gometrics.Counter

	EopEopIn_8 gometrics.Counter

	EopEopIn_9 gometrics.Counter

	EopEopIn_10 gometrics.Counter

	EopEopIn_11 gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbcintpbusviolationMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbcintpbusviolationMetrics) Size() int {
	sz := 0

	sz += mtr.SopSopIn_0.Size()

	sz += mtr.SopSopIn_1.Size()

	sz += mtr.SopSopIn_2.Size()

	sz += mtr.SopSopIn_3.Size()

	sz += mtr.SopSopIn_4.Size()

	sz += mtr.SopSopIn_5.Size()

	sz += mtr.SopSopIn_6.Size()

	sz += mtr.SopSopIn_7.Size()

	sz += mtr.SopSopIn_8.Size()

	sz += mtr.SopSopIn_9.Size()

	sz += mtr.SopSopIn_10.Size()

	sz += mtr.SopSopIn_11.Size()

	sz += mtr.EopEopIn_0.Size()

	sz += mtr.EopEopIn_1.Size()

	sz += mtr.EopEopIn_2.Size()

	sz += mtr.EopEopIn_3.Size()

	sz += mtr.EopEopIn_4.Size()

	sz += mtr.EopEopIn_5.Size()

	sz += mtr.EopEopIn_6.Size()

	sz += mtr.EopEopIn_7.Size()

	sz += mtr.EopEopIn_8.Size()

	sz += mtr.EopEopIn_9.Size()

	sz += mtr.EopEopIn_10.Size()

	sz += mtr.EopEopIn_11.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbcintpbusviolationMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SopSopIn_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_0.Size()

	mtr.SopSopIn_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_1.Size()

	mtr.SopSopIn_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_2.Size()

	mtr.SopSopIn_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_3.Size()

	mtr.SopSopIn_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_4.Size()

	mtr.SopSopIn_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_5.Size()

	mtr.SopSopIn_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_6.Size()

	mtr.SopSopIn_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_7.Size()

	mtr.SopSopIn_8 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_8.Size()

	mtr.SopSopIn_9 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_9.Size()

	mtr.SopSopIn_10 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_10.Size()

	mtr.SopSopIn_11 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_11.Size()

	mtr.EopEopIn_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_0.Size()

	mtr.EopEopIn_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_1.Size()

	mtr.EopEopIn_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_2.Size()

	mtr.EopEopIn_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_3.Size()

	mtr.EopEopIn_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_4.Size()

	mtr.EopEopIn_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_5.Size()

	mtr.EopEopIn_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_6.Size()

	mtr.EopEopIn_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_7.Size()

	mtr.EopEopIn_8 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_8.Size()

	mtr.EopEopIn_9 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_9.Size()

	mtr.EopEopIn_10 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_10.Size()

	mtr.EopEopIn_11 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_11.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbcintpbusviolationMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SopSopIn_0" {
		return offset
	}
	offset += mtr.SopSopIn_0.Size()

	if fldName == "SopSopIn_1" {
		return offset
	}
	offset += mtr.SopSopIn_1.Size()

	if fldName == "SopSopIn_2" {
		return offset
	}
	offset += mtr.SopSopIn_2.Size()

	if fldName == "SopSopIn_3" {
		return offset
	}
	offset += mtr.SopSopIn_3.Size()

	if fldName == "SopSopIn_4" {
		return offset
	}
	offset += mtr.SopSopIn_4.Size()

	if fldName == "SopSopIn_5" {
		return offset
	}
	offset += mtr.SopSopIn_5.Size()

	if fldName == "SopSopIn_6" {
		return offset
	}
	offset += mtr.SopSopIn_6.Size()

	if fldName == "SopSopIn_7" {
		return offset
	}
	offset += mtr.SopSopIn_7.Size()

	if fldName == "SopSopIn_8" {
		return offset
	}
	offset += mtr.SopSopIn_8.Size()

	if fldName == "SopSopIn_9" {
		return offset
	}
	offset += mtr.SopSopIn_9.Size()

	if fldName == "SopSopIn_10" {
		return offset
	}
	offset += mtr.SopSopIn_10.Size()

	if fldName == "SopSopIn_11" {
		return offset
	}
	offset += mtr.SopSopIn_11.Size()

	if fldName == "EopEopIn_0" {
		return offset
	}
	offset += mtr.EopEopIn_0.Size()

	if fldName == "EopEopIn_1" {
		return offset
	}
	offset += mtr.EopEopIn_1.Size()

	if fldName == "EopEopIn_2" {
		return offset
	}
	offset += mtr.EopEopIn_2.Size()

	if fldName == "EopEopIn_3" {
		return offset
	}
	offset += mtr.EopEopIn_3.Size()

	if fldName == "EopEopIn_4" {
		return offset
	}
	offset += mtr.EopEopIn_4.Size()

	if fldName == "EopEopIn_5" {
		return offset
	}
	offset += mtr.EopEopIn_5.Size()

	if fldName == "EopEopIn_6" {
		return offset
	}
	offset += mtr.EopEopIn_6.Size()

	if fldName == "EopEopIn_7" {
		return offset
	}
	offset += mtr.EopEopIn_7.Size()

	if fldName == "EopEopIn_8" {
		return offset
	}
	offset += mtr.EopEopIn_8.Size()

	if fldName == "EopEopIn_9" {
		return offset
	}
	offset += mtr.EopEopIn_9.Size()

	if fldName == "EopEopIn_10" {
		return offset
	}
	offset += mtr.EopEopIn_10.Size()

	if fldName == "EopEopIn_11" {
		return offset
	}
	offset += mtr.EopEopIn_11.Size()

	return offset
}

// SetSopSopIn_0 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetSopSopIn_0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_0"))
	return nil
}

// SetSopSopIn_1 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetSopSopIn_1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_1"))
	return nil
}

// SetSopSopIn_2 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetSopSopIn_2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_2"))
	return nil
}

// SetSopSopIn_3 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetSopSopIn_3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_3"))
	return nil
}

// SetSopSopIn_4 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetSopSopIn_4(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_4"))
	return nil
}

// SetSopSopIn_5 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetSopSopIn_5(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_5"))
	return nil
}

// SetSopSopIn_6 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetSopSopIn_6(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_6"))
	return nil
}

// SetSopSopIn_7 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetSopSopIn_7(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_7"))
	return nil
}

// SetSopSopIn_8 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetSopSopIn_8(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_8"))
	return nil
}

// SetSopSopIn_9 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetSopSopIn_9(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_9"))
	return nil
}

// SetSopSopIn_10 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetSopSopIn_10(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_10"))
	return nil
}

// SetSopSopIn_11 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetSopSopIn_11(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_11"))
	return nil
}

// SetEopEopIn_0 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetEopEopIn_0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_0"))
	return nil
}

// SetEopEopIn_1 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetEopEopIn_1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_1"))
	return nil
}

// SetEopEopIn_2 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetEopEopIn_2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_2"))
	return nil
}

// SetEopEopIn_3 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetEopEopIn_3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_3"))
	return nil
}

// SetEopEopIn_4 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetEopEopIn_4(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_4"))
	return nil
}

// SetEopEopIn_5 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetEopEopIn_5(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_5"))
	return nil
}

// SetEopEopIn_6 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetEopEopIn_6(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_6"))
	return nil
}

// SetEopEopIn_7 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetEopEopIn_7(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_7"))
	return nil
}

// SetEopEopIn_8 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetEopEopIn_8(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_8"))
	return nil
}

// SetEopEopIn_9 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetEopEopIn_9(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_9"))
	return nil
}

// SetEopEopIn_10 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetEopEopIn_10(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_10"))
	return nil
}

// SetEopEopIn_11 sets cunter in shared memory
func (mtr *PbpbcintpbusviolationMetrics) SetEopEopIn_11(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_11"))
	return nil
}

// PbpbcintpbusviolationMetricsIterator is the iterator object
type PbpbcintpbusviolationMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbcintpbusviolationMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbcintpbusviolationMetricsIterator) Next() *PbpbcintpbusviolationMetrics {
	mtr := it.iter.Next()
	tmtr := &PbpbcintpbusviolationMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbcintpbusviolationMetricsIterator) Find(key uint64) (*PbpbcintpbusviolationMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbcintpbusviolationMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbcintpbusviolationMetricsIterator) Create(key uint64) (*PbpbcintpbusviolationMetrics, error) {
	tmtr := &PbpbcintpbusviolationMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbcintpbusviolationMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbcintpbusviolationMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewPbpbcintpbusviolationMetricsIterator returns an iterator
func NewPbpbcintpbusviolationMetricsIterator() (*PbpbcintpbusviolationMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbcintpbusviolationMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbcintpbusviolationMetricsIterator{iter: iter}, nil
}

type PbpbcintrplMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	MemoryError gometrics.Counter

	ZeroLastError gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbcintrplMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbcintrplMetrics) Size() int {
	sz := 0

	sz += mtr.MemoryError.Size()

	sz += mtr.ZeroLastError.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbcintrplMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.MemoryError = mtr.metrics.GetCounter(offset)
	offset += mtr.MemoryError.Size()

	mtr.ZeroLastError = mtr.metrics.GetCounter(offset)
	offset += mtr.ZeroLastError.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbcintrplMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "MemoryError" {
		return offset
	}
	offset += mtr.MemoryError.Size()

	if fldName == "ZeroLastError" {
		return offset
	}
	offset += mtr.ZeroLastError.Size()

	return offset
}

// SetMemoryError sets cunter in shared memory
func (mtr *PbpbcintrplMetrics) SetMemoryError(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MemoryError"))
	return nil
}

// SetZeroLastError sets cunter in shared memory
func (mtr *PbpbcintrplMetrics) SetZeroLastError(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("ZeroLastError"))
	return nil
}

// PbpbcintrplMetricsIterator is the iterator object
type PbpbcintrplMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbcintrplMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbcintrplMetricsIterator) Next() *PbpbcintrplMetrics {
	mtr := it.iter.Next()
	tmtr := &PbpbcintrplMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbcintrplMetricsIterator) Find(key uint64) (*PbpbcintrplMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbcintrplMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbcintrplMetricsIterator) Create(key uint64) (*PbpbcintrplMetrics, error) {
	tmtr := &PbpbcintrplMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbcintrplMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbcintrplMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewPbpbcintrplMetricsIterator returns an iterator
func NewPbpbcintrplMetricsIterator() (*PbpbcintrplMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbcintrplMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbcintrplMetricsIterator{iter: iter}, nil
}

type PbpbcintwriteMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	OutOfCells gometrics.Counter

	OutOfCredit gometrics.Counter

	PortDisabled gometrics.Counter

	Truncation gometrics.Counter

	IntrinsicDrop gometrics.Counter

	OutOfCells1 gometrics.Counter

	EnqErr gometrics.Counter

	TailDropCpu gometrics.Counter

	TailDropSpan gometrics.Counter

	MinSizeViol gometrics.Counter

	PortRange gometrics.Counter

	CreditGrowthError gometrics.Counter

	OqRange gometrics.Counter

	XoffTimeout gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbcintwriteMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbcintwriteMetrics) Size() int {
	sz := 0

	sz += mtr.OutOfCells.Size()

	sz += mtr.OutOfCredit.Size()

	sz += mtr.PortDisabled.Size()

	sz += mtr.Truncation.Size()

	sz += mtr.IntrinsicDrop.Size()

	sz += mtr.OutOfCells1.Size()

	sz += mtr.EnqErr.Size()

	sz += mtr.TailDropCpu.Size()

	sz += mtr.TailDropSpan.Size()

	sz += mtr.MinSizeViol.Size()

	sz += mtr.PortRange.Size()

	sz += mtr.CreditGrowthError.Size()

	sz += mtr.OqRange.Size()

	sz += mtr.XoffTimeout.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbcintwriteMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.OutOfCells = mtr.metrics.GetCounter(offset)
	offset += mtr.OutOfCells.Size()

	mtr.OutOfCredit = mtr.metrics.GetCounter(offset)
	offset += mtr.OutOfCredit.Size()

	mtr.PortDisabled = mtr.metrics.GetCounter(offset)
	offset += mtr.PortDisabled.Size()

	mtr.Truncation = mtr.metrics.GetCounter(offset)
	offset += mtr.Truncation.Size()

	mtr.IntrinsicDrop = mtr.metrics.GetCounter(offset)
	offset += mtr.IntrinsicDrop.Size()

	mtr.OutOfCells1 = mtr.metrics.GetCounter(offset)
	offset += mtr.OutOfCells1.Size()

	mtr.EnqErr = mtr.metrics.GetCounter(offset)
	offset += mtr.EnqErr.Size()

	mtr.TailDropCpu = mtr.metrics.GetCounter(offset)
	offset += mtr.TailDropCpu.Size()

	mtr.TailDropSpan = mtr.metrics.GetCounter(offset)
	offset += mtr.TailDropSpan.Size()

	mtr.MinSizeViol = mtr.metrics.GetCounter(offset)
	offset += mtr.MinSizeViol.Size()

	mtr.PortRange = mtr.metrics.GetCounter(offset)
	offset += mtr.PortRange.Size()

	mtr.CreditGrowthError = mtr.metrics.GetCounter(offset)
	offset += mtr.CreditGrowthError.Size()

	mtr.OqRange = mtr.metrics.GetCounter(offset)
	offset += mtr.OqRange.Size()

	mtr.XoffTimeout = mtr.metrics.GetCounter(offset)
	offset += mtr.XoffTimeout.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbcintwriteMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "OutOfCells" {
		return offset
	}
	offset += mtr.OutOfCells.Size()

	if fldName == "OutOfCredit" {
		return offset
	}
	offset += mtr.OutOfCredit.Size()

	if fldName == "PortDisabled" {
		return offset
	}
	offset += mtr.PortDisabled.Size()

	if fldName == "Truncation" {
		return offset
	}
	offset += mtr.Truncation.Size()

	if fldName == "IntrinsicDrop" {
		return offset
	}
	offset += mtr.IntrinsicDrop.Size()

	if fldName == "OutOfCells1" {
		return offset
	}
	offset += mtr.OutOfCells1.Size()

	if fldName == "EnqErr" {
		return offset
	}
	offset += mtr.EnqErr.Size()

	if fldName == "TailDropCpu" {
		return offset
	}
	offset += mtr.TailDropCpu.Size()

	if fldName == "TailDropSpan" {
		return offset
	}
	offset += mtr.TailDropSpan.Size()

	if fldName == "MinSizeViol" {
		return offset
	}
	offset += mtr.MinSizeViol.Size()

	if fldName == "PortRange" {
		return offset
	}
	offset += mtr.PortRange.Size()

	if fldName == "CreditGrowthError" {
		return offset
	}
	offset += mtr.CreditGrowthError.Size()

	if fldName == "OqRange" {
		return offset
	}
	offset += mtr.OqRange.Size()

	if fldName == "XoffTimeout" {
		return offset
	}
	offset += mtr.XoffTimeout.Size()

	return offset
}

// SetOutOfCells sets cunter in shared memory
func (mtr *PbpbcintwriteMetrics) SetOutOfCells(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OutOfCells"))
	return nil
}

// SetOutOfCredit sets cunter in shared memory
func (mtr *PbpbcintwriteMetrics) SetOutOfCredit(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OutOfCredit"))
	return nil
}

// SetPortDisabled sets cunter in shared memory
func (mtr *PbpbcintwriteMetrics) SetPortDisabled(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PortDisabled"))
	return nil
}

// SetTruncation sets cunter in shared memory
func (mtr *PbpbcintwriteMetrics) SetTruncation(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Truncation"))
	return nil
}

// SetIntrinsicDrop sets cunter in shared memory
func (mtr *PbpbcintwriteMetrics) SetIntrinsicDrop(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("IntrinsicDrop"))
	return nil
}

// SetOutOfCells1 sets cunter in shared memory
func (mtr *PbpbcintwriteMetrics) SetOutOfCells1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OutOfCells1"))
	return nil
}

// SetEnqErr sets cunter in shared memory
func (mtr *PbpbcintwriteMetrics) SetEnqErr(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EnqErr"))
	return nil
}

// SetTailDropCpu sets cunter in shared memory
func (mtr *PbpbcintwriteMetrics) SetTailDropCpu(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TailDropCpu"))
	return nil
}

// SetTailDropSpan sets cunter in shared memory
func (mtr *PbpbcintwriteMetrics) SetTailDropSpan(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("TailDropSpan"))
	return nil
}

// SetMinSizeViol sets cunter in shared memory
func (mtr *PbpbcintwriteMetrics) SetMinSizeViol(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("MinSizeViol"))
	return nil
}

// SetPortRange sets cunter in shared memory
func (mtr *PbpbcintwriteMetrics) SetPortRange(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("PortRange"))
	return nil
}

// SetCreditGrowthError sets cunter in shared memory
func (mtr *PbpbcintwriteMetrics) SetCreditGrowthError(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CreditGrowthError"))
	return nil
}

// SetOqRange sets cunter in shared memory
func (mtr *PbpbcintwriteMetrics) SetOqRange(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("OqRange"))
	return nil
}

// SetXoffTimeout sets cunter in shared memory
func (mtr *PbpbcintwriteMetrics) SetXoffTimeout(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("XoffTimeout"))
	return nil
}

// PbpbcintwriteMetricsIterator is the iterator object
type PbpbcintwriteMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbcintwriteMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbcintwriteMetricsIterator) Next() *PbpbcintwriteMetrics {
	mtr := it.iter.Next()
	tmtr := &PbpbcintwriteMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbcintwriteMetricsIterator) Find(key uint64) (*PbpbcintwriteMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbcintwriteMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbcintwriteMetricsIterator) Create(key uint64) (*PbpbcintwriteMetrics, error) {
	tmtr := &PbpbcintwriteMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbcintwriteMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbcintwriteMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewPbpbcintwriteMetricsIterator returns an iterator
func NewPbpbcintwriteMetricsIterator() (*PbpbcintwriteMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbcintwriteMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbcintwriteMetricsIterator{iter: iter}, nil
}

type PbpbchbmintecchbmrbMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	RbUncorrectable gometrics.Counter

	RbCorrectable gometrics.Counter

	CdtUncorrectable gometrics.Counter

	CdtCorrectable gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbchbmintecchbmrbMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbchbmintecchbmrbMetrics) Size() int {
	sz := 0

	sz += mtr.RbUncorrectable.Size()

	sz += mtr.RbCorrectable.Size()

	sz += mtr.CdtUncorrectable.Size()

	sz += mtr.CdtCorrectable.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbchbmintecchbmrbMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.RbUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.RbUncorrectable.Size()

	mtr.RbCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.RbCorrectable.Size()

	mtr.CdtUncorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.CdtUncorrectable.Size()

	mtr.CdtCorrectable = mtr.metrics.GetCounter(offset)
	offset += mtr.CdtCorrectable.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbchbmintecchbmrbMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "RbUncorrectable" {
		return offset
	}
	offset += mtr.RbUncorrectable.Size()

	if fldName == "RbCorrectable" {
		return offset
	}
	offset += mtr.RbCorrectable.Size()

	if fldName == "CdtUncorrectable" {
		return offset
	}
	offset += mtr.CdtUncorrectable.Size()

	if fldName == "CdtCorrectable" {
		return offset
	}
	offset += mtr.CdtCorrectable.Size()

	return offset
}

// SetRbUncorrectable sets cunter in shared memory
func (mtr *PbpbchbmintecchbmrbMetrics) SetRbUncorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RbUncorrectable"))
	return nil
}

// SetRbCorrectable sets cunter in shared memory
func (mtr *PbpbchbmintecchbmrbMetrics) SetRbCorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("RbCorrectable"))
	return nil
}

// SetCdtUncorrectable sets cunter in shared memory
func (mtr *PbpbchbmintecchbmrbMetrics) SetCdtUncorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CdtUncorrectable"))
	return nil
}

// SetCdtCorrectable sets cunter in shared memory
func (mtr *PbpbchbmintecchbmrbMetrics) SetCdtCorrectable(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CdtCorrectable"))
	return nil
}

// PbpbchbmintecchbmrbMetricsIterator is the iterator object
type PbpbchbmintecchbmrbMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbchbmintecchbmrbMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbchbmintecchbmrbMetricsIterator) Next() *PbpbchbmintecchbmrbMetrics {
	mtr := it.iter.Next()
	tmtr := &PbpbchbmintecchbmrbMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbchbmintecchbmrbMetricsIterator) Find(key uint64) (*PbpbchbmintecchbmrbMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbchbmintecchbmrbMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbchbmintecchbmrbMetricsIterator) Create(key uint64) (*PbpbchbmintecchbmrbMetrics, error) {
	tmtr := &PbpbchbmintecchbmrbMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbchbmintecchbmrbMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbchbmintecchbmrbMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewPbpbchbmintecchbmrbMetricsIterator returns an iterator
func NewPbpbchbmintecchbmrbMetricsIterator() (*PbpbchbmintecchbmrbMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbchbmintecchbmrbMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbchbmintecchbmrbMetricsIterator{iter: iter}, nil
}

type PbpbchbminthbmaxierrrspMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Ctrl gometrics.Counter

	Pyld gometrics.Counter

	R2A gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbchbminthbmaxierrrspMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbchbminthbmaxierrrspMetrics) Size() int {
	sz := 0

	sz += mtr.Ctrl.Size()

	sz += mtr.Pyld.Size()

	sz += mtr.R2A.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbchbminthbmaxierrrspMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Ctrl = mtr.metrics.GetCounter(offset)
	offset += mtr.Ctrl.Size()

	mtr.Pyld = mtr.metrics.GetCounter(offset)
	offset += mtr.Pyld.Size()

	mtr.R2A = mtr.metrics.GetCounter(offset)
	offset += mtr.R2A.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbchbminthbmaxierrrspMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Ctrl" {
		return offset
	}
	offset += mtr.Ctrl.Size()

	if fldName == "Pyld" {
		return offset
	}
	offset += mtr.Pyld.Size()

	if fldName == "R2A" {
		return offset
	}
	offset += mtr.R2A.Size()

	return offset
}

// SetCtrl sets cunter in shared memory
func (mtr *PbpbchbminthbmaxierrrspMetrics) SetCtrl(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ctrl"))
	return nil
}

// SetPyld sets cunter in shared memory
func (mtr *PbpbchbminthbmaxierrrspMetrics) SetPyld(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Pyld"))
	return nil
}

// SetR2A sets cunter in shared memory
func (mtr *PbpbchbminthbmaxierrrspMetrics) SetR2A(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("R2A"))
	return nil
}

// PbpbchbminthbmaxierrrspMetricsIterator is the iterator object
type PbpbchbminthbmaxierrrspMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbchbminthbmaxierrrspMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbchbminthbmaxierrrspMetricsIterator) Next() *PbpbchbminthbmaxierrrspMetrics {
	mtr := it.iter.Next()
	tmtr := &PbpbchbminthbmaxierrrspMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbchbminthbmaxierrrspMetricsIterator) Find(key uint64) (*PbpbchbminthbmaxierrrspMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbchbminthbmaxierrrspMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbchbminthbmaxierrrspMetricsIterator) Create(key uint64) (*PbpbchbminthbmaxierrrspMetrics, error) {
	tmtr := &PbpbchbminthbmaxierrrspMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbchbminthbmaxierrrspMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbchbminthbmaxierrrspMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewPbpbchbminthbmaxierrrspMetricsIterator returns an iterator
func NewPbpbchbminthbmaxierrrspMetricsIterator() (*PbpbchbminthbmaxierrrspMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbchbminthbmaxierrrspMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbchbminthbmaxierrrspMetricsIterator{iter: iter}, nil
}

type PbpbchbminthbmdropMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Occupancy_0 gometrics.Counter

	Occupancy_1 gometrics.Counter

	Occupancy_2 gometrics.Counter

	Occupancy_3 gometrics.Counter

	Occupancy_4 gometrics.Counter

	Occupancy_5 gometrics.Counter

	Occupancy_6 gometrics.Counter

	Occupancy_7 gometrics.Counter

	Occupancy_8 gometrics.Counter

	Occupancy_9 gometrics.Counter

	CtrlFull_0 gometrics.Counter

	CtrlFull_1 gometrics.Counter

	CtrlFull_2 gometrics.Counter

	CtrlFull_3 gometrics.Counter

	CtrlFull_4 gometrics.Counter

	CtrlFull_5 gometrics.Counter

	CtrlFull_6 gometrics.Counter

	CtrlFull_7 gometrics.Counter

	CtrlFull_8 gometrics.Counter

	CtrlFull_9 gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbchbminthbmdropMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbchbminthbmdropMetrics) Size() int {
	sz := 0

	sz += mtr.Occupancy_0.Size()

	sz += mtr.Occupancy_1.Size()

	sz += mtr.Occupancy_2.Size()

	sz += mtr.Occupancy_3.Size()

	sz += mtr.Occupancy_4.Size()

	sz += mtr.Occupancy_5.Size()

	sz += mtr.Occupancy_6.Size()

	sz += mtr.Occupancy_7.Size()

	sz += mtr.Occupancy_8.Size()

	sz += mtr.Occupancy_9.Size()

	sz += mtr.CtrlFull_0.Size()

	sz += mtr.CtrlFull_1.Size()

	sz += mtr.CtrlFull_2.Size()

	sz += mtr.CtrlFull_3.Size()

	sz += mtr.CtrlFull_4.Size()

	sz += mtr.CtrlFull_5.Size()

	sz += mtr.CtrlFull_6.Size()

	sz += mtr.CtrlFull_7.Size()

	sz += mtr.CtrlFull_8.Size()

	sz += mtr.CtrlFull_9.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbchbminthbmdropMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Occupancy_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Occupancy_0.Size()

	mtr.Occupancy_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Occupancy_1.Size()

	mtr.Occupancy_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.Occupancy_2.Size()

	mtr.Occupancy_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.Occupancy_3.Size()

	mtr.Occupancy_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.Occupancy_4.Size()

	mtr.Occupancy_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.Occupancy_5.Size()

	mtr.Occupancy_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.Occupancy_6.Size()

	mtr.Occupancy_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.Occupancy_7.Size()

	mtr.Occupancy_8 = mtr.metrics.GetCounter(offset)
	offset += mtr.Occupancy_8.Size()

	mtr.Occupancy_9 = mtr.metrics.GetCounter(offset)
	offset += mtr.Occupancy_9.Size()

	mtr.CtrlFull_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.CtrlFull_0.Size()

	mtr.CtrlFull_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.CtrlFull_1.Size()

	mtr.CtrlFull_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.CtrlFull_2.Size()

	mtr.CtrlFull_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.CtrlFull_3.Size()

	mtr.CtrlFull_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.CtrlFull_4.Size()

	mtr.CtrlFull_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.CtrlFull_5.Size()

	mtr.CtrlFull_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.CtrlFull_6.Size()

	mtr.CtrlFull_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.CtrlFull_7.Size()

	mtr.CtrlFull_8 = mtr.metrics.GetCounter(offset)
	offset += mtr.CtrlFull_8.Size()

	mtr.CtrlFull_9 = mtr.metrics.GetCounter(offset)
	offset += mtr.CtrlFull_9.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbchbminthbmdropMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Occupancy_0" {
		return offset
	}
	offset += mtr.Occupancy_0.Size()

	if fldName == "Occupancy_1" {
		return offset
	}
	offset += mtr.Occupancy_1.Size()

	if fldName == "Occupancy_2" {
		return offset
	}
	offset += mtr.Occupancy_2.Size()

	if fldName == "Occupancy_3" {
		return offset
	}
	offset += mtr.Occupancy_3.Size()

	if fldName == "Occupancy_4" {
		return offset
	}
	offset += mtr.Occupancy_4.Size()

	if fldName == "Occupancy_5" {
		return offset
	}
	offset += mtr.Occupancy_5.Size()

	if fldName == "Occupancy_6" {
		return offset
	}
	offset += mtr.Occupancy_6.Size()

	if fldName == "Occupancy_7" {
		return offset
	}
	offset += mtr.Occupancy_7.Size()

	if fldName == "Occupancy_8" {
		return offset
	}
	offset += mtr.Occupancy_8.Size()

	if fldName == "Occupancy_9" {
		return offset
	}
	offset += mtr.Occupancy_9.Size()

	if fldName == "CtrlFull_0" {
		return offset
	}
	offset += mtr.CtrlFull_0.Size()

	if fldName == "CtrlFull_1" {
		return offset
	}
	offset += mtr.CtrlFull_1.Size()

	if fldName == "CtrlFull_2" {
		return offset
	}
	offset += mtr.CtrlFull_2.Size()

	if fldName == "CtrlFull_3" {
		return offset
	}
	offset += mtr.CtrlFull_3.Size()

	if fldName == "CtrlFull_4" {
		return offset
	}
	offset += mtr.CtrlFull_4.Size()

	if fldName == "CtrlFull_5" {
		return offset
	}
	offset += mtr.CtrlFull_5.Size()

	if fldName == "CtrlFull_6" {
		return offset
	}
	offset += mtr.CtrlFull_6.Size()

	if fldName == "CtrlFull_7" {
		return offset
	}
	offset += mtr.CtrlFull_7.Size()

	if fldName == "CtrlFull_8" {
		return offset
	}
	offset += mtr.CtrlFull_8.Size()

	if fldName == "CtrlFull_9" {
		return offset
	}
	offset += mtr.CtrlFull_9.Size()

	return offset
}

// SetOccupancy_0 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetOccupancy_0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Occupancy_0"))
	return nil
}

// SetOccupancy_1 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetOccupancy_1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Occupancy_1"))
	return nil
}

// SetOccupancy_2 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetOccupancy_2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Occupancy_2"))
	return nil
}

// SetOccupancy_3 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetOccupancy_3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Occupancy_3"))
	return nil
}

// SetOccupancy_4 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetOccupancy_4(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Occupancy_4"))
	return nil
}

// SetOccupancy_5 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetOccupancy_5(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Occupancy_5"))
	return nil
}

// SetOccupancy_6 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetOccupancy_6(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Occupancy_6"))
	return nil
}

// SetOccupancy_7 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetOccupancy_7(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Occupancy_7"))
	return nil
}

// SetOccupancy_8 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetOccupancy_8(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Occupancy_8"))
	return nil
}

// SetOccupancy_9 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetOccupancy_9(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Occupancy_9"))
	return nil
}

// SetCtrlFull_0 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetCtrlFull_0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CtrlFull_0"))
	return nil
}

// SetCtrlFull_1 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetCtrlFull_1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CtrlFull_1"))
	return nil
}

// SetCtrlFull_2 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetCtrlFull_2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CtrlFull_2"))
	return nil
}

// SetCtrlFull_3 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetCtrlFull_3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CtrlFull_3"))
	return nil
}

// SetCtrlFull_4 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetCtrlFull_4(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CtrlFull_4"))
	return nil
}

// SetCtrlFull_5 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetCtrlFull_5(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CtrlFull_5"))
	return nil
}

// SetCtrlFull_6 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetCtrlFull_6(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CtrlFull_6"))
	return nil
}

// SetCtrlFull_7 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetCtrlFull_7(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CtrlFull_7"))
	return nil
}

// SetCtrlFull_8 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetCtrlFull_8(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CtrlFull_8"))
	return nil
}

// SetCtrlFull_9 sets cunter in shared memory
func (mtr *PbpbchbminthbmdropMetrics) SetCtrlFull_9(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("CtrlFull_9"))
	return nil
}

// PbpbchbminthbmdropMetricsIterator is the iterator object
type PbpbchbminthbmdropMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbchbminthbmdropMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbchbminthbmdropMetricsIterator) Next() *PbpbchbminthbmdropMetrics {
	mtr := it.iter.Next()
	tmtr := &PbpbchbminthbmdropMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbchbminthbmdropMetricsIterator) Find(key uint64) (*PbpbchbminthbmdropMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbchbminthbmdropMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbchbminthbmdropMetricsIterator) Create(key uint64) (*PbpbchbminthbmdropMetrics, error) {
	tmtr := &PbpbchbminthbmdropMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbchbminthbmdropMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbchbminthbmdropMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewPbpbchbminthbmdropMetricsIterator returns an iterator
func NewPbpbchbminthbmdropMetricsIterator() (*PbpbchbminthbmdropMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbchbminthbmdropMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbchbminthbmdropMetricsIterator{iter: iter}, nil
}

type PbpbchbminthbmpbusviolationMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	SopSopIn_0 gometrics.Counter

	SopSopIn_1 gometrics.Counter

	SopSopIn_2 gometrics.Counter

	SopSopIn_3 gometrics.Counter

	SopSopIn_4 gometrics.Counter

	SopSopIn_5 gometrics.Counter

	SopSopIn_6 gometrics.Counter

	SopSopIn_7 gometrics.Counter

	SopSopIn_8 gometrics.Counter

	SopSopIn_9 gometrics.Counter

	EopEopIn_0 gometrics.Counter

	EopEopIn_1 gometrics.Counter

	EopEopIn_2 gometrics.Counter

	EopEopIn_3 gometrics.Counter

	EopEopIn_4 gometrics.Counter

	EopEopIn_5 gometrics.Counter

	EopEopIn_6 gometrics.Counter

	EopEopIn_7 gometrics.Counter

	EopEopIn_8 gometrics.Counter

	EopEopIn_9 gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbchbminthbmpbusviolationMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbchbminthbmpbusviolationMetrics) Size() int {
	sz := 0

	sz += mtr.SopSopIn_0.Size()

	sz += mtr.SopSopIn_1.Size()

	sz += mtr.SopSopIn_2.Size()

	sz += mtr.SopSopIn_3.Size()

	sz += mtr.SopSopIn_4.Size()

	sz += mtr.SopSopIn_5.Size()

	sz += mtr.SopSopIn_6.Size()

	sz += mtr.SopSopIn_7.Size()

	sz += mtr.SopSopIn_8.Size()

	sz += mtr.SopSopIn_9.Size()

	sz += mtr.EopEopIn_0.Size()

	sz += mtr.EopEopIn_1.Size()

	sz += mtr.EopEopIn_2.Size()

	sz += mtr.EopEopIn_3.Size()

	sz += mtr.EopEopIn_4.Size()

	sz += mtr.EopEopIn_5.Size()

	sz += mtr.EopEopIn_6.Size()

	sz += mtr.EopEopIn_7.Size()

	sz += mtr.EopEopIn_8.Size()

	sz += mtr.EopEopIn_9.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.SopSopIn_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_0.Size()

	mtr.SopSopIn_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_1.Size()

	mtr.SopSopIn_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_2.Size()

	mtr.SopSopIn_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_3.Size()

	mtr.SopSopIn_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_4.Size()

	mtr.SopSopIn_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_5.Size()

	mtr.SopSopIn_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_6.Size()

	mtr.SopSopIn_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_7.Size()

	mtr.SopSopIn_8 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_8.Size()

	mtr.SopSopIn_9 = mtr.metrics.GetCounter(offset)
	offset += mtr.SopSopIn_9.Size()

	mtr.EopEopIn_0 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_0.Size()

	mtr.EopEopIn_1 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_1.Size()

	mtr.EopEopIn_2 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_2.Size()

	mtr.EopEopIn_3 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_3.Size()

	mtr.EopEopIn_4 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_4.Size()

	mtr.EopEopIn_5 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_5.Size()

	mtr.EopEopIn_6 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_6.Size()

	mtr.EopEopIn_7 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_7.Size()

	mtr.EopEopIn_8 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_8.Size()

	mtr.EopEopIn_9 = mtr.metrics.GetCounter(offset)
	offset += mtr.EopEopIn_9.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "SopSopIn_0" {
		return offset
	}
	offset += mtr.SopSopIn_0.Size()

	if fldName == "SopSopIn_1" {
		return offset
	}
	offset += mtr.SopSopIn_1.Size()

	if fldName == "SopSopIn_2" {
		return offset
	}
	offset += mtr.SopSopIn_2.Size()

	if fldName == "SopSopIn_3" {
		return offset
	}
	offset += mtr.SopSopIn_3.Size()

	if fldName == "SopSopIn_4" {
		return offset
	}
	offset += mtr.SopSopIn_4.Size()

	if fldName == "SopSopIn_5" {
		return offset
	}
	offset += mtr.SopSopIn_5.Size()

	if fldName == "SopSopIn_6" {
		return offset
	}
	offset += mtr.SopSopIn_6.Size()

	if fldName == "SopSopIn_7" {
		return offset
	}
	offset += mtr.SopSopIn_7.Size()

	if fldName == "SopSopIn_8" {
		return offset
	}
	offset += mtr.SopSopIn_8.Size()

	if fldName == "SopSopIn_9" {
		return offset
	}
	offset += mtr.SopSopIn_9.Size()

	if fldName == "EopEopIn_0" {
		return offset
	}
	offset += mtr.EopEopIn_0.Size()

	if fldName == "EopEopIn_1" {
		return offset
	}
	offset += mtr.EopEopIn_1.Size()

	if fldName == "EopEopIn_2" {
		return offset
	}
	offset += mtr.EopEopIn_2.Size()

	if fldName == "EopEopIn_3" {
		return offset
	}
	offset += mtr.EopEopIn_3.Size()

	if fldName == "EopEopIn_4" {
		return offset
	}
	offset += mtr.EopEopIn_4.Size()

	if fldName == "EopEopIn_5" {
		return offset
	}
	offset += mtr.EopEopIn_5.Size()

	if fldName == "EopEopIn_6" {
		return offset
	}
	offset += mtr.EopEopIn_6.Size()

	if fldName == "EopEopIn_7" {
		return offset
	}
	offset += mtr.EopEopIn_7.Size()

	if fldName == "EopEopIn_8" {
		return offset
	}
	offset += mtr.EopEopIn_8.Size()

	if fldName == "EopEopIn_9" {
		return offset
	}
	offset += mtr.EopEopIn_9.Size()

	return offset
}

// SetSopSopIn_0 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) SetSopSopIn_0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_0"))
	return nil
}

// SetSopSopIn_1 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) SetSopSopIn_1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_1"))
	return nil
}

// SetSopSopIn_2 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) SetSopSopIn_2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_2"))
	return nil
}

// SetSopSopIn_3 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) SetSopSopIn_3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_3"))
	return nil
}

// SetSopSopIn_4 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) SetSopSopIn_4(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_4"))
	return nil
}

// SetSopSopIn_5 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) SetSopSopIn_5(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_5"))
	return nil
}

// SetSopSopIn_6 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) SetSopSopIn_6(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_6"))
	return nil
}

// SetSopSopIn_7 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) SetSopSopIn_7(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_7"))
	return nil
}

// SetSopSopIn_8 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) SetSopSopIn_8(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_8"))
	return nil
}

// SetSopSopIn_9 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) SetSopSopIn_9(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("SopSopIn_9"))
	return nil
}

// SetEopEopIn_0 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) SetEopEopIn_0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_0"))
	return nil
}

// SetEopEopIn_1 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) SetEopEopIn_1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_1"))
	return nil
}

// SetEopEopIn_2 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) SetEopEopIn_2(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_2"))
	return nil
}

// SetEopEopIn_3 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) SetEopEopIn_3(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_3"))
	return nil
}

// SetEopEopIn_4 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) SetEopEopIn_4(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_4"))
	return nil
}

// SetEopEopIn_5 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) SetEopEopIn_5(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_5"))
	return nil
}

// SetEopEopIn_6 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) SetEopEopIn_6(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_6"))
	return nil
}

// SetEopEopIn_7 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) SetEopEopIn_7(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_7"))
	return nil
}

// SetEopEopIn_8 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) SetEopEopIn_8(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_8"))
	return nil
}

// SetEopEopIn_9 sets cunter in shared memory
func (mtr *PbpbchbminthbmpbusviolationMetrics) SetEopEopIn_9(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("EopEopIn_9"))
	return nil
}

// PbpbchbminthbmpbusviolationMetricsIterator is the iterator object
type PbpbchbminthbmpbusviolationMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbchbminthbmpbusviolationMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbchbminthbmpbusviolationMetricsIterator) Next() *PbpbchbminthbmpbusviolationMetrics {
	mtr := it.iter.Next()
	tmtr := &PbpbchbminthbmpbusviolationMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbchbminthbmpbusviolationMetricsIterator) Find(key uint64) (*PbpbchbminthbmpbusviolationMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbchbminthbmpbusviolationMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbchbminthbmpbusviolationMetricsIterator) Create(key uint64) (*PbpbchbminthbmpbusviolationMetrics, error) {
	tmtr := &PbpbchbminthbmpbusviolationMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbchbminthbmpbusviolationMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbchbminthbmpbusviolationMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewPbpbchbminthbmpbusviolationMetricsIterator returns an iterator
func NewPbpbchbminthbmpbusviolationMetricsIterator() (*PbpbchbminthbmpbusviolationMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbchbminthbmpbusviolationMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbchbminthbmpbusviolationMetricsIterator{iter: iter}, nil
}

type PbpbchbminthbmxoffMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Timeout gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *PbpbchbminthbmxoffMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *PbpbchbminthbmxoffMetrics) Size() int {
	sz := 0

	sz += mtr.Timeout.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *PbpbchbminthbmxoffMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Timeout = mtr.metrics.GetCounter(offset)
	offset += mtr.Timeout.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Timeout" {
		return offset
	}
	offset += mtr.Timeout.Size()

	return offset
}

// SetTimeout sets cunter in shared memory
func (mtr *PbpbchbminthbmxoffMetrics) SetTimeout(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Timeout"))
	return nil
}

// PbpbchbminthbmxoffMetricsIterator is the iterator object
type PbpbchbminthbmxoffMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *PbpbchbminthbmxoffMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *PbpbchbminthbmxoffMetricsIterator) Next() *PbpbchbminthbmxoffMetrics {
	mtr := it.iter.Next()
	tmtr := &PbpbchbminthbmxoffMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *PbpbchbminthbmxoffMetricsIterator) Find(key uint64) (*PbpbchbminthbmxoffMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &PbpbchbminthbmxoffMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *PbpbchbminthbmxoffMetricsIterator) Create(key uint64) (*PbpbchbminthbmxoffMetrics, error) {
	tmtr := &PbpbchbminthbmxoffMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &PbpbchbminthbmxoffMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *PbpbchbminthbmxoffMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewPbpbchbminthbmxoffMetricsIterator returns an iterator
func NewPbpbchbminthbmxoffMetricsIterator() (*PbpbchbminthbmxoffMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("PbpbchbminthbmxoffMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &PbpbchbminthbmxoffMetricsIterator{iter: iter}, nil
}

type McmchintmcMetrics struct {
	ObjectMeta api.ObjectMeta

	key uint64

	Ecc_1BitThresPs1 gometrics.Counter

	Ecc_1BitThresPs0 gometrics.Counter

	// private state
	metrics gometrics.Metrics
}

func (mtr *McmchintmcMetrics) GetKey() uint64 {
	return mtr.key
}

// Size returns the size of the metrics object
func (mtr *McmchintmcMetrics) Size() int {
	sz := 0

	sz += mtr.Ecc_1BitThresPs1.Size()

	sz += mtr.Ecc_1BitThresPs0.Size()

	return sz
}

// Unmarshal unmarshal the raw counters from shared memory
func (mtr *McmchintmcMetrics) Unmarshal() error {
	var offset int

	gometrics.DecodeScalarKey(&mtr.key, mtr.metrics.GetKey())

	mtr.Ecc_1BitThresPs1 = mtr.metrics.GetCounter(offset)
	offset += mtr.Ecc_1BitThresPs1.Size()

	mtr.Ecc_1BitThresPs0 = mtr.metrics.GetCounter(offset)
	offset += mtr.Ecc_1BitThresPs0.Size()

	return nil
}

// getOffset returns the offset for raw counters in shared memory
func (mtr *McmchintmcMetrics) getOffset(fldName string) int {
	var offset int

	if fldName == "Ecc_1BitThresPs1" {
		return offset
	}
	offset += mtr.Ecc_1BitThresPs1.Size()

	if fldName == "Ecc_1BitThresPs0" {
		return offset
	}
	offset += mtr.Ecc_1BitThresPs0.Size()

	return offset
}

// SetEcc_1BitThresPs1 sets cunter in shared memory
func (mtr *McmchintmcMetrics) SetEcc_1BitThresPs1(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ecc_1BitThresPs1"))
	return nil
}

// SetEcc_1BitThresPs0 sets cunter in shared memory
func (mtr *McmchintmcMetrics) SetEcc_1BitThresPs0(val gometrics.Counter) error {
	mtr.metrics.SetCounter(val, mtr.getOffset("Ecc_1BitThresPs0"))
	return nil
}

// McmchintmcMetricsIterator is the iterator object
type McmchintmcMetricsIterator struct {
	iter gometrics.MetricsIterator
}

// HasNext returns true if there are more objects
func (it *McmchintmcMetricsIterator) HasNext() bool {
	return it.iter.HasNext()
}

// Next returns the next metrics
func (it *McmchintmcMetricsIterator) Next() *McmchintmcMetrics {
	mtr := it.iter.Next()
	tmtr := &McmchintmcMetrics{metrics: mtr}
	tmtr.Unmarshal()
	return tmtr
}

// Find finds the metrics object by key

func (it *McmchintmcMetricsIterator) Find(key uint64) (*McmchintmcMetrics, error) {

	mtr, err := it.iter.Find(gometrics.EncodeScalarKey(key))

	if err != nil {
		return nil, err
	}
	tmtr := &McmchintmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Create creates the object in shared memory

func (it *McmchintmcMetricsIterator) Create(key uint64) (*McmchintmcMetrics, error) {
	tmtr := &McmchintmcMetrics{}

	mtr := it.iter.Create(gometrics.EncodeScalarKey(key), tmtr.Size())

	tmtr = &McmchintmcMetrics{metrics: mtr, key: key}
	tmtr.Unmarshal()
	return tmtr, nil
}

// Delete deletes the object from shared memory

func (it *McmchintmcMetricsIterator) Delete(key uint64) error {

	return it.iter.Delete(gometrics.EncodeScalarKey(key))

}

// NewMcmchintmcMetricsIterator returns an iterator
func NewMcmchintmcMetricsIterator() (*McmchintmcMetricsIterator, error) {
	iter, err := gometrics.NewMetricsIterator("McmchintmcMetrics")
	if err != nil {
		return nil, err
	}
	// little hack to skip creating iterators on osx
	if iter == nil {
		return nil, nil
	}

	return &McmchintmcMetricsIterator{iter: iter}, nil
}
